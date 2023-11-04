#include "shade_pch.h"
#include "Plane.h"

shade::SharedPointer<shade::Plane> shade::Plane::Create(float width, float height, std::uint32_t density)
{
    return SharedPointer<Plane>::Create(width, height, density);
}

shade::Plane::Plane(float width, float height, std::uint32_t density)
{
    // Triangle Strip !

    AddVertex({ .Position = {-1.0,  1.0, 0.0} });
    AddVertex({ .Position = {-1.0, -1.0, 0.0} });
    AddVertex({ .Position = { 1.0,  1.0, 0.0} });
    AddVertex({ .Position = { 1.0, -1.0, 0.0} });

    AddIndex(0); AddIndex(1); AddIndex(2); AddIndex(3);
}
