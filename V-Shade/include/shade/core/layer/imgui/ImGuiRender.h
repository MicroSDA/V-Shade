#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/image/Texture.h>
#include <ImGui/imgui.h>

namespace shade
{
	class SHADE_API ImGuiRender
	{
	public:
		virtual ~ImGuiRender() = default;
		static SharedPointer<ImGuiRender> Create();

		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;
		virtual void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor) = 0;
		virtual void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t mip) = 0;
		virtual void DrawImage(Asset<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor) = 0;
		ImGuiContext* GetImGuiContext();

		
		template<typename T>
		T& As();
	private:

	protected:
		ImGuiContext* m_ImGuiContext;

	};
}
template<typename T>
inline T& shade::ImGuiRender::As()
{
	static_assert(std::is_base_of<ImGuiRender, T>::value, "");
	return static_cast<T&>(*this);
}
