#version 460 core
#pragma: compute

#include "././shaders/include/Common.glsl"
#include "././shaders/include/Camera.glsl"
#include "././shaders/include/Vertex.glsl"

layout(local_size_x = 16, local_size_y = 16) in;

layout (rgba32f,   set = PER_INSTANCE_SET, binding = 0) uniform image2D       u_PositionImage;
layout (rgba32f,   set = PER_INSTANCE_SET, binding = 1) uniform image2D       u_NormalImage;
layout (r8,        set = PER_INSTANCE_SET, binding = 2) uniform image2D       u_SSAOImage;

layout (std140, set = GLOBAL_SET, binding = CAMERA_BUFFER_BINDING) uniform UCamera
{
    Camera u_Camera;
};

#define MAX_SAMPLES        64

#define STAGE_CREATE       0
#define STAGE_BLUR_H       1
#define STAGE_BLUR_V       2
#define STAGE_ZOOM 		   3
#define STAGE_COMBINE      4

layout (std140, set = PER_INSTANCE_SET, binding = 4) uniform USamples
{
   vec3   Samples[MAX_SAMPLES];
   vec3   Noise[32];
  
} u_Samples;

layout (push_constant) uniform USettings 
{
   int    SamplesCount;
   float  Radius;
   float  Bias;
   int    Stage; 
   int    BlurSamples;
} u_Settings;

ivec2 FitToBorders(ivec2 Position, ivec2 ImageSize)
{
    ivec2 PixelCoords = Position;

    if(PixelCoords.x < 0) PixelCoords.x = 0;
    if(PixelCoords.y < 0) PixelCoords.y = 0;
    if(PixelCoords.x >= ImageSize.x) PixelCoords.x = ImageSize.x - 1;
    if(PixelCoords.y >= ImageSize.y) PixelCoords.y = ImageSize.y - 1;

    return PixelCoords;
}

void Create()
{
    ivec2 ImagePosition  = ivec2(gl_GlobalInvocationID.xy);
    ivec2 ImageSize      = imageSize(u_PositionImage);
    
    vec3 Position = imageLoad(u_PositionImage, ImagePosition).rgb;
    vec3 Normal   = imageLoad(u_NormalImage,   ImagePosition).rgb;

    uint NoiseIndex = (ImagePosition.x * ImagePosition.y + 1) % gl_LocalInvocationIndex; 
    vec3 RandomVector = u_Samples.Noise[(NoiseIndex) % 16] * vec3(1.0 / vec2(ImageSize), 0); // TODO

    vec3 Tangent     = normalize(RandomVector - Normal * dot(RandomVector, Normal));
    vec3 Bitangent   = cross(Tangent, Normal);
    mat3 TBN         = mat3(Tangent, Bitangent, Normal);

    float Occlusion = 0.0;
    uint  Count     = 0;
    for(uint i = 0; i < u_Settings.SamplesCount; i++)
    {
        vec3 SamplePosition = TBN * u_Samples.Samples[i].xyz;
        SamplePosition = Position + SamplePosition * u_Settings.Radius;

        vec4 Offset = vec4(SamplePosition, 1.0);
        Offset = u_Camera.ProjectionMatrix * Offset;
        Offset.xyz /= Offset.w; 
		Offset.xyz = Offset.xyz * 0.5 + 0.5; 

        ivec2 OffsetCoords = FitToBorders(ivec2(Offset.xy * ImageSize), ImageSize);
        vec2 Depth = imageLoad(u_PositionImage, OffsetCoords).zw;

        float Range = smoothstep(0.0, 1.0, u_Settings.Radius / abs(Position.z - Depth.x));
        Occlusion += (Depth.x >= SamplePosition.z + u_Settings.Bias ? 1.0 : 0.0) * Range;
    }

    Occlusion = 1.0 - (Occlusion / float(u_Settings.SamplesCount));
    imageStore(u_SSAOImage, ImagePosition, vec4(Occlusion)); 
}

void Blur(uint blur)
{
    ivec2 ImageSize     = imageSize(u_PositionImage);
	ivec2 ImagePosition = ivec2(gl_GlobalInvocationID.xy);

	if (ImagePosition.x <= ImageSize.x && ImagePosition.y <= ImageSize.y)
	{
		vec4 Color = vec4(0.0);

        if(blur == STAGE_BLUR_H)
        {
            for (int x = 0; x < u_Settings.BlurSamples; x++)
            {
                ivec2 PixelCoords = FitToBorders(ImagePosition + ivec2(x, 0), ImageSize);
                Color += imageLoad(u_NormalImage,  PixelCoords);
                PixelCoords = FitToBorders(ImagePosition - ivec2(x, 0), ImageSize);
                Color += imageLoad(u_NormalImage,  PixelCoords);
            } 
            //imageStore(u_NormalImage, ImagePosition, Color / float(u_Settings.BlurSamples * 2));
        }
      
        if(blur == STAGE_BLUR_V)
		{
            for (int y = 0; y < u_Settings.BlurSamples; y++)
            {
                ivec2 PixelCoords =  FitToBorders(ImagePosition + ivec2(0, y), ImageSize);
                Color += imageLoad(u_NormalImage,  PixelCoords);
                PixelCoords =  FitToBorders(ImagePosition - ivec2(0, y), ImageSize);
                Color += imageLoad(u_NormalImage,  PixelCoords);
            }
            //imageStore(u_NormalImage, ImagePosition, Color / float(u_Settings.BlurSamples * 2));
        }
		
		imageStore(u_NormalImage, ImagePosition, Color / float(u_Settings.BlurSamples * 2));
	}
}

void Combine()
{
    ivec2 ImagePosition    = ivec2(gl_GlobalInvocationID.xy);

    vec4  SSAO             = imageLoad(u_PositionImage, ImagePosition);
    vec4  Original         = imageLoad(u_NormalImage, ImagePosition);

    imageStore(u_NormalImage, ImagePosition, vec4(vec3(Original * SSAO.r), Original.a));
}

void Zoom()
{
   ivec2 ImagePosition = ivec2(gl_GlobalInvocationID.xy);
    ivec2 ImageSize     = imageSize(u_PositionImage);

    // Центр изображения
    vec2 Center = vec2(ImageSize) * 0.5;

    // Вычисляем направление от центра к текущей позиции
    vec2 Direction = vec2(ImagePosition) - Center;

    // Фактор увеличения (уменьшает расстояние от центра для имитации отдаления)
    vec2 ZoomFactor = vec2(u_Settings.BlurSamples);  // Используем BlurSamples для масштаба

    // Смещаем координаты пикселя в противоположную сторону от центра
    vec2 NewCoords = Center + Direction * ZoomFactor;

    // Приводим координаты к границам изображения
    //ivec2 PixelCoords = FitToBorders(ivec2(NewCoords), ImageSize);
    ivec2 PixelCoords = ivec2(NewCoords);

    // Загружаем цвет пикселя из увеличенной области и сохраняем результат
    vec4 Color = imageLoad(u_NormalImage, PixelCoords);
    imageStore(u_NormalImage, ImagePosition, Color);
}

void main()
{
    if(u_Settings.Stage == STAGE_CREATE)
    {
        Create();
    }
    if(u_Settings.Stage == STAGE_BLUR_H || u_Settings.Stage == STAGE_BLUR_V)
    {
        Blur(u_Settings.Stage);
    }
	if(u_Settings.Stage == STAGE_ZOOM)
    {
        Zoom();
    }
    if(u_Settings.Stage == STAGE_COMBINE)
    {
        Combine();
    }
	// TODO: Try to add the lats downscale stage to fix shadow jumping after blur !
}