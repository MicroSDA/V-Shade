#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/render/shader/Shader.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/core/render/vertex/Vertex.h>

namespace shade
{
	class SHADE_API VertexBuffer
	{

		SHADE_CAST_HELPER(VertexBuffer)

	public:
		class Layout
		{
		public:
			enum class Usage : std::uint32_t
			{
				PerVertex,
				PerInstance
			};
			struct Element
			{
				Element() :Name("Undefined"), Type(Shader::DataType::None), Size(0), Offset(0) {};
				Element(const std::string& name, Shader::DataType type, Layout::Usage usage = Layout::Usage::PerVertex)
					: Name(name), Type(type), Size(Shader::GetDataTypeSize(type)), Offset(0) {};

				// TODO: Is name necessary ?
				std::string Name;
				Shader::DataType Type;
				std::uint32_t Size;
				std::uint32_t Offset;
				static std::uint32_t GetComponentCount(Shader::DataType type);
			};
			struct ElementsLayout
			{
				Layout::Usage Usage;
				std::vector<Element> Elements;
			};
		public:
			Layout() = default;
			Layout(const std::initializer_list<ElementsLayout>& elements, Layout::Usage usgae = Layout::Usage::PerVertex) : m_ElementLayouts(elements) { ComputeOffsetAndStride(); }
			~Layout() = default;
		public:
			std::uint32_t GetStride(std::size_t layout);
			std::uint32_t GetCount();
			const std::vector<ElementsLayout>& GetElementLayouts() const;
		private:
			std::vector<ElementsLayout> m_ElementLayouts;
			std::vector<std::uint32_t> m_Strides;
		
			void ComputeOffsetAndStride();
		};
		enum class Usage
		{
			GPU = 0, CPU_GPU = 1
		};
	public:
		virtual ~VertexBuffer() = default;
		static SharedPointer<VertexBuffer> Create(Usage usage, std::uint32_t size, std::size_t resizeThreshold = 0, const void* data = nullptr);

		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t offset = 0) = 0;

		// Resizes the vertex buffer to the given size in bytes if size is more then 0.
		// Min size for resizing is 1.
		virtual void Resize(std::uint32_t size) = 0;
		
		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex, std::uint32_t binding, std::uint32_t offset = 0) const = 0;
		virtual std::uint32_t GetSize() const;
	protected:
		Usage m_Usage = Usage::GPU;
		std::uint32_t m_Size = 0;
		std::uint32_t m_ResizeThreshold = 0;

		bool HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold);
	};
}