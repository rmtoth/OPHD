#include "FoodProduction.h"
#include "../../StructureManager.h"
#include <algorithm>

StringTable FoodProductionInspectorViewComponent::createInspectorViewTable()
{
	FoodProduction& food = Get<FoodProduction>();

	StringTable stringTable(2, 2);

	stringTable[{0, 0}].text = "Food Stored:";
	stringTable[{1, 0}].text = std::to_string(food.foodLevel()) + " / " + std::to_string(food.foodCapacity());

	stringTable[{0, 1}].text = "Production Rate:";
	if (structure().disabled())
	{
		stringTable[{1, 1}].text = "disabled";
	}
	else if (structure().operational())
	{
		stringTable[{1, 1}].text = std::to_string(food.foodProduction());
	}
	else
	{
		stringTable[{1, 1}].text = "idle";
	}

	return stringTable;
}

void FoodProduction::produceOrDecay()
{
	if (structure().disabled())
	{
		mFoodLevel = 0;
	}
	else if (structure().operational())
	{
		add(mFoodProduction);

		if (mFoodLevel == mFoodCapacity)
		{
			structure().idle(IdleReason::InternalStorageFull);
		}
	}
}

void FoodProduction::add(int amount)
{
	mFoodLevel = std::min(mFoodLevel + amount, mFoodCapacity);
}

int FoodProduction::pull(int& amount)
{
	int pulled = std::min(amount, mFoodLevel);
	mFoodLevel -= pulled;
	amount -= pulled;
	return pulled;
}

NAS2D::Xml::XmlElement* FoodProduction::serialize() const
{
	auto* xml = new NAS2D::Xml::XmlElement("food");
	xml->attribute("level", mFoodLevel);
	xml->attribute("capacity", mFoodCapacity);
	xml->attribute("production", mFoodProduction);
	return xml;
}

void FoodProduction::deserialize(const NAS2D::Xml::XmlElement& xml)
{
	auto level = xml.attribute("level");
	auto capacity = xml.attribute("capacity");
	auto production = xml.attribute("production");

	if( !level.empty() )
		mFoodLevel = std::stoi(level);
	if( !capacity.empty() )
		mFoodCapacity = std::stoi(capacity);
	if (!production.empty())
		mFoodProduction = std::stoi(production);
}
