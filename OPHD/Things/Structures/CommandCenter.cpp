#include "CommandCenter.h"
#include "FoodProduction.h"
#include "../../StructureManager.h"
#include "../../Constants/Strings.h"

Structure* CreateCommandCenter()
{
	Structure* structure = new Structure(constants::COMMAND_CENTER,
		"structures/command_center.sprite",
		Structure::StructureClass::Command,
		StructureID::SID_COMMAND_CENTER);

	structure->maxAge(500);
	structure->turnsToBuild(4);
	structure->requiresCHAP(false);
	structure->selfSustained(true);
	structure->storageCapacity(constants::BASE_STORAGE_CAPACITY);

	auto food = new FoodProduction(structure);
	food->foodCapacity(constants::BASE_STORAGE_CAPACITY);

	auto inspector = new FoodProductionInspectorViewComponent(structure);

	NAS2D::Utility<StructureManager>::get().create(structure, food, inspector);

	return structure;
};
