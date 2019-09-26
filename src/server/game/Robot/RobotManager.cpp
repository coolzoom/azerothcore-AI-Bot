#include "RobotManager.h"

RobotManager::RobotManager()
{
    if (!sRobotConfig->enable)
    {
        return;
    }
    if (sRobotConfig->reset)
    {
        if (DeleteRobots())
        {
            sWorld->ShutdownServ(2000, SHUTDOWN_MASK_RESTART, 0);
            return;
        }
    }

    checkDelay = ROBOT_UPDATE_TIME_SPAN;

    maxLevel = 60;

    robotSet.clear();
    activePlayerLevelSet.clear();    
    availableRobotNamesVector.clear();

    std::set<std::string> usedNameSet;
    QueryResult currentNamesQR = CharacterDatabase.Query("SELECT name FROM characters");
    if (currentNamesQR)
    {
        do
        {
            Field* fields = currentNamesQR->Fetch();
            std::string eachName = fields[0].GetString();
            usedNameSet.insert(eachName);
        } while (currentNamesQR->NextRow());
    }
    QueryResult robotNamesQR = WorldDatabase.Query("SELECT name FROM robot_names order by rand()");
    if (!robotNamesQR)
    {
        sLog->outError("Found zero robot names");
        sRobotConfig->enable = false;
        return;
    }
    do
    {
        Field* fields = robotNamesQR->Fetch();
        std::string eachName = fields[0].GetString();
        if (usedNameSet.find(eachName) == usedNameSet.end())
        {
            availableRobotNamesVector.push_back(eachName);
        }
    } while (robotNamesQR->NextRow());

    availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
    availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);

    availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
    availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
    availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
    availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
    availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD_PLAYER);

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);

    availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);

    robotAccountMap.clear();
    deleteRobotAccountSet.clear();

    armorInventorySet.insert(InventoryType::INVTYPE_CHEST);
    armorInventorySet.insert(InventoryType::INVTYPE_FEET);
    armorInventorySet.insert(InventoryType::INVTYPE_HANDS);
    armorInventorySet.insert(InventoryType::INVTYPE_HEAD);
    armorInventorySet.insert(InventoryType::INVTYPE_LEGS);
    armorInventorySet.insert(InventoryType::INVTYPE_SHOULDERS);
    armorInventorySet.insert(InventoryType::INVTYPE_WAIST);
    armorInventorySet.insert(InventoryType::INVTYPE_WRISTS);

    miscInventoryMap[0] = InventoryType::INVTYPE_CLOAK;
    miscInventoryMap[1] = InventoryType::INVTYPE_FINGER;
    miscInventoryMap[2] = InventoryType::INVTYPE_FINGER;
    miscInventoryMap[3] = InventoryType::INVTYPE_NECK;

    characterTalentTabNameMap.clear();
    characterTalentTabNameMap[Classes::CLASS_WARRIOR][0] = "Arms";
    characterTalentTabNameMap[Classes::CLASS_WARRIOR][1] = "Fury";
    characterTalentTabNameMap[Classes::CLASS_WARRIOR][2] = "Protection";

    characterTalentTabNameMap[Classes::CLASS_HUNTER][0] = "Beast Mastery";
    characterTalentTabNameMap[Classes::CLASS_HUNTER][1] = "Marksmanship";
    characterTalentTabNameMap[Classes::CLASS_HUNTER][2] = "Survival";

    characterTalentTabNameMap[Classes::CLASS_SHAMAN][0] = "Elemental";
    characterTalentTabNameMap[Classes::CLASS_SHAMAN][1] = "Enhancement";
    characterTalentTabNameMap[Classes::CLASS_SHAMAN][2] = "Restoration";

    characterTalentTabNameMap[Classes::CLASS_PALADIN][0] = "Holy";
    characterTalentTabNameMap[Classes::CLASS_PALADIN][1] = "Protection";
    characterTalentTabNameMap[Classes::CLASS_PALADIN][2] = "Retribution";

    characterTalentTabNameMap[Classes::CLASS_WARLOCK][0] = "Affliction";
    characterTalentTabNameMap[Classes::CLASS_WARLOCK][1] = "Demonology";
    characterTalentTabNameMap[Classes::CLASS_WARLOCK][2] = "Destruction";

    characterTalentTabNameMap[Classes::CLASS_PRIEST][0] = "Descipline";
    characterTalentTabNameMap[Classes::CLASS_PRIEST][1] = "Holy";
    characterTalentTabNameMap[Classes::CLASS_PRIEST][2] = "Shadow";

    characterTalentTabNameMap[Classes::CLASS_ROGUE][0] = "Assassination";
    characterTalentTabNameMap[Classes::CLASS_ROGUE][1] = "Combat";
    characterTalentTabNameMap[Classes::CLASS_ROGUE][2] = "subtlety";

    characterTalentTabNameMap[Classes::CLASS_MAGE][0] = "Arcane";
    characterTalentTabNameMap[Classes::CLASS_MAGE][1] = "Fire";
    characterTalentTabNameMap[Classes::CLASS_MAGE][2] = "Frost";

    characterTalentTabNameMap[Classes::CLASS_DRUID][0] = "Balance";
    characterTalentTabNameMap[Classes::CLASS_DRUID][1] = "Feral";
    characterTalentTabNameMap[Classes::CLASS_DRUID][2] = "Restoration";

    meleeWeaponMap.clear();
    rangeWeaponMap.clear();
    armorMap.clear();
    miscMap.clear();
    uint8 levelRange = 0;
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        const ItemTemplate* proto = sObjectMgr->GetItemTemplate(itr->first);
        if (!proto)
        {
            continue;
        }
        if (proto->Quality < 2)
        {
            continue;
        }
        levelRange = proto->RequiredLevel / 10;
        if (proto->InventoryType == InventoryType::INVTYPE_CLOAK || proto->InventoryType == InventoryType::INVTYPE_FINGER || proto->InventoryType == InventoryType::INVTYPE_NECK)
        {
            miscMap[proto->InventoryType][levelRange][miscMap[proto->InventoryType][levelRange].size()] = proto->ItemId;
            continue;
        }
        if (proto->Class == ItemClass::ITEM_CLASS_WEAPON)
        {
            switch (proto->SubClass)
            {
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_STAFF:
            {
                meleeWeaponMap[0][levelRange][meleeWeaponMap[0][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_SWORD:
            {
                meleeWeaponMap[1][levelRange][meleeWeaponMap[1][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_SWORD2:
            {
                meleeWeaponMap[2][levelRange][meleeWeaponMap[2][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_DAGGER:
            {
                meleeWeaponMap[3][levelRange][meleeWeaponMap[3][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_AXE2:
            {
                meleeWeaponMap[5][levelRange][meleeWeaponMap[5][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_POLEARM:
            {
                meleeWeaponMap[6][levelRange][meleeWeaponMap[6][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_BOW:
            {
                rangeWeaponMap[0][levelRange][rangeWeaponMap[0][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_CROSSBOW:
            {
                rangeWeaponMap[0][levelRange][rangeWeaponMap[0][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_GUN:
            {
                rangeWeaponMap[0][levelRange][rangeWeaponMap[0][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_WAND:
            {
                rangeWeaponMap[1][levelRange][rangeWeaponMap[1][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_THROWN:
            {
                rangeWeaponMap[2][levelRange][rangeWeaponMap[2][levelRange].size()] = proto->ItemId;
                continue;
            }
            default:
            {
                break;
            }
            }
        }
        else if (proto->Class == ItemClass::ITEM_CLASS_ARMOR)
        {
            switch (proto->SubClass)
            {
            case ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_CLOTH:
            {
                armorMap[0][proto->InventoryType][levelRange][armorMap[0][proto->InventoryType][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_LEATHER:
            {
                armorMap[1][proto->InventoryType][levelRange][armorMap[1][proto->InventoryType][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_MAIL:
            {
                armorMap[2][proto->InventoryType][levelRange][armorMap[2][proto->InventoryType][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_PLATE:
            {
                armorMap[3][proto->InventoryType][levelRange][armorMap[3][proto->InventoryType][levelRange].size()] = proto->ItemId;
                continue;
            }
            case ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_SHIELD:
            {
                meleeWeaponMap[4][levelRange][meleeWeaponMap[4][levelRange].size()] = proto->ItemId;
                continue;
            }
            default:
            {
                break;
            }
            }
        }
    }

    spellRewardClassQuestIDSet.clear();
    std::unordered_map<uint32, Quest*> questTemplates = sObjectMgr->GetQuestTemplates();
    for (auto const& questTemplatePair : questTemplates)
    {
        const Quest* qinfo = sObjectMgr->GetQuestTemplate(questTemplatePair.second->GetQuestId());
        if (!qinfo)
        {
            continue;
        }
        if (qinfo->GetRequiredClasses() > 0)
        {
            if (qinfo->GetRewSpellCast() > 0)
            {
                spellRewardClassQuestIDSet.insert(qinfo->GetQuestId());
            }
        }
    }

    teleportCacheMap.clear();
    QueryResult normalCreatureQR = WorldDatabase.Query("SELECT CT.maxlevel, C.map, C.position_x, C.position_y, C.position_z FROM creature_template CT join creature C on CT.entry = C.id where CT.rank = 0 and (C.map <> 0 or C.map <> 1 or C.map <> 530 or C.map <> 571)");
    if (normalCreatureQR)
    {
        do
        {
            Field* creatureField = normalCreatureQR->Fetch();
            uint8 level = creatureField[0].GetUInt8();
            level = level / 10;
            int mapID = creatureField[1].GetInt32();
            float x = creatureField[2].GetFloat();
            float y = creatureField[3].GetFloat();
            float z = creatureField[4].GetFloat();
            WorldLocation eachWL = WorldLocation(mapID, x, y, z, 0);
            teleportCacheMap[level][teleportCacheMap[level].size()] = eachWL;
        } while (normalCreatureQR->NextRow());
    }

    tamableBeastEntryMap.clear();

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator cIT = ctc->begin(); cIT != ctc->end(); ++cIT)
    {
        const CreatureTemplate* cInfo = sObjectMgr->GetCreatureTemplate(cIT->first);
        if (!cInfo)
        {
            continue;
        }
        if (cInfo->IsTameable(false))
        {
            tamableBeastEntryMap[tamableBeastEntryMap.size()] = cInfo->Entry;
        }
    }

    spellNameEntryMap.clear();
    std::vector<SpellInfo*> siVector = sSpellMgr->GetSpellInfoVector();
    for (std::vector<SpellInfo*>::iterator spellIT = siVector.begin(); spellIT != siVector.end(); spellIT++)
    {
        if (*spellIT)
        {
            spellNameEntryMap[(*spellIT)->SpellName[0]].insert((*spellIT)->Id);
        }
    }

    uint32 checkLevel = 1;
    while (checkLevel <= maxLevel)
    {
        for (int checkClass = Classes::CLASS_WARRIOR; checkClass <= Classes::CLASS_DRUID; checkClass++)
        {
            if (checkClass == 6 || checkClass == 10)
            {
                continue;
            }
            std::vector<uint32> targetRaces = availableRaces[checkClass];
            for (std::vector<uint32>::iterator raceIT = targetRaces.begin(); raceIT != targetRaces.end(); raceIT++)
            {
                RobotEntity* newRE = new RobotEntity(checkLevel, checkClass, *raceIT);
                robotSet.insert(newRE);
            }
        }
        checkLevel++;
    }

    demonSpellBookSet.clear();
    QueryResult demonTainerQR = WorldDatabase.Query("SELECT item FROM npc_vendor where entry = 5520");
    if (demonTainerQR)
    {
        do
        {
            Field* fields = demonTainerQR->Fetch();
            uint32 bookItemEntry = fields[0].GetUInt32();
            demonSpellBookSet.insert(bookItemEntry);
        } while (demonTainerQR->NextRow());
    }

    sLog->outBasic("Robot system ready");
}

RobotManager* RobotManager::instance()
{
    static RobotManager instance;
    return &instance;
}

void RobotManager::UpdateRobots(uint32 pmDiff)
{
    if (sRobotConfig->enable == 0)
    {
        return;
    }
    if (checkDelay > 0)
    {
        checkDelay -= pmDiff;
        return;
    }
    checkDelay = ROBOT_UPDATE_TIME_SPAN;

    for (std::set<RobotEntity*>::iterator robotIT = robotSet.begin(); robotIT != robotSet.end(); robotIT++)
    {
        RobotEntity* targetPRE = *robotIT;
        bool validLevel = (activePlayerLevelSet.find(targetPRE->robotLevel) != activePlayerLevelSet.end());
        switch (targetPRE->rs)
        {
        case RobotStatus_None:
        {
            break;
        }
        case RobotStatus_OffLine:
        {
            if (validLevel)
            {
                sLog->outBasic("Robot %s is valid", targetPRE->accountName.c_str());
                targetPRE->activeDelay = urand(ACTIVE_DELAY_MIN_TIME, ACTIVE_DELAY_MAX_TIME);
                targetPRE->rs = RobotStatus::RobotStatus_Login_CheckingAccount;
            }
            break;
        }
        case RobotStatus_Login_CheckingAccount:
        {
            if (validLevel)
            {
                if (targetPRE->activeDelay > 0)
                {
                    targetPRE->activeDelay -= ROBOT_UPDATE_TIME_SPAN;
                }
                else
                {
                    QueryResult accountQR = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", targetPRE->accountName.c_str());
                    if (accountQR)
                    {
                        Field* accountFields = accountQR->Fetch();
                        targetPRE->accountID = accountFields[0].GetUInt32();
                        sLog->outBasic("Robot account %d is OK", targetPRE->accountID);
                        targetPRE->rs = RobotStatus::RobotStatus_Login_CheckingCharacter;
                    }
                    else
                    {
                        sLog->outBasic("Robot %s to create account", targetPRE->accountName.c_str());
                        targetPRE->rs = RobotStatus::RobotStatus_Login_CreatingAccount;
                    }
                }
            }
            else
            {
                sLog->outError("Robot %s is invalid", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
            }
            break;
        }
        case RobotStatus_Login_CreatingAccount:
        {
            if (validLevel)
            {
                AccountOpResult aor = AccountMgr::CreateAccount(targetPRE->accountName, "robot");
                if (aor == AccountOpResult::AOR_OK)
                {
                    sLog->outBasic("Robot %s account created", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Login_CheckingAccount;
                }
                else
                {
                    sLog->outBasic("Robot %s account create failed", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_None;
                }
            }
            else
            {
                sLog->outBasic("Robot %s is invalid", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
            }
            break;
        }
        case RobotStatus_Login_CheckingCharacter:
        {
            if (validLevel)
            {
                QueryResult characterQR = CharacterDatabase.PQuery("SELECT guid FROM characters where account = %d", targetPRE->accountID);
                if (characterQR)
                {
                    Field* characterFields = characterQR->Fetch();
                    targetPRE->characterGUID = characterFields[0].GetUInt32();
                    LoginRobot(targetPRE);
                    sLog->outBasic("Robot %s to login", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Login_Entering;
                }
                else
                {
                    sLog->outBasic("Robot %s to create character", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Login_CreatingCharacter;
                }
            }
            else
            {
                sLog->outBasic("Robot %s is invalid", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
            }
            break;
        }
        case RobotStatus_Login_CreatingCharacter:
        {
            if (validLevel)
            {
                if (CreateRobotCharacter(targetPRE))
                {
                    sLog->outBasic("Robot %s to login", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Login_CheckingCharacter;
                }
                else
                {
                    sLog->outBasic("Robot %s character create failed", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_None;
                }
            }
            else
            {
                sLog->outBasic("Robot %s is invalid", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
            }
            break;
        }
        case RobotStatus_Login_Entering:
        {
            Player* targetP = ObjectAccessor::FindPlayer(targetPRE->characterGUID);
            if (targetP)
            {
                targetPRE->robotPlayer = targetP;
                targetPRE->robotPlayer->SetPvP(true);
                sLog->outBasic("Robot %s is online", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_Online;
            }
            break;
        }
        case RobotStatus_Online:
        {
            if (targetPRE->robotPlayer)
            {
                if (validLevel)
                {
                    if (!targetPRE->ai)
                    {
                        targetPRE->ai = new RobotAI(targetPRE->robotPlayer);
                        targetPRE->ai->InitializeCharacter(targetPRE->robotLevel);
                        targetPRE->ai->Refresh();
                        targetPRE->ai->Prepare();
                        targetPRE->ai->RandomTeleport();
                        targetPRE->robotPlayer->rai = targetPRE->ai;
                        std::ostringstream msgStream;
                        msgStream << targetPRE->robotPlayer->GetName() << " activated";
                        sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msgStream.str().c_str());
                    }
                    else
                    {
                        targetPRE->ai->Update();
                    }
                }
                else
                {
                    targetPRE->deactiveDelay = urand(ACTIVE_DELAY_MIN_TIME, ACTIVE_DELAY_MAX_TIME);
                    sLog->outBasic("Robot %s is invalid", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Logout_Exiting;
                }
            }
            break;
        }
        case RobotStatus_Logout_Exiting:
        {
            if (targetPRE->robotPlayer)
            {
                if (validLevel)
                {
                    sLog->outBasic("Robot %s is online", targetPRE->accountName.c_str());
                    targetPRE->rs = RobotStatus::RobotStatus_Online;
                }
                else
                {
                    if (targetPRE->deactiveDelay > 0)
                    {
                        if (targetPRE->ai)
                        {
                            targetPRE->ai->Update();
                        }
                        targetPRE->deactiveDelay -= pmDiff;
                    }
                    else
                    {
                        LogoutRobot(targetPRE);
                        sLog->outBasic("Robot %s to logout", targetPRE->accountName.c_str());
                        targetPRE->rs = RobotStatus::RobotStatus_Logout_Checking;
                    }
                }
            }
            break;
        }
        case RobotStatus_Logout_Checking:
        {
            if (!targetPRE->robotPlayer)
            {
                sLog->outBasic("Robot %s is offline", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
                delete targetPRE->ai;
            }
            else if (!targetPRE->robotPlayer->IsInWorld())
            {
                sLog->outBasic("Robot %s is offline", targetPRE->accountName.c_str());
                targetPRE->rs = RobotStatus::RobotStatus_OffLine;
                delete targetPRE->ai;
            }
        }
        default:
        {
            break;
        }
        }
    }
}

bool RobotManager::DeleteRobots()
{
    QueryResult accountQR = LoginDatabase.PQuery("SELECT id, username FROM account where username like '%s%%'", sRobotConfig->robotAccountNamePrefix.c_str());

    if (accountQR)
    {
        do
        {
            Field* fields = accountQR->Fetch();
            uint32 id = fields[0].GetUInt32();
            std::string userName = fields[1].GetString();
            deleteRobotAccountSet.insert(id);
            AccountMgr::DeleteAccount(id);
            sLog->outBasic("Delete robot account %d - %s", id, userName.c_str());
        } while (accountQR->NextRow());
        return true;
    }

    return false;
}

bool RobotManager::RobotsDeleted()
{
    for (std::set<uint32>::iterator it = deleteRobotAccountSet.begin(); it != deleteRobotAccountSet.end(); it++)
    {
        QueryResult accountQR = LoginDatabase.PQuery("SELECT id FROM account where id = '%d'", (*it));
        if (accountQR)
        {
            sLog->outBasic("Account %d is under deleting", (*it));
            return false;
        }
        QueryResult characterQR = CharacterDatabase.PQuery("SELECT count(*) FROM characters where account = '%d'", (*it));
        if (characterQR)
        {
            Field* fields = characterQR->Fetch();
            uint32 count = fields[0].GetUInt32();
            if (count > 0)
            {
                sLog->outBasic("Characters for account %d are under deleting", (*it));
                return false;
            }
        }
    }

    sLog->outBasic("Robot accounts are deleted");
    return true;
}

bool RobotManager::CreateRobotAccount(std::string pmAccountName)
{
    QueryResult accountQR = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", pmAccountName.c_str());
    if (accountQR)
    {
        return false;
    }
    AccountOpResult aor = AccountMgr::CreateAccount(pmAccountName, "robot");
    sLog->outBasic("Create robot account %s", pmAccountName.c_str());

    return true;
}

bool RobotManager::CreateRobotCharacter(RobotEntity* pRE)
{
    std::string currentName = availableRobotNamesVector.back();
    availableRobotNamesVector.pop_back();
    QueryResult checkNameQR = CharacterDatabase.PQuery("SELECT guid FROM characters where name = '%s'", currentName.c_str());
    if (checkNameQR)
    {
        sLog->outError("Character name %s is occupied", currentName.c_str());
        return false;
    }

    uint8 gender = 0, skin = 0, face = 0, hairStyle = 0, hairColor = 0, facialHair = 0;
    while (true)
    {
        gender = urand(0, 1);
        face = urand(0, 5);
        hairStyle = urand(0, 5);
        hairColor = urand(0, 5);
        facialHair = urand(0, 5);

        WorldPacket p;
        CharacterCreateInfo* cci = new CharacterCreateInfo(currentName, pRE->robotRace, pRE->robotClass, gender, skin, face, hairStyle, hairColor, facialHair, 0, p);
        WorldSession* eachSession = new WorldSession(pRE->accountID, NULL, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false, true, 0);
        eachSession->isRobot = true;
        Player* newPlayer = new Player(eachSession);
        if (!newPlayer->Create(sObjectMgr->GenerateLowGuid(HighGuid::HIGHGUID_PLAYER), cci))
        {
            newPlayer->CleanupsBeforeDelete();
            delete eachSession;
            delete newPlayer;
            delete cci;
            sLog->outError("Character create failed, %s %d %d", currentName.c_str(), pRE->robotClass, pRE->robotRace);
            continue;
        }
        delete cci;
        newPlayer->GetMotionMaster()->Initialize();
        newPlayer->setCinematic(2);
        newPlayer->SetAtLoginFlag(AT_LOGIN_NONE);
        newPlayer->SaveToDB(true, true);
        sWorld->AddSession(eachSession);
        sLog->outBasic("Create character %d - %s for account %d", newPlayer->GetGUID(), currentName.c_str(), pRE->accountID);
        break;
    }

    return true;
}

bool RobotManager::LoginRobot(RobotEntity* pRE)
{
    WorldSession* loginSession = sWorld->FindSession(pRE->accountID);
    if (!loginSession)
    {
        loginSession = new WorldSession(pRE->accountID, NULL, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false, true, 0);
        loginSession->isRobot = true;
        sWorld->AddSession(loginSession);
    }
    WorldPacket data(CMSG_PLAYER_LOGIN, 8);
    data << (uint64)pRE->characterGUID;
    loginSession->HandlePlayerLoginOpcode(data);
    sLog->outBasic("Log in character %d %d", pRE->accountID, pRE->characterGUID);

    return true;
}

void RobotManager::LogoutRobots()
{
    for (std::set<RobotEntity*>::iterator robotIT = robotSet.begin(); robotIT != robotSet.end(); robotIT++)
    {
        RobotEntity* targetPRE = *robotIT;
        if (LogoutRobot(targetPRE))
        {
            targetPRE->rs = RobotStatus::RobotStatus_OffLine;
            targetPRE->ai = NULL;
            delete targetPRE->ai;
        }
    }
}

bool RobotManager::LogoutRobot(RobotEntity* pmRE)
{
    if (pmRE->robotPlayer)
    {
        if (pmRE->robotPlayer->IsInWorld())
        {
            sLog->outBasic("Log out robot %s", pmRE->robotPlayer->GetName());

            std::ostringstream msgStream;
            msgStream << pmRE->robotPlayer->GetName() << " logged out";
            sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msgStream.str().c_str());
            pmRE->robotPlayer->GetSession()->LogoutPlayer(true);
            return true;
        }
    }

    return false;
}

void RobotManager::HandlePlayerSay(Player* pmPlayer, std::string pmContent)
{
    std::vector<std::string> commandVector = sRobotManager->SplitString(pmContent, " ", true);
    std::string commandName = commandVector.at(0);
    if (commandName == "role")
    {
        std::ostringstream replyStream;
        if (commandVector.size() > 1)
        {
            std::string newRole = commandVector.at(1);
            if (newRole == "dps")
            {
                pmPlayer->groupRole = 0;
            }
            else if (newRole == "tank")
            {
                pmPlayer->groupRole = 1;
            }
            else if (newRole == "healer")
            {
                pmPlayer->groupRole = 2;
            }
        }

        replyStream << "Your group role is ";
        switch (pmPlayer->groupRole)
        {
        case 0:
        {
            replyStream << "dps";
            break;
        }
        case 1:
        {
            replyStream << "tank";
            break;
        }
        case 2:
        {
            replyStream << "healer";
            break;
        }
        default:
        {
            replyStream << "dps";
            break;
        }
        }

        sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, replyStream.str().c_str(), pmPlayer);
    }
}

bool RobotManager::StringEndWith(const std::string& str, const std::string& tail)
{
    return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

bool RobotManager::StringStartWith(const std::string& str, const std::string& head)
{
    return str.compare(0, head.size(), head) == 0;
}

std::vector<std::string> RobotManager::SplitString(std::string srcStr, std::string delimStr, bool repeatedCharIgnored)
{
    std::vector<std::string> resultStringVector;
    std::replace_if(srcStr.begin(), srcStr.end(), [&](const char& c) {if (delimStr.find(c) != std::string::npos) { return true; } else { return false; }}/*pred*/, delimStr.at(0));
    size_t pos = srcStr.find(delimStr.at(0));
    std::string addedString = "";
    while (pos != std::string::npos) {
        addedString = srcStr.substr(0, pos);
        if (!addedString.empty() || !repeatedCharIgnored) {
            resultStringVector.push_back(addedString);
        }
        srcStr.erase(srcStr.begin(), srcStr.begin() + pos + 1);
        pos = srcStr.find(delimStr.at(0));
    }
    addedString = srcStr;
    if (!addedString.empty() || !repeatedCharIgnored) {
        resultStringVector.push_back(addedString);
    }
    return resultStringVector;
}

std::string RobotManager::TrimString(std::string srcStr)
{
    std::string result = srcStr;
    if (!result.empty())
    {
        result.erase(0, result.find_first_not_of(" "));
        result.erase(result.find_last_not_of(" ") + 1);
    }

    return result;
}
