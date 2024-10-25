// pre-defined positions for Poisson Disk, used for random sampling
vec2 PoissonDisk[64] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790),
    vec2(0.19395464, -0.59710992),
    vec2(-0.95246724, -0.28234864),
    vec2(0.82848108, -0.39646106),
    vec2(-0.68821649, 0.11384947),
    vec2(-0.70052778, -0.34152595),
    vec2(-0.48602600, -0.62071020),
    vec2(0.22965106, 0.67349999),
    vec2(0.89987391, 0.25926362),
    vec2(0.29072642, 0.57471229),
    vec2(0.40587144, -0.88837403),
    vec2(-0.74051063, -0.97071062),
    vec2(-0.04221503, -0.52335644),
    vec2(0.95312555, 0.58135632),
    vec2(0.51123655, 0.01258006),
    vec2(0.35893541, -0.74317524),
    vec2(-0.54104810, 0.79802669),
    vec2(-0.51346109, 0.14786584),
    vec2(0.16868865, 0.31533074),
    vec2(0.81160056, -0.74992316),
    vec2(-0.34697805, -0.16657797),
    vec2(0.23818597, -0.55350559),
    vec2(-0.54178574, 0.85582912),
    vec2(0.38266116, 0.03750939),
    vec2(-0.28923507, -0.84519702),
    vec2(-0.73884066, 0.64234940),
    vec2(-0.07572641, -0.25826271),
    vec2(0.95108792, -0.02945685),
    vec2(0.28224578, 0.06773410),
    vec2(0.75630233, -0.07800223),
    vec2(-0.54389954, -0.19512462),
    vec2(-0.85112435, -0.36793589),
    vec2(0.15854031, 0.10911019),
    vec2(0.39301195, 0.62110090),
    vec2(-0.57718982, -0.18487056),
    vec2(-0.45611516, -0.58815774),
    vec2(0.61554461, 0.45332724),
    vec2(-0.22402351, -0.80561914),
    vec2(0.54758847, -0.62551867),
    vec2(-0.77095148, 0.48574359),
    vec2(0.05450155, 0.71985514),
    vec2(-0.48790583, -0.67645440),
    vec2(-0.94684805, 0.53650635),
    vec2(-0.92658127, 0.17225508),
    vec2(0.93285148, 0.25628743),
    vec2(-0.22176641, 0.73857760),
    vec2(0.42020236, 0.90442172),
    vec2(0.78714381, -0.55423607),
    vec2(0.64639594, -0.89219145)
);

vec3 OffsetDirections[20] = vec3[](
      vec3( 1, 1, 1),  vec3( 1, -1, 1),  vec3(-1, -1, 1),  vec3(-1, 1, 1),
      vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
      vec3( 1, 1, 0),  vec3( 1, -1, 0),  vec3(-1, -1, 0),  vec3(-1, 1, 0),
      vec3( 1, 0, 1),  vec3(-1, 0, 1),   vec3( 1, 0, -1),  vec3(-1, 0, -1),
      vec3( 0, 1, 1),  vec3( 0, -1, 1),  vec3( 0, -1, -1), vec3( 0, 1, -1) 
);
// linear interpolation function
float LineStep(float low, float high, float v)
{
    // clamps value between 0 and 1
    return clamp((v - low) / (high - low), 0.0, 1.0);
}
// check if value is between 0 and 1
bool ShadowInRange(float value)
{
    return value >= 0.0 && value <= 1.0;
}

float PCF_GeneralLight(sampler2DArray ShadowMap, vec3 ProjectionCoords, uint CascadeLevel, float Radius)
{
    const int Samples = 3;
    float Value = 0.0;
    vec2 TexelSize = 1.0 / vec2(textureSize(ShadowMap, 0));

    for (int x = -Samples; x < Samples; x++)
    {
        for (int y = -Samples; y < Samples; y++)
        {
            vec2 Offset = PoissonDisk[(x + y) % 64] * TexelSize * Radius;
            Value += step(ProjectionCoords.z, texture(ShadowMap, vec3(ProjectionCoords.xy + vec2(x, y) * Offset, CascadeLevel)).r);
        }
    }
    return Value / float((Samples * 2) * (Samples * 2));
}

float GL_ShadowMapping(
    sampler2DArray ShadowMap, 
    mat4 LightViewProjectionMatrix, 
    uint CascadeLevel, 
    vec4 FragPosWorldSapce, 
    vec3 LightDirection, 
    vec3 Normal, 
    vec3 ToCameraDirection)
{
    vec4 FragPosLightSpace = LightViewProjectionMatrix * FragPosWorldSapce;
    // Perspective divide
    vec3 ProjectionCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Set between 0.0 - 1.0 range
    ProjectionCoords.xy  = ProjectionCoords.xy * 0.5 + 0.5;
    // Check if ProjectionCoords in 0.0 - 1.0 range, else return 1.0
    // if (!ShadowInRange(ProjectionCoords.z) || !ShadowInRange(ProjectionCoords.x) || !ShadowInRange(ProjectionCoords.y))
    //     return 1.0;
    
    return PCF_GeneralLight(ShadowMap, ProjectionCoords, CascadeLevel, 1.0);
}

float PCF_SpotLight(sampler2DArray ShadowMap, vec3 ProjectionCoords, float Depth, float Angle, uint LightIndex)
{
    const int Samples   = 36;
	float Value         = 0;
    vec2 TexelSize      = 1.0 / vec2(textureSize(ShadowMap, 0));
    float Radius        = 2.0;
    for(int i = 0; i < Samples; i++)
    {
        vec2 Offset = PoissonDisk[i % 64] * TexelSize * Radius;
		Value += step(Depth, texture(ShadowMap, vec3(ProjectionCoords.xy + Offset, LightIndex)).r);
    }
    return Value / float(Samples);
}

float SPL_ShadowMapping(
    sampler2DArray ShadowMap, 
    mat4 LightViewProjectionMatrix, 
    uint LightIndex, 
    vec3 FragPosWorldSapce,
    vec3 LightDirection, 
    vec3 Normal, 
    vec3 ToCameraDirection)
{
    vec4 FragPosLightSpace = LightViewProjectionMatrix * vec4(FragPosWorldSapce, 1.0);
    // perspective divide
    vec3 ProjectionCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // set between 0 - 1 range
    ProjectionCoords.xy  = ProjectionCoords.xy * 0.5 + 0.5;
    // check if ProjectionCoords in 0 - 1 range, else return 1
    if (!ShadowInRange(ProjectionCoords.z) || !ShadowInRange(ProjectionCoords.x) || !ShadowInRange(ProjectionCoords.y))
        return 1.0;

    float Angle = dot(-LightDirection, Normal);
    return PCF_SpotLight(ShadowMap, ProjectionCoords, ProjectionCoords.z, Angle, LightIndex);
}

float PCF_PointLight(
    samplerCubeArray ShadowMap, 
    uint LightIndex, 
    vec3 ProjectionCoords, 
    float Depth,
    float Distance)
{
    int Samples = 8;
    float Value = 0.0;
    float Radius = 0.006;
    for(int i = 0; i < Samples; ++i)
    {
       Value += step(Depth, texture(ShadowMap, vec4(ProjectionCoords + OffsetDirections[i % 20] * Radius, LightIndex)).r * Distance + 0.1);
    }
    return Value / float(Samples); 
}

float PL_ShadowMapping(
    samplerCubeArray ShadowMap, 
    uint LightIndex, 
    vec3 FragPosWorldSapce, 
    vec3 LightPosition, 
    float Distance)
{
    vec3 FragPosLightSpace = FragPosWorldSapce - LightPosition;
    float CurentDepth = length(FragPosLightSpace);
    if(CurentDepth <= 0.0 || CurentDepth >= Distance)
        return 1.0;

    return PCF_PointLight(ShadowMap, LightIndex, FragPosLightSpace, CurentDepth, Distance) ;
}

