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
	static constexpr UID uid = 10;

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
 * Implements the Robot Command structure.
 */
class RobotCommandStructure : public Structure
{
public:
	RobotCommandStructure() : Structure(constants::ROBOT_COMMAND,
		"structures/robot_control.sprite",
		StructureClass::RobotCommand,
		StructureID::SID_ROBOT_COMMAND)
	{
		maxAge(500);
		turnsToBuild(3);

		requiresCHAP(false);

		NAS2D::Utility<StructureManager>::get().create(this, new RobotCommand(this));
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(5);
	}
};
