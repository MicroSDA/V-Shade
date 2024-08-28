#pragma once
#include <shade/core/window/Window.h>
#include <shade/utils/Logger.h>

#include <glfw/glfw3.h>

namespace shade
{
	// @brief This class represents a window in a Windows environment, providing 
	// the implementation of basic window operations such as initialization, 
	// event processing, and buffer swapping. It inherits from the base 
	// Window class and provides additional functionality for Vulkan integration.
	class WindowsWindow : public Window
	{
	public:
		// @brief Initializes a new WindowsWindow instance with the specified properties.
		// @param properties: Struct containing the properties for initializing 
		// the window, such as width, height, and title.
		WindowsWindow(const WindowProperties& properties);

		// @brief Default destructor for cleaning up resources.
		virtual ~WindowsWindow() = default;

		// @brief Initializes the window, setting up required context and resources.
		// This function sets up the window with the necessary graphics context 
		// and prepares it for rendering operations.
		virtual void Initialize() override;

		// @brief Cleans up and destroys the window along with its associated resources.
		// This function should be called when the window is no longer needed 
		// to ensure proper resource management and avoid memory leaks.
		virtual void Destroy() override;

		// @brief Swaps the front and back buffers to present the rendered image to the screen.
		// @param timeOut: Specifies the time to wait before forcing the swap.
		// A value of UINT32_MAX can be used to wait indefinitely.
		virtual void SwapBuffers(std::uint32_t timeOut) override;

		// @brief Returns a pointer to the native window handle.
		// This function is typically used when integrating with APIs that require 
		// access to the native window (e.g., Vulkan, DirectX).
		// @return: A void pointer to the native window handle (HWND on Windows).
		SHADE_INLINE virtual void* GetNativeWindow() override
		{
			return m_WindowHandle;
		}

		// @brief Processes all pending window events.
		// This function handles user input, window resizing, and other events 
		// that need to be processed to keep the window responsive.
		virtual void ProcessEvents() override;

		// @brief Returns the current width of the window.
		// @return: The width of the window in pixels.
		SHADE_INLINE virtual std::uint32_t GetWidth() override
		{
			return m_Properties.Width;
		}

		// @brief Returns the current height of the window.
		// @return: The height of the window in pixels.
		SHADE_INLINE virtual std::uint32_t GetHeight() override
		{
			return m_Properties.Height;
		}
	private:
		// @brief Initializes the window for use with Vulkan.
		// This function sets up the necessary Vulkan context and extensions required 
		// for rendering using Vulkan within this window.
		void InitWithVulkan();

		// @brief InitEvents
		// Initializes the event system for the window.
		// This function sets up the event handlers required for processing 
		// user input and other window events.
		void InitEvents();
	};
}
