#pragma once
#include <glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/config/ShadeAPI.h>

namespace shade
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
		glm::vec2 UV_Coordinates;

		bool operator == (const Vertex& other) const
		{
			return (Position == other.Position);
		}
		bool operator != (const Vertex& other) const
		{
			return (Position != other.Position);
		}

		template<typename T>
		static std::vector<glm::vec<3, T>> ConvertToArray(const std::vector<Vertex>& vertices)
		{
			std::vector<glm::vec<3, T>> array(vertices.size());
			for (std::size_t i = 0; i < vertices.size(); ++i)
				array[i] = vertices[i].Position;
			
			return array;
		}
	};

	static constexpr std::uint32_t MAX_BONES_PER_VERTEX = 4;

	struct Bone
	{
		Bone()
		{
			for (auto i = 0; i < MAX_BONES_PER_VERTEX; ++i)
			{
				IDs[i]		= std::numeric_limits<std::uint32_t>::max();
				Weights[i]	= 0.f;
			}
		}
		std::array<std::uint32_t, MAX_BONES_PER_VERTEX> IDs;
		std::array<float, MAX_BONES_PER_VERTEX> Weights;
	};

	using Vertices	= std::vector<Vertex>;
	using Index		= std::uint32_t;
	using Indices	= std::vector<Index>;
	using Bones		= std::vector<Bone>;
	
	namespace algo
	{
		SHADE_API void SimplifyMesh(Vertices& vertices, Indices& indices, std::size_t count);
		SHADE_API std::vector<std::size_t> CalculateFaceCountLodLevel(std::size_t levelCount, std::size_t maxFaces, std::size_t minFaces, float splitLambda);
	}

#ifndef VERTEX_DATA_SIZE
	#define VERTEX_DATA_SIZE (sizeof(Vertex))
#endif // !VERTEX_DATA_SIZE

#ifndef VERTICES_DATA_SIZE
	#define VERTICES_DATA_SIZE(count) (VERTEX_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !VERTICES_DATA_SIZE

#ifndef INDEX_DATA_SIZE
	#define INDEX_DATA_SIZE (sizeof(uint32_t))
#endif // !INDEX_DATA_SIZE

#ifndef INDICES_DATA_SIZE
	#define INDICES_DATA_SIZE(count) (INDEX_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !INDICES_DATA_SIZE

#ifndef BONE_DATA_SIZE
	#define BONE_DATA_SIZE (sizeof(Bone))
#endif // !BONE_DATA_SIZE

#ifndef BONES_DATA_SIZE
	#define BONES_DATA_SIZE(count) (BONE_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !BONES_DATA_SIZE
}