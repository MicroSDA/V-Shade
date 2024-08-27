#include "shade_pch.h"
#include "Scripts.h"
#include <shade/core/event/Input.h>
#include <shade/core/application/Application.h>

//extern "C"
//{
//	SHADE_SCRIPT_API ScriptPack InstantiatePack()
//	{
//		// TODO: 
//		return ScriptPack();
//	}
//}

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
	if (HasComponent<shade::AnimationGraphComponent>()) {
		auto& graph = GetComponent<shade::AnimationGraphComponent>();

		State rState = State::Idle;

		// Determine the desired state based on input
		if (shade::Input::IsKeyPressed(shade::Key::W)) 
		{
			rState = (shade::Input::IsKeyPressed(shade::Key::LeftShift)) ? State::Run : State::WalkForward;
		}
		else if (shade::Input::IsKeyPressed(shade::Key::S)) 
		{
			rState = State::WalkBackwards;
		}
		else if (shade::Input::IsKeyPressed(shade::Key::Space)) 
		{
			rState = State::Jump;
		}

		graph.AnimationGraph->SetInputValue("Direct state", static_cast<int>(rState));

		if (auto pNode = graph.AnimationGraph->GetGraphContext()->FindNode(2259433077)) 
		{
			const auto name = pNode->GetRootNode()->GetName();

			m_RootMotion = pNode->GetRootNode()->As<shade::animation::state_machine::StateNode>().GetOutputPose()->ComputeRootMotion();
				
			// Update the current state based on the node name
			if (name == "Idle") m_State = State::Idle;
			else if (name == "Walk-forward") m_State = State::WalkForward;
			else if (name == "Walk-backwoods") m_State = State::WalkBackwards;
			else if (name == "Running") m_State = State::Run;
			else if (name == "Jump") m_State = State::Jump;
		}
	}

	if (HasComponent<shade::TransformComponent>() && HasComponent<shade::CameraComponent>()) {
		auto& camera = GetComponent<shade::CameraComponent>();
		auto& transform = GetComponent<shade::TransformComponent>();

		shade::Input::ShowMouseCursor(false);

		glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
		glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

		// Rotate camera and character around the Y-axis
		camera->Rotate(glm::vec3(mousePosition, 0.0f) * 0.5f / 1000.f);

		// Update character's direction based on camera's forward direction
		glm::vec3 forwardDirection = glm::normalize(camera->GetForwardDirection());
		float yaw = glm::atan(forwardDirection.x, forwardDirection.z);
		transform.SetRotation(glm::vec3(0, yaw, 0));

		// Move character based on camera direction
		glm::vec3 moveDirection = glm::normalize(glm::mat3(transform.GetModelMatrix()) * glm::vec3(0.f, 0.f, 1.f));

		// Adjust character movement speed based on state
		float moveSpeedMultiplier = 1.0f;
		if (m_State == State::Run) moveSpeedMultiplier = 2.0f;

		if (m_State == State::WalkForward || m_State == State::WalkBackwards || m_State == State::Run) {
			glm::vec3 movement = moveDirection * m_MovementSpeed * moveSpeedMultiplier * deltaTime.GetInSeconds<float>();
			if (m_State == State::WalkBackwards) movement = -movement;

			transform.Move(glm::vec3(movement.x, 0.f, movement.z));
		}

		// Update camera position based on character's position and direction
		glm::vec3 cameraOffset = { 0.0f, 1.0f, 5.0f }; // Offset the camera behind and above the character
		glm::vec3 cameraPosition = transform.GetPosition() - forwardDirection * cameraOffset.z + glm::vec3(0, cameraOffset.y, 0);
		camera->SetPosition(cameraPosition);

		shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);
	}
}

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Camera()
{
	return new FreeCamera();
}

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Player()
{
	return new PlayerScript();
}