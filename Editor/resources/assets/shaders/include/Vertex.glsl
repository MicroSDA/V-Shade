// This function returns a matrix that contains the TBN (Tangent, Bitangent, Normal) vectors of a given surface.
mat3 GetTBN_Matrix(mat4 ModelMatrix, vec3 Normal, vec3 Tangent)
{
    // Calculate transformed tangent, normal, and bitangent vectors
    mat3 TBN = mat3(ModelMatrix) * mat3(Tangent, Normal, cross(Tangent, Normal));
    // Extract the transformed tangent, bitangent, and normal vectors
    vec3 T = normalize(TBN[0]);
    vec3 B = normalize(TBN[2]);
    vec3 N = normalize(TBN[1]);
    // Check if handedness of the TBN basis is correct (right-handed)
    // If not, invert the direction of the tangent vector
    if (dot(cross(N, T), B) > 0.0)
        T = -T;
    // Construct the TBN matrix from the tangent, bitangent, and normal vectors
    return mat3(T, -B, N);
}
// This function returns a matrix that contains the TBN (Tangent, Bitangent, Normal) vectors of a given surface.
mat3 GetTBN_MatrixOLD(mat4 ModelMatrix, vec3 Normal, vec3 Tangent)
{
	// Calculate transformed tangent vector
	vec3 T = normalize(vec3(ModelMatrix * vec4(Tangent, 0.0)));
	// Calculate transformed normal vector
	vec3 N = normalize(vec3(ModelMatrix * vec4(Normal, 0.0)));
	// Calculate transformed bitangent (or bi-normal) vector
	// T and N are used to calculate it instead of the original bitangent vector
	T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(cross(T, N));
	// Check if handedness of the TBN basis is correct (right-handed)
	// If not, invert the direction of the tangent vector
	if (dot(cross(N, T), B) > 0.0)
		T = T * -1.0;
	// Construct the TBN matrix from the tangent, bitangent and normal vectors
	return mat3(T, -B, N);
}
vec3 GetTangent(vec3 Normal) 
{
    vec3 C1 = cross(Normal, vec3(0.0, 0.0, 1.0));
    vec3 C2 = cross(Normal, vec3(0.0, 1.0, 0.0));

    if (dot(C1, C1) > dot(C2, C2))
        return normalize(C1);
    else
        return normalize(C2);
}

vec3 ExtractScale(mat4 matrix)
 {
    float scaleX = length(matrix[0].xyz);  
    float scaleY = length(matrix[1].xyz);  
    float scaleZ = length(matrix[2].xyz); 
    return vec3(scaleX, scaleY, scaleZ);
}

