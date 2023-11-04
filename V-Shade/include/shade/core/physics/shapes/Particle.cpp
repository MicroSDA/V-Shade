#include "shade_pch.h"
#include "Particle.h"

void shade::physic::Particle::integrate(scalar_t duration)
{
	assert(duration > 0.0);
	// Update linear position;
	position += velocity * duration;
	// Work out the acceleration from the force.
	glm::vec<3, scalar_t> resultingAcc = acceleration;
	resultingAcc += forceAccum * inverseMass;
	// Update linear velocity from the acceleration.
	velocity += resultingAcc * duration;
	// Impose drag.
	velocity *= glm::pow<scalar_t>(damping, duration);
}
