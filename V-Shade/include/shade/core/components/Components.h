#pragma once
#include <shade/core/scripting/NativeScript.h>
#include <shade/core/camera/Camera.h>
#include <shade/core/render/drawable/Mesh.h>
#include <shade/core/render/drawable/Model.h>
#include <shade/core/render/drawable/Material.h>
#include <shade/core/transform/Transform.h>
#include <shade/core/environment/GlobalLight.h>
#include <shade/core/environment/PointLight.h>
#include <shade/core/environment/SpotLight.h>
#include <shade/core/physics/RigidBody.h>
#include <shade/core/animation/Animator.h>
namespace shade
{
	using NativeScriptComponent = ecs::NativeScript;
	using CameraComponent = SharedPointer<Camera>;
	using TagComponent = std::string;
	using MeshComponent = Asset<Mesh>;
	using ModelComponent = Asset<Model>;
	using MaterialComponent = Asset<Material>;
	using TransformComponent = Transform;
	using GlobalLightComponent = SharedPointer<GlobalLight>;
	using PointLightComponent = SharedPointer<PointLight>;
	using SpotLightComponent = SharedPointer<SpotLight>;
	using RigidBodyComponent = physic::RigidBody;
	using AnimationStackComponent = std::unordered_map<std::string, shade::SharedPointer<shade::Animation>>;
}