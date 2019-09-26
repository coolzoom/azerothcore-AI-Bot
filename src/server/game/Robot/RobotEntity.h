#ifndef ROBOT_ENTITY_H
#define ROBOT_ENTITY_H

#include "RobotAI.h"

enum RobotStatus :uint8
{
	RobotStatus_None = 0,
	RobotStatus_OffLine,
	RobotStatus_Login_CheckingAccount,
	RobotStatus_Login_CreatingAccount,
	RobotStatus_Login_CheckingCharacter,
	RobotStatus_Login_CreatingCharacter,
	RobotStatus_Login_Entering,
	RobotStatus_Online,
	RobotStatus_Logout_Exiting,
	RobotStatus_Logout_Checking,
};

class RobotEntity
{
public:
	RobotEntity(uint32 pmRobotLevel, uint32 pmRobotClass, uint32 pmRobotRace);
	~RobotEntity();
	RobotAI* ai;
	Player* robotPlayer;
	RobotStatus rs;

public:
	uint32 accountID;
	uint32 characterGUID;
	uint32 robotRace;
	uint32 robotClass;
	uint32 robotLevel;
	int32 activeDelay;
	int32 deactiveDelay;
	std::string accountName;
};
#endif
