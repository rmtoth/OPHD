#include "StructureManager.h"

#include "Constants.h"
#include "ProductPool.h"
#include "IOHelper.h"
#include "PopulationPool.h"
#include "Map/Tile.h"
#include "Things/Robots/Robot.h"
#include "Things/Structures/Structures.h"

#include "States/MapViewStateHelper.h" // <-- For removeRefinedResources()

#include <algorithm>
#include <sstream>


namespace {
	/**
	 * Fills population requirements fields in a Structure.
	 */
	static void fillPopulationRequirements(PopulationPool& populationPool, const PopulationRequirements& required, PopulationRequirements& available)
	{
		available[0] = std::min(required[0], populationPool.populationAvailable(Population::PersonRole::ROLE_WORKER));
		available[1] = std::min(required[1], populationPool.populationAvailable(Population::PersonRole::ROLE_SCIENTIST));
	}
}


bool StructureManager::CHAPAvailable()
{
	for (auto chap : mStructureLists[Structure::StructureClass::LifeSupport])
	{
		if (chap->operational()) { return true; }
	}

	return false;
}


void StructureManager::update(const StorableResources& resources, PopulationPool& population)
{
	// Called separately so that 1) high priority structures can be updated first and
	// 2) so that resource handling code (like energy) can be handled between update
	// calls to lower priority structures.
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Lander]); // No resource needs
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Command]); // Self sufficient
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::EnergyProduction]); // Nothing can work without energy

	updateEnergyProduction();

	// Basic resource production
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Mine]); // Can't operate without resources.
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Smelter]);

	updateStructures(resources, population, mStructureLists[Structure::StructureClass::LifeSupport]); // Air, water food must come before others
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::FoodProduction]);

	updateStructures(resources, population, mStructureLists[Structure::StructureClass::MedicalCenter]); // No medical facilities, people die
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Nursery]);

	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Factory]); // Production

	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Storage]); // Everything else.
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Park]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::SurfacePolice]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::UndergroundPolice]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::RecreationCenter]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Recycling]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Residence]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::RobotCommand]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Warehouse]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Laboratory]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Commercial]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::University]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Communication]);
	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Road]);

	updateStructures(resources, population, mStructureLists[Structure::StructureClass::Undefined]);

	assignColonistsToResidences(population);
}


void StructureManager::updateEnergyProduction()
{
	mTotalEnergyOutput = 0;
	mTotalEnergyUsed = 0;

	for (auto structure : mStructureLists[Structure::StructureClass::EnergyProduction])
	{
		auto powerStructure = static_cast<PowerStructure*>(structure);
		if (powerStructure->operational())
		{
			mTotalEnergyOutput += powerStructure->energyProduced();
		}
	}
}


/**
 * Updates the consumed energy used cache value. Does not
 * modify any structure values.
 */
void StructureManager::updateEnergyConsumed()
{
	mTotalEnergyUsed = 0;

	for (auto structureList : mStructureLists)
	{
		for (auto structure : structureList.second)
		{
			if (structure->operational() || structure->isIdle())
			{
				mTotalEnergyUsed += structure->energyRequirement();
			}
		}
	}
}


void StructureManager::assignColonistsToResidences(PopulationPool& population)
{
	int populationCount = population.size();
	for (auto structure : mStructureLists[Structure::StructureClass::Residence])
	{
		Residence* residence = static_cast<Residence*>(structure);
		if (residence->operational())
		{
			residence->assignColonists(populationCount);
			populationCount -= residence->assignedColonists();
		}
	}
}


void StructureManager::updateStructures(const StorableResources& resources, PopulationPool& population, StructureList& structures)
{
	Structure* structure = nullptr;
	for (std::size_t i = 0; i < structures.size(); ++i)
	{
		structure = structures[i];
		structure->update();

		// State Check
		// ASSUMPTION:	Construction sites are considered self sufficient until they are
		//				completed and connected to the rest of the colony.
		if (structure->underConstruction() || structure->destroyed())
		{
			continue;
		}

		// Connection Check
		if (!structureConnected(structure) && !structure->selfSustained())
		{
			structure->disable(DisabledReason::Disconnected);
			continue;
		}

		// CHAP Check
		if (structure->requiresCHAP() && !CHAPAvailable())
		{
			structure->disable(DisabledReason::Chap);
			continue;
		}

		// Population Check
		const auto& populationRequired = structure->populationRequirements();
		auto& populationAvailable = structure->populationAvailable();

		fillPopulationRequirements(population, populationRequired, populationAvailable);

		if ((populationAvailable[0] < populationRequired[0]) ||
			(populationAvailable[1] < populationRequired[1]))
		{
			structure->disable(DisabledReason::Population);
			continue;
		}

		if (structure->energyRequirement() > totalEnergyAvailable())
		{
			structure->disable(DisabledReason::Energy);
			continue;
		}

		// Check that enough resources are available for input.
		if (!structure->isIdle() && !(resources >= structure->resourcesIn()))
		{
			structure->disable(DisabledReason::RefinedResources);
			continue;
		}

		structure->enable();

		if (structure->operational() || structure->isIdle())
		{
			population.usePopulation(Population::PersonRole::ROLE_WORKER, populationRequired[0]);
			population.usePopulation(Population::PersonRole::ROLE_SCIENTIST, populationRequired[1]);

			auto consumed = structure->resourcesIn();
			removeRefinedResources(consumed);

			mTotalEnergyUsed += structure->energyRequirement();

			structure->think();
		}
	}
}


/**
 * Adds a new Structure to the StructureManager.
 */
void StructureManager::addStructure(Structure* structure, Tile* tile)
{
	// Sanity checks
	if (tile == nullptr)
	{
		return;
	}

	if (mStructureTileTable.find(structure) != mStructureTileTable.end())
	{
		throw std::runtime_error("StructureManager::addStructure(): Attempting to add a Structure that is already managed!");
	}

	// Remove things from tile only if we know we're adding a structure.
	if (!tile->empty())
	{
		tile->removeThing();
	}

	for (auto& component : structure->Components())
	{
		mComponents[component.first].push_back(component.second.get());
	}

	mStructureTileTable[structure] = tile;

	mStructureLists[structure->structureClass()].push_back(structure);
	tile->pushThing(structure);
}


/**
 * Removes a Structure from the StructureManager.
 *
 * \warning	A Structure removed from the StructureManager will be freed.
 *			Remaining pointers and references will be invalidated.
 */
void StructureManager::removeStructure(Structure* structure)
{
	StructureList& structures = mStructureLists[structure->structureClass()];

	auto structureIt = std::find(structures.begin(), structures.end(), structure);
	if (structureIt == structures.end())
	{
		throw std::runtime_error("StructureManager::removeStructure(): Attempting to remove a Structure that is not managed by the StructureManager.");
	}
	structures.erase(structureIt);

	for (auto& component : structure->Components())
	{
		StructureComponent::UID uid = component.first;
		StructureComponent* instance = component.second.get();
		auto uidComponents = mComponents.at(uid);
		auto componentIt = std::find(uidComponents.begin(), uidComponents.end(), instance);
		if (componentIt == uidComponents.end())
		{
			throw std::runtime_error("StructureManager::removeStructure(): Attempting to remove a StructureComponent that is not managed by the StructureManager.");
		}
		uidComponents.erase(componentIt);
	}

	auto tileTableIt = mStructureTileTable.find(structure);
	if (tileTableIt == mStructureTileTable.end())
	{
		throw std::runtime_error("StructureManager::removeStructure(): Attempting to remove a Structure that is not managed by the StructureManager.");
	}
	else
	{
		tileTableIt->second->deleteThing();
		mStructureTileTable.erase(tileTableIt);
	}
}


const StructureList& StructureManager::structureList(Structure::StructureClass structureClass)
{
	return mStructureLists[structureClass];
}


/**
 * Resets the 'connected' flag on all structures in the primary structure list.
 */
void StructureManager::disconnectAll()
{
	for (auto st_it = mStructureTileTable.begin(); st_it != mStructureTileTable.end(); ++st_it)
	{
		st_it->second->connected(false);
	}
}


/**
 * Returns the number of structures currently being managed by the StructureManager.
 */
int StructureManager::count() const
{
	int count = 0;
	for (auto it = mStructureLists.begin(); it != mStructureLists.end(); ++it)
	{
		count += static_cast<int>(it->second.size());
	}

	return count;
}


int StructureManager::getCountInState(Structure::StructureClass structureClass, StructureState state)
{
	int count = 0;
	for (const auto* structure : structureList(structureClass))
	{
		if (structure->state() == state)
		{
			++count;
		}
	}
	return count;
}


/**
 * Gets a count of the number of disabled buildings.
 */
int StructureManager::disabled()
{
	int count = 0;
	for (auto it = mStructureLists.begin(); it != mStructureLists.end(); ++it)
	{
		count += getCountInState(it->first, StructureState::Disabled);
	}

	return count;
}


/**
 * Gets a count of the number of destroyed buildings.
 */
int StructureManager::destroyed()
{
	int count = 0;
	for (auto it = mStructureLists.begin(); it != mStructureLists.end(); ++it)
	{
		count += getCountInState(it->first, StructureState::Destroyed);
	}

	return count;
}


void StructureManager::dropAllStructures()
{
	for (auto map_it = mStructureTileTable.begin(); map_it != mStructureTileTable.end(); ++map_it)
	{
		map_it->second->deleteThing();
	}

	mStructureTileTable.clear();
	mStructureLists.clear();
}


Tile& StructureManager::tileFromStructure(Structure* structure)
{
	auto it = mStructureTileTable.find(structure);
	if (it == mStructureTileTable.end())
	{
		throw std::runtime_error("Could not find tile for structure");
	}
	return *it->second;
}


void serializeStructure(NAS2D::Xml::XmlElement* _ti, Structure* structure, Tile* _t)
{
	const auto position = _t->position();
	_ti->attribute("x", position.x);
	_ti->attribute("y", position.y);
	_ti->attribute("depth", _t->depth());

	_ti->attribute("age", structure->age());
	_ti->attribute("state", static_cast<int>(structure->state()));
	_ti->attribute("forced_idle", structure->forceIdle());
	_ti->attribute("disabled_reason", static_cast<int>(structure->disabledReason()));
	_ti->attribute("idle_reason", static_cast<int>(structure->idleReason()));
	_ti->attribute("type", structure->structureId());
	_ti->attribute("direction", structure->connectorDirection());

	const auto& production = structure->production();
	if (production > StorableResources{ 0 })
	{
		writeResources(_ti, production, "production");
	}

	const auto& stored = structure->storage();
	if (stored > StorableResources{ 0 })
	{
		writeResources(_ti, stored, "storage");
	}

	_ti->attribute("pop0", structure->populationAvailable()[0]);
	_ti->attribute("pop1", structure->populationAvailable()[1]);
}


void StructureManager::serialize(NAS2D::Xml::XmlElement* element)
{
	auto* structures = new NAS2D::Xml::XmlElement("structures");

	for (auto& [structure, tile] : mStructureTileTable)
	{
		auto* structureElement = new NAS2D::Xml::XmlElement("structure");
		serializeStructure(structureElement, structure, tile);

		if (structure->isFactory())
		{
			structureElement->attribute("production_completed", static_cast<Factory*>(structure)->productionTurnsCompleted());
			structureElement->attribute("production_type", static_cast<Factory*>(structure)->productType());
		}

		if (structure->isWarehouse())
		{
			auto* warehouse_products = new NAS2D::Xml::XmlElement("warehouse_products");
			structure->Get<Warehouse>()->products().serialize(warehouse_products);
			structureElement->linkEndChild(warehouse_products);
		}

		if (structure->isRobotCommand())
		{
			auto* robotsElement = new NAS2D::Xml::XmlElement("robots");

			const auto& robots = structure->Get<RobotCommand>()->robots();

			std::stringstream str;
			for (std::size_t i = 0; i < robots.size(); ++i)
			{
				str << robots[i]->id();
				if (i != robots.size() - 1) { str << ","; } // kind of a kludge
			}

			robotsElement->attribute("robots", str.str());
			structureElement->linkEndChild(robotsElement);
		}

		if (structure->structureClass() == Structure::StructureClass::FoodProduction ||
			structure->structureId() == StructureID::SID_COMMAND_CENTER)
		{
			auto* food = new NAS2D::Xml::XmlElement("food");
			food->attribute("level", static_cast<FoodProduction*>(structure)->foodLevel());
			structureElement->linkEndChild(food);
		}

		if (structure->structureClass() == Structure::StructureClass::Residence)
		{
			Residence* residence = static_cast<Residence*>(structure);
			auto* waste = new NAS2D::Xml::XmlElement("waste");
			waste->attribute("accumulated", residence->wasteAccumulated());
			waste->attribute("overflow", residence->wasteOverflow());
			structureElement->linkEndChild(waste);
		}

		structures->linkEndChild(structureElement);
	}

	element->linkEndChild(structures);
}


bool StructureManager::structureConnected(Structure* structure)
{
	return mStructureTileTable[structure]->connected();
}
