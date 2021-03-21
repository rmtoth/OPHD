#include "FoodProduction.h"
#include "../../StructureManager.h"
#include "../../Constants/Strings.h"

const int AGRIDOME_CAPACITY = 1000;
const int AGRIDOME_BASE_PRODUCUCTION = 10;

Structure* CreateAgridomeStructure()
{
	Structure* structure = new Structure(constants::AGRIDOME, "structures/agridome.sprite", Structure::StructureClass::FoodProduction, StructureID::SID_AGRIDOME);

	structure->maxAge(600);
	structure->turnsToBuild(3);
	structure->requiresCHAP(true);
	structure->resourcesIn({ 1, 0, 0, 0 });
	structure->energyRequired(2);

	auto food = new FoodProduction(structure);
	food->foodCapacity(AGRIDOME_CAPACITY);
	food->foodProduction(AGRIDOME_BASE_PRODUCUCTION);

	auto inspector = new FoodProductionInspectorViewComponent(structure);

	NAS2D::Utility<StructureManager>::get().create(structure, food, inspector);

	return structure;
}
