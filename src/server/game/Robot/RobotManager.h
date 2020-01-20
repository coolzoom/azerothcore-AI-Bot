#ifndef ROBOT_MANAGER_H
#define ROBOT_MANAGER_H

#ifndef ROBOT_UPDATE_TIME_SPAN
# define ROBOT_UPDATE_TIME_SPAN 500
#endif

#ifndef ACTIVE_DELAY_MIN_TIME
# define ACTIVE_DELAY_MIN_TIME 300000
#endif

#ifndef ACTIVE_DELAY_MAX_TIME
# define ACTIVE_DELAY_MAX_TIME 600000
#endif

#include <string>
#include "Log.h"
#include "RobotConfig.h"
#include "RobotEntity.h"
#include "AccountMgr.h"
#include <iostream>
#include <sstream>
#include "Player.h"
#include "Pet.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "MotionMaster.h"
#include "MapManager.h"
#include "Group.h"
#include "RobotAI.h"
#include "Item.h"
#include "Strategies/Strategy_Solo_Normal.h"

class RobotEntity;
class RobotAI;
class Strategy_Solo_Normal;

class RobotManager
{
    RobotManager();
    RobotManager(RobotManager const&) = delete;
    RobotManager& operator=(RobotManager const&) = delete;
    ~RobotManager() = default;

public:
    void UpdateRobots(uint32 pmDiff);    
    bool DeleteRobots();
    bool RobotsDeleted();
    bool CreateRobotAccount(std::string pmAccountName);
	bool CreateRobotCharacter(RobotEntity* pRE);
    bool LoginRobot(RobotEntity* pRE);
    void LogoutRobots();
    bool LogoutRobot(RobotEntity* pmRE);
    void HandlePlayerSay(Player* pmPlayer, std::string pmContent);
    bool StringEndWith(const std::string &str, const std::string &tail);
    bool StringStartWith(const std::string &str, const std::string &head);
    std::vector<std::string> SplitString(std::string srcStr, std::string delimStr, bool repeatedCharIgnored);
    std::string TrimString(std::string srcStr);
    static RobotManager* instance();

public:
	std::set<RobotEntity*> robotSet;
	std::set<uint32> activePlayerLevelSet;	
	std::map<uint8, std::vector<uint32>> availableRaces;
    std::vector<std::string> availableRobotNamesVector;
    int32 checkDelay;

    std::unordered_map<uint8, std::unordered_map<uint8, std::string>> characterTalentTabNameMap;
    std::set<uint32> deleteRobotAccountSet;
    std::unordered_map<uint32, std::unordered_map<uint32, uint8>> robotAccountMap;    
    std::set<uint8> armorInventorySet;
    std::unordered_map<uint8, uint8> miscInventoryMap;
    // 0, staff 1, one hand sword 2, two hand sword 3, dagger 4, shield
    // 5, two hand axe 6, polearm
    // type | level range | index entry
    std::unordered_map<uint8, std::unordered_map<uint8, std::unordered_map<uint32, uint32>>> meleeWeaponMap;
    // 0, bow / cross bow / gun 1, wand 2, throw
    // type | level range | index entry
    std::unordered_map<uint8, std::unordered_map<uint8, std::unordered_map<uint32, uint32>>> rangeWeaponMap;
    // 0, cloth 1, leather 2, mail 3, plate
    // type | slot | level range | index entry
    std::unordered_map<uint8, std::unordered_map<uint8, std::unordered_map<uint8, std::unordered_map<uint32, uint32>>>> armorMap;
    // slot | level range | index entry
    std::unordered_map<uint8, std::unordered_map<uint8, std::unordered_map<uint32, uint32>>> miscMap;
    std::set<uint32> spellRewardClassQuestIDSet;
    // level range | index
    std::unordered_map<uint8, std::unordered_map<uint32, WorldLocation>> teleportCacheMap;

    std::unordered_map<uint32, uint32> tamableBeastEntryMap;
    std::unordered_map<std::string, std::set<uint32>> spellNameEntryMap;
	std::set<uint32> demonSpellBookSet;		

	uint32 maxLevel;
};

#define sRobotManager RobotManager::instance()

#endif
