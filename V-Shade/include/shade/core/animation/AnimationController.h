#pragma once
#include <shade/core/animation/Animation.h>
#include <shade/core/time/Timer.h>
#include <shade/core/animation/Skeleton.h>

namespace shade
{
	/*
		Get current state

		Get current state -> blend with adition state (From ->Pose(Deffined by time), To->Pose(Deffined by time), blend factor)
		Get current state -> blend with adition state (From ->Pose(Deffined by time), To->Pose(Deffined by time), blend factor)
			---- Expand to:
			Play current situation, Sycled animation, Blended animation
			Current sitiation contans curent Pose(transorms)

			Can we play Pose one(calc all transforms current animation time) Play Pose two(calc all transforms ant current animation time) and blend them at end
			Do we need to use From and Till = some time or some Key frame exmpl:
			Blend animations: source from -> specific key frame, till -> specific key frame - > target from -> specific key frame, till -> specific key frame
			Or just use time as from and till positions ??

		Walk + Jump - > Get Current state + blend between Walk

		Wakl

		Stay + Jump

		Stay


		and so on....

	*/

	namespace animation
	{
		template<typename... Args>
		std::size_t PointerHashCombine(Args&&... args)
		{
			return (static_cast<std::size_t>(std::decay_t<Args>(std::forward<Args>(args))) ^ ...);
		}

		class SHADE_API Pose
		{
		public:
			enum class Type
			{
				DontKnow1,
				DontKnow2,
			};
			enum class State // Copied from Esoterica
			{
				Unset = 0,
				Pose,
				ReferencePose,
				ZeroPose,
				AdditivePose
			};
		public:
			Pose(const Asset<Skeleton>& skeleton, Type initianType = Type::DontKnow1);
			Pose(Type initianType = Type::DontKnow1);
			virtual ~Pose() = default;
		public:
			void Reset();
			inline const SharedPointer<std::vector<glm::mat4>>& GetBoneGlobalTransforms() const { return m_GlobalTransforms; }
			inline SharedPointer<std::vector<glm::mat4>>& GetBoneGlobalTransforms() { return m_GlobalTransforms; }
			inline const SharedPointer<std::vector<glm::mat4>>& GetBoneLocalTransforms() const { return m_LocalTransforms; }
			inline SharedPointer<std::vector<glm::mat4>>& GetBoneLocalTransforms() { return m_LocalTransforms; }


			inline const glm::mat4& GetBoneGlobalTransform(std::size_t index) const { assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }
			inline glm::mat4& GetBoneGlobalTransform(std::size_t index) { assert(index < m_GlobalTransforms->size()); return m_GlobalTransforms.Get()[index]; }

			inline const glm::mat4& GetBoneLocalTransform(std::size_t index) const { assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }
			inline glm::mat4& GetBoneLocalTransform(std::size_t index) { assert(index < m_LocalTransforms->size()); return m_LocalTransforms.Get()[index]; }

			inline State GetState() const { return m_State; }
			inline const Asset<Skeleton>& GetSkeleton() const { return m_Skeleton; }
			inline float GetCurrentPlayTime() const { return m_CurrentPlayTime; }
			inline void SetCurrentPlayTime(float time) { m_CurrentPlayTime = time; }
		private:

		private:
			Asset<Skeleton> m_Skeleton;
			SharedPointer<std::vector<glm::mat4>> m_GlobalTransforms;
			SharedPointer<std::vector<glm::mat4>> m_LocalTransforms;
			State m_State;
			float m_CurrentPlayTime = 0.f;
		};

		struct BoneMask
		{
			BoneMask(const Asset<Skeleton>& skeleton)
			{
				if (skeleton)
				{
					for (auto& [name, bone] : skeleton->GetBones())
						Weights.emplace(bone->ID, std::pair<std::string, float>{ name, 1.0f });
				}
			}
			~BoneMask() = default;
			void Reset() 
			{
				for (auto& [id, mask] : Weights)
					mask.second = 1.0f;
			}
			std::unordered_map<std::size_t, std::pair<std::string,float>> Weights;
		};
	}

	class SHADE_API AnimationController
	{
	public:
		template<typename T, typename Data>
		struct Task
		{
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
		static SharedPointer<AnimationController> Create();
		virtual ~AnimationController() = default;

		void AddAnimation(const Asset<Animation>& animation);
		// Set animation as current + set state
		void SetCurrentAnimation(const Asset<Animation>& animation, Animation::State state = Animation::State::Stop);
		void SetCurrentAnimation(const std::string& name, Animation::State state = Animation::State::Stop);

		void SetAnimationDuration(const Asset<Animation>& animation, float duration);
		void SetAnimationDuration(const std::string& name, float duration);

		float& GetAnimationDuration(const Asset<Animation>& animation);
		float& GetAnimationDuration(const std::string& name);

		void SetAnimationTiks(const Asset<Animation>& animation, float tiksPerSekond);
		void SetAnimationTiks(const std::string& name, float tiksPerSekond);

		float& GetAnimationTiks(const Asset<Animation>& animation);
		float& GetAnimationTiks(const std::string& name);

		void SetAnimationState(const Asset<Animation>& animation, Animation::State state);
		void SetAnimationState(const std::string& name, Animation::State state);

		float& GetCurrentAnimationTime(const Asset<Animation>& animation);
		float& GetCurrentAnimationTime(const std::string& name);

		void RemoveAnimation(const Asset<Animation>& animation);
		void RemoveAnimation(const std::string& name);

		bool IsAnimationExists(const Asset<Animation>& animation) const;

		Animation::State GetAnimationState(const Asset<Animation>& animation) const;
		Animation::State GetAnimationState(const std::string& name) const;

		Asset<Animation> GetCurentAnimation();

		std::unordered_map<Asset<Animation>, AnimationControllData>& GetAnimations();

		void ProcessPose(const Asset<Animation>& animation, float from, float till);
		void ProcessPose(const Asset<Animation>& first, float firstFrom, float firstTill, const Asset<Animation>& second, float secondFrom, float secondTill, float blendFactor, bool isSync = false, const animation::BoneMask& boneMask = animation::BoneMask{nullptr});
		SharedPointer<animation::Pose> QuerryPose(const Asset<Skeleton>& skeleton, const FrameTimer& deltaTime);
	public:
		std::unordered_map<Asset<Animation>, AnimationControllData>::iterator begin() noexcept { return m_Animations.begin(); };
		std::unordered_map<Asset<Animation>, AnimationControllData>::iterator end() noexcept { return m_Animations.end(); };
		std::unordered_map<Asset<Animation>, AnimationControllData>::const_iterator cbegin() const noexcept { return m_Animations.begin(); };
		std::unordered_map<Asset<Animation>, AnimationControllData>::const_iterator cend() const noexcept { return m_Animations.end(); };
	private:
		AnimationController();

		void CalculateBoneTransforms(
			SharedPointer<animation::Pose>& pose,
			const Asset<Animation>& animation, 
			const SharedPointer<Skeleton::BoneNode>& bone, 
			const glm::mat4& parentTransform, 
			const SharedPointer<Skeleton::BoneArmature>& armature);

		void CalculateBoneTransformsBlend(
			const shade::SharedPointer<shade::Skeleton::BoneNode>& bone,
			shade::SharedPointer<shade::animation::Pose>& targetPose,
			const shade::SharedPointer<shade::animation::Pose>& first,
			const shade::SharedPointer<shade::animation::Pose>& second, const glm::mat4& parrentTransform, float blendFactor, const animation::BoneMask& boneMask);

		std::pair<float, float> GetTimeMultiplier(float firstDuration, float secondDuration, float blendFactor) const;


		// Where std::string is asset id 
		std::unordered_map<Asset<Animation>, AnimationControllData> m_Animations; // Move to Animation graphs
		// Move to Animation graphs
		AnimationControllData* m_CurrentAnimation = nullptr;

		// Animation combintion hash - > animation pose 
		std::unordered_map<std::size_t, SharedPointer<animation::Pose>> m_Poses;
	
		SharedPointer<animation::Pose> m_DefaultPose;

		std::function<SharedPointer<animation::Pose>(const Asset<Skeleton>&, const FrameTimer&)> m_QuerryPose;
	private:
		friend class SharedPointer<AnimationController>;

		SharedPointer<animation::Pose>& CreatePose(std::size_t hash, const Asset<Skeleton>& skeleton);
		SharedPointer<animation::Pose>& ReceiveAnimationPose(const Asset<Skeleton>& skeleton, const Asset<Animation>& animation);
		SharedPointer<animation::Pose>& CalculatePose(SharedPointer<animation::Pose>& targetPose, const Asset<Animation>& animation, float from, float till, const FrameTimer& deltaTime, float timeMultiplier = 1.f);
		SharedPointer<animation::Pose>& Blend(SharedPointer<animation::Pose>& targetPose, SharedPointer<animation::Pose>& first, SharedPointer<animation::Pose>& second, float blendFactor, const animation::BoneMask& boneMask);
		template<typename... Args>
		inline SharedPointer<animation::Pose>& ReceiveAnimationPose(const Asset<Skeleton>& skeleton, Args&... args)
		{
			(CreatePose(animation::PointerHashCombine(args), skeleton), ...);
			return CreatePose(animation::PointerHashCombine(std::forward<Args>(args)...), skeleton);
		}
	};
}