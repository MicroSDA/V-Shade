#pragma once
#include <shade/core/physics/Common.h>
#include <shade/core/render/drawable/Mesh.h>

namespace shade
{
	namespace physic
	{
		namespace algo
		{

			SHADE_API std::pair<std::vector<glm::vec<3, scalar_t>>, Indices> GenerateConvexHull(const Vertices& pointsCloud, scalar_t epsilon);
		}
	}
}