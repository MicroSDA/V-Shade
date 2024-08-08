#pragma once
#pragma once
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtx/matrix_decompose.hpp>
#include <shade/config/ShadeAPI.h>

namespace shade
{
	namespace math
	{
		// Allgiment for sse
		__declspec(align(16)) struct sse_vec4f
		{
			__m128 xyzw;

			sse_vec4f() = default;
			sse_vec4f(__m128 other) { xyzw = other; }
			sse_vec4f(__m128& other) { xyzw = other; }
			//sse_vec4f(const __m128& other) { xyzw = other; }

			sse_vec4f(glm::vec4& other)
			{
				xyzw = _mm_load_ps(glm::value_ptr(other));
			}
			// Sum two vectors
			sse_vec4f operator + (const sse_vec4f& other)
			{
				return _mm_add_ps(this->xyzw, other.xyzw);
			}
			// Substract one vector from another
			sse_vec4f operator - (const sse_vec4f& other)
			{
				return _mm_sub_ps(this->xyzw, other.xyzw);
			}
			// Multyply two vectors
			sse_vec4f operator * (const sse_vec4f& other)
			{
				return _mm_mul_ps(this->xyzw, other.xyzw);
			}
			// Divide vector
			sse_vec4f operator / (const sse_vec4f& other)
			{
				return _mm_div_ps(this->xyzw, other.xyzw);
			}
			// Getting glm::vec4  Not sure if this works, didnt tested
			operator glm::vec4()
			{
				auto value = reinterpret_cast<float*>(&xyzw);
				return glm::vec4(value[0], value[1], value[2], value[3]);
			}
		};
		// Allgiment for sse
		__declspec(align(16)) struct sse_mat4f
		{
			sse_vec4f col0;
			sse_vec4f col1;
			sse_vec4f col2;
			sse_vec4f col3;

			sse_mat4f() = default;
			sse_mat4f(const glm::mat4& other)
			{
				col0 = _mm_loadu_ps(glm::value_ptr(other)); // + 0
				col1 = _mm_loadu_ps(glm::value_ptr(other) + 4);
				col2 = _mm_loadu_ps(glm::value_ptr(other) + 8);
				col3 = _mm_loadu_ps(glm::value_ptr(other) + 12);
			}

			sse_vec4f operator * (const sse_vec4f& vec)
			{
				__m128 xxxx = _mm_shuffle_ps(vec.xyzw, vec.xyzw, _MM_SHUFFLE(0, 0, 0, 0));
				__m128 yyyy = _mm_shuffle_ps(vec.xyzw, vec.xyzw, _MM_SHUFFLE(1, 1, 1, 1));
				__m128 zzzz = _mm_shuffle_ps(vec.xyzw, vec.xyzw, _MM_SHUFFLE(2, 2, 2, 2));
				__m128 wwww = _mm_shuffle_ps(vec.xyzw, vec.xyzw, _MM_SHUFFLE(3, 3, 3, 3));

				return sse_vec4f(_mm_add_ps(
					_mm_add_ps(_mm_mul_ps(xxxx, col0.xyzw), _mm_mul_ps(yyyy, col1.xyzw)),
					_mm_add_ps(_mm_mul_ps(zzzz, col2.xyzw), _mm_mul_ps(wwww, col3.xyzw))
				));
			}

			sse_mat4f operator * (const sse_mat4f& other)
			{
				sse_mat4f mat;
				mat.col0 = *this * other.col0; // Check *this !!!
				mat.col1 = *this * other.col1;
				mat.col2 = *this * other.col2;
				mat.col3 = *this * other.col3;
				return mat;
			}
		};

		SHADE_API void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
		SHADE_API void DecomposeMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

		SHADE_API float Lerp(float a, float b, float f);
		SHADE_API int BinomialCoefficient(int n, int k);
		SHADE_API float CalculateBezierFactor(float currentTime, float start, float end, float blendMin, float blendMax, const std::vector<float>& points);
	}
}