#include "shade_pch.h"
#include "WindowsWindow.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/core/event/EventManager.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>


static void GLFWErrorCallback(int error, const char* description)

{	//-------------------------------------------------------------------------
	// Error Handling
	//-------------------------------------------------------------------------
	// Callback function for handling GLFW errors.
	// Logs errors that occur within GLFW.
	SHADE_CORE_WARNING("GLFW Error ({0}): {1}", error, description);
}

shade::WindowsWindow::WindowsWindow(const WindowProperties& properties)
{
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------
	// Initializes a new WindowsWindow instance with the provided properties.

	// Set window properties and initialize the window.
	m_Properties = properties;
	m_UserData.Height = properties.Height;
	m_UserData.Width = properties.Width;
	m_UserData.Title = properties.Title;

	Initialize();
}

void shade::WindowsWindow::Initialize()
{
	//-------------------------------------------------------------------------
	// Initialization
	//-------------------------------------------------------------------------

	SHADE_CORE_INFO("Creating window {0} ({1}, {2})", m_Properties.Title, m_Properties.Width, m_Properties.Height);

	// Initialize GLFW error callback if no other windows are active.
	if (s_WindowCount == 0)
	{
		glfwSetErrorCallback(GLFWErrorCallback);
	}

	// Check if the current Render API is supported; log an error if not.
	if (RenderAPI::GetCurrentAPI() != RenderAPI::API::Vulkan)
	{
		SHADE_CORE_ERROR("Only Vulkan API is supported!");
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	}

	// Initialize Vulkan-specific settings.
	if (RenderAPI::GetCurrentAPI() == RenderAPI::API::Vulkan)
		InitWithVulkan();
}

void shade::WindowsWindow::Destroy()
{

	//-------------------------------------------------------------------------
	// Destruction
	//-------------------------------------------------------------------------
	// Cleans up and destroys the window and associated Vulkan resources.

	// Destroy swap chain and associated Vulkan resources.
	m_SwapChain->Destroy();

	// Destroy the GLFW window and reduce the window count.
	glfwDestroyWindow(static_cast<GLFWwindow*>(m_WindowHandle));
	--s_WindowCount;

	// Terminate GLFW if no other windows are active.
	if (s_WindowCount == 0)
		glfwTerminate();
}

void shade::WindowsWindow::InitWithVulkan()
{

	//-------------------------------------------------------------------------
	// Vulkan Initialization
	//-------------------------------------------------------------------------
	// Sets up the window for Vulkan rendering, including creating the swap chain and surface.

	// Set GLFW to not use any specific graphics API.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Create window in fullscreen mode if specified.
	if (m_Properties.FullScreen)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		m_WindowHandle = glfwCreateWindow(mode->width, mode->height, m_Properties.Title.c_str(), monitor, nullptr);
	}
	else
	{
		// Create window in windowed mode with specified dimensions.
		m_WindowHandle = glfwCreateWindow(m_Properties.Width, m_Properties.Height, m_Properties.Title.c_str(), nullptr, nullptr);
		// VSync for OpenGL only; Vulkan handles VSync through the swap chain.
		// glfwSwapInterval(1);
	}

	// Increment the window count.
	++s_WindowCount;

	// Create and initialize the Vulkan swap chain.
	m_SwapChain = SwapChain::Create();
	m_SwapChain->Initialize();
	m_SwapChain->As<VulkanSwapChain>().CreateSurface(static_cast<GLFWwindow*>(m_WindowHandle));
	m_SwapChain->CreateFrame(&m_UserData.Width, &m_UserData.Height, Renderer::GetFramesCount(), m_Properties.VSync);

	// Set the user pointer for GLFW window callbacks.
	glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_WindowHandle), &m_UserData);
	InitEvents();
}

void shade::WindowsWindow::InitEvents()
{

	//-------------------------------------------------------------------------
	// Event Initialization
	//-------------------------------------------------------------------------
	// Sets up GLFW callbacks for handling window events such as resizing, closing, and input.

	m_UserData.pSwapchain = m_SwapChain.Raw();

	//------------------------------------------------------------------------
	// Window Resize Event
	//------------------------------------------------------------------------
	// Callback to handle window resizing events.
	glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, int width, int height)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			if (!data.IsMinimaized && width && height)
			{
				data.pSwapchain->OnResize(width, height);
			}
			if (!width && !height) data.IsMinimaized = true;

			EventManager::PushEvent<WindowResizeEvent>(width, height);
		});

	//------------------------------------------------------------------------
	// Window Minimize Event
	//------------------------------------------------------------------------
	// Callback to handle window minimize events.
	glfwSetWindowIconifyCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, int iconified)
		{
			reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window))->IsMinimaized = iconified;
		});

	//------------------------------------------------------------------------
	// Window Close Event
	//------------------------------------------------------------------------
	// Callback to handle window close events.
	glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			EventManager::PushEvent<WindowCloseEvent>();
		});

	//------------------------------------------------------------------------
	// Keyboard Events
	//------------------------------------------------------------------------
	// Callback to handle keyboard input events.
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

	//------------------------------------------------------------------------
	// Character Input Events
	//------------------------------------------------------------------------
	// Callback to handle character input events.
	glfwSetCharCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, unsigned int keycode)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			EventManager::PushEvent<KeyTypedEvent>(keycode);
		});

	//------------------------------------------------------------------------
	// Mouse Button Events
	//------------------------------------------------------------------------
	// Callback to handle mouse button events.
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

	//------------------------------------------------------------------------
	// Mouse Scroll Events
	//------------------------------------------------------------------------
	// Callback to handle mouse scroll events.
	glfwSetScrollCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, double xOffset, double yOffset)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			EventManager::PushEvent<MouseScrolledEvent>(event);
		});

	//------------------------------------------------------------------------
	// Mouse Movement Events
	//------------------------------------------------------------------------
	// Callback to handle mouse movement events.
	glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_WindowHandle), [](GLFWwindow* window, double xPos, double yPos)
		{
			UserData& data = *(UserData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xPos, (float)yPos);
			EventManager::PushEvent<MouseMovedEvent>(event);
		});
}

void shade::WindowsWindow::ProcessEvents()
{
	//-------------------------------------------------------------------------
	// Event Processing
	//-------------------------------------------------------------------------
	// Polls for and processes incoming events from the window.

	glfwPollEvents();
}

void shade::WindowsWindow::SwapBuffers(std::uint32_t timeOut)
{
	//-------------------------------------------------------------------------
	// Buffer Swap
	//-------------------------------------------------------------------------
	// Presents the swap chain image to the screen, using a specified timeout.

	m_SwapChain->Present(timeOut);
}