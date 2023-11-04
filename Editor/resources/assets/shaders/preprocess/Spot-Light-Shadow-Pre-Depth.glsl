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
layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec3 a_Tangent;
// layout(location = 3) in vec3 a_Bitangent;
// layout(location = 4) in vec2 a_UV_Coordinates;
// Per instance
layout(location = 5) in mat4 a_Transform;

layout(push_constant) uniform PSpotLightIndex
{
	int Index;
} p_SpotLightIndex;

// Main entry point
void main()
{
    /* Set position without veiw matrix */
	gl_Position = s_SpotLight[p_SpotLightIndex.Index].Cascade.ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
    gl_Layer    = p_SpotLightIndex.Index;
}
// Fragment Shader
#version 460 core
#pragma: fragment

layout(location = 0) out vec4 FragmentColor;
// Main entry point
void main()
{
}
