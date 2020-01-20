#include "MarketerManager.h"

MarketerManager::MarketerManager()
{
    buyerCheckDelay = 120000;
    sellerCheckDelay = 60000;

    ahMmap.clear();
    ahMmap[AuctionHouseID::AUCTION_HOUSE_ID_ALLIANCE] = 46190;
    ahMmap[AuctionHouseID::AUCTION_HOUSE_ID_HORDE] = 4656;
    ahMmap[AuctionHouseID::AUCTION_HOUSE_ID_NEUTRAL] = 23442;

    vendorUnlimitItemSet.clear();
    QueryResult vendorItemQR = WorldDatabase.Query("SELECT distinct item FROM npc_vendor where maxcount = 0");
    if (vendorItemQR)
    {
        do
        {
            Field* fields = vendorItemQR->Fetch();
            uint32 eachItemEntry = fields[0].GetUInt32();
            vendorUnlimitItemSet.insert(eachItemEntry);
        } while (vendorItemQR->NextRow());
    }

    sellingItemEntryVector.clear();
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        const ItemTemplate* proto = sObjectMgr->GetItemTemplate(itr->first);
        if (!proto)
        {
            continue;
        }
        if (proto->ItemLevel < 1)
        {
            continue;
        }
        if (proto->Quality < 1)
        {
            continue;
        }
        if (proto->Quality > 4)
        {
            continue;
        }
        if (proto->Bonding != ItemBondingType::NO_BIND && proto->Bonding != ItemBondingType::BIND_WHEN_EQUIPED && proto->Bonding != ItemBondingType::BIND_WHEN_USE)
        {
            continue;
        }
        if (proto->SellPrice == 0 && proto->BuyPrice == 0)
        {
            continue;
        }
        if (vendorUnlimitItemSet.find(proto->ItemId) != vendorUnlimitItemSet.end())
        {
            continue;
        }
        bool sellThis = false;
        switch (proto->Class)
        {
        case ItemClass::ITEM_CLASS_CONSUMABLE:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_CONTAINER:
        {
            if (proto->Quality >= 2)
            {
                sellThis = true;
            }
            break;
        }
        case ItemClass::ITEM_CLASS_WEAPON:
        {
            if (proto->Quality >= 2)
            {
                sellThis = true;
            }
            break;
        }
        case ItemClass::ITEM_CLASS_GEM:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_ARMOR:
        {
            if (proto->Quality >= 2)
            {
                sellThis = true;
            }
            break;
        }
        case ItemClass::ITEM_CLASS_REAGENT:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_PROJECTILE:
        {
            break;
        }
        case ItemClass::ITEM_CLASS_TRADE_GOODS:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_GENERIC:
        {
            break;
        }
        case ItemClass::ITEM_CLASS_RECIPE:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_MONEY:
        {
            break;
        }
        case ItemClass::ITEM_CLASS_QUIVER:
        {
            if (proto->Quality >= 2)
            {
                sellThis = true;
            }
            break;
        }
        case ItemClass::ITEM_CLASS_QUEST:
        {
            sellThis = true;
            break;
        }
        case ItemClass::ITEM_CLASS_KEY:
        {
            break;
        }
        case ItemClass::ITEM_CLASS_PERMANENT:
        {
            break;
        }
        case ItemClass::ITEM_CLASS_MISC:
        {
            if (proto->Quality > 1)
            {
                sellThis = true;
            }
        }
        default:
        {
            break;
        }
        }
        if (sellThis)
        {
            sellingItemEntryVector.push_back(proto->ItemId);
        }
    }

    sLog->outBasic("Marketer system ready");
}

MarketerManager* MarketerManager::instance()
{
    static MarketerManager instance;
    return &instance;
}

void MarketerManager::ResetMarketer()
{
    sLog->outBasic("Ready to reset marketer seller");
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    for (std::unordered_map<uint32, uint32>::iterator ahIT = ahMmap.begin(); ahIT != ahMmap.end(); ahIT++)
    {
        uint32 ahID = ahIT->first;
        AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ahID);
        AuctionHouseObject* aho = sAuctionMgr->GetAuctionsMap(ahEntry->faction);
        if (!aho)
        {
            sLog->outError("AuctionHouseObject is null");
            return;
        }
        std::set<uint32> auctionIDSet;
        auctionIDSet.clear();
        for (AuctionHouseObject::AuctionEntryMap::const_iterator aeIT = aho->GetAuctionsBegin(); aeIT != aho->GetAuctionsEnd(); ++aeIT)
        {
            auctionIDSet.insert(aeIT->first);
        }
        for (std::set<uint32>::iterator auctionIDIT = auctionIDSet.begin(); auctionIDIT != auctionIDSet.end(); auctionIDIT++)
        {
            AuctionEntry* eachAE = aho->GetAuction(*auctionIDIT);
            if (eachAE)
            {
                if (eachAE->owner == 0)
                {
                    uint32 itemEntry = eachAE->item_template;
                    sAuctionMgr->RemoveAItem(eachAE->item_guidlow);
                    aho->RemoveAuction(eachAE);
                    eachAE->DeleteFromDB(trans);
                    sLog->outBasic("Auction %d removed for auctionhouse %d", itemEntry, ahID);
                }
            }
        }
    }

    CharacterDatabase.CommitTransaction(trans);
    sLog->outBasic("Marketer seller reset");
}

bool MarketerManager::UpdateSeller(uint32 pmDiff)
{
    if (!sMarketerConfig->enable)
    {
        return false;
    }
    if (sellerCheckDelay > 0)
    {
        sellerCheckDelay -= pmDiff;
        return true;
    }
    if (sMarketerConfig->reset)
    {
        ResetMarketer();
        sMarketerConfig->reset = 0;
        sellerCheckDelay = 1 * MINUTE * IN_MILLISECONDS;
        return true;
    }
    if (toSellVector.size() > 0)
    {
        uint32 toSellItemEntry = toSellVector.back();
        toSellVector.pop_back();

        const ItemTemplate* proto = sObjectMgr->GetItemTemplate(toSellItemEntry);
        if (!proto)
        {
            return false;
        }
        uint32 stackCount = proto->Stackable;
        switch (proto->Class)
        {
        case ItemClass::ITEM_CLASS_GEM:
        {
            stackCount = 1;
            break;
        }
        case ItemClass::ITEM_CLASS_RECIPE:
        {
            stackCount = 1;
            break;
        }
        case ItemClass::ITEM_CLASS_QUEST:
        {
            stackCount = 1;
            break;
        }
        default:
        {
            break;
        }
        }
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        for (std::unordered_map<uint32, uint32>::iterator ahIT = ahMmap.begin(); ahIT != ahMmap.end(); ahIT++)
        {
            uint32 ahID = ahIT->first;
            AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ahID);
            AuctionHouseObject* aho = sAuctionMgr->GetAuctionsMap(ahEntry->faction);
            if (!aho)
            {
                sLog->outError("AuctionHouseObject is null");
                return false;
            }

            uint8 unitCount = 1;
            if (proto->Stackable > 1)
            {
                unitCount = urand(1, 5);
            }
            while (unitCount > 0)
            {
                Item* item = Item::CreateItem(toSellItemEntry, 1, nullptr);
                if (item)
                {
                    uint32 finalPrice = 0;
                    uint8 qualityMuliplier = 1;
                    if (proto->Quality > 2)
                    {
                        qualityMuliplier = proto->Quality - 1;
                    }
                    finalPrice = proto->SellPrice * stackCount * urand(10, 20);
                    if (finalPrice == 0)
                    {
                        finalPrice = proto->BuyPrice * stackCount * urand(2, 4);
                    }
                    if (finalPrice == 0)
                    {
                        break;
                    }
                    finalPrice = finalPrice * qualityMuliplier;

                    uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(proto->ItemId);
                    if (randomPropertyId != 0)
                    {
                        item->SetItemRandomProperties(randomPropertyId);
                    }
                    uint32 etime = 172800;
                    item->SetCount(stackCount);
                    uint32 dep = sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, item->GetCount());

                    AuctionEntry* auctionEntry = new AuctionEntry();
                    auctionEntry->Id = sObjectMgr->GenerateAuctionID();
                    auctionEntry->auctioneer = ahIT->second;
                    auctionEntry->owner = 0;
                    auctionEntry->item_guidlow = item->GetGUIDLow();
                    auctionEntry->item_template = item->GetEntry();
                    auctionEntry->startbid = finalPrice / 2;
                    auctionEntry->buyout = finalPrice;
                    auctionEntry->bidder = 0;
                    auctionEntry->bid = 0;
                    auctionEntry->deposit = sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, stackCount);
                    auctionEntry->auctionHouseEntry = ahEntry;
                    auctionEntry->expire_time = time(NULL) + 48 * HOUR;

                    item->SaveToDB(trans);
                    sAuctionMgr->AddAItem(item);
                    aho->AddAuction(auctionEntry);
                    auctionEntry->SaveToDB(trans);
                    sLog->outBasic("Auction %s added for auctionhouse %d", proto->Name1.c_str(), ahID);
                }
                unitCount--;
            }
        }
        CharacterDatabase.CommitTransaction(trans);
    }
    else if (MarketEmpty())
    {
        toSellVector.clear();
        for (std::vector<uint32>::iterator it = sellingItemEntryVector.begin(); it != sellingItemEntryVector.end(); it++)
        {
            toSellVector.push_back(*it);
        }
        sLog->outBasic("Marketer start selling");
    }
    else
    {
        sLog->outBasic("Marketer is not empty");
        sellerCheckDelay = 2 * HOUR * IN_MILLISECONDS;
    }

    return true;
}

bool MarketerManager::MarketEmpty()
{
    for (std::unordered_map<uint32, uint32>::iterator ahIT = ahMmap.begin(); ahIT != ahMmap.end(); ahIT++)
    {
        uint32 ahID = ahIT->first;
        AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ahID);
        AuctionHouseObject* aho = sAuctionMgr->GetAuctionsMap(ahEntry->faction);
        if (!aho)
        {
            sLog->outError("AuctionHouseObject is null");
            return false;
        }
        for (AuctionHouseObject::AuctionEntryMap::const_iterator aeIT = aho->GetAuctionsBegin(); aeIT != aho->GetAuctionsEnd(); ++aeIT)
        {
            Item* checkItem = sAuctionMgr->GetAItem(aeIT->second->item_guidlow);
            if (!checkItem)
            {
                continue;
            }
            if (aeIT->second->owner == 0)
            {
                return false;
            }
        }
    }
    return true;
}

bool MarketerManager::UpdateBuyer(uint32 pmDiff)
{
    if (!sMarketerConfig->enable)
    {
        return false;
    }
    if (buyerCheckDelay > 0)
    {
        buyerCheckDelay -= pmDiff;
        return true;
    }
    buyerCheckDelay = 2 * HOUR * IN_MILLISECONDS;
    sLog->outBasic("Ready to update marketer buyer");
    std::set<uint32> toBuyAuctionIDSet;
    for (std::unordered_map<uint32, uint32>::iterator ahIT = ahMmap.begin(); ahIT != ahMmap.end(); ahIT++)
    {
        uint32 ahID = ahIT->first;
        AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ahID);
        AuctionHouseObject* aho = sAuctionMgr->GetAuctionsMap(ahEntry->faction);
        if (!aho)
        {
            sLog->outError("AuctionHouseObject is null");
            return false;
        }
        toBuyAuctionIDSet.clear();
        for (AuctionHouseObject::AuctionEntryMap::const_iterator aeIT = aho->GetAuctionsBegin(); aeIT != aho->GetAuctionsEnd(); ++aeIT)
        {
            Item* checkItem = sAuctionMgr->GetAItem(aeIT->second->item_guidlow);
            if (!checkItem)
            {
                continue;
            }
            if (aeIT->second->owner == 0)
            {
                continue;
            }
            const ItemTemplate* destIT = sObjectMgr->GetItemTemplate(aeIT->second->item_template);
            if (destIT->SellPrice == 0 && destIT->BuyPrice == 0)
            {
                continue;
            }
            if (destIT->Quality < 1)
            {
                continue;
            }
            if (destIT->Quality > 4)
            {
                continue;
            }
            if (vendorUnlimitItemSet.find(aeIT->second->item_template) != vendorUnlimitItemSet.end())
            {
                continue;
            }
            uint8 buyThis = 1;

            if (!destIT)
            {
                continue;
            }
            switch (destIT->Class)
            {
            case ItemClass::ITEM_CLASS_CONSUMABLE:
            {
                buyThis = urand(0, 5);
                break;
            }
            case ItemClass::ITEM_CLASS_CONTAINER:
            {
                if (destIT->Quality > 2)
                {
                    buyThis = 0;
                }
                else if (destIT->Quality == 2)
                {
                    buyThis = urand(0, 5);
                }
                break;
            }
            case ItemClass::ITEM_CLASS_WEAPON:
            {
                if (destIT->Quality > 2)
                {
                    buyThis = 0;
                }
                else if (destIT->Quality == 2)
                {
                    buyThis = urand(0, 5);
                }
                break;
            }
            case ItemClass::ITEM_CLASS_GEM:
            {
                buyThis = 0;
                break;
            }
            case ItemClass::ITEM_CLASS_ARMOR:
            {
                if (destIT->Quality > 2)
                {
                    buyThis = 0;
                }
                else if (destIT->Quality == 2)
                {
                    buyThis = urand(0, 5);
                }
                break;
            }
            case ItemClass::ITEM_CLASS_REAGENT:
            {
                buyThis = urand(0, 5);
                break;
            }
            case ItemClass::ITEM_CLASS_PROJECTILE:
            {
                buyThis = urand(0, 10);
                break;
            }
            case ItemClass::ITEM_CLASS_TRADE_GOODS:
            {
                buyThis = 0;
                break;
            }
            case ItemClass::ITEM_CLASS_GENERIC:
            {
                break;
            }
            case ItemClass::ITEM_CLASS_RECIPE:
            {
                buyThis = urand(0, 3);
                break;
            }
            case ItemClass::ITEM_CLASS_MONEY:
            {
                break;
            }
            case ItemClass::ITEM_CLASS_QUIVER:
            {
                if (destIT->Quality > 2)
                {
                    buyThis = 0;
                }
                else if (destIT->Quality == 2)
                {
                    buyThis = urand(0, 5);
                }
                break;
            }
            case ItemClass::ITEM_CLASS_QUEST:
            {
                buyThis = urand(0, 5);
                break;
            }
            case ItemClass::ITEM_CLASS_KEY:
            {
                break;
            }
            case ItemClass::ITEM_CLASS_PERMANENT:
            {
                break;
            }
            default:
            {
                break;
            }
            }
            if (buyThis != 0)
            {
                continue;
            }
            uint32 basePrice = 0;
            uint32 priceMultiple = 0;
            uint8 qualityMuliplier = 1;
            if (destIT->Quality > 2)
            {
                qualityMuliplier = destIT->Quality - 1;
            }
            uint32 finalPrice = 0;
            if (destIT->SellPrice > 0)
            {
                finalPrice = destIT->SellPrice * checkItem->GetCount();
            }
            else
            {
                finalPrice = destIT->BuyPrice * checkItem->GetCount();
            }
            finalPrice = finalPrice * qualityMuliplier;
            priceMultiple = aeIT->second->buyout / finalPrice;
            if (priceMultiple > 10)
            {
                priceMultiple = priceMultiple * 2;
            }
            if (urand(0, priceMultiple) == 0)
            {
                toBuyAuctionIDSet.insert(aeIT->first);
            }
        }
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        for (std::set<uint32>::iterator toBuyIT = toBuyAuctionIDSet.begin(); toBuyIT != toBuyAuctionIDSet.end(); toBuyIT++)
        {
            AuctionEntry* destAE = aho->GetAuction(*toBuyIT);
            if (destAE)
            {
                destAE->bid = destAE->buyout;
                sAuctionMgr->SendAuctionSuccessfulMail(destAE, trans);
                sAuctionMgr->SendAuctionWonMail(destAE, trans);
                sAuctionMgr->RemoveAItem(destAE->item_guidlow);
                aho->RemoveAuction(destAE);
                destAE->DeleteFromDB(trans);
                delete destAE;
                destAE = nullptr;
                sLog->outBasic("Auction %d was bought by marketer buyer", *toBuyIT);
            }
        }
        CharacterDatabase.CommitTransaction(trans);
    }
    sLog->outBasic("Marketer buyer updated");
    return true;
}
