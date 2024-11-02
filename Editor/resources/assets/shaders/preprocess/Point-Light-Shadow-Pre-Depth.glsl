#version 450 core
#pragma: vertex
#extension GL_ARB_shader_viewport_layer_array : enable

#include "./lighting/Light.glsl"

//Storage buffer containing the point light data
layout (std430, set = GLOBAL_SET, binding = POINT_LIGHT_BINDING) restrict readonly buffer SPointLight
{
	PointLight s_PointLight[];
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

layout (location = 0)  out vec4  out_FragmentPosition;
layout (location = 1)  out vec3  out_LightPosition;
layout (location = 2)  out float out_Distance;

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

layout(push_constant) uniform PLightIndex
{
	int Index;
    int Side;
} p_LightIndex;
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

	gl_Position             = s_PointLight[p_LightIndex.Index].Cascade[p_LightIndex.Side].ViewProjectionMatrix * VertexWorldSpace; 
    out_FragmentPosition    = VertexWorldSpace; // Do not forget about issue with .w component !!
#else
	gl_Position             = s_PointLight[p_LightIndex.Index].Cascade[p_LightIndex.Side].ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
    out_FragmentPosition    = vec4(a_Transform * vec4(a_Position, 1.0));
#endif // VS_SHADER_ANIMATED
 	gl_Layer                = (p_LightIndex.Index * 6) + (p_LightIndex.Side);
    out_LightPosition       = s_PointLight[p_LightIndex.Index].Position;
    out_Distance            = s_PointLight[p_LightIndex.Index].Distance;
}
// Fragment Shaders
#version 460 core
#pragma: fragment

layout (location = 0)  in vec4  in_FragmentPosition;
layout (location = 1)  in vec3  in_LightPosition;
layout (location = 2)  in float in_Distance;

layout (location = 0) out vec4 FragmentColor;
// Main entry point
void main()
{
    float LightDistance = length(in_FragmentPosition - vec4(in_LightPosition, 0));
    gl_FragDepth = LightDistance / in_Distance;
}
