#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <shade/core/physics/Common.h>

namespace shade
{
	namespace physic
	{
		namespace algo
		{
			struct SupportPoint
			{
				glm::vec<3, scalar_t> MinkowskiDifference;
				// World space support points
				glm::vec<3, scalar_t> SupportPointA_W;
				glm::vec<3, scalar_t> SupportPointB_W;
				// Local space support points
				glm::vec<3, scalar_t> SupportPointA_L;
				glm::vec<3, scalar_t> SupportPointB_L;

				bool operator==(const SupportPoint& other) const noexcept 
				{
					return  (other.MinkowskiDifference == MinkowskiDifference) &&
							(other.SupportPointA_W == SupportPointA_W)  &&
							(other.SupportPointB_W == SupportPointB_W);

				}
				bool operator!=(const SupportPoint& other) const noexcept 
				{
					return  (other.MinkowskiDifference != MinkowskiDifference) &&
							(other.SupportPointA_W != SupportPointA_W) &&
							(other.SupportPointB_W != SupportPointB_W);
				}
			};
			struct PolytopeFace
			{
				std::array<SupportPoint, 3> Points;
				glm::vec<3, scalar_t> Normal, UNormal;
				PolytopeFace() = default;
				PolytopeFace(const SupportPoint& a, const SupportPoint& b, const SupportPoint& c)
				{
					Points.at(0) = a; Points.at(1) = b; Points.at(2) = c;
					UNormal = glm::cross(b.MinkowskiDifference - a.MinkowskiDifference, c.MinkowskiDifference - a.MinkowskiDifference);
					Normal  = glm::normalize(UNormal);
				}
				bool operator==(const PolytopeFace& other) const noexcept
				{
					return (
							Points.at(0) == other.Points.at(0) && 
							Points.at(1) == other.Points.at(1) &&
							Points.at(2) == other.Points.at(2) &&
							Normal == other.Normal);
				}
				bool operator!=(const PolytopeFace& other) const noexcept
				{
					return (
						Points.at(0) != other.Points.at(0) &&
						Points.at(1) != other.Points.at(1) &&
						Points.at(2) != other.Points.at(2) && 
						Normal != other.Normal);
				}
			};
			struct PolytopeEdge
			{
				std::array<SupportPoint, 2> Points;
				PolytopeEdge(const SupportPoint& a, const SupportPoint& b)
				{
					Points.at(0) = a; Points.at(1) = b;
				}
			};
			class SHADE_API Simplex
			{
			public:
				Simplex();
				Simplex(const SupportPoint& points);
				~Simplex() = default;

				Simplex& operator=(std::initializer_list<SupportPoint> list)
				{
					for (const SupportPoint* v = list.begin();  v != list.end(); v++) {
						m_Points[std::distance(list.begin(), v)] = *v;
					}
					m_Count = list.size();
					return *this;
				};
				SupportPoint& operator[](std::size_t i) { return m_Points[i]; }
				const SupportPoint& operator[](std::size_t i) const { return m_Points[i]; }
				std::size_t GetSize() const { return m_Count; }
				void PushFront(const SupportPoint& point);

				std::array<SupportPoint, 4>::const_iterator begin() const { return m_Points.begin(); };
				std::array<SupportPoint, 4>::const_iterator end() const { return m_Points.end() - (4 - m_Count); };

			private:
				std::array<SupportPoint, 4> m_Points;
				std::size_t				 m_Count;
			};
			
		}
	}
}