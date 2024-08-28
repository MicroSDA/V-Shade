#include "shade_pch.h"
#include "Window.h"
#include <shade/platforms/system/windows/WindowsWindow.h>
#include <shade/core/application/Application.h>

std::uint8_t shade::Window::s_WindowCount = 0;

shade::WindowProperties::WindowProperties(const std::string& title, std::uint32_t width, std::uint32_t height, bool fullscreen, bool vSync) :
	Title(title), Width(width), Height(height), FullScreen(fullscreen), VSync(vSync)
{
}

bool shade::Window::OnResizeEvent(WindowResizeEvent& event)
{
	m_Properties.Height = event.Height;
	m_Properties.Width  = event.Width;
	return false;
}

bool shade::Window::OnCloseEvent(WindowCloseEvent& event)
{
	return false;
}

shade::Window& shade::Window::Create(const WindowProperties& properties)
{
#ifdef SHADE_WINDOWS_PLATFORM
	shade::Application::GetInstance().m_Window = UniquePointer<WindowsWindow>::Create(properties);
	return shade::Application::GetInstance().m_Window.Get();
#else
	SHADE_CORE_ERROR("Undefined window's platform!")
		return nullptr;
#endif
}

void shade::Window::ShutDown()
{
	shade::Application::GetInstance().m_Window->Destroy();
	shade::Application::GetInstance().m_Window.Reset();
}
