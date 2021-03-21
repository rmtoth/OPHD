#include "../../StructureManager.h"
#include "../../Constants/Strings.h"

Structure* CreateAirShaft()
{
	auto structure = new Structure(constants::AIR_SHAFT, "structures/air_shaft.sprite",
		constants::STRUCTURE_STATE_OPERATIONAL,
		Structure::StructureClass::Tube,
		StructureID::SID_AIR_SHAFT);

	structure->connectorDirection(ConnectorDir::CONNECTOR_VERTICAL);

	structure->requiresCHAP(false);
	structure->separateUgAnimation(true);
	structure->state(StructureState::Operational);

	return structure;
}
