#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/core/math/Math.h>
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	/**
	  * @brief Represents a 3D transformation including position, rotation (as a quaternion), and scale.
	  * Provides methods to manipulate the transformation and calculate the model matrix.
	  */
	class SHADE_API Transform
	{
	public:
		Transform();
		/**
		 * @brief Constructor to initialize the transform with specific position, rotation, and scale.
		 * @param position The initial position of the transform.
		 * @param rotation The initial rotation of the transform in Euler angles (degrees).
		 * @param scale The initial scale of the transform.
		 */
		 //Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
		~Transform() = default;

		/**
		 * @brief Sets the position of the transform.
		 * @param x X-coordinate of the position.
		 * @param y Y-coordinate of the position.
		 * @param z Z-coordinate of the position.
		 */
		SHADE_INLINE void SetPosition(float x, float y, float z)
		{
			m_Position.x = x; m_Position.y = y; m_Position.z = z;
		}
		/**
		 * @brief Sets the position of the transform.
		 * @param position The new position as a glm::vec3.
		 */
		SHADE_INLINE void SetPosition(const glm::vec3& position)
		{
			m_Position = position;
		}
		/**
		* @brief Gets the current position of the transform.
		* @return The current position as a glm::vec3.
		*/

		SHADE_INLINE glm::vec3& GetPosition()
		{
			return m_Position;
		}
		/**
		* @brief Gets the current position of the transform (const version).
		* @return The current position as a const glm::vec3&.
		*/
		SHADE_INLINE const glm::vec3& GetPosition() const
		{
			return m_Position;
		}

		/**
		 * @brief Sets the rotation of the transform using Euler angles in radians.
		 * @param rotation The new rotation as a glm::vec3 in radians.
		 */
		SHADE_INLINE void SetRotation(float x, float y, float z)
		{
			m_RotationQuat = glm::quat(glm::vec3(x, y, z));
		}
		/**
		 * @brief Sets the rotation of the transform using Euler angles in degrees.
		 * @param rotation The new rotation as a glm::vec3 in degrees.
		 */
		SHADE_INLINE void SetRotationDegrees(float x, float y, float z)
		{
			m_RotationQuat = glm::quat(glm::radians(glm::vec3(x, y, z)));
		}

		/**
		 * @brief Gets the current rotation of the transform in Euler angles radians.
		 * @return The current rotation as a glm::vec3 in radians.
		 */
		SHADE_INLINE void SetRotation(const glm::vec3& rotation)
		{
			m_RotationQuat = rotation;
		}

		/**
		 * @brief Gets the current rotation of the transform in Euler angles (degrees).
		 * @return The current rotation as a glm::vec3 in degrees.
		 */
		SHADE_INLINE void SetRotationDegrees(const glm::vec3& rotation)
		{
			m_RotationQuat = glm::quat(glm::radians(rotation));
		}
		/**
		 * @brief Set the current rotation of the transform in Euler angles (degrees).
		 * @return The current rotation as a glm::vec3 in degrees.
		 */
		SHADE_INLINE void SetRotationDegrees(const glm::quat& rotation)
		{
			m_RotationQuat = rotation;
		}
		/**
		* @brief Gets the current rotation of the transform in radians.
		* @return The current rotation as a glm::vec3 in radians.
		*/
		SHADE_INLINE glm::vec3 GetRotation() const
		{
			return glm::eulerAngles(m_RotationQuat);
		}
		/**
		* @brief Gets the current rotation of the transform in Euler angles (degrees).
		* @return The current rotation as a glm::vec3 in degrees.
		*/
		SHADE_INLINE glm::vec3 GetRotationDegrees() const
		{
			return glm::degrees(glm::eulerAngles(m_RotationQuat));
		}

		/**
		* @brief Sets the rotation of the transform using a quaternion.
		* @param quaternion The new rotation as a glm::quat.
		*/
		SHADE_INLINE void SetRotation(const glm::quat& quaternion)
		{
			m_RotationQuat = quaternion;
		}
		/**
		 * @brief Gets the current rotation of the transform as a quaternion.
		 * @return The current rotation as a glm::quat.
		 */
		SHADE_INLINE glm::quat& GetRotationQuaternion()
		{
			return m_RotationQuat;
		}
		/**
		* @brief Gets the current rotation of the transform as a quaternion (const version).
		* @return The current rotation as a const glm::quat&.
		*/
		SHADE_INLINE const glm::quat& GetRotationQuaternion() const
		{
			return m_RotationQuat;
		}

		/**
		 * @brief Sets the scale of the transform.
		 * @param x Scale along the X-axis.
		 * @param y Scale along the Y-axis.
		 * @param z Scale along the Z-axis.
		 */
		SHADE_INLINE void SetScale(float x, float y, float z)
		{
			m_Scale.x = x; m_Scale.y = y; m_Scale.z = z;
		}

		/**
		 * @brief Sets the scale of the transform.
		 * @param scale The new scale as a glm::vec3.
		 */
		SHADE_INLINE void SetScale(const glm::vec3& scale)
		{
			m_Scale = scale;
		}

		/**
		 * @brief Gets the current scale of the transform.
		 * @return The current scale as a glm::vec3.
		 */
		SHADE_INLINE glm::vec3& GetScale()
		{
			return m_Scale;
		}

		/**
		* @brief Gets the current scale of the transform (const version).
		* @return The current scale as a const glm::vec3&.
		*/
		SHADE_INLINE const glm::vec3& GetScale() const
		{
			return m_Scale;
		}

		/**
		 * @brief Rotates the transform by the specified Euler angles in degrees.
		 * @param x Rotation around the X-axis in degrees.
		 * @param y Rotation around the Y-axis in degrees.
		 * @param z Rotation around the Z-axis in degrees.
		 */
		SHADE_INLINE void Rotate(float x, float y, float z)
		{
			Rotate(glm::quat(glm::vec3(x, y, z)));
		}
		/**
		 * @brief Rotates the transform by the specified Euler angles in degrees.
		 * @param rotation The rotation to apply as a glm::vec3 in degrees.
		 */
		SHADE_INLINE void Rotate(const glm::vec3& rotation)
		{
			Rotate(glm::quat(rotation));
		}

		SHADE_INLINE void RotateDegrees(const glm::vec3& rotation)
		{
			Rotate(glm::quat(glm::radians(rotation)));
		}

		/**
		* @brief Rotates the transform by the specified quaternion.
		* @param quaternion The quaternion representing the rotation to apply.
		*/
		SHADE_INLINE void Rotate(const glm::quat& quaternion)
		{
			m_RotationQuat = glm::normalize(m_RotationQuat * quaternion);
		}

		/**
		 * @brief Moves the transform by the specified amount.
		 * @param x Movement along the X-axis.
		 * @param y Movement along the Y-axis.
		 * @param z Movement along the Z-axis.
		 */
		SHADE_INLINE void Move(float x, float y, float z)
		{
			m_Position.x += x; m_Position.y += y; m_Position.z += z;
		}

		/**
		* @brief Moves the transform by the specified amount.
		* @param position The movement to apply as a glm::vec3.
		*/
		SHADE_INLINE void Move(const glm::vec3& position)
		{
			m_Position += position;
		}

		/**
		 * @brief Sets the forward direction of the transform.
		 * @param direction The new forward direction.
		 * @param forwardDirection The reference forward direction, default is (0, 0, 1).
		 */
		SHADE_INLINE void SetDirection(const glm::vec3& direction, const glm::vec3& upDirection = glm::vec3(0.f, 1.f, 0.f))
		{
			m_RotationQuat = glm::quatLookAt(glm::normalize(direction), upDirection); // Assumes up is always (0,1,0)
		}

		/**
		 * @brief Gets the current forward direction of the transform.
		 * @param forwardDirection The reference forward direction, default is (0, 0, 1, 1).
		 * @return The current forward direction as a glm::vec3.
		 */
		SHADE_INLINE glm::vec3 GetForwardDirection(const glm::vec4& worldForwardDirection = glm::vec4(0.f, 0.f, 1.f, 1.f)) const
		{
			// TIP: Not sure if there should be normilize !
			return glm::normalize(GetRotationQuaternion() * worldForwardDirection);
		}

		/**
		 * @brief Calculates the pitch angle (rotation around the X-axis) from the current quaternion rotation.
		 * @return The pitch angle in radians.
		 */
		SHADE_INLINE float GetPitch() const
		{
			// Calculate the pitch angle in radians
			return glm::pitch(m_RotationQuat);
		}

		/**
		 * @brief Calculates the yaw angle (rotation around the Y-axis) from the current quaternion rotation.
		 * @return The yaw angle in radians.
		 */
		SHADE_INLINE float GetYaw() const
		{
			// Calculate the roll angle in radians
			return glm::yaw(m_RotationQuat);
		}
		/**
		 * @brief Calculates the roll angle (rotation around the Z-axis) from the current quaternion rotation.
		 * @return The roll angle in radians.
		 */
		SHADE_INLINE float GetRoll() const
		{
			// Calculate the yaw angle in radians
			return glm::roll(m_RotationQuat);
		}

		/**
		 * @brief Gets the model matrix representing the transform.
		 * @return The model matrix as a glm::mat4.
		 */
		SHADE_INLINE glm::mat4 GetModelMatrix() const
		{
			return glm::translate(glm::mat4(1.f), m_Position) * glm::toMat4(m_RotationQuat) * glm::scale(glm::mat4(1.f), m_Scale);
		}

		/**
		 * @brief Sets the transform from a given transformation matrix.
		 * @param matrix The transformation matrix to decompose.
		 */
		SHADE_INLINE void SetTransformMatrix(const glm::mat4& matrix)
		{
			math::DecomposeMatrix(matrix, m_Position, m_RotationQuat, m_Scale);
		}

		/**
		 * @brief Creates a transform from a given transformation matrix.
		 * @param matrix The transformation matrix to decompose.
		 * @return A Transform object with the decomposed position, rotation, and scale.
		 */
		SHADE_INLINE static Transform GetTransformFromMatrix(const glm::mat4& matrix)
		{
			Transform transform;
			math::DecomposeMatrix(matrix, transform.m_Position, transform.m_RotationQuat, transform.m_Scale);
			return transform;
		}

	private:
		glm::vec3 m_Position; ///< The position of the transform.
		glm::quat m_RotationQuat; ///< The rotation of the transform stored as a quaternion.
		glm::vec3 m_Scale; ///< The scale of the transform.

	private:
		friend class serialize::Serializer;
		/**
		* @brief Serializes the transform data to the provided output stream.
		* @param stream The output stream to serialize to.
		*/
		void Serialize(std::ostream& stream) const;
		/**
		 * @brief Deserializes the transform data from the provided input stream.
		 * @param stream The input stream to deserialize from.
		 */
		void Deserialize(std::istream& stream);
	};

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
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Transform& transform)
	{
		transform.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Transform& transform)
	{
		transform.Deserialize(stream);
	}
}
