#include "shade_pch.h"
#include "WindowsWindow.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/core/event/EventManager.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>

static void GLFWErrorCallback(int error, const char* description)
{
	SHADE_CORE_WARNING("GLFW Error ({0}): {1}", error, description);
}


shade::WindowsWindow::WindowsWindow(const WindowProperties& properties)
{
	m_Properties = properties;
	m_UserData.Height = properties.Height;
	m_UserData.Width = properties.Width;
	m_UserData.Title = properties.Title;
	
	Initialize();
}

void shade::WindowsWindow::Initialize()
{
	SHADE_CORE_INFO("Creating window {0} ({1}, {2})", m_Properties.Title, m_Properties.Width, m_Properties.Height);

	// Create new window in case if we have not any yeat.
	// We have not implemented full functional yeat and it looks redundant.
	if (s_WindowCount == 0)
	{
		// Set callbkack messanger.
		glfwSetErrorCallback(GLFWErrorCallback);
	}

	if (RenderAPI::GetCurrentAPI() != RenderAPI::API::Vulkan)
	{
		SHADE_CORE_ERROR("Only Vulkan api is supported!");
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	}
	if (RenderAPI::GetCurrentAPI() == RenderAPI::API::Vulkan)
		InitWithVulkan();
}

void shade::WindowsWindow::Destroy()
{
	m_SwapChain->Destroy();

	glfwDestroyWindow(static_cast<GLFWwindow*>(m_WindowHandle));
	--s_WindowCount;

	if (s_WindowCount == 0)
		glfwTerminate();
}

void shade::WindowsWindow::InitWithVulkan()
{
	// For Vulkan set NO_API
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (m_Properties.FullScreen)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		m_WindowHandle = glfwCreateWindow(mode->width, mode->height, m_Properties.Title.c_str(), monitor, nullptr);
	}
	else
	{
		m_WindowHandle = glfwCreateWindow(m_Properties.Width, m_Properties.Height, m_Properties.Title.c_str(), nullptr, nullptr);
		//glfwSwapInterval(1); // VSync for OpenGL only
	}
	++s_WindowCount;

	m_SwapChain = SwapChain::Create();
	m_SwapChain->Initialize();
	m_SwapChain->As<VulkanSwapChain>().CreateSurface(static_cast<GLFWwindow*>(m_WindowHandle));
	m_SwapChain->CreateFrame(&m_UserData.Width, &m_UserData.Height, Renderer::GetFramesCount(), m_Properties.VSync);

	glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_WindowHandle), &m_UserData);
	InitEvents();
}

void shade::WindowsWindow::InitEvents()
{
	m_UserData.pSwapchain = m_SwapChain.Raw();
	// Window's size changin event callback.
	glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, int width, int height)
		{
			// TODO: Refactor
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			data.pSwapchain->OnResize(width, height);
			/*data.Height = height;
			data.Width = width;*/
			EventManager::PushEvent<WindowResizeEvent>(width, height);
		});
	// Window's close event callback.
	glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			EventManager::PushEvent<WindowCloseEvent>();
		});
	// Window's key callback.
	glfwSetKeyCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					EventManager::PushEvent<KeyPressedEvent>(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					EventManager::PushEvent<KeyReleasedEvent>(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					EventManager::PushEvent<KeyPressedEvent>(event);
					break;
				}
			}
		});
	// Window's input text callback.
	glfwSetCharCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, unsigned int keycode)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			EventManager::PushEvent<KeyTypedEvent>(keycode);
		});
	// Window's mouse event callback.
	glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, int button, int action, int mods)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					EventManager::PushEvent<MouseButtonPressedEvent>(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					EventManager::PushEvent<MouseButtonReleasedEvent>(event);
					break;
				}
			}
		});
	// Window's mouse scroll event callback.
	glfwSetScrollCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, double xOffset, double yOffset)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			EventManager::PushEvent<MouseScrolledEvent>(event);
		});
	// Window's mouse position event callback.
	glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, double xPos, double yPos)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xPos, (float)yPos);
			EventManager::PushEvent<MouseMovedEvent>(event);
		});
}

void shade::WindowsWindow::ProcessEvents()
{
	// Pool all income window events.
	glfwPollEvents();
}

std::uint32_t shade::WindowsWindow::GetWidth()
{
	return m_Properties.Width;
}

std::uint32_t shade::WindowsWindow::GetHeight()
{
	return m_Properties.Height;
}

void shade::WindowsWindow::SwapBuffers(std::uint32_t timeOut)
{
	m_SwapChain->Present(timeOut);
}

void* shade::WindowsWindow::GetNativeWindow()
{
	return m_WindowHandle;
}
