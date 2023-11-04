#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/KeyCodes.h>
#include <shade/core/MouseCodes.h>
#include <glm/glm/glm.hpp>
namespace shade
{
	class SHADE_API Input
	{
	public:
		static bool IsKeyPressed(const KeyCode& key);
		static bool IsMouseButtonPressed(const MouseCode& button);
		static glm::vec2 GetMousePosition();
		static void SetMousePosition(const std::uint32_t& x, const std::uint32_t& y);
		static void ShowMouseCursor(const bool& show);
	};
}