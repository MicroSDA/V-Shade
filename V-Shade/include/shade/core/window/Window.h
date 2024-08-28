#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/event/Event.h>
#include <shade/core/render/SwapChain.h>

namespace shade
{
	// Contains properties for creating a window.
	struct SHADE_API WindowProperties
	{
		std::string Title;              // Title of the window.
		std::uint32_t Width;            // Width of the window.
		std::uint32_t Height;           // Height of the window.
		bool FullScreen;                // Whether the window is fullscreen.
		bool VSync;                     // Whether VSync is enabled.

		// @brief Constructs WindowProperties with optional parameters.
		// @param title: The title of the window.
		// @param width: The width of the window.
		// @param height: The height of the window.
		// @param fullscreen: Whether the window is fullscreen.
		// @param vSync: Whether VSync is enabled.
		WindowProperties(const std::string& title = "Shade", std::uint32_t width = 1600, std::uint32_t height = 900, bool fullscreen = false, bool vSync = false);
	};

	// Abstract Window class.
	class SHADE_API Window
	{
	public:
		// Data used by the window.
		struct UserData
		{
			std::string Title;           // Title of the window.
			std::uint32_t Width;         // Width of the window.
			std::uint32_t Height;        // Height of the window.
			SwapChain* pSwapchain = nullptr; // Pointer to the swap chain.
			bool IsMinimaized = false;   // Whether the window is minimized.
		};

	public:
		virtual ~Window() = default;

		// @brief Presents the completed frame to the screen.
		// @param timeOut: The maximum time to wait for the swap operation (default: UINT32_MAX).
		virtual void SwapBuffers(std::uint32_t timeOut = UINT32_MAX) = 0;

		// @brief Processes incoming events for the window.
		virtual void ProcessEvents() = 0;

		// @brief Gets the current width of the window.
		// @return The width of the window.
		virtual std::uint32_t GetWidth() = 0;

		// @brief Gets the current height of the window.
		// @return The height of the window.
		virtual std::uint32_t GetHeight() = 0;

		// @brief Gets the native window handle.
		// @return A pointer to the native window handle.
		virtual void* GetNativeWindow() = 0;

		// @brief Handles the window resize event.
		// @param event The resize event.
		// @return True if the event was handled, false otherwise.
		bool OnResizeEvent(WindowResizeEvent& event);

		// @brief Handles the window close event.
		// @param event: The close event.
		// @return True if the event was handled, false otherwise.
		bool OnCloseEvent(WindowCloseEvent& event);

		// @brief Checks if the window is minimized.
		// @return True if the window is minimized, false otherwise.
		SHADE_INLINE bool IsMinimized() const
		{
			return m_UserData.IsMinimaized;
		}

		// @brief Gets the swap chain associated with the window.
		// @return A unique pointer to the swap chain.
		SHADE_INLINE UniquePointer<SwapChain>& GetSwapChain()
		{
			return m_SwapChain;
		}

		// @brief Creates a new window instance.
		// @param properties: The properties for the window (default: WindowProperties).
		// @return A reference to the created window.
		static Window& Create(const WindowProperties& properties = WindowProperties());

		// @brief Destroys the current window instance.
		static void ShutDown();

	protected:
		// @brief Initializes the window.
		virtual void Initialize() = 0;

		// @brief Destroys the window and releases resources.
		virtual void Destroy() = 0;

		static std::uint8_t s_WindowCount; // Count of active windows.
		UserData m_UserData;               // User data for the window.
		WindowProperties m_Properties;     // Properties of the window.
		void* m_WindowHandle;              // Native handle of the window.
		UniquePointer<SwapChain> m_SwapChain; // Swap chain associated with the window.
	};
}