#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/animation/Skeleton.h>


namespace shade
{
	namespace animation
	{
		class SHADE_API Pose
		{
		public:
			enum class Type : std::size_t // Copied from Esoterica
			{
				ZeroPose		= 0,
				Pose			= 1,
				AdditivePose	= 2
			};
		public:
			Pose(const Asset<Skeleton>& skeleton, std::size_t animationHash = static_cast<std::size_t>(Type::ZeroPose), Type initianState = Type::ZeroPose);
			virtual ~Pose() = default;
		public:
			void Reset();
			SHADE_INLINE const	SharedPointer<std::vector<glm::mat4>>& GetBoneGlobalTransforms()		const	{ return m_GlobalTransforms; }
			SHADE_INLINE		SharedPointer<std::vector<glm::mat4>>& GetBoneGlobalTransforms()				{ return m_GlobalTransforms; }
			SHADE_INLINE const	SharedPointer<std::vector<glm::mat4>>& GetBoneLocalTransforms()			const	{ return m_LocalTransforms; }
			SHADE_INLINE		SharedPointer<std::vector<glm::mat4>>& GetBoneLocalTransforms()					{ return m_LocalTransforms; }

			SHADE_INLINE const	glm::mat4& GetBoneGlobalTransform(std::size_t index)					const	{ assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }
			SHADE_INLINE		glm::mat4& GetBoneGlobalTransform(std::size_t index)							{ assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }

			SHADE_INLINE const	glm::mat4& GetBoneLocalTransform(std::size_t index)						const	{ assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }
			SHADE_INLINE		glm::mat4& GetBoneLocalTransform(std::size_t index)								{ assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }

			SHADE_INLINE		Type GetType()															const	{ return m_Type; }
			SHADE_INLINE const	Asset<Skeleton>& GetSkeleton()											const	{ return m_Skeleton; }
			SHADE_INLINE		std::size_t GetAnimationHash()											const	{ return m_AnimationCombinationHash; }

			SHADE_INLINE 		void SetDuration(float time)													{ m_Duration = time; }
			SHADE_INLINE 		void SetCurrentPlayTime(float time)												{ m_CurrentPlayTime = time; }

			SHADE_INLINE 		float GetDuration()														const   { return m_Duration; }
			SHADE_INLINE 		float GetCurrentPlayTime()												const   { return m_CurrentPlayTime; }


			void UpdateRootMotion(float tickPerSecond);

			glm::vec3 GetRootMotionTranslationDifference() const;
			glm::vec3 GetRootMotionTranslation() const;
			glm::quat GetRootMotionRotationDifference() const;
			glm::quat GetRootMotionRotation() const;


		private:
			Asset<Skeleton>							m_Skeleton;
			std::size_t								m_AnimationCombinationHash;
			SharedPointer<std::vector<glm::mat4>>	m_GlobalTransforms, m_LocalTransforms;
			Type									m_Type;
			float									m_Duration;
			float									m_CurrentPlayTime;
		public:
			glm::vec3								m_RootMotionTranslation = glm::vec3(0.f);
			glm::vec3								m_RootMotionTranslationDelta = glm::vec3(0.f);
			glm::quat								m_RootMotionRotation = glm::identity<glm::quat>();
			glm::quat								m_RootMotionRotationDelta = glm::identity<glm::quat>();
		};
	}
}
