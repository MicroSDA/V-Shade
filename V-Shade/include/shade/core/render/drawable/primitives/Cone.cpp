#include "shade_pch.h"
#include "Cone.h"
#include <glm/glm/gtx/rotate_vector.hpp>
#include <glm/glm/gtc/constants.hpp>
#include <glm/glm/gtc/quaternion.hpp>

shade::SharedPointer<shade::Cone> shade::Cone::Create(float radius, float length, std::uint32_t density, float step, const glm::vec3& derection)
{
	return SharedPointer<Cone>::Create(radius, length, density, step, derection);
}

shade::Cone::Cone(float radius, float length, std::uint32_t density, float step, const glm::vec3& derection)
{
    // Polygon mode LineStrip

    const float _step = std::atan(1.0f) * step / density;
    // Make Circle
    for (std::uint32_t i = 0; i < density; i++)
    {
        float a = i * _step;
        AddVertex({ .Position = { std::cos(a) * radius, std::sin(a) * radius, length }});  AddIndex(i);
    }
    // Apex
    AddVertex({ .Position = glm::vec3(0.f) });
    // Link circle with apex
    for (std::uint32_t i = 0; i < GetVertices().size() - 1; i++)
    {
        AddIndex(i); AddIndex(density);
    }
}
