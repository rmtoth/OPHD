#pragma once

#include "../../StructureManager.h"
#include <NAS2D/Xml/XmlElement.h>

class Test01 : public StructureComponent
{
public:
	static constexpr UID uid = 20;

	Test01(SKey s) : StructureComponent(*s) {}

private:
};



SKey CreateTest01()
{

}

void SerializeTest01(SKey s, NAS2D::Xml::XmlElement* xml)
{

}

void DeserializeTest01(SKey s, NAS2D::Xml::XmlElement* xml)
{

}

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
