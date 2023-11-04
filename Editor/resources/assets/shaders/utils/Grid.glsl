//Vertex Shader
#version 460 core
#pragma: vertex
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"
//Specify the input layout
layout(location = 0) in vec3 a_Position;
// layout(location = 1) in vec3 a_Normal;
// layout(location = 2) in vec3 a_Tangent;
// layout(location = 3) in vec3 a_Bitangent;
// layout(location = 4) in vec2 a_UV_Coordinates;
//Specify the output layout
layout(location = 0) out vec3 out_Near;
layout(location = 1) out vec3 out_Far;

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Function that returns the unprojected point given specific coordinates and view projection matrix
vec3 UnprojectPoint(float x, float y, float z, mat4 viewProjection) 
{
    vec4    unprojectedPoint    =  inverse(viewProjection) * vec4(x, y, z, 1.0);
    return  unprojectedPoint.xyz / unprojectedPoint.w;
}
// Shader main entry point
void main()
{
	//Calculate the near and far unprojected points for the current vertex
	out_Near 		= UnprojectPoint(a_Position.x, a_Position.y, 0.0, u_Camera.ViewProjectionMatrix).xyz; 
	out_Far  		= UnprojectPoint(a_Position.x, a_Position.y, 1.0, u_Camera.ViewProjectionMatrix).xyz; 
	//Set the position of the vertex
	gl_Position 	= vec4(a_Position, 1.0);
}

#version 460 core
#pragma: fragment
//Include Camera header
#include "./include/Common.glsl"
#include "./include/Camera.glsl"

#define DepthRangeNear    -1.f
#define DepthRangeFar      1.f
//Specify the input layouts
layout (location = 0)   in vec3 a_Near;
layout (location = 1)   in vec3 a_Far; 

//Uniform buffer containing the camera data
layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};
//Function that creates a grid given specific parameters
vec4 CreateGrid(vec3 gridColor, vec3 fragPos3D, float scale, bool drawAxis) 
{
	//Calculate the coordinates of the current fragment in the 2D plane
    vec2  Coord = fragPos3D.xz * scale;
    vec2  Derivative = fwidth(Coord);
    vec2  Grid = abs(fract(Coord - 0.5) - 0.5) / Derivative;
    //Calculate the line value based on the coordinate values
    float Line = min(Grid.x, Grid.y);
    float MinZ = min(Derivative.y, 0.1);
    float MinX = min(Derivative.x, 0.1);
    //Set the color value based on the line value and the drawAxis parameter
    vec4  Color = vec4(gridColor.rgb, 1.0 - min(Line, 1));
    // Z axis
    if(fragPos3D.x > - 3 * MinX && fragPos3D.x < 3 * MinX)
       Color.rgb = vec3(0.0, 0.0, 2.0);
    // X axis
    if(fragPos3D.z > - 3 * MinZ && fragPos3D.z < 3 * MinZ)
        Color.rgb = vec3(2.0, 0.0, 0.0);

    return Color;
}

// This function calculates the depth of a given 3D position in the scene, based on the current camera and projection settings.
float ComputeDepth(vec3 pos) 
{
    // Transform the position into clip space using the current camera view and projection matrices.
    vec4  ClipSpacePosition = u_Camera.ViewProjectionMatrix * vec4(pos.xyz, 1.0);
    
    // Calculate the depth value in clip space by dividing the z component by the w component of the position vector.
    float ClipSpaceDepth    =  (ClipSpacePosition.z / ClipSpacePosition.w);
    
    // Retrieve the far and near clip distances from the context.
    float Far               = DepthRangeFar;
    float Near              = DepthRangeNear;
    
    // Calculate the final depth value by transforming the ClipSpaceDepth back into world space, and averaging the near and far clip distances.
    return (((Far - Near) * ClipSpaceDepth) + Near + Far) / 2.0;
}

// ComputeLinearDepth calculates the linearized depth of a given position in view space
float ComputeLinearDepth(vec3 position) 
{
    // Multiply the position by the view and projection matrices to obtain the clip space position
    vec4 ClipSpacePosition = u_Camera.ViewProjectionMatrix * vec4(position.xyz, 1.0);

    // Calculate the clip space depth
    float ClipSpaceDepth = (ClipSpacePosition.z / ClipSpacePosition.w) * 2.0 - 1.0; 

    // Calculate linear depth using the near and far clip planes of the current view frustum
    float LinearDepth = (2.0 * DepthRangeNear * DepthRangeFar) / 
                        (DepthRangeFar + DepthRangeNear - ClipSpaceDepth * (DepthRangeFar - DepthRangeNear));

    // Scale the result down for improved precision and return the linear depth value
    return LinearDepth / 100.0; 
}
// Set output color location
layout(location = 0) out vec4 FragmentColor;
// Shader main entry point
void main()
{
	// Calculate T based on near and far plane coordinates
	float T             =-a_Near.y / (a_Far.y - a_Near.y);
	// Calculate fragment position in 3D space
    vec3  FragPos3D     = a_Near + T * (a_Far - a_Near);
    // Compute linear depth based on fragment position
    float LinearDepth   = ComputeLinearDepth(FragPos3D);
    // Calculate fading based on linear depth
    float Fading        = max(0.0, (0.4 - LinearDepth));
	// Compute depth based on fragment position
    gl_FragDepth        = ComputeDepth(FragPos3D);
	// Set grid color
    vec3 GridColor      =  vec3(0.2, 0.2, 0.2);
    // Create grid and calculate its color
    vec4 Color          = (CreateGrid(GridColor, FragPos3D, 0.1, true) + CreateGrid(GridColor, FragPos3D, 1.0, true)) * float(T > 0);
	// Multiply color alpha with fading
    Color.a             *= Fading;
	// Discard fragments with low alpha
	if(Color.a <= 0.2)
        discard;
    // Set output color to the calculated color
    FragmentColor = Color;
}