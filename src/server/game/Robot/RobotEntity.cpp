#include "RobotEntity.h"

RobotEntity::RobotEntity(uint32 pmRobotLevel, uint32 pmRobotClass, uint32 pmRobotRace)
{	
	robotLevel = pmRobotLevel;
	robotClass = pmRobotClass;	
	robotRace = pmRobotRace;

	accountID = 0;
	characterGUID = 0;
	ai = NULL;
	rs = RobotStatus::RobotStatus_OffLine;
	activeDelay = 0;
	deactiveDelay = 0;

    std::stringstream accountNameStream;
    accountNameStream << sRobotConfig->robotAccountNamePrefix << "L" << robotLevel << "C" << robotClass << "R" << robotRace;
    accountName = accountNameStream.str();
}

RobotEntity::~RobotEntity()
{

}
