//Vertex Shader
#version 460 core
#pragma: vertex
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
//Input attributes
// Подключаем буфер камеры для работы с матрицей проекции
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera 
{
    mat4 ViewProjectionMatrix;
};
// Входная переменная — ID экземпляра кости
//layout(location = 0) flat out int out_InstanceId;

// Storage Buffer с матрицами трансформаций костей
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform {
    mat4 s_BoneTransform[];
};

// Выходная позиция для геометрического шейдера
layout(location = 0) out vec4 v_BonePosition;

void main() {
    // Получаем матрицу трансформации кости
    mat4 boneTransform = s_BoneTransform[gl_InstanceIndex];
    
    // Позиция в мировом пространстве (возможно, начало или конец кости)
    vec4 boneWorldPos = boneTransform * vec4(0.0, 0.0, 0.0, 1.0);  // Корень кости или ее позиция
    
    // Трансформируем в пространство камеры
    v_BonePosition = ViewProjectionMatrix * boneWorldPos;

    // Позиция вершины (считаем, что это точка для отрисовки)
    gl_Position = v_BonePosition;
}

#version 460 core
#pragma: geometry
#include "./include/Common.glsl"
#include "./include/Camera.glsl"

layout(points) in;
layout(line_strip, max_vertices = 2) out;

// Входные данные от вершинного шейдера
layout(location = 0) in vec4 v_BonePosition[];

// Камера для отображения в пространстве
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera {
    mat4 ViewProjectionMatrix;
};

// Storage Buffer с матрицами трансформаций костей
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform {
    mat4 s_BoneTransform[];
};

void main() {
    // Получаем позицию начала кости
    vec4 boneStart = v_BonePosition[0];
    
    // Получаем позицию конца кости (можно использовать позицию, смещённую по оси)
    mat4 boneTransform = s_BoneTransform[gl_PrimitiveID];  // Данные текущей кости
    vec4 boneEnd = boneTransform * vec4(0.0, 1.0, 0.0, 1.0); // Конец кости на оси Y

    // Трансформируем конец кости в пространство камеры
    boneEnd = ViewProjectionMatrix * boneEnd;

    // Генерируем линию
    gl_Position = boneStart;
    EmitVertex();

    gl_Position = boneEnd;
    EmitVertex();
    
    EndPrimitive();
}
//Vertex Shader
#version 460 core
#pragma: fragment
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"

//layout(location = 0) flat in int a_InstanceId;
// Получаем цвет из геометрического шейдера
// Выходной цвет
layout(location = 0) out vec4 out_Color;

void main()
{
   out_Color = vec4(1.0, 0.0, 0.0, 1.0);  // Красная линия для костей
}
