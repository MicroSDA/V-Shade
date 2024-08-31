#version 460 core
#pragma: compute
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
#include "./lighting/Light.glsl"

#define MAX_LIGHT_COUNT 1024

layout(local_size_x = 16, local_size_y = 16) in;
layout(set = PER_INSTANCE_SET, binding = 9) uniform sampler2D t_DepthMap;
//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Uniform buffer containing the scene data
layout (std140, set = GLOBAL_SET, binding = SCENE_DATA_BINDING) uniform UGlobalSceneData
{
    SceneData u_SceneData;
};
//Storage buffer containing the point light data
layout (std430, set = GLOBAL_SET, binding = POINT_LIGHT_BINDING) restrict readonly buffer SPointLight
{
	PointLight s_PointLight[];
};
//Storage buffer containing the point light data
layout (std430, set = GLOBAL_SET, binding = SPOT_LIGHT_BINDING) restrict readonly buffer SSpotLight
{
	SpotLight s_SpotLight[];
};
layout(push_constant) uniform UResolution
{
	uvec2 Resoulution;
} u_Resolution;

shared uint MinDepthInt;
shared uint MaxDepthInt;
shared uint VisiblePointLightCount;
shared uint VisibleSpotLightCount;
shared vec4 FrustumPlanes[6];

// Shared local storage for visible indices, will be written out to the global buffer at the end
shared int VisiblePointLightIndices[MAX_POINT_LIGHTS];
shared int VisibleSpotLightIndices[MAX_SPOT_LIGHTS];

float ScreenSpaceToViewSpaceDepth(const float screenDepth)
{
	// float depthLinearizeMul = (u_Camera.Far * u_Camera.Near ) / ( u_Camera.Far - u_Camera.Near);
	// float depthLinearizeAdd =  u_Camera.Far / ( u_Camera.Far - u_Camera.Near );

	float depthLinearizeMul = u_Camera.Near;
	float depthLinearizeAdd = u_Camera.Far;

	// Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
	return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}

// TODO: Create sphere instead of frustim for psot light
//https://simoncoenen.com/blog/programming/graphics/SpotlightCulling
//https://bartwronski.com/2017/04/13/cull-that-cone/
bool TestConeVsSphere(SpotLight spotLight, vec4 testSphere)
{
	const float radius = dot(testSphere.xy, testSphere.zw);
    const vec3 V = testSphere.xyz - spotLight.Position;
    const float  VlenSq = dot(V, V);
    const float  V1len  = dot(V, spotLight.Direction);
    const float  distanceClosestPoint = cos(spotLight.MaxAngle) * sqrt(VlenSq - V1len*V1len) - V1len * sin(spotLight.MaxAngle);
 
    const bool angleCull = distanceClosestPoint > testSphere.w;
    const bool frontCull = V1len >  testSphere.w + spotLight.Distance;
    const bool backCull  = V1len < -testSphere.w;
	return !(angleCull || frontCull || backCull);
}	

void main()
{
	ivec2 Location      = ivec2(gl_GlobalInvocationID.xy);
    ivec2 ItemID        = ivec2(gl_LocalInvocationID.xy);
    ivec2 TileID        = ivec2(gl_WorkGroupID.xy);
    ivec2 TileNumber    = ivec2(gl_NumWorkGroups.xy);
    uint  Index         = TileID.y * TileNumber.x + TileID.x;

    // Initialize shared global values for depth and light count
    if (gl_LocalInvocationIndex == 0)
    {
		MinDepthInt             = 0xFFFFFFFF;
		MaxDepthInt             = 0;
		VisiblePointLightCount  = 0;
		VisibleSpotLightCount   = 0;
    }

    barrier();
    // Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
	vec2 TC = vec2(Location) / vec2(u_Resolution.Resoulution);
    float LinearDepth = ScreenSpaceToViewSpaceDepth(textureLod(t_DepthMap, TC, 0).r);

    // Convert depth to uint so we can do atomic min and max comparisons between the threads
    uint DepthInt = floatBitsToUint(LinearDepth);
    atomicMin(MinDepthInt, DepthInt);
    atomicMax(MaxDepthInt, DepthInt);

    barrier();

	vec2 NegativeStep;
	vec2 PositiveStep;
    // Step 2: One thread should calculate the frustum planes to be used for this tile
    if (gl_LocalInvocationIndex == 0)
    {
		// Convert the min and max across the entire tile back to float
		float MinDepth = uintBitsToFloat(MinDepthInt);
		float MaxDepth = uintBitsToFloat(MaxDepthInt);

		// Steps based on tile sale
		NegativeStep = (2.0 * vec2(TileID)) / vec2(TileNumber);
		PositiveStep = (2.0 * vec2(TileID + ivec2(1, 1))) / vec2(TileNumber);
		
		// Set up starting values for planes using steps and min and max z values
		FrustumPlanes[0] = vec4( 1.0,  0.0,  0.0,  1.0 - NegativeStep.x); // Left
		FrustumPlanes[1] = vec4(-1.0,  0.0,  0.0, -1.0 + PositiveStep.x); // Right

		FrustumPlanes[2] = vec4( 0.0,  1.0,  0.0,  1.0 - NegativeStep.y); // Bottom // SOME ISSUE HERE !
		FrustumPlanes[3] = vec4( 0.0, -1.0,  0.0, -1.0 + PositiveStep.y); // Top

		// FrustumPlanes[2] = vec4( 0.0,  1.0,  0.0,   1.0 - NegativeStep.y); // Bottom // SOME ISSUE HERE !
		// FrustumPlanes[3] = vec4( 0.0, -1.0,  0.0,  -1.0 + PositiveStep.y); // Top
		
		FrustumPlanes[4] = vec4( 0.0,  0.0, -1.0, -MinDepth); // Near
		FrustumPlanes[5] = vec4( 0.0,  0.0,  1.0,  MaxDepth); // Far

		// Transform the first four planes
		for (uint i = 0; i < 4; i++)
		{
		    FrustumPlanes[i] *= u_Camera.ViewProjectionMatrix;
		    FrustumPlanes[i] /= length(FrustumPlanes[i].xyz);
		}

		// Transform the depth planes
		FrustumPlanes[4] *= u_Camera.ViewMatrix;
		FrustumPlanes[4] /= length(FrustumPlanes[4].xyz);

		FrustumPlanes[5] *= u_Camera.ViewMatrix;
		FrustumPlanes[5] /= length(FrustumPlanes[5].xyz);
    }

    barrier();

	// Step 3: Cull lights.
    // Parallelize the threads against the lights now.
    // Can handle 256 simultaniously. Anymore lights than that and additional passes are performed
    const uint ThreadCount = 16 * 16;
    uint PassCount = (u_SceneData.PointLightCount + ThreadCount - 1) / ThreadCount;
    for (uint i = 0; i < PassCount; i++)
    {
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint LightIndex = i * ThreadCount + gl_LocalInvocationIndex;
		if (LightIndex >= u_SceneData.PointLightCount)
		    break;
		
		vec4  Position = vec4(s_PointLight[LightIndex].Position, 1.0f);
		float Radius = s_PointLight[LightIndex].Distance;
		Radius += Radius * 0.3f;

		// Check if light radius is in frustum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++)
		{
		    distance = dot(Position, FrustumPlanes[j]) + Radius;
		    if (distance <= 0.0) // No Intersection
				break;
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0)
		{
		    // Add index to the shared array of visible indices
		    uint Offset = atomicAdd(VisiblePointLightCount, 1);
		    VisiblePointLightIndices[Offset] = int(LightIndex);
		}
    }

	PassCount = (u_SceneData.SpotLightCount + ThreadCount - 1) / ThreadCount;

	for (uint i = 0; i < PassCount; i++)
	{
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint LightIndex = i * ThreadCount + gl_LocalInvocationIndex;
		if (LightIndex >= u_SceneData.SpotLightCount)
		    break;

		SpotLight Light = s_SpotLight[LightIndex];
		float Radius = Light.Distance * acos(Light.MaxAngle * 0.9);
		// Check if light radius is in frustum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++)
		{
			distance = dot(vec4(Light.Position - (-Light.Direction * Light.Distance * 0.5), 1.0), FrustumPlanes[j]) + Radius;
			if (distance < 0.0) // No intersection
				break;
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0)
		{
			// Add index to the shared array of visible indices
			uint Offset = atomicAdd(VisibleSpotLightCount, 1);
			VisibleSpotLightIndices[Offset] = int(LightIndex);
		} 
		
	}

	barrier();

	// One thread should fill the global light buffer
    if (gl_LocalInvocationIndex == 0)
    {
		const uint Offset = Index * MAX_LIGHT_COUNT; // Determine position in global buffer
		for (uint i = 0; i < VisiblePointLightCount; i++) 
		{
			s_VisiblePointLightIndicesBuffer.Indices[Offset + i] = VisiblePointLightIndices[i];
		}

		for (uint i = 0; i < VisibleSpotLightCount; i++) {
			s_VisibleSpotLightIndicesBuffer.Indices[Offset + i] = VisibleSpotLightIndices[i];
		}

		if (VisiblePointLightCount != MAX_LIGHT_COUNT)
		{
		    // Unless we have totally filled the entire array, mark it's end with -1
		    // Final shader step will use this to determine where to stop (without having to pass the light count)
			s_VisiblePointLightIndicesBuffer.Indices[Offset + VisiblePointLightCount] = -1;
		}

		if (VisibleSpotLightCount != MAX_LIGHT_COUNT)
		{
			// Unless we have totally filled the entire array, mark it's end with -1
			// Final shader step will use this to determine where to stop (without having to pass the light count)
			s_VisibleSpotLightIndicesBuffer.Indices[Offset + VisibleSpotLightCount] = -1;
		}
    }
}