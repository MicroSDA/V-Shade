#include "shade_pch.h"
#include "shade/core/event/Input.h"
#include "shade/core/application/Application.h"

#include <GLFW/glfw3.h>

bool shade::Input::IsKeyPressed(const KeyCode& key)
{
	auto* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
	auto state = glfwGetKey(window, static_cast<int32_t>(key));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool shade::Input::IsMouseButtonPressed(const MouseCode& button)
{
	auto* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
	auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
	return state == GLFW_PRESS;
}

glm::vec2 shade::Input::GetMousePosition()
{
	auto* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return { (float)xpos, (float)ypos };
}
void shade::Input::SetMousePosition(const std::uint32_t& x, const std::uint32_t& y)
{
	auto* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
	glfwSetCursorPos(window, x, y);
}
void shade::Input::ShowMouseCursor(const bool& show)
{
	auto* window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
	glfwSetInputMode(window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}