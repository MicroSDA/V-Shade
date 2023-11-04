#pragma once
#include <shade/config/ShadeAPI.h>

namespace shade 
{
	namespace ecs
	{
		/* Type identifier */
		using TypeID = std::size_t;
		/* Type hash value */
		using TypeHash = std::size_t;
		/* Entity identifier */
		using EntityID = std::size_t;
		/* Entity version  */
		using EntityVersion = std::size_t;
		/**********************************************/
		template<typename, typename = void>
		struct EntityTraits;

		template<>
		struct EntityTraits<std::uint32_t>
		{
			using EntityType = std::uint32_t;
			using VersionType = std::uint16_t;
			using DifferenceType = std::int32_t;
			/* Mask to use to get the entity number out of an identifier. */
			static constexpr EntityType EntityMask = 0xFFFFF;
			/* Mask to use to get the version out of an identifier. */
			static constexpr EntityType VersionMask = 0xFFF;
			/* Extent of the entity number within an identifier. */
			static constexpr std::size_t EntityShift = 20u;
			/* Convert to itegral */
			[[nodiscard]] static constexpr std::uint32_t ToIntegral(const std::uint32_t& value) noexcept
			{
				return static_cast<EntityTraits<std::uint32_t>::EntityType>(value);
			}
			/* Convert to index without version */
			[[nodiscard]] static constexpr std::uint32_t ToID(const std::uint32_t& value) noexcept
			{
				return static_cast<EntityTraits<std::uint32_t>::EntityType>(value) & EntityTraits<uint32_t>::EntityMask;
			}
		};
		template<>
		struct EntityTraits<std::uint64_t>
		{
			using EntityType = std::uint64_t;
			using VersionType = std::uint32_t;
			using DifferenceType = std::int64_t;
			/* Mask to use to get the entity number out of an identifier. */
			static constexpr EntityType EntityMask = 0xFFFFFFFF;
			/* Mask to use to get the version out of an identifier. */
			static constexpr EntityType VersionMask = 0xFFFFFFFF;
			/* Extent of the entity number within an identifier. */
			static constexpr std::size_t EntityShift = 32u;
			/* Convert to itegral */
			[[nodiscard]] static constexpr std::uint64_t ToIntegral(const std::uint64_t& value) noexcept
			{
				return static_cast<EntityTraits<std::uint64_t>::EntityType>(value);
			}
			/* Convert to index without version */
			[[nodiscard]] static constexpr std::uint64_t ToID(const std::uint64_t& value) noexcept
			{
				return static_cast<EntityTraits<std::uint64_t>::EntityType>(value) & EntityTraits<uint64_t>::EntityMask;
			}
		};
		/* Null value for entity (unsigned - 1)*/
		struct Null
		{
			template<typename Entity>
			constexpr operator Entity() const noexcept
			{
				return Entity(EntityTraits<Entity>::EntityMask);
			}
			constexpr bool operator==(Null) const noexcept
			{
				return true;
			}
			constexpr bool operator!=(Null) const noexcept
			{
				return false;
			}
			template<typename Entity>
			constexpr bool operator==(const Entity entity) const noexcept
			{
				return (EntityTraits<Entity>::ToIntegral(entity) & EntityTraits<Entity>::EntityMask) == EntityTraits<Entity>::ToIntegral(static_cast<Entity>(*this));
			}
			template<typename Entity>
			constexpr bool operator!=(const Entity entity) const noexcept
			{
				return !(entity == *this);
			}
		};

		template<typename Entity>
		constexpr bool operator==(const Entity entity, Null other) noexcept
		{
			return other.operator==(entity);
		}

		template<typename Entity>
		constexpr bool operator!=(const Entity entity, Null other) noexcept
		{
			return !(other == entity);
		}

		inline constexpr Null null{};

		namespace internal
		{
			struct SHADE_API TypeInfoImpl
			{
				[[nodiscard]] static TypeID Next() noexcept
				{
					static TypeID value = 0u;
					return value++;
				}
			};
		}

		template<typename Type, class = void>
		struct SHADE_API TypeInfo final
		{
			///* Get index id of specific type */
			//[[nodiscard]] static TypeID ID() noexcept
			//{
			//	static const TypeID value = internal::TypeInfoImpl::Next();
			//	return value;
			//}
			/* Get name of specific type */
			[[nodiscard]] static const char* Name() noexcept
			{
				static const auto value = typeid(Type).name();
				return value;
			}
		};

		/* Get hash value of specific type */
		template<typename Type>
		[[nodiscard]] static TypeHash Hash() noexcept
		{
			static const TypeHash value = (TypeHash)typeid(Type).hash_code();
			return value;
		}
	}
}