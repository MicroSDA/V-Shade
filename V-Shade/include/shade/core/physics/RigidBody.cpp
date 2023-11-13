#include "shade_pch.h"
#include "RigidBody.h"
#include <shade/utils/Logger.h>
#include <shade/core/asset/AssetManager.h>


#define P_S_TRESHOLD 0.00005
#define R_S_TRESHOLD 0.00005

void shade::physic::RigidBody::SetBodyType(Type type)
{
	m_Type = type;
}

shade::physic::RigidBody::Type shade::physic::RigidBody::GetBodyType() const
{
	return m_Type;
}

void shade::physic::RigidBody::AddCollider(const Asset<CollisionShapes>& collider)
{
	m_CollisionShapes = collider;
	m_Extensions.clear();
	for (const auto& colider : m_CollisionShapes->GetColliders())
	{
		m_Extensions.emplace_back(
			HalfExtensions{ .MinHalfExt = colider->GetMinHalfExt(),
				.MaxHalfExt = colider->GetMaxHalfExt(),
				.MinHalfExtWorldSpace = colider->GetMinHalfExt(),
				.MaxHalfExtWorldSpace = colider->GetMaxHalfExt() });
	}
}

std::size_t shade::physic::RigidBody::GetCollidersCount() const
{
	if (m_CollisionShapes)
		return m_CollisionShapes->GetColliders().size();
	else
		return 0;
}

shade::physic::CollisionShape::Manifold shade::physic::RigidBody::TestCollision(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const
{
	std::vector<CollisionShape::Manifold> manifolds;

	if (m_CollisionShapes && other.m_CollisionShapes)
	{
		for (const auto& colliderA : m_CollisionShapes->GetColliders())
		{
			for (const auto& colliderB : other.m_CollisionShapes->GetColliders())
			{
				auto result = colliderA->TestCollision(transform, *colliderB, otherTransform);
				if (result.HasCollision)
				{
					manifolds.emplace_back(result);
				}

			}
		}
	}
	
	return CollisionShape::Manifold::GetMaxPenetration(manifolds);
}

bool shade::physic::RigidBody::AABB_X_AABB(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const
{
	if (m_CollisionShapes && other.m_CollisionShapes)
	{
		for (const auto& extA : m_Extensions)
		{
			for (const auto& extB : other.m_Extensions)
			{
				if (extA.AABB_X_AABB(extB))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

bool shade::physic::RigidBody::OBB_X_OBB(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const
{
	if (m_CollisionShapes && other.m_CollisionShapes)
	{
		for (const auto& extA : m_Extensions)
		{
			for (const auto& extB : other.m_Extensions)
			{
				if (extA.OBB_X_OBB(extB))
				{
					return true;
				}
			}
		}
	}
	return false;
}

void shade::physic::RigidBody::ApplayGravity(const glm::vec<3, physic::scalar_t>& gravity)
{
	// If body is dynamic.
	if (*this) ApplyForce(gravity);
}

void shade::physic::RigidBody::ApplyForce(const glm::vec<3, physic::scalar_t>& force, const glm::vec<3, physic::scalar_t>& position)
{
	NetForce += force;
	ApplyTorque(glm::cross<physic::scalar_t>(position, force));
}

void shade::physic::RigidBody::ApplyTorque(const glm::vec<3, physic::scalar_t>& torque)
{
	NetTorque += torque;
}

void shade::physic::RigidBody::ApplyLinearImpulse(const glm::vec<3, scalar_t>& impulse)
{
	NetForce += impulse;
}

void shade::physic::RigidBody::ApplyAngularImpulse(const glm::vec<3, scalar_t>& impulse, const glm::vec<3, scalar_t>& position)
{
	NetTorque += glm::cross<scalar_t>(position, impulse);
}

void shade::physic::RigidBody::SetSleep(bool sleep)
{
	m_IsSleep = sleep;
}

bool shade::physic::RigidBody::IsSleep() const
{
	return m_IsSleep;
}

shade::physic::scalar_t shade::physic::RigidBody::GetInverseMass() const
{
	return physic::scalar_t(1.0) / Mass;
}

const shade::StackArray<glm::vec<3, shade::physic::scalar_t>, 4>& shade::physic::RigidBody::GetCollisionContacts() const
{
	return m_CollisionContancts;
}

const shade::Asset<shade::physic::CollisionShapes>& shade::physic::RigidBody::GetCollisionShapes() const
{
	return m_CollisionShapes;
}

shade::Asset<shade::physic::CollisionShapes>& shade::physic::RigidBody::GetCollisionShapes()
{
	return m_CollisionShapes;
}

const std::vector<shade::physic::HalfExtensions>& shade::physic::RigidBody::GetExtensions() const
{
	return m_Extensions;
}

std::vector<shade::physic::HalfExtensions>& shade::physic::RigidBody::GetExtensions()
{
	return m_Extensions;
}

const glm::mat<3, 3, shade::physic::scalar_t>& shade::physic::RigidBody::GetIntertiaTensor() const
{
	return InertiaTensor;
}

void shade::physic::RigidBody::SetIntertiaTensor(const glm::vec<3, scalar_t>& scale)
{
	glm::vec<3, scalar_t> dimensions = scale;
	glm::vec<3, scalar_t> dimsSqr = dimensions * dimensions;

	InertiaTensor = glm::mat<3,3, scalar_t>((12.0 * GetInverseMass()) / (dimsSqr.y + dimsSqr.z), 0.0, 0.0,
											0.0, (12.0 * GetInverseMass()) / (dimsSqr.x + dimsSqr.z), 0.0,
											0.0, 0.0, (12.0 * GetInverseMass()) / (dimsSqr.x + dimsSqr.y));

	/*InertiaTensor = glm::mat<3, 3, scalar_t>((1.0 / 12.0) * GetInverseMass() * (dimsSqr.y + dimsSqr.z), 0.0, 0.0,
											  0.0, (1.0 / 12.0) * GetInverseMass() * (dimsSqr.x + dimsSqr.z), 0.0,
											  0.0, 0.0, (1.0 / 12.0) * GetInverseMass() * (dimsSqr.x + dimsSqr.y));*/
}

void shade::physic::RigidBody::UpdateIntertiaTensor(const glm::qua<scalar_t>& rotate)
{
	glm::mat<3, 3, scalar_t> invOrientation = glm::mat<3, 3, scalar_t>(glm::conjugate(rotate));
	glm::mat<3, 3, scalar_t> orientation = glm::mat<3, 3, scalar_t>(rotate);
	//InertiaTensor = orientation * invOrientation;
}
void shade::physic::RigidBody::UpdateIntertiaTensor(const glm::vec<3, scalar_t>& rotate)
{
	UpdateIntertiaTensor(glm::qua<scalar_t>(rotate));
}

std::size_t shade::physic::RigidBody::SerializeAsComponent(std::ostream& stream) const
{
	std::uint32_t size = 0u;
	size += Serializer::Serialize(stream, static_cast<std::uint32_t>(GetBodyType()));
	size += Serializer::Serialize(stream, static_cast<float>(Mass));
	size += Serializer::Serialize(stream, static_cast<float>(StaticFriction));
	size += Serializer::Serialize(stream, static_cast<float>(Restitution));
	size += Serializer::Serialize(stream, static_cast<float>(LinearDamping));
	size += Serializer::Serialize(stream, static_cast<float>(AngularDamping));

	std::string assetId;
	if (m_CollisionShapes)
		assetId = m_CollisionShapes->GetAssetData()->GetId();

	size += Serializer::Serialize(stream, assetId);
	return size;
}

std::size_t shade::physic::RigidBody::DeserializeAsComponent(std::istream& stream)
{
	float mass, staticFriction, restitution, linearDamping, angularDamping;
	std::uint32_t size = 0u, bodyType;
	size += Serializer::Deserialize(stream, bodyType);
	size += Serializer::Deserialize(stream, mass);
	size += Serializer::Deserialize(stream, staticFriction);
	size += Serializer::Deserialize(stream, restitution);
	size += Serializer::Deserialize(stream, linearDamping);
	size += Serializer::Deserialize(stream, angularDamping);
	Mass = mass, StaticFriction = staticFriction, Restitution = restitution, LinearDamping = linearDamping, AngularDamping = angularDamping;
	SetBodyType(static_cast<RigidBody::Type>(bodyType));

	std::string assetId;
	size += Serializer::Deserialize(stream, assetId);

	if (!assetId.empty())
	{
		AssetManager::GetAsset<CollisionShapes, shade::BaseAsset::InstantiationBehaviour::Aynchronous>(assetId, shade::AssetMeta::Category::Secondary, shade::BaseAsset::LifeTime::DontKeepAlive, [&](auto& asset) mutable
			{
				AddCollider(asset);
			});

	}

	return size;
}

void shade::physic::RigidBody::Integrate(Transform& transform, scalar_t deltaTime, scalar_t deltaDT)
{
	assert(deltaTime > 0.0);

	if (m_CollisionShapes)
	{
		for (auto& ext : m_Extensions)
		{
			ext.UpdateCorners(transform.GetModelMatrix());
		}

		/*for (auto& collider : m_CollisionShapes->GetColliders())
		{
			collider->UpdateCorners(transform.GetModelMatrix());
		}*/

		if (!*this)
		{
			LinearVelocity *= 0.0;
			AngularVelocity *= 0.0;
			return;
		}

		// Save current transform
		SetIntertiaTensor(transform.GetScale());
		////////////////////////////////////////////

		// Need to have delta inertia 
		//UpdateIntertiaTensor(currentRotation);

		// Update Linear velocity
		LinearVelocity += NetForce * GetInverseMass() * deltaTime;
		// Update Angular velocity
		AngularVelocity += NetTorque * GetIntertiaTensor() * deltaTime;

		LinearVelocity *= glm::pow<scalar_t>(LinearDamping, deltaTime);
		AngularVelocity *= glm::pow<scalar_t>(AngularDamping, deltaTime);

		glm::qua<scalar_t> rotate = glm::angleAxis(
			glm::length<3, scalar_t>(AngularVelocity),
			glm::length<3, scalar_t>(AngularVelocity) == 0.0 ? glm::vec<3, scalar_t>(0.0, 0.0, 1.0) : glm::normalize<3, scalar_t>(AngularVelocity)
		);

		//////////////////////////////////////////////
		Transform predictTransform = transform;

		glm::qua<scalar_t> currentRotation(predictTransform.GetRotation());
		glm::qua<scalar_t> newRotation((rotate * currentRotation));

		predictTransform.SetRotation(glm::eulerAngles<scalar_t>(newRotation));
		predictTransform.Move(glm::vec<3, scalar_t>(LinearVelocity));

		// We need to check delta time with las not iteration time, with global 
		const bool shouldSleep = ShouldSleep(predictTransform, deltaTime, deltaDT);

		// If should sleep but not sleep
		if (shouldSleep && !IsSleep())
		{
			m_TimeToSleep += deltaTime;
			DeltaPosition = predictTransform.GetPosition();
			DeltaRotation = predictTransform.GetRotation();
		}
		else if (!shouldSleep)
		{
			m_TimeToSleep = 0;

			UpdateIntertiaTensor(newRotation);

			transform = predictTransform;

			DeltaPosition = transform.GetPosition();
			DeltaRotation = transform.GetRotation();
		}

		if (m_TimeToSleep >= 1.0)
		{
			//SetSleep(true);

			//LinearVelocity *= 0;
			//AngularVelocity *= 0;
		}
		else
		{
			SetSleep(false);
		}

		DeltaLinearVelocity = LinearVelocity;
		DeltaAngularVelocity = AngularVelocity;
	}
}


void shade::physic::RigidBody::ClearForces()
{
	NetForce *= 0;
	NetTorque *= 0;
}

bool shade::physic::RigidBody::ShouldSleep(const Transform& transform, scalar_t deltaTime, scalar_t deltaDT)
{
	scalar_t rLinearVelocity  = glm::abs<scalar_t>(glm::length<3, scalar_t>(LinearVelocity)  - glm::length<3, scalar_t>(DeltaLinearVelocity));
	scalar_t rAngularVelocity = glm::abs<scalar_t>(glm::length<3, scalar_t>(AngularVelocity) - glm::length<3, scalar_t>(DeltaAngularVelocity));

	scalar_t rPosition = glm::abs<scalar_t>(glm::length<3, scalar_t>(static_cast<glm::vec<3, scalar_t>>(transform.GetPosition())) - glm::length<3, scalar_t>(DeltaPosition));
	scalar_t rRotation = glm::abs<scalar_t>(glm::length<3, scalar_t>(static_cast<glm::vec<3, scalar_t>>(transform.GetRotation())) - glm::length<3, scalar_t>(DeltaRotation));
	
	if (rPosition < P_S_TRESHOLD && rRotation < R_S_TRESHOLD)
		return true;

    return false;
}
