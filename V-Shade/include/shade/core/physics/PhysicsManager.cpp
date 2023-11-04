#include "shade_pch.h"
#include "PhysicsManager.h"
#include <shade/utils/Utils.h>

bool shade::physic::PhysicsManager::m_IsSimulating = true;
std::size_t shade::physic::PhysicsManager::m_IterationCount = 5;
shade::physic::scalar_t shade::physic::PhysicsManager::deltaDT = 0;
shade::physic::PhysicsManager::CashedContactData shade::physic::PhysicsManager::m_ContactsData;

void shade::physic::PhysicsManager::Init()
{
}

void shade::physic::PhysicsManager::ShutDown()
{
}

void shade::physic::PhysicsManager::Step(SharedPointer<Scene>& scene, const FrameTimer& deltaTime)
{
	// https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics
	// 1. Apply forces
	// 2. Update positions and velocity
	// 3. Detect collision
	// 4. Solve collision
	scalar_t dt	= deltaTime.GetInSeconds<scalar_t>() / scalar_t(m_IterationCount);

	SetIterationCount(5);

	if (m_IsSimulating)
	{
		// If delta time lower than 1 seconds 
		if (dt < 1.0)
		{
			auto view = scene->View<RigidBodyComponent, TransformComponent>();

			// Clear contacts data
			m_ContactsData.Clear();

			for (std::size_t i = 0; i < m_IterationCount; i++)
			{
				view.Each([&](ecs::Entity& entity, RigidBodyComponent& body, TransformComponent& transform)
					{
						Integrate(body, transform, dt, deltaDT);
					});

				DetectCollisions(view, dt);
			}
			// Keep tracking delta time from previous frame
			deltaDT = dt;
		}
	}
}

void shade::physic::PhysicsManager::SetSimulationPlaying(bool isPlay)
{
	m_IsSimulating = isPlay;
}

void shade::physic::PhysicsManager::SetIterationCount(std::size_t count)
{
	m_IterationCount = count;
}

void shade::physic::PhysicsManager::Integrate(RigidBody& body, Transform& transform, scalar_t deltaTime, scalar_t deltaDT)
{
	body.ApplayGravity({ 0.0, -9.80 / scalar_t(m_IterationCount), 0.0 });
	body.Integrate(transform, deltaTime, deltaDT);
	body.ClearForces();
}

void shade::physic::PhysicsManager::IntegrateContact(const CollisionShape::Manifold& contact, const RigidBody& bodyA, const RigidBody& bodyB)
{
	m_ContactsData.IntegrateContact(contact, bodyA, bodyB);
}

shade::StackArray<shade::physic::CollisionShape::Manifold, 4u> shade::physic::PhysicsManager::GetStableContacts(const RigidBody& bodyA, const RigidBody& bodyB)
{
	return std::move(m_ContactsData.GetReducedContacts(bodyA, bodyB));
}

void shade::physic::PhysicsManager::DetectCollisions(ecs::BasicView<ecs::EntityID, RigidBodyComponent, TransformComponent>& bodies, scalar_t deltaTime)
{
	// We don't have proper iterator + 1 realization, so that's why we use copy of i iterator and incrementing copy+ 1 and asigning to j
	// Try implement += iterator and use m_Current < m_last and i and j < view.end(); to do not overjump 

	for (auto iterA = bodies.begin(); iterA != bodies.end(); iterA++)
	{
		auto bodyA = iterA->GetComponentRaw<RigidBodyComponent>(); auto& tbA = iterA->GetComponent<TransformComponent>();
		
		for (auto iterB = ecs::BasicView<ecs::EntityID, RigidBodyComponent, TransformComponent>::iterator(iterA)++; iterB != bodies.end(); iterB++)
		{
			auto bodyB = iterB->GetComponentRaw<RigidBodyComponent>(); auto& tbB = iterB->GetComponent<TransformComponent>();
			// 1. AABB Test (Narrow phase)
			// 2. OBB Test  (Middle phase)
			// 3. Test full collision and generate contact points
			// 4. Resolve position and impulses
			if (*bodyA || *bodyB)
			{
				// Broad Phase
				if (bodyA->AABB_X_AABB(tbA.GetModelMatrix(), *bodyB, tbB.GetModelMatrix()))
				{
					// Middle Pahse
					if (bodyA->OBB_X_OBB(tbA.GetModelMatrix(), *bodyB, tbB.GetModelMatrix()))
					{
						//Narrow Phase
						auto result = bodyA->TestCollision(tbA.GetModelMatrix(), *bodyB, tbB.GetModelMatrix());

						if (result.HasCollision)
						{
							IntegrateContact(result, *bodyA, *bodyB);

							StackArray<CollisionShape::Manifold, 4> contacts = GetStableContacts(*bodyA, *bodyB);

							PositionSolver(result, std::pair<RigidBodyComponent&, TransformComponent&>{*bodyA, tbA }, std::pair<RigidBodyComponent&, TransformComponent&>{*bodyB, tbB }, deltaTime);
							ImpulseSolver(contacts, std::pair<RigidBodyComponent&, TransformComponent&>{*bodyA, tbA }, std::pair<RigidBodyComponent&, TransformComponent&>{*bodyB, tbB }, deltaTime);

							for (auto& contact : contacts)
							{
								bodyA->m_CollisionContancts.PushFront(contact.ContactPointA_L);
								bodyB->m_CollisionContancts.PushFront(contact.ContactPointB_L);
							}
						}
					}
				}
			}
		}
	}
}

void shade::physic::PhysicsManager::PositionSolver(const CollisionShape::Manifold& contact, const std::pair<RigidBody&, TransformComponent&>& bodyA, const std::pair<RigidBody&, TransformComponent&>& bodyB, scalar_t deltaTime)
{
	auto& [aBody, trbA] = bodyA;
	auto& [bBody, trbB] = bodyB;

	const scalar_t aInvMass = aBody ? aBody.GetInverseMass() : 0.0f;
	const scalar_t bInvMass = bBody ? bBody.GetInverseMass() : 0.0f;

	const scalar_t slop = 0.01;
	const scalar_t percent = 0.99;

	glm::vec<3, scalar_t> resolution = contact.Normal * percent * glm::max<scalar_t>(contact.CollisionDepth - slop, 0.0);

	if (aBody) trbA.Move(-resolution);
	if (bBody) trbB.Move( resolution);
}

void shade::physic::PhysicsManager::ImpulseSolver(const StackArray<CollisionShape::Manifold, 4>& contacts, const std::pair<RigidBody&, TransformComponent&>& bodyA, const std::pair<RigidBody&, TransformComponent&>& bodyB, scalar_t deltaTime)
{
	auto& [aBody, aTransform] = bodyA;
	auto& [bBody, bTransform] = bodyB;

	scalar_t aInvMass     = aBody ? aBody.GetInverseMass() : scalar_t(1.0), bInvMass = bBody ? bBody.GetInverseMass() : scalar_t(1.0);
	scalar_t cFriction    = (aBody.StaticFriction + bBody.StaticFriction) * scalar_t(0.5);
	scalar_t cRestitution = (aBody.Restitution + bBody.Restitution) * scalar_t(0.5);
	scalar_t totalMass    = aInvMass + bInvMass;

	scalar_t contactsCount = contacts.GetSize();

	glm::vec<3, scalar_t> aVel = aBody.LinearVelocity, aAngVel = aBody.AngularVelocity;
	glm::vec<3, scalar_t> bVel = bBody.LinearVelocity, bAngVel = bBody.AngularVelocity;

	for (auto& contact : contacts)
	{
		// If it's an empty contacts 
		if (!contact.HasCollision) continue;
		// Calculate the relative positions of the contact points on the two objects
		glm::vec<3, scalar_t> rA = (contact.ContactPointA_W - static_cast<glm::vec<3, scalar_t>>(aTransform.GetPosition()));
		glm::vec<3, scalar_t> rB = (contact.ContactPointB_W - static_cast<glm::vec<3, scalar_t>>(bTransform.GetPosition()));

		// Calculate the total velocities of both objects at the contact point
		glm::vec<3, scalar_t> aTotalVelocity = aVel + glm::cross<scalar_t>(aAngVel, rA);
		glm::vec<3, scalar_t> bTotalVelocity = bVel + glm::cross<scalar_t>(bAngVel, rB);
		// Calculate the relative velocity at the contact point
		glm::vec<3, scalar_t> rVelocity = bTotalVelocity - aTotalVelocity;
		// Get the contact normal from the collision information
		glm::vec<3, scalar_t> contactNormal = contact.Normal;
		// Calculate the impulse magnitude using the relative velocity and contact normal
		scalar_t impulse = glm::dot<3, scalar_t>(rVelocity, contactNormal);
		// If the impulse is non-negative, objects are separating, so skip
		if (impulse >= 0) continue;
		// Calculate angular effect due to inertia for both objects
		glm::vec<3, scalar_t> aInertiaTensor = aBody ? glm::cross<scalar_t>(aBody.GetIntertiaTensor() * glm::cross<scalar_t>(rA, contactNormal), rA) : glm::vec<3, scalar_t>(0.0);
		glm::vec<3, scalar_t> bInertiaTensor = bBody ? glm::cross<scalar_t>(bBody.GetIntertiaTensor() * glm::cross<scalar_t>(rB, contactNormal), rB) : glm::vec<3, scalar_t>(0.0);
		// Calculate the angular effect on the impulse
		scalar_t angularEffect = glm::dot<3, scalar_t>(aInertiaTensor + bInertiaTensor, contactNormal);
		// Calculate impulse without and with angular effect
		scalar_t j  = (-(scalar_t(1.0) + cRestitution) * impulse) / (totalMass + angularEffect);
		j /= contactsCount;
		// Calculate linear and angular impulses
		glm::vec<3, scalar_t> velocity = contactNormal * j;
		// Apply impulses to objects
		if (aBody)
		{
			aVel	-= velocity;//* aInvMass;
			aAngVel -= (aBody.GetIntertiaTensor() * glm::cross<scalar_t>(rA, velocity));
		}
		if (bBody)
		{
			bVel	+= velocity; // * bInvMass;
			bAngVel += (bBody.GetIntertiaTensor() * glm::cross<scalar_t>(rB, velocity));
		}
		
		//Calculate the tangent component of the relative velocity
 		rVelocity = (bVel + glm::cross<scalar_t>(bAngVel, rB)) - (aVel + glm::cross<scalar_t>(aAngVel, rA));
		glm::vec<3, scalar_t> fTangent = rVelocity - contactNormal * glm::dot(rVelocity, contactNormal);

		scalar_t length = glm::length<3, scalar_t>(fTangent);
	
		if (length > 1e-6)
		{
			fTangent /= length;
			// Calculate friction mass
			scalar_t frictionMass = totalMass + glm::dot(fTangent,
					glm::cross(aBody.GetIntertiaTensor() * glm::cross(rA, fTangent), rA) +
					glm::cross(bBody.GetIntertiaTensor() * glm::cross(rB, fTangent), rB)
				);

			// Apply friction impulse if mass is greater than 0
			if (frictionMass > 0.0)
			{
				scalar_t jt = -glm::dot<3, scalar_t>(rVelocity, fTangent) * cFriction;

				jt /= frictionMass;

				if (aBody)
				{
					aVel	-= fTangent * jt;
					aAngVel -= (aBody.GetIntertiaTensor() * glm::cross<scalar_t>(rA, fTangent * jt)) / contactsCount;
				}
				if (bBody) 
				{
					bVel	+= fTangent * jt;
					bAngVel += (bBody.GetIntertiaTensor() * glm::cross<scalar_t>(rB, fTangent * jt)) / contactsCount;
				}
			}
		}
	}

	if (aBody) { aBody.LinearVelocity = aVel; aBody.AngularVelocity = aAngVel; }
	if (bBody) { bBody.LinearVelocity = bVel; bBody.AngularVelocity = bAngVel; }
}

void shade::physic::PhysicsManager::CashedContactData::IntegrateContact(const CollisionShape::Manifold& manifold, const RigidBody& bodyA, const RigidBody& bodyB)
{
	std::size_t hash = reinterpret_cast<std::size_t>(&bodyA);
	glm::detail::hash_combine(hash, reinterpret_cast<std::size_t>(&bodyB));

	bool same = false;
	for (auto& point : m_Pairs[hash].Contancts)
	{
		if (point == manifold)
		{
			same = true;
		}
	}
    if (!same)
	{
		m_Pairs[hash].Contancts.PushFront(manifold);
	}
}

shade::StackArray<shade::physic::CollisionShape::Manifold, 4u> shade::physic::PhysicsManager::CashedContactData::GetReducedContacts(const RigidBody& bodyA, const RigidBody& bodyB)
{
	std::size_t hash = reinterpret_cast<std::size_t>(&bodyA);
	glm::detail::hash_combine(hash, reinterpret_cast<std::size_t>(&bodyB));

	StackArray<CollisionShape::Manifold, 4u> reduced;

	auto contacts = m_Pairs.find(hash);
	if (contacts != m_Pairs.end())
	{
		CollisionPairArray::iterator A = contacts->second.Contancts.begin();

		for (CollisionPairArray::iterator current = contacts->second.Contancts.begin()++; current != contacts->second.Contancts.end(); current++)
		{
			if (current->CollisionDepth > A->CollisionDepth && current->HasCollision)
				A = current;
		}

		reduced.PushFront(*A);

		CollisionPairArray::iterator B = contacts->second.Contancts.end(); scalar_t A_To_B_Distance = 0.0;
		for (CollisionPairArray::iterator current = contacts->second.Contancts.begin(); current != contacts->second.Contancts.end(); current++)
		{
			if (current->HasCollision && current != A)
			{
				scalar_t distance = glm::distance<3, scalar_t>(A->ContactPointA_L, current->ContactPointA_L);

				if (distance > A_To_B_Distance)
				{
					A_To_B_Distance = distance; B = current;
				}
			}
		}

		CollisionPairArray::iterator C = contacts->second.Contancts.end(); scalar_t maxArea = 0.0;
		if (B != contacts->second.Contancts.end())
		{
			reduced.PushFront(*B);

			for (CollisionPairArray::iterator current = contacts->second.Contancts.begin(); current != contacts->second.Contancts.end(); current++)
			{
				if (current->HasCollision && current != A && current != B && current != C)
				{
					glm::vec<3, scalar_t> C_A = current->ContactPointA_L * A->ContactPointA_L;
					glm::vec<3, scalar_t> C_B = current->ContactPointA_L * B->ContactPointA_L;

					scalar_t area = 0.5 * glm::dot(glm::cross<scalar_t>(C_A, C_B), A->Normal);

					if (area > maxArea)
					{
						maxArea = area; C = current;
					}
				}
			}
		}

		if (C != contacts->second.Contancts.end())
		{
			reduced.PushFront(*C);

			CollisionPairArray::iterator D = contacts->second.Contancts.end(); maxArea = 0.0;
			for (CollisionPairArray::iterator current = contacts->second.Contancts.begin(); current != contacts->second.Contancts.end(); current++)
			{
				if (current->HasCollision && current != A && current != B && current != C && current != D)
				{
					glm::vec<3, scalar_t> D_C = current->ContactPointA_L * C->ContactPointA_W;
					glm::vec<3, scalar_t> D_A = current->ContactPointA_L * A->ContactPointA_L;

					scalar_t area = 0.5 * glm::dot(glm::cross<scalar_t>(D_C, D_A), A->Normal);

					if (area > maxArea)
					{
						maxArea = area; D = current;
					}
				}
			}

			if (D != contacts->second.Contancts.end())
				reduced.PushFront(*D);
		}
	}

	return std::move(reduced);
}

void shade::physic::PhysicsManager::CashedContactData::Clear()
{
	m_Pairs.clear();
}


