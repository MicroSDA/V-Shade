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

void PlayerScript::OnCreate()
{
}

void PlayerScript::OnDestroy()
{

}

void PlayerScript::OnUpdate(const shade::FrameTimer& deltaTime)
{
	if (HasComponent<shade::AnimationGraphComponent>())
	{
		auto& graph = GetComponent<shade::AnimationGraphComponent>();

		m_State = State::Idle;

		if (shade::Input::IsKeyPressed(shade::Key::W))
			m_State = State::WalkForward;
		if (shade::Input::IsKeyPressed(shade::Key::W) && shade::Input::IsKeyPressed(shade::Key::LeftShift))
			m_State = State::Run;
		if (shade::Input::IsKeyPressed(shade::Key::S))
			m_State = State::WalkBackwards;
		if (shade::Input::IsKeyPressed(shade::Key::Space))
			m_State = State::Jump;

		graph.AnimationGraph->SetInputValue("Direct state", (int)m_State);
	}

	if (HasComponent<shade::TransformComponent>() && HasComponent<shade::CameraComponent>())
	{
		auto& camera = GetComponent<shade::CameraComponent>();
		auto& transform = GetComponent<shade::TransformComponent>();

		//// Управление вращением
		if (true)
		{
			shade::Input::ShowMouseCursor(true);

			glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
			glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

			// Вращение камеры и персонажа вокруг оси Y (вертикальная ось)
			camera->Rotate(glm::vec3(mousePosition, 0.0f) * 0.5f / 1000.f);

			// Обновляем направление персонажа в соответствии с направлением камеры
			glm::vec3 forwardDirection = glm::normalize(camera->GetForwardDirection());
			float yaw = glm::atan(forwardDirection.x, forwardDirection.z);
			transform.SetRotation(glm::vec3(0, yaw, 0));

			// Перемещение персонажа по направлению взгляда камеры
			glm::vec3 moveDirection = glm::normalize(glm::mat3(transform.GetModelMatrix()) * glm::vec3(0.f, 0.f, 1.f));
				
				
			//glm::normalize(glm::vec3(transform.GetRotation().x, transform.GetRotation().y, transform.GetRotation().z)); // Убираем вертикальное смещение
			float moveSpeed = 0.005f; // Скорость перемещения

			if (m_State == State::WalkForward)
			{
				transform.Move(glm::vec3(moveDirection.x * m_MovementSpeed * deltaTime.GetInSeconds<float>(), 0.f, moveDirection.z * m_MovementSpeed * deltaTime.GetInSeconds<float>()));
			}
			if (m_State == State::WalkBackwards)
			{
				transform.Move(glm::vec3(-moveDirection.x * m_MovementSpeed * deltaTime.GetInSeconds<float>(), 0.f, -moveDirection.z * m_MovementSpeed * deltaTime.GetInSeconds<float>()));
			}
			if (m_State == State::Run)
			{
				transform.Move(glm::vec3(moveDirection.x * m_MovementSpeed * 2.f * deltaTime.GetInSeconds<float>(), 0.f, moveDirection.z * m_MovementSpeed * 2.f * deltaTime.GetInSeconds<float>()));
			}


			// Позиция камеры с учётом поворота персонажа
			glm::vec3 cameraOffset = { 0.0f, 1.0f, 5.0f }; // Смещение камеры (позади и выше персонажа) 
			glm::vec3 cameraPosition = transform.GetPosition() - forwardDirection * cameraOffset.z + glm::vec3(0, cameraOffset.y, 0);
			camera->SetPosition(cameraPosition);

			shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);

			
		}
		else
		{
			shade::Input::ShowMouseCursor(true);
		}
	}


	//glm::vec3 moveDirection(0.0f);

	//// Управление движением
	//if (shade::Input::IsKeyPressed(shade::Key::W))
	//	moveDirection += camera->GetForwardDirection() * m_MovementSpeed * deltaTime;
	//if (shade::Input::IsKeyPressed(shade::Key::S))
	//	moveDirection -= camera->GetForwardDirection() * m_MovementSpeed * deltaTime;
	//if (shade::Input::IsKeyPressed(shade::Key::A))
	//	moveDirection -= camera->GetForwardDirection() * m_MovementSpeed * deltaTime;
	//if (shade::Input::IsKeyPressed(shade::Key::D))
	//	moveDirection += camera->GetForwardDirection() * m_MovementSpeed * deltaTime;

	//if (shade::Input::IsKeyPressed(shade::Key::W))
	//	moveDirection += camera->GetForwardDirection() * deltaTime.GetInSeconds<float>();
	//if (shade::Input::IsKeyPressed(shade::Key::S))
	//	moveDirection -= camera->GetForwardDirection() * deltaTime.GetInSeconds<float>();


	//transform.Move(moveDirection);

	//// Управление вращением
	//if (shade::Input::IsMouseButtonPressed(shade::Mouse::ButtonRight))
	//{
	//	shade::Input::ShowMouseCursor(false);
	//	glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
	//	glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

	//	// Вращение камеры и, соответственно, персонажа вокруг оси Y (вертикальная ось)
	//	camera->Rotate(glm::vec3(mousePosition, 0.0f) * 0.05f / 1000.f);

	//	// Обновляем позицию и направление камеры, чтобы она оставалась позади персонажа
	//	glm::vec3 cameraOffset = { -3.0f, 2.0f, -5.0f }; // Задайте смещение камеры
	//	glm::vec3 cameraPosition = transform.GetPosition() + glm::vec3(0, 1, 0) + cameraOffset;
	//	camera->SetPosition(cameraPosition);
	//	camera->SetDirection(transform.GetPosition() - cameraPosition);

	//	shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);
	//}
	//else
	//{
	//	shade::Input::ShowMouseCursor(true);
	//}

	//
	//// Управление движением персонажа
	//if (HasComponent<shade::TransformComponent>() && HasComponent<shade::CameraComponent>())
	//{
	//	auto& transform = GetComponent<shade::TransformComponent>();
	//	auto& camera = GetComponent<shade::CameraComponent>();

	//	glm::vec3 moveDirection(0.0f);

	//	if (shade::Input::IsKeyPressed(shade::Key::W))
	//		moveDirection += camera->GetForwardDirection() *  deltaTime.GetInSeconds<float>();
	//	if (shade::Input::IsKeyPressed(shade::Key::S))
	//		moveDirection -= camera->GetForwardDirection() * deltaTime.GetInSeconds<float>();
	//	/*if (shade::Input::IsKeyPressed(shade::Key::A))
	//		moveDirection -= camera->RotateY() * m_MovementSpeed * deltaTime;
	//	if (shade::Input::IsKeyPressed(shade::Key::D))
	//		moveDirection += camera->GetRight() * m_MovementSpeed * deltaTime;*/

	//	transform.Rotate(moveDirection);
	//	//camera->MoveForward(moveDirection.z);

	//	// Вращение персонажа в сторону движения камеры
	//	glm::vec3 forwardDirection = glm::normalize(camera->GetForwardDirection());
	//	transform.SetRotation(glm::vec3(0, glm::atan(forwardDirection.x, forwardDirection.z), 0));
	//}
}

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera()
{
	return new FreeCamera();
}

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Player()
{
	return new PlayerScript();
}