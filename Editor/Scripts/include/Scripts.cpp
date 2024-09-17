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
			camera->RotatePitch(m_RotationSpeed / 100.f * deltaTime);
		if (shade::Input::IsKeyPressed(shade::Key::E))
			camera->RotatePitch(-m_RotationSpeed / 100.f * deltaTime);

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
float NormalizeAngle(float angle) 
{
	angle = fmod(angle + 180.0f, 360.0f);
	if (angle < 0.0f)
		angle += 360.0f;
	return angle - 180.0f;
}

void PlayerScript::OnUpdate(const shade::FrameTimer& deltaTime)
{
	if (HasComponent<shade::AnimationGraphComponent>() && GetComponent<shade::AnimationGraphComponent>().AnimationGraph) 
	{
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

		graph.AnimationGraph->SetInputValue("State", static_cast<int>(rState));
		graph.AnimationGraph->SetInputValue("Direction", m_Velocity);

		m_Motion = graph.AnimationGraph->GetOutputPose()->GetRootMotionTranslationDifference();
		m_Rotation = graph.AnimationGraph->GetOutputPose()->GetRootMotionRotationDifference();

		SHADE_CORE_INFO("Translation {0},{1},{2}", m_Motion.x, m_Motion.y, m_Motion.z);
	}

	if (HasComponent<shade::TransformComponent>())
	{
		//auto& camera		= GetComponent<shade::CameraComponent>();
		auto& transform		= GetComponent<shade::TransformComponent>();

		//----------------------------------------------------------------------------------
		// Camera section
		//----------------------------------------------------------------------------------
		{
			// Get center of the screen and mouse position on the screen
			glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
			glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

			//shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);

			// Rotate camera around the Y-axis
			//camera->Rotate(glm::vec3(mousePosition, 0.0f) * m_CameraSensativity / 1000.f);
			//camera->SetForwardDirection(glm::eulerAngles(glm::conjugate(m_Rotation))); 
			

			// Update camera position based on character's position and direction

			//transform.Move(glm::vec3{ 0, 0.f, -m_Motion.x });
			transform.Move(glm::vec3{ m_Motion.x, m_Motion.y, m_Motion.z });  
			transform.Rotate(m_Rotation);
			//camera->SetForwardDirection(transform.GetForwardDirection());
			
			glm::vec3 cameraOffset = { 0.0f, 1.0f, 5.0f }; // Offset the camera behind and above the character
			//glm::vec3 cameraPosition = transform.GetPosition() - camera->GetForwardDirection() * cameraOffset.z + glm::vec3(0, cameraOffset.y, 0);

			//camera->SetPosition(cameraPosition);
		}
		//----------------------------------------------------------------------------------
		// Character section
		//----------------------------------------------------------------------------------
		//{
		//	// Update character's direction based on camera's forward direction
		//	
		//	const float cYaw = camera->GetYaw(), tYaw	= transform.GetYaw();

		//	const float angularDifference				= glm::clamp(glm::radians(NormalizeAngle(glm::degrees(cYaw - tYaw) * 2.f)), -2.0f, 2.0f);  // In degrees
		//	
	
		//	//transform.SetRotation(glm::vec3(0.f, cYaw, 0.f));
		//	

		//	float smoothingFactor = 0.07f;

		//	m_Velocity.x += (-angularDifference * deltaTime.GetInMilliseconds<float>() - m_Velocity.x) * smoothingFactor;
		//}
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