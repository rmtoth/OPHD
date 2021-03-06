#pragma once

#include "Structure.h"

#include "../../Constants.h"

class MedicalCenter : public Structure
{
public:
	MedicalCenter() : Structure(constants::MEDICAL_CENTER,
		"structures/medical.sprite",
		StructureClass::MedicalCenter,
		StructureID::SID_MEDICAL_CENTER)
	{
		maxAge(500);
		turnsToBuild(4);

		requiresCHAP(true);
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(5);
	}
};
