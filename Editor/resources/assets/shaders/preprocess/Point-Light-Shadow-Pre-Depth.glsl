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
layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec3 a_Tangent;
// layout(location = 3) in vec3 a_Bitangent;
// layout(location = 4) in vec2 a_UV_Coordinates;
// Per instance
layout(location = 5) in mat4 a_Transform;

layout (location = 0)  out vec3  out_FragmentPosition;
layout (location = 1)  out vec3  out_LightPosition;
layout (location = 2)  out float out_Distance;

layout(push_constant) uniform PLightIndex
{
	int Index;
    int Side;
} p_LightIndex;
// Main entry point
void main()
{
	gl_Position             = s_PointLight[p_LightIndex.Index].Cascade[p_LightIndex.Side].ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
    gl_Layer                = (p_LightIndex.Index * 6) + (p_LightIndex.Side); // ?
    out_FragmentPosition    = vec4(a_Transform * vec4(a_Position, 1.0)).xyz;
    out_LightPosition       = s_PointLight[p_LightIndex.Index].Position;
    out_Distance            = s_PointLight[p_LightIndex.Index].Distance;
}
// Fragment Shaders
#version 460 core
#pragma: fragment

layout (location = 0)  in vec3  in_FragmentPosition;
layout (location = 1)  in vec3  in_LightPosition;
layout (location = 2)  in float in_Distance;

layout (location = 0) out vec4 FragmentColor;
// Main entry point
void main()
{
    float LightDistance = length(in_FragmentPosition - in_LightPosition);
    gl_FragDepth = LightDistance / in_Distance;
}
