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
#include "malloc.h"
#include "script_menu.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/region_map_sections.h"
#include "move.h"
#include "data/kaizo_item_pool.h"


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

    if (ARRAY_COUNT(sKaizoItemPool) == 0)
        return originalItem;

    u32 seed = gSaveBlock2Ptr->randomizerSeed;
    if (seed == 0)
        seed = 0x54321678;

    // Rimescolamento deterministico basato su seed, mappa e item
    u32 itemSeed = seed ^ (originalItem * 7919)
                        ^ (gSaveBlock1Ptr->location.mapGroup << 16)
                        ^ (gSaveBlock1Ptr->location.mapNum << 8);

    u32 rng = itemSeed;
    rng = 1103515245 * rng + 12345;
    u16 index = (rng >> 16) % ARRAY_COUNT(sKaizoItemPool);

    return sKaizoItemPool[index];
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
// --- LOGICA IV BOOSTER NPC ---

static u8 sIvBoosterPartySlot;
static u8 sIvBoosterRemainingPoints;
static u8 sIvBoosterSelectedStat;

// Ordine MON_DATA_*_IV: HP, ATK, DEF, SPATK, SPDEF, SPEED
static const u8 sIvStatMonData[6] = {
    MON_DATA_HP_IV,
    MON_DATA_ATK_IV,
    MON_DATA_DEF_IV,
    MON_DATA_SPATK_IV,
    MON_DATA_SPDEF_IV,
    MON_DATA_SPEED_IV
};

static const u8 sIvStatName_HP[]    = _("HP");
static const u8 sIvStatName_ATK[]   = _("Attack");
static const u8 sIvStatName_DEF[]   = _("Defense");
static const u8 sIvStatName_SPATK[] = _("Sp. Atk");
static const u8 sIvStatName_SPDEF[] = _("Sp. Def");
static const u8 sIvStatName_SPD[]   = _("Speed");

static const u8 *const sIvStatNames[6] = {
    sIvStatName_HP,
    sIvStatName_ATK,
    sIvStatName_DEF,
    sIvStatName_SPATK,
    sIvStatName_SPDEF,
    sIvStatName_SPD
};

static const u8 sIvText_Max[]     = _("/31 MAX");
static const u8 sIvText_Slash31[] = _("/31");
static const u8 sIvText_Colon[]   = _(": ");
static const u8 sIvText_Plus[]    = _("+");
static const u8 sIvText_Punto[]   = _(" Point");
static const u8 sIvText_Punti[]   = _(" Points");
static const u8 sIvText_Esci[]    = _("Done / Back");
static const u8 sIvText_Annulla[] = _("Cancel");

void Script_IVBooster_Init(void)
{
    u8 partyCount = gPartiesCount[B_TRAINER_PLAYER];
    u8 slot = (u8)gSpecialVar_0x8004;
    if (slot >= partyCount)
        slot = 0;
    sIvBoosterPartySlot = slot;
    sIvBoosterRemainingPoints = 5;
    sIvBoosterSelectedStat = 0;
    GetMonData(&gParties[B_TRAINER_PLAYER][sIvBoosterPartySlot], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sIvBoosterRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = sIvBoosterRemainingPoints;
}

void Script_IVBooster_GetRemainingPoints(void)
{
    GetMonData(&gParties[B_TRAINER_PLAYER][sIvBoosterPartySlot], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sIvBoosterRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = sIvBoosterRemainingPoints;
}

void Script_IVBooster_PushStatChoices(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sIvBoosterPartySlot];
    u32 i;
    for (i = 0; i < 6; i++)
    {
        u8 currentIv = (u8)GetMonData(mon, sIvStatMonData[i]);
        u8 *buf = Alloc(64);
        u8 *ptr;
        struct ListMenuItem item;
        ptr = StringCopy(buf, sIvStatNames[i]);
        ptr = StringAppend(ptr, sIvText_Colon);
        ptr = ConvertIntToDecimalStringN(ptr, currentIv, STR_CONV_MODE_LEFT_ALIGN, 2);
        if (currentIv >= 31)
            StringAppend(ptr, sIvText_Max);
        else
            StringAppend(ptr, sIvText_Slash31);
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }
    {
        u8 *bufExit = Alloc(32);
        struct ListMenuItem exitItem;
        StringCopy(bufExit, sIvText_Esci);
        exitItem.name = bufExit;
        exitItem.id = 6;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_IVBooster_SelectStat(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sIvBoosterPartySlot];
    u8 statIndex = (u8)gSpecialVar_0x8005;
    u8 currentIv;
    u8 maxCanAdd;
    if (statIndex > 5)
        statIndex = 0;
    sIvBoosterSelectedStat = statIndex;
    currentIv = (u8)GetMonData(mon, sIvStatMonData[sIvBoosterSelectedStat]);
    StringCopy(gStringVar1, sIvStatNames[sIvBoosterSelectedStat]);
    ConvertIntToDecimalStringN(gStringVar2, currentIv, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, sIvBoosterRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 1);
    if (currentIv >= 31)
    {
        gSpecialVar_Result = 0;
    }
    else
    {
        maxCanAdd = 31 - currentIv;
        if (maxCanAdd > sIvBoosterRemainingPoints)
            maxCanAdd = sIvBoosterRemainingPoints;
        gSpecialVar_Result = maxCanAdd;
    }
}

void Script_IVBooster_PushPointChoices(void)
{
    u8 maxCanAdd = (u8)gSpecialVar_Result;
    u32 p;
    if (maxCanAdd > 5)
        maxCanAdd = 5;
    for (p = 1; p <= maxCanAdd; p++)
    {
        u8 *buf = Alloc(32);
        u8 *ptr;
        struct ListMenuItem item;
        ptr = StringCopy(buf, sIvText_Plus);
        ptr = ConvertIntToDecimalStringN(ptr, p, STR_CONV_MODE_LEFT_ALIGN, 1);
        if (p == 1)
            StringAppend(ptr, sIvText_Punto);
        else
            StringAppend(ptr, sIvText_Punti);
        item.name = buf;
        item.id = (s32)p;
        MultichoiceDynamic_PushElement(item);
    }
    {
        u8 *bufCancel = Alloc(32);
        struct ListMenuItem cancelItem;
        StringCopy(bufCancel, sIvText_Annulla);
        cancelItem.name = bufCancel;
        cancelItem.id = 0;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_IVBooster_ApplyPoints(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sIvBoosterPartySlot];
    u8 pointsToAdd = (u8)gSpecialVar_0x8005;
    u8 currentIv = (u8)GetMonData(mon, sIvStatMonData[sIvBoosterSelectedStat]);
    u8 newIv = currentIv + pointsToAdd;
    if (newIv > 31)
        newIv = 31;
    SetMonData(mon, sIvStatMonData[sIvBoosterSelectedStat], &newIv);
    CalculateMonStats(mon);
    if (sIvBoosterRemainingPoints >= pointsToAdd)
        sIvBoosterRemainingPoints -= pointsToAdd;
    else
        sIvBoosterRemainingPoints = 0;
    StringCopy(gStringVar1, sIvStatNames[sIvBoosterSelectedStat]);
    ConvertIntToDecimalStringN(gStringVar2, pointsToAdd, STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar3, newIv, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_Result = sIvBoosterRemainingPoints;
}

// --- LOGICA EV MANAGER NPC ---

static u8 sEvManagerPartySlot;
static u16 sEvManagerRemainingPoints;
static u8 sEvManagerSelectedStat;
static u8 sEvManagerWorkingEVs[6];
static u8 sEvManagerOriginalEVs[6];

static const u8 sEvStatMonData[6] = {
    MON_DATA_HP_EV,
    MON_DATA_ATK_EV,
    MON_DATA_DEF_EV,
    MON_DATA_SPATK_EV,
    MON_DATA_SPDEF_EV,
    MON_DATA_SPEED_EV
};

static const u8 sEvText_Slash252[]     = _("/252");
static const u8 sEvText_Slash252Max[]  = _("/252 MAX");
static const u8 sEvText_Colon[]        = _(": ");
static const u8 sEvText_Plus[]         = _("+");
static const u8 sEvText_Minus[]        = _("-");
static const u8 sEvText_EV[]           = _(" EV");
static const u8 sEvText_EVMax[]        = _(" EV (MAX)");
static const u8 sEvText_AllRemaining[] = _("All (+");
static const u8 sEvText_CloseParen[]   = _(" EV)");
static const u8 sEvText_ClearStat[]    = _("Reset stat (0 EV)");
static const u8 sEvText_SaveApply[]    = _("Save & Apply");
static const u8 sEvText_CancelAll[]    = _("Cancel Changes");

void Script_EVManager_Init(void)
{
    u8 partyCount = gPartiesCount[B_TRAINER_PLAYER];
    u8 slot = (u8)gSpecialVar_0x8004;
    u32 i;
    u16 totalEv = 0;
    struct Pokemon *mon;

    if (slot >= partyCount)
        slot = 0;
    sEvManagerPartySlot = slot;
    mon = &gParties[B_TRAINER_PLAYER][sEvManagerPartySlot];

    for (i = 0; i < 6; i++)
    {
        sEvManagerOriginalEVs[i] = (u8)GetMonData(mon, sEvStatMonData[i]);
        sEvManagerWorkingEVs[i] = 0;
        totalEv += sEvManagerOriginalEVs[i];
    }

    sEvManagerRemainingPoints = totalEv;
    sEvManagerSelectedStat = 0;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, totalEv, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = totalEv;
}

void Script_EVManager_GetRemainingPoints(void)
{
    GetMonData(&gParties[B_TRAINER_PLAYER][sEvManagerPartySlot], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sEvManagerRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = sEvManagerRemainingPoints;
}

void Script_EVManager_PushStatChoices(void)
{
    u32 i;
    for (i = 0; i < 6; i++)
    {
        u8 currentEv = sEvManagerWorkingEVs[i];
        u8 *buf = Alloc(64);
        u8 *ptr;
        struct ListMenuItem item;
        ptr = StringCopy(buf, sIvStatNames[i]);
        ptr = StringAppend(ptr, sEvText_Colon);
        ptr = ConvertIntToDecimalStringN(ptr, currentEv, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (currentEv >= 252)
            StringAppend(ptr, sEvText_Slash252Max);
        else
            StringAppend(ptr, sEvText_Slash252);
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }
    {
        u8 *bufSave = Alloc(32);
        struct ListMenuItem saveItem;
        StringCopy(bufSave, sEvText_SaveApply);
        saveItem.name = bufSave;
        saveItem.id = 6;
        MultichoiceDynamic_PushElement(saveItem);
    }
    {
        u8 *bufCancel = Alloc(32);
        struct ListMenuItem cancelItem;
        StringCopy(bufCancel, sEvText_CancelAll);
        cancelItem.name = bufCancel;
        cancelItem.id = 7;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_EVManager_SelectStat(void)
{
    u8 statIndex = (u8)gSpecialVar_0x8005;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sEvManagerPartySlot];
    u32 i;

    if (statIndex == 6) // Salva e Applica
    {
        for (i = 0; i < 6; i++)
        {
            SetMonData(mon, sEvStatMonData[i], &sEvManagerWorkingEVs[i]);
        }
        CalculateMonStats(mon);
        GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        gSpecialVar_Result = 1; // Success
        return;
    }
    else if (statIndex == 7) // Annulla
    {
        gSpecialVar_Result = 2; // Cancelled
        return;
    }

    if (statIndex > 5)
        statIndex = 0;
    sEvManagerSelectedStat = statIndex;
    StringCopy(gStringVar1, sIvStatNames[sEvManagerSelectedStat]);
    ConvertIntToDecimalStringN(gStringVar2, sEvManagerWorkingEVs[sEvManagerSelectedStat], STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, sEvManagerRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = 0; // Show point choices
}

void Script_EVManager_PushPointChoices(void)
{
    u8 currentEv = sEvManagerWorkingEVs[sEvManagerSelectedStat];
    u16 remaining = sEvManagerRemainingPoints;
    u16 maxCanAdd = 252 - currentEv;
    static const u8 sAddAmounts[] = { 4, 16, 32, 64, 128 };
    static const u8 sSubAmounts[] = { 4, 16, 32, 64, 128 };
    u32 i;

    // Positive additions
    for (i = 0; i < ARRAY_COUNT(sAddAmounts); i++)
    {
        u8 amt = sAddAmounts[i];
        if (remaining >= amt && maxCanAdd >= amt)
        {
            u8 *buf = Alloc(32);
            u8 *ptr = StringCopy(buf, sEvText_Plus);
            ptr = ConvertIntToDecimalStringN(ptr, amt, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(ptr, sEvText_EV);
            struct ListMenuItem item;
            item.name = buf;
            item.id = (s32)amt;
            MultichoiceDynamic_PushElement(item);
        }
    }

    // Max out stat (up to 252) if possible
    if (maxCanAdd > 0 && remaining >= maxCanAdd && maxCanAdd != 4 && maxCanAdd != 16 && maxCanAdd != 32 && maxCanAdd != 64 && maxCanAdd != 128)
    {
        u8 *bufMax = Alloc(32);
        u8 *ptr = StringCopy(bufMax, sEvText_Plus);
        ptr = ConvertIntToDecimalStringN(ptr, maxCanAdd, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(ptr, sEvText_EVMax);
        struct ListMenuItem maxItem;
        maxItem.name = bufMax;
        maxItem.id = 252;
        MultichoiceDynamic_PushElement(maxItem);
    }
    else if (remaining > 0 && remaining < maxCanAdd && remaining != 4 && remaining != 16 && remaining != 32 && remaining != 64 && remaining != 128)
    {
        u8 *bufRem = Alloc(32);
        u8 *ptr = StringCopy(bufRem, sEvText_AllRemaining);
        ptr = ConvertIntToDecimalStringN(ptr, remaining, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(ptr, sEvText_CloseParen);
        struct ListMenuItem remItem;
        remItem.name = bufRem;
        remItem.id = 999;
        MultichoiceDynamic_PushElement(remItem);
    }

    // Negative subtractions
    for (i = 0; i < ARRAY_COUNT(sSubAmounts); i++)
    {
        u8 amt = sSubAmounts[i];
        if (currentEv >= amt)
        {
            u8 *buf = Alloc(32);
            u8 *ptr = StringCopy(buf, sEvText_Minus);
            ptr = ConvertIntToDecimalStringN(ptr, amt, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(ptr, sEvText_EV);
            struct ListMenuItem item;
            item.name = buf;
            item.id = -(s32)amt;
            MultichoiceDynamic_PushElement(item);
        }
    }

    // Reset this stat to 0
    if (currentEv > 0 && currentEv != 4 && currentEv != 16 && currentEv != 32 && currentEv != 64 && currentEv != 128)
    {
        u8 *bufClear = Alloc(32);
        struct ListMenuItem clearItem;
        StringCopy(bufClear, sEvText_ClearStat);
        clearItem.name = bufClear;
        clearItem.id = -999;
        MultichoiceDynamic_PushElement(clearItem);
    }

    {
        u8 *bufBack = Alloc(32);
        struct ListMenuItem backItem;
        StringCopy(bufBack, sIvText_Annulla);
        backItem.name = bufBack;
        backItem.id = 0;
        MultichoiceDynamic_PushElement(backItem);
    }
}

void Script_EVManager_ApplyPoints(void)
{
    s32 delta = (s32)gSpecialVar_0x8005;
    u8 currentEv = sEvManagerWorkingEVs[sEvManagerSelectedStat];
    u16 maxCanAdd = 252 - currentEv;

    if (delta == 252) // Max out
    {
        u16 toAdd = (sEvManagerRemainingPoints < maxCanAdd) ? sEvManagerRemainingPoints : maxCanAdd;
        sEvManagerWorkingEVs[sEvManagerSelectedStat] += (u8)toAdd;
        sEvManagerRemainingPoints -= toAdd;
    }
    else if (delta == 999) // Add all remaining
    {
        u16 toAdd = (sEvManagerRemainingPoints < maxCanAdd) ? sEvManagerRemainingPoints : maxCanAdd;
        sEvManagerWorkingEVs[sEvManagerSelectedStat] += (u8)toAdd;
        sEvManagerRemainingPoints -= toAdd;
    }
    else if (delta == -999) // Clear stat
    {
        sEvManagerRemainingPoints += currentEv;
        sEvManagerWorkingEVs[sEvManagerSelectedStat] = 0;
    }
    else if (delta > 0)
    {
        u16 toAdd = (u16)delta;
        if (toAdd > maxCanAdd)
            toAdd = maxCanAdd;
        if (toAdd > sEvManagerRemainingPoints)
            toAdd = sEvManagerRemainingPoints;
        sEvManagerWorkingEVs[sEvManagerSelectedStat] += (u8)toAdd;
        sEvManagerRemainingPoints -= toAdd;
    }
    else if (delta < 0)
    {
        u16 toSub = (u16)(-delta);
        if (toSub > currentEv)
            toSub = currentEv;
        sEvManagerWorkingEVs[sEvManagerSelectedStat] -= (u8)toSub;
        sEvManagerRemainingPoints += toSub;
    }

    StringCopy(gStringVar1, sIvStatNames[sEvManagerSelectedStat]);
    ConvertIntToDecimalStringN(gStringVar2, sEvManagerWorkingEVs[sEvManagerSelectedStat], STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, sEvManagerRemainingPoints, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = sEvManagerRemainingPoints;
}

void Script_EVManager_ResetAllEVs(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sEvManagerPartySlot];
    u8 zero = 0;
    u32 i;
    for (i = 0; i < 6; i++)
    {
        SetMonData(mon, sEvStatMonData[i], &zero);
    }
    CalculateMonStats(mon);
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
}

// --- NPC CAMBIO NATURA RANDOM ---
void Script_NatureChanger_Init(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);

    u8 nature = GetMonData(mon, MON_DATA_HIDDEN_NATURE);
    if (nature >= NUM_NATURES)
        nature = GetNature(mon);

    StringCopy(gStringVar2, gNaturesInfo[nature].name);
}

void Script_NatureChanger_Apply(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];

    u8 currentNature = GetMonData(mon, MON_DATA_HIDDEN_NATURE);
    if (currentNature >= NUM_NATURES)
        currentNature = GetNature(mon);

    u8 newNature = (currentNature + 1 + (Random() % (NUM_NATURES - 1))) % NUM_NATURES;
    u8 hiddenNature = newNature;
    SetMonData(mon, MON_DATA_HIDDEN_NATURE, &hiddenNature);
    CalculateMonStats(mon);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    StringCopy(gStringVar2, gNaturesInfo[newNature].name);
}

// --- NPC CAMBIO ABILITÀ RANDOM ---
void Script_AbilityChanger_Init(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);

    enum Ability ability = GetMonAbility(mon);
    StringCopy(gStringVar2, gAbilitiesInfo[ability].name);
}

void Script_AbilityChanger_Apply(void)
{
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];

    enum Ability currentAbility = GetMonAbility(mon);
    enum Ability newAbility;
    u32 tries = 0;
    do {
        newAbility = (Random() % (ABILITIES_COUNT - 1)) + 1;
        tries++;
    } while ((newAbility == currentAbility || gAbilitiesInfo[newAbility].name[0] == '-' || gAbilitiesInfo[newAbility].name[0] == 0) && tries < 1000);

    u16 customAbility = newAbility;
    SetMonData(mon, MON_DATA_CUSTOM_ABILITY, &customAbility);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    StringCopy(gStringVar2, gAbilitiesInfo[newAbility].name);
}

// --- POKÉ MART BUILD HUB LIMIT (1 PER CITTÀ) ---

static u8 GetCurrentCityMartIndex(void)
{
    u8 mapSec = gMapHeader.regionMapSectionId;
    switch (mapSec)
    {
    case MAPSEC_VIRIDIAN_CITY: return 0;
    case MAPSEC_PEWTER_CITY: return 1;
    case MAPSEC_CERULEAN_CITY: return 2;
    case MAPSEC_VERMILION_CITY: return 3;
    case MAPSEC_LAVENDER_TOWN: return 4;
    case MAPSEC_CELADON_CITY: return 5;
    case MAPSEC_SAFFRON_CITY: return 6;
    case MAPSEC_FUCHSIA_CITY: return 7;
    case MAPSEC_CINNABAR_ISLAND: return 8;
    case MAPSEC_INDIGO_PLATEAU:
    case MAPSEC_POKEMON_LEAGUE: return 9;
    case MAPSEC_ONE_ISLAND: return 10;
    case MAPSEC_TWO_ISLAND: return 11;
    case MAPSEC_THREE_ISLAND: return 12;
    case MAPSEC_FOUR_ISLAND: return 13;
    case MAPSEC_FIVE_ISLAND: return 14;
    case MAPSEC_SIX_ISLAND:
    case MAPSEC_SEVEN_ISLAND: return 15;
    default: return 0;
    }
}

void Script_MartBuild_CheckCanUse(void)
{
    u8 city = GetCurrentCityMartIndex();
    if (gSaveBlock2Ptr->martBuildUsedBitfield & (1 << city))
        gSpecialVar_Result = 0; // Already used for this city
    else
        gSpecialVar_Result = 1; // Available
}

void Script_MartBuild_Consume(void)
{
    u8 city = GetCurrentCityMartIndex();
    gSaveBlock2Ptr->martBuildUsedBitfield |= (1 << city);
}

// --- SESSION TRACKING FOR BUILD HUB (DISALLOW RE-CHOOSING AN OPTION) ---

static u8 sMartBuildSessionUsed = 0;

void Script_MartBuild_InitSession(void)
{
    sMartBuildSessionUsed = 0;
}

static const u8 sMartMenuText_StatPoints[]      = _("Stat Points");
static const u8 sMartMenuText_MoveDraft[]       = _("Draft Random Move");
static const u8 sMartMenuText_MoveRelearner[]   = _("Remember Move");
static const u8 sMartMenuText_IVBooster[]       = _("Modify IVs");
static const u8 sMartMenuText_EVManager[]       = _("Train EVs");
static const u8 sMartMenuText_NatureChanger[]   = _("Change Nature");
static const u8 sMartMenuText_AbilityChanger[]  = _("Change Ability");
static const u8 sMartMenuText_Shop[]            = _("Buy Items");
static const u8 sMartMenuText_Exit[]            = _("Done / Exit");

static const u8 sMartMenuText_RedPrefix[]       = _("{COLOR RED}{SHADOW LIGHT_RED}");
static const u8 sMartMenuText_UsedSuffix[]      = _(" (Used)");

void Script_MartBuild_PushMainMenuChoices(void)
{
    static const u8 *const sOptionNames[] = {
        sMartMenuText_StatPoints,
        sMartMenuText_MoveDraft,
        sMartMenuText_MoveRelearner,
        sMartMenuText_IVBooster,
        sMartMenuText_EVManager,
        sMartMenuText_NatureChanger,
        sMartMenuText_AbilityChanger,
    };

    for (u32 i = 0; i < ARRAY_COUNT(sOptionNames); i++)
    {
        u8 *buf = Alloc(64);
        if (sMartBuildSessionUsed & (1 << i))
        {
            u8 *ptr = StringCopy(buf, sMartMenuText_RedPrefix);
            ptr = StringAppend(ptr, sOptionNames[i]);
            StringAppend(ptr, sMartMenuText_UsedSuffix);
        }
        else
        {
            StringCopy(buf, sOptionNames[i]);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufShop = Alloc(32);
        StringCopy(bufShop, sMartMenuText_Shop);
        struct ListMenuItem shopItem;
        shopItem.name = bufShop;
        shopItem.id = 7;
        MultichoiceDynamic_PushElement(shopItem);
    }

    {
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sMartMenuText_Exit);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 8;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_MartBuild_SelectOption(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    if (option < 7 && (sMartBuildSessionUsed & (1 << option)))
    {
        gSpecialVar_Result = 100; // Already used
        return;
    }
    gSpecialVar_Result = option;
}

void Script_MartBuild_MarkOptionUsed(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    if (option < 7)
        sMartBuildSessionUsed |= (1 << option);
}

void Script_MartBuild_ExitSession(void)
{
    if (sMartBuildSessionUsed != 0)
    {
        u8 city = GetCurrentCityMartIndex();
        gSaveBlock2Ptr->martBuildUsedBitfield |= (1 << city);
        gSpecialVar_Result = 1; // Modifications were made, city token consumed!
    }
    else
    {
        gSpecialVar_Result = 0; // Exited without making any modifications
    }
}

// --- LOGIC STAT POINTS (+20 PER BADGE UP TO 500 BST) ---

static u8 sStatPointsPartySlot = 0;
static u8 sStatPointsSelectedStat = 0;

static const u8 sStatPointNames[6][16] = {
    _("HP"),
    _("Attack"),
    _("Defense"),
    _("Speed"),
    _("Sp. Atk"),
    _("Sp. Def")
};

static const u8 sStatPointsText_Colon[]         = _(": ");
static const u8 sStatPointsText_OpenParenPlus[] = _(" (+");
static const u8 sStatPointsText_CloseParen[]    = _(")");
static const u8 sStatPointsText_Exit[]          = _("Done / Back");
static const u8 sStatPointsText_Plus[]          = _("+");
static const u8 sStatPointsText_Minus[]         = _("-");
static const u8 sStatPointsText_Points[]        = _(" Points");
static const u8 sStatPointsText_AllRemaining[]  = _("+All (");
static const u8 sStatPointsText_Reset[]         = _("Reset stat bonus");
static const u8 sStatPointsText_Back[]          = _("Back");
static const u8 sDraftText_Cancel[]             = _("Cancel");

static u16 GetPlayerBadgesCount(void)
{
    u16 count = 0;
    for (u32 i = FLAG_BADGE01_GET; i < FLAG_BADGE01_GET + NUM_BADGES; i++)
    {
        if (FlagGet(i))
            count++;
    }
    return count;
}

static u16 GetTotalCustomStatPointsAllocated(void)
{
    u16 total = 0;
    for (int i = 0; i < 6; i++)
        total += gSaveBlock2Ptr->customStatPoints[i];
    return total;
}

void Script_StatPoints_Init(void)
{
    u8 slot = (u8)VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    sStatPointsPartySlot = slot;

    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 badges = GetPlayerBadgesCount();
    u16 totalEarned = badges * 20;
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, effectiveBST, STR_CONV_MODE_LEFT_ALIGN, 3);

    if (naturalBST >= 500 || effectiveBST >= 500)
    {
        gSpecialVar_Result = 1; // BST naturale >= 500 o già a 500
        return;
    }

    if (totalEarned <= totalUsed)
    {
        ConvertIntToDecimalStringN(gStringVar3, badges, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_Result = 2; // Nessun punto da distribuire
        return;
    }

    u16 remainingEarned = totalEarned - totalUsed;
    u16 maxCanAdd = 500 - effectiveBST;
    u16 availableToAdd = (remainingEarned < maxCanAdd) ? remainingEarned : maxCanAdd;

    ConvertIntToDecimalStringN(gStringVar3, availableToAdd, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = 0; // Pronto per distribuire
}

void Script_StatPoints_GetRemainingPoints(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 badges = GetPlayerBadgesCount();
    u16 totalEarned = badges * 20;
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, effectiveBST, STR_CONV_MODE_LEFT_ALIGN, 3);

    u16 remainingEarned = (totalEarned > totalUsed) ? (totalEarned - totalUsed) : 0;
    u16 maxCanAdd = (effectiveBST < 500) ? (500 - effectiveBST) : 0;
    u16 availableToAdd = (remainingEarned < maxCanAdd) ? remainingEarned : maxCanAdd;

    ConvertIntToDecimalStringN(gStringVar3, availableToAdd, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = availableToAdd;
}

void Script_StatPoints_PushStatChoices(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    for (int i = 0; i < 6; i++)
    {
        u16 baseStat = (i == STAT_HP) ? GetSpeciesBaseHP(species) : GetSpeciesBaseStat(species, i);
        u8 customBonus = gSaveBlock2Ptr->customStatPoints[i];
        u16 totalBase = baseStat + customBonus;

        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, sStatPointNames[i]);
        ptr = StringAppend(ptr, sStatPointsText_Colon);
        ptr = ConvertIntToDecimalStringN(ptr, totalBase, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (customBonus > 0)
        {
            ptr = StringAppend(ptr, sStatPointsText_OpenParenPlus);
            ptr = ConvertIntToDecimalStringN(ptr, customBonus, STR_CONV_MODE_LEFT_ALIGN, 3);
            ptr = StringAppend(ptr, sStatPointsText_CloseParen);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sStatPointsText_Exit);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 6;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_StatPoints_SelectStat(void)
{
    u8 statIndex = (u8)gSpecialVar_0x8005;
    if (statIndex >= 6)
    {
        gSpecialVar_Result = 1; // Esci
        return;
    }

    sStatPointsSelectedStat = statIndex;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 baseStat = (statIndex == STAT_HP) ? GetSpeciesBaseHP(species) : GetSpeciesBaseStat(species, statIndex);
    u8 customBonus = gSaveBlock2Ptr->customStatPoints[statIndex];

    StringCopy(gStringVar1, sStatPointNames[statIndex]);
    ConvertIntToDecimalStringN(gStringVar2, baseStat + customBonus, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, customBonus, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = 0;
}

void Script_StatPoints_PushAmountChoices(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 badges = GetPlayerBadgesCount();
    u16 totalEarned = badges * 20;
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;
    u16 remainingEarned = (totalEarned > totalUsed) ? (totalEarned - totalUsed) : 0;
    u16 maxCanAdd = (effectiveBST < 500) ? (500 - effectiveBST) : 0;
    u16 availableToAdd = (remainingEarned < maxCanAdd) ? remainingEarned : maxCanAdd;
    u8 customBonus = gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat];

    static const u8 sAddAmounts[] = { 1, 5, 10, 20 };
    for (int i = 0; i < ARRAY_COUNT(sAddAmounts); i++)
    {
        u8 amt = sAddAmounts[i];
        if (availableToAdd >= amt)
        {
            u8 *buf = Alloc(32);
            u8 *ptr = StringCopy(buf, sStatPointsText_Plus);
            ptr = ConvertIntToDecimalStringN(ptr, amt, STR_CONV_MODE_LEFT_ALIGN, 2);
            ptr = StringAppend(ptr, sStatPointsText_Points);
            struct ListMenuItem item;
            item.name = buf;
            item.id = amt;
            MultichoiceDynamic_PushElement(item);
        }
    }

    if (availableToAdd > 0 && availableToAdd != 1 && availableToAdd != 5 && availableToAdd != 10 && availableToAdd != 20)
    {
        u8 *bufAll = Alloc(32);
        u8 *ptr = StringCopy(bufAll, sStatPointsText_AllRemaining);
        ptr = ConvertIntToDecimalStringN(ptr, availableToAdd, STR_CONV_MODE_LEFT_ALIGN, 3);
        ptr = StringAppend(ptr, sStatPointsText_CloseParen);
        struct ListMenuItem allItem;
        allItem.name = bufAll;
        allItem.id = 999;
        MultichoiceDynamic_PushElement(allItem);
    }

    // Sottrazioni / Riassegnazioni
    static const u8 sSubAmounts[] = { 1, 5, 10, 20 };
    for (int i = 0; i < ARRAY_COUNT(sSubAmounts); i++)
    {
        u8 amt = sSubAmounts[i];
        if (customBonus >= amt)
        {
            u8 *buf = Alloc(32);
            u8 *ptr = StringCopy(buf, sStatPointsText_Minus);
            ptr = ConvertIntToDecimalStringN(ptr, amt, STR_CONV_MODE_LEFT_ALIGN, 2);
            ptr = StringAppend(ptr, sStatPointsText_Points);
            struct ListMenuItem item;
            item.name = buf;
            item.id = -(s32)amt;
            MultichoiceDynamic_PushElement(item);
        }
    }

    if (customBonus > 0 && customBonus != 1 && customBonus != 5 && customBonus != 10 && customBonus != 20)
    {
        u8 *bufReset = Alloc(32);
        StringCopy(bufReset, sStatPointsText_Reset);
        struct ListMenuItem resetItem;
        resetItem.name = bufReset;
        resetItem.id = -999;
        MultichoiceDynamic_PushElement(resetItem);
    }

    {
        u8 *bufBack = Alloc(32);
        StringCopy(bufBack, sStatPointsText_Back);
        struct ListMenuItem backItem;
        backItem.name = bufBack;
        backItem.id = 0;
        MultichoiceDynamic_PushElement(backItem);
    }
}

void Script_StatPoints_Apply(void)
{
    s32 delta = (s32)gSpecialVar_0x8005;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 badges = GetPlayerBadgesCount();
    u16 totalEarned = badges * 20;
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;
    u16 remainingEarned = (totalEarned > totalUsed) ? (totalEarned - totalUsed) : 0;
    u16 maxCanAdd = (effectiveBST < 500) ? (500 - effectiveBST) : 0;
    u16 availableToAdd = (remainingEarned < maxCanAdd) ? remainingEarned : maxCanAdd;
    u8 currentBonus = gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat];

    if (delta == 999)
    {
        gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat] += (u8)availableToAdd;
    }
    else if (delta == -999)
    {
        gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat] = 0;
    }
    else if (delta > 0)
    {
        u16 toAdd = (u16)delta;
        if (toAdd > availableToAdd)
            toAdd = availableToAdd;
        gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat] += (u8)toAdd;
    }
    else if (delta < 0)
    {
        u16 toSub = (u16)(-delta);
        if (toSub > currentBonus)
            toSub = currentBonus;
        gSaveBlock2Ptr->customStatPoints[sStatPointsSelectedStat] -= (u8)toSub;
    }

    CalculateMonStats(mon);
}

// --- LOGICA MOVE DRAFT (5 MOSSE RANDOM) ---

static u16 sDraftedMoves[5];

void Script_MoveDraft_Roll(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u16 monMoves[4];
    for (int i = 0; i < 4; i++)
        monMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);

    for (int i = 0; i < 5; i++)
    {
        u16 move;
        bool32 valid;
        int tries = 0;
        do {
            valid = TRUE;
            move = (Random() % (MOVES_COUNT - 1)) + 1;
            if (move == MOVE_NONE || move == MOVE_STRUGGLE || move >= MOVES_COUNT)
                valid = FALSE;
            else if (gMovesInfo[move].name[0] == 0 || gMovesInfo[move].name[0] == '-')
                valid = FALSE;
            
            for (int m = 0; m < 4; m++)
            {
                if (monMoves[m] == move)
                {
                    valid = FALSE;
                    break;
                }
            }

            for (int d = 0; d < i; d++)
            {
                if (sDraftedMoves[d] == move)
                {
                    valid = FALSE;
                    break;
                }
            }
            tries++;
        } while (!valid && tries < 3000);

        sDraftedMoves[i] = move;

        u8 *buf = Alloc(64);
        StringCopy(buf, gMovesInfo[move].name);
        struct ListMenuItem item;
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, sDraftText_Cancel);
        struct ListMenuItem cancelItem;
        cancelItem.name = bufCancel;
        cancelItem.id = 5;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_MoveDraft_Select(void)
{
    u8 choice = (u8)gSpecialVar_0x8005;
    if (choice >= 5)
    {
        gSpecialVar_Result = 0;
        return;
    }
    u16 move = sDraftedMoves[choice];
    gSpecialVar_0x8005 = move;
    gSpecialVar_Result = move;
}


