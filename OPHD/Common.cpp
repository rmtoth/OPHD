#include "Common.h"
#include "Constants.h"
#include "StructureManager.h"

#include "Things/Structures/Structure.h"
#include "Things/Structures/Warehouse.h"

#include <NAS2D/Utility.h>
#include <NAS2D/Filesystem.h>
#include <NAS2D/Xml/XmlDocument.h>
#include <NAS2D/Xml/XmlElement.h>

#include <SDL2/SDL.h>

#if defined(WINDOWS) || defined(WIN32)
#include <Windows.h>
#endif

#include <iostream>


using namespace NAS2D;


const std::map<Difficulty, std::string> DIFFICULTY_STRING_TABLE
{
	{ Difficulty::Beginner, constants::DIFFICULTY_BEGINNER },
	{ Difficulty::Easy, constants::DIFFICULTY_EASY },
	{ Difficulty::Medium, constants::DIFFICULTY_MEDIUM },
	{ Difficulty::Hard, constants::DIFFICULTY_HARD },
};

std::string StringFromDifficulty(const Difficulty& difficulty)
{
	return DIFFICULTY_STRING_TABLE.at(difficulty);
}

Difficulty DifficultyFromString(std::string difficultyStr)
{
	difficultyStr = NAS2D::toLowercase(difficultyStr);
	for(const auto& difficulty : DIFFICULTY_STRING_TABLE)
	{
		if (NAS2D::toLowercase(difficulty.second) == difficultyStr)
		{
			return difficulty.first;
		}
	}
	throw std::runtime_error(difficultyStr + " is not a valid difficulty.");
}


const std::map<StructureState, Color> STRUCTURE_COLOR_TABLE
{
	{ StructureState::UnderConstruction, Color{150, 150, 150, 100} },
	{ StructureState::Operational, Color{0, 185, 0} },
	{ StructureState::Idle, Color{0, 185, 0, 100} },
	{ StructureState::Disabled, Color{220, 0, 0} },
	{ StructureState::Destroyed, Color{220, 0, 0} }
};


const std::map<StructureState, Color> STRUCTURE_TEXT_COLOR_TABLE
{
	{ StructureState::UnderConstruction, Color{185, 185, 185, 100} },
	{ StructureState::Operational, Color{0, 185, 0} },
	{ StructureState::Idle, Color{0, 185, 0, 100} },
	{ StructureState::Disabled, Color{220, 0, 0} },
	{ StructureState::Destroyed, Color{220, 0, 0} }
};


const std::map<TerrainType, std::string> TILE_INDEX_TRANSLATION =
{
	{ TerrainType::Dozed, constants::TILE_INDEX_TRANSLATION_BULLDOZED },
	{ TerrainType::Clear, constants::TILE_INDEX_TRANSLATION_CLEAR },
	{ TerrainType::Rough, constants::TILE_INDEX_TRANSLATION_ROUGH },
	{ TerrainType::Difficult, constants::TILE_INDEX_TRANSLATION_DIFFICULT },
	{ TerrainType::Impassable, constants::TILE_INDEX_TRANSLATION_IMPASSABLE },
};


const std::map<MineProductionRate, std::string> MINE_YIELD_TRANSLATION =
{
	{ MineProductionRate::High , constants::MINE_YIELD_HIGH },
	{ MineProductionRate::Low , constants::MINE_YIELD_LOW },
	{ MineProductionRate::Medium , constants::MINE_YIELD_MEDIUM }
};


const std::map<DisabledReason, std::string> DISABLED_REASON_TABLE =
{
	{ DisabledReason::None, constants::STRUCTURE_DISABLED_NONE },

	{ DisabledReason::Chap, constants::STRUCTURE_DISABLED_CHAP },
	{ DisabledReason::Disconnected, constants::STRUCTURE_DISABLED_DISCONNECTED },
	{ DisabledReason::Energy, constants::STRUCTURE_DISABLED_ENERGY },
	{ DisabledReason::Population, constants::STRUCTURE_DISABLED_POPULATION },
	{ DisabledReason::RefinedResources, constants::STRUCTURE_DISABLED_REFINED_RESOURCES },
	{ DisabledReason::StructuralIntegrity, constants::STRUCTURE_DISABLED_STRUCTURAL_INTEGRITY }
};


const std::map<IdleReason, std::string> IDLE_REASON_TABLE =
{
	{ IdleReason::None, constants::STRUCTURE_IDLE_NONE },

	{ IdleReason::PlayerSet, constants::STRUCTURE_IDLE_PLAYER_SET },
	{ IdleReason::InternalStorageFull, constants::STRUCTURE_IDLE_INTERNAL_STORAGE_FULL },
	{ IdleReason::FactoryProductionComplete, constants::STRUCTURE_IDLE_FACTORY_PRODUCTION_COMPLETE },
	{ IdleReason::FactoryInsufficientResources, constants::STRUCTURE_IDLE_FACTORY_INSUFFICIENT_RESOURCES },
	{ IdleReason::FactoryInsufficientRobotCommandCapacity, constants::STRUCTURE_IDLE_FACTORY_INSUFFICIENT_ROBOT_COMMAND_CAPACITY },
	{ IdleReason::FactoryInsufficientWarehouseSpace, constants::STRUCTURE_IDLE_FACTORY_INSUFFICIENT_WAREHOUSE_SPACE },
	{ IdleReason::MineExhausted, constants::STRUCTURE_IDLE_MINE_EXHAUSTED },
	{ IdleReason::MineInactive, constants::STRUCTURE_IDLE_MINE_INACTIVE },
	{ IdleReason::InsufficientLuxuryProduct, constants::STRUCTURE_IDLE_INSUFFICIENT_LUXURY_PRODUCT }
};


const std::array MoraleStringTable =
{
	std::string("Terrible"),
	std::string("Poor"),
	std::string("Fair"),
	std::string("Good"),
	std::string("Excellent"),

	std::string("Morale is "),
	std::string("Births"),
	std::string("Deaths"),
	std::string("No active Food Production"),
	std::string("Parks & Arboretums"),
	std::string("Recreational Facilities"),
	std::string("Luxury Availability"),
	std::string("Residential Over Capacity"),
	std::string("Biowaste Overflowing"),
	std::string("Structures Disabled"),
	std::string("Structures Destroyed")
};


const std::string& moraleString(int index)
{
	return MoraleStringTable[index];
}


const std::string& moraleString(Morale m)
{
	return MoraleStringTable[static_cast<int>(m)];
}


int moraleStringTableCount()
{
	return static_cast<int>(MoraleStringTable.size());
}


/**
 * Description table for products.
 */
std::array<std::string, ProductType::PRODUCT_COUNT> PRODUCT_DESCRIPTION_TABLE =
{
	constants::ROBODIGGER,
	constants::ROBODOZER,
	constants::ROBOMINER,
	constants::ROBOEXPLORER,
	constants::TRUCK,

	"PRODUCT_RESERVED_AG_05",
	"PRODUCT_RESERVED_AG_06",
	"PRODUCT_RESERVED_AG_07",

	constants::ROAD_MATERIALS,
	constants::MAINTENANCE_SUPPLIES,

	"PRODUCT_RESERVED_AG_10",
	"PRODUCT_RESERVED_AG_11",
	"PRODUCT_RESERVED_AG_12",
	"PRODUCT_RESERVED_AG_13",
	"PRODUCT_RESERVED_AG_14",
	"PRODUCT_RESERVED_AG_15",

	"PRODUCT_RESERVED_AG_16",
	"PRODUCT_RESERVED_AG_17",
	"PRODUCT_RESERVED_AG_18",
	"PRODUCT_RESERVED_AG_19",
	"PRODUCT_RESERVED_AG_20",
	"PRODUCT_RESERVED_AG_21",
	"PRODUCT_RESERVED_AG_22",
	"PRODUCT_RESERVED_AG_23",

	"PRODUCT_RESERVED_AG_24",
	"PRODUCT_RESERVED_AG_25",
	"PRODUCT_RESERVED_AG_26",
	"PRODUCT_RESERVED_AG_27",
	"PRODUCT_RESERVED_AG_28",
	"PRODUCT_RESERVED_AG_29",
	"PRODUCT_RESERVED_AG_30",
	"PRODUCT_RESERVED_AG_31",


	// =====================================
	// = UNDERGROUND FACTORIES
	// =====================================
	constants::CLOTHING,
	constants::MEDICINE,
	"PRODUCT_RESERVED_UG_34",
	"PRODUCT_RESERVED_UG_35",
	"PRODUCT_RESERVED_UG_36",
	"PRODUCT_RESERVED_UG_37",
	"PRODUCT_RESERVED_UG_38",
	"PRODUCT_RESERVED_UG_39",

	"PRODUCT_RESERVED_UG_40",
	"PRODUCT_RESERVED_UG_41",
	"PRODUCT_RESERVED_UG_42",
	"PRODUCT_RESERVED_UG_43",
	"PRODUCT_RESERVED_UG_44",
	"PRODUCT_RESERVED_UG_45",
	"PRODUCT_RESERVED_UG_46",
	"PRODUCT_RESERVED_UG_47",

	"PRODUCT_RESERVED_UG_48",
	"PRODUCT_RESERVED_UG_49",
	"PRODUCT_RESERVED_UG_50",
	"PRODUCT_RESERVED_UG_51",
	"PRODUCT_RESERVED_UG_52",
	"PRODUCT_RESERVED_UG_53",
	"PRODUCT_RESERVED_UG_54",
	"PRODUCT_RESERVED_UG_55",

	"PRODUCT_RESERVED_UG_56",
	"PRODUCT_RESERVED_UG_57",
	"PRODUCT_RESERVED_UG_58",
	"PRODUCT_RESERVED_UG_59",
	"PRODUCT_RESERVED_UG_60",
	"PRODUCT_RESERVED_UG_61",
	"PRODUCT_RESERVED_UG_62",
	"PRODUCT_RESERVED_UG_63"
};


const std::array<std::string, 4> ResourceNamesRefined =
{
	{"Common Metals", "Common Minerals", "Rare Metals", "Rare Minerals" }
};


const std::array<std::string, 4> ResourceNamesOre =
{
	{"Common Metals Ore", "Common Minerals Ore", "Rare Metals Ore", "Rare Minerals Ore" }
};


const std::map<std::array<bool, 4>, std::string> IntersectionPatternTable =
{
	{ { true, false, true, false }, "left" },
	{ { true, false, false, false }, "left" },
	{ { false, false, true, false }, "left" },

	{ { false, true, false, true }, "right" },
	{ { false, true, false, false }, "right" },
	{ { false, false, false, true }, "right" },

	{ { false, false, false, false }, "intersection" },
	{ { true, true, false, false }, "intersection" },
	{ { false, false, true, true }, "intersection" },
	{ { false, true, true, true }, "intersection" },
	{ { true, true, true, false }, "intersection" },
	{ { true, true, true, true }, "intersection" },
	{ { true, false, false, true }, "intersection" },
	{ { false, true, true, false }, "intersection" },

	{ { false, true, true, true }, "intersection" },
	{ { true, false, true, true }, "intersection" },
	{ { true, true, false, true }, "intersection" },
	{ { true, true, true, false }, "intersection" }
};


Difficulty CURRENT_DIFFICULTY = Difficulty::Beginner;


#if defined(WINDOWS) || defined(WIN32)
#include <SDL2/SDL_syswm.h>
/**
 * Gets a Windows API HWND handle to the main
 * window of the application.
 * 
 * \note	This is only exposed for Windows applications.
 * 
 * \warning	Uses internal knowledge of NAS2D to force an
 *			export that should not be exposed. This will
 *			break if NAS2D changes its structure.
 */
HWND WIN32_getWindowHandle()
{
	SDL_SysWMinfo systemInfo;
	SDL_VERSION(&systemInfo.version);

	/** \fixme Evil hack exposing an internal NAS2D variable. */
	extern SDL_Window* underlyingWindow;
	if (SDL_GetWindowWMInfo(underlyingWindow, &systemInfo) != 1)
	{
		return nullptr;
	}

	return systemInfo.info.win.window;
}
#endif


const std::string& productDescription(ProductType type)
{
	if (type == ProductType::PRODUCT_NONE) { return constants::NONE; }

	return PRODUCT_DESCRIPTION_TABLE[static_cast<std::size_t>(type)];
}


ProductType productTypeFromDescription(const std::string& description)
{
	for (std::size_t i = 0; i < PRODUCT_DESCRIPTION_TABLE.size(); ++i)
	{
		if (PRODUCT_DESCRIPTION_TABLE[i] == description)
		{
			// dubious (and slow)
			return static_cast<ProductType>(i);
		}
	}

	return ProductType::PRODUCT_NONE;
}


const std::string& disabledReason(DisabledReason _d)
{
	return DISABLED_REASON_TABLE.at(_d);
}


const std::string& idleReason(IdleReason _i)
{
	return IDLE_REASON_TABLE.at(_i);
}


Color structureColorFromIndex(StructureState structureState)
{
	return STRUCTURE_COLOR_TABLE.at(structureState);
}


Color structureTextColorFromIndex(StructureState structureState)
{
	return STRUCTURE_TEXT_COLOR_TABLE.at(structureState);
}


/**
 * Bit of a kludge to get state of a window
 * outside of what the EventHandler provides.
 */
bool windowMaximized()
{
	extern SDL_Window* underlyingWindow;
	unsigned int flags = SDL_GetWindowFlags(underlyingWindow);
	return (flags & SDL_WINDOW_MAXIMIZED);
}


/**
 * Shows a message dialog box.
 */
void doNonFatalErrorMessage(const std::string& title, const std::string& msg)
{
#if defined(WINDOWS) || defined(WIN32)
	MessageBoxA(WIN32_getWindowHandle(), msg.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), msg.c_str(), NULL);
#endif
}


/**
 * Shows a message dialog box with no icon.
 */
void doAlertMessage(const std::string& title, const std::string& msg)
{
#if defined(WINDOWS) || defined(WIN32)
	MessageBoxA(WIN32_getWindowHandle(), msg.c_str(), title.c_str(), MB_OK | /*MB_ICONINFORMATION |*/ MB_TASKMODAL);
#else
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title.c_str(), msg.c_str(), NULL);
#endif
}


/**
 * Shows a message dialog box with Yes and No buttons.
 */
bool doYesNoMessage(const std::string& title, const std::string msg)
{
	bool yes = false;
#if defined(WINDOWS) || defined(WIN32)
	yes = (MessageBoxA(WIN32_getWindowHandle(), msg.c_str(), title.c_str(), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES);
#else
	const SDL_MessageBoxButtonData buttons[] =
	{
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" }
	};

	const SDL_MessageBoxData messageboxdata =
	{
		SDL_MESSAGEBOX_INFORMATION,
		nullptr,
		title.c_str(),
		msg.c_str(),
		SDL_arraysize(buttons),
		buttons,
		nullptr
	};

	int _bid = 0;
	SDL_ShowMessageBox(&messageboxdata, &_bid);
	yes = (_bid == 1);
#endif

	return yes;
}


/**
 * Checks a savegame version.
 * 
 * \throws	Throws a std::runtime_error if there are any errors with a savegame version, formation or missing root nodes.
 * 
 * \fixme	Find a more efficient way to do this.
 */
void checkSavegameVersion(const std::string& filename)
{
	auto xmlFile = Utility<Filesystem>::get().open(filename);

	NAS2D::Xml::XmlDocument doc;
	doc.parse(xmlFile.raw_bytes());
	if (doc.error())
	{
		throw std::runtime_error("Malformed savegame ('" + filename + "'). Error on Row " + std::to_string(doc.errorRow()) + ", Column " + std::to_string(doc.errorCol()) + ": " + doc.errorDesc());
	}

	NAS2D::Xml::XmlElement* root = doc.firstChildElement(constants::SAVE_GAME_ROOT_NODE);
	if (root == nullptr)
	{
		throw std::runtime_error("Root element in '" + filename + "' is not '" + constants::SAVE_GAME_ROOT_NODE + "'.");
	}

	std::string sg_version = root->attribute("version");
	if (sg_version != constants::SAVE_GAME_VERSION)
	{
		throw std::runtime_error("Savegame version mismatch: '" + filename + "'. Expected " + constants::SAVE_GAME_VERSION + ", found " + sg_version + ".");
	}
}


NAS2D::StringList split_string(const char *str, char delim)
{
	std::vector<std::string> result;
	do
	{
		const char *begin = str;
		while (*str != delim && *str) { str++; }
		result.push_back(std::string(begin, str));
	} while (0 != *str++);

	return result;
}


void drawBasicProgressBar(int x, int y, int width, int height, float percent, int padding)
{
	auto& renderer = Utility<Renderer>::get();
	renderer.drawBox(NAS2D::Rectangle{x, y, width, height}, NAS2D::Color{0, 185, 0});

	if (percent > 0.0f)
	{
		int bar_width = static_cast<int>(static_cast<float>(width - (padding + padding)) * percent);
		renderer.drawBoxFilled(NAS2D::Rectangle{x + padding, y + padding + 1, bar_width - 1, height - (padding + padding) - 1}, NAS2D::Color{0, 100, 0});
	}
}


int getTruckAvailability()
{
	int trucksAvailable = 0;

	for (auto warehouse : StructureManager::GetComponents<Warehouse>())
	{
		trucksAvailable += warehouse->products().count(ProductType::PRODUCT_TRUCK);
	}

	return trucksAvailable;
}


int pullTruckFromInventory()
{
	int trucksAvailable = getTruckAvailability();

	if (trucksAvailable == 0) { return 0; }

	for (auto warehouse : StructureManager::GetComponents<Warehouse>())
	{
		if (warehouse->products().pull(ProductType::PRODUCT_TRUCK, 1) > 0)
		{
			return 1;
		}
	}

	return 0;
}


int pushTruckIntoInventory()
{
	const int storageNeededForTruck = storageRequiredPerUnit(ProductType::PRODUCT_TRUCK);

	for (auto warehouse : StructureManager::GetComponents<Warehouse>())
	{
		if (warehouse->products().availableStorage() >= storageNeededForTruck)
		{
			warehouse->products().store(ProductType::PRODUCT_TRUCK, 1);
			return 1;
		}
	}

	return 0;
}
