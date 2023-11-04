#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/render/shader/Shader.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/core/render/vertex/Vertex.h>

namespace shade
{
	class SHADE_API VertexBuffer
	{
	public:
		class Layout
		{
		public:
			enum class Usage
			{
				None = 0,
				PerVertex,
				PerInstance
			};
			struct Element
			{
				Element() :Name("Undefined"), Type(Shader::DataType::None), Usage(Layout::Usage::PerVertex), Size(0), Offset(0) {};
				Element(const std::string& name, Shader::DataType type, Layout::Usage usgae = Layout::Usage::PerVertex)
					: Name(name), Type(type), Usage(usgae), Size(Shader::GetDataTypeSize(type)), Offset(0) {};

				// TODO: Is name necessary ?
				std::string Name;
				Layout::Usage Usage;
				Shader::DataType Type;
				std::uint32_t Size;
				std::uint32_t Offset;
				static std::uint32_t GetComponentCount(Shader::DataType type);
			};
		public:
			Layout() :m_PerVertexStride(0), m_PerInstanceStride(0) {};
			Layout(const std::initializer_list<Element>& elements, Layout::Usage usgae = Layout::Usage::PerVertex) : m_Elements(elements) { ComputeOffsetAndStride(); }
		public:
			std::uint32_t GetStride(Layout::Usage usage);
			std::uint32_t GetCount();
			const std::vector<Element>& GetElements() const;
		private:
			std::vector<Element> m_Elements;
			std::uint32_t m_PerVertexStride, m_PerInstanceStride;
		
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

		template<typename T>
		T& As();
	protected:
		Usage m_Usage = Usage::GPU;
		std::uint32_t m_Size = 0;
		std::uint32_t m_ResizeThreshold = 0;

		bool HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold);
	};
	template<typename T>
	inline T& VertexBuffer::As()
	{
		static_assert(std::is_base_of<VertexBuffer, T>::value, "");
		return static_cast<T&>(*this);
	}
}