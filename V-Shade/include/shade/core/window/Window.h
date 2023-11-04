#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/event/Event.h>
#include <shade/core/render/SwapChain.h>

namespace shade
{
	// Contains all properties for creating and window
	struct SHADE_API WindowProperties
	{
		std::string Title;
		std::uint32_t Width;
		std::uint32_t Height;
		bool FullScreen;
		bool VSync;
		WindowProperties(const std::string& title = "Shade", std::uint32_t width = 1600, std::uint32_t height = 900, bool fullscreen = false, bool vSync = false);
	};
	// Window class
	class SHADE_API Window
	{
	public:
		virtual ~Window();
		// Present compleated frame to screen.
		virtual void SwapBuffers(std::uint32_t timeOut = UINT32_MAX) = 0;
		// Recive and process all income events.

		virtual std::uint32_t GetWidth() = 0;
		virtual std::uint32_t GetHeight() = 0;
		virtual void ProcessEvents() = 0;
		// Return pointer to window handle.
		virtual void* GetNativeWindow() = 0;
		// On resize window event callback.
		bool OnResizeEvent(WindowResizeEvent& event);
		// On close window event callback.
		bool OnCloseEvent(WindowCloseEvent& event);
		// Retunr window swapchain
		UniquePointer<SwapChain>& GetSwapChain();
		// Creating window's instance.
		static Window& Create(const WindowProperties& properties = WindowProperties());
		// Destroy window's instance.
		static void ShutDown();
	protected:
		// Initialize window
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		static std::uint8_t s_WindowCount;
		struct UserData
		{
			std::string Title;
			std::uint32_t Width;
			std::uint32_t Height;
			SwapChain* pSwapchain = nullptr;
		} m_UserData;

		WindowProperties m_Properties;
		void* m_WindowHandle;
		UniquePointer<SwapChain> m_SwapChain;
	};

}