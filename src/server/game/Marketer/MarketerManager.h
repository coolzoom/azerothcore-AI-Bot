#ifndef MARKETER_MANAGER_H
#define MARKETER_MANAGER_H

#include <string>
#include "Log.h"
#include "MarketerConfig.h"
#include "AuctionHouseMgr.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "RobotManager.h"

enum AuctionHouseID
{
    AUCTION_HOUSE_ID_ALLIANCE = 1,
    AUCTION_HOUSE_ID_HORDE = 6,
    AUCTION_HOUSE_ID_NEUTRAL = 7
};

class MarketerManager
{
    MarketerManager();
    MarketerManager(MarketerManager const&) = delete;
    MarketerManager& operator=(MarketerManager const&) = delete;
    ~MarketerManager() = default;

public:
    void ResetMarketer();
    bool UpdateSeller(uint32 pmDiff);
    bool UpdateBuyer(uint32 pmDiff);
    static MarketerManager* instance();

private:
    bool MarketEmpty();

public:
    std::unordered_set<uint32> vendorUnlimitItemSet;

    int32 buyerCheckDelay;
    int32 sellerCheckDelay;

private:
    std::vector<uint32> sellingItemEntryVector;
    std::vector<uint32> toSellVector;

    std::unordered_map<uint32, uint32> ahMmap;
};

#define sMarketerManager MarketerManager::instance()

#endif
