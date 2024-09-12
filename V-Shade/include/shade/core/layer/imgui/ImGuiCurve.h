#pragma once
#include <shade/config/ShadeAPI.h>
#include <ImGui/imgui.h>
#include <glm/glm/glm.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS

namespace shade
{
	SHADE_API bool CurveEditor(const char* lable, float v, float bMin, float bMax, float tMin, float tMax, std::vector<float>& points);
	SHADE_API bool GradientBand2DSpace(const char* lable, const ImVec2& size, const std::vector<float>& weights, const std::vector<glm::vec2>& points, const glm::vec2& sample_point);
}