#include "shade_pch.h"
#include "GameLayer.h"
#include <shade/core/system/FileDialog.h>

void GameLayer::OnCreate()
{
	m_SceneRenderer = shade::SceneRenderer::Create(true);

	auto path = shade::FileDialog::OpenFile("Shade scene(*.scene) \0*.scene\0");
	if (!path.empty())
	{
		shade::File file(path.string(), shade::File::In, "@s_scene", shade::File::VERSION(0, 0, 1));
		if (file.IsOpen())
		{
			shade::Scene::GetActiveScene()->DestroyAllEntites();
			file.Read(shade::Scene::GetActiveScene());
		}
		else
		{
			SHADE_CORE_WARNING("Couldn't open scene file, path ={0}", path);
		}
	}
}

void GameLayer::OnUpdate(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime)
{
	shade::physic::PhysicsManager::Step(scene, deltaTime);

	//m_SceneRenderer->OnUpdate(scene, deltaTime);
}

void GameLayer::OnRenderBegin()
{
}

void GameLayer::OnRender(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime)
{
	m_SceneRenderer->OnRender(scene, deltaTime);
}

void GameLayer::OnRenderEnd()
{
}

void GameLayer::OnEvent(shade::SharedPointer<shade::Scene>& scene, const shade::Event& event, const shade::FrameTimer& deltaTime)
{
	m_SceneRenderer->OnEvent(scene, event, deltaTime);
}

void GameLayer::OnDestroy()
{
}
