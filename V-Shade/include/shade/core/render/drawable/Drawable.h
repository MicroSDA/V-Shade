#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/render/drawable/Material.h>
#include <shade/core/render/vertex/Vertex.h>

namespace shade
{
	class SHADE_API Drawable
	{
	public:
		static constexpr std::uint32_t MAX_LEVEL_OF_DETAIL = 10;

		struct Lod
		{
			Vertices Vertices;
			Indices	 Indices;
		};

		Drawable() = default;
		virtual ~Drawable() = default;
	public:
		void AddVertex(const Vertex& vertex, std::size_t lodLevel = 0);
		void AddIndex(std::uint32_t index, std::size_t lodLevel = 0);

		void AddVertices(const Vertices& vertices, std::size_t lodLevel = 0);
		void AddIndices(const Indices& indices, std::size_t lodLevel = 0);

		void SetVertices(Vertices& vertices, std::size_t lodLevel = 0);
		void SetVertices(std::vector<glm::vec3>& vertices, std::size_t lodLevel = 0);
		void SetIndices(Indices& indices, std::size_t lodLevel = 0);

		const Lod& GetLod(std::size_t level) const;
		Lod& GetLod(std::size_t level);

		void RecalculateAllLods(std::size_t levelCount, std::size_t maxFaces, std::size_t minFaces, float splitLambda);
		void RecalculateLod(std::size_t level, std::size_t faces);

		const std::array<shade::Drawable::Lod, MAX_LEVEL_OF_DETAIL>& GetLods() const;
		std::array<shade::Drawable::Lod, MAX_LEVEL_OF_DETAIL>& GetLods();
		
		void GenerateHalfExt();
		// Get Vertices of lod = 0;
		const Vertices& GetVertices() const;
		// Get Indices of lod = 0;
		const Indices& GetIndices() const;
		// Get Vertices of lod = 0;
		Vertices& GetVertices();
		// Get Indices of lod = 0;
		Indices& GetIndices();

		void SetMaterial(Asset<Material> material);
		const Asset<Material>& GetMaterial() const;
		Asset<Material>& GetMaterial();

	public:
		void SetMinHalfExt(const glm::vec3& ext);
		void SetMaxHalfExt(const glm::vec3& ext);
		const glm::vec3& GetMinHalfExt() const;
		const glm::vec3& GetMaxHalfExt() const;
	private:
		std::array<Lod, MAX_LEVEL_OF_DETAIL> m_Lods;
		Asset<Material> m_Material;
	private:
		glm::vec3			 m_MinHalfExt = glm::vec3(-1.0f);
		glm::vec3			 m_MaxHalfExt = glm::vec3(1.0f);
	};
}