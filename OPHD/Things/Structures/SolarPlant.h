#pragma once

#include "PowerStructure.h"

#include "../../Constants.h"

const int SOLAR_PLANT_BASE_PRODUCUCTION = 2000;

class SolarPlant : public PowerStructure
{
public:
	SolarPlant(float meanSolarDistance) : PowerStructure(constants::SOLAR_PLANT,
		"structures/solar_plant.sprite",
		StructureClass::EnergyProduction,
		StructureID::SID_SOLAR_PLANT),
		mMeanSolarDistance(meanSolarDistance)
	{
		maxAge(1000);
		turnsToBuild(4);
		requiresCHAP(false);
	}

protected:
	int calculateMaxEnergyProduction() override
	{
		// Prevent possible dividing by zero
		float solarDistance = mMeanSolarDistance != 0 ? mMeanSolarDistance : 1;

		return static_cast<int>(SOLAR_PLANT_BASE_PRODUCUCTION / solarDistance);
	}

private:
	const float mMeanSolarDistance;
};
