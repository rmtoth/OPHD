#pragma once

#include "Things/Structures/Structure.h"
#include "StructureComponent.h"

namespace NAS2D {
	namespace Xml {
		class XmlElement;
	}
}

class Tile;
class PopulationPool;
struct StorableResources;

/**
 * Handles structure updating and resource management for structures.
 *
 * Keeps track of which structures are operational, idle and disabled.
 */
class StructureManager
{
private:
	/**
	 * Master table of all StructureComponent instances.
	 * It is divided into one sub-table per StructureComponent type.
	 * Each sub-table maps structure keys to a StructureComponent-derived instance.
	 * Only keys to structures that actually have a given StructureComponent type
	 * are present in the respective sub-tables.
	 */
	std::map<StructureComponent::ComponentTypeID, std::map<SKey, StructureComponent*>> mComponents; /**< Master list of all the StructureComponent instances. */

	/**
	 * The Structure class isn't a StructureComponent, so we keep a separate list
	 * of instances of that type rather than tracking them in mComponents.
	 * This may change in the future if SKey changes into something other than Structure*.
	 */
	std::vector<Structure*> mStructures;

public:
	/**
	 * Returns a range object that can be used to iterate over all instances of the
	 * given StructureComponent type. It is suitable for use in range-based for loops.
	 */
	template<typename ComponentTy>
	const ComponentRange<ComponentTy> enumerate()
	{
		return ComponentRange<ComponentTy>(mComponents[ComponentTy::componentTypeID]);
	}

	/**
	 * Returns a range object that can be used to iterate over all instances of the
	 * Structure type. It is suitable for use in range-based for loops.
	 * This allows writing code that's agnostic to whether Structure inherits StructureComponent.
	 */
	template<>
	const ComponentRange<Structure> enumerate<Structure>()
	{
		return ComponentRange<Structure>(mStructures);
	}

	/**
	 * Return a reference to the given StructureComponent type belonging to
	 * a structure. The structure is assumed to have the given component,
	 * and it is an error to try to get a component from a structure that
	 * does not have it.
	 */
	template<typename ComponentTy>
	ComponentTy& get(SKey s)
	{
		auto& table = mComponents[ComponentTy::componentTypeID];
		auto it = table.find(s);
#if defined(_DEBUG)
		if (it == table.end())
		{
			std::cout << "Trying to get a component from a structure that does not have it!!!" << std::endl;
			throw std::runtime_error("StructureManager::get() was called on a Structure without the requested component!");
		}
#endif
		return *reinterpret_cast<ComponentTy*>(it->second);
	}

	/**
	 * Return a reference to the Structure type belonging to a structure.
	 * This allows writing code that's agnostic to the SKey type.
	 */
	template<>
	Structure& get<Structure>(SKey s)
	{
		return *s;
	}

	/**
	 * Return a pointer to the given StructureComponent type belonging
	 * to a structure, if it has the corresponding component type.
	 * Otherwise return nullptr.
	 */
	template<typename ComponentTy>
	ComponentTy* tryGet(SKey s)
	{
		auto& table = mComponents[ComponentTy::componentTypeID];
		auto it = table.find(s);
		if (it != table.end())
			return reinterpret_cast<ComponentTy*>(it->second);
		return nullptr;
	}

	/**
	 * Return a pointer to the Structure type belonging to a structure.
	 * This allows writing code that's agnostic to the SKey type.
	 */
	template<>
	Structure* tryGet<Structure>(SKey s)
	{
		return s;
	}

	/**
	 * Create a new structure associated to a Structure instance.
	 * Returns a structure key identifying the new structure.
	 *
	 * WARNING: This may invalidate Structure iterators.
	 *          If this becomes an issue, consider a different
	 *          storage container for tracking Structure instances.
	 */
	SKey create(Structure* structure)
	{
#if defined(_DEBUG)
		if (std::find(mStructures.begin(), mStructures.end(), structure) != mStructures.end())
		{
			std::cout << "Trying to double-create a structure!!!" << std::endl;
			throw std::runtime_error("StructureManager::create() was repeatedly called on a Structure!");
		}
#endif
		mStructures.push_back(structure);
		return SKey(structure);
	}

	/**
	 * Create a new structure associated to a Structure instance
	 * and a set of structure component instances.
	 * Returns a structure key identifying the new structure.
	 */
	template<typename ComponentTy, typename... AdditionalTys>
	SKey create(Structure* structure, ComponentTy* component, AdditionalTys... additionalArgs)
	{
		SKey s = create(structure, additionalArgs...);
		auto& table = mComponents[ComponentTy::componentTypeID];
		bool success = table.insert(std::make_pair(s, static_cast<StructureComponent*>(component))).second;
#if defined(_DEBUG)
		if (!success)
		{
			std::cout << "Trying to attach duplicate component!!!" << std::endl;
			throw std::runtime_error("Structure::Attach() was called on a Structure that already had the component!");
		}
#endif
		return s;
	}

public:
	void addStructure(Structure* structure, Tile* tile);
	void removeStructure(Structure* structure);

	const StructureList& structureList(Structure::StructureClass structureClass);
	Tile& tileFromStructure(Structure* structure);

	void disconnectAll();
	void dropAllStructures();

	int count() const;

	int getCountInState(Structure::StructureClass structureClass, StructureState state);

	int disabled();
	int destroyed();

	bool CHAPAvailable();

	void updateEnergyProduction();
	void updateEnergyConsumed();
	int totalEnergyProduction() const { return mTotalEnergyOutput; }
	int totalEnergyUsed() const { return mTotalEnergyUsed; }
	int totalEnergyAvailable() const { return mTotalEnergyOutput - mTotalEnergyUsed; }

	void assignColonistsToResidences(PopulationPool&);

	void update(const StorableResources&, PopulationPool&);

	void serialize(NAS2D::Xml::XmlElement* element);

private:
	using StructureTileTable = std::map<Structure*, Tile*>;
	using StructureClassTable = std::map<Structure::StructureClass, StructureList>;

	void remove(SKey s);

	void updateStructures(const StorableResources&, PopulationPool&, StructureList&);

	bool structureConnected(Structure* structure);

	StructureTileTable mStructureTileTable; /**< List mapping Structures to a particular tile. */
	StructureClassTable mStructureLists; /**< Map containing all of the structure list types available. */

	int mTotalEnergyOutput = 0; /**< Total energy output of all energy producers in the structure list. */
	int mTotalEnergyUsed = 0;
};
