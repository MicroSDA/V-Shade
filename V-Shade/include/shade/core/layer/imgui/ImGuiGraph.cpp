#include "shade_pch.h"
#include "ImGuiGraph.h"

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

void shade::ImGuiGraphNodeRender::DrawConnection(ImDrawList* drawList, const ImVec2& offset, float scaleFactor, const ImVec2& from, const ImVec2& till, const ImVec4& connectionColor, float thickness)
{
	ImVec2 const	p1 = offset + from;
	ImVec2 const	p4 = offset + till;
	ImVec2 const	p2 = p1 + ImVec2(-50 * scaleFactor, 0);
	ImVec2 const	p3 = p4 + ImVec2(+50 * scaleFactor, 0);

	drawList->AddBezierCubic(p1, p2, p3, p4, ImGui::ColorConvertFloat4ToU32(connectionColor), thickness * scaleFactor);
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