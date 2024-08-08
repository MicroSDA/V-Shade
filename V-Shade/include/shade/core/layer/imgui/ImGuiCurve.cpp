#include "shade_pch.h"
#include "ImGuiCurve.h"
#include <imgui_internal.h>

////namespace ImGui
////{
////    // Функция для вычисления биномиальных коэффициентов
////    int binomialCoefficient(int n, int k) {
////        if (k > n) return 0;
////        if (k == 0 || k == n) return 1;
////        int c = 1;
////        for (int i = 0; i < k; ++i) {
////            c = c * (n - i) / (i + 1);
////        }
////        return c;
////    }
////
////    template<int steps>
////    void bezier_table(const std::vector<ImVec2>& controlPoints, ImVec2 results[256]) {
////        size_t numPoints = controlPoints.size();
////        std::vector<float> C((steps + 1) * numPoints);
////
////        for (unsigned step = 0; step <= steps; ++step) {
////            float t = (float)step / (float)steps;
////            for (size_t i = 0; i < numPoints; ++i) {
////                C[step * numPoints + i] = binomialCoefficient(numPoints - 1, i) * pow(1 - t, numPoints - 1 - i) * pow(t, i);
////            }
////        }
////
////        for (unsigned step = 0; step <= steps; ++step) {
////            ImVec2 point = { 0, 0 };
////            for (size_t i = 0; i < numPoints; ++i) {
////                point.x += C[step * numPoints + i] * controlPoints[i].x;
////                point.y += C[step * numPoints + i] * controlPoints[i].y;
////            }
////            results[step] = point;
////        }
////    }
////
////    float BezierValue(float dt01, const std::vector<float>& P) {
////        enum { STEPS = 256 };
////        std::vector<ImVec2> Q;
////        Q.emplace_back(0, 0);
////        for (size_t i = 0; i < P.size(); i += 2) {
////            Q.emplace_back(P[i], P[i + 1]);
////        }
////        Q.emplace_back(1, 1);
////        ImVec2 results[STEPS + 1];
////        bezier_table<STEPS>(Q, results);
////        return results[(int)((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * STEPS)].y;
////    }
////
////    int Bezier(const char* label, std::vector<float>& P) {
////        // visuals
////        enum { SMOOTHNESS = 64 };
////        enum { CURVE_WIDTH = 4 };
////        enum { LINE_WIDTH = 1 };
////        enum { GRAB_RADIUS = 8 };
////        enum { GRAB_BORDER = 2 };
////        enum { AREA_CONSTRAINED = true };
////        enum { AREA_WIDTH = 0 };
////
////        // curve presets
////        static struct { const char* name; std::vector<float> points; } presets[] = {
////            { "Linear", { 0.000f, 0.000f, 1.000f, 1.000f } },
////            // Add more presets as needed
////        };
////
////        bool reload = false;
////        ImGui::PushID(label);
////        if (ImGui::ArrowButton("##lt", ImGuiDir_Left)) {
////            if (--P.back() >= 0) reload = true; else ++P.back();
////        }
////        ImGui::SameLine();
////
////        if (ImGui::Button("Presets")) {
////            ImGui::OpenPopup("!Presets");
////        }
////        if (ImGui::BeginPopup("!Presets")) {
////            for (int i = 0; i < IM_ARRAYSIZE(presets); ++i) {
////                if (i == 1 || i == 9 || i == 17) ImGui::Separator();
////                if (ImGui::MenuItem(presets[i].name, NULL, P.back() == i)) {
////                    P.back() = i;
////                    reload = true;
////                }
////            }
////            ImGui::EndPopup();
////        }
////        ImGui::SameLine();
////
////        if (ImGui::ArrowButton("##rt", ImGuiDir_Right)) {
////            if (++P.back() < IM_ARRAYSIZE(presets)) reload = true; else --P.back();
////        }
////        ImGui::SameLine();
////        ImGui::PopID();
////
////        if (reload) {
////            P = presets[(int)P.back()].points;
////        }
////
////        const ImGuiStyle& Style = GetStyle();
////        const ImGuiIO& IO = GetIO();
////        ImDrawList* DrawList = GetWindowDrawList();
////        ImGuiWindow* Window = GetCurrentWindow();
////        if (Window->SkipItems)
////            return false;
////
////        int changed = 0;
////        for (size_t i = 0; i < P.size() - 1; ++i) {
////            char buf[32];
////            sprintf(buf, "P%zu", i);
////            changed |= SliderFloat(buf, &P[i], 0.0f, 1.0f, "%.3f");
////        }
////        int hovered = IsItemActive() || IsItemHovered();
////        Dummy(ImVec2(0, 3));
////
////        const float avail = std::min(GetContentRegionAvail().x, GetContentRegionAvail().y);
////        const float dim = AREA_WIDTH > 0 ? AREA_WIDTH : avail;
////        ImVec2 Canvas(dim, dim);
////
////        ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
////        ItemSize(bb);
////        if (!ItemAdd(bb, NULL))
////            return changed;
////
////        const ImGuiID id = Window->GetID(label);
////        hovered |= 0 != ItemHoverable(ImRect(bb.Min, bb.Min + ImVec2(avail, dim)), id);
////
////        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);
////
////        for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4)) {
////            DrawList->AddLine(
////                ImVec2(bb.Min.x + i, bb.Min.y),
////                ImVec2(bb.Min.x + i, bb.Max.y),
////                GetColorU32(ImGuiCol_TextDisabled));
////        }
////        for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4)) {
////            DrawList->AddLine(
////                ImVec2(bb.Min.x, bb.Min.y + i),
////                ImVec2(bb.Max.x, bb.Min.y + i),
////                GetColorU32(ImGuiCol_TextDisabled));
////        }
////
////        std::vector<ImVec2> Q;
////        Q.emplace_back(0, 0);
////        for (size_t i = 0; i < P.size(); i += 2) {
////            Q.emplace_back(P[i], P[i + 1]);
////        }
////        Q.emplace_back(1, 1);
////
////        ImVec2 results[SMOOTHNESS + 1];
////        bezier_table<SMOOTHNESS>(Q, results);
////
////        {
////            ImVec2 mouse = GetIO().MousePos, pos;
////            for (size_t i = 0; i < P.size() / 2; ++i) {
////                pos = ImVec2(P[i * 2], 1 - P[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
////                float distance = (pos.x - mouse.x) * (pos.x - mouse.x) + (pos.y - mouse.y) * (pos.y - mouse.y);
////
////                if (distance < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS)) {
////                    SetTooltip("(%4.3f, %4.3f)", P[i * 2], P[i * 2 + 1]);
////
////                    if (IsMouseClicked(0) || IsMouseDragging(0)) {
////                        P[i * 2] += GetIO().MouseDelta.x / Canvas.x;
////                        P[i * 2 + 1] -= GetIO().MouseDelta.y / Canvas.y;
////
////                        if (AREA_CONSTRAINED) {
////                            P[i * 2] = std::max(0.0f, std::min(1.0f, P[i * 2]));
////                            P[i * 2 + 1] = std::max(0.0f, std::min(1.0f, P[i * 2 + 1]));
////                        }
////
////                        changed = true;
////                    }
////                }
////            }
////        }
////
////        ImColor color(GetStyle().Colors[ImGuiCol_PlotLines]);
////        for (int i = 0; i < SMOOTHNESS; ++i) {
////            ImVec2 p = { results[i].x, 1 - results[i].y };
////            ImVec2 q = { results[i + 1].x, 1 - results[i + 1].y };
////            ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
////            ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
////            DrawList->AddLine(r, s, color, CURVE_WIDTH);
////        }
////
////        static clock_t epoch = clock();
////        ImVec4 white(GetStyle().Colors[ImGuiCol_Text]);
////        for (int i = 0; i < 3; ++i) {
////            double now = ((clock() - epoch) / (double)CLOCKS_PER_SEC);
////            float delta = (float)(now - (int)now);
////
////            ImVec2 p0 = { delta, 1 - BezierValue(delta, P) };
////            ImVec2 p1 = { p0.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p0.y * (bb.Max.y - bb.Min.y) + bb.Min.y };
////            DrawList->AddCircleFilled(p1, GRAB_RADIUS / 2, ImColor(white));
////        }
////
////        for (size_t i = 0; i < P.size() / 2; ++i) {
////            ImVec2 p0 = ImVec2(P[i * 2], 1 - P[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
////            DrawList->AddCircleFilled(p0, GRAB_RADIUS, ImColor(white));
////            DrawList->AddCircle(p0, GRAB_RADIUS + GRAB_BORDER, ImColor(white));
////        }
////
////        return changed;
////    }
////} // namespace ImGui
//

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
	std::cout << math::CalculateBezierFactor(v, tMin, tMax, bMin, bMax, points) << std::endl;

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
