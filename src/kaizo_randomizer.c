#include "global.h"
#include "pokemon.h"
#include "random.h"
#include "event_data.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "item.h"
#include "string_util.h"
#include "script.h"
#include "constants/vars.h"
#include "field_specials.h"

// --- PROTOTIPI GENERALI ---
static bool32 IsValidKaizoSpecies(u16 species);
u16 GetKaizoRandomizedItem(u16 originalItem);
void GenerateNewKaizoSeedAndStarters(void);
void BufferKaizoStarterName(void);
void GiveSelectedStarter(void);
void Script_SetupOaksLabStarter(void);
void Script_GetKaizoStarterSpecies(void);
void Script_GiveKaizoStarter(void);
void Script_RandomizeFindItemResult(void);

// --- LOGICA ITEM RANDOMIZER ---
u16 GetKaizoRandomizedItem(u16 originalItem)
{
    if (originalItem == ITEM_NONE || originalItem >= ITEMS_COUNT)
        return originalItem;

    // Se è uno Strumento Base (Key Item come Bici, Mappa, ecc.), non toccarlo
    if (gItemsInfo[originalItem].pocket == POCKET_KEY_ITEMS)
        return originalItem;

    u32 seed = gSaveBlock2Ptr->randomizerSeed;
    if (seed == 0)
        seed = 0x54321678;

    // Rimescolamento basato su mappa e item
    u32 itemSeed = seed ^ (originalItem * 7919)
                        ^ (gSaveBlock1Ptr->location.mapGroup << 16)
                        ^ (gSaveBlock1Ptr->location.mapNum << 8);

    u32 rng = itemSeed;
    u16 candidate;

    for (int attempts = 0; attempts < 100; attempts++)
    {
        rng = 1103515245 * rng + 12345;
        candidate = 1 + ((rng >> 16) % (ITEMS_COUNT - 1));

        if (candidate == ITEM_NONE || candidate >= ITEMS_COUNT)
            continue;
        if (gItemsInfo[candidate].pocket == POCKET_KEY_ITEMS)
            continue;

        return candidate;
    }

    return ITEM_SUPER_POTION;
}

void Script_RandomizeFindItemResult(void)
{
    u16 originalItem = VarGet(VAR_RESULT);
    VarSet(VAR_RESULT, GetKaizoRandomizedItem(originalItem));
}

// --- LOGICA SPECIE / STARTER ---
static bool32 IsValidKaizoSpecies(u16 species)
{
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return FALSE;

    if (!IsSpeciesEnabled(species))
        return FALSE;

    if (gSpeciesInfo[species].isMegaEvolution || gSpeciesInfo[species].isPrimalReversion)
        return FALSE;

#if P_GEN_7_POKEMON == TRUE
    if (gSpeciesInfo[species].isTotem)
        return FALSE;
#endif

#if P_GEN_8_POKEMON == TRUE
    if (gSpeciesInfo[species].isGigantamax)
        return FALSE;
#endif

    return TRUE;
}

void GenerateNewKaizoSeedAndStarters(void)
{
    u32 entropy = REG_VCOUNT | (REG_TM1CNT_L << 16) | Random32();
    if (entropy == 0)
        entropy = 0x54321678;
    gSaveBlock2Ptr->randomizerSeed = entropy;

    u32 rng = entropy;
    for (int i = 0; i < 3; i++)
    {
        u16 mon;
        bool32 duplicate;
        do {
            duplicate = FALSE;
            rng = 1103515245 * rng + 12345;
            mon = 1 + ((rng >> 16) % (NUM_SPECIES - 1));

            if (!IsValidKaizoSpecies(mon))
            {
                duplicate = TRUE;
                continue;
            }
            for (int j = 0; j < i; j++)
            {
                if (gSaveBlock2Ptr->randomStarters[j] == mon)
                    duplicate = TRUE;
            }
        } while (duplicate);

        gSaveBlock2Ptr->randomStarters[i] = mon;
    }
}

void BufferKaizoStarterName(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot > 2)
        slot = 0;

    if (gSaveBlock2Ptr->randomStarters[slot] == SPECIES_NONE)
        GenerateNewKaizoSeedAndStarters();

    u16 species = gSaveBlock2Ptr->randomStarters[slot];
    StringCopy(gStringVar1, GetSpeciesName(species));
    gSpecialVar_Result = species;
}

void GiveSelectedStarter(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot > 2)
        slot = 0;

    if (gSaveBlock2Ptr->randomStarters[slot] == SPECIES_NONE)
        GenerateNewKaizoSeedAndStarters();

    u16 chosenSpecies = gSaveBlock2Ptr->randomStarters[slot];
    struct Pokemon mon;

    CreateMon(&mon, chosenSpecies, 5, Random32(), OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    CalculateMonStats(&mon);
    GiveCapturedMonToPlayer(&mon);
}

void Script_SetupOaksLabStarter(void)
{
    u8 slot = VarGet(VAR_TEMP_1); // 0, 1, 2
    if (slot > 2)
        slot = 0;

    if (gSaveBlock2Ptr->randomStarters[0] == SPECIES_NONE)
        GenerateNewKaizoSeedAndStarters();

    u16 playerSpecies = gSaveBlock2Ptr->randomStarters[slot];
    u8 rivalSlot = (slot == 0) ? 2 : (slot == 1) ? 0 : 1;
    u16 rivalSpecies = gSaveBlock2Ptr->randomStarters[rivalSlot];

    gSaveBlock2Ptr->rivalStarterSpecies = rivalSpecies;

    VarSet(VAR_TEMP_2, playerSpecies);
    VarSet(VAR_TEMP_3, rivalSpecies);

    StringCopy(gStringVar1, GetSpeciesName(playerSpecies));
    StringCopy(gStringVar2, GetSpeciesName(rivalSpecies));
}

void Script_GetKaizoStarterSpecies(void)
{
    Script_SetupOaksLabStarter();
}

void Script_GiveKaizoStarter(void)
{
    u16 species = VarGet(VAR_TEMP_2);
    struct Pokemon mon;

    CreateMon(&mon, species, 5, Random32(), OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    CalculateMonStats(&mon);
    GiveCapturedMonToPlayer(&mon);
}

// --- LOGICA EXP MULTIPLIER ---
u32 GetCurrentExpMultiplier(void)
{
    u16 val = VarGet(VAR_EXP_MULTIPLIER);
    if (val == 0 || val == 100)
        return 100; // 1.0x (Predefinito vanilla)
    if (val == 1)
        return 0;   // 0x (Nessuna EXP)
    return val;     // 50 (0.5x), 150 (1.5x), 200 (2x), 300 (3x), 500 (5x), 1000 (10x)
}

static const u8 sText_Exp0x[]  = _("0x (Nessuna EXP)");
static const u8 sText_Exp05x[] = _("0.5x (Dimezzata)");
static const u8 sText_Exp1x[]  = _("1x (Normale)");
static const u8 sText_Exp15x[] = _("1.5x");
static const u8 sText_Exp2x[]  = _("2x (Doppia)");
static const u8 sText_Exp3x[]  = _("3x (Tripla)");
static const u8 sText_Exp5x[]  = _("5x");
static const u8 sText_Exp10x[] = _("10x");
static const u8 sText_ExpX[]   = _("x");

void BufferCurrentExpMultiplierName(void)
{
    u32 mult = GetCurrentExpMultiplier();
    switch (mult)
    {
    case 0:
        StringCopy(gStringVar1, sText_Exp0x);
        break;
    case 50:
        StringCopy(gStringVar1, sText_Exp05x);
        break;
    case 100:
        StringCopy(gStringVar1, sText_Exp1x);
        break;
    case 150:
        StringCopy(gStringVar1, sText_Exp15x);
        break;
    case 200:
        StringCopy(gStringVar1, sText_Exp2x);
        break;
    case 300:
        StringCopy(gStringVar1, sText_Exp3x);
        break;
    case 500:
        StringCopy(gStringVar1, sText_Exp5x);
        break;
    case 1000:
        StringCopy(gStringVar1, sText_Exp10x);
        break;
    default:
        ConvertIntToDecimalStringN(gStringVar2, mult / 100, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringCopy(gStringVar1, gStringVar2);
        StringAppend(gStringVar1, sText_ExpX);
        break;
    }
}