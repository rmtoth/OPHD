#pragma once

#include "Structure.h"

#include "../../ProductPool.h"

class Warehouse : public StructureComponent
{
public:
	static constexpr UID uid = 20;

	Warehouse(Structure& structure) : StructureComponent(structure) {}

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

		Attach(new Warehouse(*this));
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(1);
	}
};
