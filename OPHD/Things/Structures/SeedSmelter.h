#pragma once

#include "OreRefining.h"


class SeedSmelter : public OreRefining
{
	const int StorageCapacity = 500;

public:
	SeedSmelter() : OreRefining(constants::SEED_SMELTER,
		"structures/seed_1.sprite",
		StructureClass::Smelter,
		StructureID::SID_SEED_SMELTER)
	{
		maxAge(150);
		turnsToBuild(6);
		requiresCHAP(false);

		storageCapacity(StorageCapacity);
	}

protected:

private:
	void defineResourceInput() override
	{
		energyRequired(5);
	}
};
