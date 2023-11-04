#pragma once
#include <shade/core/render/drawable/Drawable.h>

namespace shade
{
	class SHADE_API Box : public Drawable
	{
	public:
		static SharedPointer<Box> Create(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		virtual ~Box() = default;

		const glm::vec3& GetMinHalfExt() const;
		const glm::vec3& GetMaxHalfExt() const;
	private:
		Box(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		friend class SharedPointer<Box>;
	private:
		glm::vec3 m_MinHalfExt;
		glm::vec3 m_MaxHalfExt;
	};
}