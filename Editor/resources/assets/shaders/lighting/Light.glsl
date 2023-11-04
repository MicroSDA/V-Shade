#include "./include/Common.glsl" 
// Maximum number of global lights that can be used in the scene
#define MAX_GLOBAL_LIGHTS 10
// Maximum number of point lights that can be used in the scene
#define MAX_POINT_LIGHTS 1024
// Maximum number of spot lights that can be used in the scene
#define MAX_SPOT_LIGHTS 1024
// Maximum number of cascade levels for directional light shadow maps
#define MAX_GLOBAL_LIGHT_SHADOW_CASCADE_COUNT 4
// Struct for the cascade information for a directional light shadow map
struct ShadowCascade
{
	mat4  ViewProjectionMatrix;
	float SplitDistance;
};
// Struct for a directional light
struct GlobalLight
{
	float Intensity;
	vec3  DiffuseColor;
	vec3  SpecularColor;	
	vec3  Direction;

	ShadowCascade Cascades[MAX_GLOBAL_LIGHT_SHADOW_CASCADE_COUNT];
};
// Struct for a point light
struct PointLight
{
	float Intensity;
	vec3  DiffuseColor;
	vec3  SpecularColor;
	vec3  Position;
	float Distance;
	float Falloff;
	ShadowCascade Cascade[6];
};
// Struct for a spot light
struct SpotLight
{
	float Intensity;
	vec3  DiffuseColor;
	vec3  SpecularColor;
	vec3  Position;
	vec3  Direction;
	float Distance;
	float Falloff;
	float MinAngle;
  	float MaxAngle;
	ShadowCascade Cascade;
};
// Buffer for visible indices of point lights
layout(std430, set = PER_INSTANCE_SET, binding = POINT_LIGHT_INDINCES_BINDING) buffer SVisiblePointLightIndicesBuffer
{
	int Indices[];
} s_VisiblePointLightIndicesBuffer;
// Buffer for visible indices of spot lights
layout(std430, set = PER_INSTANCE_SET, binding = SPOT_LIGHT_INDINCES_BINDING) buffer SVisibleSpotLightIndicesBuffer
{
	int Indices[];
} s_VisibleSpotLightIndicesBuffer;
// Function to get the buffer index of a point light from a fragment coordinate and an index
int GetPointLightBufferIndex(ivec2 fragCoord, int i, uint tilesCountX)
{
	// Get tile ID based on the fragment coordinate and tile size
	ivec2 tileID = fragCoord / ivec2(16, 16);
	// Calculate the index of the tile in the buffer
	uint index = tileID.y * tilesCountX + tileID.x;
	// Calculate the offset for the index of the point light in the buffer
	uint offset = index * MAX_POINT_LIGHTS;
	// Return the buffer index of the point light
	return s_VisiblePointLightIndicesBuffer.Indices[offset + i];
}
// Function to get the number of visible point lights from a fragment coordinate and a total count of point lights
int GetPointLightCount(ivec2 fragCoord, uint totalCount, uint tilesCountX)
{
	int result = 0;
	// Loop through all point lights
	for (int i = 0; i < totalCount; i++)
	{
		// Get the buffer index for the current point light
		uint lightIndex = GetPointLightBufferIndex(fragCoord, i, tilesCountX);
		// If the buffer index is -1, there are no more visible point lights in the tile
		if (lightIndex == -1)
			break;
		// Increment the count of visible point lights
		result++;
	}
	// Return the number of visible point lights
	return result;
}
// Function to get the buffer index of a spot light from a fragment coordinate and an index
int GetSpotLightBufferIndex(ivec2 fragCoord, uint i, uint tilesCountX)
{
	// Get tile ID based on the fragment coordinate and tile size
	ivec2 tileID = fragCoord / ivec2(16, 16);
	// Calculate the index of the tile in the buffer
	uint index = tileID.y * tilesCountX + tileID.x;
	// Calculate the offset for the index of the spot light in the buffer
	uint offset = index * MAX_SPOT_LIGHTS;
	// Return the buffer index of the spot light
	return s_VisibleSpotLightIndicesBuffer.Indices[offset + i];
}
// Function to get the number of visible spot lights from a fragment coordinate and a total count of spot lights
int GetSpotLightCount(ivec2 fragCoord, uint totalCount, uint tilesCountX)
{
	int result = 0;
	// Loop through all spot lights
	for (int i = 0; i < totalCount; i++)
	{
		// Get the buffer index for the current spot light
		uint lightIndex = GetSpotLightBufferIndex(fragCoord, i, tilesCountX);
		// If the buffer index is -1, there are no more visible spot lights in the tile
		if (lightIndex == -1)
			break;
		// Increment the count of visible spot lights
		result++;
	}
	// Return the number of visible spot lights
	return result;
}