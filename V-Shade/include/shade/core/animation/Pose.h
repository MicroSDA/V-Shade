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
			Pose(const Asset<Skeleton>& skeleton, std::size_t animationHash = 0, Type initianType = Type::DontKnow1);
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
			inline std::size_t GetAnimationHash() const { return m_AnimationCombinationHash; }
			inline float GetCurrentPlayTime() const { return m_CurrentPlayTime; }
			inline void SetCurrentPlayTime(float time) { m_CurrentPlayTime = time; }

		private:

		private:
			Asset<Skeleton> m_Skeleton;
			std::size_t m_AnimationCombinationHash;
			SharedPointer<std::vector<glm::mat4>> m_GlobalTransforms;
			SharedPointer<std::vector<glm::mat4>> m_LocalTransforms;
			State m_State;
			float m_CurrentPlayTime = 0.f;
		};
	}
}
