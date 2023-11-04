//Vertex Shader
#version 460 core
#pragma: vertex

//Include Camera header
#include "include/Common.glsl"
#include "include/Camera.glsl"

//Input attributes
layout(location = 0) in vec3 a_Position;
layout(location = 5) in mat4 a_Transform;

layout(location = 0) flat out int out_InstanceId;

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Vertex shader entry point
void main() 
{
    out_InstanceId = gl_InstanceIndex;
    gl_Position    = u_Camera.ViewProjectionMatrix * a_Transform * vec4(a_Position, 1.0);
}
//Fragment Shader
#version 460 core
#pragma: fragment

#include "include/Common.glsl"
#include "include/Material.glsl"

layout(location = 0) flat in int a_InstanceId;
//Storage buffer containing the material data
layout (std430, set = PER_INSTANCE_SET, binding = MATERIAL_BINDING) restrict readonly buffer SMaterial
{
	Material u_Material[];
};
//Output variables
layout(location = 0) out vec4 FragmentColor;
//Fragment shader entry point
void main()
{
    //gl_FragDepth = 0;
	FragmentColor = vec4(u_Material[a_InstanceId].DiffuseColor, 1.0);
}