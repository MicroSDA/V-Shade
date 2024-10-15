//Vertex Shader
#version 460 core
#pragma: vertex
//Include Camera header
#include "include/Common.glsl"
#include "include/Camera.glsl"
#include "include/Vertex.glsl"
//Input attributes

layout(location = 0)  in vec3  a_Position;
layout(location = 1)  in vec3  a_Normal;
layout(location = 2)  in vec3  a_Tangent;
layout(location = 3)  in vec3  a_Bitangent;
layout(location = 4)  in vec2  a_UV_Coordinates;
layout(location = 5)  in ivec4 a_BoneId;
layout(location = 6)  in vec4  a_BoneWeight;
layout(location = 7)  in mat4  a_Transform;

//Output variables
layout(location = 0) out vec2 out_UV_Coordinates;
layout(location = 1) out vec3 out_NormalWorldSpace;
layout(location = 2) out vec3 out_NormalViewSpace;
layout(location = 3) out vec3 out_VertexWorldSpace;
layout(location = 4) out vec3 out_VertexViewSpace;
layout(location = 5) flat out int out_InstanceId;
layout(location = 6) out mat3 out_TBN_Matrix;

layout(location = 9) out float out_Weight;
//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
// //Storage buffer containing the bones data
// layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform
// {
	// mat4 s_BoneTransform[];
// };

struct BoneData 
{
	mat4 s_Transform;
	int s_ParentId;
};
	
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform
{
	BoneData s_BoneTransform[];
}; 

layout(push_constant) uniform DrawInstance
{
	uint Index;
} u_DrawInstance;

//Vertex shader entry point
void main() 
{
   uint BoneInstanceOffset = gl_InstanceIndex * MAX_BONES_PER_INSTANCE;
   // TODO: Mby refactor this step wit + matrices
   mat4 BoneTransform = mat4(0.0);
   out_Weight = 0.0;
   for(uint i = 0; i < BONE_INFLUENCE; i++)
   {
	  BoneTransform += (a_BoneId[i] != ~0) ? s_BoneTransform[BoneInstanceOffset + a_BoneId[i]].s_Transform * a_BoneWeight[i] : mat4(0.0);
	  out_Weight += a_BoneWeight[i];
   }
   out_Weight /= BONE_INFLUENCE;
   
   mat4 BoneTransformWorldSpace = a_Transform * BoneTransform;
   vec4 VertexWorldSpace = BoneTransformWorldSpace * vec4(a_Position, 1.0);
   //Transform vertex to clip space
   gl_Position = u_Camera.ViewProjectionMatrix * VertexWorldSpace;
   //gl_Position.y = -gl_Position.y;	
   //Forward texture coordinates to fragment shader
   out_UV_Coordinates = vec2(a_UV_Coordinates.x, a_UV_Coordinates.y);
   //Forward instance index to fragment shader
   out_InstanceId = gl_InstanceIndex;
  
   out_NormalWorldSpace = normalize((BoneTransformWorldSpace * vec4(a_Normal, 	0.0)).xyz);
   out_NormalViewSpace  = ((mat3(u_Camera.ViewMatrix * BoneTransformWorldSpace))) * a_Normal;
   
   out_VertexWorldSpace = VertexWorldSpace.xyz;
   out_VertexViewSpace  = vec3(u_Camera.ViewMatrix * VertexWorldSpace);
  
   out_TBN_Matrix = GetTBN_Matrix(BoneTransformWorldSpace, a_Normal, a_Tangent);
}
//Fragment Shader
#version 460 core
#pragma: fragment
//Include headers
#include "include/Common.glsl"
#include "include/Camera.glsl"
#include "include/Material.glsl"
#include "include/Fragment.glsl"
#include "lighting/Blinn-Phong.glsl"
#include "shadow/Shadow-Mapping.glsl"
#include "lighting/Debug.glsl"
//Input variables

layout(location = 0) in vec2 a_UV_Coordinates;
layout(location = 1) in vec3 a_NormalWorldSpace;
layout(location = 2) in vec3 a_NormalViewSpace;
layout(location = 3) in vec3 a_VertexWorldSpace;
layout(location = 4) in vec3 a_VertexViewSpace;
layout(location = 5) flat in int a_InstanceId;
layout(location = 6) in mat3 a_TBN_Matrix;
layout(location = 9) in float a_Weight;
//Output variables
layout(location = 0) out vec4 MainColor;
layout(location = 1) out vec4 Position;
layout(location = 2) out vec4 Normal;

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Uniform buffer containing the scene data
layout (std140, set = GLOBAL_SET, binding = SCENE_DATA_BINDING) uniform USceneData
{
    SceneData u_SceneData;
};
//Uniform buffer containing the scene data
layout (std140, set = GLOBAL_SET, binding = SCENE_RENDER_SETTING_BINDING) uniform USceneRenderSettings
{
    RenderSettings u_RenderSettings;
};
//Storage buffer containing the direct light data
layout (std430, set = GLOBAL_SET, binding = GLOBAL_LIGHT_BINDING) restrict readonly buffer SGlobalLight
{
	GlobalLight s_GlobalLight[];
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
//Storage buffer containing the material data
layout (std430, set = PER_INSTANCE_SET, binding = MATERIAL_BINDING) restrict readonly buffer SMaterial
{
	Material u_Material[];
};
// Light culling structure
layout(push_constant) uniform UTilesCountX
{
	uint TilesCountX;
} u_TilesCountX;

//Textures
layout(set = PER_INSTANCE_SET, binding = DIFFUSE_TEXTURE_BINDING) 		uniform sampler2D t_DiffuseTexture;
layout(set = PER_INSTANCE_SET, binding = SPECULAR_TEXTURE_BINDING) 		uniform sampler2D t_SpecularTexture;
layout(set = PER_INSTANCE_SET, binding = NORMAL_TEXTURE_BINDING) 		uniform sampler2D t_NormalTexture;

layout(set = PER_INSTANCE_SET, binding = GLOBAL_SHADOW_MAP_BINDING) 	uniform sampler2DArray   t_GlobalightShadowMap;
layout(set = PER_INSTANCE_SET, binding = SPOT_SHADOW_MAP_BINDING) 		uniform sampler2DArray   t_SpotLightShadowMap;
layout(set = PER_INSTANCE_SET, binding = POINT_SHADOW_MAP_BINDING) 		uniform samplerCubeArray t_PointLightShadowMap;
// Maby move to Common.glsl
float LinearDepth(float depth, float near, float far)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * near * far) / (far + near - z * (far - near));	
}

//Fragment shader entry point
void main() 
{
    vec3 ToCameraDirection = normalize(u_Camera.Position - a_VertexWorldSpace);
    vec3 NormalWorldSpace  = u_Material[a_InstanceId].NormalMapEnabled ? Get_TBNNormal(texture(t_NormalTexture, a_UV_Coordinates).rgb, a_TBN_Matrix) : a_NormalWorldSpace;

	vec4    FragPosVeiwSpace    = u_Camera.ViewMatrix * vec4(a_VertexWorldSpace, 1.0);
	float   Depth               = FragPosVeiwSpace.z;

    MainColor =  vec4(0.0, 0.0, 0.0, 1.0);

    for(int i = 0; i < u_SceneData.GlobalLightCount;  i++)
    {
		uint    CascadeLevel        = 0;
		for(int j = 0; j < s_GlobalLight[i].Cascades.length() - 1; j++)
			if(Depth < s_GlobalLight[i].Cascades[j].SplitDistance)
				CascadeLevel = j + 1;

		 float Shadow = 1.0;

		 if(u_RenderSettings.GlobalShadowsEnabled)
		 {
			Shadow = GL_ShadowMapping(
						t_GlobalightShadowMap, 
						s_GlobalLight[i].Cascades[CascadeLevel].ViewProjectionMatrix, 
						CascadeLevel, 
						a_VertexWorldSpace,
						s_GlobalLight[i].Direction, NormalWorldSpace, ToCameraDirection);	
		 }

        MainColor += BilinPhongGlobalLight(
			s_GlobalLight[i],
		 	u_Material[a_InstanceId], 
			texture(t_DiffuseTexture,  a_UV_Coordinates).rgba, 
			texture(t_SpecularTexture, a_UV_Coordinates).rgba, 
			NormalWorldSpace, ToCameraDirection, Shadow);

			if(u_RenderSettings.GlobalShadowsEnabled && u_RenderSettings.ShowShadowCascades)
			{
				/* Cascades visualizing */
				if(CascadeLevel == 0)		
					MainColor += vec4(0.0, 0.2, 0, 0);
				if(CascadeLevel == 1)
					MainColor += vec4(0.0, 0.2, 0.2, 0);
				if(CascadeLevel == 2)
					MainColor += vec4(0.2, 0.2, 0.0, 0);
				if(CascadeLevel == 3)
					MainColor += vec4(0.2, 0.0, 0.0, 0);
			}
    }

    for(int i = 0; i < u_SceneData.SpotLightCount;  i++)
    {
		float Shadow = 1.0;
		uint LightIndex = (u_RenderSettings.LightCulling) ? GetSpotLightBufferIndex(ivec2(gl_FragCoord), i, u_TilesCountX.TilesCountX) : i;
		if (LightIndex == -1)
				break;
		if(u_RenderSettings.SpotShadowEnabled)
			Shadow = SPL_ShadowMapping(t_SpotLightShadowMap, s_SpotLight[LightIndex].Cascade.ViewProjectionMatrix,LightIndex, a_VertexWorldSpace, s_SpotLight[LightIndex].Direction, NormalWorldSpace, ToCameraDirection);

		MainColor += BilinPhongSpotLight(s_SpotLight[LightIndex], u_Material[a_InstanceId], texture(t_DiffuseTexture, a_UV_Coordinates).rgba, texture(t_SpecularTexture, a_UV_Coordinates).rgba, NormalWorldSpace, a_VertexWorldSpace, ToCameraDirection, Shadow);	
    }

    for(int i = 0; i < u_SceneData.PointLightCount;  i++)
    {
		float Shadow = 1.0;
		uint LightIndex = (u_RenderSettings.LightCulling) ? GetPointLightBufferIndex(ivec2(gl_FragCoord), i, u_TilesCountX.TilesCountX) : i;

		if (LightIndex == -1)
				break;

		if(u_RenderSettings.PointShadowEnabled)
			Shadow = PL_ShadowMapping(t_PointLightShadowMap, LightIndex, a_VertexWorldSpace, s_PointLight[LightIndex].Position, s_PointLight[LightIndex].Distance);

		MainColor += BilinPhongPointLight(s_PointLight[LightIndex], u_Material[a_InstanceId], texture(t_DiffuseTexture, a_UV_Coordinates).rgba, texture(t_SpecularTexture, a_UV_Coordinates).rgba, NormalWorldSpace, a_VertexWorldSpace, ToCameraDirection,  Shadow);
    }
	//Emissive
	MainColor.rgb += u_Material[a_InstanceId].DiffuseColor * u_Material[a_InstanceId].Emissive;
	// Ambient
	MainColor.rgb += vec3(texture(t_DiffuseTexture, a_UV_Coordinates).rgb * u_Material[a_InstanceId].AmbientColor);
	// SSAO
	if(u_RenderSettings.SSAOEnabled)
	{
		Position = vec4(a_VertexViewSpace, LinearDepth(gl_FragCoord.z, u_Camera.Near, u_Camera.Far));
		Normal   = vec4(normalize(a_NormalViewSpace), 1.0);
	}
	
	if(u_RenderSettings.LightCulling && u_RenderSettings.ShowLightComplexity)
	{
		float value = float(GetPointLightCount(ivec2(gl_FragCoord), u_SceneData.PointLightCount, u_TilesCountX.TilesCountX) + GetSpotLightCount(ivec2(gl_FragCoord), u_SceneData.SpotLightCount, u_TilesCountX.TilesCountX));
		MainColor.rgb += (MainColor.rgb * 0.2) + GetGradient(value);
	}
	
	if(texture(t_DiffuseTexture,  a_UV_Coordinates).a < 0.1) discard;
}