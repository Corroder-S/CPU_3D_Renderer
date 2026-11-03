#pragma once

// STL
#include <cstdint>
#include <numeric>
#include <vector>

namespace ECS
{
	using EntityID = std::uint32_t;

	constexpr EntityID INVALID_VALUE = std::numeric_limits<uint32_t>::max();

	using Index = std::uint32_t;

	const std::uint32_t VALUE_MASK = 0xFFFFFF;

	struct entityElement
	{

		std::uint8_t version = 0;

		std::uint32_t nextfreeID = 0;

		bool active = false;
	};

	class EntityIDGenerator
	{
	public:

		EntityID generateID();


		void destroyEntity(EntityID id);

		bool isValid(EntityID id);

		EntityID forceID(EntityID id);

	private:

		void grow(std::size_t newSize);

		std::vector<entityElement> elements;

		EntityID nextAvailable = INVALID_VALUE;
	};

	class ComponentIDGenerator
	{
	public:
		template<typename T>
		static std::uint32_t get()
		{
			static std::uint32_t id = nextID();
			return id;
		}

		static std::uint32_t count() { return m_nextID; }

	private:
		static std::uint32_t nextID()
		{
			return m_nextID++;
		}

		static inline std::uint32_t m_nextID = 0;
	};
}

#include <ECS/detail/entity_component.inl>
