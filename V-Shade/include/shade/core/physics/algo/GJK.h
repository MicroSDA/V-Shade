#pragma once
#include <shade/core/physics/shapes/CollisionShape.h>
#include <shade/core/physics/algo/EPA.h>

namespace shade
{
	namespace physic
	{
		namespace algo
		{
			[[nodiscard]] inline bool Line(
				Simplex& points,
				glm::vec<3, scalar_t>& direction)
			{
				const SupportPoint a = points[0], b = points[1];
				const glm::vec<3, scalar_t> ab = b.MinkowskiDifference - a.MinkowskiDifference, ao = -a.MinkowskiDifference;

				if (SameDirection(ab, ao))
				{
					direction = glm::cross(glm::cross(ab, ao), ab);
				}
				else
				{
					points = { a };
					direction = ao;
				}

				return false;
			}
			[[nodiscard]] inline bool Triangle(
				Simplex& points,
				glm::vec<3, scalar_t>& direction)
			{
				const SupportPoint a = points[0], b = points[1], c = points[2];
				const glm::vec<3, scalar_t> ab = b.MinkowskiDifference - a.MinkowskiDifference, ac = c.MinkowskiDifference - a.MinkowskiDifference, ao = -a.MinkowskiDifference;
				const glm::vec<3, scalar_t> abc = glm::cross(ab, ac);

				if (SameDirection(glm::cross(abc, ac), ao))
				{
					if (SameDirection(ac, ao))
					{
						points = { a, c };
						direction = glm::cross(glm::cross(ac, ao), ac);
					}
					else
					{
						return Line(points = { a, b }, direction);
					}
				}
				else
				{
					if (SameDirection(glm::cross(ab, abc), ao))
					{
						return Line(points = { a, b }, direction);
					}
					else
					{
						if (SameDirection(abc, ao))
						{
							direction = abc;
						}
						else
						{
							points = { a, c, b };
							direction = -abc;
						}
					}
				}

				return false;
			}
			bool Tetrahedron(
				Simplex& points,
				glm::vec<3, scalar_t>& direction)
			{
				const SupportPoint a = points[0], b = points[1], c = points[2], d = points[3];
				const glm::vec<3, scalar_t> ab = b.MinkowskiDifference - a.MinkowskiDifference, ac = c.MinkowskiDifference - a.MinkowskiDifference, ad = d.MinkowskiDifference - a.MinkowskiDifference, ao = -a.MinkowskiDifference;
				
				const glm::vec<3, scalar_t> abc = glm::cross(ab, ac), acd = glm::cross(ac, ad), adb = glm::cross(ad, ab);
				
				if (SameDirection(abc, ao))
					return Triangle(points = { a, b, c }, direction);

				if (SameDirection(acd, ao))
					return Triangle(points = { a, c, d }, direction);

				if (SameDirection(adb, ao))
					return Triangle(points = { a, d, b }, direction);

				return true;
			}
			[[nodiscard]] inline bool NextSimplex(
				Simplex& points,
				glm::vec<3, scalar_t>& direction)
			{
				switch (points.GetSize()) 
				{
					case 2: return Line(points, direction);
					case 3: return Triangle(points, direction);
					case 4: return Tetrahedron(points, direction);
				}

				assert(true);
				// never should be here
				return false;
			}
			[[nodiscard]] inline std::pair<bool, Simplex> GJK(const CollisionShape& shapeA, const glm::mat4& transformA, const CollisionShape& shapeB, const glm::mat4& transformB)
			{
				// Get initial support point in any direction
				SupportPoint support = Support(shapeA, transformA, shapeB, transformB, glm::vec<3, scalar_t>(1.f, 0.f, 0.f));
				// Simplex is an array of points, max count is 4
				Simplex points(support);
				// New direction is towards the origin
				glm::vec<3, scalar_t> direction = -(support.MinkowskiDifference);
				for (std::size_t i = 0; i < MAX_ITERATION; i++)
				{
					support = Support(shapeA, transformA, shapeB, transformB, direction);

					if (glm::dot(support.MinkowskiDifference, direction) <= 0) // If there <= then no proper colliding when any of 2 axies are aligned
						break;

					points.PushFront(support);

					if (NextSimplex(points, direction))
						return { true, points };
				}
				// Return no collision 
				return { false,  points};
			}
		}
	}
}
