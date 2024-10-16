//Vertex Shader
#version 460 core
#pragma: vertex
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
#include "./include/Vertex.glsl"

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Storage buffer containing the bones data
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform
{
	mat4 s_BoneTransform[];
};

layout(location = 0) flat out int out_InstanceId;

void main() 
{
  out_InstanceId   = gl_InstanceIndex;
}

#version 460 core
#pragma: geometry
#include "./include/Common.glsl"
#include "./include/Camera.glsl"

layout(points) in; 
layout(triangle_strip, max_vertices = 100) out;

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};

struct BoneData 
{
	mat4 s_Transform;
	int s_ParentId;
};
	
layout (std430, set = PER_INSTANCE_SET, binding = BONE_TRANSFORMS_BINDING) restrict readonly buffer SBoneTransform
{
	BoneData s_BoneTransform[];
}; 

layout(location = 0) flat in int a_InstanceId[];
layout(location = 0) out vec4 out_Color;

vec4 ColorPalette[5] = {
    vec4(0.9, 0.3, 0.3, 0.8),  
    vec4(0.2, 0.8, 0.4, 0.8),  
    vec4(0.4, 0.6, 0.9, 0.8), 
    vec4(1.0, 0.8, 0.2, 0.8), 
    vec4(0.7, 0.2, 0.8, 0.8)  
};	
const float scale = 0.09;

vec4 vertices[4] = {
    vec4(-scale, scale, -scale, 1),
    vec4(scale,  scale, -scale, 1), 
    vec4(scale,  scale,  scale, 1),  
    vec4(-scale, scale,  scale, 1) 
};

vec3 ExtractScale(mat4 matrix) {
    // Извлекаем масштабы по X, Y, Z
    float scaleX = length(matrix[0].xyz);  // Длина первого столбца
    float scaleY = length(matrix[1].xyz);  // Длина второго столбца
    float scaleZ = length(matrix[2].xyz);  // Длина третьего столбца

    return vec3(scaleX, scaleY, scaleZ);
}

void main() 
{
	int id 	= a_InstanceId[0] + 1;
	
	if(s_BoneTransform[id].s_ParentId != ~0)
	{
		float dst = distance(s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(0, 0, 0, 1),
		s_BoneTransform[id].s_Transform * vec4(0, 0, 0, 1));
		vec3 Scale = ExtractScale(s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform);
		Scale = vec3(dst / Scale.x, dst / Scale.y, dst /Scale.z);
		
		//////////////////Cone Base/////////////////
		
		for(int i = 0; i < 4; i++)
		{
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(0, 0, 0, 1.0);
			out_Color = ColorPalette[4];
			EmitVertex();
		
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * (vertices[i % 4] * vec4(Scale, 1.0));
			out_Color = ColorPalette[2];
			EmitVertex();
		
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * (vertices[(i + 1) % 4] * vec4(Scale, 1.0));
			out_Color = ColorPalette[1];
			EmitVertex();
		} 
		
		EndPrimitive();
		
		for(int i = 0; i < 4; i++)
		{
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[id].s_Transform * vec4(0, 0, 0, 1.0);
			out_Color = ColorPalette[2];
			EmitVertex();
		
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * (vertices[i % 4] * vec4(Scale, 1.0));
			out_Color = ColorPalette[1];
			EmitVertex();
		
			gl_Position = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * (vertices[(i + 1) % 4] * vec4(Scale, 1.0));
			out_Color = ColorPalette[2];
			EmitVertex();
		} 
		
		EndPrimitive();
		//!/////////////Normals//////////////////!//
		
		// vec4 pointA = u_Camera.ViewProjectionMatrix * s_BoneTransform[id].s_Transform * vec4(0, 0, 0, 1);
		// vec4 pointB = u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(0, 0, 0, 1);
		// //vec4 midPoint = (pointA + pointB) * 2.0 / dst;
		// vec4 midPoint = pointB * 3.0 / dst;
		
		// // X
		// gl_Position = midPoint;
		// out_Color = vec4(1, 0, 0, 1);
		// EmitVertex();
		
		// gl_Position = midPoint + u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(1, 0, 0, 0);
		// out_Color = vec4(1, 0, 0, 1);
		// EmitVertex();
		// EndPrimitive(); 
		
		// // Y
		// gl_Position = midPoint;
		// out_Color = vec4(0, 1, 0, 1);
		// EmitVertex();
		
		// gl_Position = midPoint + u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(0, -1, 0, 1);
		// out_Color = vec4(0, 1, 0, 1);
		// EmitVertex();
		// EndPrimitive();
		
		// // Z
		// gl_Position = midPoint;
		// out_Color = vec4(0, 0, 1, 1);
		// EmitVertex();
		
		// gl_Position = midPoint + u_Camera.ViewProjectionMatrix * s_BoneTransform[s_BoneTransform[id].s_ParentId].s_Transform * vec4(0, 0, 1, 1);
		// out_Color = vec4(0, 0, 1, 1);
		// EmitVertex();
		// EndPrimitive();
				
		//!/////////////////////////////////////!//
	}
	
    gl_PointSize = 2.0;
    
}

//Vertex Shader
#version 460 core
#pragma: fragment
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"

layout(location = 0) in vec4 a_Color;
layout(location = 0) out vec4 out_Color;

void main()
{
   out_Color = a_Color;
   gl_FragDepth = gl_FragCoord.z - 0.5;
}