#include "shade_pch.h"
#include "Application.h"
#include <shade/core/event/EventManager.h>

shade::Application* shade::Application::m_spInstance = nullptr;

shade::Application::Application(int argc, char* argv[]):
	m_IsQuitRequested(false)
{
	// Check if app has been already created.
	if (!m_spInstance)
	{
		m_spInstance = this;
	}
	else
	{
		SHADE_CORE_ERROR("Application already exists!")
	}
}

shade::Application::~Application()
{
	//m_Window->ShutDown();
}

shade::UniquePointer<shade::Window>& shade::Application::GetWindow()
{
	return m_spInstance->m_Window;
}

void shade::Application::Initialize()
{
	file::FileManager::Initialize("./");
	AssetManager::Initialize();

	// Create dummy scene.
	Scene::Create("Dummy")->SetAsActiveScene();

	EventManager::Initialize();
}

shade::Application& shade::Application::GetInstance()
{
	return *m_spInstance;
}

void shade::Application::Launch()
{
	WhileRunning();
}

void shade::Application::Terminate()
{
	Layer::RemoveAllLayers();
	Scene::GetActiveScene()->DestroyAllEntites();
	EventManager::ShutDown();
	AssetManager::ShutDown();
	Window::ShutDown();
	Renderer::ShutDown();
	OnDestroy();
}

void shade::Application::WhileRunning()
{
	while (!m_IsQuitRequested)
	{
		/* Update delta time */
		m_FrameTimer.Update();
		// Resive all events.
		m_Window->ProcessEvents();
		EventManager::PollEvents();

		if(Scene::GetActiveScene()->IsPlaying())
			Scene::GetActiveScene()->OnPlaying(m_FrameTimer);
		
		// Update layer
		

		/* Render part */

		if (!m_Window->IsMinimized())
		{
			Layer::OnLayersUpdate(Scene::GetActiveScene(), m_FrameTimer);

			m_Window->GetSwapChain()->BeginFrame();
				Layer::OnLayersRender(Scene::GetActiveScene(), m_FrameTimer);
				m_Window->SwapBuffers();
			m_Window->GetSwapChain()->EndFrame();
		}
		//SHADE_CORE_DEBUG("FPS : {0}", 1000 / m_FrameTimer.GetInMilliseconds());

		AssetManager::DeliveryAssets();
	}
}

void shade::Application::OnEvent(Event& event)
{
	auto window = m_spInstance->m_Window.Raw();
	auto app = m_spInstance;

	if (window)
	{
		EventManager::Dispatch<WindowResizeEvent>([window](WindowResizeEvent& e) 
			{ return window->OnResizeEvent(e); }, event);

		EventManager::Dispatch<WindowCloseEvent>([window, app](WindowCloseEvent& e) 
			{ m_spInstance->m_IsQuitRequested = true; return window->OnCloseEvent(e); }, event);
	}

	//TODO: Make thos functions virtual and impemelnt in client application 
	EventManager::Dispatch<KeyPressedEvent>([](KeyPressedEvent& e)
		{ return OnKeyPressedEvent(e); }, event);

	EventManager::Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e)
		{ return OnKeyReleasedEvent(e); }, event);

	EventManager::Dispatch<KeyTypedEvent>([](KeyTypedEvent& e)
		{ return OnKeyTypedEvent(e); }, event);

	Layer::OnLayersEvent(Scene::GetActiveScene(), event, m_spInstance->m_FrameTimer);
}

bool shade::Application::OnKeyPressedEvent(KeyPressedEvent& event)
{
	return false;
}

bool shade::Application::OnKeyReleasedEvent(KeyReleasedEvent& event)
{
	return false;
}

bool shade::Application::OnKeyTypedEvent(KeyTypedEvent& event)
{
	return false;
}
