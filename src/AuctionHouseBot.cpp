/*
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2023+ Nathan Handley <https://github.com/NathanHandley>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ObjectMgr.h"
#include "AuctionHouseMgr.h"
#include "AuctionHouseBot.h"
#include "ahbot_config.h"
#include "ahbot_log.h"
#include "Common.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include "World.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemPrototype.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "DBCEnums.h"
#include "DBCStores.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Util.h"
#include <cmath>
#include <cstdio>
#include <memory>

#include <set>
#include <unordered_map>

using namespace std;

AuctionHouseBot::AuctionHouseBot() :
    debug_Out(false),
    debug_Out_Filters(false),
    SellingBotEnabled(false),
    BuyingBotEnabled(false),
    ReturnExpiredAuctionItemsToBot(false),
    CyclesBetweenBuyActionMin(1),
    CyclesBetweenBuyAction(1),
    CyclesBetweenBuyActionMax(1),
    CyclesBetweenSellActionMin(1),
    CyclesBetweenSellAction(1),
    CyclesBetweenSellActionMax(1),
    MaxBuyoutPriceInCopper(1000000000),
    CompleteItemValueOverrideEnabled(false),
    CompleteItemValueOverrideDoApplyBidVariations(false),
    CompleteItemValueOverrideDoApplyBuyoutVariations(false),
    BuyoutVariationReducePercent(0.15f),
    BuyoutVariationAddPercent(0.25f),
    BidVariationHighReducePercent(0),
    BidVariationLowReducePercent(0.25f),
    BuyoutBelowVendorVariationAddPercentEnabled(true),
    BuyoutBelowVendorVariationAddPercent(0.25f),
    BuyingBotBuyCandidatesPerBuyCycleMin(1),
    BuyingBotBuyCandidatesPerBuyCycleMax(1),
    ListingExpireTimeInSecondsMin(900),
    ListingExpireTimeInSecondsMax(86400),
    BuyingBotAcceptablePriceModifier(1),
    BuyingBotAlwaysBidMaxCalculatedPrice(false),
    BuyingBotWillBidAgainstPlayers(false),
    AHCharactersGUIDsForQuery(""),
    ItemsPerCycle(75),
    DisabledItemTextFilter(true),
    DisabledRecipeProducedItemFilterEnabled(false),
    ListedItemLevelRestrictedEnabled(false),
    ListedItemLevelRestrictedUseCraftedItemForCalculation(true),
    ListedItemLevelMin(0),
    ListedItemLevelMax(999),    
    ListedItemUseOrEquipRestrictedEnabled(false),
    ListedItemUseOrEquipRestrictMinLevel(0),
    ListedItemUseOrEquipRestrictMaxLevel(999),    
    RandomStackRatioConsumable(1),
    RandomStackRatioContainer(1),
    RandomStackRatioWeapon(1),
    RandomStackRatioGem(1),
    RandomStackRatioArmor(1),
    RandomStackRatioReagent(1),
    RandomStackRatioProjectile(1),
    RandomStackRatioTradeGood(1),
    RandomStackRatioGeneric(1),
    RandomStackRatioRecipe(1),
    RandomStackRatioQuiver(1),
    RandomStackRatioQuest(1),
    RandomStackRatioKey(1),
    RandomStackRatioMisc(1),
    RandomStackRatioGlyph(1),
    RandomStackIncrementConsumable(1),
    RandomStackIncrementContainer(1),
    RandomStackIncrementWeapon(1),
    RandomStackIncrementGem(1),
    RandomStackIncrementArmor(1),
    RandomStackIncrementReagent(1),
    RandomStackIncrementProjectile(1),
    RandomStackIncrementTradeGood(1),
    RandomStackIncrementGeneric(1),
    RandomStackIncrementRecipe(1),
    RandomStackIncrementQuiver(1),
    RandomStackIncrementQuest(1),
    RandomStackIncrementKey(1),
    RandomStackIncrementMisc(1),
    RandomStackIncrementGlyph(1),
    MaximumStackSizeConsumable(0),
    MaximumStackSizeContainer(0),
    MaximumStackSizeWeapon(0),
    MaximumStackSizeGem(0),
    MaximumStackSizeArmor(0),
    MaximumStackSizeReagent(0),
    MaximumStackSizeProjectile(0),
    MaximumStackSizeTradeGood(0),
    MaximumStackSizeGeneric(0),
    MaximumStackSizeRecipe(0),
    MaximumStackSizeQuiver(0),
    MaximumStackSizeQuest(0),
    MaximumStackSizeKey(0),
    MaximumStackSizeMisc(0),
    MaximumStackSizeGlyph(0),
    PriceMultiplierCategoryConsumable(1),
    PriceMultiplierCategoryContainer(1),
    PriceMultiplierCategoryWeapon(1),
    PriceMultiplierCategoryGem(1),
    PriceMultiplierCategoryArmor(1),
    PriceMultiplierCategoryReagent(1),
    PriceMultiplierCategoryProjectile(1),
    PriceMultiplierCategoryTradeGood(1),
    PriceMultiplierCategoryGeneric(1),
    PriceMultiplierCategoryRecipe(1),
    PriceMultiplierCategoryQuiver(1),
    PriceMultiplierCategoryQuest(1),
    PriceMultiplierCategoryKey(1),
    PriceMultiplierCategoryMisc(1),
    PriceMultiplierCategoryGlyph(1),
    PriceMultiplierItemLevelCategoryConsumable(0),
    PriceMultiplierItemLevelCategoryContainer(0),
    PriceMultiplierItemLevelCategoryWeapon(0),
    PriceMultiplierItemLevelCategoryGem(0),
    PriceMultiplierItemLevelCategoryArmor(0),
    PriceMultiplierItemLevelCategoryReagent(0),
    PriceMultiplierItemLevelCategoryProjectile(0),
    PriceMultiplierItemLevelCategoryTradeGood(0),
    PriceMultiplierItemLevelCategoryGeneric(0),
    PriceMultiplierItemLevelCategoryRecipe(0),
    PriceMultiplierItemLevelCategoryQuiver(0),
    PriceMultiplierItemLevelCategoryQuest(0),
    PriceMultiplierItemLevelCategoryKey(0),
    PriceMultiplierItemLevelCategoryMisc(0),
    PriceMultiplierItemLevelCategoryGlyph(0),
    PriceMultiplierQualityPoor(1),
    PriceMultiplierQualityNormal(1),
    PriceMultiplierQualityUncommon(1),
    PriceMultiplierQualityRare(1),
    PriceMultiplierQualityEpic(1),
    PriceMultiplierQualityLegendary(1),
    PriceMultiplierQualityArtifact(1),
    PriceMultiplierQualityHeirloom(1),
    UseItemSellPriceIfHigherThanPriceMinimumCenterBase(true),
    PriceMinimumCenterBaseConsumable(1),
    PriceMinimumCenterBaseContainer(1),
    PriceMinimumCenterBaseWeapon(1),
    PriceMinimumCenterBaseGem(1),
    PriceMinimumCenterBaseArmor(1),
    PriceMinimumCenterBaseReagent(1),
    PriceMinimumCenterBaseProjectile(1),
    PriceMinimumCenterBaseTradeGood(1),
    PriceMinimumCenterBaseGeneric(1),
    PriceMinimumCenterBaseRecipe(1),
    PriceMinimumCenterBaseQuiver(1),
    PriceMinimumCenterBaseQuest(1),
    PriceMinimumCenterBaseKey(1),
    PriceMinimumCenterBaseMisc(1),
    PriceMinimumCenterBaseGlyph(1),
    ListedItemIDRestrictedEnabled(false),
    ListedItemIDMin(0),
    ListedItemIDMax(200000),
    AdvancedListingRuleUseDropRatesEnabled(false),
    AdvancedListingRuleUseDropRatesWeaponEnabled(true),
    AdvancedListingRuleUseDropRatesArmorEnabled(true),
    AdvancedListingRuleUseDropRatesRecipeEnabled(true),
    AdvancedListingRuleUseDropRatesMinDropRate(0.005),
    LastBuyCycleCount(0),
    LastSellCycleCount(0),
    ActiveListMultipleItemID(0),
    RemainingListMultipleCount(0)
{
    AllianceConfig = FactionSpecificAuctionHouseConfig(2);
    HordeConfig = FactionSpecificAuctionHouseConfig(6);
    NeutralConfig = FactionSpecificAuctionHouseConfig(7);
}

AuctionHouseBot::~AuctionHouseBot()
{
}

uint32 AuctionHouseBot::GetStackSizeForItem(ItemTemplate const* itemProto) const
{
    // Determine the stack ratio based on class type
    if (itemProto == NULL)
        return 1;

    uint32 stackRatio = 0;
    switch (itemProto->Class)
    {
        case ITEM_CLASS_CONSUMABLE:     stackRatio = RandomStackRatioConsumable; break;
        case ITEM_CLASS_CONTAINER:      stackRatio = RandomStackRatioContainer; break;
        case ITEM_CLASS_WEAPON:         stackRatio = RandomStackRatioWeapon; break;
        case ITEM_CLASS_GEM:            stackRatio = RandomStackRatioGem; break;
        case ITEM_CLASS_REAGENT:        stackRatio = RandomStackRatioReagent; break;
        case ITEM_CLASS_ARMOR:          stackRatio = RandomStackRatioArmor; break;
        case ITEM_CLASS_PROJECTILE:     stackRatio = RandomStackRatioProjectile; break;
        case ITEM_CLASS_TRADE_GOODS:    stackRatio = RandomStackRatioTradeGood; break;
        case ITEM_CLASS_GENERIC:        stackRatio = RandomStackRatioGeneric; break;
        case ITEM_CLASS_RECIPE:         stackRatio = RandomStackRatioRecipe; break;
        case ITEM_CLASS_QUIVER:         stackRatio = RandomStackRatioQuiver; break;
        case ITEM_CLASS_QUEST:          stackRatio = RandomStackRatioQuest; break;
        case ITEM_CLASS_KEY:            stackRatio = RandomStackRatioKey; break;
        case ITEM_CLASS_MISCELLANEOUS:           stackRatio = RandomStackRatioMisc; break;
        case ITEM_CLASS_GLYPH:          stackRatio = RandomStackRatioGlyph; break;
        default:                        stackRatio = 0; break;
    }

    uint32 stackIncrement = 1;
    switch (itemProto->Class)
    {
        case ITEM_CLASS_CONSUMABLE:     stackIncrement = RandomStackIncrementConsumable; break;
        case ITEM_CLASS_CONTAINER:      stackIncrement = RandomStackIncrementContainer; break;
        case ITEM_CLASS_WEAPON:         stackIncrement = RandomStackIncrementWeapon; break;
        case ITEM_CLASS_GEM:            stackIncrement = RandomStackIncrementGem; break;
        case ITEM_CLASS_REAGENT:        stackIncrement = RandomStackIncrementReagent; break;
        case ITEM_CLASS_ARMOR:          stackIncrement = RandomStackIncrementArmor; break;
        case ITEM_CLASS_PROJECTILE:     stackIncrement = RandomStackIncrementProjectile; break;
        case ITEM_CLASS_TRADE_GOODS:    stackIncrement = RandomStackIncrementTradeGood; break;
        case ITEM_CLASS_GENERIC:        stackIncrement = RandomStackIncrementGeneric; break;
        case ITEM_CLASS_RECIPE:         stackIncrement = RandomStackIncrementRecipe; break;
        case ITEM_CLASS_QUIVER:         stackIncrement = RandomStackIncrementQuiver; break;
        case ITEM_CLASS_QUEST:          stackIncrement = RandomStackIncrementQuest; break;
        case ITEM_CLASS_KEY:            stackIncrement = RandomStackIncrementKey; break;
        case ITEM_CLASS_MISCELLANEOUS:           stackIncrement = RandomStackIncrementMisc; break;
        case ITEM_CLASS_GLYPH:          stackIncrement = RandomStackIncrementGlyph; break;
        default:                        stackIncrement = 1; break;
    }
    stackIncrement = std::max(stackIncrement, (uint32)1);

    uint32 configStackSizeMax = 0;
    switch (itemProto->Class)
    {
        case ITEM_CLASS_CONSUMABLE:     configStackSizeMax = MaximumStackSizeConsumable; break;
        case ITEM_CLASS_CONTAINER:      configStackSizeMax = MaximumStackSizeContainer; break;
        case ITEM_CLASS_WEAPON:         configStackSizeMax = MaximumStackSizeWeapon; break;
        case ITEM_CLASS_GEM:            configStackSizeMax = MaximumStackSizeGem; break;
        case ITEM_CLASS_REAGENT:        configStackSizeMax = MaximumStackSizeReagent; break;
        case ITEM_CLASS_ARMOR:          configStackSizeMax = MaximumStackSizeArmor; break;
        case ITEM_CLASS_PROJECTILE:     configStackSizeMax = MaximumStackSizeProjectile; break;
        case ITEM_CLASS_TRADE_GOODS:    configStackSizeMax = MaximumStackSizeTradeGood; break;
        case ITEM_CLASS_GENERIC:        configStackSizeMax = MaximumStackSizeGeneric; break;
        case ITEM_CLASS_RECIPE:         configStackSizeMax = MaximumStackSizeRecipe; break;
        case ITEM_CLASS_QUIVER:         configStackSizeMax = MaximumStackSizeQuiver; break;
        case ITEM_CLASS_QUEST:          configStackSizeMax = MaximumStackSizeQuest; break;
        case ITEM_CLASS_KEY:            configStackSizeMax = MaximumStackSizeKey; break;
        case ITEM_CLASS_MISCELLANEOUS:           configStackSizeMax = MaximumStackSizeMisc; break;
        case ITEM_CLASS_GLYPH:          configStackSizeMax = MaximumStackSizeGlyph; break;
        default:                        configStackSizeMax = 0; break;
    }

    if (stackRatio > urand(0, 99))
    {
        uint32 maxPossibleStackSize = itemProto->GetMaxStackSize();
        if (configStackSizeMax != 0)
            maxPossibleStackSize = std::min(configStackSizeMax, maxPossibleStackSize);
        uint32 numOfPossibleStackIncrements = (uint32)std::ceil((float)maxPossibleStackSize / (float)stackIncrement);
        uint32 numOfStacks = urand(1, numOfPossibleStackIncrements);
        uint32 randomStackSize = numOfStacks * stackIncrement;
        if (randomStackSize > maxPossibleStackSize)
            return maxPossibleStackSize;
        else
            return randomStackSize;
    }
    else
        return 1;
}

void AuctionHouseBot::CalculateItemValue(ItemTemplate const* itemProto, uint64& outBidPrice, uint64& outBuyoutPrice)
{
    if (CompleteItemValueOverrideEnabled == true)
    {
        auto it = CompleteItemValueOverrideItemListByItemID.find(itemProto->ItemId);
        if (it != CompleteItemValueOverrideItemListByItemID.end())
        {
            outBuyoutPrice = it->second;
            if (CompleteItemValueOverrideDoApplyBuyoutVariations == true)
                outBuyoutPrice = urand(outBuyoutPrice * (1.0f - BuyoutVariationReducePercent), outBuyoutPrice * (1.0f + BuyoutVariationAddPercent));

            if (CompleteItemValueOverrideDoApplyBidVariations == true)
            {
                float sellVarianceBidPriceTopPercent = 1.0f - BidVariationHighReducePercent;
                float sellVarianceBidPriceBottomPercent = 1.0f - BidVariationLowReducePercent;
                outBidPrice = urand(sellVarianceBidPriceBottomPercent * outBuyoutPrice, sellVarianceBidPriceTopPercent * outBuyoutPrice);
            }
            else
                outBidPrice = outBuyoutPrice;

            return;
        }
    }


    // Start with a buyout price related to the sell price, if configured
    if (UseItemSellPriceIfHigherThanPriceMinimumCenterBase == true)
        outBuyoutPrice = itemProto->SellPrice;
    else
        outBuyoutPrice = 1; // Avoid zeros

    // Get the price multipliers
    float classPriceMultiplier = 1;
    switch (itemProto->Class)
    {
    case ITEM_CLASS_CONSUMABLE:     classPriceMultiplier = PriceMultiplierCategoryConsumable; break;
    case ITEM_CLASS_CONTAINER:      classPriceMultiplier = PriceMultiplierCategoryContainer; break;
    case ITEM_CLASS_WEAPON:         classPriceMultiplier = PriceMultiplierCategoryWeapon; break;
    case ITEM_CLASS_GEM:            classPriceMultiplier = PriceMultiplierCategoryGem; break;
    case ITEM_CLASS_REAGENT:        classPriceMultiplier = PriceMultiplierCategoryReagent; break;
    case ITEM_CLASS_ARMOR:          classPriceMultiplier = PriceMultiplierCategoryArmor; break;
    case ITEM_CLASS_PROJECTILE:     classPriceMultiplier = PriceMultiplierCategoryProjectile; break;
    case ITEM_CLASS_TRADE_GOODS:    classPriceMultiplier = PriceMultiplierCategoryTradeGood; break;
    case ITEM_CLASS_GENERIC:        classPriceMultiplier = PriceMultiplierCategoryGeneric; break;
    case ITEM_CLASS_RECIPE:         classPriceMultiplier = PriceMultiplierCategoryRecipe; break;
    case ITEM_CLASS_QUIVER:         classPriceMultiplier = PriceMultiplierCategoryQuiver; break;
    case ITEM_CLASS_QUEST:          classPriceMultiplier = PriceMultiplierCategoryQuest; break;
    case ITEM_CLASS_KEY:            classPriceMultiplier = PriceMultiplierCategoryKey; break;
    case ITEM_CLASS_MISCELLANEOUS:           classPriceMultiplier = PriceMultiplierCategoryMisc; break;
    case ITEM_CLASS_GLYPH:          classPriceMultiplier = PriceMultiplierCategoryGlyph; break;
    default:                        break;
    }

    float qualityPriceMultplier = 1;
    switch (itemProto->Quality)
    {
    case ITEM_QUALITY_POOR:         qualityPriceMultplier = PriceMultiplierQualityPoor; break;
    case ITEM_QUALITY_NORMAL:       qualityPriceMultplier = PriceMultiplierQualityNormal; break;
    case ITEM_QUALITY_UNCOMMON:     qualityPriceMultplier = PriceMultiplierQualityUncommon; break;
    case ITEM_QUALITY_RARE:         qualityPriceMultplier = PriceMultiplierQualityRare; break;
    case ITEM_QUALITY_EPIC:         qualityPriceMultplier = PriceMultiplierQualityEpic; break;
    case ITEM_QUALITY_LEGENDARY:    qualityPriceMultplier = PriceMultiplierQualityLegendary; break;
    case ITEM_QUALITY_ARTIFACT:     qualityPriceMultplier = PriceMultiplierQualityArtifact; break;
    case ITEM_QUALITY_HEIRLOOM:     qualityPriceMultplier = PriceMultiplierQualityHeirloom; break;
    default: break;
    }

    // Custom items missing from Item.dbc skip the core's class/quality validation, so bounds check before indexing
    float classQualityPriceMultiplier = 1;
    if (itemProto->Class < MAX_ITEM_CLASS && itemProto->Quality < MAX_ITEM_QUALITY)
        classQualityPriceMultiplier = PriceMultiplierCategoryQuality[itemProto->Class][itemProto->Quality];

    float advancedPricingMultiplier = GetAdvancedPricingMultiplier(itemProto);

    // Grab the minimum prices
    uint64 PriceMinimumCenterBase = 1000;
    auto it = PriceMinimumCenterBaseOverridesByItemID.find(itemProto->ItemId);
    if (it != PriceMinimumCenterBaseOverridesByItemID.end())
        PriceMinimumCenterBase = it->second;
    else
    {
        switch (itemProto->Class)
        {
        case ITEM_CLASS_CONSUMABLE:     PriceMinimumCenterBase = PriceMinimumCenterBaseConsumable; break;
        case ITEM_CLASS_CONTAINER:      PriceMinimumCenterBase = PriceMinimumCenterBaseContainer; break;
        case ITEM_CLASS_WEAPON:         PriceMinimumCenterBase = PriceMinimumCenterBaseWeapon; break;
        case ITEM_CLASS_GEM:            PriceMinimumCenterBase = PriceMinimumCenterBaseGem; break;
        case ITEM_CLASS_REAGENT:        PriceMinimumCenterBase = PriceMinimumCenterBaseReagent; break;
        case ITEM_CLASS_ARMOR:          PriceMinimumCenterBase = PriceMinimumCenterBaseArmor; break;
        case ITEM_CLASS_PROJECTILE:     PriceMinimumCenterBase = PriceMinimumCenterBaseProjectile; break;
        case ITEM_CLASS_TRADE_GOODS:    PriceMinimumCenterBase = PriceMinimumCenterBaseTradeGood; break;
        case ITEM_CLASS_GENERIC:        PriceMinimumCenterBase = PriceMinimumCenterBaseGeneric; break;
        case ITEM_CLASS_RECIPE:         PriceMinimumCenterBase = PriceMinimumCenterBaseRecipe; break;
        case ITEM_CLASS_QUIVER:         PriceMinimumCenterBase = PriceMinimumCenterBaseQuiver; break;
        case ITEM_CLASS_QUEST:          PriceMinimumCenterBase = PriceMinimumCenterBaseQuest; break;
        case ITEM_CLASS_KEY:            PriceMinimumCenterBase = PriceMinimumCenterBaseKey; break;
        case ITEM_CLASS_MISCELLANEOUS:           PriceMinimumCenterBase = PriceMinimumCenterBaseMisc; break;
        case ITEM_CLASS_GLYPH:          PriceMinimumCenterBase = PriceMinimumCenterBaseGlyph; break;
        default:                        break;
        }
    }

    // Set the minimum price
    if (outBuyoutPrice < PriceMinimumCenterBase)
        outBuyoutPrice = urand(PriceMinimumCenterBase * (1.0f - BuyoutVariationReducePercent), PriceMinimumCenterBase * (1.0f + BuyoutVariationAddPercent));
    else
        outBuyoutPrice = urand(outBuyoutPrice * (1.0f - BuyoutVariationReducePercent), outBuyoutPrice * (1.0f + BuyoutVariationAddPercent));

    // Ensure no multipliers are zero or negative
    if (classPriceMultiplier <= 0.0f)
        classPriceMultiplier = 1.0f;
    if (qualityPriceMultplier <= 0.0f)
        qualityPriceMultplier = 1.0f;
    if (classQualityPriceMultiplier <= 0.0f)
        classQualityPriceMultiplier = 1.0f;
    if (advancedPricingMultiplier <= 0.0f)
        advancedPricingMultiplier = 1.0f;

    // Grab any item level price multipliers
    float itemLevelPriceMultplier = 0.0f;
    switch (itemProto->Class)
    {
        case ITEM_CLASS_CONSUMABLE:     itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryConsumable; break;
        case ITEM_CLASS_CONTAINER:      itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryContainer; break;
        case ITEM_CLASS_WEAPON:         itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryWeapon; break;
        case ITEM_CLASS_GEM:            itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryGem; break;
        case ITEM_CLASS_REAGENT:        itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryReagent; break;
        case ITEM_CLASS_ARMOR:          itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryArmor; break;
        case ITEM_CLASS_PROJECTILE:     itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryProjectile; break;
        case ITEM_CLASS_TRADE_GOODS:    itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryTradeGood; break;
        case ITEM_CLASS_GENERIC:        itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryGeneric; break;
        case ITEM_CLASS_RECIPE:         itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryRecipe; break;
        case ITEM_CLASS_QUIVER:         itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryQuiver; break;
        case ITEM_CLASS_QUEST:          itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryQuest; break;
        case ITEM_CLASS_KEY:            itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryKey; break;
        case ITEM_CLASS_MISCELLANEOUS:           itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryMisc; break;
        case ITEM_CLASS_GLYPH:          itemLevelPriceMultplier = PriceMultiplierItemLevelCategoryGlyph; break;
        default:                        break;
    }

    // Multiply the price based on multipliers
    outBuyoutPrice *= qualityPriceMultplier;
    outBuyoutPrice *= classPriceMultiplier;
    outBuyoutPrice *= classQualityPriceMultiplier;
    outBuyoutPrice *= static_cast<float>(advancedPricingMultiplier);

    // Only apply item level multiplier if set, and no advanced pricing has been enabled
    if (itemLevelPriceMultplier > 0.0f && itemProto->ItemLevel > 0 && advancedPricingMultiplier == 1.0f)
        outBuyoutPrice *= itemProto->ItemLevel * itemLevelPriceMultplier;

    // Avoid price overflows
    if (outBuyoutPrice > MaxBuyoutPriceInCopper)
        outBuyoutPrice = MaxBuyoutPriceInCopper;

    // If variance brought price below sell price, bring it back up to avoid making money off vendoring AH items
    if (BuyoutBelowVendorVariationAddPercentEnabled == true && outBuyoutPrice < itemProto->SellPrice)
    {
        float minLowPriceAddVariancePercent = 1.0f + BuyoutBelowVendorVariationAddPercent;
        outBuyoutPrice = urand(itemProto->SellPrice, minLowPriceAddVariancePercent * itemProto->SellPrice);
    }

    // Calculate a bid price based on a variance against buyout price
    float sellVarianceBidPriceTopPercent = 1.0f - BidVariationHighReducePercent;
    float sellVarianceBidPriceBottomPercent = 1.0f - BidVariationLowReducePercent;
    outBidPrice = urand(sellVarianceBidPriceBottomPercent * outBuyoutPrice, sellVarianceBidPriceTopPercent * outBuyoutPrice);

    // Catch any zeros
    if (outBuyoutPrice == 0)
        outBuyoutPrice = 1;
}

float AuctionHouseBot::GetAdvancedPricingMultiplier(ItemTemplate const* itemProto)
{
    /* "ADVANCED" SUBCLASS PRICE MULTIPLIER FORMULA NOTES

      1. multiplierHelper = log(1 + b * ItemLevel)
      2. 
            clothMultiplierHelper ^ p
        ---------------------------------   +   c * (clothMultiplierHelper ^ r)  -  d
          1 + a * clothMultiplierHelper

      Variables:
        b  // Influences the growth rate at low item levels
        p  // Exponent for first term (controls curve steepness)
        a  // Coefficient in denominator (dampens growth, especially at high item levels)
        c  // Scaling coefficient for second term
        r  // Exponent for second term (adds nonlinear boost)
        d  // Constant shift (adjusts baseline multiplier). This becomes apparent if you graph the equation - it shifts the entire curve down.
     
      Notes:
      - This formula produces a multiplier that grows logarithmically with ItemLevel (uses natural log, not base10)
      - The first term (before '+') heavily influences low item levels, the second term adds some fine-tuning for higher levels.
      - The subtraction of 'r' can help ensure low-level items don't get inflated excessively. Sometimes it isn't necessary
    */

    // Try to approximate real world prices for Trade Goods based on subclass and item level
    double advancedPricingMultiplier = 1.0f;
    if (itemProto->Class == ITEM_CLASS_CONSUMABLE)
    {
        switch (itemProto->SubClass)
        {
            case ITEM_SUBCLASS_POTION:
            {
                if (!AdvancedPricingConsumablePotionEnabled) 
                    break;
                double potionMultiplierHelper = std::log(1.0 + (0.08 * itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(potionMultiplierHelper,3.0)) / (1 + (4.0 * potionMultiplierHelper))) + (std::pow(potionMultiplierHelper,2.5));
                break;
            }
            case ITEM_SUBCLASS_ELIXIR:
            {
                if (!AdvancedPricingConsumableElixirEnabled) 
                    break;
                double elixirMultiplierHelper = std::log(1.0 + (1.6 * itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(elixirMultiplierHelper,3.1)) / (1 + (5.0 * elixirMultiplierHelper))) + (0.05 * std::pow(elixirMultiplierHelper,3.2)) - 1.0;
                break;
            }
            case ITEM_SUBCLASS_FLASK:
            {
                if (!AdvancedPricingConsumableFlaskEnabled)
                    break;
                // Use logarithmic scaling to compress large differences in vendorSellPrice to a range of ~22g-25g
                // advPricingMultiplier = LowTargetRange + (UpperTargetRange - LowTargetRange) * ( ln(vendorSellPrice) - ln(minVendorPrice) ) / ( ln(maxVendorPrice) - ln(minVendorPrice) ) / vendorSellPrice
                advancedPricingMultiplier = (220000 + (250000-220000) * (std::log(itemProto->SellPrice) - std::log(1250)) / (std::log(10000) - std::log(1250))) / itemProto->SellPrice;
            }
            default:
                break;
        }
    }
    else if (itemProto->Class == ITEM_CLASS_GEM && AdvancedPricingGemEnabled)
    {
        // No switch for subclass needed since Gem subclass represents gem color
        double gemMultiplierHelper = std::log(1.0 + (0.05 * itemProto->ItemLevel));
        advancedPricingMultiplier = ((std::pow(gemMultiplierHelper,1.0)) / (1 + (10.0 * gemMultiplierHelper))) + (std::pow(gemMultiplierHelper,3.0));
    }
    else if (itemProto->Class == ITEM_CLASS_TRADE_GOODS) 
    {
        switch (itemProto->SubClass)
        {
            case ITEM_SUBCLASS_CLOTH:
            {
                if (!AdvancedPricingTradeGoodClothEnabled) 
                    break;
                double clothMultiplierHelper = std::log(1.0 + (itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(clothMultiplierHelper,2.0)) / (1 + (0.8 * clothMultiplierHelper))) + (0.001 * std::pow(clothMultiplierHelper,3.5)) - 0.3;
                break;
            }
            case ITEM_SUBCLASS_HERB:
            {
                if (!AdvancedPricingTradeGoodHerbEnabled) 
                    break;
                double herbMultiplierHelper = std::log(1.0 + (5.0 * itemProto->ItemLevel));
                advancedPricingMultiplier = (std::pow(herbMultiplierHelper,3.0) / (1 + (1.8 * herbMultiplierHelper))) - 4.2; 
                break;
            }
            case ITEM_SUBCLASS_METAL_STONE:
            {
                if (!AdvancedPricingTradeGoodMetalStoneEnabled) 
                    break;
                double metalMultiplierHelper = std::log(1.0 + (75.0 * itemProto->ItemLevel));
                advancedPricingMultiplier =  ((std::pow(metalMultiplierHelper,3.0)) / (1 + (7.0 * metalMultiplierHelper))) + (0.001 * std::pow(metalMultiplierHelper,3.5)) - 5.2;
                break;
            }
            case ITEM_SUBCLASS_LEATHER:
            {
                if (!AdvancedPricingTradeGoodLeatherEnabled) 
                    break;
                double leatherMultiplierHelper = std::log(1.0 + (0.25 * itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(leatherMultiplierHelper,0.15)) / (1 + (2.0 * leatherMultiplierHelper))) + (0.4 * std::pow(leatherMultiplierHelper,3.0)) - 0.2;
                break;
            }
            case ITEM_SUBCLASS_ENCHANTING:
            {
                if (!AdvancedPricingTradeGoodEnchantingEnabled) 
                    break;
                double enchantingMultiplierHelper = std::log(1.0 + (0.25 * itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(enchantingMultiplierHelper,0.15)) / (1 + (2.0 * enchantingMultiplierHelper))) + (0.4 * std::pow(enchantingMultiplierHelper,3.0)) - 0.2; 
                break;
            }
            case ITEM_SUBCLASS_ELEMENTAL:
            {
                if (!AdvancedPricingTradeGoodElementalEnabled) 
                    break;
                advancedPricingMultiplier = 85 - (itemProto->ItemLevel / 0.97);
                break;
            }
            case ITEM_SUBCLASS_MEAT:
            {
                if (!AdvancedPricingTradeGoodMeatEnabled)
                    break;
                double meatMultiplierHelper = std::log(1.0 + (0.5 * itemProto->ItemLevel));
                advancedPricingMultiplier = ((std::pow(meatMultiplierHelper,3.2)) / (1 + (2.0 * meatMultiplierHelper))) + (0.05 * std::pow(meatMultiplierHelper,3.2)) - 0.1; 
                break;
            }
            default:
                break;
        }
    }
    else if (itemProto->Class == ITEM_CLASS_MISCELLANEOUS)
    {
        switch (itemProto->SubClass)
        {
            // Tuned for pricing lockboxes
            case ITEM_SUBCLASS_JUNK:
            {
                if (!AdvancedPricingMiscJunkEnabled) 
                    break;
                double miscMultiplierHelper = std::log(1.0 + (0.12 * itemProto->ItemLevel));
                advancedPricingMultiplier = (std::pow(miscMultiplierHelper,3.2) / (1 + miscMultiplierHelper)); 
                break;
            }
            case ITEM_SUBCLASS_JUNK_MOUNT:
            {
                if (!AdvancedPricingMiscMountEnabled) 
                    break;
                switch (itemProto->Quality)
                {
                    case ITEM_QUALITY_POOR:         advancedPricingMultiplier = PriceMultiplierCategoryMountQualityPoor;      break;
                    case ITEM_QUALITY_NORMAL:       advancedPricingMultiplier = PriceMultiplierCategoryMountQualityNormal;    break;
                    case ITEM_QUALITY_UNCOMMON:     advancedPricingMultiplier = PriceMultiplierCategoryMountQualityUncommon;  break;
                    case ITEM_QUALITY_RARE:         advancedPricingMultiplier = PriceMultiplierCategoryMountQualityRare;      break;
                    case ITEM_QUALITY_EPIC:         advancedPricingMultiplier = PriceMultiplierCategoryMountQualityEpic;      break;
                    case ITEM_QUALITY_LEGENDARY:    advancedPricingMultiplier = PriceMultiplierCategoryMountQualityLegendary; break;
                    case ITEM_QUALITY_ARTIFACT:     advancedPricingMultiplier = PriceMultiplierCategoryMountQualityArtifact;  break;
                    case ITEM_QUALITY_HEIRLOOM:     advancedPricingMultiplier = PriceMultiplierCategoryMountQualityHeirloom;  break;
                    default: break;
                }
                break;
            }
            case ITEM_SUBCLASS_JUNK_PET:
            {
                if (!AdvancedPricingMiscPetEnabled)
                    break;
                switch (itemProto->Quality)
                {
                    case ITEM_QUALITY_POOR:         advancedPricingMultiplier = PriceMultiplierCategoryPetQualityPoor;      break;
                    case ITEM_QUALITY_NORMAL:       advancedPricingMultiplier = PriceMultiplierCategoryPetQualityNormal;    break;
                    case ITEM_QUALITY_UNCOMMON:     advancedPricingMultiplier = PriceMultiplierCategoryPetQualityUncommon;  break;
                    case ITEM_QUALITY_RARE:         advancedPricingMultiplier = PriceMultiplierCategoryPetQualityRare;      break;
                    case ITEM_QUALITY_EPIC:         advancedPricingMultiplier = PriceMultiplierCategoryPetQualityEpic;      break;
                    case ITEM_QUALITY_LEGENDARY:    advancedPricingMultiplier = PriceMultiplierCategoryPetQualityLegendary; break;
                    case ITEM_QUALITY_ARTIFACT:     advancedPricingMultiplier = PriceMultiplierCategoryPetQualityArtifact;  break;
                    case ITEM_QUALITY_HEIRLOOM:     advancedPricingMultiplier = PriceMultiplierCategoryPetQualityHeirloom;  break;
                    default: break;
                }
                break;
            }
            default: break;
        }
    }

    return static_cast<float>(advancedPricingMultiplier);
}

ItemTemplate const* AuctionHouseBot::GetProducedItemFromRecipe(ItemTemplate const* recipeItemTemplate)
{
    if (!recipeItemTemplate)
        return nullptr;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (recipeItemTemplate->Spells[i].SpellId)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(recipeItemTemplate->Spells[i].SpellId);
            if (!spellInfo)
                continue;

            for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
            {
                if (spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_CREATE_ITEM)
                {
                    uint32 createdItemId = spellInfo->Effects[effIndex].ItemType;
                    if (createdItemId)
                    {
                        ItemTemplate const* producedItem = sObjectMgr->GetItemTemplate(createdItemId);
                        if (producedItem)
                            return producedItem;
                    }
                }
            }
        }
    }

    return nullptr;
}

static const std::unordered_set<uint32> professionSkills = {
    164,  // Blacksmithing
    165,  // Leatherworking
    171,  // Alchemy
    182,  // Herbalism
    185,  // Cooking
    186,  // Mining
    197,  // Tailoring
    202,  // Engineering
    333,  // Enchanting
    356,  // Fishing
    393,  // Skinning
    755,  // Jewelcrafting
    773,  // Inscription
    129   // First Aid
};

std::unordered_set<uint32> AuctionHouseBot::GetItemIDsProducedByRecipes()
{
    std::unordered_set<uint32> recipeItemIDs;
    for (uint32 spellId = 1; spellId < sSpellStore.GetNumRows(); ++spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        // Check if the spell is tied to a profession skill
        bool isProfessionSpell = false;
        SkillLineAbilityMapBounds skillBounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
        for (SkillLineAbilityMap::const_iterator skillItr = skillBounds.first; skillItr != skillBounds.second; ++skillItr)
        {
            if (professionSkills.find(skillItr->second->skillId) != professionSkills.end())
            {
                isProfessionSpell = true;
                break;
            }
        }

        if (isProfessionSpell == false)
            continue;

        // SPELL_EFFECT_CREATE_ITEM (effect ID 24) identify created items
        for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        {
            if (spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_CREATE_ITEM && spellInfo->Effects[effIndex].ItemType > 0)
            {
                uint32 itemID = spellInfo->Effects[effIndex].ItemType;
                recipeItemIDs.insert(itemID);
            }
        }
    }
    return recipeItemIDs;
}

bool AuctionHouseBot::IsItemADisabledRecipeProducedClassSubclass(ItemTemplate const* itemTemplate)
{
    if (DisabledRecipeProducedItemClassSubClasses.find(itemTemplate->Class) == DisabledRecipeProducedItemClassSubClasses.end())
        return false;
    else if (DisabledRecipeProducedItemClassSubClasses[itemTemplate->Class].find(itemTemplate->SubClass) == DisabledRecipeProducedItemClassSubClasses[itemTemplate->Class].end())
        return false;
    else if (ItemIDsProducedByRecipes.find(itemTemplate->ItemId) == ItemIDsProducedByRecipes.end())
        return false;
    return true;
}

void AuctionHouseBot::PopulateItemCandidatesAndProportions()
{
    // Clear old list and rebuild it
    ItemCandidatesByItemClassAndQuality.clear();

    // Item include exceptions
    set<uint32> includeItemIDExecptions;
    includeItemIDExecptions.insert(11732);
    includeItemIDExecptions.insert(11733);
    includeItemIDExecptions.insert(11734);
    includeItemIDExecptions.insert(11736);
    includeItemIDExecptions.insert(11737);
    includeItemIDExecptions.insert(18332);
    includeItemIDExecptions.insert(18333);
    includeItemIDExecptions.insert(18334);
    includeItemIDExecptions.insert(18335);

    ItemIDsProducedByRecipes.clear();
    ItemIDsProducedByRecipes = GetItemIDsProducedByRecipes();

    // Fill candidate item templates
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        // Never store curBlock zero
        if (itr->second.ItemId == 0)
            continue;

        // If there is an iLevel exception, honor it
        if (ListedItemLevelRestrictedEnabled == true)
        {
            // Only test if it's not an exception
            if (ListedItemLevelExceptionItems.find(itr->second.ItemId) == ListedItemLevelExceptionItems.end())
            {
                uint32 itemLevelToCompare = itr->second.ItemLevel;

                // Recipes might need to consider produced items
                if (ListedItemLevelRestrictedUseCraftedItemForCalculation == true && itr->second.Class == ITEM_CLASS_RECIPE)
                {
                    ItemTemplate const* producedItemTemplate = GetProducedItemFromRecipe(&itr->second);
                    if (producedItemTemplate != nullptr)
                    {
                        if (debug_Out_Filters)
                            AHBotLogError("AuctionHouseBot: Using item {} for recipe {} for item level comparison since ListedItemLevelRestrictedUseCraftedItemForCalculation is true", producedItemTemplate->ItemId, itr->second.ItemId);
                        itemLevelToCompare = producedItemTemplate->ItemLevel;
                    }
                }

                if (itemLevelToCompare < ListedItemLevelMin)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item level is lower than ListedItemLevelRestrict.MinItemLevel", itr->second.ItemId);
                    continue;
                }
                if (itemLevelToCompare > ListedItemLevelMax)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item level is higher than ListedItemLevelRestrict.MaxItemLevel", itr->second.ItemId);
                    continue;
                }
            }
        }

        // If there is an item ID exception, honor it
        if (ListedItemIDRestrictedEnabled == true)
        {
            // Only test if it's not an exception
            if (ListedItemIDExceptionItems.find(itr->second.ItemId) == ListedItemIDExceptionItems.end())
            {
                if (itr->second.ItemId < ListedItemIDMin)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item id is lower than ListedItemLevelRestrict.MinItemID", itr->second.ItemId);
                    continue;
                }
                if (itr->second.ItemId > ListedItemIDMax)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item id is higher than ListedItemLevelRestrict.MaxItemID", itr->second.ItemId);
                    continue;
                }
            }
        }

        // If there is a use/equip level exception, honor it
        if (ListedItemUseOrEquipRestrictedEnabled == true)
        {
            // Only test if it's not an exception
            if (ListedItemUseOrEquipExceptionItems.find(itr->second.ItemId) == ListedItemUseOrEquipExceptionItems.end())
            {
                uint32 useOrEquipLevelCompare = itr->second.RequiredLevel;

                if (useOrEquipLevelCompare > 0 && useOrEquipLevelCompare < ListedItemUseOrEquipRestrictMinLevel)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item use or equip level is lower than EquipItemUseOrEquipLevelRestrict.MinItemLevel", itr->second.ItemId);
                    continue;
                }
                if (useOrEquipLevelCompare > 0 && useOrEquipLevelCompare > ListedItemUseOrEquipRestrictMaxLevel)
                {
                    if (debug_Out_Filters)
                        AHBotLogError("AuctionHouseBot: Item {} disabled since item use or equip level is higher than EquipItemUseOrEquipLevelRestrict.MaxItemLevel", itr->second.ItemId);
                    continue;
                }
            }
        }

        // Disabled items by Id
        if (DisabledItems.find(itr->second.ItemId) != DisabledItems.end())
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Configured by DisabledItemIDs and DisabledCraftedItemIDs)", itr->second.ItemId);
            continue;
        }

        // Disabled recipe item check
        if (DisabledRecipeProducedItemFilterEnabled == true)
        {
            if (IsItemADisabledRecipeProducedClassSubclass(&itr->second) == true)
            {
                if (debug_Out_Filters)
                    AHBotLogError("AuctionHouseBot: Item {} disabled (Configured by DisabledRecipeProducedItemFilterEnabled and DisabledRecipeProducedItemClassSubClasses)", itr->second.ItemId);
                continue;
            }
        }

        // These items should be included and would otherwise be skipped due to conditions below
        if (includeItemIDExecptions.find(itr->second.ItemId) != includeItemIDExecptions.end())
        {
            ItemCandidatesByItemClassAndQuality[itr->second.Class][itr->second.Quality].push_back(itr->second.ItemId);
            continue;
        }

        // Skip any BOP items
        if (itr->second.Bonding == BIND_WHEN_PICKED_UP || itr->second.Bonding == BIND_QUEST_ITEM)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (BOP or BQI)", itr->second.ItemId);
            continue;
        }

        // Restrict quality to anything under 7 (artifact and below) or above poor
        if (itr->second.Quality == 0 || itr->second.Quality > 6)
            continue;

        // Disable conjured items
        if (itr->second.IsConjuredConsumable())
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Conjured Consumable)", itr->second.ItemId);
            continue;
        }

        // Disable money
        if (itr->second.Class == ITEM_CLASS_MONEY)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Money)", itr->second.ItemId);
            continue;
        }

        // Disable moneyloot
        if (itr->second.MinMoneyLoot > 0)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (MoneyLoot)", itr->second.ItemId);
            continue;
        }

        // Disable items with duration
        if (itr->second.Duration > 0)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Has a Duration)", itr->second.ItemId);
            continue;
        }

        // Disable containers with zero slots
        if (itr->second.Class == ITEM_CLASS_CONTAINER && itr->second.ContainerSlots == 0)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Container with no slots)", itr->second.ItemId);
            continue;
        }

        // Disable normal class 'book' recipes, since they are junk
        if (itr->second.Class == ITEM_CLASS_RECIPE && itr->second.SubClass == ITEM_SUBCLASS_BOOK && itr->second.Quality <= ITEM_QUALITY_NORMAL)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled (Normal or lower recipe book)", itr->second.ItemId);
            continue;
        }

        // Disable anything with the string literal of a testing or deprecated item
        if (DisabledItemTextFilter == true && 
            (itr->second.Name1.find("Test ") != std::string::npos ||
            itr->second.Name1.find("TEST") != std::string::npos ||
            itr->second.Name1.find("Deprecated") != std::string::npos ||
            itr->second.Name1.find("Depricated") != std::string::npos ||
            itr->second.Name1.find(" Epic ") != std::string::npos ||
            itr->second.Name1.find("]") != std::string::npos ||            
            itr->second.Name1.find("D'Sak") != std::string::npos ||
            itr->second.Name1.find("(") != std::string::npos ||
            itr->second.Name1.find("OLD") != std::string::npos ||
            itr->second.Name1.find("PVP") != std::string::npos))
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled item with a temp or unused item name", itr->second.ItemId);
            continue;
        }

        // Disabled crafted gems that start with "Perfect"
        if (itr->second.Class == ITEM_CLASS_GEM && itr->second.Name1.find("Perfect ") != std::string::npos)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled as it's a perfect crafted gem", itr->second.ItemId);
            continue;
        }

        // Disable all items that have neither a sell or a buy price, with exception of item enhancements and trade goods
        bool isEnchantingTradeGood = (itr->second.Class == ITEM_CLASS_TRADE_GOODS && itr->second.SubClass == ITEM_SUBCLASS_ENCHANTING);
        bool isItemEnhancement = (itr->second.Class == ITEM_CLASS_CONSUMABLE && itr->second.SubClass == ITEM_SUBCLASS_ITEM_ENHANCEMENT);
        bool hasNoPrice = (itr->second.SellPrice == 0 && itr->second.BuyPrice == 0);
        if (hasNoPrice == true && isItemEnhancement == false && isEnchantingTradeGood == false)
        {
            if (debug_Out_Filters)
                AHBotLogError("AuctionHouseBot: Item {} disabled misc item", itr->second.ItemId);
            continue;
        }

        // Store the item ID
        ItemCandidatesByItemClassAndQuality[itr->second.Class][itr->second.Quality].push_back(itr->second.ItemId);
    }

    // Show any debugging information
    if (debug_Out)
    {
        AHBotLogInfo("AHBot Candidate item counts by item category (class) and quality after appyling filters:");
        for (auto& itemCandidateQualityGroupInClass : ItemCandidatesByItemClassAndQuality)
            for (auto& itemCandidateInQualityGroup : itemCandidateQualityGroupInClass.second)
            {
                uint32 classID = itemCandidateQualityGroupInClass.first;
                uint32 qualityID = itemCandidateInQualityGroup.first;
                size_t elementCount = itemCandidateInQualityGroup.second.size();
                AHBotLogInfo("Item count in class {} quality {} is {}", classID, qualityID, elementCount);
            }
    }

    // Fill list proportions
    ItemListProportionNodesLookup.clear();
    for (uint32 i = 0; i < ItemListProportionNodesSeed.size(); ++i)
    {
        ListProportionNode curNode = ItemListProportionNodesSeed[i];
        if (ItemCandidatesByItemClassAndQuality[curNode.ItemClassID][curNode.ItemQualityID].size() > 0)
        {
            for (uint32 j = 0; j < curNode.Proportion; j++)
                ItemListProportionNodesLookup.push_back(curNode);
        }
    }
}

uint32 AuctionHouseBot::GetRandomItemIDForListing()
{
    // Start with a listing proportion
    if (ItemListProportionNodesLookup.size() == 0)
    {
        AHBotLogError("No valid list proportion for new listing could be found (ItemListProportionNodesLookup was empty)");
        SellingBotEnabled = false;
        return 0;
    }
    ListProportionNode listProportionNode = ItemListProportionNodesLookup[urand(0, ItemListProportionNodesLookup.size() - 1)];

    // Grab an item
    size_t numOfValidItemsInGroup = ItemCandidatesByItemClassAndQuality[listProportionNode.ItemClassID][listProportionNode.ItemQualityID].size();
    if (numOfValidItemsInGroup == 0)
    {
        AHBotLogError("Unable to find a candidate item with Category (class) {} and Quality {}", listProportionNode.ItemClassID, listProportionNode.ItemQualityID);
        return 0;
    }
    return ItemCandidatesByItemClassAndQuality[listProportionNode.ItemClassID][listProportionNode.ItemQualityID][urand(0, numOfValidItemsInGroup-1)];
}

void AuctionHouseBot::AddNewAuctions(std::vector<Player*> AHBPlayers, FactionSpecificAuctionHouseConfig *config)
{
    if (!SellingBotEnabled)
    {
        if (debug_Out)
            AHBotLogInfo("AHSeller: Disabled");
        return;
    }

    uint32 minItems = config->GetMinItems();
    uint32 maxItems = config->GetMaxItems();

    if (maxItems == 0)
        return;

    if (!config->GetAuctioneerGuid())
    {
        AHBotLogError("AuctionHouseBot: No auctioneer spawn GUID resolved for house {}, skipping listings", config->GetAHID());
        return;
    }

    AuctionHouseEntry const* ahEntry = sAuctionMgr->GetAuctionHouseEntry(config->GetAHFID());
    if (!ahEntry)
    {
        return;
    }
    AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());
    if (!auctionHouse)
    {
        return;
    }

    uint32 currentAuctionItemListCount = auctionHouse->Getcount();
    if (currentAuctionItemListCount >= minItems)
    {
        if (debug_Out)
            AHBotLogInfo("AHSeller: Auctions above minimum, so no auctions will be listed this cycle");
        return;
    }

    if (currentAuctionItemListCount >= maxItems)
    {
        if (debug_Out)
            AHBotLogInfo("AHSeller: Auctions at or above maximum, so no auctions will be listed this cycle");
        return;
    }

    uint32 newItemsToListCount = 0;
    if ((maxItems - currentAuctionItemListCount) >= ItemsPerCycle)
        newItemsToListCount = ItemsPerCycle;
    else
        newItemsToListCount = (maxItems - currentAuctionItemListCount);

    if (debug_Out)
        AHBotLogInfo("AHSeller: Adding {} Auctions", newItemsToListCount);

    if (debug_Out)
        AHBotLogInfo("AHSeller: Current house id is {}", config->GetAHID());

    // only insert a few at a time, so as not to peg the processor
    uint32 itemsGenerated = 0;
    for (uint32 cnt = 1; cnt <= newItemsToListCount; cnt++)
    {
        auto trans = CharacterDatabase.BeginTransaction();
        ItemTemplate const* prototype = nullptr;
        uint32 batchCount = 0;

        while (batchCount < 500 && itemsGenerated < newItemsToListCount)
        {
            // GetRandomItemIDForListing can disable the seller mid-cycle, and every failed attempt must count
            // against the batch limit or a misconfigured item list spins this loop forever on the world thread
            if (!SellingBotEnabled)
                break;

            // Either generate a new item ID to list, or grab from the remaining list
            uint32 itemID;
            if (ActiveListMultipleItemID != 0)
            {
                itemID = ActiveListMultipleItemID;

                prototype = sObjectMgr->GetItemTemplate(itemID);
                if (!prototype)
                {
                    if (debug_Out)
                        AHBotLogError("AHSeller: prototype == NULL");
                    ActiveListMultipleItemID = 0;
                    batchCount++;
                    continue;
                }

                RemainingListMultipleCount--;
                if (RemainingListMultipleCount <= 0)
                    ActiveListMultipleItemID = 0;
            }
            else
            {
                itemID = GetRandomItemIDForListing();
                if (itemID == 0)
                {
                    if (debug_Out)
                        AHBotLogError("AHSeller: Item::CreateItem() failed as the ItemID is 0");
                    batchCount++;
                    continue;
                }

                prototype = sObjectMgr->GetItemTemplate(itemID);
                if (!prototype)
                {
                    if (debug_Out)
                        AHBotLogError("AHSeller: prototype == NULL");
                    batchCount++;
                    continue;
                }

                if (IsItemEligibleForDBDropRates(prototype))
                {
                    bool foundDBDropRatesItem = HandleAdvancedListingRuleUseDropRates(prototype);
                    if (foundDBDropRatesItem)
                        itemID = prototype->ItemId;
                    else
                    {
                        batchCount++;
                        continue;
                    }
                }

                if (ItemListProportionMultipliedItemIDs.find(itemID) != ItemListProportionMultipliedItemIDs.end() &&
                    ItemListProportionMultipliedItemIDs[itemID] > 1)
                {
                    ActiveListMultipleItemID = itemID;
                    RemainingListMultipleCount = ItemListProportionMultipliedItemIDs[itemID] - 1;
                    if (debug_Out)
                        AHBotLogInfo("AHSeller: Is listing item ID {} which is configured for {} multiples from ListMultipliedItemIDs", itemID, ItemListProportionMultipliedItemIDs[itemID]);
                }
            }

            Player* AHBplayer = AHBPlayers[urand(0, AHBPlayers.size() - 1)];

            Item* item = Item::CreateItem(itemID, 1, AHBplayer);
            if (item == NULL)
            {
                if (debug_Out)
                    AHBotLogError("AHSeller: Item::CreateItem() returned NULL");
                break;
            }
            item->AddToUpdateQueueOf(AHBplayer);

            int32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemID);
            if (randomPropertyId != 0)
                item->SetItemRandomProperties(randomPropertyId);

            // Determine price
            uint64 buyoutPrice = 0;
            uint64 bidPrice = 0;
            CalculateItemValue(prototype, bidPrice, buyoutPrice);

            // Define a duration
            uint32 etime = urand(ListingExpireTimeInSecondsMin, ListingExpireTimeInSecondsMax);

            // Set stack size
            uint32 stackCount = GetStackSizeForItem(prototype);
            item->SetCount(stackCount);

            uint32 dep =  sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, stackCount);

            AuctionEntry* auctionEntry = new AuctionEntry();
            auctionEntry->Id = sObjectMgr->GenerateAuctionID();
            auctionEntry->auctioneer = config->GetAuctioneerGuid();
            auctionEntry->itemGUIDLow = item->GetGUIDLow();
            auctionEntry->itemEntry = item->GetEntry();
            auctionEntry->itemCount = item->GetCount();
            auctionEntry->owner = AHBplayer->GetGUIDLow();
            auctionEntry->startbid = bidPrice * stackCount;
            auctionEntry->buyout = buyoutPrice * stackCount;
            auctionEntry->bid = 0;
            auctionEntry->bidder = 0;
            auctionEntry->deposit = dep;
            auctionEntry->expire_time = (time_t) etime + time(NULL);
            auctionEntry->auctionHouseEntry = ahEntry;
            auctionEntry->factionTemplateId = config->GetAHFID();
            item->SaveToDB(trans);
            item->RemoveFromUpdateQueueOf(AHBplayer);
            sAuctionMgr->AddAItem(item);
            auctionHouse->AddAuction(auctionEntry);
            auctionEntry->SaveToDB(trans);
            itemsGenerated++;
            batchCount++;
        }

        CharacterDatabase.CommitTransaction(trans);
    }
    if (debug_Out)
        AHBotLogInfo("AHSeller: Added {} items", itemsGenerated);
}

bool AuctionHouseBot::HandleAdvancedListingRuleUseDropRates(ItemTemplate const*& proto)
{
    // The AHBot has chosen a rare/epic armor/weapon/recipe, so select another item
    //   of that type based on drop rates. This way ListProportions are respected.

    // Custom items missing from Item.dbc skip the core's class/quality validation, so bounds check before indexing
    if (proto->Class >= ItemTiersByClassAndQuality.size() || proto->Quality >= ItemTiersByClassAndQuality[proto->Class].size())
        return false;

    // Roll for rarity tier
    double r = 100.0 * (urand(0, INT32_MAX) / static_cast<double>(INT32_MAX));
    int tier = GetItemDropChanceTier(r);

    // If chosen tier is empty, search more common tiers until not empty
    auto& tierBuckets = ItemTiersByClassAndQuality[proto->Class][proto->Quality];
    while (tierBuckets[tier].empty() && tier > 0) {
        tier--;
    }

    // Pull a random item from selected rarity tier
    uint32 itemID = proto->ItemId;
    auto& bucket = tierBuckets[tier];
    if (!bucket.empty())
    {
        itemID = bucket[rand() % bucket.size()];
        proto = sObjectMgr->GetItemTemplate(itemID);
        if (!proto)
        {
            if (debug_Out)
                AHBotLogError("AHSeller: prototype == NULL");
            return false;
        }
    }
    else return false;

    return true;
}

void AuctionHouseBot::PopulateItemDropChances()
{
    InitializeAdvancedListingRuleUseDropRatesTiers();

    if (!AdvancedListingRuleUseDropRatesWeaponEnabled &&
        !AdvancedListingRuleUseDropRatesArmorEnabled &&
        !AdvancedListingRuleUseDropRatesRecipeEnabled)
    {
        AHBotLogError("AuctionHouseBot: No categories are enabled for AuctionHouseBot.Seller.AdvancedListingRules.UseDropRates");
        return;
    }

    auto handleDropChancesForCategoryAndQuality = [this](ItemClass category, std::set<uint32> affectedQualities) {
        std::string qualities = "";
        int i = 0;
        for (uint32 q : affectedQualities)
        {
            if (i > 0)
                qualities += ",";
            qualities += to_string(q);
            i++;
        }
        PopulateItemDropChancesForCategoryAndQuality(category, qualities);
    };

    if (AdvancedListingRuleUseDropRatesWeaponEnabled)
        handleDropChancesForCategoryAndQuality(ITEM_CLASS_WEAPON, AdvancedListingRuleUseDropRatesWeaponAffectedQualities);
    if (AdvancedListingRuleUseDropRatesArmorEnabled)
        handleDropChancesForCategoryAndQuality(ITEM_CLASS_ARMOR, AdvancedListingRuleUseDropRatesArmorAffectedQualities);
    if (AdvancedListingRuleUseDropRatesRecipeEnabled)
        handleDropChancesForCategoryAndQuality(ITEM_CLASS_RECIPE, AdvancedListingRuleUseDropRatesRecipeAffectedQualities);

    // Erase item candidates that are not crafted, not quest rewards, and missing DB drop rate
    for (auto& [classID, qualityGroups] : ItemCandidatesByItemClassAndQuality)
    {
        for (auto& [qualityID, candidates] : qualityGroups)
        {
            candidates.erase(
                std::remove_if(
                    candidates.begin(),
                    candidates.end(),
                    [&](uint32 id)
                    {
                        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(id);
                        if (!IsItemCategoryQualityInDBDropRatesConfig(proto))
                            return false;

                        bool shouldErase = !IsItemCrafted(id)
                                        && !IsItemQuestReward(id)
                                        && !CachedItemDropRates.contains(id);

                        return shouldErase;
                    }),
                candidates.end()
            );
        }
    }

    if (debug_Out)
    {
        // Show number of items in each tier
        AHBotLogInfo("AHBot AdvancedListingRule UseDropRates item counts by class and quality after appyling filters:");
        for (size_t i = 0; i < ItemTiersByClassAndQuality.size(); ++i)
        {
            const auto& qualities = ItemTiersByClassAndQuality[i];
            for (size_t j = 0; j < qualities.size(); ++j)
            {
                const auto& tiers = qualities[j];
                for (size_t k = 0; k < tiers.size(); ++k)
                {
                    const auto& items = tiers[k];
                    if (i == ITEM_CLASS_WEAPON)
                        AHBotLogInfo("Weapon Count: {} Tier {} has {} items", GetQualityName((ItemQualities)j), k, items.size());
                    if (i == ITEM_CLASS_ARMOR)
                        AHBotLogInfo("Armor Count: {} Tier {} has {} items", GetQualityName((ItemQualities)j), k, items.size());
                    if (i == ITEM_CLASS_RECIPE)
                        AHBotLogInfo("Recipe Count: {} Tier {} has {} items", GetQualityName((ItemQualities)j), k, items.size());
                }
            }
        }
    }
}

void AuctionHouseBot::PopulateItemDropChancesForCategoryAndQuality(ItemClass category, std::string qualities)
{
    if (qualities.empty())
    {
        AHBotLogError("AuctionHouseBot: PopulateItemDropChancesForCategoryAndQuality() qualities are not set. "
                            "Verify that mod_ahbot.conf has values for AdvancedListingRules.UseDropRates.<Category>.AffectedQualities. "
                            "Defaulting to '2,3,4,5' to prevent crash.");
        qualities = "2,3,4,5";
    }

    char directSql[1024];
    char referenceSql[1024];
    char danglingSql[1024];
    char groupSql[1024];
    char objectsSql[2048];
    snprintf(directSql, sizeof(directSql),
        "SELECT it.entry AS itemID, MIN(ABS(clt.ChanceOrQuestChance)) AS direct_chance, 0 AS reference_chance "
        "FROM creature_loot_template clt JOIN item_template it ON it.entry = clt.item "
        "WHERE clt.mincountOrRef >= 0 AND clt.groupid = 0 AND it.class IN (%u) AND it.quality IN (%s) GROUP BY it.entry",
        uint32(category), qualities.c_str());
    snprintf(referenceSql, sizeof(referenceSql),
        "SELECT it.entry AS itemID, 0 AS direct_chance, MIN(ABS(rlt.ChanceOrQuestChance)) AS reference_chance "
        "FROM creature_loot_template clt JOIN reference_loot_template rlt ON rlt.entry = ABS(clt.mincountOrRef) "
        "JOIN item_template it ON it.entry = rlt.item "
        "WHERE clt.mincountOrRef < 0 AND it.class IN (%u) AND it.quality IN (%s) GROUP BY it.entry",
        uint32(category), qualities.c_str());
    snprintf(danglingSql, sizeof(danglingSql),
        "SELECT it.entry AS itemID, 0 AS direct_chance, MIN(ABS(rlt.ChanceOrQuestChance)) AS reference_chance "
        "FROM reference_loot_template rlt JOIN item_template it ON it.entry = rlt.item "
        "WHERE rlt.entry NOT IN (SELECT ABS(mincountOrRef) FROM creature_loot_template WHERE mincountOrRef < 0) "
        "AND it.class IN (%u) AND it.quality IN (%s) GROUP BY it.entry",
        uint32(category), qualities.c_str());
    snprintf(groupSql, sizeof(groupSql),
        "SELECT it.entry AS itemID, 0 AS direct_chance, MIN(ABS(clt.ChanceOrQuestChance)) AS reference_chance "
        "FROM creature_loot_template clt JOIN item_template it ON it.entry = clt.item "
        "WHERE clt.groupid != 0 AND clt.mincountOrRef >= 0 AND it.class IN (%u) AND it.quality IN (%s) GROUP BY it.entry",
        uint32(category), qualities.c_str());
    snprintf(objectsSql, sizeof(objectsSql),
        "SELECT it.entry AS itemID, ABS(ilt.ChanceOrQuestChance) AS direct_chance, 0 AS reference_chance "
        "FROM item_loot_template ilt JOIN item_template it ON it.entry = ilt.item "
        "WHERE it.class IN (%u) AND it.quality IN (%s) AND ilt.ChanceOrQuestChance != 0 "
        "UNION ALL SELECT it.entry, ABS(golt.ChanceOrQuestChance), 0 FROM gameobject_loot_template golt "
        "JOIN item_template it ON it.entry = golt.item WHERE it.class IN (%u) AND it.quality IN (%s) AND golt.ChanceOrQuestChance != 0",
        uint32(category), qualities.c_str(), uint32(category), qualities.c_str());

    QueryResult directResult = WorldDatabase.Query(directSql);
    QueryResult referenceResult = WorldDatabase.Query(referenceSql);
    QueryResult danglingReferenceResult = WorldDatabase.Query(danglingSql);
    QueryResult groupResult = WorldDatabase.Query(groupSql);
    QueryResult objectsDropResult = WorldDatabase.Query(objectsSql);
    if (!directResult && !referenceResult && !danglingReferenceResult && !groupResult && !objectsDropResult)
    {
        AHBotLogError("AuctionHouseBot: PopulateItemDropChances() failed to query items' drop rates.");
        return;
    }

    // Add drop rate of all results to CachedItemDropRates 
    auto parseResults = [this](QueryResult result, bool overwriteDropRate) 
    {
        if (!result)
            return;
        do {
            Field* fields = result->Fetch();
            double directDropChance = 0.0;
            double referenceDropChance = 0.0;
            uint32 itemID = fields[0].GetUInt32();

            // Ignore quest rewards and crafted items, they have "100%" drop rate
            if (IsItemQuestReward(itemID) || IsItemCrafted(itemID))
                continue;

            if (CachedItemDropRates.contains(itemID) && !overwriteDropRate)
                continue;

            if (!fields[1].IsNull())
                directDropChance = fields[1].GetDouble(); 
            if (!fields[2].IsNull())
                referenceDropChance = fields[2].GetDouble();
            
            // Choose higher of two rates (one is normally 0), then raise to MinDropRate if less than
            double higherDropChance = (directDropChance > referenceDropChance) ? directDropChance : referenceDropChance;
            higherDropChance = (higherDropChance < AdvancedListingRuleUseDropRatesMinDropRate) ? AdvancedListingRuleUseDropRatesMinDropRate : higherDropChance;

            if (CachedItemDropRates[itemID] < higherDropChance) 
                CachedItemDropRates[itemID] = higherDropChance;

        } while (result->NextRow());
    };

    parseResults(directResult, true);
    parseResults(referenceResult, true);
    parseResults(danglingReferenceResult, false);
    parseResults(groupResult, true);
    parseResults(objectsDropResult, true);

    // Populate drop rates Tiers
    for (auto& [classID, qualityGroups] : ItemCandidatesByItemClassAndQuality)
    {
        for (auto& [qualityID, candidates] : qualityGroups)
        {
            for (uint32 id : candidates)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(id);
                if (!proto || !IsItemCategoryQualityInDBDropRatesConfig(proto))
                    continue;

                // Custom items missing from Item.dbc skip the core's class/quality validation, so bounds check
                if (proto->Class >= ItemTiersByClassAndQuality.size() || proto->Quality >= ItemTiersByClassAndQuality[proto->Class].size())
                    continue;

                // Skip items that haven't been populated yet.
                if (!CachedItemDropRates.contains(id))
                    continue;

                double rate = CachedItemDropRates[id];
                int tier = GetItemDropChanceTier(rate);
                ItemTiersByClassAndQuality[proto->Class][proto->Quality][tier].push_back(id);
            }
        }
    }

    // Remove duplicates
    for (auto& byClass : ItemTiersByClassAndQuality)
        for (auto& byQuality : byClass)
            for (auto& byTier : byQuality) {
                std::sort(byTier.begin(), byTier.end());
                byTier.erase(std::unique(byTier.begin(), byTier.end()), byTier.end());
            }
}

void AuctionHouseBot::InitializeAdvancedListingRuleUseDropRatesTiers()
{
    // Re-initialize to reset
    DropRatesToTierMap = std::map<double, int, std::greater<double>>();
    ItemTiersByClassAndQuality = std::vector<std::vector<std::vector<std::vector<uint32>>>>(17); // 17 Categories
    std::string tiersConfigString = AHBotConfig::GetString("AuctionHouseBot.AdvancedListingRules.UseDropRates.TiersConfig", "50,10,5,2,1,0.5,0.2,0.1,0.05,0.02,0.01,0.005");

    // Parse tiers config, populate DropRate -> Tier map for later lookups
    int curTier = 0;
    std::string delimitedValue;
    std::stringstream tiersConfigStream;
    tiersConfigStream.str(tiersConfigString);
    while (std::getline(tiersConfigStream, delimitedValue, ',')) // Process each tier in the string, delimited by the comma ","
    {
        // Trim whitespace
        delimitedValue.erase(0, delimitedValue.find_first_not_of(" \t"));
        delimitedValue.erase(delimitedValue.find_last_not_of(" \t") + 1);

        if (delimitedValue.empty())
        {
            AHBotLogError("AuctionHouseBot: Empty entry found in AuctionHouseBot.AdvancedListingRules.UseDropRates.TiersConfig: '{}'", tiersConfigString);
            continue;
        }

        double rate = std::stod(delimitedValue);
        if (rate <= 0.0)
        {
            AHBotLogError("AuctionHouseBot: Invalid (non-positive) drop rate '{}' in AuctionHouseBot.AdvancedListingRules.UseDropRates.TiersConfig: '{}'", delimitedValue, tiersConfigString);
            continue;
        }

        // Check for duplicates
        if (DropRatesToTierMap.contains(rate))
        {
            AHBotLogError("AuctionHouseBot: Duplicate drop rate '{}' found in AuctionHouseBot.AdvancedListingRules.UseDropRates.TiersConfig: '{}'", rate, tiersConfigString);
            continue;
        }

        DropRatesToTierMap[rate] = curTier;
        curTier++;
    }

    if (DropRatesToTierMap.empty())
        AHBotLogError("AuctionHouseBot: Failed to parse any valid drop rates from AuctionHouseBot.AdvancedListingRules.UseDropRates.TiersConfig: '{}'", tiersConfigString);

    // Resize ItemTiersByClassAndQuality by tier count
    for (auto& qualities : ItemTiersByClassAndQuality)
    {
        qualities.resize(7);    // 7 Qualities
        for (auto& tiers : qualities)
            tiers.resize(DropRatesToTierMap.size() + 1); // Create an extra "catch-all" bucket
    }
}

void AuctionHouseBot::PopulateQuestRewardItemIDs()
{
    QueryResult questRewardsResult = WorldDatabase.Query(
        "SELECT DISTINCT item_id FROM ("
        "SELECT RewardItemId1 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardItemId2 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardItemId3 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardItemId4 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId1 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId2 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId3 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId4 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId5 AS item_id FROM quest_template UNION ALL "
        "SELECT RewardChoiceItemId6 AS item_id FROM quest_template"
        ") AS quest_rewards WHERE item_id != 0");
    if (!questRewardsResult)
    {
        AHBotLogError("AuctionHouseBot: Quest Rewards lookup failed.");
        return;
    }

    do 
    {
        uint32 id = questRewardsResult->Fetch()->GetUInt32();
        QuestRewardItemIDs.insert(id);
    } while (questRewardsResult->NextRow());
}

int AuctionHouseBot::GetItemDropChanceTier(double dropRate)
{
    for(const auto& pair : DropRatesToTierMap)
    {
        double mapDropRate = pair.first;
        int mapTier = pair.second;
        if (dropRate >= mapDropRate) 
            return mapTier;
    }

    // If dropRate is lower than smallest configured tier, return "catch-all" bucket index
    return DropRatesToTierMap.size();
}

void AuctionHouseBot::AddNewAuctionBuyerBotBid(std::vector<Player*> AHBPlayers, FactionSpecificAuctionHouseConfig *config)
{
    if (!BuyingBotEnabled)
    {
        if (debug_Out)
            AHBotLogInfo("AHBuyer: Disabled");
        return;
    }

    QueryResult result;
    if (BuyingBotWillBidAgainstPlayers == false)
        result = CharacterDatabase.PQuery("SELECT id FROM auctionhouse WHERE itemowner NOT IN (%s) AND buyguid NOT IN (%s) AND lastbid = 0",
            AHCharactersGUIDsForQuery.c_str(), AHCharactersGUIDsForQuery.c_str());
    else
        result = CharacterDatabase.PQuery("SELECT id FROM auctionhouse WHERE itemowner NOT IN (%s) AND buyguid NOT IN (%s)",
            AHCharactersGUIDsForQuery.c_str(), AHCharactersGUIDsForQuery.c_str());

    if (!result)
        return;

    if (result->GetRowCount() == 0)
        return;

    // Fetches content of selected AH
    AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());
    vector<uint32> possibleBids;

    do
    {
        uint32 tmpdata = result->Fetch()->GetUInt32();
        possibleBids.push_back(tmpdata);
    } while (result->NextRow());

    int randBuyingBotBuyCandidatesPerBuyCycle = urand(BuyingBotBuyCandidatesPerBuyCycleMin, BuyingBotBuyCandidatesPerBuyCycleMax);
    for (int count = 1; count <= randBuyingBotBuyCandidatesPerBuyCycle; ++count)
    {
        // Do we have anything to bid? If not, stop here.
        if (possibleBids.empty())
        {
            //if (debug_Out) sLog->outError( "AHBuyer: I have no items to bid on.");
            count = randBuyingBotBuyCandidatesPerBuyCycle;
            continue;
        }

        // Choose random auction from possible currentAuctionItemListCount
        uint32 vectorPos = urand(0, possibleBids.size() - 1);
        vector<uint32>::iterator iter = possibleBids.begin();
        advance(iter, vectorPos);

        // from auctionhousehandler.cpp, creates auction pointer & player pointer
        AuctionEntry* auction = auctionHouse->GetAuction(*iter);

        // Erase the auction from the vector to prevent bidding on item in next iteration.
        possibleBids.erase(iter);

        if (!auction)
            continue;

        // get exact item information
		Item *pItem = sAuctionMgr->GetAItem(auction->itemGUIDLow);
        if (!pItem || pItem->GetCount() == 0)
        {
			if (debug_Out)
                AHBotLogError("AHBuyer: Item {} doesn't exist, perhaps bought already?", std::to_string(auction->itemGUIDLow));
            continue;
        }

        // get item prototype
        ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(auction->itemEntry);
        if (!prototype)
        {
            if (debug_Out)
                AHBotLogError("AHBuyer: Item template {} for auction {} does not exist, skipping", auction->itemEntry, auction->Id);
            continue;
        }

        // Calculate a potential price for the item
        uint64 willingToSpendPerItemPrice = 0;
        uint64 discardBidPrice = 0;
        CalculateItemValue(prototype, discardBidPrice, willingToSpendPerItemPrice);
        willingToSpendPerItemPrice = (uint64)((float)willingToSpendPerItemPrice * BuyingBotAcceptablePriceModifier);
        uint64 willingToPayForStackPrice = willingToSpendPerItemPrice * pItem->GetCount();

        // Determine if it's a bid, buyout, or skip
        bool doBuyout = false;
        bool doBid = false;
        uint64 calcBidAmount = 0;

        if (auction->buyout != 0 && auction->buyout < willingToPayForStackPrice)
            doBuyout = true;
        else
        {
            if (auction->bid == 0 && auction->startbid <= willingToPayForStackPrice)
            {
                doBid = true;
                if (BuyingBotAlwaysBidMaxCalculatedPrice == true)
                    calcBidAmount = willingToPayForStackPrice;
                else
                    calcBidAmount = auction->startbid;
            }
            else if (auction->bid != 0 && (auction->bid + auction->GetAuctionOutBid()) < willingToPayForStackPrice)
            {
                doBid = true;
                if (BuyingBotAlwaysBidMaxCalculatedPrice == true)
                    calcBidAmount = willingToPayForStackPrice;
                else
                    calcBidAmount = auction->bid + auction->GetAuctionOutBid();
            }
        }

        // Check that the item isn't listed above Vendor sell price. Out-of-range entries (items added after the
        // price table was built) get UINT32_MAX, which behaves the same as "not sold by a vendor"
        bool preventedOverpayingForVendorItem = false;
        uint32 vendorSellPrice = prototype->ItemId < vendorItemsPrices.size() ? vendorItemsPrices[prototype->ItemId] : UINT32_MAX;
        if (PreventOverpayingForVendorItems && vendorSellPrice > 0)
        {
            if (doBuyout && auction->buyout > vendorSellPrice)
            {
                doBuyout = false;
                preventedOverpayingForVendorItem = true;
            }
            if (doBid && calcBidAmount > vendorSellPrice)
            {
                doBid = false;
                preventedOverpayingForVendorItem = true;
            }
        }

        if (debug_Out)
        {
            AHBotLogInfo("-------------------------------------------------");
            AHBotLogInfo("AHBuyer: Info for Auction #{}:", auction->Id);
            AHBotLogInfo("AHBuyer: AuctionHouse: {}", auction->GetHouseId());
            AHBotLogInfo("AHBuyer: Owner: {}", auction->owner);
            AHBotLogInfo("AHBuyer: Bidder: {}", auction->bidder);
            AHBotLogInfo("AHBuyer: Expire Time: {}", uint32(auction->expire_time));
            AHBotLogInfo("AHBuyer: Item GUID: {}", std::to_string(auction->itemGUIDLow));
            AHBotLogInfo("AHBuyer: Item Template: {}", auction->itemEntry);
            AHBotLogInfo("AHBuyer: Item Info:");
            AHBotLogInfo("AHBuyer: Item ID: {}", prototype->ItemId);
            AHBotLogInfo("AHBuyer: Vendor Buy Price: {}", prototype->BuyPrice);
            AHBotLogInfo("AHBuyer: Vendor Sell Price (Base): {}", prototype->SellPrice);
            if (PreventOverpayingForVendorItems == true)
                AHBotLogInfo("AHBuyer: Vender Sell Price (Vendor): {}", vendorSellPrice);
            AHBotLogInfo("AHBuyer: Deposit: {}", auction->deposit);
            AHBotLogInfo("AHBuyer: Bonding: {}", prototype->Bonding);
            AHBotLogInfo("AHBuyer: Quality: {}", prototype->Quality);
            AHBotLogInfo("AHBuyer: Item Level: {}", prototype->ItemLevel);
            AHBotLogInfo("AHBuyer: Stack Size: {}", pItem->GetCount());
            AHBotLogInfo("AHBuyer: Starting Bid: {}", auction->startbid);
            AHBotLogInfo("AHBuyer: Current Bid: {}", auction->bid);
            AHBotLogInfo("AHBuyer: Buyout Price: {}", auction->buyout);
            AHBotLogInfo("AHBuyer: Willing To Pay Per Item Price (Buyout): {}", willingToSpendPerItemPrice);
            AHBotLogInfo("AHBuyer: Willing To Pay For Stack Price (Buyout): {}", willingToPayForStackPrice);
            AHBotLogInfo("AHBuyer: Calculated Bid Amount (0 means too expensive to bid): {}", calcBidAmount);
            AHBotLogInfo("AHBuyer: Decided to Buyout?: {}", doBuyout);
            AHBotLogInfo("AHBuyer: Decided to Bid?: {}", doBid);
            AHBotLogInfo("AHBuyer: Stopped from buying due to 'PreventOverpayingForVendorItems'?: {}", preventedOverpayingForVendorItem);
            AHBotLogInfo("-------------------------------------------------");
        }

        Player* AHBplayer = AHBPlayers[urand(0, AHBPlayers.size() - 1)];

        if (doBid)
        {
            auto trans = CharacterDatabase.BeginTransaction();

            if (auction->bidder)
                sAuctionMgr->SendAuctionOutbiddedMail(auction, uint32(calcBidAmount), AHBplayer, trans);

            auction->bidder = AHBplayer->GetGUIDLow();
            auction->bid = calcBidAmount;

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_AUCTION_BID);
            stmt->setUInt32(0, auction->bidder);
            stmt->setUInt64(1, auction->bid);
            stmt->setUInt32(2, auction->Id);
            trans->Append(stmt);

            CharacterDatabase.CommitTransaction(trans);
        }
        else if (doBuyout)
        {
            auto trans = CharacterDatabase.BeginTransaction();

            if ((auction->bidder) && (AHBplayer->GetGUIDLow() != auction->bidder))
                sAuctionMgr->SendAuctionOutbiddedMail(auction, uint32(auction->buyout), AHBplayer, trans);
            auction->bidder = AHBplayer->GetGUIDLow();
            auction->bid = auction->buyout;

            // Send mails to buyer & seller
            sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
            sAuctionMgr->SendAuctionWonMail(auction, trans);
            auction->DeleteFromDB(trans);

            sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
            auctionHouse->RemoveAuction(auction, auction->itemEntry);
            CharacterDatabase.CommitTransaction(trans);
        }
    }
}

void AuctionHouseBot::Update()
{
    if (AHCharacters.empty() == true)
        return;

    if ((SellingBotEnabled == false) && (BuyingBotEnabled == false))
        return;

    LastBuyCycleCount++;
    LastSellCycleCount++;

    bool buyReady = false;
    bool sellReady = false;

    // Check if an update cycle has been hit. Set # of cycles until next update
    if (LastBuyCycleCount >= CyclesBetweenBuyAction) // Should never be >, but we check anyway
    {
        LastBuyCycleCount = 0;
        CyclesBetweenBuyAction = urand(CyclesBetweenBuyActionMin, CyclesBetweenBuyActionMax);
        buyReady = true;
    }
    if (LastSellCycleCount >= CyclesBetweenSellAction) // Should never be >, but we check anyway
    {
        LastSellCycleCount = 0;
        CyclesBetweenSellAction = urand(CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
        sellReady = true;
    }
    
    // Only update if a Buy or Sell update cycle has been hit
    if (!buyReady && !sellReady)
        return;
    
    // Load all AH Bot Players
    std::vector<std::pair<std::unique_ptr<Player>, std::unique_ptr<WorldSession>>> AHBPlayers;
    AHBPlayers.reserve(AHCharacters.size());
    for (uint32 botIndex = 0; botIndex < AHCharacters.size(); ++botIndex)
    {
        CurrentBotCharGUID = AHCharacters[botIndex].CharacterGUID;

        SessionKey emptyKey{};
        auto session = std::make_unique<WorldSession>(
            AHCharacters[botIndex].AccountID, nullptr, AccountTypes::SEC_PLAYER,
            uint8(sWorld->getIntConfig(WorldIntConfigs::CONFIG_EXPANSION)),
            0, LOCALE_enUS, 0, false, false, false, false, std::string(), emptyKey);
        auto player = std::make_unique<Player>(session.get());
        player->Initialize(AHCharacters[botIndex].CharacterGUID);
        AHBPlayers.emplace_back(std::move(player), std::move(session));
    }

    // Create a vector of Player* for passing to methods
    std::vector<Player*> playersPointerVector;
    playersPointerVector.reserve(AHBPlayers.size());
    for (const auto& pair : AHBPlayers)
        playersPointerVector.emplace_back(pair.first.get());

     // List New Auctions
     if (sellReady) 
     {
         if (sWorld->GetBoolConfig(WorldBoolConfigs::CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION) == false)
         {
             AddNewAuctions(playersPointerVector, &AllianceConfig);
             AddNewAuctions(playersPointerVector, &HordeConfig);
         }
         AddNewAuctions(playersPointerVector, &NeutralConfig);
     }

     // Place New Bids
     if (buyReady && BuyingBotBuyCandidatesPerBuyCycleMin > 0) 
     {
         if (sWorld->GetBoolConfig(WorldBoolConfigs::CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION) == false)
         {
             AddNewAuctionBuyerBotBid(playersPointerVector, &AllianceConfig);
             AddNewAuctionBuyerBotBid(playersPointerVector, &HordeConfig);
         }
         AddNewAuctionBuyerBotBid(playersPointerVector, &NeutralConfig);
     }

    // Destroy dummy players before their sessions.
    for (auto& pair : AHBPlayers)
        pair.first.reset();
}

bool AuctionHouseBot::IsModuleEnabled()
{
    bool sellerEnabled = AHBotConfig::GetBool("AuctionHouseBot.EnableSeller", false);
    bool buyerEnabled = AHBotConfig::GetBool("AuctionHouseBot.Buyer.Enabled", false);
    if (sellerEnabled == false && buyerEnabled == false)
        return false;
    string charString = AHBotConfig::GetString("AuctionHouseBot.GUIDs", "0");
    if (charString == "0" || charString.empty())
    {
        AHBotLogInfo("AuctionHouseBot: AuctionHouseBot.GUIDs is not configured so this module will be disabled");
        return false;
    }
    return true;
}

bool AuctionHouseBot::IsBotCharacter(uint32 characterGuid) const
{
    for (AuctionHouseBotCharacter const& character : AHCharacters)
        if (character.CharacterGUID == characterGuid)
            return true;
    return false;
}

void AuctionHouseBot::LoadModuleConfigFile()
{
    std::string filename = sConfigMgr->GetFilename();
    std::string::size_type slash = filename.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? std::string(".") : filename.substr(0, slash);

    std::string conf = dir + "/mod_ahbot.conf";
    std::string dist = dir + "/mod_ahbot.conf.dist";
    if (!sConfigMgr->LoadMore(conf.c_str()))
        sConfigMgr->LoadMore(dist.c_str());
}

void AuctionHouseBot::ResolveAuctioneers()
{
    uint32 allianceOverride = AHBotConfig::GetUInt("AuctionHouseBot.Alliance.AuctioneerGuid", 0);
    uint32 hordeOverride = AHBotConfig::GetUInt("AuctionHouseBot.Horde.AuctioneerGuid", 0);
    uint32 neutralOverride = AHBotConfig::GetUInt("AuctionHouseBot.Neutral.AuctioneerGuid", 0);

    uint32 allianceGuid = allianceOverride;
    uint32 hordeGuid = hordeOverride;
    uint32 neutralGuid = neutralOverride;

    QueryResult result = WorldDatabase.PQuery(
        "SELECT c.guid, ct.faction_A FROM creature c INNER JOIN creature_template ct ON ct.entry = c.id WHERE (ct.npcflag & %u) != 0",
        uint32(0x00200000));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            uint32 faction = fields[1].GetUInt32();
            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(faction);
            if (!factionEntry)
                continue;

            if (!allianceGuid && (factionEntry->ourMask & FACTION_MASK_ALLIANCE))
                allianceGuid = guid;
            else if (!hordeGuid && (factionEntry->ourMask & FACTION_MASK_HORDE))
                hordeGuid = guid;
            else if (!neutralGuid && !(factionEntry->ourMask & (FACTION_MASK_ALLIANCE | FACTION_MASK_HORDE)))
                neutralGuid = guid;
        } while (result->NextRow());
    }

    AllianceConfig.SetAuctioneerGuid(allianceGuid);
    HordeConfig.SetAuctioneerGuid(hordeGuid);
    NeutralConfig.SetAuctioneerGuid(neutralGuid);

    if (!allianceGuid)
        AHBotLogError("AuctionHouseBot: Could not resolve an Alliance auctioneer spawn GUID");
    if (!hordeGuid)
        AHBotLogError("AuctionHouseBot: Could not resolve a Horde auctioneer spawn GUID");
    if (!neutralGuid)
        AHBotLogError("AuctionHouseBot: Could not resolve a Neutral auctioneer spawn GUID");
}

void AuctionHouseBot::InitializeConfiguration()
{
    debug_Out = AHBotConfig::GetBool("AuctionHouseBot.DEBUG", false);
    debug_Out_Filters = AHBotConfig::GetBool("AuctionHouseBot.DEBUG_FILTERS", false);

    SellingBotEnabled = AHBotConfig::GetBool("AuctionHouseBot.EnableSeller", false);
    BuyingBotEnabled = AHBotConfig::GetBool("AuctionHouseBot.Buyer.Enabled", false);

    string charString = AHBotConfig::GetString("AuctionHouseBot.GUIDs", "0");
    AddCharacters(charString);

    // Top level overrides
    CompleteItemValueOverrideEnabled = AHBotConfig::GetBool("AuctionHouseBot.CompleteItemValueOverride.Enabled", false);
    AddItemValuePairsToItemIDMap(CompleteItemValueOverrideItemListByItemID, AHBotConfig::GetString("AuctionHouseBot.CompleteItemValueOverride.Items", ""));
    CompleteItemValueOverrideDoApplyBidVariations = AHBotConfig::GetBool("AuctionHouseBot.CompleteItemValueOverride.DoApplyBidVariations", false);
    CompleteItemValueOverrideDoApplyBuyoutVariations = AHBotConfig::GetBool("AuctionHouseBot.CompleteItemValueOverride.DoApplyBuyoutVariations", false);

    // Buyer & Seller core properties
    SetCyclesBetweenBuyOrSell();
    ReturnExpiredAuctionItemsToBot = AHBotConfig::GetBool("AuctionHouseBot.ReturnExpiredAuctionItemsToBot", false);
    ItemsPerCycle = AHBotConfig::GetUInt("AuctionHouseBot.ItemsPerCycle", 75);
    AdvancedListingRuleUseDropRatesEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Enabled", false);
    AdvancedListingRuleUseDropRatesWeaponEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Weapon", true);
    AdvancedListingRuleUseDropRatesArmorEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Armor", true);
    AdvancedListingRuleUseDropRatesRecipeEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedListingRules.UseDropRates.Recipe", true);
    AdvancedListingRuleUseDropRatesMinDropRate = AHBotConfig::GetFloat("AuctionHouseBot.AdvancedListingRules.UseDropRates.MinDropRate", 0.005);
    if (AdvancedListingRuleUseDropRatesMinDropRate < 0 || AdvancedListingRuleUseDropRatesMinDropRate > 100) AdvancedListingRuleUseDropRatesMinDropRate = 0.005;
    AdvancedListingRuleUseDropRatesWeaponAffectedQualities.clear();
    AdvancedListingRuleUseDropRatesArmorAffectedQualities.clear();
    AdvancedListingRuleUseDropRatesRecipeAffectedQualities.clear();
    ParseNumberListToSet(AdvancedListingRuleUseDropRatesWeaponAffectedQualities, AHBotConfig::GetString("AuctionHouseBot.AdvancedListingRules.UseDropRates.Weapon.AffectedQualities", "2,3,4,5"), "AdvancedListingRules.UseDropRates.Weapon.AffectedQualities");
    ParseNumberListToSet(AdvancedListingRuleUseDropRatesArmorAffectedQualities, AHBotConfig::GetString("AuctionHouseBot.AdvancedListingRules.UseDropRates.Armor.AffectedQualities", "2,3,4,5"), "AdvancedListingRules.UseDropRates.Armor.AffectedQualities");
    ParseNumberListToSet(AdvancedListingRuleUseDropRatesRecipeAffectedQualities, AHBotConfig::GetString("AuctionHouseBot.AdvancedListingRules.UseDropRates.Recipe.AffectedQualities", "2,3,4,5"), "AdvancedListingRules.UseDropRates.Recipe.AffectedQualities");
    AdvancedListingRuleUseDropRatesExceptionItems.clear();
    ParseNumberListToSet(AdvancedListingRuleUseDropRatesExceptionItems, AHBotConfig::GetString("AuctionHouseBot.AdvancedListingRules.UseDropRates.DisabledItemIDs", ""), "AdvancedListingRules.UseDropRates.DisabledItemIDs");
    MaxBuyoutPriceInCopper = AHBotConfig::GetUInt("AuctionHouseBot.MaxBuyoutPriceInCopper", 1000000000);
    BuyoutVariationReducePercent = AHBotConfig::GetFloat("AuctionHouseBot.BuyoutVariationReducePercent", 0.15f);
    BuyoutVariationAddPercent = AHBotConfig::GetFloat("AuctionHouseBot.BuyoutVariationAddPercent", 0.25f);
    BidVariationHighReducePercent = AHBotConfig::GetFloat("AuctionHouseBot.BidVariationHighReducePercent", 0);
    BidVariationLowReducePercent = AHBotConfig::GetFloat("AuctionHouseBot.BidVariationLowReducePercent", 0.25f);
    BuyoutBelowVendorVariationAddPercentEnabled = AHBotConfig::GetBool("AuctionHouseBot.BuyoutBelowVendorVariationAddPercentEnabled", true);
    BuyoutBelowVendorVariationAddPercent = AHBotConfig::GetFloat("AuctionHouseBot.BuyoutBelowVendorVariationAddPercent", 0.25f);
    ListingExpireTimeInSecondsMin = AHBotConfig::GetUInt("AuctionHouseBot.ListingExpireTimeInSecondsMin", 900);
    if (ListingExpireTimeInSecondsMin < 900)
    {
        AHBotLogError("AuctionHouseBot: ListingExpireTimeInSecondsMin was set below 900 (15 min), so setting to 900");
        ListingExpireTimeInSecondsMin = 900;
    }
    ListingExpireTimeInSecondsMax = AHBotConfig::GetUInt("AuctionHouseBot.ListingExpireTimeInSecondsMax", 86400);
    if (ListingExpireTimeInSecondsMax > 172800)
    {
        AHBotLogError("AuctionHouseBot: ListingExpireTimeInSecondsMax was set above 172800 (48 hours), so setting to 172800");
        ListingExpireTimeInSecondsMax = 172800;
    }
    if (ListingExpireTimeInSecondsMax < ListingExpireTimeInSecondsMin)
    {
        AHBotLogError("AuctionHouseBot: ListingExpireTimeInSecondsMax was smaller than ListingExpireTimeInSecondsMin, setting to 172800 (48 hours) and 900 (15 min)");
        ListingExpireTimeInSecondsMin = 900;
        ListingExpireTimeInSecondsMax = 172800;
    }

    // Buyer Bot
    SetBuyingBotBuyCandidatesPerBuyCycle();
    BuyingBotAcceptablePriceModifier = AHBotConfig::GetFloat("AuctionHouseBot.Buyer.AcceptablePriceModifier", 1);
    BuyingBotAlwaysBidMaxCalculatedPrice = AHBotConfig::GetBool("AuctionHouseBot.Buyer.AlwaysBidMaxCalculatedPrice", false);
    PreventOverpayingForVendorItems = AHBotConfig::GetBool("AuctionHouseBot.Buyer.PreventOverpayingForVendorItems", true);
    if (PreventOverpayingForVendorItems)
        PopulateVendorItemsPrices();
    BuyingBotWillBidAgainstPlayers = AHBotConfig::GetBool("AuctionHouseBot.Buyer.BidAgainstPlayers", false);

    // Stack Ratios
    RandomStackRatioConsumable = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Consumable", 50);
    RandomStackRatioContainer = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Container", 0);
    RandomStackRatioWeapon = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Weapon", 0);
    RandomStackRatioGem = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Gem", 5);
    RandomStackRatioArmor = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Armor", 0);
    RandomStackRatioReagent = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Reagent", 50);
    RandomStackRatioProjectile = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Projectile", 100);
    RandomStackRatioTradeGood = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.TradeGood", 50);
    RandomStackRatioGeneric = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Generic", 100);
    RandomStackRatioRecipe = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Recipe", 0);
    RandomStackRatioQuiver = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Quiver", 0);
    RandomStackRatioQuest = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Quest", 10);
    RandomStackRatioKey = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Key", 10);
    RandomStackRatioMisc = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Misc", 100);
    RandomStackRatioGlyph = GetRandomStackValue("AuctionHouseBot.ListingStack.RandomRatio.Glyph", 0);

    // Stack Increments
    RandomStackIncrementConsumable = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Consumable", 5);
    RandomStackIncrementContainer = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Container", 1);
    RandomStackIncrementWeapon = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Weapon", 1);
    RandomStackIncrementGem = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Gem", 1);
    RandomStackIncrementArmor = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Armor", 1);
    RandomStackIncrementReagent = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Reagent", 1);
    RandomStackIncrementProjectile = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Projectile", 1000);
    RandomStackIncrementTradeGood = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.TradeGood", 5);
    RandomStackIncrementGeneric = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Generic", 1);
    RandomStackIncrementRecipe = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Recipe", 1);
    RandomStackIncrementQuiver = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Quiver", 1);
    RandomStackIncrementQuest = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Quest", 1);
    RandomStackIncrementKey = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Key", 1);
    RandomStackIncrementMisc = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Misc", 1);
    RandomStackIncrementGlyph = GetRandomStackIncrementValue("AuctionHouseBot.ListingStack.RandomStackIncrement.Glyph", 1);

    // Max stack size
    MaximumStackSizeConsumable = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Consumable", 0);
    MaximumStackSizeContainer = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Container", 0);
    MaximumStackSizeWeapon = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Weapon", 0);
    MaximumStackSizeGem = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Gem", 0);
    MaximumStackSizeArmor = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Armor", 0);
    MaximumStackSizeReagent = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Reagent", 0);
    MaximumStackSizeProjectile = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Projectile", 0);
    MaximumStackSizeTradeGood = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.TradeGood", 0);
    MaximumStackSizeGeneric = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Generic", 0);
    MaximumStackSizeRecipe = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Recipe", 0);
    MaximumStackSizeQuiver = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Quiver", 0);
    MaximumStackSizeQuest = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Quest", 0);
    MaximumStackSizeKey = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Key", 0);
    MaximumStackSizeMisc = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Misc", 0);
    MaximumStackSizeGlyph = AHBotConfig::GetUInt("AuctionHouseBot.ListingStack.MaxStackSize.Glyph", 0);

    // List proportions
    ItemListProportionNodesSeed.clear();
    for (int category = 0; category < MAX_ITEM_CLASS; category++)
    {
        for (int quality = 0; quality < MAX_ITEM_QUALITY; quality++)
        {
            if (category != ITEM_CLASS_MONEY && category != ITEM_CLASS_PERMANENT)
            {
                std::string key = std::string("AuctionHouseBot.ListProportion.Category") + GetCategoryName((ItemClass)category) + ".Quality" + GetQualityName((ItemQualities)quality);
                ListProportionNode node;
                node.ItemClassID = category;
                node.ItemQualityID = quality;
                node.Proportion = AHBotConfig::GetUInt(key.c_str(), 0);
                ItemListProportionNodesSeed.push_back(node);
            }
        }
    }
    ItemListProportionMultipliedItemIDs.clear();
    AddItemValuePairsToItemIDMap(ItemListProportionMultipliedItemIDs, AHBotConfig::GetString("AuctionHouseBot.ListProportion.ListMultipliedItemIDs", ""));

    // Price Multipliers
    PriceMultiplierCategoryConsumable = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Consumable", 1);
    PriceMultiplierCategoryContainer = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Container", 1);
    PriceMultiplierCategoryWeapon = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Weapon", 1);
    PriceMultiplierCategoryGem = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Gem", 1);
    PriceMultiplierCategoryArmor = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Armor", 1);
    PriceMultiplierCategoryReagent = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Reagent", 1);
    PriceMultiplierCategoryProjectile = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Projectile", 1);
    PriceMultiplierCategoryTradeGood = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.TradeGood", 1);
    PriceMultiplierCategoryGeneric = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Generic", 1);
    PriceMultiplierCategoryRecipe = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Recipe", 1);
    PriceMultiplierCategoryQuiver = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Quiver", 1);
    PriceMultiplierCategoryQuest = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Quest", 1);
    PriceMultiplierCategoryKey = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Key", 1);
    PriceMultiplierCategoryMisc = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Misc", 1);
    PriceMultiplierCategoryGlyph = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Category.Glyph", 1);
    PriceMultiplierItemLevelCategoryConsumable = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Consumable", 0);
    PriceMultiplierItemLevelCategoryContainer = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Container", 0);
    PriceMultiplierItemLevelCategoryWeapon = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Weapon", 0);
    PriceMultiplierItemLevelCategoryGem = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Gem", 0);
    PriceMultiplierItemLevelCategoryArmor = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Armor", 0);
    PriceMultiplierItemLevelCategoryReagent = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Reagent", 0);
    PriceMultiplierItemLevelCategoryProjectile = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Projectile", 0);
    PriceMultiplierItemLevelCategoryTradeGood = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.TradeGood", 0);
    PriceMultiplierItemLevelCategoryGeneric = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Generic", 0);
    PriceMultiplierItemLevelCategoryRecipe = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Recipe", 0);
    PriceMultiplierItemLevelCategoryQuiver = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Quiver", 0);
    PriceMultiplierItemLevelCategoryQuest = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Quest", 0);
    PriceMultiplierItemLevelCategoryKey = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Key", 0);
    PriceMultiplierItemLevelCategoryMisc = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Misc", 0);
    PriceMultiplierItemLevelCategoryGlyph = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.ItemLevel.Category.Glyph", 0);
    PriceMultiplierQualityPoor = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Poor", 1);
    PriceMultiplierQualityNormal = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Normal", 1);
    PriceMultiplierQualityUncommon = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Uncommon", 1.8);
    PriceMultiplierQualityRare = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Rare", 1.9);
    PriceMultiplierQualityEpic = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Epic", 2.1);
    PriceMultiplierQualityLegendary = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Legendary", 3);
    PriceMultiplierQualityArtifact = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Artifact", 3);
    PriceMultiplierQualityHeirloom = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.Quality.Heirloom", 3);
    for (int category = 0; category < MAX_ITEM_CLASS; category++)
    {
        for (int quality = 0; quality < MAX_ITEM_QUALITY; quality++)
        {
            std::string key = std::string("AuctionHouseBot.PriceMultiplier.Category") + GetCategoryName((ItemClass)category) +
                            ".Quality" + GetQualityName((ItemQualities)quality);

            float multiplier = AHBotConfig::GetFloat(key.c_str(), 1.0f);
            PriceMultiplierCategoryQuality[category][quality] = multiplier;
        }
    }
    PriceMultiplierCategoryMountQualityPoor = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityPoor", 1.0);
    PriceMultiplierCategoryMountQualityNormal = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityNormal", 1.0);
    PriceMultiplierCategoryMountQualityUncommon = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityUncommon", 1.0);
    PriceMultiplierCategoryMountQualityRare = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityRare", 3000.0);
    PriceMultiplierCategoryMountQualityEpic = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityEpic", 5750.0);
    PriceMultiplierCategoryMountQualityLegendary = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityLegendary", 1.0);
    PriceMultiplierCategoryMountQualityArtifact = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityArtifact", 1.0);
    PriceMultiplierCategoryMountQualityHeirloom = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryMount.QualityHeirloom", 1.0);

    PriceMultiplierCategoryPetQualityPoor = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityPoor", 1.0);
    PriceMultiplierCategoryPetQualityNormal = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityNormal", 1.0);
    PriceMultiplierCategoryPetQualityUncommon = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityUncommon", 1.0);
    PriceMultiplierCategoryPetQualityRare = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityRare", 1.0);
    PriceMultiplierCategoryPetQualityEpic = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityEpic", 1.0);
    PriceMultiplierCategoryPetQualityLegendary = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityLegendary", 1.0);
    PriceMultiplierCategoryPetQualityArtifact = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityArtifact", 1.0);
    PriceMultiplierCategoryPetQualityHeirloom = AHBotConfig::GetFloat("AuctionHouseBot.PriceMultiplier.CategoryPet.QualityHeirloom", 1.0);

    // Advanced Pricing
    AdvancedPricingConsumablePotionEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Consumable.Potion.Enabled", true);
    AdvancedPricingConsumableElixirEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Consumable.Elixir.Enabled", true);
    AdvancedPricingConsumableFlaskEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Consumable.Flask.Enabled", true);
    AdvancedPricingGemEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Gem.Enabled", true);
    AdvancedPricingTradeGoodClothEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Cloth.Enabled", true);
    AdvancedPricingTradeGoodHerbEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Herb.Enabled", true);
    AdvancedPricingTradeGoodMetalStoneEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.MetalStone.Enabled", true);
    AdvancedPricingTradeGoodLeatherEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Leather.Enabled", true);
    AdvancedPricingTradeGoodEnchantingEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Enchanting.Enabled", true);
    AdvancedPricingTradeGoodElementalEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Elemental.Enabled", true);
    AdvancedPricingTradeGoodMeatEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.TradeGood.Meat.Enabled", true);
    AdvancedPricingMiscJunkEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Misc.Junk.Enabled", true);
    AdvancedPricingMiscMountEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Misc.Mount.Enabled", true);
    AdvancedPricingMiscPetEnabled = AHBotConfig::GetBool("AuctionHouseBot.AdvancedPricing.Misc.Pet.Enabled", true);

    // Price minimums
    UseItemSellPriceIfHigherThanPriceMinimumCenterBase = AHBotConfig::GetBool("AuctionHouseBot.PriceMinimumCenterBase.UseItemSellPriceIfHigher", true);
    PriceMinimumCenterBaseConsumable = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Consumable",1000);
    PriceMinimumCenterBaseContainer = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Container", 1000);
    PriceMinimumCenterBaseWeapon = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Weapon", 1000);
    PriceMinimumCenterBaseGem = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Gem", 1000);
    PriceMinimumCenterBaseArmor = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Armor", 1000);
    PriceMinimumCenterBaseReagent = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Reagent", 1000);
    PriceMinimumCenterBaseProjectile = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Projectile", 5);
    PriceMinimumCenterBaseTradeGood = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.TradeGood", 850);
    PriceMinimumCenterBaseGeneric = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Generic", 1000);
    PriceMinimumCenterBaseRecipe = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Recipe", 1000);
    PriceMinimumCenterBaseQuiver = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Quiver", 1000);
    PriceMinimumCenterBaseQuest = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Quest", 1000);
    PriceMinimumCenterBaseKey = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Key", 1000);
    PriceMinimumCenterBaseMisc = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Misc", 1000);
    PriceMinimumCenterBaseGlyph = AHBotConfig::GetUInt("AuctionHouseBot.PriceMinimumCenterBase.Glyph", 1000);
    AddItemValuePairsToItemIDMap(PriceMinimumCenterBaseOverridesByItemID, AHBotConfig::GetString("AuctionHouseBot.PriceMinimumCenterBase.OverrideItems", ""));

    // Item level Restrictions
    ListedItemLevelRestrictedEnabled = AHBotConfig::GetBool("AuctionHouseBot.ListedItemLevelRestrict.Enabled", false);
    ListedItemLevelRestrictedUseCraftedItemForCalculation = AHBotConfig::GetBool("AuctionHouseBot.ListedItemLevelRestrict.UseCraftedItemForCalculation", true);
    ListedItemLevelMin = AHBotConfig::GetUInt("AuctionHouseBot.ListedItemLevelRestrict.MinItemLevel", 0);
    ListedItemLevelMax = AHBotConfig::GetUInt("AuctionHouseBot.ListedItemLevelRestrict.MaxItemLevel", 999);
    ListedItemLevelExceptionItems.clear();
    ParseNumberListToSet(ListedItemLevelExceptionItems, AHBotConfig::GetString("AuctionHouseBot.ListedItemLevelRestrict.ExceptionItemIDs", ""), "ListedItemLevelRestrict.ExceptionItemIDs");

    // Item ID Restrictions
    ListedItemIDRestrictedEnabled = AHBotConfig::GetBool("AuctionHouseBot.ListedItemIDRestrict.Enabled", false);
    ListedItemIDMin = AHBotConfig::GetUInt("AuctionHouseBot.ListedItemIDRestrict.MinItemID", 0);
    ListedItemIDMax = AHBotConfig::GetUInt("AuctionHouseBot.ListedItemIDRestrict.MaxItemID", 200000);
    ListedItemIDExceptionItems.clear();
    ParseNumberListToSet(ListedItemIDExceptionItems, AHBotConfig::GetString("AuctionHouseBot.ListedItemIDRestrict.ExceptionItemIDs", ""), "ListedItemIDRestrict.ExceptionItemIDs");

    // Equip or use restrictions
    ListedItemUseOrEquipRestrictedEnabled = AHBotConfig::GetBool("AuctionHouseBot.EquipItemUseOrEquipLevelRestrict.Enabled", false);
    ListedItemUseOrEquipRestrictMinLevel = AHBotConfig::GetUInt("AuctionHouseBot.EquipItemUseOrEquipLevelRestrict.MinLevel", 0);
    ListedItemUseOrEquipRestrictMaxLevel = AHBotConfig::GetUInt("AuctionHouseBot.EquipItemUseOrEquipLevelRestrict.MaxLevel", 999);
    ListedItemUseOrEquipExceptionItems.clear();
    ParseNumberListToSet(ListedItemUseOrEquipExceptionItems, AHBotConfig::GetString("AuctionHouseBot.EquipItemUseOrEquipLevelRestrict.ExceptionItemIDs", ""), "EquipItemUseOrEquipLevelRestrict.ExceptionItemIDs");

    // Disabled Items
    DisabledItemTextFilter = AHBotConfig::GetBool("AuctionHouseBot.DisabledItemTextFilter", true);
    DisabledRecipeProducedItemFilterEnabled = AHBotConfig::GetBool("AuctionHouseBot.DisabledRecipeProducedItemFilterEnabled", false);
    DisabledItems.clear();
    ParseNumberListToSet(DisabledItems, AHBotConfig::GetString("AuctionHouseBot.DisabledInvalidItemIDs", ""), "AuctionHouseBot.DisabledInvalidItemIDs");
    ParseNumberListToSet(DisabledItems, AHBotConfig::GetString("AuctionHouseBot.DisabledCustomItemIDs", ""), "AuctionHouseBot.DisabledCustomItemIDs");
    AddValuesToSetByKeyMap(DisabledRecipeProducedItemClassSubClasses, AHBotConfig::GetString("AuctionHouseBot.DisabledRecipeProducedItemClassSubClasses", ""), 0, 20);

    if (!sWorld->GetBoolConfig(WorldBoolConfigs::CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        AllianceConfig.SetMinItems(AHBotConfig::GetUInt("AuctionHouseBot.Alliance.MinItems", 15000));
        AllianceConfig.SetMaxItems(AHBotConfig::GetUInt("AuctionHouseBot.Alliance.MaxItems", 15000));

        HordeConfig.SetMinItems(AHBotConfig::GetUInt("AuctionHouseBot.Horde.MinItems", 15000));
        HordeConfig.SetMaxItems(AHBotConfig::GetUInt("AuctionHouseBot.Horde.MaxItems", 15000));
    }
    NeutralConfig.SetMinItems(AHBotConfig::GetUInt("AuctionHouseBot.Neutral.MinItems", 15000));
    NeutralConfig.SetMaxItems(AHBotConfig::GetUInt("AuctionHouseBot.Neutral.MaxItems", 15000));

    ResolveAuctioneers();
}

void AuctionHouseBot::EmptyAuctionHouses()
{
    if (AHCharacters.empty())
    {
        AHBotLogError("AuctionHouseBot: No character GUIDs found when emptying Auction Houses via '.ahbot empty' .");
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    auto emptyHouse = [&](AuctionHouseObject* auctionHouse)
    {
        if (!auctionHouse)
            return;

        std::vector<uint32> botAuctionIds;
        for (AuctionHouseObject::AuctionEntryMap::iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
            if (itr->second && IsBotCharacter(itr->second->owner))
                botAuctionIds.push_back(itr->second->Id);

        for (uint32 auctionId : botAuctionIds)
        {
            AuctionEntry* auction = auctionHouse->GetAuction(auctionId);
            if (!auction)
                continue;

            Item* pItem = sAuctionMgr->GetAItem(auction->itemGUIDLow);
            if (auction->bidder)
            {
                if (pItem)
                    sAuctionMgr->SendAuctionCancelledToBidderMail(auction, trans, pItem);
            }

            if (ReturnExpiredAuctionItemsToBot)
                sAuctionMgr->SendAuctionExpiredMail(auction, trans);
            else if (pItem)
            {
                Item::DeleteFromDB(trans, pItem->GetGUIDLow());
                sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
                delete pItem;
                pItem = nullptr;
            }

            uint32 itemEntry = auction->itemEntry;
            auction->DeleteFromDB(trans);
            if (pItem)
                sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
            auctionHouse->RemoveAuction(auction, itemEntry);
        }
    };

    emptyHouse(sAuctionMgr->GetAuctionsMap(AllianceConfig.GetAHFID()));
    emptyHouse(sAuctionMgr->GetAuctionsMap(HordeConfig.GetAHFID()));
    emptyHouse(sAuctionMgr->GetAuctionsMap(NeutralConfig.GetAHFID()));

    CharacterDatabase.CommitTransaction(trans);
}

uint32 AuctionHouseBot::GetRandomStackValue(std::string configKeyString, uint32 defaultValue)
{
    uint32 stackValue = AHBotConfig::GetUInt(configKeyString, defaultValue);
    if (stackValue > 100 || stackValue < 0)
    {
        AHBotLogError("{} value is invalid.  Setting to default ({}).", configKeyString, defaultValue);
        stackValue = defaultValue;
    }
    return stackValue;
}

uint32 AuctionHouseBot::GetRandomStackIncrementValue(std::string configKeyString, uint32 defaultValue)
{
    uint32 stackIncrementValue = AHBotConfig::GetUInt(configKeyString, defaultValue);
    if (stackIncrementValue <= 0)
    {
        AHBotLogError("{} value is invalid.  Setting to default ({}).", configKeyString, defaultValue);
        stackIncrementValue = defaultValue;
    }
    return stackIncrementValue;
}

void AuctionHouseBot::SetCyclesBetweenBuyOrSell()
{
    std::string buyCyclesConfigString = AHBotConfig::GetString("AuctionHouseBot.MinutesBetweenBuyCycle", "1");
    GetConfigMinAndMax(buyCyclesConfigString, CyclesBetweenBuyActionMin, CyclesBetweenBuyActionMax);
    CyclesBetweenBuyAction = urand(CyclesBetweenBuyActionMin, CyclesBetweenBuyActionMax);

    std::string sellCyclesConfigString = AHBotConfig::GetString("AuctionHouseBot.MinutesBetweenSellCycle", "1");
    GetConfigMinAndMax(sellCyclesConfigString, CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
    CyclesBetweenSellAction = urand(CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
}

void AuctionHouseBot::SetBuyingBotBuyCandidatesPerBuyCycle()
{
    std::string candidatesPerCycleString = AHBotConfig::GetString("AuctionHouseBot.Buyer.BuyCandidatesPerBuyCycle", "1");
    GetConfigMinAndMax(candidatesPerCycleString, BuyingBotBuyCandidatesPerBuyCycleMin, BuyingBotBuyCandidatesPerBuyCycleMax);
}

void AuctionHouseBot::GetConfigMinAndMax(std::string config, uint32& min, uint32& max)
{
    size_t pos = config.find(':');
    if (pos != std::string::npos)
    {
        min = std::stoul(config.substr(0, pos));
        max = std::stoul(config.substr(pos + 1));

        if (min < 1)
            min = 1;
        if (max < min)
            max = min;
    }
    else
        min = max = std::stoul(config);
}

void AuctionHouseBot::AddCharacters(std::string characterGUIDString)
{
    AHCharacters.clear();
    std::string delimitedValue;
    std::stringstream characterGUIDStream;
    std::set<uint32> characterGUIDs;

    // Grab from the string
    characterGUIDStream.str(characterGUIDString);
    while (std::getline(characterGUIDStream, delimitedValue, ',')) // Process each charecter GUID in the string, delimited by the comma ","
    {
        std::string valueOne;
        std::stringstream characterGUIDStream(delimitedValue);
        characterGUIDStream >> valueOne;
        auto characterGUID = atoi(valueOne.c_str());
        if (characterGUID == 0)
            continue;
        if (characterGUIDs.find(characterGUID) != characterGUIDs.end())
        {
            if (debug_Out)
                AHBotLogError("AuctionHouseBot: Duplicate character with GUID of {} found, skipping", characterGUID);
        }
        else
            characterGUIDs.insert(characterGUID);
    }

    // Lookup accounts and add them
    if (characterGUIDs.empty() == true)
    {
        AHBotLogError("AuctionHouseBot: No character GUIDs were supplied. Be sure to set AuctionHouseBot.GUIDs");
        return;
    }
    AHCharactersGUIDsForQuery = "";
    bool first = true;
    for (uint32 curGUID : characterGUIDs)
    {
        if (first == false)
        {
            AHCharactersGUIDsForQuery += ", ";
        }
        AHCharactersGUIDsForQuery += std::to_string(curGUID);
        first = false;
    }
    QueryResult queryResult = CharacterDatabase.PQuery("SELECT `guid`, `account` FROM `characters` WHERE guid IN (%s)", AHCharactersGUIDsForQuery.c_str());
    if (!queryResult || queryResult->GetRowCount() == 0)
    {
        AHBotLogError("AuctionHouseBot: No character GUIDs found when looking up values from AuctionHouseBot.GUIDs from the character database 'characters.guid'.");
        return;
    }
    do
    {
        // Pull the data out
        Field* fields = queryResult->Fetch();
        uint32 guid = fields[0].GetUInt32();
        uint32 account = fields[1].GetUInt32();
        AuctionHouseBotCharacter curChar = AuctionHouseBotCharacter(account, guid);
        AHCharacters.push_back(curChar);
    } while (queryResult->NextRow());
}

template <typename ValueType>
void AuctionHouseBot::AddItemValuePairsToItemIDMap(std::unordered_map<uint32, ValueType>& workingValueToItemIDMap, std::string valueToItemIDMapString)
{
    std::string delimitedValue;
    std::stringstream valueToItemIDStream;
    valueToItemIDStream.str(valueToItemIDMapString);
    while (std::getline(valueToItemIDStream, delimitedValue, ',')) // Process each item ID in the string, delimited by the comma ","
    {
        std::string curBlock;
        std::stringstream itemPairStream(delimitedValue);
        itemPairStream >> curBlock;

        // Only process if it has a colon (:)
        if (curBlock.find(":") != std::string::npos)
        {
            std::string itemIDString = curBlock.substr(0, curBlock.find(":"));
            auto itemId = atoi(itemIDString.c_str());
            std::string valueString = curBlock.substr(curBlock.find(":") + 1);
            auto convertedValue = atoi(valueString.c_str());
            if (itemId > 0 && convertedValue > 0)
                workingValueToItemIDMap.insert({ itemId, convertedValue });
        }
    }
}

void AuctionHouseBot::AddValuesToSetByKeyMap(std::map<uint32, std::unordered_set<uint32>>& workingSetByKeyMap, std::string valuesToKeyMapString,
    uint32 wildcardLowValue, uint32 wildcardHighValue)
{
    std::string delimitedValue;
    std::stringstream valueToKeyStream;
    valueToKeyStream.str(valuesToKeyMapString);
    while (std::getline(valueToKeyStream, delimitedValue, ',')) // Process each block delimited by the comma ","
    {
        std::string curBlock;
        std::stringstream itemPairStream(delimitedValue);
        itemPairStream >> curBlock;

        // Only process if it has a colon (:)
        if (curBlock.find(":") != std::string::npos)
        {
            std::string itemIDString = curBlock.substr(0, curBlock.find(":"));
            auto keyValue = atoi(itemIDString.c_str());
            std::string valueString = curBlock.substr(curBlock.find(":") + 1);
            if (valueString == "*")
            {
                for (uint32 i = wildcardLowValue; i <= wildcardHighValue; i++)
                    workingSetByKeyMap[keyValue].insert(i);
            }
            else
            {
                auto convertedValue = atoi(valueString.c_str());
                workingSetByKeyMap[keyValue].insert(convertedValue);
            }
        }
    }
}

void AuctionHouseBot::ParseNumberListToSet(std::set<uint32>& workingItemIDSet, std::string itemString, const char* parentOperationName)
{
    std::string delimitedValue;
    std::stringstream itemIdStream;

    itemIdStream.str(itemString);
    while (std::getline(itemIdStream, delimitedValue, ',')) // Process each item ID in the string, delimited by the comma ","
    {
        // Trim whitespace
        delimitedValue.erase(0, delimitedValue.find_first_not_of(" \t"));
        delimitedValue.erase(delimitedValue.find_last_not_of(" \t") + 1);

        std::string valueOne;
        std::stringstream itemPairStream(delimitedValue);
        itemPairStream >> valueOne;

        // If it has a hypen, then it's a range of numbers
        if (valueOne.find("-") != std::string::npos)
        {
            std::string leftIDString = valueOne.substr(0, valueOne.find("-"));
            std::string rightIDString = valueOne.substr(valueOne.find("-") + 1);

            auto leftId = atoi(leftIDString.c_str());
            auto rightId = atoi(rightIDString.c_str());

            if (leftId > rightId)
            {
                AHBotLogError("AuctionHouseBot: Duplicate item ID range of {} to {} needs to be smallest to largest for {}, skipping", leftId, rightId, parentOperationName);
            }
            else
            {
                for (int32 i = leftId; i <= rightId; ++i)
                    AddToNumberListSet(workingItemIDSet, i, parentOperationName);
            }
        }
        else
        {
            auto itemId = atoi(valueOne.c_str());
            AddToNumberListSet(workingItemIDSet, itemId, parentOperationName);
        }
    }
}

void AuctionHouseBot::AddToNumberListSet(std::set<uint32>& workingItemIDSet, uint32 itemID, const char* parentOperationName)
{
    if (workingItemIDSet.find(itemID) != workingItemIDSet.end())
    {
        if (debug_Out)
            AHBotLogError("AuctionHouseBot: Duplicate item id {} attempted to be put into a working item set from operation {}, skipping", itemID, parentOperationName);
    }
    else
    {
        workingItemIDSet.insert(itemID);
    }
}

const char* AuctionHouseBot::GetQualityName(ItemQualities quality)
{
    switch (quality)
    {
        case ITEM_QUALITY_POOR:       return "Poor";
        case ITEM_QUALITY_NORMAL:     return "Normal";
        case ITEM_QUALITY_UNCOMMON:   return "Uncommon";
        case ITEM_QUALITY_RARE:       return "Rare";
        case ITEM_QUALITY_EPIC:       return "Epic";
        case ITEM_QUALITY_LEGENDARY:  return "Legendary";
        case ITEM_QUALITY_ARTIFACT:   return "Artifact";
        case ITEM_QUALITY_HEIRLOOM:   return "Heirloom";
        default:                      return "Unknown";
    }
}

const char* AuctionHouseBot::GetCategoryName(ItemClass category)
{
    switch (category)
    {
        case ITEM_CLASS_CONSUMABLE:   return "Consumable";
        case ITEM_CLASS_CONTAINER:    return "Container";
        case ITEM_CLASS_WEAPON:       return "Weapon";
        case ITEM_CLASS_GEM:          return "Gem";
        case ITEM_CLASS_ARMOR:        return "Armor";
        case ITEM_CLASS_REAGENT:      return "Reagent";
        case ITEM_CLASS_PROJECTILE:   return "Projectile";
        case ITEM_CLASS_TRADE_GOODS:  return "TradeGood";
        case ITEM_CLASS_GENERIC:      return "Generic";
        case ITEM_CLASS_RECIPE:       return "Recipe";
        case ITEM_CLASS_MONEY:        return "Money";
        case ITEM_CLASS_QUIVER:       return "Quiver";
        case ITEM_CLASS_QUEST:        return "Quest";
        case ITEM_CLASS_KEY:          return "Key";
        case ITEM_CLASS_PERMANENT:    return "Permanent";
        case ITEM_CLASS_MISCELLANEOUS:         return "Misc";
        case ITEM_CLASS_GLYPH:        return "Glyph";
        default:                      return "Unknown";
    }
}

void AuctionHouseBot::PopulateVendorItemsPrices()
{
    // Load vendor items' prices into a vector for fast lookup
    QueryResult r = WorldDatabase.Query("SELECT MAX(entry) FROM item_template");
    if (!r)
    {
        vendorItemsPrices.clear();
        return;
    }
    Field* f = r->Fetch();
    uint32_t maxItemID = f[0].GetUInt32();
    // Size by max entry + 1 so the highest entry itself is a valid index
    vendorItemsPrices = std::vector<uint32>(maxItemID + 1, UINT32_MAX);

    QueryResult result = WorldDatabase.PQuery("SELECT v.entry, MIN(v.SellPrice) AS SellPrice FROM item_template v JOIN npc_vendor p ON v.entry = p.item WHERE v.class != %u GROUP BY v.entry", ITEM_CLASS_TRADE_GOODS);
    if (result)
    {
        do
        {
            Field* pFields = result->Fetch();
            uint32_t itemID = pFields[0].GetUInt32();
            uint32_t itemPrice = pFields[1].GetUInt32();
            if (itemID < vendorItemsPrices.size())
                vendorItemsPrices[itemID] = itemPrice;
        } while (result->NextRow());
    }
}

void AuctionHouseBot::CleanupBotMail()
{
    if (AHCharactersGUIDsForQuery.empty())
        return;

    // Dummy listing characters should not keep auction mail. Keep expired-item
    // returns only when ReturnExpiredAuctionItemsToBot is enabled (subject field 3 = AUCTION_EXPIRED).
    QueryResult mailResult;
    if (ReturnExpiredAuctionItemsToBot)
        mailResult = CharacterDatabase.PQuery(
            "SELECT id FROM mail WHERE receiver IN (%s) AND NOT (messageType = 2 AND SUBSTRING_INDEX(SUBSTRING_INDEX(subject, ':', 3), ':', -1) = '3')",
            AHCharactersGUIDsForQuery.c_str());
    else
        mailResult = CharacterDatabase.PQuery(
            "SELECT id FROM mail WHERE receiver IN (%s)",
            AHCharactersGUIDsForQuery.c_str());

    if (mailResult)
    {
        std::string mailIds;
        bool first = true;
        do
        {
            if (!first)
                mailIds += ",";
            first = false;
            mailIds += std::to_string(mailResult->Fetch()[0].GetUInt32());
        } while (mailResult->NextRow());

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        QueryResult mailItems = CharacterDatabase.PQuery(
            "SELECT item_guid FROM mail_items WHERE mail_id IN (%s)", mailIds.c_str());
        if (mailItems)
        {
            do
            {
                Item::DeleteFromDB(trans, mailItems->Fetch()[0].GetUInt32());
            } while (mailItems->NextRow());
        }

        trans->PAppend("DELETE FROM mail_items WHERE mail_id IN (%s)", mailIds.c_str());
        trans->PAppend("DELETE FROM mail WHERE id IN (%s)", mailIds.c_str());
        CharacterDatabase.CommitTransaction(trans);
    }

    CleanupExpiredAuctionItems();
}

void AuctionHouseBot::CleanupExpiredAuctionItems()
{
    if (AHCharactersGUIDsForQuery.empty())
        return;

    QueryResult queryItemInstancesResult;
    if (ReturnExpiredAuctionItemsToBot)
        queryItemInstancesResult = CharacterDatabase.PQuery(
            "SELECT item_instance.guid FROM item_instance "
            "LEFT JOIN auctionhouse ON auctionhouse.itemguid = item_instance.guid "
            "LEFT JOIN mail_items ON mail_items.item_guid = item_instance.guid "
            "WHERE item_instance.owner_guid IN (%s) AND auctionhouse.id IS NULL AND mail_items.mail_id IS NULL",
            AHCharactersGUIDsForQuery.c_str());
    else
        queryItemInstancesResult = CharacterDatabase.PQuery(
            "SELECT guid FROM item_instance LEFT JOIN auctionhouse ON auctionhouse.itemguid = item_instance.guid "
            "WHERE item_instance.owner_guid IN (%s) AND auctionhouse.id IS NULL",
            AHCharactersGUIDsForQuery.c_str());

    if (!queryItemInstancesResult)
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    do
    {
        uint32 guid = queryItemInstancesResult->Fetch()[0].GetUInt32();
        Item::DeleteFromDB(trans, guid);
    } while (queryItemInstancesResult->NextRow());

    CharacterDatabase.CommitTransaction(trans);
}

bool AuctionHouseBot::IsItemQuestReward(uint32 itemID)
{
    return (QuestRewardItemIDs.find(itemID) != QuestRewardItemIDs.end());
}

bool AuctionHouseBot::IsItemCrafted(uint32 itemID)
{
    return (ItemIDsProducedByRecipes.find(itemID) != ItemIDsProducedByRecipes.end());
}

bool AuctionHouseBot::IsItemCategoryQualityInDBDropRatesConfig(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    switch (proto->Class)
    {
        case ITEM_CLASS_WEAPON:
            return (AdvancedListingRuleUseDropRatesWeaponEnabled &&
                    AdvancedListingRuleUseDropRatesWeaponAffectedQualities.contains(proto->Quality));
        case ITEM_CLASS_ARMOR:
            return (AdvancedListingRuleUseDropRatesArmorEnabled &&
                    AdvancedListingRuleUseDropRatesArmorAffectedQualities.contains(proto->Quality));
        case ITEM_CLASS_RECIPE:
            return (AdvancedListingRuleUseDropRatesRecipeEnabled &&
                    AdvancedListingRuleUseDropRatesRecipeAffectedQualities.contains(proto->Quality));
        default:
            return false;
    }
}

bool AuctionHouseBot::IsItemEligibleForDBDropRates(ItemTemplate const* proto)
{
    if (!proto || !AdvancedListingRuleUseDropRatesEnabled)
        return false;

    // Only continue if the itemID isn't an exception
    if (AdvancedListingRuleUseDropRatesExceptionItems.find(proto->ItemId) != AdvancedListingRuleUseDropRatesExceptionItems.end())
        return false;

    // If feature enabled, && item category/quality enabled
    //   && is not crafted, && is not a quest reward
    return (AdvancedListingRuleUseDropRatesEnabled &&
            IsItemCategoryQualityInDBDropRatesConfig(proto) &&
            !IsItemCrafted(proto->ItemId) &&
            !IsItemQuestReward(proto->ItemId)
    );
}
