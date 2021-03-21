#pragma once

#include "Structure.h"
#include "../../StructureComponent.h"

class CHAP : public StructureComponent
{
public:
	static constexpr ComponentTypeID componentTypeID = 50;

	CHAP(SKey key) : StructureComponent(key) {}
};
