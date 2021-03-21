#pragma once

#include "../StructureComponent.h"

class InspectorViewComponent : public StructureComponent
{
protected:
	InspectorViewComponent(SKey key) : StructureComponent(key) {}

public:
	// TODO: This isn't a composable design.
	// Several components should be able to hook into this component.
	virtual StringTable createInspectorViewTable() = 0;
};
