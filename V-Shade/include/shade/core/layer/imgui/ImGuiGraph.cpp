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

void shade::ImGuiGraphNodeRender::DrawBorder(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, bool isActive, const ImVec2& nodePosition, const ImVec2& nodeSize)
{
	const ImVec2 scaledPosition = nodePosition * context.Zoom.Zoom;
	const ImVec2 pMin = (offset + scaledPosition);
	const ImVec2 pMax = (offset + scaledPosition + nodeSize * context.Zoom.Zoom);

	const ImVec4 color = (isActive) ? ImVec4(
		context.Style.NodeBorderColor.x * 1.6f,
		context.Style.NodeBorderColor.y * 1.6f,
		context.Style.NodeBorderColor.z * 1.6f,
		context.Style.NodeBorderColor.w) : context.Style.NodeBorderColor;

	drawList->AddRect(pMin, pMax, ImGui::ColorConvertFloat4ToU32(color), context.Style.Rounding, 0, context.Style.NodeBorderWidth);
}

float shade::ImGuiGraphNodeRender::DrawHeader(ImDrawList* drawList, 
	const ImVec2& offset, 
	const GraphViewContext& context,
	const char* title, 
	const ImVec2& nodePosition,
	const ImVec2& nodeSize,
	const ImVec4& headerColor)
{
	const ImVec2 scaledPosition = nodePosition * context.Zoom.Zoom;
	const ImVec2 pMin = (offset + scaledPosition);
	const ImVec2 pMax = (offset + scaledPosition + (ImVec2{ nodeSize.x, context.Style.HeaderHeight } *context.Zoom.Zoom));

	drawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(headerColor), context.Style.Rounding, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersTopLeft);

	ImGui::SetCursorScreenPos(offset + (context.Style.Padding + nodePosition) * context.Zoom.Zoom );
	ImGui::PushStyleColor(ImGuiCol_Text, context.Style.HeaderTextColor);

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 1.5f);

	ImGui::BeginGroup();
	{
		ImGui::Text(title);
	}
	ImGui::EndGroup();

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale / 1.5f);

	ImGui::PopStyleColor(1);

	return nodePosition.y + context.Style.HeaderHeight;
}

bool shade::ImGuiGraphNodeRender::DrawInputEndpoint(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor)
{
	const float radius			= context.Style.EndpointRadius * context.Zoom.Zoom;
	const ImRect endpointFrame	= { (offset + screenPosition) - ImVec2{ radius, radius }, (offset + screenPosition) + ImVec2{ radius,radius } };

	if (endpointFrame.Contains(ImGui::GetIO().MousePos))
	{
		drawList->AddCircleFilled(offset + screenPosition, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(hoveredColor));

		return true;
	}
	else
	{
		drawList->AddCircleFilled(offset + screenPosition, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(color));

		return false;
	}
}

bool shade::ImGuiGraphNodeRender::DrawOutputEndpoint(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor)
{
	const float radius = context.Style.EndpointRadius * context.Zoom.Zoom;
	const ImRect endpointFrame = { (offset + screenPosition) - ImVec2{ radius, radius }, (offset + screenPosition) + ImVec2{ radius,radius } };

	if (endpointFrame.Contains(ImGui::GetIO().MousePos))
	{
		drawList->AddCircleFilled(offset + screenPosition, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(hoveredColor));

		return true;
	}
	else
	{
		drawList->AddCircleFilled(offset + screenPosition, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(color));

		return false;
	}
}

void shade::ImGuiGraphNodeRender::DrawConnection(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& from, const ImVec2& till)
{
	ImVec2 const	p1 = offset + from;
	ImVec2 const	p4 = offset + till;
	ImVec2 const	p2 = p1 + ImVec2(+50 * context.Zoom.Zoom, 0);
	ImVec2 const	p3 = p4 + ImVec2(-50 * context.Zoom.Zoom, 0);

	drawList->AddBezierCubic(p1, p2, p3, p4, ImGui::ColorConvertFloat4ToU32(context.Style.ConnectionColor), context.Style.ConnectionThickness * context.Zoom.Zoom);
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

void shade::ImGuiGraphNodeRender::Grid(ImDrawList* drawList, ImVec2 windowPos, const GraphViewContext& context, const ImVec2& canvasSize, ImU32 gridColor, ImU32 gridColor2, float gridSize)
{
	const float gridSpace = gridSize * context.Zoom.Zoom;
	const int divx = static_cast<int>(-context.ViewPosition.x / gridSize);
	const int divy = static_cast<int>(-context.ViewPosition.y / gridSize);

	DrawGridLines(context.ViewPosition * context.Zoom.Zoom, canvasSize, gridSpace, windowPos, gridColor, gridColor2, drawList, divx, divy);
}