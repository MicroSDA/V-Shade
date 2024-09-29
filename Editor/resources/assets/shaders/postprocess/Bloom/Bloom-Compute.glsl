#version 460 core
#pragma: compute

#include "././shaders/include/Common.glsl"
#include "Filters.glsl"
// The folowing extension allow to pass images as function parameters
layout(local_size_x = 16, local_size_y = 16) in;

layout (         set = PER_INSTANCE_SET, binding = 0) uniform sampler2D  	u_Sampler;
layout (rgba32f, set = PER_INSTANCE_SET, binding = 1) uniform image2D       u_AboveImage;
layout (rgba32f, set = PER_INSTANCE_SET, binding = 2) uniform image2D       u_BelowImage;

#define STAGE_HDR          0
#define STAGE_DOWN_SAMPLE  1
#define STAGE_UP_SAMPLE    2
#define STAGE_COMBINE      3

layout (push_constant) uniform UBloom 
{
   vec3   Curve;
   float  Exposure;
   float  Threshold; 
   float  Knee;
   int 	  Mip;
   int    Stage;
} u_Bloom;

ivec2 FitToBorders(ivec2 Position, ivec2 ImageSize)
{
    ivec2 PixelCoords = Position;

    if(PixelCoords.x < 0) PixelCoords.x = 0;
    if(PixelCoords.y < 0) PixelCoords.y = 0;
    if(PixelCoords.x >= ImageSize.x) PixelCoords.x = ImageSize.x;
    if(PixelCoords.y >= ImageSize.y) PixelCoords.y = ImageSize.y;

    return PixelCoords;
}
void HDR(ivec2 ImagePosition)
{
   vec2  ImageSize      = vec2(imageSize(u_AboveImage));
   vec2  TextureCoords  = vec2(ImagePosition) / ImageSize;
   float ClampValue = 1.0; // Don't know why.

   vec4  Color = imageLoad(u_AboveImage, ImagePosition);
   Color.rgb = min(vec3(ClampValue), Color.rgb);
   Color = QThreshold(Color, u_Bloom.Curve, u_Bloom.Threshold, u_Bloom.Exposure);
   imageStore(u_BelowImage, ImagePosition, Color);
}

void Downsample(int Mip, ivec2 ImagePosition)
{
   //ImagePosition  = ivec2(clamp(vec2(ImagePosition), vec2(0.0,0.0), vec2(imageSize(u_AboveImage))));
   //ImagePosition = FitToBorders(ImagePosition, ivec2(imageSize(u_AboveImage)));
   vec2  ImageSize      = vec2(imageSize(u_AboveImage));
   vec2  TextureCoords  = vec2(ImagePosition) / ImageSize; 
   vec2  TexelSize      = 1.0 / vec2(textureSize(u_Sampler, 0));

   TextureCoords        += (1.0 / ImageSize) * 0.5;
   //TextureCoords 		= clamp(TextureCoords, -0.5, 0.5);
   
   vec4 Color = DownsampleBox13(u_Sampler, TextureCoords, TexelSize, 0);
   
   imageStore(u_BelowImage, ImagePosition, Color);
}

void Upsample(int Mip, ivec2 ImagePosition)
{
	//ImagePosition  = ivec2(clamp(vec2(ImagePosition), vec2(0.0,0.0), vec2(imageSize(u_AboveImage))));
	//ImagePosition = FitToBorders(ImagePosition, ivec2(imageSize(u_AboveImage)));
    vec2  ImageSize      = vec2(imageSize(u_AboveImage));

    vec2  TextureCoords  = vec2(ImagePosition) / ImageSize;
    vec2  TexelSize      = 1.0 / vec2(textureSize(u_Sampler, 0));

    TextureCoords        += (1.0 / ImageSize) * 0.5;
	
	//TextureCoords 		= clamp(TextureCoords, 0.0, 1.0);
    
    vec4  Above          = imageLoad(u_AboveImage, ImagePosition);
    vec4  Current        = UpsampleTent(u_Sampler, TextureCoords, TexelSize, 0);

    imageStore(u_BelowImage, ImagePosition, Above + Current);
}

void Combine(int Mip, ivec2 ImagePosition)
{
   vec2  ImageSize      = vec2(imageSize(u_AboveImage));
   vec4  Bloom            = imageLoad(u_AboveImage, ImagePosition);
   vec4  Original         = imageLoad(u_BelowImage, ImagePosition);
   
   imageStore(u_BelowImage, ImagePosition, vec4(Original.rgb + Bloom.rgb, Original.a)); 
}
void main()
{
    ivec2 ImagePosition    = ivec2(gl_GlobalInvocationID.xy);

    if(u_Bloom.Stage == STAGE_HDR)
    {
        HDR(ImagePosition);
    }
    if(u_Bloom.Stage == STAGE_DOWN_SAMPLE)
    {   
        Downsample(u_Bloom.Mip, ImagePosition); 
    }
    if(u_Bloom.Stage == STAGE_UP_SAMPLE)
    {
        Upsample(u_Bloom.Mip, ImagePosition); 
    }
    if(u_Bloom.Stage == STAGE_COMBINE)
    {
        Combine(u_Bloom.Mip, ImagePosition); 
    }
}