#include "lighting/Light.glsl"
// The function calculates the final color of a pixel using the Blinn-Phong lighting model with direct lighting
vec4 BilinPhongGlobalLight(GlobalLight light, Material material, vec4 diffuseTexture, vec4 specularTexture, vec3 normal, vec3 toCameraDirection, float shadow)
{
    // Calculate ambient color using the diffuse texture and the material's ambient color
    vec4 ambientColor = vec4(diffuseTexture.rgb * material.AmbientColor, diffuseTexture.a);
    // Initialize variables for diffuse and specular colors
    vec4 diffuseColor = vec4(0.0);
    vec4 specularColor = vec4(0.0);

    // Calculate the intensity of the dot product between the normal and the direction of the light
    float diffuseShading = dot(normal, -light.Direction);

    // Check if the diffuse shading is positive
    if (diffuseShading > 0.0)
    {
        // Calculate the diffuse color using the diffuse texture, the material's diffuse color, the light's diffuse color, and the diffuse shading
        diffuseColor = vec4(diffuseTexture.rgb * material.DiffuseColor * light.DiffuseColor * diffuseShading, diffuseTexture.a);
        //diffuseColor = vec4((diffuseTexture.rgb * material.DiffuseColor * light.DiffuseColor * diffuseShading), diffuseTexture.a);
        // Calculate the light reflection vector and the intensity of the dot product between the reflection and the camera's direction
        vec3 lightReflection = reflect(light.Direction, normal);
        float specularShading = dot(toCameraDirection, lightReflection);

        // Check if the specular shading is positive
        if (specularShading > 0.0)
        {
            // Apply the specular power to the specular shading and calculate the specular color using the specular texture, the material's specular color, the light's specular color, the specular power, and the specular shading
            specularShading = pow(specularShading, material.Shininess);
            specularColor = vec4(specularTexture.rgb * material.SpecularColor * light.SpecularColor * material.ShininessStrength * specularShading, specularTexture.a);
        }
    }

    // Calculate the final color, which is the sum of the ambient color, the diffuse color, and the specular color, multiplied by the light's intensity and shadow factor
    return vec4(vec3((diffuseColor.rgb + specularColor.rgb) * light.Intensity * shadow), diffuseTexture.a);
}
// The function calculates the final color of a pixel using the Blinn-Phong lighting model with point lighting
vec4 BilinPhongPointLight(PointLight light, Material material, vec4 diffuseTexture, vec4 specularTexture, vec3 normal, vec3 vertex, vec3 toCameraDirection, float shadow)
{
    // Some issue with alpha !
	// Calculate the vector from the vertex position to the light position
    vec3 lightDirection = vertex - light.Position;
    // Calculate the distance between the vertex and the light
    float distanceBetween = length(lightDirection);
    // Normalize the light direction vector
    lightDirection = normalize(lightDirection);
    // Create a new directional light object and set its properties
    GlobalLight globalLight;
    globalLight.Direction = lightDirection;
    globalLight.DiffuseColor = light.DiffuseColor;
    globalLight.SpecularColor = light.SpecularColor;
    globalLight.Intensity = light.Intensity;
    // Calculate the attenuation factor based on the distance and light properties
    float attenuation = clamp(1.0 - (distanceBetween * distanceBetween) / (light.Distance * light.Distance), 0.0, 1.0);
    attenuation *= mix(attenuation, 1.0, light.Falloff );
    // Calculate the total color using the BilinPhongDirectLight function and the attenuation factor
    vec4 totalColor = BilinPhongGlobalLight(globalLight, material, diffuseTexture, specularTexture, normal, toCameraDirection, shadow);
    return totalColor * attenuation;
}
// The function calculates the final color of a pixel using the Blinn-Phong lighting model with spot lighting
vec4 BilinPhongSpotLight(SpotLight light, Material material, vec4 diffuseTexture, vec4 specularTexture, vec3 normal, vec3 vertex, vec3 toCameraDirection, float shadow) {

	// Harcoded value for smoothstep
	const float Smooth = 2.0;
	// Direction from the current vertex to the light position
	vec3 lightDirection = normalize(vertex - light.Position);
	// Compute dot product between the light direction and the spot light direction
	float spotFactor = dot(lightDirection, light.Direction);
	// Check if angle is within the spot light cone
	if (spotFactor > light.MaxAngle) 
    {
		// Compute the spot light factor based on the range between the MaxAngle and MinAngle values
		float epsilon = light.MinAngle - light.MaxAngle;
		spotFactor = smoothstep(0.0, Smooth, (spotFactor - light.MaxAngle) / epsilon);
		// Create a point light from the given spot light
		PointLight pointLight;
		pointLight.Position = light.Position;
		pointLight.DiffuseColor = light.DiffuseColor;
		pointLight.SpecularColor = light.SpecularColor;
		pointLight.Intensity = light.Intensity;
		pointLight.Distance = light.Distance;
		pointLight.Falloff = light.Falloff;
		// Compute bilinear interpolation Phong lighting using the created point light
		vec4 totalColor = BilinPhongPointLight(pointLight, material, diffuseTexture, specularTexture, normal, vertex, toCameraDirection, shadow);
		// Apply the previously computed spot factor to the total color
		return totalColor * spotFactor;
	}
    // If the angle is outside the spot light cone, return black
	return vec4(0, 0, 0, diffuseTexture.a);
}
