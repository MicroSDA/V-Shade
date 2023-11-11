#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <shade/core/physics/Common.h>
#include <shade/core/physics/algo/Simplex.h>
#include <shade/core/asset/Asset.h>

namespace shade
{
	namespace physic
	{
		class SHADE_API CollisionShape
		{
		public:
			struct Manifold
			{
				bool HasCollision		= false;
				scalar_t CollisionDepth	= 0.f;
				glm::vec<3, scalar_t> Normal = glm::vec<3, scalar_t>(0.f); // B - Normalized

				glm::vec<3, scalar_t> ContactPointA_W = glm::vec<3, scalar_t>(0.f); // Contact point of A with B in world coordinates
				glm::vec<3, scalar_t> ContactPointB_W = glm::vec<3, scalar_t>(0.f); // Contact point of B with A in world coordinates
				glm::vec<3, scalar_t> ContactPointA_L = glm::vec<3, scalar_t>(0.f); // Contact point of A with B in local coordinates
				glm::vec<3, scalar_t> ContactPointB_L = glm::vec<3, scalar_t>(0.f); // Contact point of B with A in local coordinates
				

				[[nodiscard]] static CollisionShape::Manifold GetMaxPenetration(const std::vector<Manifold> points)
				{
					if (points.size() == 0) return Manifold(); // Return no collision
					Manifold* maxNormalPoint = const_cast<Manifold*>(&points[0]);

					float  maxNormalDist = std::numeric_limits<float>::min();
					for (const Manifold& point : points)
					{
						if (point.CollisionDepth > maxNormalDist)
						{
							maxNormalDist = point.CollisionDepth;
							maxNormalPoint = const_cast<Manifold*>(&point);
						}
					}
					return *maxNormalPoint;
				}

				bool operator==(const Manifold& other) const noexcept
				{
					return (
						ContactPointA_L == other.ContactPointA_L && 
						ContactPointB_L == other.ContactPointB_L &&
						ContactPointA_W == other.ContactPointA_W &&
						ContactPointB_W == other.ContactPointB_W );
				}
				bool operator!=(const Manifold& other) const noexcept
				{
					return (
						ContactPointA_L != other.ContactPointA_L &&
						ContactPointB_L != other.ContactPointB_L &&
						ContactPointA_W != other.ContactPointA_W &&
						ContactPointB_W != other.ContactPointB_W);
				}
			};
			enum Shape : std::uint32_t
			{
				Sphere = 0u,
				Cylinder,
				Capsule,
				Plane,
				Mesh,

				SHAPE_MAX_ENUM
			};

			static Shape GetShapeFromString(const std::string& shape);
			static std::string GetShapeAsString(Shape shape);
		public:
			CollisionShape(Shape shape);
			virtual ~CollisionShape() = default;
		public:
			void SetMinMaxHalfExt(const glm::vec<3, scalar_t>& minExt, const glm::vec<3, scalar_t>& maxExt);

			virtual Manifold TestCollision(const glm::mat<4, 4, scalar_t>& transform, const CollisionShape& otherShape, const glm::mat<4, 4, scalar_t>& otherTransform) const = 0; // TODO: {} insead of  = 0
			bool AABB_X_AABB(const glm::mat<4, 4, scalar_t>& transform, const CollisionShape& otherShape, const glm::mat<4, 4, scalar_t>& otherTransform) const; // TODO: {} insead of  = 0
			bool OBB_X_OBB(const glm::mat<4, 4, scalar_t>& transform, const CollisionShape& otherShape, const glm::mat<4, 4, scalar_t>& otherTransform) const; // TODO: {} insead of  = 0
			virtual glm::vec<3, scalar_t> FindFurthestPointWorld(const glm::mat<4, 4, scalar_t>& transform, const glm::vec<3, scalar_t>& direction) const = 0; // TODO: {} insead of  = 0
		
			const glm::vec<3, scalar_t>& GetMinHalfExt() const;
			const glm::vec<3, scalar_t>& GetMaxHalfExt() const;

			const glm::vec<3, scalar_t>& GetMinHalfExtWorldSpace() const;
			const glm::vec<3, scalar_t>& GetMaxHalfExtWorldSpace() const;

			const std::array<glm::vec<3, scalar_t>, 8>& GetCorners() const;
			Shape GetShape() const;

			void UpdateCorners(const glm::mat<4, 4, scalar_t>& transform);
		private:
			const Shape m_Shape;
			glm::vec<3, scalar_t> m_MinHalfExt = glm::vec<3, scalar_t>(-1.0);
			glm::vec<3, scalar_t> m_MaxHalfExt = glm::vec<3, scalar_t>(1.0);

			// Using only for debug purposes
			glm::vec<3, scalar_t> m_MinHalfExtWorldSpace = glm::vec<3, scalar_t>(-1.0);
			glm::vec<3, scalar_t> m_MaxHalfExtWorldSpace = glm::vec<3, scalar_t>(1.0);

			std::array<glm::vec<3, scalar_t>, 8> m_Corners;
		protected:
			virtual std::size_t Serialize(std::ostream& stream) const = 0;
			virtual std::size_t Deserialize(std::istream& stream) = 0;
		private:
			friend class Serializer;
		};

		class SHADE_API CollisionShapes : public BaseAsset, public Asset<CollisionShapes>
		{
		public:
			CollisionShapes() = default;
			virtual ~CollisionShapes() = default;
			static AssetMeta::Type GetAssetStaticType();
			virtual AssetMeta::Type GetAssetType() const override;

			const std::vector<SharedPointer<CollisionShape>>& GetColliders() const;
			std::vector<SharedPointer<CollisionShape>>& GetColliders();
			std::size_t GetCollidersCount() const;

			void AddShape(const SharedPointer<CollisionShape>& shape);
		private:
			CollisionShapes(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
			static CollisionShapes* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		private:
			std::size_t Serialize(std::ostream& stream) const;
			std::size_t Deserialize(std::istream& stream);
		private:
			std::vector<SharedPointer<CollisionShape>> m_Colliders;

			friend class Serializer;
			friend class Asset<CollisionShapes>;
		};
	}

	/* Serialize CollisionShapes.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const physic::CollisionShapes& shapes, std::size_t)
	{
		return shapes.Serialize(stream);
	}
	/* Deserialize CollisionShapes.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, physic::CollisionShapes& shapes, std::size_t)
	{
		return shapes.Deserialize(stream);
	}
	/* Serialize Asset<CollisionShapes>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<physic::CollisionShapes>& shapes, std::size_t)
	{
		return shapes->Serialize(stream);
	}
	/* Deserialize Asset<CollisionShapes>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<physic::CollisionShapes>& shapes, std::size_t)
	{
		return shapes->Deserialize(stream);
	}

	/* Serialize CollisionShape.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const physic::CollisionShape& shape, std::size_t)
	{
		return shape.Serialize(stream);
	}
	/* Deserialize CollisionShape.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, physic::CollisionShape& shape, std::size_t)
	{
		return shape.Deserialize(stream);
	}
	/* Serialize SharedPointer<CollisionShape>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<physic::CollisionShape>& shape, std::size_t)
	{
		return shape->Serialize(stream);
	}
	/* Deserialize SharedPointer<CollisionShape>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<physic::CollisionShape>& shape, std::size_t)
	{
		return shape->Deserialize(stream);
	}
}
