#pragma once
#include <shade/core/physics/shapes/CollisionShape.h>
#include <shade/core/physics/algo/Simplex.h>

namespace shade
{
	namespace physic
	{
		namespace algo
		{
			constexpr std::size_t MAX_ITERATION = 32u;

			[[nodiscard]] inline bool SameDirection(const glm::vec<3, scalar_t>& direction, const glm::vec<3, scalar_t>& ao)
			{
				return glm::dot(direction, ao) > 0;
			}
			[[nodiscard]] inline void AddIfUniqueEdge(
				std::vector<PolytopeEdge>& edges, const SupportPoint& a, const SupportPoint& b)
			{
				for (auto iterator = edges.begin(); iterator != edges.end(); ++iterator)
				{
					if (iterator->Points[0] == b && iterator->Points[1] == a)
					{
						// Encountered the same edge with opposite winding, remove it and don't add a new one
						edges.erase(iterator);
						return;
					}
				}
				edges.emplace_back(a, b);
			}
			[[nodiscard]] inline SupportPoint Support(const CollisionShape& shapeA, const glm::mat4& transformA, const CollisionShape& shapeB, const glm::mat4& transformB, const glm::vec<3, scalar_t>& direction)
			{
				SupportPoint support;
				support.SupportPointA_W = shapeA.FindFurthestPointWorld(transformA, direction);
				support.SupportPointB_W = shapeB.FindFurthestPointWorld(transformB, -direction);

				support.MinkowskiDifference = support.SupportPointA_W - support.SupportPointB_W;
				return support;
			}
			[[nodiscard]] inline void BarycentricProjection(const glm::vec<3, scalar_t>& aPoint, const glm::vec<3, scalar_t>& a, const glm::vec<3, scalar_t>& b, const glm::vec<3, scalar_t>& c, scalar_t& u, scalar_t& v, scalar_t& w)
			{
				glm::vec<3, scalar_t> v0 = b - a, v1 = c - a, v2 = aPoint - a;
				scalar_t d00 = glm::dot(v0, v0);
				scalar_t d01 = glm::dot(v0, v1);
				scalar_t d11 = glm::dot(v1, v1);
				scalar_t d20 = glm::dot(v2, v0);
				scalar_t d21 = glm::dot(v2, v1);
				scalar_t denom = d00 * d11 - d01 * d01;
				v = (d11 * d20 - d01 * d21) / denom;
				w = (d00 * d21 - d01 * d20) / denom;
				u = scalar_t(1.0) - v - w;
			}
			[[nodiscard]] inline std::pair<glm::vec<3, scalar_t>, glm::vec<3, scalar_t>> FindContactPoints(PolytopeFace& face, const  glm::mat4& transformA, const glm::mat4& transformB)
			{
				const scalar_t distanceFromOrigin = glm::dot(face.UNormal, face.Points[0].MinkowskiDifference);
				// calculate the barycentric coordinates of the closest triangle with respect to
				// the projection of the origin onto the triangle
				scalar_t bary_u, bary_v, bary_w; 
				BarycentricProjection(face.Normal * distanceFromOrigin, face.Points[0].MinkowskiDifference, face.Points[1].MinkowskiDifference, face.Points[2].MinkowskiDifference, bary_u, bary_v, bary_w);
				return 
				{   
					(bary_u * face.Points[0].SupportPointA_W) + (bary_v * face.Points[1].SupportPointA_W) + (bary_w * face.Points[2].SupportPointA_W),  // Contact point on object A in world space
					(bary_u * face.Points[0].SupportPointB_W) + (bary_v * face.Points[1].SupportPointB_W) + (bary_w * face.Points[2].SupportPointB_W)   // Contact point on object B in world space
				};
			}
			[[nodiscard]] inline CollisionShape::Manifold EPA(const Simplex& simplex, const CollisionShape& shapeA, const glm::mat4& transformA, const CollisionShape& shapeB, const glm::mat4& transformB)
			{
				std::vector<PolytopeFace> polytopeFaces
				{
					PolytopeFace(simplex[0], simplex[1], simplex[2]),
					PolytopeFace(simplex[0], simplex[2], simplex[3]),
					PolytopeFace(simplex[0], simplex[3], simplex[1]),
					PolytopeFace(simplex[1], simplex[3], simplex[2]),
				};

				std::vector<PolytopeEdge> polytopeEdges;
				std::size_t iterationCount = 0; scalar_t minimumDistance = std::numeric_limits<scalar_t>::max();

				std::vector<PolytopeFace>::iterator closestFace;
				while (minimumDistance == std::numeric_limits<scalar_t>::max())
				{
					if (iterationCount++ >= MAX_ITERATION)
						break;

					closestFace = polytopeFaces.begin();

					for (auto iterator = polytopeFaces.begin(); iterator != polytopeFaces.end(); ++iterator)
					{
						scalar_t distance = glm::dot(iterator->Normal, iterator->Points[0].MinkowskiDifference);
						if (distance < minimumDistance)
						{
							minimumDistance = distance;
							closestFace = iterator;
						}
					}

					SupportPoint newPolytopePoint = Support(shapeA, transformA, shapeB, transformB, closestFace->Normal);
					const scalar_t sDistance = glm::dot(closestFace->Normal, newPolytopePoint.MinkowskiDifference);

					if (glm::abs(sDistance - minimumDistance) > 0.001)
					{
						minimumDistance = std::numeric_limits<scalar_t>::max();

						for (auto iterator = polytopeFaces.begin(); iterator != polytopeFaces.end();)
						{
							// A face is considered as "seen" if the new support point is on the positive halfspace of the plane defined by it
							glm::vec<3, scalar_t> planeVector = newPolytopePoint.MinkowskiDifference - iterator->Points[0].MinkowskiDifference;

							if (SameDirection(iterator->Normal, planeVector))
							{
								// Only adds the outside edges of the 'seen' face, and ignores the others
								AddIfUniqueEdge(polytopeEdges, iterator->Points[0], iterator->Points[1]);
								AddIfUniqueEdge(polytopeEdges, iterator->Points[1], iterator->Points[2]);
								AddIfUniqueEdge(polytopeEdges, iterator->Points[2], iterator->Points[0]);
								// Remove the face from the Polytope
								iterator = polytopeFaces.erase(iterator);
								continue;
							}
							++iterator;
						}

						// Create new polytope faces using the new support point from the valid edges in the edge list
						for (auto iterator = polytopeEdges.begin(); iterator != polytopeEdges.end(); ++iterator)
						{
							polytopeFaces.emplace_back(newPolytopePoint, iterator->Points[0], iterator->Points[1]);
						}

						// Clear the edge list every iteration of the expansion
						polytopeEdges.clear();
					}
				}

				if (minimumDistance != std::numeric_limits<scalar_t>::max())
				{
					auto [contactPointA, contactPointB] = FindContactPoints(*closestFace, transformA, transformB);

					return {	true, minimumDistance + 0.001, closestFace->Normal, 
								contactPointA,
								contactPointB, 
								glm::vec<4, scalar_t>(contactPointA, 1.f) * glm::inverse(glm::transpose(transformA)),
								glm::vec<4, scalar_t>(contactPointB, 1.f) * glm::inverse(glm::transpose(transformB))
					};
				}

				return { false };
			}
		}
	}
}
