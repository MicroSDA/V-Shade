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
		Camera(const glm::vec3& position,
			float fov,
			float aspect,
			float zNear,
			float zFar);
		virtual ~Camera() = default;
		RenderData GetRenderData() const;
		// Get camera view matrix.
		glm::mat4 GetView() const;
		// Get camera view matrix.
		glm::mat4 GetView();
		// Get camera projection matrix.
		const glm::mat4& GetProjection() const;
		// Get camera view * projection matrix.
		glm::mat4 GetViewProjection() const;
		// Get camera forward dirrection.
		const glm::vec3& GetForwardDirection() const;
		// Get camera forward dirrection.
		glm::vec3& GetForwardDirection();
		// Get camera up dirrection.
		const glm::vec3& GetUpDirection() const;
		// Get camera position.
		const glm::vec3& GetPosition() const;
		// Get camera position.
		glm::vec3& GetPosition();
		// Set camera view matrix.
		void SetVeiw(const glm::mat4& view);
		// Get camera filed of view (FOV).
		const float& GetFov() const;
		// Get camera filed of view (FOV).
		float& GetFov();
		// Get camera aspect ratio.
		const float& GetAspect() const;
		// Get camera aspect ratio.
		float& GetAspect();
		// Get camera near distance.
		const float& GetNear() const;
		// Get camera near distance.
		float& GetNear();
		// Get camera far distance.
		const float& GetFar() const;
		// Get camera far distance.
		float& GetFar();
		// Set camera position.
		void SetPosition(float x, float y, float z);
		// Set camera position.
		void SetPosition(const glm::vec3& position);
		// Set camera direction.
		void SetDirection(float x, float y, float z);
		// Set camera direction.
		void SetDirection(const glm::vec3& direction);
		// Set camera direction.
		void MoveForward(float value);
		// Move camera backward.
		void MoveBackward(float value);
		// Move camera right.
		void MoveRight(float value);
		// Move camera left.
		void MoveLeft(float value);
		// Set camera aspect ratio.
		void SetAspect(float aspect);
		// Set camera filed of view (FOV).
		void SetFov(float fov);
		// Set camera near distance.
		void SetNear(float zNear);
		// Set camera far distance.
		void SetFar(float zFar);
		// Rotate camera by y (Yaw).
		void RotateY(float angle);
		// Rotate camera by x (Pitch).
		void RotateX(float angle);
		// Rotate camera by z (Roll).
		void RotateZ(float angle);
		// Rotate camera by xyz.
		void Rotate(float x, float y, float z);
		// Rotate camera by xyz.
		void Rotate(const glm::vec3& angle);
		// TODO Rename resize.
		void Resize(float aspect = 0);
		// Recalculate camera perspective.
		void RecalculatePerpective();
		// Return true if camera set as primary.
		bool IsPrimary() const;
		// Set camera as primary.
		void SetPrimary(bool isPrimary);

		CameraFrustum GetCameraFrustum() const;
	private:
		const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f); // Y is up
		glm::mat4 m_Perpective;
		glm::vec3 m_Position, m_Forward, m_Up;
		float	m_Fov, m_Aspect, m_zNear, m_zFar;
		// TODO: make it like component
		bool	m_IsPrimary = false;
	private:
		friend class Serializer;
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
	};

	inline Camera::RenderData shade::Camera::GetRenderData() const { return { GetViewProjection(), GetView(), GetProjection(), GetPosition(), GetForwardDirection(), /*-m_Perpective[3][2], m_Perpective[2][2]*/  GetNear(), GetFar() }; }
	inline glm::mat4 Camera::GetView() const { return glm::lookAt(m_Position, m_Position + m_Forward, m_Up); }
	inline glm::mat4 Camera::GetView() { return glm::lookAt(m_Position, m_Position + m_Forward, m_Up); }
	inline const glm::mat4& Camera::GetProjection() const { return m_Perpective; }
	inline glm::mat4 Camera::GetViewProjection() const { return m_Perpective * GetView(); }
	inline const glm::vec3& Camera::GetForwardDirection() const { return m_Forward; }
	inline glm::vec3& Camera::GetForwardDirection() { return const_cast<glm::vec3&>(const_cast<const shade::Camera*>(this)->GetForwardDirection()); }
	inline 	const glm::vec3& Camera::GetUpDirection() const { return m_Up; }
	inline const glm::vec3& Camera::GetPosition() const { return m_Position; }
	inline glm::vec3& Camera::GetPosition() { return const_cast<glm::vec3&>(const_cast<const shade::Camera*>(this)->GetPosition()); }
	inline const float& Camera::GetFov() const { return m_Fov; }
	inline float& Camera::GetFov() { return const_cast<float&>(const_cast<const shade::Camera*>(this)->GetFov()); }
	inline const float& Camera::GetAspect() const { return m_Aspect; }
	inline float& Camera::GetAspect() { return const_cast<float&>(const_cast<const shade::Camera*>(this)->GetAspect()); }
	inline const float& Camera::GetNear() const { return m_zNear; }
	inline float& Camera::GetNear() { return const_cast<float&>(const_cast<const shade::Camera*>(this)->GetNear()); }
	inline const float& Camera::GetFar() const { return m_zFar; }
	inline float& Camera::GetFar() { return const_cast<float&>(const_cast<const shade::Camera*>(this)->GetFar()); }
	inline void Camera::SetPosition(float x, float y, float z) { m_Position = glm::vec3(x, y, z); }
	inline void Camera::SetPosition(const glm::vec3& position) { m_Position = position; }
	inline void Camera::SetDirection(float x, float y, float z) { m_Forward = glm::vec3(x, y, z); }
	inline void Camera::SetDirection(const glm::vec3& direction) { m_Forward = direction; }
	inline void Camera::MoveForward(float value) { m_Position += m_Forward * value; }
	inline void Camera::MoveBackward(float value) { m_Position -= m_Forward * value; }
	inline void Camera::MoveRight(float value) { m_Position -= glm::cross(m_Up, m_Forward) * value; }
	inline void Camera::MoveLeft(float value) { m_Position += glm::cross(m_Up, m_Forward) * value; }
	inline void Camera::SetAspect(float aspect) { m_Aspect = aspect; RecalculatePerpective(); }
	inline void Camera::SetFov(float fov) { m_Fov = fov; RecalculatePerpective(); }
	inline void Camera::SetNear(float zNear) { m_zNear = zNear; RecalculatePerpective(); }
	inline void Camera::SetFar(float zFar) { m_zFar = zFar; RecalculatePerpective(); }
	inline void Camera::RecalculatePerpective() 
	{
		m_Perpective = glm::perspective(glm::radians(m_Fov), m_Aspect, m_zNear, m_zFar);
		// Flip Y
		m_Perpective[1][1] *= -1.0f;
	}
	inline bool Camera::IsPrimary() const { return m_IsPrimary; }
	inline void Camera::SetPrimary(bool isPrimary) { m_IsPrimary = isPrimary; }
	inline CameraFrustum Camera::GetCameraFrustum() const { return CameraFrustum(GetViewProjection()); }


#ifndef CAMERA_DATA_SIZE
	#define CAMERA_DATA_SIZE (sizeof(Camera::RenderData))
#endif // !CAMERA_DATA_SIZE
}

namespace shade
{
	
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Camera& camera, std::size_t)
	{
		return camera.Serialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<Camera>& camera, std::size_t)
	{
		return camera->Serialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Camera& camera, std::size_t)
	{
		return camera.Deserialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<Camera>& camera, std::size_t)
	{
		return camera->Deserialize(stream);
	}
}