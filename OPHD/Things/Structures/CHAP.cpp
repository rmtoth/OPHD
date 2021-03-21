#include "CHAP.h"
#include "../../StructureManager.h"
#include "../../Constants/Strings.h"

Structure* CreateCHAP()
{
	auto structure = new Structure(constants::CHAP, "structures/chap.sprite", Structure::StructureClass::LifeSupport, StructureID::SID_CHAP);

	structure->maxAge(600);
	structure->turnsToBuild(5);
	structure->requiresCHAP(false);
	structure->resourcesIn({ 2, 0, 1, 1 });
	structure->energyRequired(10);

	auto chap = new CHAP(structure);

	NAS2D::Utility<StructureManager>::get().create(structure, chap);

	return structure;
};
