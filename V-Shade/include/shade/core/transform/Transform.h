#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/core/math/Math.h>
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	class SHADE_API Transform
	{
	public:
		Transform();
		Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
		~Transform() = default;

		// Set transform position.
		void SetPosition(float x, float y, float z);
		// Set transform position.
		void SetPosition(const glm::vec3& position);
		// Move transform
		void Move(float x, float y, float z);
		// Move transform
		void Move(const glm::vec3& position);
		// Set transform rotation.
		void SetRotation(float x, float y, float z);
		// Set transform rotation.
		void SetRotation(const glm::vec3& rotation);
		// Rotate transform.
		void Rotate(float x, float y, float z);
		// Rotate transform.
		void Rotate(const glm::vec3& rotation);
		// Set transform scale.
		void SetScale(float x, float y, float z);
		// Set transform scale.
		void SetScale(const glm::vec3& scale);
		// Get transform position.
		const glm::vec3& GetPosition() const;
		// Get transform position.
		glm::vec3& GetPosition();
		// Get transform rotation.
		const glm::vec3& GetRotation() const;
		// Get transform rotation.
		glm::vec3& GetRotation();
		// Get transform scale.
		const glm::vec3& GetScale()    const;
		// Get transform scale.
		glm::vec3& GetScale();
		// Get model matrix based on position, rotation and scale
		glm::mat4 GetModelMatrix() const;
		// Set transform from matrix 
		void SetTransform(const glm::mat4& matrix);
		// Get transform from matrix
		static Transform GetTransformFromMatrix(const glm::mat4& matrix);

		void SetDirection(const glm::vec3& direction);
	private:
		glm::vec3 m_Possition;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
	private:
		friend class Serializer;
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
	};

	inline void Transform::SetPosition(float x, float y, float z) { m_Possition.x = x; m_Possition.y = y; m_Possition.z = z; };
	inline void Transform::SetPosition(const glm::vec3& position) { m_Possition = position; }
	inline void Transform::Move(float x, float y, float z) { m_Possition.x += x; m_Possition.y += y; m_Possition.z += z; }
	inline void Transform::Move(const glm::vec3& position) { m_Possition += position; }
	inline void Transform::SetRotation(float x, float y, float z) { m_Rotation.x = x; m_Rotation.y = y; m_Rotation.z = z; };
	inline void Transform::SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }
	inline void Transform::Rotate(float x, float y, float z) { m_Rotation.x += x; m_Rotation.y += y; m_Rotation.z += z; }
	inline void Transform::Rotate(const glm::vec3& rotation) { m_Rotation *= rotation; }
	inline void Transform::SetScale(float x, float y, float z) { m_Scale.x = x; m_Scale.y = y; m_Scale.z = z; };
	inline void Transform::SetScale(const glm::vec3& scale) { m_Scale = scale; };
	inline const glm::vec3& Transform::GetPosition() const { return m_Possition; }
	inline glm::vec3& Transform::GetPosition() { return const_cast<glm::fvec3&>(const_cast<const shade::Transform*>(this)->GetPosition()); }
	inline const glm::vec3& Transform::GetRotation() const { return m_Rotation; }
	inline glm::vec3& Transform::GetRotation() { return const_cast<glm::fvec3&>(const_cast<const shade::Transform*>(this)->GetRotation()); }
	inline const glm::vec3& Transform::GetScale()    const { return m_Scale; }
	inline glm::vec3& Transform::GetScale() { return const_cast<glm::fvec3&>(const_cast<const shade::Transform*>(this)->GetScale()); }
	inline glm::mat4 Transform::GetModelMatrix() const { return glm::translate(glm::mat4(1.f), m_Possition) * glm::toMat4(glm::quat((m_Rotation))) * glm::scale(glm::mat4(1.f), m_Scale); }
	inline void Transform::SetTransform(const glm::mat4& matrix) { math::DecomposeMatrix(matrix, m_Possition, m_Rotation, m_Scale); }
	inline Transform Transform::GetTransformFromMatrix(const glm::mat4& matrix) { Transform transform; math::DecomposeMatrix(matrix, transform.m_Possition, transform.m_Rotation, transform.m_Scale); return transform; }

#ifndef TRANSFORM_DATA_SIZE
	#define TRANSFORM_DATA_SIZE (sizeof(glm::mat4))
#endif // !TRANSFORM_DATA_SIZE

#ifndef TRANSFORMS_DATA_SIZE
	#define TRANSFORMS_DATA_SIZE(count) (TRANSFORM_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !TRANSFORMS_DATA_SIZE
}

namespace shade
{
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Transform& transform, std::size_t)
	{
		return transform.Serialize(stream);
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Transform& transform, std::size_t)
	{
		return transform.Deserialize(stream);
	}
}