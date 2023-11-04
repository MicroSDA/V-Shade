//Vertex Shader
#version 460 core
#pragma: vertex
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
//Input attributes
layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec3 a_Tangent;
// layout(location = 3) in vec3 a_Bitangent;
// layout(location = 4) in vec2 a_UV_Coordinates;
// Per instance
layout(location = 5) in mat4 a_Transform;
//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};

precise invariant gl_Position; //??

void main()
{
    gl_Position = u_Camera.ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
}
//Fragment Shader
#version 460 core
#pragma: fragment
//Fragment shader entry point
void main()
{
    // Do notning.
}