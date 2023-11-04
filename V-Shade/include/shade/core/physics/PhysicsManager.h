#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/time/Timer.h>
#include <shade/core/scene/Scene.h>
#include <shade/core/physics/Common.h>
#include <shade/core/physics/algo/ConvexHullGenerator.h>

namespace shade
{
	namespace physic
	{
		class SHADE_API PhysicsManager
		{
		private:
			class CashedContactData
			{
			public:
				using CollisionPairArray = StackArray<CollisionShape::Manifold, 64U>;
				struct CollisionPair
				{
					RigidBody* aBody;
					RigidBody* bBody;
					CollisionPairArray Contancts;
				};
			public:
				CashedContactData() = default;
				~CashedContactData() = default;

				void IntegrateContact(const CollisionShape::Manifold& manifold, const RigidBody& bodyA, const RigidBody& bodyB);
				StackArray<CollisionShape::Manifold, 4u> GetReducedContacts(const RigidBody& bodyA, const RigidBody& bodyB);

				void Clear();
			
				std::unordered_map<std::size_t, CollisionPair> m_Pairs;
			};
		public:
			PhysicsManager() = default;
			~PhysicsManager() = default;

			static void Init();
			static void ShutDown();
			static void Step(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
			static void SetSimulationPlaying(bool isPlay);
			static void SetIterationCount(std::size_t count);
		private:
			static void Integrate(RigidBody& body, Transform& transform, scalar_t deltaTime, scalar_t deltaDT);
			static void IntegrateContact(const CollisionShape::Manifold& contact, const RigidBody& bodyA, const RigidBody& bodyB);
			static StackArray<CollisionShape::Manifold, 4u> GetStableContacts(const RigidBody& bodyA, const RigidBody& bodyB);
			static void DetectCollisions(ecs::BasicView<ecs::EntityID, RigidBodyComponent, TransformComponent>& bodies, scalar_t deltaTime);
			static void PositionSolver(const CollisionShape::Manifold& contact, const std::pair<RigidBody&, TransformComponent&>& bodyA, const std::pair<RigidBody&, TransformComponent&>& bodyB, scalar_t deltaTime);
			static void ImpulseSolver(const StackArray<CollisionShape::Manifold, 4>& contacts, const std::pair<RigidBody&, TransformComponent&>& bodyA, const std::pair<RigidBody&, TransformComponent&>& bodyB, scalar_t deltaTime);

			static CashedContactData m_ContactsData;
			static std::size_t m_IterationCount;
			static scalar_t deltaDT;
			static bool m_IsSimulating;
		private:
			

			template<typename T, typename... Args>
			static void AddIntoSolver(Args&&... args)
			{
				T::AddNew(std::forward<Args>(args)...);
			}
			template<typename T, typename... Args>
			static void Solve(Args&&... args)
			{
				T::Solve(std::forward<Args>(args)...);
			}
		};
	}
}
