#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/scripting/ScriptableEntity.h>
#include <shade/core/serializing/Serializer.h>


namespace shade
{
	class Scene;

	namespace ecs
	{
		class SHADE_API NativeScript
		{
		public:
			template<typename T>
			void Bind();
			void Bind(ScriptableEntity* instance)
			{
				DestroyScript = [](NativeScript* nsc) { delete nsc->m_pInstance; nsc->m_pInstance = nullptr; };

				if (m_pInstance)
				{
					DestroyScript(this);
				}

				InstantiateScript = [=]() { return instance; };
			}
			ScriptableEntity* GetIsntace() { return m_pInstance; }
		private:
			ScriptableEntity* m_pInstance = nullptr;
			std::function<ScriptableEntity* ()>	InstantiateScript;
			void (*DestroyScript)(NativeScript*) = nullptr;
		private:
			friend class shade::Scene;
		private:
			friend class Serializer;
			std::size_t Serialize(std::ostream& stream) const;
			std::size_t Desirialize(std::istream& stream);
		};
		template<typename T>
		inline void NativeScript::Bind()
		{
			DestroyScript = [](NativeScript* nsc) { delete nsc->m_pInstance; nsc->m_pInstance = nullptr; };

			if (m_pInstance)
			{
				DestroyScript(this);
			}

			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
		}
	}
}
namespace shade
{
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const ecs::NativeScript& script, std::size_t)
	{
		return script.Serialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, ecs::NativeScript& script, std::size_t)
	{
		return script.Desirialize(stream);
	}
}