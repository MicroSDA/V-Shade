#pragma once

#include <shade/core/layer/Layer.h>

class GameLayer : public shade::Layer
{
public:
	GameLayer() = default;
	virtual ~GameLayer() = default;

	// Inherited via Layer
	void OnCreate() override;
	void OnUpdate(shade::SharedPointer<shade:: Scene>& scene, const shade::FrameTimer& deltaTime) override;
	void OnRenderBegin() override;
	void OnRender(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime) override;
	void OnRenderEnd() override;
	void OnEvent(shade::SharedPointer<shade::Scene>& scene, const shade::Event& event, const shade::FrameTimer& deltaTime) override;
	void OnDestroy() override;

private:
	shade::SharedPointer<shade::SceneRenderer>  m_SceneRenderer;
};

