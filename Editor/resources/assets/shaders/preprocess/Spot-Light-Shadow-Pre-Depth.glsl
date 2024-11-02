#version 450 core
#pragma: vertex
#extension GL_ARB_shader_viewport_layer_array : enable

#include "./lighting/Light.glsl"

//Storage buffer containing the point light data
layout (std430, set = GLOBAL_SET, binding = SPOT_LIGHT_BINDING) restrict readonly buffer SSpotLight
{
	SpotLight s_SpotLight[];
};
//Input attributes
layout(location = 0)  in vec3  a_Position;
layout(location = 1)  in vec3  a_Normal;
layout(location = 2)  in vec3  a_Tangent;
layout(location = 3)  in vec3  a_Bitangent;
layout(location = 4)  in vec2  a_UV_Coordinates;
#ifdef VS_SHADER_ANIMATED
layout(location = 5)  in ivec4 a_BoneId;
layout(location = 6)  in vec4  a_BoneWeight;
// per instance
layout(location = 7)  in mat4  a_Transform;
#else
// per instance
layout(location = 5)  in mat4  a_Transform;
#endif

#ifdef VS_SHADER_ANIMATED
struct BoneData 
{
	mat4 s_Transform;
	int s_ParentId;
};
//Storage buffer containing the bones data
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform
{
	BoneData s_BoneTransform[];
}; 
#endif // VS_SHADER_ANIMATED

layout(push_constant) uniform PSpotLightIndex
{
	int Index;
} p_SpotLightIndex;

// Main entry point
void main()
{
#ifdef VS_SHADER_ANIMATED
	uint BoneInstanceOffset = gl_InstanceIndex * MAX_BONES_PER_INSTANCE;
    mat4 BoneTransform = mat4(0.0);

    for (uint i = 0; i < BONE_INFLUENCE; i++) 
	{
		BoneTransform += (a_BoneId[i] != ~0) ? s_BoneTransform[BoneInstanceOffset + a_BoneId[i]].s_Transform * a_BoneWeight[i]: mat4(0.0);
	}

    vec4 VertexWorldSpace   = a_Transform * BoneTransform * vec4(a_Position, 1.0);
	gl_Position             = s_SpotLight[p_SpotLightIndex.Index].Cascade.ViewProjectionMatrix * VertexWorldSpace; 
#else
	gl_Position 			= s_SpotLight[p_SpotLightIndex.Index].Cascade.ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
#endif // VS_SHADER_ANIMATED
    gl_Layer    			= p_SpotLightIndex.Index;
}
// Fragment Shader
#version 460 core
#pragma: fragment

layout(location = 0) out vec4 FragmentColor;
// Main entry point
void main()
{
}
