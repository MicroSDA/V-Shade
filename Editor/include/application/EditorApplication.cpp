#include "shade_pch.h"
#include "EditorApplication.h"

shade::Application* shade::CreateApplication(int argc, char* argv[])
{
	return new EditorApplication(argc, argv);
}

EditorApplication::EditorApplication(int argc, char* argv[]) :
	shade::Application(argc, argv)
{
	
}

void EditorApplication::OnCreate()
{
	// Initialzie render.
	shade::Renderer::Initialize(shade::RenderAPI::API::Vulkan, shade::SystemsRequirements{ .GPU { .Discrete = true }, .FramesInFlight = 3 });
	// Crete window.
	shade::Window::Create({"Editor", 1900, 1080, false, true});
	shade::scripts::ScriptManager::Initialize("./resources/scripts");

	// Create layer.
	shade::Layer::Create<EditorLayer>();
}

void EditorApplication::OnDestroy()
{
	
}

