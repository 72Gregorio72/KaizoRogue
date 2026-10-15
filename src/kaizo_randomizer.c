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
#include "battle_main.h"
#include "roguelike_rarity.h"
#include "window.h"
#include "menu.h"
#include "constants/opponents_frlg.h"
#include "money.h"
#include "constants/map_groups.h"
#include "pokemon_storage_system.h"

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
void Script_BufferItemRarity(void);

// --- LOGICA ITEM RANDOMIZER ---
u16 GetKaizoRandomizedItem(u16 originalItem)
{
    if (originalItem == ITEM_NONE || originalItem >= ITEMS_COUNT)
        return originalItem;

    // Se è uno Strumento Base (Key Item come Bici, Mappa, ecc.) o una MN (HM01-HM08), non toccarlo
    if (gItemsInfo[originalItem].pocket == POCKET_KEY_ITEMS)
        return originalItem;

    if (originalItem >= ITEM_HM01 && originalItem <= ITEM_HM08)
        return originalItem;

    u32 seed = gSaveBlock2Ptr->randomizerSeed;
    if (seed == 0)
        seed = 0x54321678;

    // Rimescolamento deterministico basato su seed, mappa e item
    u32 itemSeed = seed ^ (originalItem * 7919)
                        ^ (gSaveBlock1Ptr->location.mapGroup << 16)
                        ^ (gSaveBlock1Ptr->location.mapNum << 8);

    if (GetRarityItemsCount() > 0)
    {
        u32 rng = itemSeed;
        u8 tier = RollRarityTier(CATEGORY_ITEM, LOOT_CONTEXT_STANDARD, &rng);
        return GetRandomRarityItemByTier(tier, &rng);
    }

    if (ARRAY_COUNT(sKaizoItemPool) == 0)
        return originalItem;

    u32 rng = itemSeed;
    rng = 1103515245 * rng + 12345;
    u16 index = (rng >> 16) % ARRAY_COUNT(sKaizoItemPool);

    return sKaizoItemPool[index];
}

void Script_BufferItemRarity(void)
{
    u16 item = VarGet(VAR_0x8004);
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        item = VarGet(VAR_0x8006);

    u8 tier = GetItemRarityTier(item);
    if (tier == RARITY_NONE)
        tier = RARITY_COMMON;

    StringCopy(gStringVar1, GetRarityColoredName(tier));
    LoadRarityFramePalette(tier, DLG_WINDOW_PALETTE_NUM);
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

static const u8 sStarterValidTypes[] = {
    TYPE_NORMAL,
    TYPE_FIGHTING,
    TYPE_FLYING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_ROCK,
    TYPE_BUG,
    TYPE_GHOST,
    TYPE_STEEL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_PSYCHIC,
    TYPE_ICE,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_FAIRY,
};

static EWRAM_DATA u8 sOakDraftTypes[3] = {0};
static EWRAM_DATA bool8 sOakTypesGenerated = FALSE;
static EWRAM_DATA u16 sStarterDraftMoves[3] = {0};
static EWRAM_DATA bool8 sStarterMovesGenerated = FALSE;
static EWRAM_DATA bool8 sDraftMovesGenerated = FALSE;
static EWRAM_DATA u8 sDraftMovesCity = 0;
static EWRAM_DATA u16 sDraftedMoves[5] = {0};
static EWRAM_DATA bool8 sDraftNaturesGenerated = FALSE;
static EWRAM_DATA u8 sDraftedNatures[5] = {0};
static EWRAM_DATA bool8 sDraftAbilitiesGenerated = FALSE;
static EWRAM_DATA u16 sDraftedAbilities[5] = {0};

static EWRAM_DATA u8 sOakRolledTier = 0;

static EWRAM_DATA u8 sStarterRerollsRemaining = 0;
static EWRAM_DATA u16 sLastRunLevel = 0;
static EWRAM_DATA u16 sLastRunBadges = 0;
static EWRAM_DATA u32 sLastRunShardsEarned = 0;
static EWRAM_DATA u8 sShardsWindowId = 0;
static EWRAM_DATA bool8 sShardsWindowOpen = FALSE;

void GenerateStartersForTypeAndTier(u8 chosenType, u8 tier)
{
    u32 rng = Random32();
    if (rng == 0)
        rng = 0x54321678;

    for (int i = 0; i < 3; i++)
    {
        u16 mon;
        bool32 duplicate;
        int tries = 0;
        do {
            duplicate = FALSE;
            if (tries < 250 && tier != RARITY_NONE)
                mon = GetRandomRaritySpeciesByTier(tier, &rng);
            else if (GetRarityPokemonCount() > 0)
                mon = GetRandomRaritySpecies(&rng);
            else
            {
                rng = 1103515245 * rng + 12345;
                mon = 1 + ((rng >> 16) % (NUM_SPECIES - 1));
            }

            if (!IsValidKaizoSpecies(mon) || !IsSpeciesInRarityPool(mon))
            {
                duplicate = TRUE;
                tries++;
                continue;
            }

            if (tries < 250 && tier != RARITY_NONE && GetSpeciesRarityTier(mon) != tier)
            {
                duplicate = TRUE;
                tries++;
                continue;
            }

            if (chosenType != TYPE_NONE)
            {
                if (gSpeciesInfo[mon].types[0] != chosenType && gSpeciesInfo[mon].types[1] != chosenType)
                {
                    duplicate = TRUE;
                    tries++;
                    continue;
                }
            }

            for (int j = 0; j < i; j++)
            {
                if (gSaveBlock2Ptr->randomStarters[j] == mon)
                    duplicate = TRUE;
            }
            tries++;
        } while (duplicate && tries < 5000);

        gSaveBlock2Ptr->randomStarters[i] = mon;
    }
}

void GenerateStartersForType(u8 chosenType)
{
    if (sOakRolledTier == RARITY_NONE)
        sOakRolledTier = RARITY_COMMON;
    GenerateStartersForTypeAndTier(chosenType, sOakRolledTier);
}

void InitKaizoMetaProgression(void)
{
    if (gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel == 0)
    {
        gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel = 1;
        gSaveBlock2Ptr->metaProgression.martStatPointsLevel = 0;
        gSaveBlock2Ptr->metaProgression.martUnlockIV = FALSE;
        gSaveBlock2Ptr->metaProgression.martUnlockEV = FALSE;
        gSaveBlock2Ptr->metaProgression.martUnlockNature = FALSE;
        gSaveBlock2Ptr->metaProgression.martUnlockAbility = FALSE;
        gSaveBlock2Ptr->metaProgression.starterRerollCount = 0;
        gSaveBlock2Ptr->metaProgression.starterMoveTierLevel = 0;
        gSaveBlock2Ptr->metaProgression.starterItemPouchLevel = 0;
        gSaveBlock2Ptr->metaProgression.lootLuckLevel = 0;
    }
}

void AddRogueShards(u32 amount)
{
    InitKaizoMetaProgression();
    gSaveBlock2Ptr->metaProgression.rogueShards += amount;
    gSaveBlock2Ptr->metaProgression.totalShardsEarned += amount;
}

bool8 RemoveRogueShards(u32 amount)
{
    InitKaizoMetaProgression();
    if (gSaveBlock2Ptr->metaProgression.rogueShards >= amount)
    {
        gSaveBlock2Ptr->metaProgression.rogueShards -= amount;
        return TRUE;
    }
    return FALSE;
}

void AwardTrainerDefeatRogueShards(u16 opponent)
{
    u32 shards = 5;
    if (opponent == TRAINER_LEADER_BROCK)
        shards = 100;
    else if (opponent == TRAINER_LEADER_MISTY)
        shards = 120;
    else if (opponent == TRAINER_LEADER_LT_SURGE)
        shards = 140;
    else if (opponent == TRAINER_LEADER_ERIKA)
        shards = 160;
    else if (opponent == TRAINER_LEADER_KOGA)
        shards = 180;
    else if (opponent == TRAINER_LEADER_SABRINA)
        shards = 200;
    else if (opponent == TRAINER_LEADER_BLAINE)
        shards = 220;
    else if (opponent == TRAINER_LEADER_GIOVANNI)
        shards = 250;
    else if (opponent >= TRAINER_ELITE_FOUR_LORELEI && opponent <= TRAINER_ELITE_FOUR_LANCE)
        shards = 250;
    else if (opponent >= TRAINER_CHAMPION_FIRST_SQUIRTLE && opponent <= TRAINER_CHAMPION_FIRST_CHARMANDER)
        shards = 400;

    AddRogueShards(shards);
}

void SetLastRunStats(u16 level, u16 badges, u32 shards)
{
    sLastRunLevel = level;
    sLastRunBadges = badges;
    sLastRunShardsEarned = shards;
}

void GenerateNewKaizoSeedAndStarters(void)
{
    InitKaizoMetaProgression();
    sStarterRerollsRemaining = gSaveBlock2Ptr->metaProgression.starterRerollCount;

    u32 entropy = REG_VCOUNT | (REG_TM1CNT_L << 16) | Random32();
    if (entropy == 0)
        entropy = 0x54321678;
    gSaveBlock2Ptr->randomizerSeed = entropy;
    for (int c = 0; c < 16; c++)
        gSaveBlock2Ptr->martBuildCityOptionUsed[c] = 0;
    gSaveBlock2Ptr->starterChosenType = TYPE_NONE;
    for (int s = 0; s < 6; s++)
        gSaveBlock2Ptr->customStatPoints[s] = 0;

    sOakRolledTier = RARITY_COMMON;
    sOakTypesGenerated = FALSE;
    for (int i = 0; i < 3; i++)
        sOakDraftTypes[i] = TYPE_NONE;

    sStarterMovesGenerated = FALSE;
    for (int i = 0; i < 3; i++)
        sStarterDraftMoves[i] = MOVE_NONE;

    sDraftMovesGenerated = FALSE;
    sDraftNaturesGenerated = FALSE;
    sDraftAbilitiesGenerated = FALSE;
    sDraftMovesCity = 0;
    for (int i = 0; i < 5; i++)
    {
        sDraftedMoves[i] = MOVE_NONE;
        sDraftedNatures[i] = 0;
        sDraftedAbilities[i] = ABILITY_NONE;
    }

    GenerateStartersForType(TYPE_NONE);
}

void Script_Oak_RollDraftTypes(void)
{
    if (!sOakTypesGenerated)
    {
        for (int i = 0; i < 3; i++)
        {
            u8 t;
            bool32 dup;
            do {
                dup = FALSE;
                t = sStarterValidTypes[Random() % ARRAY_COUNT(sStarterValidTypes)];
                for (int j = 0; j < i; j++)
                {
                    if (sOakDraftTypes[j] == t)
                        dup = TRUE;
                }
            } while (dup);
            sOakDraftTypes[i] = t;
        }
        sOakTypesGenerated = TRUE;
    }

    for (int i = 0; i < 3; i++)
    {
        u8 t = sOakDraftTypes[i];
        u8 *buf = Alloc(32);
        StringCopy(buf, gTypesInfo[t].name);
        struct ListMenuItem item;
        item.name = buf;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }

    if (sStarterRerollsRemaining > 0)
    {
        static const u8 sText_RerollTypes[] = _("{COLOR GREEN}Reroll Types (");
        static const u8 sText_CloseParen[] = _(")");
        u8 *bufReroll = Alloc(48);
        u8 *ptr = StringCopy(bufReroll, sText_RerollTypes);
        ConvertIntToDecimalStringN(gStringVar1, sStarterRerollsRemaining, STR_CONV_MODE_LEFT_ALIGN, 2);
        ptr = StringAppend(ptr, gStringVar1);
        StringAppend(ptr, sText_CloseParen);
        struct ListMenuItem rerollItem;
        rerollItem.name = bufReroll;
        rerollItem.id = 3;
        MultichoiceDynamic_PushElement(rerollItem);
    }
}

void Script_Oak_SelectType(void)
{
    u8 choice = (u8)gSpecialVar_0x8005;
    if (choice == 3 && sStarterRerollsRemaining > 0)
    {
        sStarterRerollsRemaining--;
        sOakTypesGenerated = FALSE;
        gSpecialVar_Result = 99; // signal reroll
        return;
    }

    if (choice >= 3)
        choice = 0;

    u8 type = sOakDraftTypes[choice];
    gSaveBlock2Ptr->starterChosenType = type;
    StringCopy(gStringVar1, gTypesInfo[type].name);

    u32 rng = Random32();
    if (rng == 0)
        rng = 0x54321678;

    sOakRolledTier = RollRarityTier(CATEGORY_POKEMON, LOOT_CONTEXT_STANDARD, &rng);
    StringCopy(gStringVar2, GetRarityColoredName(sOakRolledTier));

    GenerateStartersForTypeAndTier(type, sOakRolledTier);
    gSpecialVar_Result = choice;
}

void Script_Oak_RerollStarters(void)
{
    if (sStarterRerollsRemaining > 0)
        sStarterRerollsRemaining--;

    u8 type = gSaveBlock2Ptr->starterChosenType;
    if (type == TYPE_NONE)
        type = TYPE_FIRE;

    u32 rng = Random32();
    if (rng == 0)
        rng = 0x54321678;

    sOakRolledTier = RollRarityTier(CATEGORY_POKEMON, LOOT_CONTEXT_STANDARD, &rng);
    StringCopy(gStringVar2, GetRarityColoredName(sOakRolledTier));
    StringCopy(gStringVar1, gTypesInfo[type].name);
    ConvertIntToDecimalStringN(gStringVar3, sStarterRerollsRemaining, STR_CONV_MODE_LEFT_ALIGN, 2);

    GenerateStartersForTypeAndTier(type, sOakRolledTier);
}

void Script_Starter_GetRerollsRemaining(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sStarterRerollsRemaining, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sStarterRerollsRemaining, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_Result = sStarterRerollsRemaining;
}

void Script_Starter_ConsumeReroll(void)
{
    if (sStarterRerollsRemaining > 0)
        sStarterRerollsRemaining--;
    ConvertIntToDecimalStringN(gStringVar1, sStarterRerollsRemaining, STR_CONV_MODE_LEFT_ALIGN, 2);
    gSpecialVar_Result = sStarterRerollsRemaining;
}

void Script_Oak_BufferChosenTypeName(void)
{
    u8 type = gSaveBlock2Ptr->starterChosenType;
    if (type == TYPE_NONE)
        type = TYPE_FIRE;

    StringCopy(gStringVar1, gTypesInfo[type].name);
    StringCopy(gStringVar2, GetRarityColoredName(sOakRolledTier));
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
        GenerateStartersForType(gSaveBlock2Ptr->starterChosenType);

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
    
    // Lo starter parte con SOLO Graffio (Scratch)
    u16 moveScratch = MOVE_SCRATCH;
    u8 ppScratch = gMovesInfo[MOVE_SCRATCH].pp;
    u16 moveNone = MOVE_NONE;
    u8 ppZero = 0;

    SetMonData(&mon, MON_DATA_MOVE1, &moveScratch);
    SetMonData(&mon, MON_DATA_PP1, &ppScratch);
    SetMonData(&mon, MON_DATA_MOVE2, &moveNone);
    SetMonData(&mon, MON_DATA_PP2, &ppZero);
    SetMonData(&mon, MON_DATA_MOVE3, &moveNone);
    SetMonData(&mon, MON_DATA_PP3, &ppZero);
    SetMonData(&mon, MON_DATA_MOVE4, &moveNone);
    SetMonData(&mon, MON_DATA_PP4, &ppZero);

    // Starting item based on starterItemPouchLevel
    u16 heldItem = ITEM_NONE;
    if (gSaveBlock2Ptr->metaProgression.starterItemPouchLevel == 1)
        heldItem = ITEM_ORAN_BERRY;
    else if (gSaveBlock2Ptr->metaProgression.starterItemPouchLevel == 2)
        heldItem = ITEM_LUM_BERRY;
    else if (gSaveBlock2Ptr->metaProgression.starterItemPouchLevel >= 3)
        heldItem = ITEM_SITRUS_BERRY;

    if (heldItem != ITEM_NONE)
        SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);

    CalculateMonStats(&mon);
    GiveCapturedMonToPlayer(&mon);

    AddBagItem(ITEM_TOWN_MAP, 1);
    AddBagItem(ITEM_LIFT_KEY, 1);
    AddBagItem(ITEM_SILPH_SCOPE, 1);
    FlagSet(FLAG_CAN_USE_ROCKET_HIDEOUT_LIFT);
}

// --- DRAFT MOSSA ATTACCO INIZIALE STARTER (3 MOSSE RANDOM ATTACCO FISICO O SPECIALE) ---

void Script_Starter_RollAttackMoves(void)
{
    if (!sStarterMovesGenerated)
    {
        u32 rng = Random32();
        if (rng == 0)
            rng = 0x54321678;

        for (int i = 0; i < 3; i++)
        {
            u16 move;
            bool32 valid;
            int tries = 0;
            u8 rolledTier = RollRarityTier(CATEGORY_MOVE, LOOT_CONTEXT_STANDARD, &rng);
            if (i == 0)
            {
                if (gSaveBlock2Ptr->metaProgression.starterMoveTierLevel >= 2)
                    rolledTier = RARITY_RARE;
                else if (gSaveBlock2Ptr->metaProgression.starterMoveTierLevel == 1 && rolledTier == RARITY_COMMON)
                    rolledTier = RARITY_UNCOMMON;
            }
            do {
                valid = TRUE;
                if (tries < 250 && rolledTier != RARITY_NONE)
                    move = GetRandomRarityMoveByTier(rolledTier, &rng);
                else if (GetRarityMovesCount() > 0)
                    move = GetRandomRarityMove(&rng);
                else
                {
                    rng = 1103515245 * rng + 12345;
                    move = (rng >> 16) % (MOVES_COUNT - 1) + 1;
                }

                if (move == MOVE_NONE || move == MOVE_STRUGGLE || move == MOVE_SCRATCH || move >= MOVES_COUNT)
                {
                    valid = FALSE;
                }
                else if (!IsMoveInRarityPool(move))
                {
                    valid = FALSE;
                }
                else if (tries < 250 && rolledTier != RARITY_NONE && GetMoveRarityTier(move) != rolledTier)
                {
                    valid = FALSE;
                }
                else if (gMovesInfo[move].name[0] == 0 || gMovesInfo[move].name[0] == '-')
                {
                    valid = FALSE;
                }
                else if (gMovesInfo[move].category != DAMAGE_CATEGORY_PHYSICAL && gMovesInfo[move].category != DAMAGE_CATEGORY_SPECIAL)
                {
                    valid = FALSE;
                }
                else if (gMovesInfo[move].power == 0)
                {
                    valid = FALSE;
                }

                for (int d = 0; d < i; d++)
                {
                    if (sStarterDraftMoves[d] == move)
                    {
                        valid = FALSE;
                        break;
                    }
                }
                tries++;
            } while (!valid && tries < 5000);

            sStarterDraftMoves[i] = move;
        }
        sStarterMovesGenerated = TRUE;
    }

    for (int i = 0; i < 3; i++)
    {
        u16 m = sStarterDraftMoves[i];
        u8 tier = GetMoveRarityTier(m);
        u8 *buf = Alloc(64);
        StringCopy(buf, GetRarityColorPrefix(tier));
        StringAppend(buf, gMovesInfo[m].name);
        struct ListMenuItem item;
        item.name = buf;
        item.id = m;
        MultichoiceDynamic_PushElement(item);
    }
}

void Script_Starter_SelectAttackMove(void)
{
    u16 move = (u16)gSpecialVar_0x8005;
    if (move == MOVE_NONE || move >= MOVES_COUNT)
        move = sStarterDraftMoves[0];

    u8 pp = gMovesInfo[move].pp;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];

    SetMonData(mon, MON_DATA_MOVE2, &move);
    SetMonData(mon, MON_DATA_PP2, &pp);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar2);
    StringCopy(gStringVar1, gMovesInfo[move].name);
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

static EWRAM_DATA u8 sIvBoosterPartySlot = 0;
static EWRAM_DATA u8 sIvBoosterRemainingPoints = 0;
static EWRAM_DATA u8 sIvBoosterSelectedStat = 0;

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

static EWRAM_DATA u8 sEvManagerPartySlot = 0;
static EWRAM_DATA u16 sEvManagerRemainingPoints = 0;
static EWRAM_DATA u8 sEvManagerSelectedStat = 0;
static EWRAM_DATA u8 sEvManagerWorkingEVs[6] = {0};
static EWRAM_DATA u8 sEvManagerOriginalEVs[6] = {0};

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

static const u8 sDraftText_Cancel[] = _("Cancel");

// --- NPC CAMBIO NATURA DRAFT ---
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

void Script_NatureChanger_Roll(void)
{
    if (!sDraftNaturesGenerated)
    {
        u8 slot = VarGet(VAR_0x8004);
        if (slot >= PARTY_SIZE)
            slot = 0;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
        u8 currentNature = GetMonData(mon, MON_DATA_HIDDEN_NATURE);
        if (currentNature >= NUM_NATURES)
            currentNature = GetNature(mon);

        for (int i = 0; i < 5; i++)
        {
            u8 nat;
            bool32 valid;
            int tries = 0;
            do {
                valid = TRUE;
                nat = Random() % NUM_NATURES;
                if (nat == currentNature)
                    valid = FALSE;
                for (int d = 0; d < i; d++)
                {
                    if (sDraftedNatures[d] == nat)
                    {
                        valid = FALSE;
                        break;
                    }
                }
                tries++;
            } while (!valid && tries < 200);
            sDraftedNatures[i] = nat;
        }
        sDraftNaturesGenerated = TRUE;
    }

    static const u8 sStatAbbrevNames[6][8] = {
        _("HP"),
        _("Atk"),
        _("Def"),
        _("Speed"),
        _("Sp.Atk"),
        _("Sp.Def")
    };
    static const u8 sNatureText_Neutral[]       = _(" (Neutral)");
    static const u8 sNatureText_OpenParenPlus[] = _(" (+");
    static const u8 sNatureText_SpaceMinus[]    = _(" -");
    static const u8 sNatureText_CloseParen[]    = _(")");

    for (int i = 0; i < 5; i++)
    {
        u8 nat = sDraftedNatures[i];
        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, gNaturesInfo[nat].name);
        if (gNaturesInfo[nat].statUp == gNaturesInfo[nat].statDown)
        {
            ptr = StringAppend(ptr, sNatureText_Neutral);
        }
        else
        {
            ptr = StringAppend(ptr, sNatureText_OpenParenPlus);
            ptr = StringAppend(ptr, sStatAbbrevNames[gNaturesInfo[nat].statUp]);
            ptr = StringAppend(ptr, sNatureText_SpaceMinus);
            ptr = StringAppend(ptr, sStatAbbrevNames[gNaturesInfo[nat].statDown]);
            ptr = StringAppend(ptr, sNatureText_CloseParen);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = nat;
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, sDraftText_Cancel);
        struct ListMenuItem cancelItem;
        cancelItem.name = bufCancel;
        cancelItem.id = NUM_NATURES;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_NatureChanger_Select(void)
{
    u8 choice = (u8)gSpecialVar_0x8005;
    if (choice >= NUM_NATURES)
    {
        gSpecialVar_Result = 0xFF;
        return;
    }
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    u8 hiddenNature = choice;
    SetMonData(mon, MON_DATA_HIDDEN_NATURE, &hiddenNature);
    CalculateMonStats(mon);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    StringCopy(gStringVar2, gNaturesInfo[choice].name);
    gSpecialVar_Result = choice;
}

void Script_NatureChanger_Apply(void)
{
    Script_NatureChanger_Select();
}

// --- NPC CAMBIO ABILITÀ DRAFT ---
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

void Script_AbilityChanger_Roll(void)
{
    if (!sDraftAbilitiesGenerated)
    {
        u8 slot = VarGet(VAR_0x8004);
        if (slot >= PARTY_SIZE)
            slot = 0;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
        enum Ability currentAbility = GetMonAbility(mon);

        for (int i = 0; i < 5; i++)
        {
            enum Ability newAbility;
            bool32 valid;
            int tries = 0;
            do {
                valid = TRUE;
                newAbility = (Random() % (ABILITIES_COUNT - 1)) + 1;
                if (newAbility == currentAbility || gAbilitiesInfo[newAbility].name[0] == '-' || gAbilitiesInfo[newAbility].name[0] == 0)
                    valid = FALSE;
                for (int d = 0; d < i; d++)
                {
                    if (sDraftedAbilities[d] == newAbility)
                    {
                        valid = FALSE;
                        break;
                    }
                }
                tries++;
            } while (!valid && tries < 3000);
            sDraftedAbilities[i] = newAbility;
        }
        sDraftAbilitiesGenerated = TRUE;
    }

    for (int i = 0; i < 5; i++)
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, gAbilitiesInfo[sDraftedAbilities[i]].name);
        struct ListMenuItem item;
        item.name = buf;
        item.id = sDraftedAbilities[i];
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, sDraftText_Cancel);
        struct ListMenuItem cancelItem;
        cancelItem.name = bufCancel;
        cancelItem.id = ABILITY_NONE;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_AbilityChanger_Select(void)
{
    u16 ability = (u16)gSpecialVar_0x8005;
    if (ability == ABILITY_NONE || ability >= ABILITIES_COUNT)
    {
        gSpecialVar_Result = 0xFF;
        return;
    }
    u8 slot = VarGet(VAR_0x8004);
    if (slot >= PARTY_SIZE)
        slot = 0;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    u16 customAbility = ability;
    SetMonData(mon, MON_DATA_CUSTOM_ABILITY, &customAbility);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    StringCopy(gStringVar2, gAbilitiesInfo[ability].name);
    gSpecialVar_Result = ability;
}

void Script_AbilityChanger_Apply(void)
{
    Script_AbilityChanger_Select();
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

static bool8 IsMartOptionUnlocked(u8 option)
{
    switch (option)
    {
    case 0: return TRUE; // Stat Points always available
    case 1: return TRUE; // Move Draft always available
    case 2: return gSaveBlock2Ptr->metaProgression.martUnlockIV;
    case 3: return gSaveBlock2Ptr->metaProgression.martUnlockEV;
    case 4: return gSaveBlock2Ptr->metaProgression.martUnlockNature;
    case 5: return gSaveBlock2Ptr->metaProgression.martUnlockAbility;
    default: return FALSE;
    }
}

void Script_MartBuild_CheckCanUse(void)
{
    InitKaizoMetaProgression();
    u8 city = GetCurrentCityMartIndex();
    u8 usedMask = gSaveBlock2Ptr->martBuildCityOptionUsed[city];
    u8 availableMask = 0;
    for (u8 i = 0; i < 6; i++)
    {
        if (IsMartOptionUnlocked(i))
            availableMask |= (1 << i);
    }
    if ((usedMask & availableMask) == availableMask)
        gSpecialVar_Result = 0; // All unlocked options already used for this city
    else
        gSpecialVar_Result = 1; // Available
}

void Script_MartBuild_Consume(void)
{
    u8 city = GetCurrentCityMartIndex();
    gSaveBlock2Ptr->martBuildCityOptionUsed[city] = 0x3F;
}

static void Script_MartDraft_Reset(void)
{
    sDraftMovesGenerated = FALSE;
    sDraftNaturesGenerated = FALSE;
    sDraftAbilitiesGenerated = FALSE;
    for (int i = 0; i < 5; i++)
    {
        sDraftedMoves[i] = MOVE_NONE;
        sDraftedNatures[i] = 0;
        sDraftedAbilities[i] = ABILITY_NONE;
    }
}

void Script_MartBuild_InitSession(void)
{
    u8 city = GetCurrentCityMartIndex() + 1;
    if (sDraftMovesCity != city)
    {
        Script_MartDraft_Reset();
        sDraftMovesCity = city;
    }
}

static const u8 sMartMenuText_StatPoints[]      = _("Stat Points");
static const u8 sMartMenuText_MoveDraft[]       = _("Draft Random Move");
static const u8 sMartMenuText_IVBooster[]       = _("Modify IVs");
static const u8 sMartMenuText_EVManager[]       = _("Train EVs");
static const u8 sMartMenuText_NatureChanger[]   = _("Change Nature");
static const u8 sMartMenuText_AbilityChanger[]  = _("Change Ability");
static const u8 sMartMenuText_Exit[]            = _("Done / Exit");

static const u8 sMartMenuText_RedPrefix[]       = _("{COLOR RED}{SHADOW LIGHT_RED}");
static const u8 sMartMenuText_UsedSuffix[]      = _(" (Used)");

void Script_MartBuild_PushMainMenuChoices(void)
{
    InitKaizoMetaProgression();
    static const u8 *const sOptionNames[] = {
        sMartMenuText_StatPoints,
        sMartMenuText_MoveDraft,
        sMartMenuText_IVBooster,
        sMartMenuText_EVManager,
        sMartMenuText_NatureChanger,
        sMartMenuText_AbilityChanger,
    };
    u8 city = GetCurrentCityMartIndex();
    u8 usedMask = gSaveBlock2Ptr->martBuildCityOptionUsed[city];

    for (u32 i = 0; i < ARRAY_COUNT(sOptionNames); i++)
    {
        if (!IsMartOptionUnlocked(i))
            continue;

        u8 *buf = Alloc(64);
        if (usedMask & (1 << i))
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
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sMartMenuText_Exit);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 6;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_MartBuild_SelectOption(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    u8 city = GetCurrentCityMartIndex();
    if (option < 6 && (gSaveBlock2Ptr->martBuildCityOptionUsed[city] & (1 << option)))
    {
        gSpecialVar_Result = 100; // Already used
        return;
    }
    gSpecialVar_Result = option;
}

void Script_MartBuild_MarkOptionUsed(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    u8 city = GetCurrentCityMartIndex();
    if (option < 6)
        gSaveBlock2Ptr->martBuildCityOptionUsed[city] |= (1 << option);
}

void Script_MartBuild_ExitSession(void)
{
    gSpecialVar_Result = 0;
}

// --- LOGIC STAT POINTS (+20 PER BADGE UP TO 500 BST) ---

static EWRAM_DATA u8 sStatPointsPartySlot = 0;
static EWRAM_DATA u8 sStatPointsSelectedStat = 0;

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

static u16 GetStatPointsPerBadge(void)
{
    switch (gSaveBlock2Ptr->metaProgression.martStatPointsLevel)
    {
    case 1:  return 15;
    case 2:  return 20;
    default: return 10;
    }
}

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
    u16 totalEarned = badges * GetStatPointsPerBadge();
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, effectiveBST, STR_CONV_MODE_LEFT_ALIGN, 3);

    CalculateMonStats(mon);

    if (naturalBST >= 500 || effectiveBST >= 500)
    {
        gSpecialVar_Result = 1; // BST naturale >= 500 o BST effettivo ha già raggiunto/superato 500
        return;
    }

    if (totalEarned <= totalUsed && totalUsed == 0)
    {
        ConvertIntToDecimalStringN(gStringVar3, badges, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_Result = 2; // Nessun punto da distribuire
        return;
    }

    u16 remainingEarned = (totalEarned > totalUsed) ? (totalEarned - totalUsed) : 0;
    u16 maxCanAdd = 500 - effectiveBST;
    u16 availableToAdd = (remainingEarned < maxCanAdd) ? remainingEarned : maxCanAdd;

    if (availableToAdd == 0 && totalUsed == 0)
    {
        ConvertIntToDecimalStringN(gStringVar3, badges, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_Result = 2; // Nessun punto da distribuire
        return;
    }

    ConvertIntToDecimalStringN(gStringVar3, availableToAdd, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = 0; // Pronto per distribuire / modificare
}

void Script_StatPoints_GetRemainingPoints(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStatPointsPartySlot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u16 badges = GetPlayerBadgesCount();
    u16 totalEarned = badges * GetStatPointsPerBadge();
    u16 totalUsed = GetTotalCustomStatPointsAllocated();
    u16 naturalBST = GetSpeciesBaseStatTotal(species);
    u16 effectiveBST = naturalBST + totalUsed;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, effectiveBST, STR_CONV_MODE_LEFT_ALIGN, 3);

    u16 remainingEarned = (totalEarned > totalUsed) ? (totalEarned - totalUsed) : 0;
    u16 maxCanAdd = (effectiveBST < 500 && naturalBST < 500) ? (500 - effectiveBST) : 0;
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
    u16 maxCanAdd = (effectiveBST < 500 && naturalBST < 500) ? (500 - effectiveBST) : 0;
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
    u16 maxCanAdd = (effectiveBST < 500 && naturalBST < 500) ? (500 - effectiveBST) : 0;
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

void Script_MoveDraft_Roll(void)
{
    InitKaizoMetaProgression();
    u8 maxChoices = gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel;
    if (maxChoices < 1)
        maxChoices = 1;
    if (maxChoices > 5)
        maxChoices = 5;

    if (!sDraftMovesGenerated)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
        u16 monMoves[4];
        for (int i = 0; i < 4; i++)
            monMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);

        for (int i = 0; i < maxChoices; i++)
        {
            u16 move;
            bool32 valid;
            int tries = 0;
            u8 rolledTier = RollRarityTier(CATEGORY_MOVE, LOOT_CONTEXT_MART, NULL);
            do {
                valid = TRUE;
                if (tries < 250 && rolledTier != RARITY_NONE)
                    move = GetRandomRarityMoveByTier(rolledTier, NULL);
                else if (GetRarityMovesCount() > 0)
                    move = GetRandomRarityMove(NULL);
                else
                    move = (Random() % (MOVES_COUNT - 1)) + 1;

                if (move == MOVE_NONE || move == MOVE_STRUGGLE || move >= MOVES_COUNT)
                    valid = FALSE;
                else if (!IsMoveInRarityPool(move))
                    valid = FALSE;
                else if (tries < 250 && rolledTier != RARITY_NONE && GetMoveRarityTier(move) != rolledTier)
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
        }
        sDraftMovesGenerated = TRUE;
    }

    for (int i = 0; i < maxChoices; i++)
    {
        u16 m = sDraftedMoves[i];
        u8 tier = GetMoveRarityTier(m);
        u8 *buf = Alloc(64);
        StringCopy(buf, GetRarityColorPrefix(tier));
        StringAppend(buf, gMovesInfo[m].name);
        struct ListMenuItem item;
        item.name = buf;
        item.id = m;
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, sDraftText_Cancel);
        struct ListMenuItem cancelItem;
        cancelItem.name = bufCancel;
        cancelItem.id = MOVE_NONE;
        MultichoiceDynamic_PushElement(cancelItem);
    }
}

void Script_MoveDraft_Select(void)
{
    u16 move = (u16)gSpecialVar_0x8005;
    if (move == MOVE_NONE || move >= MOVES_COUNT)
    {
        gSpecialVar_Result = 0;
        return;
    }
    gSpecialVar_0x8005 = move;
    gSpecialVar_Result = move;
}

// --- HUB META UPGRADES MENUS & SPECIALS ---

void Script_Meta_Init(void)
{
    InitKaizoMetaProgression();
}

void Script_Meta_GetShards(void)
{
    InitKaizoMetaProgression();
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock2Ptr->metaProgression.rogueShards, STR_CONV_MODE_LEFT_ALIGN, 6);
    gSpecialVar_Result = gSaveBlock2Ptr->metaProgression.rogueShards;
}

static const u8 sText_ShardsHeader[] = _("SHARDS: ");

static const u8 sText_DraftMax[] = _("Draft: 5 Mosse (MAX)");
static const u8 sText_DraftPrefix[] = _("Draft: ");
static const u8 sText_Arrow[] = _("->");
static const u8 sText_MosseShards[] = _(" Mosse (");
static const u8 sText_ShardsClose[] = _(" Shards)");

static const u8 sText_StatPointsMax[] = _("Stat Points: +20 BST (MAX)");
static const u8 sText_StatPrefix[] = _("Stat: +");
static const u8 sText_ArrowPlus[] = _("->+");
static const u8 sText_BstShards[] = _(" BST (");

static const u8 sText_LicenzaIV_Done[] = _("Licenza IV (Sbloccata)");
static const u8 sText_LicenzaIV_Buy[] = _("Licenza IV Booster (80 Shards)");
static const u8 sText_LicenzaEV_Done[] = _("Licenza EV (Sbloccata)");
static const u8 sText_LicenzaEV_Buy[] = _("Licenza EV Trainer (100 Shards)");
static const u8 sText_LicenzaNature_Done[] = _("Licenza Natura (Sbloccata)");
static const u8 sText_LicenzaNature_Buy[] = _("Licenza Natura (120 Shards)");
static const u8 sText_LicenzaAbility_Done[] = _("Licenza Abilita' (Sbloccata)");
static const u8 sText_LicenzaAbility_Buy[] = _("Licenza Abilita' (180 Shards)");
static const u8 sText_Indietro[] = _("Indietro");

static const u8 sText_RerollStarterMax[] = _("Reroll Starter: 2 (MAX)");
static const u8 sText_RerollPrefix[] = _("Reroll: ");
static const u8 sText_ParenShards[] = _(" (");

static const u8 sText_MovesRareMax[] = _("Mosse: 1 Rare (MAX)");
static const u8 sText_MovesRareBuy[] = _("Mosse: 1 Rare (300 Shards)");
static const u8 sText_MovesUncommonBuy[] = _("Mosse: 1 Uncommon (120 Shards)");

static const u8 sText_PouchSitrusMax[] = _("Tasca: Baccasugna (MAX)");
static const u8 sText_PouchSitrusBuy[] = _("Tasca: Baccasugna (300 Shards)");
static const u8 sText_PouchLumBuy[] = _("Tasca: Baccaprugna (150 Shards)");
static const u8 sText_PouchOranBuy[] = _("Tasca: Baccacedro (60 Shards)");

static const u8 sText_FortuneMax[] = _("Fortuna Drop: Liv. 3 (MAX)");
static const u8 sText_FortunePrefix[] = _("Fortuna: Liv. ");

void Script_Meta_ShowShardsWindow(void)
{
    InitKaizoMetaProgression();
    if (!sShardsWindowOpen)
    {
        struct WindowTemplate template;
        SetWindowTemplateFields(&template, 0, 1, 1, 11, 2, 0xF, 0x141);
        sShardsWindowId = AddWindow(&template);
        FillWindowPixelBuffer(sShardsWindowId, PIXEL_FILL(0));
        PutWindowTilemap(sShardsWindowId);
        DrawStdFrameWithCustomTileAndPalette(sShardsWindowId, FALSE, 0x214, 0xE);
        sShardsWindowOpen = TRUE;
    }
    else
    {
        FillWindowPixelBuffer(sShardsWindowId, PIXEL_FILL(0));
    }

    u8 text[32];
    u8 *ptr = StringCopy(text, sText_ShardsHeader);
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock2Ptr->metaProgression.rogueShards, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringAppend(ptr, gStringVar1);
    AddTextPrinterParameterized(sShardsWindowId, FONT_NORMAL, text, 4, 1, 0, NULL);
    CopyWindowToVram(sShardsWindowId, COPYWIN_FULL);
}

void Script_Meta_HideShardsWindow(void)
{
    if (sShardsWindowOpen)
    {
        ClearStdWindowAndFrame(sShardsWindowId, TRUE);
        RemoveWindow(sShardsWindowId);
        sShardsWindowOpen = FALSE;
    }
}

void Script_Meta_GetLastRunRecap(void)
{
    if (sLastRunShardsEarned > 0)
    {
        ConvertIntToDecimalStringN(gStringVar1, sLastRunLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, sLastRunBadges, STR_CONV_MODE_LEFT_ALIGN, 2);
        ConvertIntToDecimalStringN(gStringVar3, sLastRunShardsEarned, STR_CONV_MODE_LEFT_ALIGN, 5);
        ConvertIntToDecimalStringN(gStringVar4, gSaveBlock2Ptr->metaProgression.rogueShards, STR_CONV_MODE_LEFT_ALIGN, 6);
        sLastRunShardsEarned = 0;
        gSpecialVar_Result = 1;
    }
    else
    {
        gSpecialVar_Result = 0;
    }
}

static const u16 sMartDraftCosts[] = { 0, 50, 120, 250, 500 };
static const u16 sMartStatCosts[]  = { 150, 350 };
static const u16 sStarterRerollCosts[] = { 100, 250 };
static const u16 sStarterMoveCosts[]   = { 120, 300 };
static const u16 sStarterItemCosts[]   = { 60, 150, 300 };
static const u16 sFortuneCosts[]       = { 100, 250, 450 };

void Script_Meta_PushMartChoices(void)
{
    InitKaizoMetaProgression();
    
    // 0: Move Draft Choices (Liv. 1..5)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel;
        if (cur >= 5)
        {
            StringCopy(buf, sText_DraftMax);
        }
        else
        {
            u16 cost = sMartDraftCosts[cur];
            ConvertIntToDecimalStringN(gStringVar1, cur, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar2, cur + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar3, cost, STR_CONV_MODE_LEFT_ALIGN, 4);
            u8 *ptr = StringCopy(buf, sText_DraftPrefix);
            ptr = StringAppend(ptr, gStringVar1);
            ptr = StringAppend(ptr, sText_Arrow);
            ptr = StringAppend(ptr, gStringVar2);
            ptr = StringAppend(ptr, sText_MosseShards);
            ptr = StringAppend(ptr, gStringVar3);
            StringAppend(ptr, sText_ShardsClose);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 0;
        MultichoiceDynamic_PushElement(item);
    }

    // 1: Stat Points Mastery (Liv. 0..2)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.martStatPointsLevel;
        if (cur >= 2)
        {
            StringCopy(buf, sText_StatPointsMax);
        }
        else
        {
            u16 cost = sMartStatCosts[cur];
            u16 curVal = (cur == 0) ? 10 : 15;
            u16 nextVal = (cur == 0) ? 15 : 20;
            ConvertIntToDecimalStringN(gStringVar1, curVal, STR_CONV_MODE_LEFT_ALIGN, 2);
            ConvertIntToDecimalStringN(gStringVar2, nextVal, STR_CONV_MODE_LEFT_ALIGN, 2);
            ConvertIntToDecimalStringN(gStringVar3, cost, STR_CONV_MODE_LEFT_ALIGN, 4);
            u8 *ptr = StringCopy(buf, sText_StatPrefix);
            ptr = StringAppend(ptr, gStringVar1);
            ptr = StringAppend(ptr, sText_ArrowPlus);
            ptr = StringAppend(ptr, gStringVar2);
            ptr = StringAppend(ptr, sText_BstShards);
            ptr = StringAppend(ptr, gStringVar3);
            StringAppend(ptr, sText_ShardsClose);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 1;
        MultichoiceDynamic_PushElement(item);
    }

    // 2: Licenza IV Booster (80 Shards)
    {
        u8 *buf = Alloc(64);
        if (gSaveBlock2Ptr->metaProgression.martUnlockIV)
            StringCopy(buf, sText_LicenzaIV_Done);
        else
            StringCopy(buf, sText_LicenzaIV_Buy);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 2;
        MultichoiceDynamic_PushElement(item);
    }

    // 3: Licenza EV Trainer (100 Shards)
    {
        u8 *buf = Alloc(64);
        if (gSaveBlock2Ptr->metaProgression.martUnlockEV)
            StringCopy(buf, sText_LicenzaEV_Done);
        else
            StringCopy(buf, sText_LicenzaEV_Buy);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 3;
        MultichoiceDynamic_PushElement(item);
    }

    // 4: Licenza Cambio Natura (120 Shards)
    {
        u8 *buf = Alloc(64);
        if (gSaveBlock2Ptr->metaProgression.martUnlockNature)
            StringCopy(buf, sText_LicenzaNature_Done);
        else
            StringCopy(buf, sText_LicenzaNature_Buy);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 4;
        MultichoiceDynamic_PushElement(item);
    }

    // 5: Licenza Cambio Abilita' (180 Shards)
    {
        u8 *buf = Alloc(64);
        if (gSaveBlock2Ptr->metaProgression.martUnlockAbility)
            StringCopy(buf, sText_LicenzaAbility_Done);
        else
            StringCopy(buf, sText_LicenzaAbility_Buy);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 5;
        MultichoiceDynamic_PushElement(item);
    }

    // 6: Esci
    {
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sText_Indietro);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 6;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_Meta_BuyMartUpgrade(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    gSpecialVar_Result = 0; // default 0: cancel / invalid

    switch (option)
    {
    case 0: // Move Draft
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel;
        if (cur >= 5) { gSpecialVar_Result = 3; return; }
        u16 cost = sMartDraftCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel++;
        gSpecialVar_Result = 1;
        break;
    }
    case 1: // Stat Points
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.martStatPointsLevel;
        if (cur >= 2) { gSpecialVar_Result = 3; return; }
        u16 cost = sMartStatCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martStatPointsLevel++;
        gSpecialVar_Result = 1;
        break;
    }
    case 2: // IV
    {
        if (gSaveBlock2Ptr->metaProgression.martUnlockIV) { gSpecialVar_Result = 3; return; }
        if (!RemoveRogueShards(80)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martUnlockIV = TRUE;
        gSpecialVar_Result = 1;
        break;
    }
    case 3: // EV
    {
        if (gSaveBlock2Ptr->metaProgression.martUnlockEV) { gSpecialVar_Result = 3; return; }
        if (!RemoveRogueShards(100)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martUnlockEV = TRUE;
        gSpecialVar_Result = 1;
        break;
    }
    case 4: // Nature
    {
        if (gSaveBlock2Ptr->metaProgression.martUnlockNature) { gSpecialVar_Result = 3; return; }
        if (!RemoveRogueShards(120)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martUnlockNature = TRUE;
        gSpecialVar_Result = 1;
        break;
    }
    case 5: // Ability
    {
        if (gSaveBlock2Ptr->metaProgression.martUnlockAbility) { gSpecialVar_Result = 3; return; }
        if (!RemoveRogueShards(180)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.martUnlockAbility = TRUE;
        gSpecialVar_Result = 1;
        break;
    }
    default:
        gSpecialVar_Result = 0;
        break;
    }
}

void Script_Meta_PushStarterChoices(void)
{
    InitKaizoMetaProgression();

    // 0: Reroll Starter & Tipi (Liv. 0..2)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.starterRerollCount;
        if (cur >= 2)
        {
            StringCopy(buf, sText_RerollStarterMax);
        }
        else
        {
            u16 cost = sStarterRerollCosts[cur];
            ConvertIntToDecimalStringN(gStringVar1, cur, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar2, cur + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar3, cost, STR_CONV_MODE_LEFT_ALIGN, 4);
            u8 *ptr = StringCopy(buf, sText_RerollPrefix);
            ptr = StringAppend(ptr, gStringVar1);
            ptr = StringAppend(ptr, sText_Arrow);
            ptr = StringAppend(ptr, gStringVar2);
            ptr = StringAppend(ptr, sText_ParenShards);
            ptr = StringAppend(ptr, gStringVar3);
            StringAppend(ptr, sText_ShardsClose);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 0;
        MultichoiceDynamic_PushElement(item);
    }

    // 1: Qualita' Mosse Starter (Liv. 0..2)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.starterMoveTierLevel;
        if (cur >= 2)
        {
            StringCopy(buf, sText_MovesRareMax);
        }
        else if (cur == 1)
        {
            StringCopy(buf, sText_MovesRareBuy);
        }
        else
        {
            StringCopy(buf, sText_MovesUncommonBuy);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 1;
        MultichoiceDynamic_PushElement(item);
    }

    // 2: Tasca Strumento Iniziale (Liv. 0..3)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.starterItemPouchLevel;
        if (cur >= 3)
        {
            StringCopy(buf, sText_PouchSitrusMax);
        }
        else if (cur == 2)
        {
            StringCopy(buf, sText_PouchSitrusBuy);
        }
        else if (cur == 1)
        {
            StringCopy(buf, sText_PouchLumBuy);
        }
        else
        {
            StringCopy(buf, sText_PouchOranBuy);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 2;
        MultichoiceDynamic_PushElement(item);
    }

    // 3: Indietro
    {
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sText_Indietro);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 3;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_Meta_BuyStarterUpgrade(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    gSpecialVar_Result = 0;

    switch (option)
    {
    case 0: // Rerolls
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.starterRerollCount;
        if (cur >= 2) { gSpecialVar_Result = 3; return; }
        u16 cost = sStarterRerollCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.starterRerollCount++;
        sStarterRerollsRemaining = gSaveBlock2Ptr->metaProgression.starterRerollCount;
        gSpecialVar_Result = 1;
        break;
    }
    case 1: // Move Quality
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.starterMoveTierLevel;
        if (cur >= 2) { gSpecialVar_Result = 3; return; }
        u16 cost = sStarterMoveCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.starterMoveTierLevel++;
        gSpecialVar_Result = 1;
        break;
    }
    case 2: // Item Pouch
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.starterItemPouchLevel;
        if (cur >= 3) { gSpecialVar_Result = 3; return; }
        u16 cost = sStarterItemCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.starterItemPouchLevel++;
        gSpecialVar_Result = 1;
        break;
    }
    default:
        gSpecialVar_Result = 0;
        break;
    }
}

void Script_Meta_PushFortuneChoices(void)
{
    InitKaizoMetaProgression();

    // 0: Loot Luck (Liv. 0..3)
    {
        u8 *buf = Alloc(64);
        u8 cur = gSaveBlock2Ptr->metaProgression.lootLuckLevel;
        if (cur >= 3)
        {
            StringCopy(buf, sText_FortuneMax);
        }
        else
        {
            u16 cost = sFortuneCosts[cur];
            ConvertIntToDecimalStringN(gStringVar1, cur, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar2, cur + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            ConvertIntToDecimalStringN(gStringVar3, cost, STR_CONV_MODE_LEFT_ALIGN, 4);
            u8 *ptr = StringCopy(buf, sText_FortunePrefix);
            ptr = StringAppend(ptr, gStringVar1);
            ptr = StringAppend(ptr, sText_Arrow);
            ptr = StringAppend(ptr, gStringVar2);
            ptr = StringAppend(ptr, sText_ParenShards);
            ptr = StringAppend(ptr, gStringVar3);
            StringAppend(ptr, sText_ShardsClose);
        }
        struct ListMenuItem item;
        item.name = buf;
        item.id = 0;
        MultichoiceDynamic_PushElement(item);
    }

    // 1: Indietro
    {
        u8 *bufExit = Alloc(32);
        StringCopy(bufExit, sText_Indietro);
        struct ListMenuItem exitItem;
        exitItem.name = bufExit;
        exitItem.id = 1;
        MultichoiceDynamic_PushElement(exitItem);
    }
}

void Script_Meta_BuyFortuneUpgrade(void)
{
    u8 option = (u8)gSpecialVar_0x8005;
    gSpecialVar_Result = 0;

    if (option == 0)
    {
        u8 cur = gSaveBlock2Ptr->metaProgression.lootLuckLevel;
        if (cur >= 3) { gSpecialVar_Result = 3; return; }
        u16 cost = sFortuneCosts[cur];
        if (!RemoveRogueShards(cost)) { gSpecialVar_Result = 2; return; }
        gSaveBlock2Ptr->metaProgression.lootLuckLevel++;
        gSpecialVar_Result = 1;
    }
}

static const u8 sText_Cheat_MaxAll[] = _("Sblocca Tutti Powerup");
static const u8 sText_Cheat_Money[] = _("Monete e Shards MAX");
static const u8 sText_Cheat_Reset[] = _("Reset Meta a Default");
static const u8 sText_Cheat_Exit[] = _("Esci");

void Script_Meta_PushCheatChoices(void)
{
    InitKaizoMetaProgression();

    // 0: Sblocca Tutti Powerup
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_Cheat_MaxAll);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 0;
        MultichoiceDynamic_PushElement(item);
    }

    // 1: Monete e Shards MAX
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_Cheat_Money);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 1;
        MultichoiceDynamic_PushElement(item);
    }

    // 2: Reset Meta a Default
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_Cheat_Reset);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 2;
        MultichoiceDynamic_PushElement(item);
    }

    // 3: Esci
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_Cheat_Exit);
        struct ListMenuItem item;
        item.name = buf;
        item.id = 3;
        MultichoiceDynamic_PushElement(item);
    }
}

void Script_Meta_CheatMaxAll(void)
{
    InitKaizoMetaProgression();
    gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel = 5;
    gSaveBlock2Ptr->metaProgression.martStatPointsLevel = 2;
    gSaveBlock2Ptr->metaProgression.martUnlockIV = TRUE;
    gSaveBlock2Ptr->metaProgression.martUnlockEV = TRUE;
    gSaveBlock2Ptr->metaProgression.martUnlockNature = TRUE;
    gSaveBlock2Ptr->metaProgression.martUnlockAbility = TRUE;

    gSaveBlock2Ptr->metaProgression.starterRerollCount = 2;
    sStarterRerollsRemaining = 2;
    gSaveBlock2Ptr->metaProgression.starterMoveTierLevel = 2;
    gSaveBlock2Ptr->metaProgression.starterItemPouchLevel = 3;

    gSaveBlock2Ptr->metaProgression.lootLuckLevel = 3;
}

void Script_Meta_CheatInfiniteMoneyAndShards(void)
{
    InitKaizoMetaProgression();
    gSaveBlock2Ptr->metaProgression.rogueShards = 999999;
    gSaveBlock2Ptr->metaProgression.totalShardsEarned += 999999;
    SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
}

void Script_Meta_CheatReset(void)
{
    gSaveBlock2Ptr->metaProgression.martDraftChoicesLevel = 1;
    gSaveBlock2Ptr->metaProgression.martStatPointsLevel = 0;
    gSaveBlock2Ptr->metaProgression.martUnlockIV = FALSE;
    gSaveBlock2Ptr->metaProgression.martUnlockEV = FALSE;
    gSaveBlock2Ptr->metaProgression.martUnlockNature = FALSE;
    gSaveBlock2Ptr->metaProgression.martUnlockAbility = FALSE;

    gSaveBlock2Ptr->metaProgression.starterRerollCount = 0;
    sStarterRerollsRemaining = 0;
    gSaveBlock2Ptr->metaProgression.starterMoveTierLevel = 0;
    gSaveBlock2Ptr->metaProgression.starterItemPouchLevel = 0;

    gSaveBlock2Ptr->metaProgression.lootLuckLevel = 0;
    gSaveBlock2Ptr->metaProgression.rogueShards = 0;
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    ResetPokemonStorageSystem();
}

void Script_Kaizo_PrepareNewRun(void)
{
    u8 removedCount = 0;
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            CopyMonToPC(&gParties[B_TRAINER_PLAYER][i]);
            removedCount++;
        }
    }
    ZeroPlayerPartyMons();
    gPartiesCount[B_TRAINER_PLAYER] = 0;

    GenerateNewKaizoSeedAndStarters();

    gSpecialVar_Result = removedCount;
}

void Script_CheckIsKaizoHub(void)
{
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_KAIZO_HUB)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_KAIZO_HUB))
    {
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}




