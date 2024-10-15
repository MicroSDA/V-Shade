//Vertex Shader
#version 450 core
#pragma: vertex
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
// Input attributes
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_UV_Coordinates;
// Per instance
layout(location = 5) in mat4 a_Transform;

layout(location = 0) flat out int out_InstanceId;
layout(location = 1) out vec3 out_Position;
layout(location = 2) out vec3 out_Normal;

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Vertex shader entry point
void main() 
{
   gl_Position 		= u_Camera.ViewProjectionMatrix  * a_Transform * vec4(a_Position, 1.0);
   
   out_Normal 		= mat3(transpose(inverse(a_Transform))) * a_Normal;
   out_InstanceId 	= gl_InstanceIndex;
   out_Position 	= a_Position;
}
//Fragment Shader
#version 460 core
#pragma: fragment

#include "./include/Common.glsl"
#include "./include/Material.glsl"
//Output variables
layout(location = 0) flat in int a_InstanceId;
layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_Normal; 

layout(location = 0) out vec4 FragmentColor;
//Fragment shader entry point

//Storage buffer containing the material data
layout (std430, set = PER_INSTANCE_SET, binding = MATERIAL_BINDING) restrict readonly buffer SMaterial
{
	Material u_Material[];
};

void main()
{
	FragmentColor = vec4(u_Material[a_InstanceId].AmbientColor, 0.8);
	gl_FragDepth = gl_FragCoord.z - 0.1;
	
	
	// Нормализуем нормаль для освещения
    // vec3 normal = normalize(a_Normal);
    
    // // Рассчитываем освещение с использованием Lambertian (diffuse) модели
    // float diffuseFactor = max(dot(normal, -vec3(1.0)), 0.0);
    
    // // Основной цвет материала (Ambient)
    // vec3 ambientColor = u_Material[a_InstanceId].AmbientColor;

    // // Итоговый цвет с учетом освещения
    // vec3 lightingColor = ambientColor * diffuseFactor * vec3(1.0);

    // // Задаем цвет фрагмента
    // FragmentColor = vec4(lightingColor, 1.0);

    // Глубина фрагмента
    gl_FragDepth = gl_FragCoord.z - 0.1;
}