#pragma once
#include <shade/config/ShadeAPI.h>
#include <ImGui/imgui.h>


#define IMGUI_DEFINE_MATH_OPERATORS

namespace shade
{
	SHADE_API bool CurveEditor(const char* lable, float v, float bMin, float bMax, float tMin, float tMax, std::vector<float>& points);
}