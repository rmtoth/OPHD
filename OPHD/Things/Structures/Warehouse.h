#pragma once

#include "Structure.h"
#include "../../StructureManager.h"
#include <NAS2D/Utility.h>

#include "../../ProductPool.h"

class Warehouse : public StructureComponent
{
public:
	static constexpr ComponentTypeID componentTypeID = 20;

	Warehouse(SKey structure) : StructureComponent(structure) {}

	ProductPool& products() { return mProducts; }

private:
	ProductPool mProducts;
};

class WarehouseStructure : public Structure
{
public:
	WarehouseStructure() : Structure(constants::WAREHOUSE,
		"structures/warehouse.sprite",
		StructureClass::Warehouse,
		StructureID::SID_WAREHOUSE)
	{
		maxAge(500);
		turnsToBuild(2);

		requiresCHAP(false);

		NAS2D::Utility<StructureManager>::get().create(this, new Warehouse(this));
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(1);
	}
};
