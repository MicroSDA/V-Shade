#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/animation/Skeleton.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	namespace animation
	{
		class AnimationController;

		class SHADE_API Pose
		{
		public:

			struct LocalTransform
			{
				glm::vec3 Translation	= glm::vec3(0.f);
				glm::quat Rotation		= glm::identity<glm::quat>();
				glm::vec3 Scale			= glm::vec3(0.f);
			};

			using GlobalTransform		= glm::mat4;

			struct SHADE_API RootMotion
			{
				template<typename T>
				struct Motion
				{
					T Start;
					T End;
					T Current;
					T Delta;
				};

				Skeleton::BoneID  RootBone		= 0u;
				Motion<glm::vec3> Translation	= { glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f) };
				Motion<glm::quat> Rotation		= { glm::identity<glm::quat>(), glm::identity<glm::quat>(), glm::identity<glm::quat>(), glm::identity<glm::quat>() };
				Motion<glm::vec3> Scale			= { glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f) };
			
				glm::vec3 GetTranlsationDifference() const;

				
				
				SHADE_INLINE glm::quat GetRotationDifference() const
				{
					//return glm::inverse(Rotation.Delta) * Rotation.Current;
					return glm::conjugate(Rotation.Delta) * Rotation.Current;
				}

				SHADE_INLINE glm::vec3 GetScaleDifference() const
				{
					return Scale.Current - Scale.Delta;
				}

				SHADE_INLINE void Reset()
				{
					*this = RootMotion{};
				}

				void FinalizeRootMotion(const Asset<Skeleton>& skeleton, const Asset<Animation>& aniamtion, float start, float end);
			};

			enum class Type : std::size_t 
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
			SHADE_INLINE const	SharedPointer<std::vector<GlobalTransform>>& GetBoneGlobalTransforms()		const		{ return m_GlobalTransforms; }
			SHADE_INLINE		SharedPointer<std::vector<GlobalTransform>>& GetBoneGlobalTransforms()					{ return m_GlobalTransforms; }
			SHADE_INLINE const	SharedPointer<std::vector<LocalTransform>>& GetBoneLocalTransforms()			const	{ return m_LocalTransforms; }
			SHADE_INLINE		SharedPointer<std::vector<LocalTransform>>& GetBoneLocalTransforms()					{ return m_LocalTransforms; }

			SHADE_INLINE const	GlobalTransform& GetBoneGlobalTransform(std::size_t index)					const	{ assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }
			SHADE_INLINE		GlobalTransform& GetBoneGlobalTransform(std::size_t index)							{ assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }

			SHADE_INLINE const	LocalTransform& GetBoneLocalTransform(std::size_t index)						const	{ assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }
			SHADE_INLINE		LocalTransform& GetBoneLocalTransform(std::size_t index)								{ assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }

			SHADE_INLINE		Type GetType()															const	{ return m_Type; }
			SHADE_INLINE const	Asset<Skeleton>& GetSkeleton()											const	{ return m_Skeleton; }
			SHADE_INLINE		std::size_t GetAnimationHash()											const	{ return m_AnimationCombinationHash; }

			SHADE_INLINE 		void SetDuration(float time)													{ m_Duration = time; }
			SHADE_INLINE 		void SetCurrentPlayTime(float time)												{ m_CurrentPlayTime = time; }

			SHADE_INLINE 		float GetDuration()														const   { return m_Duration; }
			SHADE_INLINE 		float GetCurrentPlayTime()												const   { return m_CurrentPlayTime; }

			SHADE_INLINE RootMotion* GetRootMotion()
			{
				return m_RootMotion;
			}

			SHADE_INLINE const RootMotion* GetRootMotion() const
			{
				return m_RootMotion;
			}

			SHADE_INLINE void SetRootMotion(RootMotion* motion)
			{
				m_RootMotion = motion;
			}

			SHADE_INLINE bool HasRootMotion() const
			{
				return (m_RootMotion != nullptr);
			}
		private:
			Asset<Skeleton>								m_Skeleton;
			std::size_t									m_AnimationCombinationHash;
			SharedPointer<std::vector<GlobalTransform>>	m_GlobalTransforms;
			SharedPointer<std::vector<LocalTransform>>	m_LocalTransforms;

			Type										m_Type;
			float										m_Duration;
			float										m_CurrentPlayTime;
			bool										m_HasRootMotion = false;
			RootMotion*									m_RootMotion = nullptr;
		};
	}
}
