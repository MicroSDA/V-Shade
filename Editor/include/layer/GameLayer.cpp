#include "shade_pch.h"
#include "GameLayer.h"
#include <shade/core/system/FileDialog.h>

void GameLayer::OnCreate()
{
	m_SceneRenderer = shade::SceneRenderer::Create(true);

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
