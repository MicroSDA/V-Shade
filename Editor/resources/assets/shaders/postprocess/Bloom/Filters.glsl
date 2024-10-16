#define EPSILON 1.0e-4

/* Quadratic threshold */
vec4 QThreshold(vec4 color, vec3 curve, float threshold, float exposure)
{
	float br =  max(max(color.r, color.g), color.b);
    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * (rq * rq);
    // Combine and apply the brightness response curve.
    color.rgb *= (max(rq, br - threshold) / max(br, EPSILON)) * exposure;
    return color;
};

vec4 DownsampleBox4(sampler2D texture, vec2 uv, vec2 texelSize, int lod)
{
	vec4 offset  = texelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0);
    
    vec4 color 	 = textureLod(texture, uv + offset.xy, lod);
    color.rgb 	+= textureLod(texture, uv + offset.zy, lod).rgb;
    color.rgb	+= textureLod(texture, uv + offset.xw, lod).rgb;
    color.rgb 	+= textureLod(texture, uv + offset.zw, lod).rgb;

    return  vec4(color.rgb * (1.0 / 4.0), color.a);
};

vec4 DownsampleBox13(sampler2D texture, vec2 uv, vec2 texelSize, int lod)
{
	texelSize /= 0.5; // Sample from center of texels
    
	vec4 A = textureLod(texture, uv + texelSize * vec2(-1.0, -1.0), lod);
    vec4 B = textureLod(texture, uv + texelSize * vec2( 0.0,  0.0), lod);
    vec4 C = textureLod(texture, uv + texelSize * vec2( 1.0, -1.0), lod);
    vec4 D = textureLod(texture, uv + texelSize * vec2(-0.5, -0.5), lod);
    vec4 E = textureLod(texture, uv + texelSize * vec2( 0.5, -0.5), lod);
    vec4 F = textureLod(texture, uv + texelSize * vec2(-1.0,  0.0), lod);

    vec4 G = textureLod(texture, uv, lod				   	             );

    vec4 H = textureLod(texture, uv + texelSize * vec2( 1.0,  0.0), lod);
    vec4 I = textureLod(texture, uv + texelSize * vec2(-0.5,  0.5), lod);
    vec4 J = textureLod(texture, uv + texelSize * vec2( 0.5,  0.5), lod);
    vec4 K = textureLod(texture, uv + texelSize * vec2(-1.0,  1.0), lod);
    vec4 L = textureLod(texture, uv + texelSize * vec2( 0.0,  1.0), lod);
    vec4 M = textureLod(texture, uv + texelSize * vec2( 1.0,  1.0), lod);

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 color  = (D + E + I + J) * div.x;
		 color += (A + B + G + F) * div.y;
    	 color += (B + C + H + G) * div.y;
    	 color += (F + G + L + K) * div.y;
    	 color += (G + H + M + L) * div.y;

    return color;
};

vec4 UpsampleBox4(sampler2D texture, vec2 uv, vec2 texelSize, int lod)
{
	vec4 offset  = texelSize.xyxy * vec4(-1.0, -1.0, 1.0, 1.0) * (1.0 * 0.5);
	
    vec4 color 	 = textureLod(texture, uv + offset.xy, lod);
    color.rgb 	+= textureLod(texture, uv + offset.zy, lod).rgb;
    color.rgb	+= textureLod(texture, uv + offset.xw, lod).rgb;
    color.rgb 	+= textureLod(texture, uv + offset.zw, lod).rgb;

    return  vec4(color.rgb * (1.0 / 4.0), color.a);
};

vec4 UpsampleTent(sampler2D texture, vec2 uv, vec2 texelSize, int lod)
{
	vec4 offset  = texelSize.xyxy * vec4(1.0, 1.0, -1.0, 0.0) * (1.0);
	
	vec4 color   = textureLod(texture, uv - offset.xy, lod);
	color 		+= textureLod(texture, uv - offset.wy, lod) * 2.0;
    color 		+= textureLod(texture, uv - offset.zy, lod);
	
    color 		+= textureLod(texture, uv + offset.zw, lod) * 2.0;
	color 		+= textureLod(texture, uv 		     , lod) * 4.0;
	color 		+= textureLod(texture, uv + offset.xw, lod) * 2.0;
		
	color 		+= textureLod(texture, uv + offset.zy, lod);
	color 		+= textureLod(texture, uv + offset.wy, lod) * 2.0;
	color 		+= textureLod(texture, uv + offset.xy, lod);
	
	return color * (1.0 / 16.0);
};