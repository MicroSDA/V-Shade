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
	shade::Window::Create({"Editor", 1900, 900, false, false});

	shade::ScriptManager::Initialize("./resources/scripts");

	auto cameraEntity  = shade::Scene::GetActiveScene()->CreateEntity();
	auto script = shade::ScriptManager::InstantiateScript<shade::ecs::ScriptableEntity*>("Scripts", "Camera");
	
	if (script)
	{
		cameraEntity.AddComponent<shade::CameraComponent>(shade::CameraComponent::Create())->SetPrimary(true);
		cameraEntity.AddComponent<shade::NativeScriptComponent>().Bind(script);
		cameraEntity.AddComponent<shade::TagComponent>("Camera");

		cameraEntity.GetComponent<shade::CameraComponent>()->SetDirection(glm::vec3(-0.011512465, -0.33462766, 0.94228005));
		cameraEntity.GetComponent<shade::CameraComponent>()->SetPosition(glm::vec3(0, 10, -20));
	}
	/*auto entity = shade::Scene::GetActiveScene()->CreateEntity();
	auto& transform = entity.AddComponent<shade::TransformComponent>();

	transform.SetPosition(0, 0, 5);
	transform.SetScale(1, 1, 1);

	shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
		{
			entity.AddComponent<shade::ModelComponent>(model);
		});*/

	/*auto light = shade::Scene::GetActiveScene()->CreateEntity();
	light.AddComponent<shade::TransformComponent>();
	light.AddComponent<shade::GlobalLightComponent>(shade::GlobalLightComponent::Create());*/
	/*for (auto i = 0; i < 20; i++)
	{
		auto spotLight = shade::Scene::GetActiveScene()->CreateEntity();
		spotLight.AddComponent<shade::TagComponent>("SpotLight");
		spotLight.AddComponent<shade::TransformComponent>().SetPosition(i * 10, 0, -2);
		spotLight.AddComponent<shade::SpotLightComponent>(shade::SpotLightComponent::Create())->Distance = 10.f;
	}*/

	{
		/*auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		entity.AddComponent<shade::TransformComponent>().SetScale(5, 5, 5);
		entity.AddComponent<shade::TagComponent>("Sponza");

		shade::AssetManager::GetAsset<shade::Model>("Sponza.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});*/
		
	}
	/*{
		auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		auto& transform = entity.AddComponent<shade::TransformComponent>();

		transform.SetPosition(-10, 5, 0);
		transform.SetScale(1, 2, 1);

		shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});

	}
	{
		auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		auto& transform = entity.AddComponent<shade::TransformComponent>();

		transform.SetPosition(0, 10, 0);
		transform.SetScale(2, 1, 2);

		shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});

	}
	{
		auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		auto& transform = entity.AddComponent<shade::TransformComponent>();

		transform.SetPosition(0, 5, 10);
		transform.SetRotation(0, 0, -0.8);
		transform.SetScale(1, 3, 1);

		shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});

	}
	{
		auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		auto& transform = entity.AddComponent<shade::TransformComponent>();

		transform.SetPosition(0, 5, -10);
		transform.SetRotation(0, 0, -0.8);
		transform.SetScale(1, 1, 1);
		

		shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});

	}
	{
		auto entity = shade::Scene::GetActiveScene()->CreateEntity();
		auto& transform = entity.AddComponent<shade::TransformComponent>();

		transform.SetPosition(10, 5, 0);

		shade::AssetManager::GetAsset<shade::Model>("Cube.Model", shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
			{
				entity.AddComponent<shade::ModelComponent>(model);
			});

	}*/


	// Create layer.
	shade::Layer::Create<EditorLayer>();
	//shade::Layer::Create<GameLayer>();
}

void EditorApplication::OnDestroy()
{
	
}

