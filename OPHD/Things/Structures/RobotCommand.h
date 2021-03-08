#pragma once

#include "Structure.h"
#include "../../Constants.h"

#include <vector>


class Robot;


class RobotCommandComponent : public StructureComponent
{
public:
	static constexpr UID uid = 10;

	RobotCommandComponent(Structure& structure) : StructureComponent(structure) {}

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
class RobotCommand : public Structure
{
public:
	RobotCommand() : Structure(constants::ROBOT_COMMAND,
		"structures/robot_control.sprite",
		StructureClass::RobotCommand,
		StructureID::SID_ROBOT_COMMAND)
	{
		maxAge(500);
		turnsToBuild(3);

		requiresCHAP(false);

		Attach(new RobotCommandComponent(*this));
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(5);
	}
};
