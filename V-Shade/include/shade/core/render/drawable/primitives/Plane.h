#pragma once
#include <shade/core/render/drawable/Drawable.h>

namespace shade 
{
	class SHADE_API Plane : public Drawable
	{
	public:
		static SharedPointer<Plane> Create(float width, float height, std::uint32_t density);
		virtual ~Plane() = default;
	private:
		Plane(float width, float height, std::uint32_t density);
		friend class SharedPointer<Plane>;
	};
}

