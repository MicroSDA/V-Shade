#pragma once

#define SHADE_SCRIPT
#include <shade/core/scripting/Scripting.h>

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

extern "C"
{
	SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera();
}
