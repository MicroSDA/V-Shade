#pragma once
#include <shade/core/render/drawable/Drawable.h>

namespace shade
{
	class SHADE_API Sphere : public Drawable
	{
	public:
		static SharedPointer<Sphere> Create(float radius, float density, float step);
		virtual ~Sphere() = default;
	private:
		Sphere(std::size_t xDensity, std::size_t yDensity, float radius);
		friend class SharedPointer<Sphere>;
	};
}
