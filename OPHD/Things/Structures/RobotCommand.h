#pragma once

#include "Structure.h"
#include "../../Constants.h"

#include <vector>


class Robot;


class RobotCommand : public StructureComponent
{
public:
	static constexpr UID uid = 10;

	RobotCommand(Structure& structure) : StructureComponent(structure) {}

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

		Attach(new RobotCommand(*this));
	}

protected:
	void defineResourceInput() override
	{
		energyRequired(5);
	}
};
