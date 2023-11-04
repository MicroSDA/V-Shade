//Vertex Shader
#version 450 core
#pragma: vertex
#extension GL_ARB_shader_viewport_layer_array : enable

#include "./include/Common.glsl"
#include "./lighting/Light.glsl"
//Input attributes
layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec3 a_Tangent;
// layout(location = 3) in vec3 a_Bitangent;
// layout(location = 4) in vec2 a_UV_Coordinates;
// Per instance
layout(location = 5) in mat4 a_Transform;

//Storage buffer containing the direct light data
layout (std430, set = GLOBAL_SET, binding = GLOBAL_LIGHT_BINDING) restrict readonly buffer SGlobalLight
{
	GlobalLight s_GlobalLight[];
};

layout(push_constant) uniform PGlobalLightCascade
{
	int Index;
} p_GlobalLightCascade;

// Main entry point
void main()
{
	gl_Position = s_GlobalLight[0].Cascades[p_GlobalLightCascade.Index].ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
    gl_Layer    = p_GlobalLightCascade.Index;
}
// Fragment Shader
#version 460 core
#pragma: fragment

layout(location = 0) out vec2 FragmentColor;
// Main entry point
void main()
{
	
}