#pragma once

#include "../../Components/InspectorViewComponent.h"

class FoodProductionInspectorViewComponent : public InspectorViewComponent
{
public:
	static constexpr ComponentTypeID componentTypeID = 31;

	FoodProductionInspectorViewComponent(SKey key) : InspectorViewComponent(key) {}

	StringTable createInspectorViewTable() override;
};


/**
* \class	FoodProduction
* \brief	Virtual class for structures whose primary purpose is agricultural production
*
* \note	FoodProduction is an abstract class
*/
class FoodProduction : public StructureComponent
{
public:
	static constexpr ComponentTypeID componentTypeID = 30;

	FoodProduction(SKey key) : StructureComponent(key) {}

	NAS2D::Xml::XmlElement* serialize() const override;
	void deserialize(const NAS2D::Xml::XmlElement& element) override;

	int foodLevel() const { return mFoodLevel; }
	void foodLevel(int level) { mFoodLevel = std::clamp(level, 0, mFoodCapacity); }
	int foodCapacity() { return mFoodCapacity; }
	void foodCapacity(int capacity) { mFoodCapacity = capacity; }
	int foodProduction() { return std::min(mFoodProduction, mFoodCapacity - mFoodLevel); }
	void foodProduction(int production) { mFoodProduction = production; }

	void produceOrDecay();
	void add(int amount);
	int pull(int& amount);

protected:
	int mFoodLevel = 0;
	int mFoodCapacity = 0;
	int mFoodProduction = 0;
};
