#include "shade_pch.h"
#include "Math.h"

void shade::math::DecomposeMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
{
	/*	Keep in mind that the resulting quaternion in not correct. It returns its conjugate!
		To fix this add this to your code:
		rotation = glm::conjugate(rotation);
	*/
	glm::quat r;
	glm::vec3 s;
	glm::vec4 p;
	glm::decompose(matrix, scale, r, translation, s, p);
	rotation = glm::eulerAngles(r);
}

void shade::math::DecomposeMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
{
	/*	Keep in mind that the resulting quaternion in not correct. It returns its conjugate!
		To fix this add this to your code:
		rotation = glm::conjugate(rotation);
	*/
	glm::quat r;
	glm::vec3 s;
	glm::vec4 p;
	glm::decompose(matrix, scale, r, translation, s, p);
	rotation = glm::conjugate(r);
}

SHADE_API float shade::math::Lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
