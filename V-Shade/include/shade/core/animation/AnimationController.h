#pragma once
#include <shade/core/animation/Animation.h>
#include <shade/core/time/Timer.h>
#include <shade/core/animation/Skeleton.h>
#include <shade/core/threads/ThreadPool.h>
#include <shade/core/animation/Pose.h>
#include <shade/core/arrays/Arrays.h>

namespace shade
{

	/* 1. Create Pose task 
	
	*/
	namespace animation
	{
		template<typename... Args>
		std::size_t PointerHashCombine(Args&&... args)
		{
			return (static_cast<std::size_t>(std::decay_t<Args>(std::forward<Args>(args))) ^ ...);
		}

		class SHADE_API AnimationController
		{
		public:
		public:
			struct AnimationControlData
			{
				AnimationControlData() = default;
				AnimationControlData(const Asset<Animation>& animation) :
					Animation(animation),
					Start(0.f),
					End(animation->GetDuration()),
					Duration(animation->GetDuration()),
					CurrentPlayTime(0.f),
					TicksPerSecond(animation->GetTiksPerSecond())
				{}

				Asset<Animation>		Animation;
				Animation::State		State = Animation::State::Stop;
				float					Start = 0.f, End = 0.f, Duration = 0.f, CurrentPlayTime = 0.f, TicksPerSecond = 0.f;
			};
			template<typename T, typename Data>
			struct Task
			{
				// Do we use this anywhere ?
				enum Status
				{
					None,
					Start,
					InProcess,
					Pause,
					End
				};

				Data Data;
				std::function<T> Process;
			};
		public:
			static SharedPointer<AnimationController> Create();
		public:
			virtual ~AnimationController() = default;

			animation::Pose* ProcessPose(const Asset<Skeleton>& skeleton, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier = 1.f);
			animation::Pose* Blend(const Asset<Skeleton>& skeleton, const animation::Pose* first, const animation::Pose* second, float blendFactor, const animation::BoneMask& boneMask);
			std::pair<float, float> GetTimeMultiplier(float firstDuration, float secondDuration, float blendFactor) const;
		private:
			AnimationController() = default;
			std::unordered_map<std::size_t, animation::Pose> m_Poses; //NOTE: Thats can be a problem !!
			
			friend class SharedPointer<AnimationController>;
		private:
			animation::Pose* CreatePose(const Asset<Skeleton>& skeleton, std::size_t hash);
			animation::Pose* CalculatePose(animation::Pose* targetPose, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier = 1.f);
			animation::Pose* ReceiveAnimationPose(const Asset<Skeleton>& skeleton, std::size_t hash);

			template<typename... Args>
			inline animation::Pose* ReceiveAnimationPose(const Asset<Skeleton>& skeleton, Args&&... args)
			{
				(CreatePose(skeleton, animation::PointerHashCombine(args)), ...);
				return CreatePose(skeleton, animation::PointerHashCombine(std::forward<Args>(args)...));
			}
		
			void CalculateBoneTransforms(
				animation::Pose* pose,
				const AnimationControlData& animationData,
				const Skeleton::BoneNode* bone,
				const glm::mat4& parentTransform,
				const Skeleton::BoneArmature& armature);

			void CalculateBoneTransformsBlend(
				const shade::Skeleton::BoneNode* bone,
				animation::Pose* targetPose,
				const animation::Pose* first,
				const animation::Pose* second,
				const glm::mat4& parrentTransform,
				float blendFactor, 
				const animation::BoneMask& boneMask);
		};
	}
}