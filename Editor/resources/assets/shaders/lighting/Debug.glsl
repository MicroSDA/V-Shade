vec3 GetGradient(float value)
{
	vec3 zero   = vec3(0.0, 0.0, 0.0);
	vec3 white  = vec3(0.0, 0.1, 0.9);
	vec3 red    = vec3(0.2, 0.9, 0.4);
	vec3 blue   = vec3(0.8, 0.8, 0.3);
	vec3 green  = vec3(0.9, 0.2, 0.3);

	float step0 = 0.0f;
	float step1 = 2.0f;
	float step2 = 4.0f;
	float step3 = 8.0f;
	float step4 = 16.0f;

	vec3    color = mix(zero,   white,  smoothstep(step0, step1, value));
            color = mix(color,  white,  smoothstep(step1, step2, value));
            color = mix(color,  red,    smoothstep(step1, step2, value));
            color = mix(color,  blue,   smoothstep(step2, step3, value));
            color = mix(color,  green,  smoothstep(step3, step4, value));

	return color;
}