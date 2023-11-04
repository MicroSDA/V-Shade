#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/physics/shapes/CollisionShape.h>

namespace shade
{
	namespace physic
	{
		class SHADE_API MeshShape : public CollisionShape
		{
		public:
			MeshShape() : CollisionShape(Shape::Mesh) {}
			virtual ~MeshShape() = default;
		public:
			virtual Manifold TestCollision(const glm::mat<4,4, scalar_t>& transform, const CollisionShape& otherShape, const glm::mat<4, 4, scalar_t>& otherTransform) const override;
			virtual glm::vec<3, scalar_t> FindFurthestPointWorld(const glm::mat<4, 4, scalar_t>& transform, const glm::vec<3, scalar_t>& direction) const override;

			void AddVertex(const glm::vec<3, scalar_t>& vertex);
			void AddVertices(const std::vector<glm::vec<3, scalar_t>>& vertices);
			void SetVertices(std::vector<glm::vec<3, scalar_t>>& vertices);

			const std::vector<glm::vec<3, scalar_t>>& GetVertices() const;
			std::vector<glm::vec<3, scalar_t>>& GetVertices();
		private:
			virtual std::size_t Serialize(std::ostream& stream) const override;
			virtual std::size_t Deserialize(std::istream& stream) override;

			std::vector<glm::vec<3, scalar_t>> m_Vertices;
			friend class Serializer;
		};
	}
}