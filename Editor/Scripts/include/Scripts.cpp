#include "shade_pch.h"
#include "Scripts.h"
#include <shade/core/event/Input.h>
#include <shade/core/application/Application.h>

extern "C"
{
    SHADE_SCRIPT_API ScriptPack InstantiatePack()
    {
        // TODO: 
        return ScriptPack();
    }
}

void FreeCamera::OnCreate()
{
   
}

void FreeCamera::OnDestroy()
{
  
}

void FreeCamera::OnUpdate(const shade::FrameTimer& deltaTime)
{
	auto camera = GetComponent<shade::CameraComponent>();
	{
		// Movment
		if (shade::Input::IsKeyPressed(shade::Key::W))
			camera->MoveForward(m_MovementSpeed * deltaTime);
		if (shade::Input::IsKeyPressed(shade::Key::S))
			camera->MoveBackward(m_MovementSpeed * deltaTime);

		if (shade::Input::IsKeyPressed(shade::Key::A))
			camera->MoveLeft(m_MovementSpeed * deltaTime);
		if (shade::Input::IsKeyPressed(shade::Key::D))
			camera->MoveRight(m_MovementSpeed * deltaTime);

		if (shade::Input::IsKeyPressed(shade::Key::Q))
			camera->RotateZ(m_RotationSpeed / 100.f * deltaTime);
		if (shade::Input::IsKeyPressed(shade::Key::E))
			camera->RotateZ(-m_RotationSpeed / 100.f * deltaTime);

	}
	{
		if (shade::Input::IsMouseButtonPressed(shade::Mouse::ButtonRight))
		{
			shade::Input::ShowMouseCursor(false);
			glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
			glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

			camera->Rotate(glm::vec3(mousePosition, 0.0f) * m_RotationSpeed / 1000.f);

			shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);
		}
		else
			shade::Input::ShowMouseCursor(true);

	}
}

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera()
{
    return new FreeCamera();
}
