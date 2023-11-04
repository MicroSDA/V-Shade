// Function that calculates the normal vector using a Tangent-Bitangent-Normal matrix.
vec3 Get_TBNNormal(vec3 Normal, mat3 TBN_Matrix)
{
   return normalize(TBN_Matrix * (2.0 * Normal - vec3(1.0)));
}
// Function that calculates the normal vector using a Tangent-Bitangent-Normal matrix.
vec3 Get_TBNNormalOLD(vec3 Normal, mat3 TBN_Matrix)
{
   // Assign the normal vector to the TBN_Normal vector.
   vec3 TBN_Normal = Normal;
   // Apply a transformation to the TBN_Normal vector.
   TBN_Normal = 2.0 * TBN_Normal - vec3(1.0, 1.0, 1.0);
   // Apply the TBN matrix to the transformed TBN_Normal vector and normalize the result.
   TBN_Normal = normalize(TBN_Matrix * TBN_Normal);
   // Return the calculated TBN_Normal vector.
   return TBN_Normal;
}
