#pragma once
#include <shade/core/window/Window.h>
#include <shade/utils/Logger.h>

#include <glfw/glfw3.h>

namespace shade
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& properties);
		virtual ~WindowsWindow() = default;
		virtual void Initialize() override;
		virtual void Destroy() override;
		virtual void SwapBuffers(std::uint32_t timeOut) override;
		virtual void* GetNativeWindow() override;
		virtual void ProcessEvents() override;

		virtual std::uint32_t GetWidth() override;
		virtual std::uint32_t GetHeight() override;

	private:
		void InitWithVulkan();
		void InitEvents();
	};
}