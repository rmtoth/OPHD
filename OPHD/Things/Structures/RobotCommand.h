#pragma once

#include "Structure.h"
#include "../../Constants.h"
#include "../../StructureManager.h"
#include <NAS2D/Utility.h>

#include <vector>


class Robot;


class RobotCommand : public StructureComponent
{
public:
	static constexpr ComponentTypeID componentTypeID = 10;

	RobotCommand(SKey s) : StructureComponent(s) {}

	bool isControlling(Robot* robot) const;

	bool commandCapacityAvailable() const;
	void addRobot(Robot* robot);
	void removeRobot(Robot* robot);

	const std::vector<Robot*>& robots() { return mRobotList; }

private:
	std::vector<Robot*> mRobotList;
};

/**
 * Creates the Robot Command structure.
 */
Structure* CreateRobotCommandStructure();
