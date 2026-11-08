#pragma once

// STL
#include <cstdint>
#include <numeric>
#include <vector>

// ============================================================================
// IDENTIFIANTS D'ENTITÉS ET DE COMPOSANTS
// ============================================================================

namespace ECS
{
	using EntityID = std::uint32_t;

	/// @brief Valeur sentinelle représentant un EntityID invalide.
	constexpr EntityID INVALID_VALUE = std::numeric_limits<uint32_t>::max();

	using Index = std::uint32_t;

	/// @brief Masque extrayant les 24 bits de valeur d'un EntityID versionné.
	const std::uint32_t VALUE_MASK = 0xFFFFFF;

	/// @brief Entrée de la table d'entités : version, liste libre et état actif.
	struct entityElement
	{
		/// @brief Numéro de version incrémenté à chaque réutilisation du slot.
		std::uint8_t version = 0;

		/// @brief Index du prochain slot libre dans la liste chaînée.
		std::uint32_t nextfreeID = 0;

		bool active = false;
	};

	/// @brief Génère et recycle les EntityID avec détection de dangling references par version.
	class EntityIDGenerator
	{
	public:

		/// @brief Crée un nouvel EntityID valide (réutilise un slot libre si disponible).
		/// @return Nouvel identifiant d'entité.
		EntityID generateID();

		/// @brief Marque l'entité comme détruite et remet son slot dans la liste libre.
		/// @param id Identifiant à invalider.
		void destroyEntity(EntityID id);

		/// @brief Vérifie si un EntityID est encore valide (entité non détruite).
		/// @param id Identifiant à tester.
		/// @return Vrai si l'entité est active.
		bool isValid(EntityID id);

		/// @brief Force l'utilisation d'un ID précis (utilisé lors de la désérialisation).
		/// @param id Identifiant à réserver.
		/// @return L'identifiant réservé.
		EntityID forceID(EntityID id);

	private:

		/// @brief Agrandit le tableau d'éléments jusqu'à newSize.
		void grow(std::size_t newSize);

		std::vector<entityElement> elements;

		EntityID nextAvailable = INVALID_VALUE;
	};

	/// @brief Assigne un identifiant entier unique à chaque type de composant à la compilation.
	class ComponentIDGenerator
	{
	public:
		/// @brief Retourne l'ID du type T (généré une seule fois par type grâce à la variable statique locale).
		/// @tparam T Type de composant.
		/// @return Identifiant numérique stable du type.
		template<typename T>
		static std::uint32_t get()
		{
			static std::uint32_t id = nextID();
			return id;
		}

		/// @brief Retourne le nombre total de types de composants enregistrés.
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
