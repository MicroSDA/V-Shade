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
	if (HasComponent<shade::AnimationGraphComponent>()) 
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

		if (graph.AnimationGraph)
		{
			graph.AnimationGraph->SetInputValue("State", static_cast<int>(rState)); 
			graph.AnimationGraph->SetInputValue("Direction", m_Velocity);  

			if (const shade::animation::Pose* finalPose = graph.AnimationGraph->GetOutputPose())
			{				
				m_RootMotion = finalPose->GetRootMotion();
			}
		}
		
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

			transform.Rotate(m_RootMotion.Rotation.Difference); 
			// Получаем текущий forward direction
			glm::vec3 forward = transform.GetForwardDirection();
			glm::vec3 baseDirection = glm::normalize(glm::eulerAngles(m_RootMotion.Rotation.Difference)); // базовое направление (нулевая точка)
			glm::vec3 translationDir = glm::normalize(m_RootMotion.Translation.Difference); // направление, которое нужно повернуть

			// Находим кватернион, который вращает базовое направление в направление forward
			glm::quat rotationToForward = glm::rotation(baseDirection, forward);

			// Применяем этот кватернион к вектору направления трансляции
			glm::vec3 adjustedTranslation = rotationToForward * translationDir * glm::length(m_RootMotion.Translation.Difference);

			// Применяем скорректированный вектор к transform
			transform.Move(adjustedTranslation);

			glm::vec3 dir = glm::degrees(transform.GetForwardDirection());
			std::cout << "X " << adjustedTranslation.x<< " Y " << adjustedTranslation.y << " Z " << adjustedTranslation.z << std::endl;
			//GetEntity().GetManager().View<shade::TransformComponent>().Each([&](shade::ecs::Entity& entity, shade::TransformComponent& tr)
			//	{
			//		if (!entity.HasComponent<shade::AnimationGraphComponent>())
			//		{
			//			if (HasComponent<shade::AnimationGraphComponent>())
			//			{
			//				if (auto pose = GetComponent<shade::AnimationGraphComponent>().AnimationGraph->GetOutputPose())
			//				{
			//					if (auto bone = pose->GetSkeleton()->GetBone(8))
			//					{
			//						tr.SetTransformMatrix(pose->GetBoneGlobalTransform(bone->ID).Transform);
			//					}
			//				}
			//			}
			//			//entity.GetComponent<shade::TransformComponent>().SetDirection(transform.GetRotationQuaternion() * glm::vec3(0, 0, -1));
			//		}
			//	});
			
			
			/*for (auto& Endity : *this)
			{
				
			}*/
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

SHADE_SCRIPT_API shade::ecs::ScriptableEntity* Player()
{
	return new PlayerScript();
}