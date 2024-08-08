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

int shade::math::BinomialCoefficient(int n, int k)
{
	if (k > n) return 0;
	if (k == 0 || k == n) return 1;
	int c = 1;
	for (int i = 0; i < k; ++i)
	{
		c = c * (n - i) / (i + 1);
	}
	return c;
}

float shade::math::CalculateBezierFactor(float currentTime, float start, float end, float blendMin, float blendMax, const std::vector<float>& points)
{
	float u = glm::clamp((currentTime - start) / (end - start), 0.0f, 1.0f);
	float v = 1.0f - u;
	int n = points.size() + 1;

	float blendFactor = glm::pow(v, n) * blendMin;

	for (int i = 0; i < points.size(); ++i)
	{
		int coeff = BinomialCoefficient(n, i + 1);
		blendFactor += coeff * glm::pow(v, n - i - 1) * glm::pow(u, i + 1) * points[i];
	}

	blendFactor += glm::pow(u, n) * blendMax;

	return blendFactor;

	//float u = glm::clamp((currentTime - start) / (end - start), 0.0f, 1.0f);
	//float v = 1.0f - u;
	//int n = points.size() + 1;

	//// Если всего 4 точки (кубическая кривая Безье)
	//if (points.size() == 4) {
	//	return (v * v * v * v * v) * blendMin +
	//		(5 * v * v * v * v * u) * points[0] +
	//		(10 * v * v * v * u * u) * points[1] +
	//		(10 * v * v * u * u * u) * points[2] +
	//		(5 * v * u * u * u * u) * points[3] +
	//		(u * u * u * u * u) * blendMax;
	//}

	//// Общий случай для кривой Безье любого порядка
	//float blendFactor = glm::pow(v, n) * blendMin;

	//for (int i = 0; i < points.size(); ++i)
	//{
	//	int coeff = BinomialCoefficient(n, i + 1);
	//	blendFactor += coeff * glm::pow(v, n - i - 1) * glm::pow(u, i + 1) * points[i];
	//}

	//blendFactor += glm::pow(u, n) * blendMax;

	//return blendFactor;
}