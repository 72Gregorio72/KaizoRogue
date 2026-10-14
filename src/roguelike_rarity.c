#include "global.h"
#include "random.h"
#include "event_data.h"
#include "palette.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "roguelike_rarity.h"
#include "data/roguelike_rarity.h"

// Rarity Frame Palettes (GBA 15-bit BGR colors)
static const u16 sRarityFramePal_Common[16] = {
    0x0000, 0x18C6, 0x2108, 0x3DEF, 0x4631, 0x5294, 0x5EF7, 0x6739,
    0x7BDE, 0x0000, 0x0000, 0x5EF7, 0x4631, 0x18C6, 0x7FFF, 0x18C6
};

static const u16 sRarityFramePal_Uncommon[16] = {
    0x0000, 0x04E1, 0x0941, 0x1642, 0x1AA4, 0x22E5, 0x2B47, 0x376B,
    0x53F4, 0x0000, 0x0000, 0x2B47, 0x1AA4, 0x04E1, 0x7FFF, 0x04E1
};

static const u16 sRarityFramePal_Rare[16] = {
    0x0000, 0x2061, 0x34A2, 0x6563, 0x69C5, 0x7206, 0x7A88, 0x7ECC,
    0x7F95, 0x0000, 0x0000, 0x7A88, 0x69C5, 0x2061, 0x7FFF, 0x2061
};

static const u16 sRarityFramePal_Epic[16] = {
    0x0000, 0x0428, 0x084D, 0x1479, 0x14DA, 0x153C, 0x15BE, 0x1A5F,
    0x2B9F, 0x0000, 0x0000, 0x15BE, 0x14DA, 0x0428, 0x7FFF, 0x0428
};

// Colored Rarity Strings
static const u8 sText_RarityCommon[]   = _("{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}COMMON{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}");
static const u8 sText_RarityUncommon[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}UNCOMMON{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}");
static const u8 sText_RarityRare[]     = _("{COLOR BLUE}{SHADOW LIGHT_BLUE}RARE{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}");
static const u8 sText_RarityEpic[]     = _("{COLOR RED}{SHADOW LIGHT_RED}EPIC{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}");

static const u8 sText_ColorCommon[]    = _("{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}");
static const u8 sText_ColorUncommon[]  = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}");
static const u8 sText_ColorRare[]      = _("{COLOR BLUE}{SHADOW LIGHT_BLUE}");
static const u8 sText_ColorEpic[]      = _("{COLOR RED}{SHADOW LIGHT_RED}");

bool32 IsSpeciesInRarityPool(u16 species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;
    return (sRarityTierTable_Pokemon[species] != RARITY_NONE);
}

bool32 IsMoveInRarityPool(u16 move)
{
    if (move == MOVE_NONE || move >= MOVES_COUNT_ALL)
        return FALSE;
    return (sRarityTierTable_Moves[move] != RARITY_NONE);
}

bool32 IsItemInRarityPool(u16 item)
{
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return FALSE;
    return (sRarityTierTable_Items[item] != RARITY_NONE);
}

u8 GetSpeciesRarityTier(u16 species)
{
    if (species >= NUM_SPECIES)
        return RARITY_NONE;
    return sRarityTierTable_Pokemon[species];
}

u8 GetMoveRarityTier(u16 move)
{
    if (move >= MOVES_COUNT_ALL)
        return RARITY_NONE;
    return sRarityTierTable_Moves[move];
}

u8 GetItemRarityTier(u16 item)
{
    if (item >= ITEMS_COUNT)
        return RARITY_NONE;
    return sRarityTierTable_Items[item];
}

u32 GetRarityPokemonCount(void)
{
    return ARRAY_COUNT(sRarityPokemon_All);
}

u32 GetRarityMovesCount(void)
{
    return ARRAY_COUNT(sRarityMoves_All);
}

u32 GetRarityItemsCount(void)
{
    return ARRAY_COUNT(sRarityItems_All);
}

u8 GetCurrentGamePhase(void)
{
    u8 badges = 0;
    int i;
    for (i = 0; i < NUM_BADGES; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i))
            badges++;
    }

    if (badges <= 2)
        return GAME_PHASE_EARLY;
    else if (badges <= 5)
        return GAME_PHASE_MID;
    else
        return GAME_PHASE_LATE;
}

static const u8 sWeights_Standard_Early_Pokemon[4] = { 70, 25, 4, 1 };
static const u8 sWeights_Standard_Early_Move[4]    = { 65, 25, 9, 1 };
static const u8 sWeights_Standard_Early_Item[4]    = { 75, 20, 4, 1 };

static const u8 sWeights_Standard_Mid_Pokemon[4]   = { 45, 38, 15, 2 };
static const u8 sWeights_Standard_Mid_Move[4]      = { 40, 40, 17, 3 };
static const u8 sWeights_Standard_Mid_Item[4]      = { 45, 35, 17, 3 };

static const u8 sWeights_Standard_Late_Pokemon[4]  = { 20, 45, 32, 3 };
static const u8 sWeights_Standard_Late_Move[4]     = { 15, 45, 32, 8 };
static const u8 sWeights_Standard_Late_Item[4]     = { 20, 40, 32, 8 };

static const u8 sWeights_Mart_Pokemon[4]           = { 40, 45, 14, 1 };
static const u8 sWeights_Mart_Move[4]              = { 35, 45, 18, 2 };
static const u8 sWeights_Mart_Item[4]              = { 40, 40, 18, 2 };

static const u8 sWeights_MiniBoss_Pokemon[4]       = {  0, 45, 45, 10 };
static const u8 sWeights_MiniBoss_Move[4]          = {  0, 35, 50, 15 };
static const u8 sWeights_MiniBoss_Item[4]          = {  0, 35, 50, 15 };

static const u8 sWeights_GymBoss_Pokemon[4]        = {  0, 20, 70, 10 };
static const u8 sWeights_GymBoss_Move[4]           = {  0, 10, 60, 30 };
static const u8 sWeights_GymBoss_Item[4]           = {  0, 10, 60, 30 };

static const u8 *GetTierWeights(u8 category, u8 lootContext, u8 phase)
{
    switch (lootContext)
    {
    case LOOT_CONTEXT_MART:
        if (category == CATEGORY_POKEMON)
            return sWeights_Mart_Pokemon;
        if (category == CATEGORY_MOVE)
            return sWeights_Mart_Move;
        return sWeights_Mart_Item;

    case LOOT_CONTEXT_MINIBOSS:
        if (category == CATEGORY_POKEMON)
            return sWeights_MiniBoss_Pokemon;
        if (category == CATEGORY_MOVE)
            return sWeights_MiniBoss_Move;
        return sWeights_MiniBoss_Item;

    case LOOT_CONTEXT_GYM_BOSS:
        if (category == CATEGORY_POKEMON)
            return sWeights_GymBoss_Pokemon;
        if (category == CATEGORY_MOVE)
            return sWeights_GymBoss_Move;
        return sWeights_GymBoss_Item;

    case LOOT_CONTEXT_STANDARD:
    default:
        switch (phase)
        {
        case GAME_PHASE_MID:
            if (category == CATEGORY_POKEMON)
                return sWeights_Standard_Mid_Pokemon;
            if (category == CATEGORY_MOVE)
                return sWeights_Standard_Mid_Move;
            return sWeights_Standard_Mid_Item;

        case GAME_PHASE_LATE:
            if (category == CATEGORY_POKEMON)
                return sWeights_Standard_Late_Pokemon;
            if (category == CATEGORY_MOVE)
                return sWeights_Standard_Late_Move;
            return sWeights_Standard_Late_Item;

        case GAME_PHASE_EARLY:
        default:
            if (category == CATEGORY_POKEMON)
                return sWeights_Standard_Early_Pokemon;
            if (category == CATEGORY_MOVE)
                return sWeights_Standard_Early_Move;
            return sWeights_Standard_Early_Item;
        }
    }
}

u8 RollRarityTier(u8 category, u8 lootContext, u32 *rng)
{
    const u8 *weights = GetTierWeights(category, lootContext, GetCurrentGamePhase());
    u32 roll;
    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        roll = ((*rng) >> 16) % 100;
    }
    else
    {
        roll = Random() % 100;
    }

    if (roll < weights[0])
        return RARITY_COMMON;
    roll -= weights[0];
    if (roll < weights[1])
        return RARITY_UNCOMMON;
    roll -= weights[1];
    if (roll < weights[2])
        return RARITY_RARE;
    return RARITY_EPIC;
}

u16 GetRandomRaritySpecies(u32 *rng)
{
    if (ARRAY_COUNT(sRarityPokemon_All) == 0)
        return SPECIES_NONE;

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return sRarityPokemon_All[((*rng) >> 16) % ARRAY_COUNT(sRarityPokemon_All)];
    }
    return sRarityPokemon_All[Random() % ARRAY_COUNT(sRarityPokemon_All)];
}

u16 GetRandomRarityMove(u32 *rng)
{
    if (ARRAY_COUNT(sRarityMoves_All) == 0)
        return MOVE_NONE;

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return sRarityMoves_All[((*rng) >> 16) % ARRAY_COUNT(sRarityMoves_All)];
    }
    return sRarityMoves_All[Random() % ARRAY_COUNT(sRarityMoves_All)];
}

u16 GetRandomRarityItem(u32 *rng)
{
    if (ARRAY_COUNT(sRarityItems_All) == 0)
        return ITEM_NONE;

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return sRarityItems_All[((*rng) >> 16) % ARRAY_COUNT(sRarityItems_All)];
    }
    return sRarityItems_All[Random() % ARRAY_COUNT(sRarityItems_All)];
}

u16 GetRandomRaritySpeciesByTier(u8 tier, u32 *rng)
{
    const u16 *pool = NULL;
    u32 count = 0;

    switch (tier)
    {
    case RARITY_COMMON:
        pool = sRarityPokemon_Common;
        count = ARRAY_COUNT(sRarityPokemon_Common);
        break;
    case RARITY_UNCOMMON:
        pool = sRarityPokemon_Uncommon;
        count = ARRAY_COUNT(sRarityPokemon_Uncommon);
        break;
    case RARITY_RARE:
        pool = sRarityPokemon_Rare;
        count = ARRAY_COUNT(sRarityPokemon_Rare);
        break;
    case RARITY_EPIC:
        pool = sRarityPokemon_Epic;
        count = ARRAY_COUNT(sRarityPokemon_Epic);
        break;
    default:
        return GetRandomRaritySpecies(rng);
    }

    if (count == 0)
        return GetRandomRaritySpecies(rng);

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return pool[((*rng) >> 16) % count];
    }
    return pool[Random() % count];
}

u16 GetRandomRarityMoveByTier(u8 tier, u32 *rng)
{
    const u16 *pool = NULL;
    u32 count = 0;

    switch (tier)
    {
    case RARITY_COMMON:
        pool = sRarityMoves_Common;
        count = ARRAY_COUNT(sRarityMoves_Common);
        break;
    case RARITY_UNCOMMON:
        pool = sRarityMoves_Uncommon;
        count = ARRAY_COUNT(sRarityMoves_Uncommon);
        break;
    case RARITY_RARE:
        pool = sRarityMoves_Rare;
        count = ARRAY_COUNT(sRarityMoves_Rare);
        break;
    case RARITY_EPIC:
        pool = sRarityMoves_Epic;
        count = ARRAY_COUNT(sRarityMoves_Epic);
        break;
    default:
        return GetRandomRarityMove(rng);
    }

    if (count == 0)
        return GetRandomRarityMove(rng);

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return pool[((*rng) >> 16) % count];
    }
    return pool[Random() % count];
}

u16 GetRandomRarityItemByTier(u8 tier, u32 *rng)
{
    const u16 *pool = NULL;
    u32 count = 0;

    switch (tier)
    {
    case RARITY_COMMON:
        pool = sRarityItems_Common;
        count = ARRAY_COUNT(sRarityItems_Common);
        break;
    case RARITY_UNCOMMON:
        pool = sRarityItems_Uncommon;
        count = ARRAY_COUNT(sRarityItems_Uncommon);
        break;
    case RARITY_RARE:
        pool = sRarityItems_Rare;
        count = ARRAY_COUNT(sRarityItems_Rare);
        break;
    case RARITY_EPIC:
        pool = sRarityItems_Epic;
        count = ARRAY_COUNT(sRarityItems_Epic);
        break;
    default:
        return GetRandomRarityItem(rng);
    }

    if (count == 0)
        return GetRandomRarityItem(rng);

    if (rng != NULL)
    {
        *rng = 1103515245 * (*rng) + 12345;
        return pool[((*rng) >> 16) % count];
    }
    return pool[Random() % count];
}

u16 RollRandomRaritySpecies(u8 lootContext, u32 *rng)
{
    u8 tier = RollRarityTier(CATEGORY_POKEMON, lootContext, rng);
    return GetRandomRaritySpeciesByTier(tier, rng);
}

u16 RollRandomRarityMove(u8 lootContext, u32 *rng)
{
    u8 tier = RollRarityTier(CATEGORY_MOVE, lootContext, rng);
    return GetRandomRarityMoveByTier(tier, rng);
}

u16 RollRandomRarityItem(u8 lootContext, u32 *rng)
{
    u8 tier = RollRarityTier(CATEGORY_ITEM, lootContext, rng);
    return GetRandomRarityItemByTier(tier, rng);
}

const u8 *GetRarityColoredName(u8 tier)
{
    switch (tier)
    {
    case RARITY_COMMON:
        return sText_RarityCommon;
    case RARITY_UNCOMMON:
        return sText_RarityUncommon;
    case RARITY_RARE:
        return sText_RarityRare;
    case RARITY_EPIC:
        return sText_RarityEpic;
    default:
        return sText_RarityCommon;
    }
}

const u8 *GetRarityColorPrefix(u8 tier)
{
    switch (tier)
    {
    case RARITY_COMMON:
        return sText_ColorCommon;
    case RARITY_UNCOMMON:
        return sText_ColorUncommon;
    case RARITY_RARE:
        return sText_ColorRare;
    case RARITY_EPIC:
        return sText_ColorEpic;
    default:
        return sText_ColorCommon;
    }
}

void LoadRarityFramePalette(u8 tier, u8 paletteNum)
{
    const u16 *pal;
    switch (tier)
    {
    case RARITY_UNCOMMON:
        pal = sRarityFramePal_Uncommon;
        break;
    case RARITY_RARE:
        pal = sRarityFramePal_Rare;
        break;
    case RARITY_EPIC:
        pal = sRarityFramePal_Epic;
        break;
    case RARITY_COMMON:
    default:
        pal = sRarityFramePal_Common;
        break;
    }
    LoadPalette(pal, BG_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
}
