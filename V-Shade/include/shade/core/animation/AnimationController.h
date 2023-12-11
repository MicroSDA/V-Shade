#pragma once
#include <shade/core/animation/Animation.h>
#include <shade/core/time/Timer.h>
#include <shade/core/animation/Skeleton.h>

namespace shade
{
	class SHADE_API AnimationController
	{
		struct AnimationControllData
		{
			Asset<Animation> Animation;
			Animation::State State = Animation::State::Stop;
			float CurrentPlayTime = 0.0;
			float Duration = 0.0;
			float TiksPerSecond = 0.0;
			SharedPointer<std::vector<glm::mat4>> BoneTransforms;
		};

	public:
		
	public:
		virtual ~AnimationController() = default;

		void AddAnimation(const Asset<Animation>& animation);
		// Set animation as current + set state
		void SetCurrentAnimation(const Asset<Animation>& animation, Animation::State state = Animation::State::Stop);
		void SetCurrentAnimation(const std::string& name, Animation::State state = Animation::State::Stop);

		void SetAnimationDuration(const Asset<Animation>& animation, float duration);
		void SetAnimationDuration(const std::string& name, float duration);

		float GetAnimationDuration(const Asset<Animation>& animation) const;
		float GetAnimationDuration(const std::string& name) const ;

		void SetAnimationTiks(const Asset<Animation>& animation, float tiksPerSekond);
		void SetAnimationTiks(const std::string& name, float tiksPerSekond);

		float GetAnimationTiks(const Asset<Animation>& animation) const;
		float GetAnimationTiks(const std::string& name) const;

		AnimationControllData& GetCurentAnimation();

		void UpdateCurrentAnimation(const FrameTimer& deltaTime, const Asset<Skeleton>& skeleton);

		// return bone transforms at current animation
		const SharedPointer<std::vector<glm::mat4>>& GetBoneTransforms() const;

		static SharedPointer<AnimationController> Create();
	private:
		AnimationController() = default;

		void CalculateBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature);

		// Where std::string is asset id
		std::unordered_map<std::string, AnimationControllData> m_Animation;
		AnimationControllData* m_CurrentAnimation = nullptr;
	private:
		friend class SharedPointer<AnimationController>;
	};
}