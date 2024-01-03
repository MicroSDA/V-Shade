#pragma once
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/core/image/Texture.h>
#include <shade/utils/Logger.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

namespace shade
{
	class SHADE_API Material : ASSET_INHERITANCE(Material)
	{
		ASSET_DEFINITION_HELPER(Material)

	public:
		// An enumeration class to represent different shading models available
		enum class ShadingModel : std::uint32_t
		{
			NonShading,			// No shading at all
			Flat,				// Uses a single color for each face of a 3D object
			Gouraud,			// Interpolates colors across vertices of a 3D object's faces
			Billin_Phong,		// Uses a reflection model to simulate highlights on a surface
			Physically_Based,	// A physically-based model that considers real-world material properties
			Ray_Tracing,		// Traces the path of light rays in a scene to simulate realistic lighting effects
			Toon,				// Produces a cartoon-like effect on 3D objects
			OrenNayer,			// A reflection model that simulates light scattering within a material
			Minnaert,			// A reflection model that simulates light absorption and scattering within a material
			CookTorance,		// A physically-based reflection model that simulates realistic surface textures and roughness
			Fresnel				// A reflection model that takes into account the angle of incidence of light on a surface
		};
		// It supposed to be used in conjunction with std 430
		struct RenderData
		{
			alignas(16) glm::vec3	ColorAmbient;
			alignas(16) glm::vec3	ColorDiffuse;
			alignas(16) glm::vec3	ColorSpecular;
			alignas(16) glm::vec3	ColorTransparent;

			float					Emmisive;
			float					Opacity;
			float					Shininess;
			float					ShininessStrength;
			float					Refracti;
			ShadingModel			Shading;

			alignas(4) bool			NormalMapEnabled;
			alignas(4) bool			BumpMapEnabled;
		};
	public:
		virtual ~Material() = default;
	public:

		glm::vec3					ColorAmbient		= glm::vec3(0.f);
		glm::vec3					ColorDiffuse		= glm::vec3(1.f);
		glm::vec3					ColorSpecular		= glm::vec3(1.f);
		glm::vec3					ColorTransparent	= glm::vec3(-1.f); // Minus one, so defualt transparent color doesn't exist

		float						Emmisive = 0.0f;
		float						Opacity = 1.0f;
		float						Shininess = 50.0f;
		float						ShininessStrength = 1.0f;
		float						Refracti = 0.0f;
		ShadingModel				Shading = ShadingModel::NonShading;
		bool						NormalMapEnabled = false;
		bool						BumpMapEnabled = false;

		Asset<Texture2D>			TextureDiffuse;
		Asset<Texture2D>			TextureSpecular;
		Asset<Texture2D>			TextureNormals;
		Asset<Texture2D>			TextureRoughness;
		Asset<Texture2D>			TextureAlbedo;
		Asset<Texture2D>			TextureMetallic;

		inline RenderData GetRenderData() const 
		{
			// Shader interpriate bool as uint with size = 4, so we need to have proper memory alignment here for each elements.
			// Since bool it's uint in the shader we need to use memset to set all additional bits to 0 after alignment.
			RenderData data;
			memset(&data, 0, sizeof(Material::RenderData));
			data.ColorAmbient		= ColorAmbient;
			data.ColorDiffuse		= ColorDiffuse;
			data.ColorSpecular		= ColorSpecular;
			data.ColorTransparent	= ColorTransparent;
			data.Emmisive			= Emmisive;
			data.Opacity			= Opacity;
			data.Shininess			= Shininess;
			data.ShininessStrength	= ShininessStrength;
			data.Refracti			= Refracti;
			data.Shading			= Shading;
			data.NormalMapEnabled	= NormalMapEnabled;
			data.BumpMapEnabled		= BumpMapEnabled;
			return data;
		}
	private:
		Material(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);

		friend class Serializer;
	};

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Material::ShadingModel& shading, std::size_t)
	{
		return Serializer::Serialize(stream, static_cast<std::uint32_t>(shading));
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Material::ShadingModel& shading, std::size_t)
	{
		return Serializer::Deserialize<std::uint32_t>(stream, (std::uint32_t&)shading);
	}

	/* Serialize Material.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Material& material, std::size_t)
	{
		return material.Serialize(stream);
	}
	/* Deserialize Material.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Material& material, std::size_t)
	{
		return material.Deserialize(stream);
	}
	/* Serialize Asset<Material>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<Material>& material, std::size_t)
	{
		return material->Serialize(stream);
	}
	/* Deserialize Asset<Material>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<Material>& material, std::size_t)
	{
		return material->Deserialize(stream);
	}
	/* Serialize SharedPointer<Material>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<Material>& material, std::size_t)
	{
		return material->Serialize(stream);
	}
	/* Deserialize SharedPointer<Material>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<Material>& material, std::size_t)
	{
		return material->Deserialize(stream);
	}

#ifndef MATERIAL_DATA_SIZE
	#define MATERIAL_DATA_SIZE (sizeof(Material::RenderData))
#endif // !MATERIAL_DATA_SIZE

#ifndef MATERIALS_DATA_SIZE
	#define MATERIALS_DATA_SIZE(count) (MATERIAL_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !MATERIALS_DATA_SIZE
}