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
	glm::decompose(matrix, scale, rotation, translation, s, p);
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

std::vector<glm::vec2> FindThree(const std::vector<glm::vec2>& points, const std::vector<float>& weights)
{
	// Вектор для хранения индексов трех точек с наибольшими весами
	std::vector<size_t> topIndices(3);

	// Вектор для хранения индексов точек
	std::vector<size_t> indices(points.size());
	std::iota(indices.begin(), indices.end(), 0);

	// Находим топ-3 индекса с наибольшими весами
	std::nth_element(indices.begin(), indices.begin() + 3, indices.end(),
		[&](size_t a, size_t b) { return weights[a] > weights[b]; });

	//// Сортируем топ-3 индекса для корректного порядка
	//std::sort(indices.begin(), indices.begin() + 3,
	//	[&](size_t a, size_t b) { return weights[a] > weights[b]; });

	// Получаем топ-3 точки
	std::vector<glm::vec2> topThree;
	for (size_t i = 0; i < 3; ++i) {
		topThree.push_back(points[indices[i]]);
	}
	return topThree;
}

std::array<std::size_t, 3> FindTree(const std::vector<glm::vec2>& points, const std::vector<float>& weights)
{
	std::vector<std::pair<float, std::size_t>> value_index_pairs;

	for (std::size_t i = 0; i < weights.size(); ++i)
		value_index_pairs.emplace_back(weights[i], i);

	std::sort(value_index_pairs.begin(), value_index_pairs.end(),[](const auto& a, const auto& b) { return a.first > b.first; });

	std::array<size_t, 3> indices = { value_index_pairs[0].second, value_index_pairs[1].second, value_index_pairs[2].second };
	
	std::sort(indices.begin(), indices.end(), [](const auto& a, const auto& b) { return a < b; });

	return indices;
}

glm::vec3 ComputeBarycentricCoordinates(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& sample_point) 
{

	glm::vec2 v0 = p1 - p0;
	glm::vec2 v1 = p2 - p0;
	glm::vec2 v2 = sample_point - p0;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return glm::vec3(u, v, w);
}

SHADE_API std::array<std::pair<std::size_t, float>, 3> shade::math::CalculateGeneralizedTriangularWeights(const std::vector<glm::vec2>& points, const glm::vec2& sample_point)
{
	assert(points.size() >= 3);

	std::array<std::pair<std::size_t, float>, 3> pointsWithWeight;

	std::vector<float> weights = Calculate2DWeightsPolar(points, sample_point);

	std::array<std::size_t, 3> indices = FindTree(points, weights);

	glm::vec3 barycentric_coords = ComputeBarycentricCoordinates(points[indices[0]], points[indices[1]], points[indices[2]], sample_point);

	if (barycentric_coords.x >= 0.0f && barycentric_coords.y >= 0.0f && barycentric_coords.z >= 0.0f)
	{
		pointsWithWeight.at(0) = { indices[0] , barycentric_coords.x };
		pointsWithWeight.at(1) = { indices[1] , barycentric_coords.y };
		pointsWithWeight.at(2) = { indices[2] , barycentric_coords.z };
	}
	else
	{
		pointsWithWeight.at(0) = { indices[0] , weights[indices[0]]};
		pointsWithWeight.at(1) = { indices[1] , weights[indices[1]] };
		pointsWithWeight.at(2) = { indices[2] , weights[indices[2]] };
	}

	// Нормализуем веса в случае необходимости
	
	float totalWeight = pointsWithWeight[0].second + pointsWithWeight[1].second + pointsWithWeight[2].second;

	if (totalWeight > 0.0f) 
	{
		for (auto& [i, weight] : pointsWithWeight)
		{
			weight /= totalWeight;
			
			if (std::isnan(weight) || -std::isnan(weight)) weight = 0.f;
		}
	}
	
	return pointsWithWeight;
}

std::vector<float> shade::math::Calculate2DWeightsPolar(const std::vector<glm::vec2>& points, const glm::vec2& sample_point)
{
	const int POINT_COUNT = static_cast<int>(points.size());
	const float kDirScale = 2.0f;
	std::vector<float> weights(POINT_COUNT, 0.0f);
	float total_weight = 0.0f;

	float sample_mag = glm::length(sample_point);

	for (int i = 0; i < POINT_COUNT; ++i) {
		glm::vec2 point_i = points[i];
		float point_mag_i = glm::length(point_i);

		float weight = 1.0f;

		for (int j = 0; j < POINT_COUNT; ++j) {
			if (j == i) continue;

			glm::vec2 point_j = points[j];
			float point_mag_j = glm::length(point_j);

			float ij_avg_mag = (point_mag_j + point_mag_i) * 0.5f;

			float mag_is = (sample_mag - point_mag_i) / ij_avg_mag;
			float angle_is = SignedAngle(point_i, sample_point);

			float mag_ij = (point_mag_j - point_mag_i) / ij_avg_mag;
			float angle_ij = SignedAngle(point_i, point_j);

			glm::vec2 vec_is = glm::vec2(mag_is, angle_is * kDirScale);
			glm::vec2 vec_ij = glm::vec2(mag_ij, angle_ij * kDirScale);

			float lensq_ij = glm::dot(vec_ij, vec_ij);
			float new_weight = glm::dot(vec_is, vec_ij) / lensq_ij;
			new_weight = 1.0f - new_weight;
			new_weight = std::clamp(new_weight, 0.0f, 1.0f);

			weight = std::min(new_weight, weight);
		}

		weights[i] = weight;
		total_weight += weight;
	}

	for (int i = 0; i < POINT_COUNT; ++i)
	{
		weights[i] /= total_weight;
		if (std::isnan(weights[i]) || -std::isnan(weights[i]))
			weights[i] = 0.f;
	}

	return weights;
}

std::vector<float> shade::math::Calculate2DWeightsCartesian(const std::vector<glm::vec2>& points, const glm::vec2& sample_point)
{
	const int POINT_COUNT = static_cast<int>(points.size());
	std::vector<float> weights(POINT_COUNT, 0.0f);
	float total_weight = 0.0f;

	for (int i = 0; i < POINT_COUNT; ++i) {
		glm::vec2 point_i = points[i];
		glm::vec2 vec_is = sample_point - point_i;

		float weight = 1.0f;

		for (int j = 0; j < POINT_COUNT; ++j) {
			if (j == i) continue;

			glm::vec2 point_j = points[j];
			glm::vec2 vec_ij = point_j - point_i;

			float lensq_ij = glm::dot(vec_ij, vec_ij);
			float new_weight = glm::dot(vec_is, vec_ij) / lensq_ij;
			new_weight = 1.0f - new_weight;
			new_weight = std::clamp(new_weight, 0.0f, 1.0f);

			weight = std::min(weight, new_weight);
		}

		weights[i] = weight;
		total_weight += weight;
	}

	for (int i = 0; i < POINT_COUNT; ++i) {
		weights[i] /= total_weight;
	}

	return weights;
}

SHADE_API std::vector<float> shade::math::Calculate2DWeightsGBi(const std::vector<glm::vec2>& points, const glm::vec2& sample_point)
{
	std::vector<float> weights(points.size(), 0.0f);
	std::vector<float> distances(points.size(), 0.0f);

	float totalInverseDistance = 0.0f;


	for (size_t i = 0; i < points.size(); ++i) {
		float distance = glm::distance(points[i], sample_point);
		distances[i] = distance;

		if (distance > 0.0f) {
			float inverseDistance = 1.0f / distance;
			totalInverseDistance += inverseDistance;
			distances[i] = inverseDistance;
		}
		else {

			weights[i] = 1.0f;
			return weights;
		}
	}


	for (size_t i = 0; i < points.size(); ++i) {
		if (distances[i] > 0.0f) {
			weights[i] = distances[i] / totalInverseDistance;
		}
		else {
			weights[i] = 0.0f;
		}
	}

	return weights;
}
