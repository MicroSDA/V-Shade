#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/window/Window.h>
#include <shade/core/render/Renderer.h>
#include <shade/utils/Logger.h>
#include <shade/core/event/Event.h>
#include <shade/core/entity/ECS.h>
#include <shade/core/layer/Layer.h>
#include <shade/core/scripting/ScriptManager.h>

int main(int argc, char* argv[]);

#undef CreateWindow

namespace shade
{
	// Main application class 
	class SHADE_API Application
	{
	public:
		Application(int argc, char* argv[]);
		virtual ~Application();
		static UniquePointer<Window>& GetWindow();
		// Pure virtual functions which has to be implemented by client.
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
	private:
		// Support function for initializing some internal stuff.
		void Initialize();
		static Application& GetInstance();
		// Internal function for starting app, awalible only in main entry point.
		void Launch();
		// Internal function for terminating app, awalible only in main entry point.
		void Terminate();
		// Application main loop.
		void WhileRunning();

		/* Friends class section */
		friend int ::main(int argc, char* argv[]);
		friend class Window;
		/* !Friends class section*/

	public:
		// General event callback function which dispatching all events.
		static void OnEvent(Event& event);
		// Specific on key press event callback function.
		static bool OnKeyPressedEvent(KeyPressedEvent& event);
		// Specific on key released event callback function.
		static bool OnKeyReleasedEvent(KeyReleasedEvent& event);
		// Specific on key typed event callback function.
		static bool OnKeyTypedEvent(KeyTypedEvent& event);
	private:
		// Application instance
		static Application* m_spInstance;
		UniquePointer<Window> m_Window;
		bool m_IsQuitRequested;
		FrameTimer m_FrameTimer;
		SharedPointer<Scene> m_CurrentScene;
	};

	// Has to be created by client
	Application* CreateApplication(int argc, char* argv[]);
}