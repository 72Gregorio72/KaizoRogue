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
#include "constants/party_menu.h"
#include "data.h"
#include "daycare.h"
#include "move.h"
#include "roguelike_rarity.h"
#include "kaizo_roster.h"

static EWRAM_DATA struct BoxPokemon *sChosenBuildMon = NULL;
static EWRAM_DATA bool8 sChosenMonIsParty = FALSE;
static EWRAM_DATA u8 sChosenPartySlot = 0;

static const u8 sCompetitiveNatures[] = {
    NATURE_ADAMANT,
    NATURE_JOLLY,
    NATURE_MODEST,
    NATURE_TIMID,
    NATURE_IMPISH,
    NATURE_BOLD,
    NATURE_CAREFUL,
    NATURE_CALM,
    NATURE_HARDY
};

static const u8 sCompetitiveNatureDescs[][24] = {
    _(" (+Att, -AttSp)"),
    _(" (+Vel, -AttSp)"),
    _(" (+AttSp, -Att)"),
    _(" (+Vel, -Att)"),
    _(" (+Dif, -AttSp)"),
    _(" (+Dif, -Att)"),
    _(" (+DifSp, -AttSp)"),
    _(" (+DifSp, -Att)"),
    _(" (Neutra)")
};

static const u16 sTierStarterMoves[5][4] = {
    [RARITY_COMMON]   = { MOVE_QUICK_ATTACK, MOVE_WATER_GUN, MOVE_EMBER, MOVE_BITE },
    [RARITY_UNCOMMON] = { MOVE_AERIAL_ACE, MOVE_SWIFT, MOVE_WATER_PULSE, MOVE_MAGICAL_LEAF },
    [RARITY_RARE]     = { MOVE_FLAMETHROWER, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_PSYCHIC },
    [RARITY_EPIC]     = { MOVE_DRAGON_PULSE, MOVE_SHADOW_BALL, MOVE_MOONBLAST, MOVE_EARTH_POWER }
};

static const u8 sText_BuildOpt_StarterSet[]    = _("Imposta Starter Run");
static const u8 sText_BuildOpt_StarterRemove[] = _("Rimuovi Starter Run");
static const u8 sText_BuildOpt_Nature[]        = _("Cambia Natura (5 Pt)");
static const u8 sText_BuildOpt_Ability[]       = _("Cambia Abilita' (10 Pt)");
static const u8 sText_BuildOpt_Move[]          = _("Modifica Mossa (10 Pt)");
static const u8 sText_BuildOpt_Fuse[]          = _("Fondi Pokemon (+10 Pt)");
static const u8 sText_BuildOpt_SwitchMon[]     = _("Scegli Altro Pokemon");
static const u8 sText_BuildOpt_Exit[]          = _("Fine");

void Script_BuildMon_ShowPointsSummary(void)
{
    InitKaizoMetaProgression();
    u8 *ptr1 = gStringVar1;
    ptr1 = StringCopy(ptr1, COMPOUND_STRING("Comune: "));
    ptr1 = ConvertIntToDecimalStringN(ptr1, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_COMMON], STR_CONV_MODE_LEFT_ALIGN, 4);
    ptr1 = StringAppend(ptr1, COMPOUND_STRING("  -  Non Comune: "));
    ConvertIntToDecimalStringN(ptr1, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_UNCOMMON], STR_CONV_MODE_LEFT_ALIGN, 4);

    u8 *ptr2 = gStringVar2;
    ptr2 = StringCopy(ptr2, COMPOUND_STRING("Raro: "));
    ptr2 = ConvertIntToDecimalStringN(ptr2, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_RARE], STR_CONV_MODE_LEFT_ALIGN, 4);
    ptr2 = StringAppend(ptr2, COMPOUND_STRING("  -  Epico: "));
    ConvertIntToDecimalStringN(ptr2, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_EPIC], STR_CONV_MODE_LEFT_ALIGN, 4);
}

void Script_BuildMon_RefreshInfo(void)
{
    if (sChosenBuildMon == NULL)
        return;

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    u16 points = gSaveBlock2Ptr->metaProgression.rarityPoints[tier];

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
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
    if (sChosenBuildMon == NULL)
        return;

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    bool8 isStarter = (species != SPECIES_NONE && gSaveBlock2Ptr->metaProgression.chosenGachaSpecies == species);

    // 0: Starter
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, isStarter ? sText_BuildOpt_StarterRemove : sText_BuildOpt_StarterSet);
        struct ListMenuItem item = { buf, 0 };
        MultichoiceDynamic_PushElement(item);
    }
    // 1: Natura
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Nature);
        struct ListMenuItem item = { buf, 1 };
        MultichoiceDynamic_PushElement(item);
    }
    // 2: Abilita
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Ability);
        struct ListMenuItem item = { buf, 2 };
        MultichoiceDynamic_PushElement(item);
    }
    // 3: Mossa
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Move);
        struct ListMenuItem item = { buf, 3 };
        MultichoiceDynamic_PushElement(item);
    }
    // 4: Fondi
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_Fuse);
        struct ListMenuItem item = { buf, 4 };
        MultichoiceDynamic_PushElement(item);
    }
    // 5: Cambia Pokemon
    {
        u8 *buf = Alloc(64);
        StringCopy(buf, sText_BuildOpt_SwitchMon);
        struct ListMenuItem item = { buf, 5 };
        MultichoiceDynamic_PushElement(item);
    }
    // 6: Fine
    {
        u8 *buf = Alloc(32);
        StringCopy(buf, sText_BuildOpt_Exit);
        struct ListMenuItem item = { buf, 6 };
        MultichoiceDynamic_PushElement(item);
    }
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

void Script_BuildMon_PushNatures(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 5)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    for (int i = 0; i < ARRAY_COUNT(sCompetitiveNatures); i++)
    {
        u8 nat = sCompetitiveNatures[i];
        u8 *buf = Alloc(64);
        u8 *ptr = StringCopy(buf, gNaturesInfo[nat].name);
        StringAppend(ptr, sCompetitiveNatureDescs[i]);
        struct ListMenuItem item = { buf, nat };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Annulla"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_ApplyNature(void)
{
    u8 chosenNature = (u8)gSpecialVar_0x8005;
    if (chosenNature >= NUM_NATURES || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 5)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 5;

    SetBoxMonData(sChosenBuildMon, MON_DATA_HIDDEN_NATURE, &chosenNature);

    if (sChosenMonIsParty)
    {
        CalculateMonStats(&gParties[B_TRAINER_PLAYER][sChosenPartySlot]);
    }

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gNaturesInfo[chosenNature].name);
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
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
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
        StringCopy(bufCancel, COMPOUND_STRING("Annulla"));
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
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
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
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        u16 move = sTierStarterMoves[tier][i];
        u8 *buf = Alloc(64);
        StringCopy(buf, gMovesInfo[move].name);
        struct ListMenuItem item = { buf, move };
        MultichoiceDynamic_PushElement(item);
    }

    {
        u8 *bufCancel = Alloc(32);
        StringCopy(bufCancel, COMPOUND_STRING("Annulla"));
        struct ListMenuItem cancelItem = { bufCancel, 0xFFFF };
        MultichoiceDynamic_PushElement(cancelItem);
    }

    gSpecialVar_Result = 0;
}

void Script_BuildMon_ApplyMove(void)
{
    u16 move = (u16)gSpecialVar_0x8005;
    if (move == 0xFFFF || move == MOVE_NONE || move >= MOVES_COUNT || sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0; // Cancelled
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] < 10)
    {
        gSpecialVar_Result = 2; // Not enough points
        return;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 10;
    SetBoxMonData(sChosenBuildMon, MON_DATA_MOVE1, &move);
    u8 pp = gMovesInfo[move].pp;
    SetBoxMonData(sChosenBuildMon, MON_DATA_PP1, &pp);

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, gMovesInfo[move].name);
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    gSpecialVar_Result = 1; // Success
}

void Script_BuildMon_FuseSelectedMon(void)
{
    if (sChosenBuildMon == NULL)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u16 species = GetBoxMonData(sChosenBuildMon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
    {
        gSpecialVar_Result = 0;
        return;
    }

    u8 tier = GetSpeciesRarityTier(species);
    if (tier == RARITY_NONE || tier > RARITY_EPIC)
        tier = RARITY_COMMON;

    if (gSaveBlock2Ptr->metaProgression.chosenGachaSpecies == species)
        gSaveBlock2Ptr->metaProgression.chosenGachaSpecies = SPECIES_NONE;

    GetBoxMonNickname(sChosenBuildMon, gStringVar1);
    StringCopy(gStringVar2, GetRarityTierName(tier));

    ZeroBoxMonData(sChosenBuildMon);
    if (sChosenMonIsParty)
    {
        for (int i = sChosenPartySlot; i < PARTY_SIZE - 1; i++)
            gParties[B_TRAINER_PLAYER][i] = gParties[B_TRAINER_PLAYER][i + 1];
        ZeroMonData(&gParties[B_TRAINER_PLAYER][PARTY_SIZE - 1]);
        if (gPartiesCount[B_TRAINER_PLAYER] > 0)
            gPartiesCount[B_TRAINER_PLAYER]--;
    }

    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] += 10;
    ConvertIntToDecimalStringN(gStringVar3, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    sChosenBuildMon = NULL;
    gSpecialVar_Result = 1;
}

void OpenMonBuildScreen(struct BoxPokemon *mon, MainCallback exitCallback)
{
    SetMainCallback2(exitCallback);
}

void OpenGachaRosterPC(void)
{
    ShowPokemonStorageSystemPC();
}
