#include "global.h"
#include "main.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "strings.h"
#include "string_util.h"
#include "script_menu.h"
#include "script.h"
#include "malloc.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/party_menu.h"
#include "constants/event_objects.h"
#include "event_object_movement.h"
#include "data.h"
#include "daycare.h"
#include "item.h"
#include "move.h"
#include "battle_main.h"
#include "random.h"
#include "roguelike_rarity.h"
#include "kaizo_roster.h"

static EWRAM_DATA struct BoxPokemon *sChosenBuildMon = NULL;
static EWRAM_DATA bool8 sChosenMonIsParty = FALSE;
static EWRAM_DATA u8 sChosenPartySlot = 0;

static const u16 sTierStarterMoves[6][8] = {
    [RARITY_COMMON]    = { MOVE_QUICK_ATTACK, MOVE_WATER_GUN, MOVE_EMBER, MOVE_VINE_WHIP, MOVE_BITE, MOVE_DRAINING_KISS, MOVE_BULLET_SEED, MOVE_SHOCK_WAVE },
    [RARITY_UNCOMMON]  = { MOVE_AERIAL_ACE, MOVE_SWIFT, MOVE_WATER_PULSE, MOVE_MAGICAL_LEAF, MOVE_DRAIN_PUNCH, MOVE_FIRE_FANG, MOVE_THUNDER_FANG, MOVE_ICE_FANG },
    [RARITY_RARE]      = { MOVE_FLAMETHROWER, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_ENERGY_BALL, MOVE_EARTH_POWER, MOVE_SHADOW_BALL, MOVE_SCALD },
    [RARITY_EPIC]      = { MOVE_DRAGON_PULSE, MOVE_CLOSE_COMBAT, MOVE_MOONBLAST, MOVE_GUNK_SHOT, MOVE_FLASH_CANNON, MOVE_DARK_PULSE, MOVE_STONE_EDGE, MOVE_BUG_BUZZ },
    [RARITY_LEGENDARY] = { MOVE_JUDGMENT, MOVE_SPACIAL_REND, MOVE_ROAR_OF_TIME, MOVE_PSYSTRIKE, MOVE_SACRED_FIRE, MOVE_AEROBLAST, MOVE_ORIGIN_PULSE, MOVE_PRECIPICE_BLADES }
};

static const u8 sTeraTypes[] = {
    TYPE_NORMAL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_ICE,
    TYPE_FIGHTING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_FLYING,
    TYPE_PSYCHIC,
    TYPE_BUG,
    TYPE_ROCK,
    TYPE_GHOST,
    TYPE_DRAGON,
    TYPE_STEEL,
    TYPE_DARK,
    TYPE_FAIRY,
    TYPE_STELLAR
};

enum {
    BUILD_OPT_STARTER = 0,
    BUILD_OPT_NATURE,
    BUILD_OPT_ABILITY,
    BUILD_OPT_MOVE1,
    BUILD_OPT_MOVE2,
    BUILD_OPT_MEGA_STONE,
    BUILD_OPT_TERA,
    BUILD_OPT_SHINY,
    BUILD_OPT_SWITCH,
    BUILD_OPT_EXIT
};

static const u8 sText_BuildOpt_StarterSet[]    = _("Set as Run Starter");
static const u8 sText_BuildOpt_StarterRemove[] = _("Remove Run Starter");
static const u8 sText_BuildOpt_Nature[]        = _("Reroll Nature (5 Pts)");
static const u8 sText_BuildOpt_Ability[]       = _("Change Ability (10 Pts)");
static const u8 sText_BuildOpt_Move1[]         = _("Starter Move 1 (10 Pts)");
static const u8 sText_BuildOpt_Move2[]         = _("Starter Move 2 (10 Pts)");
static const u8 sText_BuildOpt_MegaStone[]     = _("Mega Stone (15 Pts)");
static const u8 sText_BuildOpt_Tera[]          = _("Change Tera Type (5 Pts)");
static const u8 sText_BuildOpt_SwitchMon[]     = _("Choose Other Pokémon");
static const u8 sText_BuildOpt_Exit[]          = _("Done");

void Script_BuildMon_ShowPointsSummary(void)
{
    InitKaizoMetaProgression();
    u8 *ptr1 = gStringVar1;
    ptr1 = StringCopy(ptr1, COMPOUND_STRING("Com: "));
    ptr1 = ConvertIntToDecimalStringN(ptr1, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_COMMON], STR_CONV_MODE_LEFT_ALIGN, 4);
    ptr1 = StringAppend(ptr1, COMPOUND_STRING(" Unc: "));
    ConvertIntToDecimalStringN(ptr1, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_UNCOMMON], STR_CONV_MODE_LEFT_ALIGN, 4);

    u8 *ptr2 = gStringVar2;
    ptr2 = StringCopy(ptr2, COMPOUND_STRING("Rar: "));
    ptr2 = ConvertIntToDecimalStringN(ptr2, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_RARE], STR_CONV_MODE_LEFT_ALIGN, 4);
    ptr2 = StringAppend(ptr2, COMPOUND_STRING(" Ep: "));
    ptr2 = ConvertIntToDecimalStringN(ptr2, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_EPIC], STR_CONV_MODE_LEFT_ALIGN, 4);
    ptr2 = StringAppend(ptr2, COMPOUND_STRING(" Leg: "));
    ConvertIntToDecimalStringN(ptr2, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_LEGENDARY], STR_CONV_MODE_LEFT_ALIGN, 4);
}

void Script_BuildMon_RefreshInfo(void)
{
    if (sChosenBuildMon == NULL)
        return;

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    u16 points = gSaveBlock2Ptr->metaProgression.rarityPoints[tier];

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    if (GetBoxMonData(sChosenBuildMon, MON_DATA_IS_SHINY))
    {
        StringAppend(gStringVar1, COMPOUND_STRING(" {COLOR RED}{STAR}"));
    }
    StringCopy(gStringVar2, GetRarityTierName(tier));
    ConvertIntToDecimalStringN(gStringVar3, points, STR_CONV_MODE_LEFT_ALIGN, 4);
}

void Script_BuildMon_Select(void)
{
    InitKaizoMetaProgression();

    if (gSpecialVar_0x8004 == PARTY_NOTHING_CHOSEN)
    {
        sChosenBuildMon = NULL;
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    if (gSpecialVar_0x8004 == PC_MON_CHOSEN)
    {
        sChosenBuildMon = GetBoxedMonPtr(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos);
        sChosenMonIsParty = FALSE;
        sChosenPartySlot = 0;
    }
    else
    {
        sChosenBuildMon = &(gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004].box);
        sChosenMonIsParty = TRUE;
        sChosenPartySlot = gSpecialVar_0x8004;
    }

    if (sChosenBuildMon == NULL || GetBoxMonData(sChosenBuildMon, MON_DATA_IS_EGG))
    {
        sChosenBuildMon = NULL;
        gSpecialVar_Result = 2; // Is an egg
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
    {
        sChosenBuildMon = NULL;
        gSpecialVar_Result = 0; // Empty/invalid
        return;
    }

    Script_BuildMon_RefreshInfo();
    gSpecialVar_Result = 1; // Success
}

void Script_BuildMon_PushMainMenu(void)
{
    MultichoiceDynamic_DestroyStack();
    if (sChosenBuildMon == NULL)
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_BuildOpt_Exit);
        struct ListMenuItem item = { buf, BUILD_OPT_EXIT };
        MultichoiceDynamic_PushElement(item);
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    bool8 isStarter = (species != SPECIES_NONE && gSaveBlock2Ptr->metaProgression.chosenGachaSpecies == species);
    bool8 shinyUnlocked = GetBoxMonData(sChosenBuildMon, MON_DATA_SHINY_UNLOCKED);
    bool8 isShiny = GetBoxMonData(sChosenBuildMon, MON_DATA_IS_SHINY);

    // 0: Starter
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, isStarter ? sText_BuildOpt_StarterRemove : sText_BuildOpt_StarterSet);
        struct ListMenuItem item = { buf, BUILD_OPT_STARTER };
        MultichoiceDynamic_PushElement(item);
    }
    // 1: Natura Reroll
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Nature);
        struct ListMenuItem item = { buf, BUILD_OPT_NATURE };
        MultichoiceDynamic_PushElement(item);
    }
    // 2: Abilita
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Ability);
        struct ListMenuItem item = { buf, BUILD_OPT_ABILITY };
        MultichoiceDynamic_PushElement(item);
    }
    // 3: Mossa 1
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Move1);
        struct ListMenuItem item = { buf, BUILD_OPT_MOVE1 };
        MultichoiceDynamic_PushElement(item);
    }
    // 4: Mossa 2
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Move2);
        struct ListMenuItem item = { buf, BUILD_OPT_MOVE2 };
        MultichoiceDynamic_PushElement(item);
    }
    // 5: Mega Stone (if species line has mega evolution)
    {
        u16 m1 = ITEM_NONE, m2 = ITEM_NONE;
        u8 megaCount = GetSpeciesMegaStones(species, &m1, &m2);
        if (megaCount > 0)
        {
            u8 *buf = Alloc(64);
            StringCopy(buf, sText_BuildOpt_MegaStone);
            struct ListMenuItem item = { buf, BUILD_OPT_MEGA_STONE };
            MultichoiceDynamic_PushElement(item);
        }
    }
    // 6: Cambia Teratipo
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Tera);
        struct ListMenuItem item = { buf, BUILD_OPT_TERA };
        MultichoiceDynamic_PushElement(item);
    }
    // 7: Shiny Skin
    if (shinyUnlocked)
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, isShiny ? COMPOUND_STRING("Shiny Skin: ON") : COMPOUND_STRING("Shiny Skin: OFF"));
        struct ListMenuItem item = { buf, BUILD_OPT_SHINY };
        MultichoiceDynamic_PushElement(item);
    }
    // 8: Cambia Pokemon
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_SwitchMon);
        struct ListMenuItem item = { buf, BUILD_OPT_SWITCH };
        MultichoiceDynamic_PushElement(item);
    }
    // 9: Fine
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_BuildOpt_Exit);
        struct ListMenuItem item = { buf, BUILD_OPT_EXIT };
        MultichoiceDynamic_PushElement(item);
    }
}

void Script_BuildMon_ToggleShiny(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    bool32 isShiny = GetBoxMonData(sChosenBuildMon, MON_DATA_IS_SHINY);
    isShiny = !isShiny;
    SetBoxMonData(sChosenBuildMon, MON_DATA_IS_SHINY, &isShiny);

    if (sChosenMonIsParty)
    {
        SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_IS_SHINY, &isShiny);
        CalculateMonStats(&gParties[B_TRAINER_PLAYER][sChosenPartySlot]);
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    gSpecialVar_Result = isShiny ? 1 : 2; // 1 = Activated, 2 = Deactivated
}

void Script_BuildMon_ToggleStarter(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    GetBoxMonNickname(sChosenBuildMon, gStringVar1);

    if (species != SPECIES_NONE && gSaveBlock2Ptr->metaProgression.chosenGachaSpecies == species)
    {
        gSaveBlock2Ptr->metaProgression.chosenGachaSpecies = SPECIES_NONE;
        gSpecialVar_Result = 0; // Removed
    }
    else
    {
        gSaveBlock2Ptr->metaProgression.chosenGachaSpecies = species;
        gSpecialVar_Result = 1; // Set
    }
}

void Script_BuildMon_RerollNature(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 5)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    u8 curNature = GetBoxMonData(sChosenBuildMon, MON_DATA_HIDDEN_NATURE);
    u8 newNature;
    do {
        newNature = Random() % NUM_NATURES;
    } while (newNature == curNature);

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 5;
    SetBoxMonData(sChosenBuildMon, MON_DATA_HIDDEN_NATURE, &newNature);

    if (sChosenMonIsParty)
    {
        SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_HIDDEN_NATURE, &newNature);
        CalculateMonStats(&gParties[B_TRAINER_PLAYER][sChosenPartySlot]);
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gNaturesInfo[newNature].name);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

void Script_BuildMon_PushAbilities(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    u16 abil0 = GetAbilityBySpecies(species, 0);
    u16 abil1 = GetAbilityBySpecies(species, 1);
    u16 abilHA = GetAbilityBySpecies(species, 2);

    // Slot 0
    if (abil0 != ABILITY_NONE)
    {
        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, COMPOUND_STRING("1: "));
        StringAppend(ptr, gAbilitiesInfo[abil0].name);
        struct ListMenuItem item = { buf, 0 };
        MultichoiceDynamic_PushElement(item);
    }

    // Slot 1
    if (abil1 != ABILITY_NONE && abil1 != abil0)
    {
        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, COMPOUND_STRING("2: "));
        StringAppend(ptr, gAbilitiesInfo[abil1].name);
        struct ListMenuItem item = { buf, 1 };
        MultichoiceDynamic_PushElement(item);
    }

    // Hidden Ability
    if (abilHA != ABILITY_NONE)
    {
        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, COMPOUND_STRING("H: "));
        StringAppend(ptr, gAbilitiesInfo[abilHA].name);
        struct ListMenuItem item = { buf, 2 };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Cancel"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_ApplyAbility(void)
{
    u8 abilitySlot = (u8)gSpecialVar_0x8005;
    if (abilitySlot > 2 || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    u16 chosenAbility = GetAbilityBySpecies(species, abilitySlot);
    if (chosenAbility == ABILITY_NONE)
    {
        gSpecialVar_Result = 0;
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 10;
    u16 noneAbility = ABILITY_NONE;
    SetBoxMonData(sChosenBuildMon, MON_DATA_CUSTOM_ABILITY, &noneAbility);
    SetBoxMonData(sChosenBuildMon, MON_DATA_ABILITY_NUM, &abilitySlot);

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gAbilitiesInfo[chosenAbility].name);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

void Script_BuildMon_PushMoves(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    MultichoiceDynamic_DestroyStack();
    for (int i = 0; i < ARRAY_COUNT(sTierStarterMoves[tier]); i++)
    {
        u16 move = sTierStarterMoves[tier][i];
        if (move == MOVE_NONE)
            continue;
        u8 *buf = Alloc(64);
        StringCopy(buf, gMovesInfo[move].name);
        struct ListMenuItem item = { buf, move };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Cancel"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFFFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_ApplyMove(void)
{
    u16 move = (u16)gSpecialVar_0x8005;
    u8 slot = (u8)gSpecialVar_0x8006; // 0 for move 1, 1 for move 2
    if (move == 0xFFFF || move == MOVE_NONE || move >= MOVES_COUNT || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 10;
    u8 pp = gMovesInfo[move].pp;

    if (slot == 0)
    {
        SetBoxMonData(sChosenBuildMon, MON_DATA_MOVE1, &move);
        SetBoxMonData(sChosenBuildMon, MON_DATA_PP1, &pp);
        if (sChosenMonIsParty)
        {
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_MOVE1, &move);
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_PP1, &pp);
        }
    }
    else
    {
        SetBoxMonData(sChosenBuildMon, MON_DATA_MOVE2, &move);
        SetBoxMonData(sChosenBuildMon, MON_DATA_PP2, &pp);
        if (sChosenMonIsParty)
        {
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_MOVE2, &move);
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_PP2, &pp);
        }
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gMovesInfo[move].name);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

static void CheckAndAddMegaStones(u16 sp, u16 *stone1, u16 *stone2)
{
    if (sp == SPECIES_NONE || sp >= NUM_SPECIES)
        return;

    const struct FormChange *formChanges = GetSpeciesFormChanges(sp);
    if (formChanges == NULL)
        return;

    for (int i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM)
        {
            u16 stone = formChanges[i].param1;
            if (stone != ITEM_NONE)
            {
                if (*stone1 == ITEM_NONE)
                {
                    *stone1 = stone;
                }
                else if (*stone2 == ITEM_NONE && *stone1 != stone)
                {
                    *stone2 = stone;
                }
            }
        }
    }
}

static void FindFamilyMegaStones(u16 sp, u16 *stone1, u16 *stone2, u8 depth)
{
    if (depth > 5 || sp == SPECIES_NONE || sp >= NUM_SPECIES)
        return;

    CheckAndAddMegaStones(sp, stone1, stone2);

    const struct Evolution *evos = GetSpeciesEvolutions(sp);
    if (evos != NULL)
    {
        for (int i = 0; evos[i].method != EVOLUTIONS_END; i++)
        {
            if (evos[i].targetSpecies != SPECIES_NONE && evos[i].targetSpecies != sp)
            {
                FindFamilyMegaStones(evos[i].targetSpecies, stone1, stone2, depth + 1);
            }
        }
    }
}

u8 GetSpeciesMegaStones(u16 species, u16 *outStone1, u16 *outStone2)
{
    u16 s1 = ITEM_NONE;
    u16 s2 = ITEM_NONE;
    u8 count = 0;

    FindFamilyMegaStones(species, &s1, &s2, 0);

    if (outStone1 != NULL)
        *outStone1 = s1;
    if (outStone2 != NULL)
        *outStone2 = s2;

    if (s1 != ITEM_NONE)
        count++;
    if (s2 != ITEM_NONE)
        count++;

    return count;
}

void Script_BuildMon_CheckMegaStone(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u16 stone1 = ITEM_NONE, stone2 = ITEM_NONE;
    u8 count = GetSpeciesMegaStones(species, &stone1, &stone2);
    if (count == 0)
    {
        gSpecialVar_Result = 0;
        return;
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);

    bool8 beatSurge = GetBoxMonData(sChosenBuildMon, MON_DATA_BEAT_SURGE);
    if (!beatSurge)
    {
        gSpecialVar_Result = 2; // Not beaten Surge yet
        return;
    }

    bool8 u1 = GetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_1_UNLOCKED);
    bool8 u2 = GetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_2_UNLOCKED);

    if ((count == 1 && u1) || (count == 2 && u1 && u2))
    {
        gSpecialVar_Result = 3; // All stones already unlocked
        return;
    }

    gSpecialVar_Result = 1; // Eligible
}

void Script_BuildMon_PushMegaChoices(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u16 stone1 = ITEM_NONE, stone2 = ITEM_NONE;
    u8 count = GetSpeciesMegaStones(species, &stone1, &stone2);
    if (count == 0)
    {
        gSpecialVar_Result = 0;
        return;
    }

    bool8 u1 = GetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_1_UNLOCKED);
    bool8 u2 = GetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_2_UNLOCKED);

    MultichoiceDynamic_DestroyStack();

    if (stone1 != ITEM_NONE)
    {
        u8 *buf = Alloc(64);
        CopyItemName(stone1, buf);
        if (u1)
            StringAppend(buf, COMPOUND_STRING(" (SBLOCCATA)"));
        else
            StringAppend(buf, COMPOUND_STRING(" (15 Pts)"));
        struct ListMenuItem item = { buf, u1 ? 0xFFFE : 0 };
        MultichoiceDynamic_PushElement(item);
    }

    if (stone2 != ITEM_NONE)
    {
        u8 *buf = Alloc(64);
        CopyItemName(stone2, buf);
        if (u2)
            StringAppend(buf, COMPOUND_STRING(" (SBLOCCATA)"));
        else
            StringAppend(buf, COMPOUND_STRING(" (15 Pts)"));
        struct ListMenuItem item = { buf, u2 ? 0xFFFE : 1 };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Cancel"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFFFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_BuyMegaStone(void)
{
    u16 choice = gSpecialVar_0x8005;
    if (choice > 1 || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 15)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    u16 stone1 = ITEM_NONE, stone2 = ITEM_NONE;
    GetSpeciesMegaStones(species, &stone1, &stone2);
    u16 boughtStone = (choice == 0) ? stone1 : stone2;
    if (boughtStone == ITEM_NONE)
    {
        gSpecialVar_Result = 0;
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 15;
    bool32 val = TRUE;
    if (choice == 0)
    {
        SetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_1_UNLOCKED, &val);
        if (sChosenMonIsParty)
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_MEGA_STONE_1_UNLOCKED, &val);
    }
    else
    {
        SetBoxMonData(sChosenBuildMon, MON_DATA_MEGA_STONE_2_UNLOCKED, &val);
        if (sChosenMonIsParty)
            SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_MEGA_STONE_2_UNLOCKED, &val);
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    CopyItemName(boughtStone, gStringVar2);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

void Script_BuildMon_PushTera(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 5)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    MultichoiceDynamic_DestroyStack();
    for (int i = 0; i < ARRAY_COUNT(sTeraTypes); i++)
    {
        u8 t = sTeraTypes[i];
        u8 *buf = Alloc(32);
        StringCopy(buf, gTypesInfo[t].name);
        struct ListMenuItem item = { buf, t };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Cancel"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_ApplyTera(void)
{
    u8 teraType = (u8)gSpecialVar_0x8005;
    if (teraType == 0xFF || teraType >= NUMBER_OF_MON_TYPES || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_LEGENDARY)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 5)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 5;
    SetBoxMonData(sChosenBuildMon, MON_DATA_TERA_TYPE, &teraType);

    if (sChosenMonIsParty)
        SetMonData(&gParties[B_TRAINER_PLAYER][sChosenPartySlot], MON_DATA_TERA_TYPE, &teraType);

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gTypesInfo[teraType].name);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

static bool8 AreSpeciesInSameFamily(u16 sp1, u16 sp2)
{
    if (sp1 == sp2)
        return TRUE;

    u16 root1 = sp1;
    int limit1 = 0;
    while (GetSpeciesPreEvolution(root1) != SPECIES_NONE && GetSpeciesPreEvolution(root1) != root1 && limit1++ < 10)
        root1 = GetSpeciesPreEvolution(root1);

    u16 root2 = sp2;
    int limit2 = 0;
    while (GetSpeciesPreEvolution(root2) != SPECIES_NONE && GetSpeciesPreEvolution(root2) != root2 && limit2++ < 10)
        root2 = GetSpeciesPreEvolution(root2);

    return (root1 == root2);
}

void Script_RecordSurgeDefeatSpecies(void)
{
    bool32 beat = TRUE;
    int p, box, slot;

    for (p = 0; p < PARTY_SIZE; p++)
    {
        u16 partySpecies = GetMonData(&gParties[B_TRAINER_PLAYER][p], MON_DATA_SPECIES);
        if (partySpecies == SPECIES_NONE || GetMonData(&gParties[B_TRAINER_PLAYER][p], MON_DATA_IS_EGG))
            continue;

        SetMonData(&gParties[B_TRAINER_PLAYER][p], MON_DATA_BEAT_SURGE, &beat);

        for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        {
            for (slot = 0; slot < IN_BOX_COUNT; slot++)
            {
                struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);
                u16 boxSpecies = GetBoxMonData(boxMon, MON_DATA_SPECIES);
                if (boxSpecies == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                    continue;

                if (AreSpeciesInSameFamily(partySpecies, boxSpecies))
                {
                    SetBoxMonData(boxMon, MON_DATA_BEAT_SURGE, &beat);
                }
            }
        }
    }
}

void OpenMonBuildScreen(struct BoxPokemon *mon, MainCallback exitCallback)
{
    SetMainCallback2(exitCallback);
}

void OpenGachaRosterPC(void)
{
    ShowPokemonStorageSystemPC();
}

// --- HUB ROAMING POKÉMON SYSTEM (ROCK, LAKE & PARK) ---

static const u16 sHubMonFlags[21] = {
    // Rock (0..6)
    FLAG_TEMP_1, FLAG_TEMP_2, FLAG_TEMP_3, FLAG_TEMP_4, FLAG_TEMP_5, FLAG_TEMP_6, FLAG_TEMP_7,
    // Lake (7..13)
    FLAG_TEMP_8, FLAG_TEMP_9, FLAG_TEMP_A, FLAG_TEMP_B, FLAG_TEMP_C, FLAG_TEMP_D, FLAG_TEMP_F,
    // Park (14..20)
    FLAG_TEMP_10, FLAG_TEMP_12, FLAG_TEMP_13, FLAG_TEMP_14, FLAG_TEMP_15, FLAG_TEMP_16, FLAG_TEMP_17
};

static u8 DetermineMonHubZone(u16 species)
{
    const struct SpeciesInfo *speciesInfo;
    u8 type1, type2;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return HUB_ZONE_PARK;

    speciesInfo = &gSpeciesInfo[species];
    type1 = speciesInfo->types[0];
    type2 = speciesInfo->types[1];

    if (type1 == TYPE_GROUND || type1 == TYPE_ROCK || type2 == TYPE_GROUND || type2 == TYPE_ROCK)
        return HUB_ZONE_ROCK;
    if (type1 == TYPE_WATER || type2 == TYPE_WATER)
        return HUB_ZONE_LAKE;
    return HUB_ZONE_PARK;
}

bool8 IsMonWanderingInHub(struct BoxPokemon *boxmon)
{
    u16 species = GetBoxMonData(boxmon, MON_DATA_SPECIES);
    u32 personality = GetBoxMonData(boxmon, MON_DATA_PERSONALITY);
    u8 i;

    if (species == SPECIES_NONE || GetBoxMonData(boxmon, MON_DATA_IS_EGG))
        return FALSE;

    for (i = 0; i < 7; i++)
    {
        if (gSaveBlock2Ptr->metaProgression.hubRockMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubRockPersonality[i] == personality)
            return TRUE;
        if (gSaveBlock2Ptr->metaProgression.hubLakeMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubLakePersonality[i] == personality)
            return TRUE;
        if (gSaveBlock2Ptr->metaProgression.hubParkMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubParkPersonality[i] == personality)
            return TRUE;
    }
    return FALSE;
}

void RemoveMonFromHubWander(struct BoxPokemon *boxmon)
{
    u16 species = GetBoxMonData(boxmon, MON_DATA_SPECIES);
    u32 personality = GetBoxMonData(boxmon, MON_DATA_PERSONALITY);
    u8 i;

    for (i = 0; i < 7; i++)
    {
        if (gSaveBlock2Ptr->metaProgression.hubRockMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubRockPersonality[i] == personality)
        {
            gSaveBlock2Ptr->metaProgression.hubRockMons[i] = SPECIES_NONE;
            gSaveBlock2Ptr->metaProgression.hubRockPersonality[i] = 0;
            gSaveBlock2Ptr->metaProgression.hubRockShiny &= ~(1 << i);
            return;
        }
        if (gSaveBlock2Ptr->metaProgression.hubLakeMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubLakePersonality[i] == personality)
        {
            gSaveBlock2Ptr->metaProgression.hubLakeMons[i] = SPECIES_NONE;
            gSaveBlock2Ptr->metaProgression.hubLakePersonality[i] = 0;
            gSaveBlock2Ptr->metaProgression.hubLakeShiny &= ~(1 << i);
            return;
        }
        if (gSaveBlock2Ptr->metaProgression.hubParkMons[i] == species
         && gSaveBlock2Ptr->metaProgression.hubParkPersonality[i] == personality)
        {
            gSaveBlock2Ptr->metaProgression.hubParkMons[i] = SPECIES_NONE;
            gSaveBlock2Ptr->metaProgression.hubParkPersonality[i] = 0;
            gSaveBlock2Ptr->metaProgression.hubParkShiny &= ~(1 << i);
            return;
        }
    }
}

bool8 TryAddMonToHubWander(struct BoxPokemon *boxmon, u8 *outZone)
{
    u16 species = GetBoxMonData(boxmon, MON_DATA_SPECIES);
    u32 personality = GetBoxMonData(boxmon, MON_DATA_PERSONALITY);
    bool32 isShiny = GetBoxMonData(boxmon, MON_DATA_IS_SHINY);
    u8 targetZone = DetermineMonHubZone(species);
    u8 i;

    if (species == SPECIES_NONE || GetBoxMonData(boxmon, MON_DATA_IS_EGG))
        return FALSE;

    // Se è tipo Acqua E (Roccia o Terra), e Roccia è piena, prova nel Lago!
    if (targetZone == HUB_ZONE_ROCK)
    {
        const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
        if (speciesInfo->types[0] == TYPE_WATER || speciesInfo->types[1] == TYPE_WATER)
        {
            bool8 rockHasRoom = FALSE;
            for (i = 0; i < 7; i++)
            {
                if (gSaveBlock2Ptr->metaProgression.hubRockMons[i] == SPECIES_NONE)
                {
                    rockHasRoom = TRUE;
                    break;
                }
            }
            if (!rockHasRoom)
                targetZone = HUB_ZONE_LAKE;
        }
    }

    if (targetZone == HUB_ZONE_ROCK)
    {
        for (i = 0; i < 7; i++)
        {
            if (gSaveBlock2Ptr->metaProgression.hubRockMons[i] == SPECIES_NONE)
            {
                gSaveBlock2Ptr->metaProgression.hubRockMons[i] = species;
                gSaveBlock2Ptr->metaProgression.hubRockPersonality[i] = personality;
                if (isShiny)
                    gSaveBlock2Ptr->metaProgression.hubRockShiny |= (1 << i);
                else
                    gSaveBlock2Ptr->metaProgression.hubRockShiny &= ~(1 << i);
                if (outZone)
                    *outZone = HUB_ZONE_ROCK;
                return TRUE;
            }
        }
    }
    else if (targetZone == HUB_ZONE_LAKE)
    {
        for (i = 0; i < 7; i++)
        {
            if (gSaveBlock2Ptr->metaProgression.hubLakeMons[i] == SPECIES_NONE)
            {
                gSaveBlock2Ptr->metaProgression.hubLakeMons[i] = species;
                gSaveBlock2Ptr->metaProgression.hubLakePersonality[i] = personality;
                if (isShiny)
                    gSaveBlock2Ptr->metaProgression.hubLakeShiny |= (1 << i);
                else
                    gSaveBlock2Ptr->metaProgression.hubLakeShiny &= ~(1 << i);
                if (outZone)
                    *outZone = HUB_ZONE_LAKE;
                return TRUE;
            }
        }
    }
    else // HUB_ZONE_PARK
    {
        for (i = 0; i < 7; i++)
        {
            if (gSaveBlock2Ptr->metaProgression.hubParkMons[i] == SPECIES_NONE)
            {
                gSaveBlock2Ptr->metaProgression.hubParkMons[i] = species;
                gSaveBlock2Ptr->metaProgression.hubParkPersonality[i] = personality;
                if (isShiny)
                    gSaveBlock2Ptr->metaProgression.hubParkShiny |= (1 << i);
                else
                    gSaveBlock2Ptr->metaProgression.hubParkShiny &= ~(1 << i);
                if (outZone)
                    *outZone = HUB_ZONE_PARK;
                return TRUE;
            }
        }
    }

    return FALSE; // Zone is full (7/7)
}

u16 GetHubRoamingMonGraphicsId(u8 idx)
{
    u16 species = SPECIES_NONE;
    bool32 shiny = FALSE;

    if (idx < 7) // Rock (0..6)
    {
        species = gSaveBlock2Ptr->metaProgression.hubRockMons[idx];
        shiny = (gSaveBlock2Ptr->metaProgression.hubRockShiny & (1 << idx)) != 0;
        if (species != SPECIES_NONE && species < NUM_SPECIES)
            return GetGraphicsIdForMon(species, shiny, FALSE);
    }
    else if (idx < 14) // Lake (7..13)
    {
        u8 lakeIdx = idx - 7;
        species = gSaveBlock2Ptr->metaProgression.hubLakeMons[lakeIdx];
        shiny = (gSaveBlock2Ptr->metaProgression.hubLakeShiny & (1 << lakeIdx)) != 0;
        if (species != SPECIES_NONE && species < NUM_SPECIES)
            return GetGraphicsIdForMon(species, shiny, FALSE) | OBJ_EVENT_MON_SWIMMING;
    }
    else if (idx < 21) // Park (14..20)
    {
        u8 parkIdx = idx - 14;
        species = gSaveBlock2Ptr->metaProgression.hubParkMons[parkIdx];
        shiny = (gSaveBlock2Ptr->metaProgression.hubParkShiny & (1 << parkIdx)) != 0;
        if (species != SPECIES_NONE && species < NUM_SPECIES)
            return GetGraphicsIdForMon(species, shiny, FALSE);
    }

    return OBJ_EVENT_GFX_LITTLE_BOY;
}

void Script_UpdateHubRoamingMons(void)
{
    u8 i;

    // Assicura che il giocatore abbia sempre MN03 Surf nell'Hub
    if (!CheckBagHasItem(ITEM_HM03, 1))
        AddBagItem(ITEM_HM03, 1);

    for (i = 0; i < 7; i++)
    {
        u16 sp = gSaveBlock2Ptr->metaProgression.hubRockMons[i];
        if (sp != SPECIES_NONE && sp < NUM_SPECIES)
            FlagClear(sHubMonFlags[i]);
        else
            FlagSet(sHubMonFlags[i]);
    }

    for (i = 0; i < 7; i++)
    {
        u16 sp = gSaveBlock2Ptr->metaProgression.hubLakeMons[i];
        if (sp != SPECIES_NONE && sp < NUM_SPECIES)
            FlagClear(sHubMonFlags[7 + i]);
        else
            FlagSet(sHubMonFlags[7 + i]);
    }

    for (i = 0; i < 7; i++)
    {
        u16 sp = gSaveBlock2Ptr->metaProgression.hubParkMons[i];
        if (sp != SPECIES_NONE && sp < NUM_SPECIES)
            FlagClear(sHubMonFlags[14 + i]);
        else
            FlagSet(sHubMonFlags[14 + i]);
    }
}

void Script_HubMon_Interact(void)
{
    u8 localId = gSpecialVar_LastTalked;
    u16 species = SPECIES_NONE;

    // Tester NPC has localId 1. Hub Mons have localId 2..22.
    if (localId >= 2 && localId <= 22)
    {
        u8 idx = localId - 2;
        if (idx < 7)
            species = gSaveBlock2Ptr->metaProgression.hubRockMons[idx];
        else if (idx < 14)
            species = gSaveBlock2Ptr->metaProgression.hubLakeMons[idx - 7];
        else if (idx < 21)
            species = gSaveBlock2Ptr->metaProgression.hubParkMons[idx - 14];
    }

    if (species != SPECIES_NONE && species < NUM_SPECIES)
    {
        StringCopy(gStringVar1, GetSpeciesName(species));
        gSpecialVar_Result = species;
    }
    else
    {
        gSpecialVar_Result = SPECIES_NONE;
    }
}
