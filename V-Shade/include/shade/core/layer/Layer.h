#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/utils/Logger.h>
#include <shade/core/event/Event.h>
#include <shade/core/time/Timer.h>
#include <shade/core/render/SceneRenderer.h>
#include <shade/core/render/Renderer.h>
#include <shade/core/components/Components.h>
#include <shade/core/asset/AssetManager.h>
#include <shade/core/physics/PhysicsManager.h>
#include <shade/core/scripting/ScriptManager.h>

// TODO: Add time, scene and so on.
namespace shade
{
	class SHADE_API Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		// On create callback.
		virtual void OnCreate() = 0;
		// On update callback.
		virtual void OnUpdate(SharedPointer<Scene>& scene, const FrameTimer& deltaTime) = 0;
		// On redner begin callback.
		virtual void OnRenderBegin() = 0;
		// On render callback.
		virtual void OnRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime) = 0;
		// On render end callback.
		virtual void OnRenderEnd() = 0;
		// On event callback.
		virtual void OnEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime) = 0;
		// On destroy callback.
		virtual void OnDestroy() = 0;

		// Crate layer.
		template<typename T, typename ...Args>
		static SharedPointer<T> Create(Args&&... args);
		// Get Layer.
		template<typename T>
		static SharedPointer<T> GetLayer();
		// Remove layer
		template<typename T>
		static void RemoveLayer();

		// Activate or diactivate layer.
		void SetActive(const bool& isActive);
		// Activate or diactivate layer for render.
		void SetRender(const bool& isRender);
		// Activate or diactivate layer for update.
		void SetUpdate(const bool& isUpdate);

		const bool& IsActive() const;
		const bool& IsRender() const;
		const bool& IsUpdate() const;

	protected:
		bool m_IsActive = true;
		bool m_IsRender = true;
		bool m_IsUpdate = true;
	private:
		static std::unordered_map<std::type_index, SharedPointer<Layer>> m_sLayers;
		// Internal.
		static void OnLayersUpdate(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
		static void OnLayersRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
		static void OnLayersEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime);
		static void RemoveAllLayers();

		friend class Application;
	};

	template<typename T, typename ...Args>
	inline shade::SharedPointer<T> shade::Layer::Create(Args && ...args)
	{
		static_assert(std::is_base_of<Layer, T>::value, "Is not base of Layer!");
		if (m_sLayers.find(typeid(T)) != m_sLayers.end())
			SHADE_CORE_ERROR("'{0}' already exist!", typeid(T).name());

		auto layer = SharedPointer<T>::Create(args...);
		m_sLayers[typeid(T)] = layer;
		layer->OnCreate();
		return layer;
	}
	template<typename T>
	inline SharedPointer<T> Layer::GetLayer()
	{
		if (m_sLayers.find(typeid(T)) != m_sLayers.end())
			return static_cast<SharedPointer<T>>(m_sLayers[typeid(T)]);
		else
			SHADE_CORE_ERROR("Requested '{0}' has not been found!", typeid(T).name());
	}
	template<typename T> 
	inline void Layer::RemoveLayer()
	{
		auto layer = m_sLayers.find(typeid(T));
		if (layer != m_sLayers.end())
		{
			layer->second->OnDestroy();
			m_sLayers.erase(layer);
		}
		else
			SHADE_CORE_ERROR("Requested '{0}' has not been found!", typeid(T).name());
	}
}
