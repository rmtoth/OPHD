#pragma once

#include "Things/Structures/Structure.h"
#include <NAS2D/Utility.h>

namespace NAS2D {
	namespace Xml {
		class XmlElement;
	}
}

class Tile;
class PopulationPool;
struct StorableResources;

typedef Structure* SKey;

class StructureComponent
{
public:
	typedef int UID;

	// Every subclass of StructureComponent should have the following field:
	//static constexpr UID uid = ...

private:
	SKey mKey;

protected:
	StructureComponent(SKey key) : mKey(key) {}

public:
	virtual ~StructureComponent() {}
	Structure& structure() const;
};


/**
 * Handles structure updating and resource management for structures.
 *
 * Keeps track of which structures are operational, idle and disabled.
 */
class StructureManager
{
private:
	std::map<StructureComponent::UID, std::map<SKey, StructureComponent*>> mComponents;

	// Structure isn't a StructureComponent right now, so special case it.
	std::map<SKey, Structure*> mStructures;

public:
	template<typename ComponentTy>
	const std::map<SKey, ComponentTy*>& enumerate()
	{
		return reinterpret_cast<std::map<SKey, ComponentTy*>&>(mComponents[ComponentTy::uid]);
	}
	// Structure isn't a StructureComponent right now, so special case it.
	template<>
	const std::map<SKey, Structure*>& enumerate<Structure>()
	{
		return mStructures;
	}

	template<typename ComponentTy>
	ComponentTy* get(SKey s)
	{
		auto& table = mComponents[ComponentTy::uid];
		auto it = table.find(s);
		if (it != table.end())
			return reinterpret_cast<ComponentTy*>(it->second);
		return nullptr;
	}
	// Structure isn't a StructureComponent right now, so special case it.
	template<>
	Structure* get<Structure>(SKey s)
	{
		return s;
	}

	SKey create(Structure* structure)
	{
		SKey s = structure;
		bool success = mStructures.insert(std::make_pair(s, structure)).second;
#if defined(_DEBUG)
		if (!success)
		{
			std::cout << "Trying to double-create a structure!!!" << std::endl;
			throw std::runtime_error("StructureManager::create() was repeatedly called on a Structure!");
		}
#endif
		return s;
	}

	template<typename ComponentTy, typename... AdditionalTys>
	SKey create(Structure* structure, ComponentTy* component, AdditionalTys... additionalArgs)
	{
		SKey s = create(structure, additionalArgs...);
		auto& table = mComponents[ComponentTy::uid];
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
		auto it = mStructures.find(s);
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
		for (auto& [uid, table] : mComponents)
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

// Shorthand
template<typename T>
inline T* GetComponent(SKey s)
{
	return NAS2D::Utility<StructureManager>::get().get<T>(s);
}

// Special case if SKey is Structure*
template<>
inline Structure* GetComponent<Structure>(SKey s)
{
	return s;
}

template<typename T>
inline const std::map<SKey,T*>& GetComponents()
{
	return NAS2D::Utility<StructureManager>::get().enumerate<T>();
}
