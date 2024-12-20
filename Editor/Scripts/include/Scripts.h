#pragma once
#define SHADE_SCRIPT
#include <shade/core/scripting/Scripting.h>
#include <shade/core/animation/Pose.h>
#include <glm/glm/glm.hpp>

class PlayerScript : public shade::ecs::ScriptableEntity
{
public:
	void OnCreate();
	void OnDestroy();
	void OnUpdate(const shade::FrameTimer& deltaTime);
private:
	enum State : int
	{
		Idle = 0,
		WalkForward = 1,
		Run = 2,
		WalkBackwards = 3,
		Jump = 4

	} m_State = State::Idle;

	glm::vec2 m_Velocity				= glm::vec2(0.f);
	glm::vec3 m_CameraSensativity		= glm::vec3(0.5f);

	shade::animation::Pose::RootMotion	m_RootMotion;
};

extern "C"
{
	SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera();
	SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Player();
}
