struct Camera
{
	mat4 	ViewProjectionMatrix;
	mat4 	ViewMatrix;
	mat4 	ProjectionMatrix;
	vec3 	Position;
	vec3 	ForwardDirection;
	float 	Near;
	float 	Far;
};