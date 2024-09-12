#include "shade_pch.h"
#include "ImGuiCurve.h"
#include <imgui_internal.h>

double InterpolateTime(double tMin, double tMax, int currentIteration, int totalIterations)
{
	return tMin + (tMax - tMin) * (static_cast<double>(currentIteration) / (totalIterations - 1));
}

void DrawGrid(const ImVec2& size, ImDrawList* drawList, float density, float bMin, float bMax, float tMin, float tMax)
{
	ImGuiWindow* Window = ImGui::GetCurrentWindow();

	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + size);

	for (int i = 0; i <= size.x; i += (size.x / density))
	{
		const ImVec2 p0{ bb.Min.x + i, bb.Min.y };
		const ImVec2 p1{ bb.Min.x + i, bb.Max.y };
		drawList->AddLine(p0, p1, ImGui::GetColorU32(ImGuiCol_TextDisabled), 1.f);

		ImGui::SetCursorScreenPos(p1 - ImVec2{ 8.f, 8.f });
		if (i) ImGui::Text("%.2f", InterpolateTime(tMin, tMax, i, size.x));
	}
	for (int i = 0; i <= size.y; i += (size.y / density))
	{
		const ImVec2 p0{ bb.Min.x, bb.Min.y + i };
		const ImVec2 p1{ bb.Max.x, bb.Min.y + i };
		drawList->AddLine(p0, p1, ImGui::GetColorU32(ImGuiCol_TextDisabled), 1.f);

		ImGui::SetCursorScreenPos(p0 - ImVec2{ 8.f, 8.f });
		ImGui::Text("%.2f", InterpolateTime(bMax, bMin, i, size.x));
	}

	const ImVec2 px0{ bb.Min.x + size.x / 2.f, bb.Min.y }, px1{ bb.Min.x + size.x / 2.f, bb.Max.y };
	const ImVec2 py0{ bb.Min.x,bb.Min.y + size.y / 2.f }, py1{ bb.Max.x ,bb.Min.y + size.y / 2.f };

	drawList->AddLine(px0, px1, ImGui::GetColorU32(ImGuiCol_TextDisabled), 2.5f);
	drawList->AddLine(py0, py1, ImGui::GetColorU32(ImGuiCol_TextDisabled), 2.5f);
}
//
//// Функция для вычисления биномиальных коэффициентов
//int binomialCoefficient(int n, int k) {
//    if (k > n) return 0;
//    if (k == 0 || k == n) return 1;
//    int c = 1;
//    for (int i = 0; i < k; ++i) {
//        c = c * (n - i) / (i + 1);
//    }
//    return c;
//}
//
//// Функция для вычисления значения кривой Безье в определенный момент времени t
//std::pair<float, float> bezierPoint(const std::vector<std::pair<float, float>>& controlPoints, float t) {
//    int n = controlPoints.size() - 1;
//    std::pair<float, float> result = { 0.0f, 0.0f };
//
//    for (int i = 0; i <= n; ++i) {
//        int coeff = binomialCoefficient(n, i);
//        float term = coeff * std::pow(1 - t, n - i) * std::pow(t, i);
//        result.first += term * controlPoints[i].first;
//        result.second += term * controlPoints[i].second;
//    }
//
//    return result;
//}
//
//// Функция для генерации кривой на основе входных параметров
//float generateCurve(float blendMin, float blendMax, float transitionStart, float transitionEnd, float currentTime, const std::vector<std::pair<float, float>>& controlPoints) {
//    // Нормализуем время в диапазон [0, 1]
//    float t = (currentTime - transitionStart) / (transitionEnd - transitionStart);
//    t = std::clamp(t, 0.0f, 1.0f);
//
//    // Вычисляем точку на кривой Безье
//    std::pair<float, float> bezierPointAtT = bezierPoint(controlPoints, t);
//
//    // Линейно интерполируем значение в диапазоне [blendMin, blendMax] на основе y-координаты точки на кривой
//    float result = blendMin + bezierPointAtT.second * (blendMax - blendMin);
//
//    return result;
//}
//// Функция для интерполяции точки в пространстве окна
//std::pair<float, float> interpolatePoint(const std::pair<float, float>& point, const std::pair<float, float>& windowPos, const std::pair<float, float>& windowSize) {
//	return {
//		windowPos.first + point.first * windowSize.first,
//		windowPos.second + windowSize.second - point.second * windowSize.second
//	};
//}
//
//float CalculateBezierBlendFactor(float currentTime, float start, float end, float blendMin, float blendMax, const std::vector<float>& controlPoints) {
//	float u = std::clamp((currentTime - start) / (end - start), 0.0f, 1.0f);
//	float v = 1.0f - u;
//	int n = controlPoints.size() + 1;
//
//	// Обобщенная формула кубической кривой Безье с произвольным количеством контрольных точек
//	float blendFactor = std::pow(v, n) * blendMin;
//
//	for (int i = 0; i < controlPoints.size(); ++i) {
//		int coeff = binomialCoefficient(n, i + 1);
//		blendFactor += coeff * std::pow(v, n - i - 1) * std::pow(u, i + 1) * controlPoints[i];
//	}
//
//	blendFactor += std::pow(u, n) * blendMax;
//
//	return blendFactor;
//}

#include <shade/core/math/Math.h>

ImVec2 GetPointPositionOnScreen(float t, float f, float tMin, float tMax, const ImVec2& size)
{
	// 1.f ? is blend max ??
	return ImVec2{ (t - tMin) / (tMax - tMin) * size.x, (1.0f - f) * size.y }; // size x
}

int GetProportionalIndex(int currentIndex, int n1Size, int nSize)
{
	// Clamp currentIndex to be within valid range
	nSize = nSize - int(nSize / (n1Size + 1));
	currentIndex = std::clamp(currentIndex, 0, n1Size);

	// Compute proportion in container n1
	float proportion = static_cast<float>(currentIndex) / (n1Size);

	// Compute corresponding index in container n
	int proportionalIndex = static_cast<int>(proportion * (nSize));

	return proportionalIndex;
}

struct SelectedPoint
{
	ImVec2  Position = ImVec2{ 0,0 };
	std::uint32_t Index = 0;
	const char* Lable = "";
};

static struct
{
	const char* Name;
	std::vector<float> Points;

} Presets[] = {
	{ "Linear", {0.5f} },
	{ "Sine in", {0.f} },
	{ "Sine out", {1.f} },
	{ "Sine in-out", {0.f, 0.f, 1.f, 1.f} },
	{ "Ease", {0.f, 1.f, 1.f} },
	{ "Ease in", {0.f, 0.f, 0.5f} },
	{ "Ease out", {0.5f, 1.f, 1.0f} },
};

bool shade::CurveEditor(const char* lable, float v, float bMin, float bMax, float tMin, float tMax, std::vector<float>& points)
{
	ImGui::PushID(lable);

	const ImVec2 screenPosition = ImGui::GetCursorScreenPos() + ImVec2{ 15.f, 15.f };
	ImGui::SetCursorScreenPos(screenPosition);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	// Number of samples along the curve
	const std::uint32_t curveSegments = 100, linearSegments = points.size() + 1;

	static SelectedPoint selectedPoint;

	const float dim = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) - 15.f, iCr = 3.f, oCr = 6.f;
	const ImVec2 size = { dim, dim };
	const ImRect frameSize = { screenPosition, screenPosition + size };

	std::vector<float> lineValues(curveSegments + 1), lineValuesLinear(linearSegments + 1);

	for (int i = 0; i <= curveSegments; ++i)
	{
		float t = tMin + (static_cast<float>(i) / curveSegments) * (tMax - tMin);
		lineValues[i] = math::CalculateBezierFactor(t, tMin, tMax, bMin, bMax, points);
	}
	for (int i = 0; i <= linearSegments; ++i)
	{
		float t = tMin + (static_cast<float>(i) / linearSegments) * (tMax - tMin);
		lineValuesLinear[i] = math::CalculateBezierFactor(t, tMin, tMax, bMin, bMax, points);
	}
	// Keep it allways as sqaure

	ImGui::PlotLines("##LinearBezierCurve", lineValuesLinear.data(), lineValuesLinear.size(), 0, nullptr, bMin, bMax, size);

	ImGui::SetCursorScreenPos(screenPosition);
	const ImVec4 fbgc = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ fbgc.x, fbgc.y, fbgc.z, 0.7f });
	ImGui::PlotLines("##BezierCurve", lineValues.data(), lineValues.size(), 0, nullptr, bMin, bMax, size);
	ImGui::PopStyleColor();
	ImGui::SetCursorScreenPos(screenPosition);
	DrawGrid(size, drawList, (points.size() % 2 == 0) ? points.size() : points.size() + 1, bMin, bMax, tMin, tMax);

	for (std::uint32_t i = 1; i < points.size() + 1; i++)
	{
		const float time = i * (tMax / (points.size() + 1));

		const ImVec2 circlePosition = screenPosition +
			GetPointPositionOnScreen(time, lineValues[GetProportionalIndex(i, points.size(), lineValues.size())], tMin, tMax, size);

		int color = IM_COL32(255, 255, 0, 255);

		const ImRect circleFrame = { circlePosition - ImVec2{ oCr, oCr } *2.f, circlePosition + ImVec2{ oCr, oCr } *2.f };

		const bool isHovered = circleFrame.Contains(ImGui::GetIO().MousePos);

		if ((isHovered || selectedPoint.Index == i) && selectedPoint.Lable == lable) color = IM_COL32(0, 183, 225, 255);

		if (isHovered && !selectedPoint.Index) { selectedPoint.Index = i; selectedPoint.Lable = lable; };

		if (ImGui::IsMouseDragging(0) && selectedPoint.Index == i) { selectedPoint.Position = circlePosition; }


		drawList->AddCircle(circlePosition, isHovered ? oCr * 1.4f : oCr, color); drawList->AddCircleFilled(circlePosition, isHovered ? iCr * 1.4f : iCr, color);
	}

	if (!ImGui::IsMouseDragging(0)) { selectedPoint.Index = 0; };

	if (selectedPoint.Index && selectedPoint.Lable == lable)
	{
		drawList->AddLine(selectedPoint.Position, ImGui::GetIO().MousePos, IM_COL32(0, 183, 225, 255), 2.5f);
		drawList->AddCircle(ImGui::GetIO().MousePos, oCr * 1.4f, IM_COL32(0, 183, 225, 255));
		drawList->AddCircleFilled(ImGui::GetIO().MousePos, iCr * 1.4f, IM_COL32(0, 183, 225, 255));
		ImGui::SetTooltip(std::to_string(points[selectedPoint.Index - 1]).c_str());


		points[selectedPoint.Index - 1] -= ImGui::GetIO().MouseDelta.y / size.y;
		points[selectedPoint.Index - 1] = glm::clamp(points[selectedPoint.Index - 1], bMin, bMax);
	}

	// Draw time point
	//drawList->AddCircle(screenPosition + GetPointPositionOnScreen(v, math::CalculateBezierFactor(v, tMin, tMax, bMin, bMax, points), tMin, tMax, size), 7.0f, ImGui::GetColorU32(ImGuiCol_Text));
	drawList->AddCircleFilled(screenPosition + GetPointPositionOnScreen(v, math::CalculateBezierFactor(v, tMin, tMax, bMin, bMax, points), tMin, tMax, size), oCr, ImGui::GetColorU32(ImGuiCol_Text));
	
	ImGui::SetCursorScreenPos({ screenPosition.x, screenPosition.y + size.y + 15.f });

	if (ImGui::Button("- Point", ImVec2{ size.x / 2.f, 0 }))
	{
		if (points.size() > 1) points.pop_back();

		selectedPoint.Index = 0;
	}

	ImGui::SameLine();

	if (ImGui::Button("+ Point", ImVec2{ size.x / 2.f, 0 }))
	{
		if (points.size() < curveSegments) points.push_back(0.5f);

		selectedPoint.Index = 0;
	}

	ImGui::SetCursorScreenPos({ screenPosition.x, ImGui::GetCursorScreenPos().y });

	if (ImGui::BeginMenu("Pressets"))
	{
		for (int i = 0; i < IM_ARRAYSIZE(Presets); ++i)
		{
			if (ImGui::MenuItem(Presets[i].Name))
				points = Presets[i].Points;
		}

		ImGui::EndMenu();
	}
	
	ImGui::PopID();

	return false;
}

bool shade::GradientBand2DSpace(const char* label, const ImVec2& size, const std::vector<float>& weights, const std::vector<glm::vec2>& points, const glm::vec2& sample_point)
{
	// Проверка на правильность входных данных
	if (points.empty() || weights.empty() || points.size() != weights.size())
		return false;

	// Создаем окно ChildNode
	ImGui::BeginChild(label, size, false);

	// Получаем текущий ImDrawList для рисования
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos(); // верхний левый угол окна

	// Определяем максимальные значения координат по модулю
	float x_max_abs = 0.0f, y_max_abs = 0.0f;

	for (const auto& point : points)
	{
		x_max_abs = std::max(x_max_abs, std::abs(point.x));
		y_max_abs = std::max(y_max_abs, std::abs(point.y));
	}

	// Добавляем офсет к границам для визуального отступа
	float offset = 0.5f; // произвольное значение офсета
	float x_range = x_max_abs + offset;
	float y_range = y_max_abs + offset;

	// Определяем размеры окна
	float width = size.x;
	float height = size.y;

	// Рассчитываем максимальный радиус
	float max_radius = std::min(width, height) / 3.0f;

	// Рисуем сетку
	const int num_lines = 9; // Количество линий сетки
	float step_x = width / (1 * num_lines); // Шаг для X
	float step_y = height / (1 * num_lines); // Шаг для Y

	// Рисуем вертикальные и горизонтальные линии
	for (int i = -num_lines; i <= num_lines; ++i)
	{
		float x = p0.x + width / 2 + i * step_x;
		draw_list->AddLine(ImVec2(x, p0.y), ImVec2(x, p0.y + height), IM_COL32(150, 150, 150, 50));

		float y = p0.y + height / 2 + i * step_y;
		draw_list->AddLine(ImVec2(p0.x, y), ImVec2(p0.x + width, y), IM_COL32(150, 150, 150, 50));
	}

	draw_list->AddRect(p0, p0 + size, IM_COL32(150, 150, 150, 50), 2.f, 0, 2.f);

	// Рисуем координатную сетку и точки
	for (size_t i = 0; i < points.size(); ++i)
	{
		const auto& point = points[i];
		float weight = weights[i];

		// Нормализация координат для отрисовки в пределах окна
		float normalized_x = (point.x / x_range) * (width / 2) + p0.x + width / 2;
		float normalized_y = (1.0f - (point.y / y_range)) * (height / 2) + p0.y;

		// Квадратичная интерполяция радиуса в зависимости от веса
		float radius = 1.0f + weight * weight * (max_radius - 1.0f);

		// Рисуем точку
		draw_list->AddCircle(ImVec2(normalized_x, normalized_y), radius, ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.857f, 0.420f, 0.500f }), 0.f, 5.f);
		draw_list->AddCircleFilled(ImVec2(normalized_x, normalized_y), 5.f, ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.857f, 0.420f, 0.500f }));
		draw_list->AddCircleFilled(ImVec2(normalized_x, normalized_y), radius, ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.857f, 0.420f, 0.500f }));

		ImGui::SetCursorScreenPos(ImVec2(normalized_x, normalized_y));
		ImGui::Text(std::to_string(weight).c_str());
	}

	// Рисуем sample_point как отдельный объект
	float normalized_sample_x = (sample_point.x / x_range) * (width / 2) + p0.x + width / 2;
	float normalized_sample_y = (1.0f - (sample_point.y / y_range)) * (height / 2) + p0.y;
	
	draw_list->AddCircleFilled(ImVec2(normalized_sample_x, normalized_sample_y),	3.0f, ImGui::ColorConvertFloat4ToU32({ 1.0f, 1.0f, 0.2f, 0.8f }));
	draw_list->AddCircle(ImVec2(normalized_sample_x, normalized_sample_y),			9.0f, ImGui::ColorConvertFloat4ToU32({ 1.0f, 1.0f, 0.2f, 0.8f }),0, 3.f);

	ImGui::EndChild();

	return true;

}
