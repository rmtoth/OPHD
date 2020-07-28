#pragma once

#include "Structure.h"

class StorageTanks : public Structure
{
public:
	StorageTanks() : Structure(constants::STORAGE_TANKS, "structures/storage_tanks.sprite", StructureClass::Storage)
	{
		sprite().play(constants::STRUCTURE_STATE_CONSTRUCTION);
		maxAge(500);
		turnsToBuild(2);

		requiresCHAP(false);

		storage().capacity(1000);
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(1);
	}
};
