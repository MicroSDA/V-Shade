#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/rotate_vector.hpp>
#include <shade/utils/Logger.h>
#include <shade/core/camera/CameraFrustum.h>
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	class SHADE_API Camera
	{
	public:
		struct RenderData
		{
			glm::mat4 ViewProjection;
			glm::mat4 View;
			glm::mat4 Projection;
			alignas(16) glm::vec3 Position;
			alignas(16) glm::vec3 Forward;
			float Near;
			float Far;
		};
	public:
		Camera();
		/**
		 * @brief Constructor for Camera.
		 * @param position Initial position of the camera.
		 * @param fov Field of view in degrees.
		 * @param aspect Aspect ratio of the camera.
		 * @param zNear Near clipping plane distance.
		 * @param zFar Far clipping plane distance.
		 */
		Camera(const glm::vec3& position,
			float fov,
			float aspect,
			float zNear,
			float zFar);
		virtual ~Camera() = default;

		/**
		 * @brief Get the view matrix.
		 * @return View matrix.
		 */
		SHADE_INLINE glm::mat4 GetView() const
		{
			return glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
		}

		/**
		 * @brief Get the projection matrix.
		 * @return Projection matrix.
		 */
		SHADE_INLINE const glm::mat4& GetProjection() const
		{
			return m_Perpective;
		}

		/**
		 * @brief Get the combined view-projection matrix.
		 * @return View-projection matrix.
		 */
		SHADE_INLINE glm::mat4 GetViewProjection() const
		{
			return m_Perpective * GetView();
		}
		
		/**
		 * @brief Get a modifiable reference to the projection matrix.
		 * @return Reference to the projection matrix.
		 */
		SHADE_INLINE glm::mat4& GetProjection()
		{
			return m_Perpective;
		}

		/**
		 * @brief Get a modifiable reference to the forward direction.
		 * @return Reference to the forward direction vector.
		 */
		SHADE_INLINE glm::vec3& GetForwardDirection()
		{
			return m_Forward;
		}

		/**
		 * @brief Get a constant reference to the forward direction.
		 * @return Constant reference to the forward direction vector.
		 */
		SHADE_INLINE const glm::vec3& GetForwardDirection() const
		{
			return m_Forward;
		}

		/**
		 * @brief Get a modifiable reference to the up direction.
		 * @return Reference to the up direction vector.
		 */
		SHADE_INLINE glm::vec3& GetUpDirection()
		{
			return m_Up;
		}

		/**
		 * @brief Get a constant reference to the up direction.
		 * @return Constant reference to the up direction vector.
		 */
		SHADE_INLINE const glm::vec3& GetUpDirection() const
		{
			return m_Up;
		}

		/**
		 * @brief Get a modifiable reference to the position.
		 * @return Reference to the position vector.
		 */
		SHADE_INLINE glm::vec3& GetPosition()
		{
			return m_Position;
		}

		/**
		 * @brief Get a constant reference to the position.
		 * @return Constant reference to the position vector.
		 */
		SHADE_INLINE const glm::vec3& GetPosition() const
		{
			return m_Position;
		}
	
		/**
		 * @brief Set the camera view using a view matrix.
		 * @param view The view matrix to set.
		 */
		SHADE_INLINE void SetVeiw(const glm::mat4& view)
		{
			glm::mat4 matrix	= GetProjection() * glm::inverse(view);
			//glm::vec2 right	= glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0]);
			m_Forward			= glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]);
			m_Up				= glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1]);
			m_Position			= glm::vec3(view[3][0], view[3][1], view[3][2]);
		}
		
		/**
		 * @brief Get the field of view (FOV).
		 * @return FOV in degrees.
		 */
		SHADE_INLINE const float& GetFov() const
		{
			return m_Fov;
		}

		/**
		 * @brief Get a modifiable reference to the field of view (FOV).
		 * @return Reference to the FOV in degrees.
		 */
		SHADE_INLINE float& GetFov()
		{
			return m_Fov;
		}
		
		/**
		 * @brief Get the aspect ratio.
		 * @return Aspect ratio.
		 */
		SHADE_INLINE const float& GetAspect() const
		{
			return m_Aspect;
		}

		/**
		 * @brief Get a modifiable reference to the aspect ratio.
		 * @return Reference to the aspect ratio.
		 */
		SHADE_INLINE float& GetAspect()
		{
			return m_Aspect;
		}
		
		/**
		 * @brief Get the near clipping plane distance.
		 * @return Near clipping plane distance.
		 */
		SHADE_INLINE const float& GetNear() const
		{
			return m_zNear;
		}

		/**
		 * @brief Get a modifiable reference to the near clipping plane distance.
		 * @return Reference to the near clipping plane distance.
		 */
		SHADE_INLINE float& GetNear()
		{
			return m_zNear;
		}
		
		/**
		 * @brief Get the far clipping plane distance.
		 * @return Far clipping plane distance.
		 */
		SHADE_INLINE const float& GetFar() const
		{
			return m_zFar;
		}

		/**
		 * @brief Get a modifiable reference to the far clipping plane distance.
		 * @return Reference to the far clipping plane distance.
		 */
		SHADE_INLINE float& GetFar()
		{
			return m_zFar;
		}

		/**
		 * @brief Set the camera position.
		 * @param x X coordinate of the position.
		 * @param y Y coordinate of the position.
		 * @param z Z coordinate of the position.
		 */
		SHADE_INLINE void SetPosition(float x, float y, float z)
		{
			m_Position.x = x; m_Position.y = y; m_Position.z = z;
		}

		/**
		 * @brief Set the camera position.
		 * @param position The new position of the camera.
		 */
		SHADE_INLINE void SetPosition(const glm::vec3& position)
		{
			m_Position = position;
		}
	
		/**
		 * @brief Set the forward direction of the camera.
		 * @param x X component of the forward direction.
		 * @param y Y component of the forward direction.
		 * @param z Z component of the forward direction.
		 */
		SHADE_INLINE void SetForwardDirection(float x, float y, float z)
		{
			m_Forward.x = x; m_Forward.y = y; m_Forward.z = z;
		}
		
		/**
		 * @brief Set the forward direction of the camera.
		 * @param direction The new forward direction.
		 */
		SHADE_INLINE void SetForwardDirection(const glm::vec3& direction) 
		{
			m_Forward = direction;
		}
		
		/**
		 * @brief Move the camera forward by a given value.
		 * @param value Distance to move forward.
		 */
		SHADE_INLINE void MoveForward(float value)
		{
			m_Position += m_Forward * value;
		}
		
		/**
		 * @brief Move the camera backward by a given value.
		 * @param value Distance to move backward.
		 */
		SHADE_INLINE void MoveBackward(float value)
		{
			m_Position -= m_Forward * value;
		}
		
		/**
		 * @brief Move the camera to the right by a given value.
		 * @param value Distance to move right.
		 */
		SHADE_INLINE void MoveRight(float value)
		{
			m_Position -= glm::cross(m_Up, m_Forward) * value;
		}
		
		/**
		 * @brief Move the camera to the left by a given value.
		 * @param value Distance to move left.
		 */
		SHADE_INLINE void MoveLeft(float value)
		{
			m_Position += glm::cross(m_Up, m_Forward) * value;
		}
	
		/**
		 * @brief Set the aspect ratio of the camera.
		 * @param aspect New aspect ratio.
		 */
		SHADE_INLINE void SetAspect(float aspect)
		{
			m_Aspect = aspect; 	RecalculatePerpective();
		}
		
		/**
		 * @brief Set the field of view (FOV) of the camera.
		 * @param fov New field of view in degrees.
		 */
		SHADE_INLINE void SetFov(float fov)
		{
			m_Fov = fov; RecalculatePerpective();
		}
		
		/**
		 * @brief Set the near clipping plane distance.
		 * @param zNear New near clipping plane distance.
		 */
		SHADE_INLINE void SetNear(float zNear)
		{
			m_zNear = zNear; RecalculatePerpective();
		}
		
		/**
		 * @brief Set the far clipping plane distance.
		 * @param zFar New far clipping plane distance.
		 */
		SHADE_INLINE void SetFar(float zFar)
		{
			m_zFar = zFar; RecalculatePerpective();
		}
		
		/**
		 * @brief Rotate the camera around the yaw axis.
		 * @param angle Angle in radians to rotate around the yaw axis.
		 */
		SHADE_INLINE void RotateYaw(float angle)
		{
			// With counterclockwise issue
			// glm::mat4 rotation = glm::rotate(angle, UP);
			// Without counterclockwise issue
			glm::mat4 rotation = glm::rotate(angle, glm::dot(UP, m_Up) < 0.f ? -UP : UP);
			m_Forward = glm::vec3(glm::normalize(rotation * glm::vec4(m_Forward, 0.f)));
			m_Up = glm::vec3(glm::normalize(rotation * glm::vec4(m_Up, 0.f)));
		}
		
		/**
		 * @brief Rotate the camera around the pitch axis.
		 * @param angle Angle in radians to rotate around the pitch axis.
		 */
		SHADE_INLINE void RotatePitch(float angle)
		{
			glm::vec3 right = glm::normalize(glm::cross(m_Up, m_Forward));
			m_Forward = glm::normalize(glm::rotate(-angle, right) * glm::vec4(m_Forward, 0.f));
			m_Up = glm::normalize(glm::cross(m_Forward, right));
		}
		
		/**
		 * @brief Rotate the camera around the roll axis.
		 * @param angle Angle in radians to rotate around the roll axis.
		 */
		SHADE_INLINE void RotateRoll(float angle)
		{
			m_Up = glm::normalize(glm::rotate(-angle, m_Forward) * glm::vec4(m_Up, 0.f));
		}
		
		/**
		 * @brief Rotate the camera around all three axes.
		 * @param x Angle in radians to rotate around the yaw axis.
		 * @param y Angle in radians to rotate around the pitch axis.
		 * @param z Angle in radians to rotate around the roll axis.
		 */
		SHADE_INLINE void Rotate(float x, float y, float z)
		{
			RotateYaw(-x);
			RotatePitch(-y);
			RotateRoll(z);
		}
	
		/**
		 * @brief Rotate the camera around all three axes.
		 * @param angle Vector containing angles in radians for yaw (x), pitch (y), and roll (z).
		 */
		SHADE_INLINE void Rotate(const glm::vec3& angle)
		{
			RotateYaw(-angle.x);
			RotatePitch(-angle.y);
			RotateRoll(angle.z);
		}
		
		/**
		 * @brief Resize the camera perspective.
		 * @param aspect New aspect ratio. If zero, keep the current aspect ratio.
		 */
		SHADE_INLINE void Resize(float aspect = 0)
		{
			(aspect) ? m_Aspect = aspect, RecalculatePerpective() : RecalculatePerpective();
		}
		
		/**
		 * @brief Recalculate the camera perspective matrix.
		 */
		SHADE_INLINE void RecalculatePerpective()
		{
			m_Perpective = glm::perspective(glm::radians(m_Fov), m_Aspect, m_zNear, m_zFar);
			m_Perpective[1][1] *= -1.0f; // Flip Y
		}
		
		/**
		 * @brief Check if the camera is set as the primary camera.
		 * @return True if the camera is primary, false otherwise.
		 */
		SHADE_INLINE bool IsPrimary() const
		{
			return m_IsPrimary;
		}
		
		/**
		 * @brief Set the camera as primary or not.
		 * @param isPrimary True to set the camera as primary.
		 */
		SHADE_INLINE void SetPrimary(bool isPrimary)
		{
			m_IsPrimary = isPrimary;
		}

		/**
		 * @brief Get the yaw angle of the camera.
		 * @return Yaw angle in radians.
		 */
		SHADE_INLINE float GetYaw() const
		{
			return glm::atan(m_Forward.x, m_Forward.z);
		}

		/**
		 * @brief Get the pitch angle of the camera.
		 * @return pitch angle in radians.
		 */
		SHADE_INLINE float GetPitch() const
		{
			return glm::asin(m_Forward.y);
		}

		/**
		 * @brief Get the pitch angle of the camera.
		 * @return roll angle in radians.
		 */
		SHADE_INLINE float GetRoll() const
		{
			return glm::atan(m_Up.x, m_Up.y);
		}

		/**
		 * @brief Get the camera frustum.
		 * @return CameraFrustum object representing the frustum.
		 */
		SHADE_INLINE CameraFrustum GetCameraFrustum() const
		{
			return CameraFrustum(GetViewProjection());
		}

		/**
		 * @brief Get camera render data.
		 * @return RenderData structure containing camera data.
		 */
		SHADE_INLINE RenderData GetRenderData() const 
		{
			return { GetViewProjection(),
					GetView(),
					GetProjection(),
					GetPosition(), 
					GetForwardDirection(), /*-m_Perpective[3][2], m_Perpective[2][2]*/  
					GetNear(), 
					GetFar() }; 
		}
	private:
		const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f); // Y is up
		glm::mat4 m_Perpective;
		glm::vec3 m_Position, m_Forward, m_Up;
		float	m_Fov, m_Aspect, m_zNear, m_zFar;
		// TODO: make it like component
		bool	m_IsPrimary = false;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};
#ifndef CAMERA_DATA_SIZE
	#define CAMERA_DATA_SIZE (sizeof(Camera::RenderData))
#endif // !CAMERA_DATA_SIZE
}

namespace shade
{
	
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Camera& camera)
	{
		return camera.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<Camera>& camera)
	{
		return camera->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Camera& camera)
	{
		return camera.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<Camera>& camera)
	{
		return camera->Deserialize(stream);
	}
}