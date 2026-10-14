#ifndef GUARD_ROGUELIKE_RARITY_H
#define GUARD_ROGUELIKE_RARITY_H

#include "global.h"

enum RarityTier
{
    RARITY_NONE     = 0,
    RARITY_COMMON   = 1,
    RARITY_UNCOMMON = 2,
    RARITY_RARE     = 3,
    RARITY_EPIC     = 4,
    RARITY_TIERS_COUNT = 4
};

enum GamePhase
{
    GAME_PHASE_EARLY, // 0-2 badges
    GAME_PHASE_MID,   // 3-5 badges
    GAME_PHASE_LATE   // 6-8 badges
};

enum LootContext
{
    LOOT_CONTEXT_STANDARD,
    LOOT_CONTEXT_MART,
    LOOT_CONTEXT_MINIBOSS,
    LOOT_CONTEXT_GYM_BOSS
};

enum RarityCategory
{
    CATEGORY_POKEMON,
    CATEGORY_MOVE,
    CATEGORY_ITEM
};

bool32 IsSpeciesInRarityPool(u16 species);
bool32 IsMoveInRarityPool(u16 move);
bool32 IsItemInRarityPool(u16 item);

u8 GetSpeciesRarityTier(u16 species);
u8 GetMoveRarityTier(u16 move);
u8 GetItemRarityTier(u16 item);

u8 GetCurrentGamePhase(void);
u8 RollRarityTier(u8 category, u8 lootContext, u32 *rng);

u16 GetRandomRaritySpecies(u32 *rng);
u16 GetRandomRarityMove(u32 *rng);
u16 GetRandomRarityItem(u32 *rng);

u16 GetRandomRaritySpeciesByTier(u8 tier, u32 *rng);
u16 GetRandomRarityMoveByTier(u8 tier, u32 *rng);
u16 GetRandomRarityItemByTier(u8 tier, u32 *rng);

u16 RollRandomRaritySpecies(u8 lootContext, u32 *rng);
u16 RollRandomRarityMove(u8 lootContext, u32 *rng);
u16 RollRandomRarityItem(u8 lootContext, u32 *rng);

u32 GetRarityPokemonCount(void);
u32 GetRarityMovesCount(void);
u32 GetRarityItemsCount(void);

const u8 *GetRarityColoredName(u8 tier);
const u8 *GetRarityColorPrefix(u8 tier);
void LoadRarityFramePalette(u8 tier, u8 paletteNum);

#endif // GUARD_ROGUELIKE_RARITY_H
