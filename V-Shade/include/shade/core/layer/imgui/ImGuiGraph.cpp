#include "shade_pch.h"
#include "ImGuiGraph.h"
#include <shade/core/layer/imgui/ImGuiLayer.h>

void shade::ImGuiGraphNodeRender::DrawGridLines(const ImVec2& start, const ImVec2& canvasSize, const float gridSpace, const ImVec2& windowPos, const ImColor& gridColor, const ImColor& gridColor2, ImDrawList* drawList, int divx, int divy)
{
	for (float coord = fmodf(start.x, gridSpace); coord < canvasSize.x; coord += gridSpace, divx++)
		drawList->AddLine(ImVec2(coord, 0.0f) + windowPos, ImVec2(coord, canvasSize.y) + windowPos, !(divx % 10) ? gridColor2 : gridColor);

	for (float coord = fmodf(start.y, gridSpace); coord < canvasSize.y; coord += gridSpace, divy++)
		drawList->AddLine(ImVec2(0.0f, coord) + windowPos, ImVec2(canvasSize.x, coord) + windowPos, !(divy % 10) ? gridColor2 : gridColor);
}

bool shade::ImGuiGraphNodeRender::DrawEndpoint(ImDrawList* drawList, const ImVec2& offset, float radius, float scaleFactor, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor)
{
	const float scaledRadius	= radius * scaleFactor ;
	const float hitboxRaduis	= scaledRadius * 1.5f;
	const ImRect endpointFrame	= { (offset + screenPosition) - ImVec2{ hitboxRaduis, hitboxRaduis }, (offset + screenPosition) + ImVec2{ hitboxRaduis, hitboxRaduis } };

	if (endpointFrame.Contains(ImGui::GetIO().MousePos))
	{
		drawList->AddCircleFilled(offset + screenPosition, scaledRadius, ImGui::ColorConvertFloat4ToU32(hoveredColor));

		return true;
	}
	else
	{
		drawList->AddCircleFilled(offset + screenPosition, scaledRadius, ImGui::ColorConvertFloat4ToU32(color));

		return false;
	}
}

bool shade::ImGuiGraphNodeRender::DrawConnection(ImDrawList* drawList, const ImVec2& offset, float scaleFactor, const ImVec2& from, const ImVec2& till, const ImVec4& color, const ImVec4& colorHovered, float thickness)
{
	ImVec2 const	p1 = offset + from;
	ImVec2 const	p4 = offset + till;
	ImVec2 const	p2 = p1 + ImVec2(-50 * scaleFactor, 0);
	ImVec2 const	p3 = p4 + ImVec2(+50 * scaleFactor, 0);

	const bool isOnCurve = IsPointOnBezierCurve(p1, p2, p3, p4, ImGui::GetMousePos(), thickness);

	drawList->AddBezierCubic(p1, p2, p3, p4, 
		isOnCurve ? ImGui::ColorConvertFloat4ToU32(colorHovered) : ImGui::ColorConvertFloat4ToU32(color),
		thickness * scaleFactor);
	return isOnCurve;
}

void shade::ImGuiGraphNodeRender::DrawGrid(
	ImDrawList* drawList,
	const ImVec2& windowPosition, 
	const ImVec2& canvasSize,
	const ImVec2& canvasPosition,
	float scaleFactor,
	ImU32 gridColor, 
	ImU32 gridColor2, 
	float gridSize)
{
	const float gridSpace = gridSize * scaleFactor;
	const int divx = static_cast<int>(-canvasPosition.x / gridSize);
	const int divy = static_cast<int>(-canvasPosition.y / gridSize);

	DrawGridLines(canvasPosition * scaleFactor, canvasSize, gridSpace, windowPosition, gridColor, gridColor2, drawList, divx, divy);
}

bool shade::ImGuiGraphNodeRender::DrawReferNodeConnection(
	ImDrawList* drawList, 
	const ImVec2& offset, 
	float scaleFactor, 
	const ImVec2& point, 
	const ImVec2& fontSizePx, 
	const ImVec4& connectionColor,
	float iconSize)
{
	ImVec2	const	icoScreenPos = offset + point - fontSizePx / 2.f * scaleFactor;
	ImRect  rect = { icoScreenPos, icoScreenPos + fontSizePx * scaleFactor };

	bool hasFocus = false;

	if (rect.Contains(ImGui::GetIO().MousePos))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4 { connectionColor.x * 1.5f,connectionColor.y * 1.5f, connectionColor.z * 1.5f, connectionColor.w } );
		hasFocus = true;
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, connectionColor);
	}

	ImGui::SetCursorScreenPos(icoScreenPos);
	
	ImGuiLayer::DrawFontIcon(u8"\xe867", 1, iconSize);
	ImGui::PopStyleColor();

	return hasFocus;
}


void shade::ImGuiGraphNodeRender::DrawArrowLine(ImDrawList* drawList, const ImVec2& p1, const ImVec2& p2, const ImVec4& color, float factor, float thickness, float scale)
{
	ImVec2 arrowDir = p2 - p1;

	const ImVec2 sEPosition = { p1.x + factor * arrowDir.x, p1.y + factor * arrowDir.y };

	float arrowLength = sqrt(ImLengthSqr(arrowDir));
	arrowDir = (arrowLength) ? arrowDir /= arrowLength : arrowDir;

	const ImVec2 orthogonal = ImVec2{ arrowDir.y, -arrowDir.x };

	const float th = thickness * scale;

	const ImVec2 arrowHeadSize = ImVec2{ th, th + (1.f * scale) };

	const ImVec2 arrowEndPoint1 = sEPosition - arrowDir * arrowHeadSize.x + orthogonal * arrowHeadSize.y;
	const ImVec2 arrowEndPoint2 = sEPosition - arrowDir * arrowHeadSize.x - orthogonal * arrowHeadSize.y;

	drawList->AddLine(p1, sEPosition, ImGui::ColorConvertFloat4ToU32(color), th);
	drawList->AddTriangleFilled(p2, arrowEndPoint1, arrowEndPoint2, ImGui::ColorConvertFloat4ToU32(color));
}

bool shade::ImGuiGraphNodeRender::DrawTransition(ImDrawList* drawList, const ImRect& sR, const ImRect& dR, const ImVec4& color, float thickness, float scale)
{
	const ImVec2 sPosition = sR.GetCenter();
	const ImVec2 ePosition = GetClosestPointOnRectBorder(dR, sPosition);

	const float th = thickness * scale;

	bool hovered = IsPointOnLine(sPosition, ePosition, ImGui::GetMousePos(), th);

	ImVec4 c = (hovered) ? ImVec4{ color.x * 1.5f,  color.y * 1.5f,  color.z * 1.5f,  color.w } : color;

	DrawArrowLine(drawList, sPosition, ePosition, c, 0.95f, thickness, scale);

	return hovered;
}

ImVec2 shade::ImGuiGraphNodeRender::GetClosestPointOnRectBorder(const ImRect& rect, const ImVec2& point)
{
	ImVec2 const points[4] =
	{
		ImLineClosestPoint(rect.GetTL(), rect.GetTR(), point),
		ImLineClosestPoint(rect.GetBL(), rect.GetBR(), point),
		ImLineClosestPoint(rect.GetTL(), rect.GetBL(), point),
		ImLineClosestPoint(rect.GetTR(), rect.GetBR(), point)
	};

	float distancesSq[4] =
	{
		ImLengthSqr(points[0] - point),
		ImLengthSqr(points[1] - point),
		ImLengthSqr(points[2] - point),
		ImLengthSqr(points[3] - point)
	};

	float lowestDistance = FLT_MAX;
	int32_t closestPointIdx = -1;
	for (auto i = 0; i < 4; i++)
	{
		if (distancesSq[i] < lowestDistance)
		{
			closestPointIdx = i;
			lowestDistance = distancesSq[i];
		}
	}

	return points[closestPointIdx];
}

bool shade::ImGuiGraphNodeRender::IsPointOnLine(const ImVec2& A, const ImVec2& B, const ImVec2& P, float thickness)
{
	const glm::vec2 a = ToGLMVec2(A), b = ToGLMVec2(B), p = ToGLMVec2(P);

	glm::vec2 lineDirection = glm::normalize(b - a);
	glm::vec2 pointDirection = glm::normalize(p - a);

	float dotProduct = glm::dot(pointDirection, lineDirection);
	float distance = glm::length(p - a);

	float projectionLength = dotProduct * distance;

	// Check if the projection is within the bounds of the line segment
	if (projectionLength >= 0.0f && projectionLength <= glm::length(b - a)) {
		// Calculate the distance between the point and the line
		float distanceToLine = glm::length(p - (a + lineDirection * projectionLength));

		// Check if the distance is within the thickness
		if (distanceToLine <= thickness) {
			return true; // Point is within the line with thickness
		}
	}

	return false; // Point is not within the line with thickness
}

bool shade::ImGuiGraphNodeRender::IsPointOnBezierCurve(const ImVec2& P1, const ImVec2& P2, const ImVec2& P3, const ImVec2& P4, const ImVec2& P, float tolerance)
{
	const int numSteps = 100;
	float minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i <= numSteps; i++) {
		float t = (float)i / (float)numSteps;
		ImVec2 pointOnCurve = ImBezierCubicCalc(P1, P2, P3, P4, t);
		float distance = ImSqrt(ImLengthSqr(pointOnCurve - P));

		if (distance < minDistance) {
			minDistance = distance;
		}
	}

	return minDistance <= tolerance;
}

glm::vec2 shade::ToGLMVec2(const ImVec2& v) noexcept
{
	return { v.x, v.y };
}
