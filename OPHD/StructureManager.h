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
	std::map<StructureComponent::ComponentTypeID, std::map<SKey, StructureComponent*>> mComponents;

	// Structure isn't a StructureComponent, so special case it.
	std::vector<Structure*> mStructures;

public:
	template<typename ComponentTy>
	const ComponentRange<ComponentTy> enumerate()
	{
		return ComponentRange<ComponentTy>(mComponents[ComponentTy::componentTypeID]);
	}
	// Structure isn't a StructureComponent right now, so special case it.
	template<>
	const ComponentRange<Structure> enumerate<Structure>()
	{
		return ComponentRange<Structure>(mStructures);
	}

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
	// Structure isn't a StructureComponent right now, so special case it.
	template<>
	Structure& get<Structure>(SKey s)
	{
		return *s;
	}

	template<typename ComponentTy>
	ComponentTy* tryGet(SKey s)
	{
		auto& table = mComponents[ComponentTy::componentTypeID];
		auto it = table.find(s);
		if (it != table.end())
			return reinterpret_cast<ComponentTy*>(it->second);
		return nullptr;
	}
	// Structure isn't a StructureComponent right now, so special case it.
	template<>
	Structure* tryGet<Structure>(SKey s)
	{
		return s;
	}

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

	void remove(SKey s)
	{
		auto it = std::find(mStructures.begin(), mStructures.end(), s);
		if (it == mStructures.end())
		{
#if defined(_DEBUG)
			std::cout << "Trying to remove an unknown structure!!!" << std::endl;
			throw std::runtime_error("StructureManager::remove() was called on a Structure that's not managed!");
#endif
		}
		else
		{
			mStructures.erase(it);
		}

		// Assumption: we do not know anything about the set of components belonging to the SKey.
		for (auto& [componentTypeID, table] : mComponents)
		{
			auto cit = table.find(s);
			if (cit != table.end())
			{
				delete cit->second;
				table.erase(cit);
			}
		}
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

	void updateStructures(const StorableResources&, PopulationPool&, StructureList&);

	bool structureConnected(Structure* structure);

	StructureTileTable mStructureTileTable; /**< List mapping Structures to a particular tile. */
	StructureClassTable mStructureLists; /**< Map containing all of the structure list types available. */

	int mTotalEnergyOutput = 0; /**< Total energy output of all energy producers in the structure list. */
	int mTotalEnergyUsed = 0;
};
