#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/physics/Common.h>

namespace shade
{
	namespace physic
	{
		class Particle
		{
		public:
			/* Holds the linear position of the particle in world space.*/
			glm::vec<3, scalar_t> position;
			/* Holds the linear velocity of the particle in world space.*/
			glm::vec<3, scalar_t> velocity;
			/* Holds the acceleration of the particle. This value can be used to set acceleration due to gravity 
			(its primary use) or any other constant acceleration.*/
			glm::vec<3, scalar_t> acceleration;
			/* Holds the amount of damping applied to linear motion.Damping is required to remove energy added
			through numerical instability in the integrator.*/
			scalar_t damping;
			/*Holds the inverse of the mass of the particle. It is more useful to hold the inverse mass because
			integration is simpler and because in real-time simulation it is more useful to have objects with 
			infinite mass (immovable) than zero mass (completely unstable in numerical simulation).*/
			scalar_t inverseMass;
			/**
			* Holds the accumulated force to be applied at the next
			* simulation iteration only. This value is zeroed at each
			* integration step.
			*/
			glm::vec<3, scalar_t> forceAccum;
			/**
			* Integrates the particle forward in time by the given amount.
			* This function uses a Newton-Euler integration method, which is a
			* linear approximation to the correct integral. For this reason it
			* may be inaccurate in some cases.
			*/
			void integrate(scalar_t duration);
		};

	}
}