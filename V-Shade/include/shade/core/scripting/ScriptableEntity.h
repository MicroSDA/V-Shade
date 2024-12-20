#pragma once
#include <shade/core/entity/Entity.h>
#include <shade/core/time/Timer.h>

namespace shade
{
	class Scene;

	namespace scripts { class ScriptManager; }

	namespace ecs
	{
		class SHADE_API ScriptableEntity
		{
		public:
			ScriptableEntity() = default;
			virtual ~ScriptableEntity() = default;
			bool IsUpdate();
			void SetUpdate(bool update);

			template<typename T>
			T& GetComponent();
			template<typename T, typename... Args>
			T& AddComponent(Args&&... args);
			template<typename T>
			bool HasComponent();

			const std::string& GetModuleName() const;
			const std::string& GetName() const;
		protected:
			// Leave them empty
			virtual void OnCreate()											{}
			virtual void OnUpdate(const shade::FrameTimer& deltaTime)		{}
			virtual void OnDesctory()										{}

			Entity& GetEntity() { return m_Entity; }
		private:
			Entity		m_Entity;
			bool		m_IsUpdate = true;
			std::string m_ModuleName;
			std::string m_Name;
		private:
			friend class shade::Scene;
			friend class scripts::ScriptManager;
		};

		inline bool ScriptableEntity::IsUpdate() { return m_IsUpdate; }

		inline void ScriptableEntity::SetUpdate(bool update) { m_IsUpdate = update; }

		template<typename T>
		inline bool ScriptableEntity::HasComponent()
		{
			return m_Entity.HasComponent<T>();
		}
		template<typename T>
		inline T& ScriptableEntity::GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}
		template<typename T, typename ...Args>
		inline T& ScriptableEntity::AddComponent(Args && ...args)
		{
			return m_Entity.AddComponent<T>(std::forward<Args>(args)...);
		}
	}
}
