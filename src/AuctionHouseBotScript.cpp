/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
 * Ported to SkyFire 5.4.8
*/

#include "Chat.h"
#include "Common.h"
#include "ScriptMgr.h"
#include "AuctionHouseBot.h"
#include "ahbot_config.h"
#include "ahbot_log.h"
#include "RBAC.h"

class AHBot_WorldScript : public WorldScript
{
private:
    bool HasPerformedStartup;
    uint32 UpdateTimer;

public:
    AHBot_WorldScript() : WorldScript("AHBot_WorldScript"), HasPerformedStartup(false), UpdateTimer(0) { }

    void OnConfigLoad(bool /*reload*/) override
    {
        auctionbot->LoadModuleConfigFile();
        if (!auctionbot->IsModuleEnabled())
            return;

        auctionbot->InitializeConfiguration();
        if (HasPerformedStartup)
        {
            AHBotLogInfo("AuctionHouseBot: (Re)populating item candidate lists ...");
            auctionbot->PopulateItemCandidatesAndProportions();

            if (AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Enabled", false))
            {
                auctionbot->PopulateQuestRewardItemIDs();
                auctionbot->PopulateItemDropChances();
            }
        }
    }

    void OnStartup() override
    {
        auctionbot->LoadModuleConfigFile();
        if (!auctionbot->IsModuleEnabled())
            return;

        auctionbot->InitializeConfiguration();
        AHBotLogInfo("AuctionHouseBot: (Re)populating item candidate lists ...");
        auctionbot->PopulateItemCandidatesAndProportions();
        if (AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Enabled", false))
        {
            auctionbot->PopulateQuestRewardItemIDs();
            auctionbot->PopulateItemDropChances();
        }
        HasPerformedStartup = true;
    }

    void OnUpdate(uint32 diff) override
    {
        if (!HasPerformedStartup)
            return;

        UpdateTimer += diff;
        if (UpdateTimer < uint32(MINUTE * IN_MILLISECONDS))
            return;

        UpdateTimer = 0;
        auctionbot->Update();
        auctionbot->CleanupBotMail();
    }
};

class AHBot_CommandScript : public CommandScript
{
public:
    AHBot_CommandScript() : CommandScript("AHBot_CommandScript") { }

    std::vector<ChatCommand> GetCommands() const OVERRIDE
    {
        static std::vector<ChatCommand> AHBotCommandTable =
        {
            { "update", rbac::RBAC_PERM_COMMAND_RELOAD, true,  &HandleAHBotUpdateCommand, "", },
            { "reload", rbac::RBAC_PERM_COMMAND_RELOAD, true,  &HandleAHBotReloadCommand, "", },
            { "empty",  rbac::RBAC_PERM_COMMAND_RELOAD, true,  &HandleAHBotEmptyCommand,  "", },
            { "help",   rbac::RBAC_PERM_COMMAND_RELOAD, true,  &HandleAHBotHelpCommand,   "", },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "ahbot", rbac::RBAC_PERM_COMMAND_RELOAD, true, NULL, "", AHBotCommandTable },
        };

        return commandTable;
    }

    static bool HandleAHBotUpdateCommand(ChatHandler* handler, char const* /*args*/)
    {
        AHBotLogInfo("AuctionHouseBot: Updating Auction House...");
        handler->PSendSysMessage("AuctionHouseBot: Updating Auction House...");
        AuctionHouseBot::instance()->Update();
        AuctionHouseBot::instance()->CleanupBotMail();
        AHBotLogInfo("AuctionHouseBot: Auction House Updated.");
        handler->PSendSysMessage("AuctionHouseBot: Auction House Updated.");
        return true;
    }

    static bool HandleAHBotReloadCommand(ChatHandler* handler, char const* /*args*/)
    {
        AHBotLogInfo("AuctionHouseBot: Reloading Config...");
        handler->PSendSysMessage("AuctionHouseBot: Reloading Config...");

        AuctionHouseBot::instance()->LoadModuleConfigFile();
        AuctionHouseBot::instance()->InitializeConfiguration();
        AuctionHouseBot::instance()->PopulateItemCandidatesAndProportions();

        if (AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Enabled", false))
        {
            auctionbot->PopulateQuestRewardItemIDs();
            auctionbot->PopulateItemDropChances();
        }

        AHBotLogInfo("AuctionHouseBot: Config reloaded.");
        handler->PSendSysMessage("AuctionHouseBot: Config reloaded.");
        return true;
    }

    static bool HandleAHBotEmptyCommand(ChatHandler* handler, char const* /*args*/)
    {
        AHBotLogInfo("AuctionHouseBot: Emptying Auction House...");
        handler->PSendSysMessage("AuctionHouseBot: Emptying Auction House...");
        AuctionHouseBot::instance()->EmptyAuctionHouses();
        AuctionHouseBot::instance()->CleanupBotMail();
        AHBotLogInfo("AuctionHouseBot: Auction Houses Emptied.");
        handler->PSendSysMessage("AuctionHouseBot: Auction Houses Emptied.");
        return true;
    }

    static bool HandleAHBotHelpCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("AuctionHouseBot commands:");
        handler->PSendSysMessage("  .ahbot reload - Reloads configuration");
        handler->PSendSysMessage("  .ahbot empty  - Removes all AuctionHouseBot auctions");
        handler->PSendSysMessage("  .ahbot update - Runs an update cycle");
        return true;
    }
};

void AddAHBotScripts()
{
    new AHBot_WorldScript();
    new AHBot_CommandScript();
}
