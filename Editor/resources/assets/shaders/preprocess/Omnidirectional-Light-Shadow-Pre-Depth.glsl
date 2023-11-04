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
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_UV_Coordinates;
layout(location = 5) in mat4 a_Transform;

// layout (location = 0)  out vec3  out_FragmentPosition;
// layout (location = 1)  out vec3  out_LightPosition;
// layout (location = 2)  out float out_Distance;

// Main entry point
void main()
{
      /* Set position without veiw matrix */
	gl_Position = a_Transform * vec4(a_Position, 1.0);
}
// Fragment Shaders
#version 460 core
#pragma: geometry

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out; 

#include "./lighting/Light.glsl"

//Storage buffer containing the point light data
layout (std430, set = GLOBAL_SET, binding = POINT_LIGHT_BINDING) restrict readonly buffer SPointLight
{
	PointLight s_PointLight[];
};
//Uniform buffer containing the scene data
layout (std140, set = GLOBAL_SET, binding = SCENE_DATA_BINDING) uniform USceneData
{
    SceneData u_SceneData;
};

// float GetClipDistance(vec3 LightPosition, uint VertexIndex, uint PlaneIndex)
// {
//     return 0.0; 
// }

// void main()
// {
//     for(int i = 0; i < u_SceneData.PointLightCount; i++)
//     {
//         for(int j = 0; j < 4; j++)
//         {
//             gl_Layer = j;
//             out_Distance        = s_PointLight[i].Distance;
//             out_LightPosition   = s_PointLight[i].Position.xyz;
            
//             for(int vertexIndex = 0; vertexIndex < 3; vertexIndex++)
//             {
//                 gl_Position  = s_PointLight[i].Cascade[j].ViewProjectionMatrix * gl_in[vertexIndex].gl_Position;
//                 out_FragmentPosition = gl_in[vertexIndex].gl_Position.xyz;
//                 gl_ClipDistance[0] = 0.0;
//                 gl_ClipDistance[1] = -0.57735026;
//                 gl_ClipDistance[2] = 0.81649661;
//                 EmitVertex(); 
//             }
//             EndPrimitive();
//         }
//     }
// }

const vec3 planeNormals[12] =
{
  vec3(0.00000000, -0.03477280, 0.99939519),
  vec3(-0.47510946, -0.70667917, 0.52428567),
  vec3(0.47510946, -0.70667917, 0.52428567),
  vec3(0.00000000, -0.03477280, -0.99939519),
  vec3(0.47510946, -0.70667917, -0.52428567),
  vec3(-0.47510946, -0.70667917, -0.52428567),
  vec3(-0.52428567, 0.70667917, -0.47510946),
  vec3(-0.52428567, 0.70667917, 0.47510946),
  vec3(-0.99939519, 0.03477280, 0.00000000),
  vec3(0.52428567, 0.70667917, -0.47510946),
  vec3(0.99939519, 0.03477280, 0.00000000),
  vec3(0.52428567, 0.70667917, 0.47510946)
};

float GetClipDistance(in vec3 lightPosition, in uint vertexIndex, in uint planeIndex)
{
  vec3 normal = planeNormals[planeIndex];
  return (dot(gl_in[vertexIndex].gl_Position.xyz, normal)+dot(-normal, lightPosition));
}

layout (location = 0)  out vec3  out_FragmentPosition;
layout (location = 1)  out vec3  out_LightPosition;
layout (location = 2)  out float out_Distance;

void main()
{
  const uint lightIndex = 0;
  const vec3 lightPosition = s_PointLight[lightIndex].Position; // s_PointLight[i].Cascade[j].ViewProjectionMatrix
   
  // back-face culling
  vec3 normal = cross(gl_in[2].gl_Position.xyz-gl_in[0].gl_Position.xyz, gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);
  vec3 view = lightPosition - gl_in[0].gl_Position.xyz;

  if(dot(normal, view) < 0.0f)
    return;

  for(int faceIndex = 0; faceIndex < 4; faceIndex++)
  {
    gl_Layer = faceIndex;
    out_Distance        = s_PointLight[lightIndex].Distance;
    out_LightPosition   = s_PointLight[lightIndex].Position;
    
    uint inside = 0;
    float clipDistances[9];
    for(uint sideIndex=0; sideIndex<3; sideIndex++)
    {
      const uint planeIndex = (faceIndex*3)+sideIndex;
      const uint bit = 1 << sideIndex;
      for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
      {
        uint clipDistanceIndex = sideIndex*3+vertexIndex;
        clipDistances[clipDistanceIndex] = GetClipDistance(lightPosition, vertexIndex, planeIndex);
        inside |= (clipDistances[clipDistanceIndex] > 0.001) ? bit : 0;
      }
    }

    if(inside == 0x7)
    {
        for(uint vertexIndex=0; vertexIndex<3; vertexIndex++)
        {
          gl_Position = s_PointLight[lightIndex].Cascade[faceIndex].ViewProjectionMatrix * gl_in[vertexIndex].gl_Position;
          out_FragmentPosition = gl_in[vertexIndex].gl_Position.xyz;
          gl_ClipDistance[0] = clipDistances[vertexIndex];
          gl_ClipDistance[1] = clipDistances[3+vertexIndex];
          gl_ClipDistance[2] = clipDistances[6+vertexIndex];
          EmitVertex();  
        }
        EndPrimitive(); 
    }
  }
}
// Fragment Shaders
#version 460 core
#pragma: fragment

layout (location = 0)  in vec3  in_FragmentPosition;
layout (location = 1)  in vec3  in_LightPosition;
layout (location = 2)  in float in_Distance;

layout (location = 0)  out vec4 FragmentColor;
// Main entry point
void main()
{
    float LightDistance = length(in_FragmentPosition - in_LightPosition);
    gl_FragDepth = LightDistance / in_Distance;
}
