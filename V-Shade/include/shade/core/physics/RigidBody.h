#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/physics/shapes/MeshShape.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/core/physics/Common.h>
#include <shade/core/transform/Transform.h>

namespace shade
{
	namespace physic
	{
		class SHADE_API RigidBody
		{
		public:
			enum class Type : std::size_t
			{
				Dynamic = 0,
				Static = 1,
			};
			RigidBody() = default;
			virtual ~RigidBody() = default;

			void SetBodyType(Type type);
			Type GetBodyType() const;

			//void AddCollider(const SharedPointer<CollisionShape>& collider); 
			void AddCollider(const Asset<CollisionShapes>& collider);

			std::size_t GetCollidersCount() const;
			CollisionShape::Manifold TestCollision(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const;

			bool AABB_X_AABB(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const;
			bool OBB_X_OBB(const glm::mat<4, 4, physic::scalar_t>& transform, const RigidBody& other, const glm::mat<4, 4, physic::scalar_t>& otherTransform) const;

			void ApplayGravity(const glm::vec<3, physic::scalar_t>& gravity);
			void ApplyForce(const glm::vec<3, physic::scalar_t>& force, const glm::vec<3, physic::scalar_t>& position = glm::vec<3, physic::scalar_t>(0.0));
			void ApplyTorque(const glm::vec<3, physic::scalar_t>& torque);

			void ApplyLinearImpulse(const glm::vec<3, scalar_t>& impulse);
			void ApplyAngularImpulse(const glm::vec<3, scalar_t>& impulse, const glm::vec<3, scalar_t>& position);

			void SetSleep(bool sleep);
			bool IsSleep() const;
			void SetIntertiaTensor(const glm::vec<3, physic::scalar_t>& scale);
			const glm::mat<3, 3, scalar_t>& GetIntertiaTensor() const;
			physic::scalar_t GetInverseMass() const;

			const shade::StackArray<glm::vec<3, scalar_t>, 4>& GetCollisionContacts() const;
			const Asset<CollisionShapes>& GetCollisionShapes() const;
			Asset<CollisionShapes>& GetCollisionShapes();
			const std::vector<HalfExtensions>& GetExtensions() const;
			std::vector<HalfExtensions>& GetExtensions();
			
		public:
			std::vector<SharedPointer<CollisionShape>>::iterator begin() { return m_CollisionShapes->GetColliders().begin(); }
			std::vector<SharedPointer<CollisionShape>>::iterator end()   { return m_CollisionShapes->GetColliders().end(); }

			std::vector<SharedPointer<CollisionShape>>::const_iterator begin() const { return m_CollisionShapes->GetColliders().begin(); }
			std::vector<SharedPointer<CollisionShape>>::const_iterator end() const { return m_CollisionShapes->GetColliders().end(); }

			operator bool() { return m_Type != Type::Static; }
		public: 
			glm::vec<3, physic::scalar_t> LinearVelocity        = glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> AngularVelocity		= glm::vec<3, physic::scalar_t>(0.0);

			physic::scalar_t Mass = 10.0;
			physic::scalar_t StaticFriction = 0.5;		
			physic::scalar_t Restitution	= 0.5; 
			physic::scalar_t LinearDamping	= 0.95;
			physic::scalar_t AngularDamping = 0.8;

		private:
			glm::vec<3, physic::scalar_t> NetForce				= glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> NetTorque				= glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> DeltaLinearVelocity	= glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> DeltaAngularVelocity	= glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> DeltaPosition			= glm::vec<3, physic::scalar_t>(0.0);
			glm::vec<3, physic::scalar_t> DeltaRotation			= glm::vec<3, physic::scalar_t>(0.0);

			glm::mat<3, 3 ,physic::scalar_t> InertiaTensor	    = glm::mat<3, 3, physic::scalar_t>(1.0);
		
			shade::StackArray<glm::vec<3, scalar_t>, 4>		m_CollisionContancts;
			void ClearForces();
		private:
			void Integrate(Transform& transform, scalar_t deltaTime, scalar_t deltaDT);
			bool ShouldSleep(const Transform& transform, scalar_t deltaTime, scalar_t deltaDT);
			void UpdateIntertiaTensor(const glm::qua<scalar_t>& rotate);
			void UpdateIntertiaTensor(const glm::vec<3, scalar_t>& rotate);
		private:
			Type m_Type = Type::Static;

			Asset<CollisionShapes> m_CollisionShapes;
			std::vector<HalfExtensions> m_Extensions;

			scalar_t m_TimeToSleep = 0.0;
			bool m_IsSleep = false;
			friend class PhysicsManager;
		private:
			friend class serialize::Serializer;
			
			void Serialize(std::ostream& stream) const;

			
			void Deserialize(std::istream& stream);
		};
	}

	
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const physic::RigidBody& body)
	{
		body.Serialize(stream);
	}
	
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, physic::RigidBody& body)
	{
		body.Deserialize(stream);
	}
}
