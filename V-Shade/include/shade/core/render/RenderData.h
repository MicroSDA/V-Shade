#pragma once
#include <shade/core/render/drawable/Drawable.h>
#include <shade/core/render/drawable/Material.h>
#include <shade/core/render/buffers/VertexBuffer.h>
#include <shade/core/render/buffers/IndexBuffer.h>
#include <shade/core/render/buffers/StorageBuffer.h>
#include <shade/core/render/buffers/UniformBuffer.h>
#include <shade/core/camera/Camera.h>
#include <shade/core/animation/Pose.h>

namespace shade
{
	namespace render
	{
		//template<typename... Args>
		//struct PointerHashCombined
		//{
		//	std::size_t operator()(Args&&... args) const noexcept
		//	{
		//		//return (std::hash<std::decay_t<Args>>()(std::forward<Args>(args)) ^ ...);
		//		return (std::decay_t<Args>(std::forward<Args>(args)) ^ ...);
		//	}
		//};

		// Create combination of pointers
		// To use this function make sure that your class represents operator std::size_t
		template<typename... Args>
		std::size_t PointerHashCombine(Args&&... args)
		{
			return (static_cast<std::size_t>(std::decay_t<Args>(std::forward<Args>(args))) ^ ...);
			//return (std::hash<std::decay_t<Args>>{}(std::forward<Args>(args)) ^ ...);
		}

		// Cast hash to pointer.
		// IMPORTANT: To use make sure that you 'hash' is the right pointer address.
		// IMPORTANT: Doesn't work for SharedPointer since it has mix between ponter and time stamp!
		template<typename T>
		T* FromHashToPointerCast(std::size_t hash)
		{
			return reinterpret_cast<T*>(hash);
		}

		struct SubmitedMaterials
		{
		};

		struct PairHash 
		{
		public:
			template <typename T, typename U>
			std::size_t operator()(const std::pair<T, U>& x) const
			{
				return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
			}
		};
		struct MaterialModelPair
		{
			std::unordered_set<std::pair<std::size_t, Asset<Material>>, PairHash> Materials;
			std::size_t ModelHash = 0u;
		};

		struct SubmitedInstances
		{
			// Where size_t is Asset<Drawable> 		
			std::unordered_map <std::size_t, MaterialModelPair> Instances;
		};

		struct GeometryBuffer
		{
			// Vertices.
			SharedPointer<VertexBuffer> VB;
			// Indices.
			SharedPointer<IndexBuffer>  IB;
			// Bones&Weights
			SharedPointer<VertexBuffer> BW;
		};

		struct InstanceRawData
		{
			// Transform per unique instance encounter, combination of (Pipeline, Drawable, Material).
			std::vector<glm::mat4> Transforms;
			// Transform offset within unique instance.
			std::uint32_t TransformOffset = 0;

			// Material per unique instance encounter, combination of (Pipeline, Drawable, Material).
			std::vector<Material::RenderData> Materials;
			// Material offset within unique instance.
			std::uint32_t MaterialOffset = 0;
		};
		struct BoneSubmitedMetaData
		{
			// Bone transform per unique pipeline
			std::vector<SharedPointer<std::vector<animation::Pose::GlobalTransform>>> BoneTransforms;
			// Offset within unique pipeline + model.
			std::uint32_t PipelineModelOffset = 0;
		};
		struct SubmitedSceneRenderData
		{
			// Where size_t is Asset<Drawable> hash - > Geometry buffers
			std::unordered_map<std::size_t, std::array<render::GeometryBuffer, Drawable::MAX_LEVEL_OF_DETAIL>> GeometryBuffers;
			// Where size_t is hash of (Pipeline, Drawable, Material) - > Transforms and materials with offset.
			std::unordered_map<std::size_t, InstanceRawData> InstanceRawData;
			// Where size_t hash of Pipeline -> std::uint32_t bone transforms data per with offset
			std::unordered_map<std::size_t, BoneSubmitedMetaData> BoneOffsetsData;
			// Where index is frame index.
			std::vector<SharedPointer<VertexBuffer>> TransformBuffers;
			SharedPointer<StorageBuffer> MaterialsBuffer;
			SharedPointer<StorageBuffer> GlobalLightsBuffer;
			SharedPointer<StorageBuffer> PointsLightsBuffer;
			SharedPointer<StorageBuffer> SpotLightsBuffer;
			SharedPointer<StorageBuffer> BoneTransfromsBuffer;
			SharedPointer<UniformBuffer> CameraBuffer;
			SharedPointer<UniformBuffer> SceneRenderDataBuffer;
			SharedPointer<UniformBuffer> RenderSettingsDataBuffer;
			SharedPointer<Camera> Camera;
		};
	}
}
