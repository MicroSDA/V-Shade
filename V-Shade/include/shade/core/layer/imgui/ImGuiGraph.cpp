#include "shade_pch.h"
#include "ImGuiGraph.h"

void shade::ImGuiGraphNodeRender::DrawGridLines(const ImVec2& start, const ImVec2& canvasSize, const float gridSpace, const ImVec2& windowPos, const ImColor& gridColor, const ImColor& gridColor2, ImDrawList* drawList, int divx, int divy)
{
	for (float coord = fmodf(start.x, gridSpace); coord < canvasSize.x; coord += gridSpace, divx++)
		drawList->AddLine(ImVec2(coord, 0.0f) + windowPos, ImVec2(coord, canvasSize.y) + windowPos, !(divx % 10) ? gridColor2 : gridColor);

	for (float coord = fmodf(start.y, gridSpace); coord < canvasSize.y; coord += gridSpace, divy++)
		drawList->AddLine(ImVec2(0.0f, coord) + windowPos, ImVec2(canvasSize.x, coord) + windowPos, !(divy % 10) ? gridColor2 : gridColor);

}

void shade::ImGuiGraphNodeRender::MoveNode(const ImGuiIO& io, const GraphViewContext& context, ImVec2& position)
{
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		ImVec2 delta = io.MouseDelta / context.Zoom.Zoom;

		if (fabsf(delta.x) >= 0.1f || fabsf(delta.y) >= 0.1f)
		{
			position.x += delta.x;
			position.y += delta.y;
		}
	}
}

bool shade::ImGuiGraphNodeRender::DrawEndpoint(ImDrawList* drawList, const ImVec2& offset, float radius, float scaleFactor, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor)
{
	const float scaledRadius	= radius * scaleFactor;
	const ImRect endpointFrame	= { (offset + screenPosition) - ImVec2{ scaledRadius, scaledRadius }, (offset + screenPosition) + ImVec2{ scaledRadius, scaledRadius } };

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
void shade::ImGuiGraphNodeRender::UpdateZoom(GraphViewContext& context, const ImGuiIO& io)
{
	if (io.MouseWheel <= -std::numeric_limits<float>::epsilon())
		context.Zoom.ZoomTarget *= 1.0f - context.Zoom.ZoomRatio;

	if (io.MouseWheel >= std::numeric_limits<float>::epsilon())
		context.Zoom.ZoomTarget *= 1.0f + context.Zoom.ZoomRatio;
}

void shade::ImGuiGraphNodeRender::Zoom(ImRect region, GraphViewContext& context)
{
	ImGuiIO& io = ImGui::GetIO(); // todo add as arg
	if (region.Contains(io.MousePos))
		UpdateZoom(context, io);

	UpdateView(context, io);
}

void shade::ImGuiGraphNodeRender::UpdateView(GraphViewContext& context, const ImGuiIO& io)
{
	ImVec2 mouseWPosPre = CalculateMouseWorldPos(io, context);

	context.Zoom.ZoomTarget = ImClamp(context.Zoom.ZoomTarget, context.Zoom.MinZoom, context.Zoom.MaxZoom);
	context.Zoom.Zoom = ImClamp(context.Zoom.Zoom, context.Zoom.MinZoom, context.Zoom.MaxZoom);

	context.Zoom.Zoom = ImLerp(context.Zoom.Zoom, context.Zoom.ZoomTarget, context.Zoom.ZoomLerp);

	ImVec2 mouseWPosPost = CalculateMouseWorldPos(io, context);

	if (ImGui::IsMousePosValid())
		context.ViewPosition += mouseWPosPost - mouseWPosPre;
}

ImVec2 shade::ImGuiGraphNodeRender::CalculateMouseWorldPos(const ImGuiIO& io, const GraphViewContext& context)
{
	return (io.MousePos - ImGui::GetCursorScreenPos()) / context.Zoom.Zoom;
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