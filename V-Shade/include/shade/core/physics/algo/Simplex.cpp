#include "shade_pch.h"
#include "Simplex.h"

shade::physic::algo::Simplex::Simplex() : 
	m_Points({ glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f) }),
	m_Count(0u)
{
}

shade::physic::algo::Simplex::Simplex(const SupportPoint& points) :
	m_Points({ glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f), glm::vec<3, scalar_t>(0.f) }),
	m_Count(0u)
{
	PushFront(points);
}

void shade::physic::algo::Simplex::PushFront(const SupportPoint& point)
{
	m_Points = { point, m_Points[0], m_Points[1], m_Points[2] };
	m_Count = std::min<std::size_t>(m_Count + 1u, 4u);
}
