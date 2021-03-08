#pragma once

#include "Things/Structures/Structure.h"


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
public:
	template<typename T>
	std::vector<T*>& getComponents()
	{
		return reinterpret_cast<std::vector<T*>&>(mComponents[T::uid]);
	}
	template<typename T>
	static std::vector<T*>& GetComponents()
	{
		return Utility<StructureManager>::get().getComponents<T>();
	}
private:
	std::map<StructureComponent::UID, std::vector<StructureComponent*>> mComponents;

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
