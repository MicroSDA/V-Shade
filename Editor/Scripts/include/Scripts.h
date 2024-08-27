#pragma once

#define SHADE_SCRIPT
#include <shade/core/scripting/Scripting.h>
#include <glm/glm/glm.hpp>

class FreeCamera : public shade::ecs::ScriptableEntity
{
public:
	void OnCreate();
	void OnDestroy();
	void OnUpdate(const shade::FrameTimer& deltaTime);
private:
	float m_RotationSpeed = 0.3f;
	float m_MovementSpeed = 0.05f;

};

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
	float m_MovementSpeed = 1.0f;
	glm::mat4 m_RootMotion;
};

extern "C"
{
	SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera();
	SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Player();
}
