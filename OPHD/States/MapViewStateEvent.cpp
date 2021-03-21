// ==================================================================================
// = This file implements the non-UI event handlers like factory production, robot
// = task completeion, etc.
// ==================================================================================
#include "MapViewState.h"

#include "../DirectionOffset.h"
#include "../StructureCatalogue.h"
#include "../StructureManager.h"
#include "../Map/TileMap.h"
#include "../Things/Robots/Robots.h"
#include "../Things/Structures/Structures.h"
#include "../Things/Structures/FoodProduction.h"

#include <NAS2D/Utility.h>


void MapViewState::pullRobotFromFactory(ProductType pt, Factory& factory)
{
	RobotCommand* rcc = getAvailableRobotCommand();

	if ((rcc != nullptr) || mRobotPool.commandCapacityAvailable())
	{
		Robot* robot = nullptr;

		switch (pt)
		{
		case ProductType::PRODUCT_DIGGER:
			robot = mRobotPool.addRobot(Robot::Type::Digger);
			robot->taskComplete().connect(this, &MapViewState::diggerTaskFinished);
			factory.pullProduct();
			checkRobotSelectionInterface(Robot::Type::Digger);
			break;

		case ProductType::PRODUCT_DOZER:
			robot = mRobotPool.addRobot(Robot::Type::Dozer);
			robot->taskComplete().connect(this, &MapViewState::dozerTaskFinished);
			factory.pullProduct();
			checkRobotSelectionInterface(Robot::Type::Dozer);
			break;

		case ProductType::PRODUCT_MINER:
			robot = mRobotPool.addRobot(Robot::Type::Miner);
			robot->taskComplete().connect(this, &MapViewState::minerTaskFinished);
			factory.pullProduct();
			checkRobotSelectionInterface(Robot::Type::Miner);
			break;

		default:
			throw std::runtime_error("pullRobotFromFactory():: unsuitable robot type.");
		}

		rcc->addRobot(robot);
	}
	else
	{
		factory.idle(IdleReason::FactoryInsufficientRobotCommandCapacity);
	}

}


/**
 * Called whenever a Factory's production is complete.
 */
void MapViewState::factoryProductionComplete(Factory& factory)
{
	switch (factory.productWaiting())
	{
	case ProductType::PRODUCT_DIGGER:
		pullRobotFromFactory(ProductType::PRODUCT_DIGGER, factory);
		break;

	case ProductType::PRODUCT_DOZER:
		pullRobotFromFactory(ProductType::PRODUCT_DOZER, factory);
		break;

	case ProductType::PRODUCT_MINER:
		pullRobotFromFactory(ProductType::PRODUCT_MINER, factory);
		break;

	case ProductType::PRODUCT_TRUCK:
	case ProductType::PRODUCT_CLOTHING:
	case ProductType::PRODUCT_MEDICINE:
		{
			Warehouse* _wh = getAvailableWarehouse(factory.productWaiting(), 1);
			if (_wh) { _wh->products().store(factory.productWaiting(), 1); factory.pullProduct(); }
			else { factory.idle(IdleReason::FactoryInsufficientWarehouseSpace); }
			break;
		}

	default:
		std::cout << "Unknown Product." << std::endl;
		break;
	}
}


/**
 * Lands colonists on the surfaces and adds them to the population pool.
 */
void MapViewState::deployColonistLander()
{
	mPopulation.addPopulation(Population::PersonRole::ROLE_STUDENT, 10);
	mPopulation.addPopulation(Population::PersonRole::ROLE_WORKER, 20);
	mPopulation.addPopulation(Population::PersonRole::ROLE_SCIENTIST, 20);
}


/**
 * Lands cargo on the surface and adds resources to the resource pool.
 */
void MapViewState::deployCargoLander()
{
	auto cc = mTileMap->getTile(ccLocation(), 0).structure();
	GetComponent<FoodProduction>(cc).add(125);
	cc->storage() += StorableResources{ 25, 25, 15, 15 };

	updateStructuresAvailability();
}


/**
 * Sets up the initial colony deployment.
 *
 * \note	The deploy callback only gets called once so there is really no
 *			need to disconnect the callback since it will automatically be
 *			released when the seed lander is destroyed.
 */
void MapViewState::deploySeedLander(NAS2D::Point<int> point)
{
	// Bulldoze lander region
	for (const auto& direction : DirectionScan3x3)
	{
		mTileMap->getTile(point + direction).index(TerrainType::Dozed);
	}

	auto& structureManager = NAS2D::Utility<StructureManager>::get();

	// Place initial tubes
	for (const auto& direction : DirectionClockwise4)
	{
		structureManager.addStructure(new Tube(ConnectorDir::CONNECTOR_INTERSECTION, false), &mTileMap->getTile(point + direction));
	}

	// TOP ROW
	structureManager.addStructure(new SeedPower(), &mTileMap->getTile(point + DirectionNorthWest));

	Structure* cc = StructureCatalogue::get(StructureID::SID_COMMAND_CENTER);
	cc->sprite().setFrame(3);
	structureManager.addStructure(cc, &mTileMap->getTile(point + DirectionNorthEast));
	ccLocation() = point + DirectionNorthEast;

	// BOTTOM ROW
	SeedFactory* sf = static_cast<SeedFactory*>(StructureCatalogue::get(StructureID::SID_SEED_FACTORY));
	sf->resourcePool(&mResourcesCount);
	sf->productionComplete().connect(this, &MapViewState::factoryProductionComplete);
	sf->sprite().setFrame(7);
	structureManager.addStructure(sf, &mTileMap->getTile(point + DirectionSouthWest));

	SeedSmelter* ss = static_cast<SeedSmelter*>(StructureCatalogue::get(StructureID::SID_SEED_SMELTER));
	ss->sprite().setFrame(10);
	structureManager.addStructure(ss, &mTileMap->getTile(point + DirectionSouthEast));

	// Robots only become available after the SEED Factory is deployed.
	mRobots.addItem(constants::ROBODOZER, constants::ROBODOZER_SHEET_ID, static_cast<int>(Robot::Type::Dozer));
	mRobots.addItem(constants::ROBODIGGER, constants::ROBODIGGER_SHEET_ID, static_cast<int>(Robot::Type::Digger));
	mRobots.addItem(constants::ROBOMINER, constants::ROBOMINER_SHEET_ID, static_cast<int>(Robot::Type::Miner));
	mRobots.sort();

	mRobotPool.addRobot(Robot::Type::Dozer)->taskComplete().connect(this, &MapViewState::dozerTaskFinished);
	mRobotPool.addRobot(Robot::Type::Digger)->taskComplete().connect(this, &MapViewState::diggerTaskFinished);
	mRobotPool.addRobot(Robot::Type::Miner)->taskComplete().connect(this, &MapViewState::minerTaskFinished);
}


/**
 * Called whenever a RoboDozer completes its task.
 */
void MapViewState::dozerTaskFinished(Robot* /*robot*/)
{
	checkRobotSelectionInterface(Robot::Type::Dozer);
}


/**
 * Called whenever a RoboDigger completes its task.
 */
void MapViewState::diggerTaskFinished(Robot* robot)
{
	if (mRobotList.find(robot) == mRobotList.end()) { throw std::runtime_error("MapViewState::diggerTaskFinished() called with a Robot not in the Robot List!"); }

	Tile* t = mRobotList[robot];

	if (t->depth() > mTileMap->maxDepth())
	{
		throw std::runtime_error("Digger defines a depth that exceeds the maximum digging depth!");
	}

	Direction dir = static_cast<Robodigger*>(robot)->direction(); // fugly

	NAS2D::Point<int> origin = t->position();
	int newDepth = t->depth();

	if (dir == Direction::Down)
	{
		++newDepth;

		auto* as1 = CreateAirShaft();
		NAS2D::Utility<StructureManager>::get().addStructure(as1, t);

		auto* as2 = CreateAirShaft();
		NAS2D::Utility<StructureManager>::get().addStructure(as2, &mTileMap->getTile(origin, newDepth));

		mTileMap->getTile(origin, t->depth()).index(TerrainType::Dozed);
		mTileMap->getTile(origin, newDepth).index(TerrainType::Dozed);

		/// \fixme Naive approach; will be slow with large colonies.
		NAS2D::Utility<StructureManager>::get().disconnectAll();
		checkConnectedness();
	}
	else if (dir == Direction::North)
	{
		origin += DirectionNorth;
	}
	else if (dir == Direction::South)
	{
		origin += DirectionSouth;
	}
	else if (dir == Direction::West)
	{
		origin += DirectionWest;
	}
	else if (dir == Direction::East)
	{
		origin += DirectionEast;
	}

	/**
	 * \todo	Add checks for obstructions and things that explode if
	 *			a digger gets in the way (or should diggers be smarter than
	 *			puncturing a fusion reactor containment vessel?)
	 */
	for (const auto& offset : DirectionScan3x3)
	{
		mTileMap->getTile(origin + offset, newDepth).excavated(true);
	}

	checkRobotSelectionInterface(Robot::Type::Digger);
}


/**
 * Called whenever a RoboMiner completes its task.
 */
void MapViewState::minerTaskFinished(Robot* robot)
{
	if (mRobotList.find(robot) == mRobotList.end()) { throw std::runtime_error("MapViewState::minerTaskFinished() called with a Robot not in the Robot List!"); }

	auto& robotTile = *mRobotList[robot];

	// Surface structure
	MineFacility* mineFacility = new MineFacility(robotTile.mine());
	mineFacility->maxDepth(mTileMap->maxDepth());
	NAS2D::Utility<StructureManager>::get().addStructure(mineFacility, &robotTile);
	mineFacility->extensionComplete().connect(this, &MapViewState::mineFacilityExtended);

	// Tile immediately underneath facility.
	auto& tileBelow = mTileMap->getTile(robotTile.position(), robotTile.depth() + 1);
	NAS2D::Utility<StructureManager>::get().addStructure(new MineShaft(), &tileBelow);

	robotTile.index(TerrainType::Dozed);
	tileBelow.index(TerrainType::Dozed);
	tileBelow.excavated(true);

	robot->die();
}


void MapViewState::mineFacilityExtended(MineFacility* mineFacility)
{
	if (mMineOperationsWindow.mineFacility() == mineFacility) { mMineOperationsWindow.mineFacility(mineFacility); }

	auto& mineFacilityTile = NAS2D::Utility<StructureManager>::get().tileFromStructure(mineFacility);
	auto& mineDepthTile = mTileMap->getTile(mineFacilityTile.position(), mineFacility->mine()->depth());
	NAS2D::Utility<StructureManager>::get().addStructure(new MineShaft(), &mineDepthTile);
	mineDepthTile.index(TerrainType::Dozed);
	mineDepthTile.excavated(true);
}
