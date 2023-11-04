struct Material
{
	vec3  AmbientColor;
	vec3  DiffuseColor;
	vec3  SpecularColor;
	vec3  TransparentColor;
	float Emissive;
	float Opacity;
	float Shininess;
	float ShininessStrength;
	float Refracti;
	int   Shading;
	bool  NormalMapEnabled;
	bool  BumpMapEnabled;
};