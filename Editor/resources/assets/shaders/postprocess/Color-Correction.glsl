#version 460 core
#pragma: compute

layout(local_size_x = 16, local_size_y = 16) in;

layout (rgba16f, set = 1, binding = 0) uniform image2D u_InputImage;
layout (rgba16f, set = 1, binding = 1) uniform image2D u_OutputImage;

layout (push_constant) uniform USettings 
{
   float Exposure;
   float Gamma;
} u_Settings;

vec3 ACES(vec3 color)
{
	//Exposure
	color.rgb *= u_Settings.Exposure;
	// Gamma
	color.rgb  = pow(color.rgb, vec3(1.0 / u_Settings.Gamma));
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
};

vec3 FilmicToneMapping(vec3 color)
{
	//Exposure
	color.rgb *= u_Settings.Exposure;
	// Gamma
	color.rgb  = pow(color.rgb, vec3(1.0 / u_Settings.Gamma));
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
};

vec3 LinearToneMapping(vec3 color)
{
	float exposure = u_Settings.Exposure;
	color = clamp(exposure * color, 0., 1.);
	color = pow(color, vec3(1.0 / u_Settings.Gamma));
	return color;
};

vec3 ReinhardToneMapping(vec3 color)
{
	float exposure = u_Settings.Exposure;
	color = color / (color + vec3(1.0)) * u_Settings.Exposure;
	color = pow(color, vec3(1.0 / u_Settings.Gamma));
	return color;
};

vec3 LumaBasedReinhardToneMapping(vec3 color)
{
	color.rgb *= u_Settings.Exposure;
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1.0 / u_Settings.Gamma));
	return color;
};

vec3 RomBinDaHouseToneMapping(vec3 color)
{
	color.rgb *= u_Settings.Exposure;
    color = exp( -1.0 / ( 2.72*color + 0.15 ) );
	color = pow(color, vec3(1.0 / u_Settings.Gamma));
	return color;
};

vec3 Uncharted2ToneMapping(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	float exposure = u_Settings.Exposure;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	color = pow(color, vec3(1.0 / u_Settings.Gamma));
	return color;
};

void main()
{
	ivec2 possition = ivec2(gl_GlobalInvocationID.xy);
	vec4  color = imageLoad(u_InputImage, possition);
	imageStore(u_OutputImage, possition, vec4(ACES(color.rgb), color.a));
}