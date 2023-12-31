#include "shade_pch.h"
#include "CollisionShape.h"
#include "MeshShape.h"

static void CalculateAxes(const std::array<glm::vec<3, shade::physic::scalar_t>, 8>& aCorners, const std::array<glm::vec<3, shade::physic::scalar_t>, 8>& bCorners, std::array<glm::vec<3, shade::physic::scalar_t>, 15>& axes)
{
	glm::vec<3, shade::physic::scalar_t> e1 = aCorners[1] - aCorners[0], e2 = aCorners[2] - aCorners[0], e3 = aCorners[4] - aCorners[0];

	// Calculate the axes for the a box
	axes[0] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e1, e2));
	axes[1] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e1, e3));
	axes[2] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e2, e3));

	// Calculate the axes for the b box
	e1 = bCorners[1] - bCorners[0]; e2 = bCorners[2] - bCorners[0]; e3 = bCorners[4] - bCorners[0];

	axes[3] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e1, e2));
	axes[4] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e1, e3));
	axes[5] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(e2, e3));

	// Calculate the remaining edge cross product axes
	axes[6] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[0], axes[3]));
	axes[7] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[0], axes[4]));
	axes[8] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[0], axes[5]));
	axes[9] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[1], axes[3]));
	axes[10] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[1], axes[4]));
	axes[11] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[1], axes[5]));
	axes[12] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[2], axes[3]));
	axes[13] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[2], axes[4]));
	axes[14] = glm::normalize<3, shade::physic::scalar_t>(glm::cross<shade::physic::scalar_t>(axes[2], axes[5]));
}

void shade::physic::HalfExtensions::UpdateCorners(const glm::mat<4, 4, shade::physic::scalar_t>& transform)
{
	Corners[0] = transform * glm::vec<4, physic::scalar_t>(MinHalfExt.x, MinHalfExt.y, MinHalfExt.z, 1.0f);
	Corners[1] = transform * glm::vec<4, physic::scalar_t>(MinHalfExt.x, MinHalfExt.y, MaxHalfExt.z, 1.0f);
	Corners[2] = transform * glm::vec<4, physic::scalar_t>(MinHalfExt.x, MaxHalfExt.y, MinHalfExt.z, 1.0f);
	Corners[3] = transform * glm::vec<4, physic::scalar_t>(MinHalfExt.x, MaxHalfExt.y, MaxHalfExt.z, 1.0f);
	Corners[4] = transform * glm::vec<4, physic::scalar_t>(MaxHalfExt.x, MinHalfExt.y, MinHalfExt.z, 1.0f);
	Corners[5] = transform * glm::vec<4, physic::scalar_t>(MaxHalfExt.x, MinHalfExt.y, MaxHalfExt.z, 1.0f);
	Corners[6] = transform * glm::vec<4, physic::scalar_t>(MaxHalfExt.x, MaxHalfExt.y, MinHalfExt.z, 1.0f);
	Corners[7] = transform * glm::vec<4, physic::scalar_t>(MaxHalfExt.x, MaxHalfExt.y, MaxHalfExt.z, 1.0f);

	glm::vec<3, physic::scalar_t> newMin = glm::vec<3, physic::scalar_t>(Corners[0]);
	glm::vec<3, physic::scalar_t> newMax = glm::vec<3, physic::scalar_t>(Corners[0]);

	for (const auto& corner : Corners)
	{
		newMin = glm::min(newMin, corner); newMax = glm::max(newMax, corner);
	}

	MaxHalfExtWorldSpace = newMax; MinHalfExtWorldSpace = newMin;
}

bool shade::physic::HalfExtensions::AABB_X_AABB(const HalfExtensions& extB) const
{
	return (
		MinHalfExtWorldSpace.x <= extB.MaxHalfExtWorldSpace.x &&
		MaxHalfExtWorldSpace.x >= extB.MinHalfExtWorldSpace.x &&
		MinHalfExtWorldSpace.y <= extB.MaxHalfExtWorldSpace.y &&
		MaxHalfExtWorldSpace.y >= extB.MinHalfExtWorldSpace.y &&
		MinHalfExtWorldSpace.z <= extB.MaxHalfExtWorldSpace.z &&
		MaxHalfExtWorldSpace.z >= extB.MinHalfExtWorldSpace.z
		);
}

bool shade::physic::HalfExtensions::OBB_X_OBB(const HalfExtensions& extB) const
{
	// 3 normals of each box's faces and 9 edge cross products
	std::array<glm::vec<3, scalar_t>, 15> axes;
	CalculateAxes(Corners, extB.Corners, axes);

	for (int i = 0; i < 15; i++)
	{
		if (glm::all(glm::isnan<3, scalar_t>(axes[i])))
			axes[i] = glm::vec3(1.0f, 0.0f, 0.0f);

		scalar_t aMin = std::numeric_limits<scalar_t>::max(), aMax = -aMin, bMin = aMin, bMax = -bMin;

		for (int j = 0; j < 8; j++)
		{
			// Project corners of box a onto the axis
			scalar_t projection = glm::dot<3, scalar_t>(Corners[j], axes[i]);
			aMin = std::min<scalar_t>(aMin, projection);
			aMax = std::max<scalar_t>(aMax, projection);

			// Project corners of box b onto the axis
			projection = glm::dot<3, scalar_t>(extB.Corners[j], axes[i]);
			bMin = std::min<scalar_t>(bMin, projection);
			bMax = std::max<scalar_t>(bMax, projection);
		}
		// Check for overlap
		if (aMax < bMin || bMax < aMin)
			return false; // No overlap on this axis, boxes are not colliding
	}

	// If there is no axis with no overlap, the boxes are colliding
	return true;
}

shade::physic::CollisionShape::Shape shade::physic::CollisionShape::GetShapeFromString(const std::string& shape)
{
	if (shape == "Sphere")
		return Shape::Sphere;
	if (shape == "Cylinder")
		return Shape::Cylinder;
	if (shape == "Capsule")
		return Shape::Capsule;
	if (shape == "Plane")
		return Shape::Plane;
	if (shape == "Mesh")
		return Shape::Mesh;

	return Shape::SHAPE_MAX_ENUM;
}

std::string shade::physic::CollisionShape::GetShapeAsString(Shape shape)
{
	switch (shape)
	{
	case shade::physic::CollisionShape::Shape::Sphere: return "Sphere";
	case shade::physic::CollisionShape::Shape::Cylinder:  return "Cylinder";
	case shade::physic::CollisionShape::Shape::Capsule:  return "Capsule";
	case shade::physic::CollisionShape::Shape::Plane:  return "Plane";
	case shade::physic::CollisionShape::Shape::Mesh:  return "Mesh";
	default:
		return "Undefined";
	}
}

shade::physic::CollisionShape::CollisionShape(Shape shape) : m_Shape(shape)
{
}

void shade::physic::CollisionShape::SetMinMaxHalfExt(const glm::vec<3, scalar_t>& minExt, const glm::vec<3, scalar_t>& maxExt)
{
	m_MinHalfExt = minExt;
	m_MaxHalfExt = maxExt;
}

shade::physic::CollisionShape::Shape shade::physic::CollisionShape::GetShape() const
{
	return m_Shape;
}

const glm::vec<3, shade::physic::scalar_t>& shade::physic::CollisionShape::GetMinHalfExt() const
{
	return m_MinHalfExt;
}

const glm::vec<3, shade::physic::scalar_t>& shade::physic::CollisionShape::GetMaxHalfExt() const
{
	return m_MaxHalfExt;
}

shade::physic::CollisionShapes::CollisionShapes(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	auto filePath = assetData->GetAttribute<std::string>("Path");

	shade::File file(filePath, shade::File::In, "@s_c_shape", shade::File::VERSION(0, 0, 1));

	if (!file.IsOpen())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
	{
		file.Read(*this);
	}
	file.CloseFile();
}

std::size_t shade::physic::CollisionShapes::Serialize(std::ostream& stream) const
{
	std::size_t size = Serializer::Serialize(stream, static_cast<std::uint32_t>(m_Colliders.size()));

	for (const auto& collider : m_Colliders)
	{
		size += Serializer::Serialize(stream, static_cast<float>(collider->GetMinHalfExt().x));	Serializer::Serialize(stream, static_cast<float>(collider->GetMinHalfExt().y)); Serializer::Serialize(stream, static_cast<float>(collider->GetMinHalfExt().z));
		size += Serializer::Serialize(stream, static_cast<float>(collider->GetMaxHalfExt().x));	Serializer::Serialize(stream, static_cast<float>(collider->GetMaxHalfExt().y)); Serializer::Serialize(stream, static_cast<float>(collider->GetMaxHalfExt().z));

		size += Serializer::Serialize(stream, collider);
	}

	return size;
}

std::size_t shade::physic::CollisionShapes::Deserialize(std::istream& stream)
{
	std::uint32_t collidersCount = 0;
	std::size_t size = Serializer::Deserialize(stream, collidersCount);

	if (collidersCount <= 0 || collidersCount >= UINT32_MAX)
		throw std::exception("Invalide lods count!");

	for (std::size_t i = 0; i < collidersCount; i++)
	{
		glm::vec3 minHalf, maxHalf;
		Serializer::Deserialize(stream, minHalf.x);	Serializer::Deserialize(stream, minHalf.y); Serializer::Deserialize(stream, minHalf.z);
		Serializer::Deserialize(stream, maxHalf.x);	Serializer::Deserialize(stream, maxHalf.y); Serializer::Deserialize(stream, maxHalf.z);

		std::uint32_t type;
		size += Serializer::Deserialize(stream, type);

		switch (type)
		{
		case CollisionShape::Shape::Mesh:
		{
			auto collider = shade::SharedPointer<shade::physic::MeshShape>::Create();
			collider->SetMinMaxHalfExt(minHalf, maxHalf);

			std::uint32_t verticesCount = 0;
			size += Serializer::Deserialize(stream, verticesCount);

			if (verticesCount <= 0 || verticesCount >= UINT32_MAX)
				throw std::exception("Invalide lods count!");

			for (std::size_t v = 0; v < verticesCount; v++)
			{
				glm::vec3 point;

				size += Serializer::Deserialize(stream, point.x);
				size += Serializer::Deserialize(stream, point.y);
				size += Serializer::Deserialize(stream, point.z);

				collider->AddVertex(point);
			}

			AddShape(collider);
		}

		}

	}

	return size;
}

const std::vector<shade::SharedPointer<shade::physic::CollisionShape>>& shade::physic::CollisionShapes::GetColliders() const
{
	return m_Colliders;
}

std::vector<shade::SharedPointer<shade::physic::CollisionShape>>& shade::physic::CollisionShapes::GetColliders()
{
	return m_Colliders;
}

std::size_t shade::physic::CollisionShapes::GetCollidersCount() const
{
	return m_Colliders.size();
	;
}

void shade::physic::CollisionShapes::AddShape(const SharedPointer<CollisionShape>& shape)
{
	m_Colliders.emplace_back(shape);
}
