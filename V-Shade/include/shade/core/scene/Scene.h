#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/entity/ECS.h>
#include <shade/core/memory/Memory.h>
#include <shade/utils/Logger.h>
#include <shade/core/time/Timer.h>
#include <shade/core/components/Components.h>


namespace shade
{
	class SHADE_API Scene : public ecs::EntityManager
	{
	public:
		Scene(const std::string& name);
		virtual ~Scene();

		static SharedPointer<Scene> Create(const std::string& name);
		static SharedPointer<Scene>& GetScene(const std::string& name);
		static SharedPointer<Scene>& GetActiveScene();
		static SharedPointer<Scene>& SetActiveScene(const std::string& name);

		void NativeScriptsUpdate(const shade::FrameTimer& deltaTime);
		void AnimationsUpdate(const shade::FrameTimer& deltaTime);

		ecs::Entity GetPrimaryCamera();
		void OnPlayStart();
		void OnPlaying(const shade::FrameTimer& deltaTime);
		void OnPlayStop();

		const bool& IsPlaying() const;
		void SetPlaying(const bool& play);
		// Set scene as active.
		void SetAsActiveScene();

		glm::mat4 ComputePCTransform(ecs::Entity& entity);

	private:
		std::string m_Name;
		bool        m_IsPlaying;
	private:
		static SharedPointer<Scene> m_sActiveScene;
		static std::unordered_map<std::string, SharedPointer<Scene>> m_sScenes;
	private:
		friend class Serializer;

		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);

		void SerrializeEntity(std::ostream& stream, ecs::Entity entity, bool isParentCall = false) const;
		void DeserrializeEntity(std::istream& stream, ecs::Entity entity, std::uint32_t& index);
	};

	/* Serialize Scene.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Scene& scene, std::size_t)
	{
		return scene.Serialize(stream);
	}
	/* Deserialize Scene.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Scene& scene, std::size_t)
	{
		return scene.Deserialize(stream);
	}
	/* Serialize SharedPointer<Scene>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<Scene>& scene, std::size_t)
	{
		return scene->Serialize(stream);
	}
	/* Deserialize SharedPointer<Scene>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<Scene>& scene, std::size_t)
	{
		return scene->Deserialize(stream);
	}
}
