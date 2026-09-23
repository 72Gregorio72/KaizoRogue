#ifndef GUARD_KAIZO_RANDOMIZER_H
#define GUARD_KAIZO_RANDOMIZER_H

#include "global.h"
#include "constants/species.h"
#include "constants/moves.h"
u16 GetKaizoRandomizedItem(u16 originalItem);
void Script_RandomizeFieldItem(void);

static u32 sKaizoRng;

static inline void SeedKaizoRng(u32 seed)
{
    sKaizoRng = seed;
}

static inline u16 NextKaizoRng(void)
{
    sKaizoRng = 1103515245 * sKaizoRng + 12345;
    return (sKaizoRng >> 16) & 0x7FFF;
}

static inline void GetRandomizedBaseStats(enum Species species, u8 *statsOut)
{
    species = SanitizeSpeciesId(species);

    if (species == SPECIES_NONE || species == SPECIES_EGG)
    {
        for (u32 i = 0; i < NUM_STATS; i++)
            statsOut[i] = 10;
        return;
    }

    bool32 isShedinja = HasShedinjaHPHandling(species);

    u16 bst = gSpeciesInfo[species].baseHP +
              gSpeciesInfo[species].baseAttack +
              gSpeciesInfo[species].baseDefense +
              gSpeciesInfo[species].baseSpeed +
              gSpeciesInfo[species].baseSpAttack +
              gSpeciesInfo[species].baseSpDefense;

    u32 runSeed = gSaveBlock2Ptr->randomizerSeed;
    if (runSeed == 0)
        runSeed = 0x12345678;

    SeedKaizoRng(runSeed ^ (species * 0x45D9F3B));

    u32 weights[NUM_STATS];
    u32 totalWeight = 0;

    for (u32 i = 0; i < NUM_STATS; i++)
    {
        if (i == STAT_HP && isShedinja)
        {
            weights[i] = 0;
            continue;
        }
        weights[i] = 15 + (NextKaizoRng() % 85);
        totalWeight += weights[i];
    }

    u16 targetBST = isShedinja ? (bst - 1) : bst;
    u16 allocated = 0;

    for (u32 i = 0; i < NUM_STATS - 1; i++)
    {
        if (i == STAT_HP && isShedinja)
        {
            statsOut[i] = 1;
            continue;
        }

        u32 val = (targetBST * weights[i]) / totalWeight;
        if (val < 10) val = 10;
        if (val > 255) val = 255;

        statsOut[i] = (u8)val;
        allocated += statsOut[i];
    }

    s32 remainder = targetBST - allocated;
    if (remainder < 10) remainder = 10;
    if (remainder > 255) remainder = 255;
    statsOut[NUM_STATS - 1] = (u8)remainder;
}

#endif // GUARD_KAIZO_RANDOMIZER_H
