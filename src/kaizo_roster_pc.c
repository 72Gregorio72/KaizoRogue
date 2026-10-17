#include "global.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "field_specials.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "roguelike_rarity.h"
#include "kaizo_roster.h"
#include "move.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "util.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define MAX_SPECIES_PER_GEN 120
#define VISIBLE_ROWS 6

enum RosterMode
{
    MODE_GEN_BROWSER,
    MODE_DETAIL_BUILD,
    MODE_SELECT_NATURE,
    MODE_SELECT_ABILITY,
    MODE_SELECT_MOVE,
    MODE_EXIT,
};

struct RosterPCData
{
    u8 mode;
    u8 currentGen;
    u16 genSpecies[MAX_SPECIES_PER_GEN];
    struct BoxPokemon *unlockedMons[MAX_SPECIES_PER_GEN];
    u16 genSpeciesCount;
    u16 selectedIndex;
    u16 scrollOffset;
    u8 buildMenuIndex;
    u8 subMenuIndex;

    u8 previewSpriteId;
    u16 currentSpecies;
    struct BoxPokemon *selectedBoxMon;

    u8 headerWindowId;
    u8 listWindowId;
    u8 rightWindowId;
    u8 bottomWindowId;
    u8 subMenuWindowId;

    u16 moveChoices[4];
};

static EWRAM_DATA struct RosterPCData *sRoster = NULL;

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const struct WindowTemplate sWindowTemplates[] =
{
    // 0: Header (Top)
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 28,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 1
    },
    // 1: Left List (Species)
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 5,
        .width = 15,
        .height = 14,
        .paletteNum = 15,
        .baseBlock = 85
    },
    // 2: Right Preview / Detail (Stats & Graph)
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 5,
        .width = 12,
        .height = 14,
        .paletteNum = 15,
        .baseBlock = 295
    },
    // 3: Submenu / Dialog
    {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 6,
        .width = 15,
        .height = 12,
        .paletteNum = 15,
        .baseBlock = 465
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sRosterBgPalette[] =
{
    RGB(1, 4, 10),      // 0: Deep PC Navy
    RGB(31, 31, 31),    // 1: White
    RGB(10, 24, 28),    // 2: Cyan highlight
    RGB(4, 12, 16),     // 3: Cyan shadow
    RGB(8, 14, 24),     // 4: Border dark blue
    RGB(28, 22, 6),     // 5: Gold
    RGB(0, 24, 8),      // 6: Green
    RGB(24, 4, 4),      // 7: Red
    RGB(6, 8, 14),      // 8: Slot background
    RGB(18, 18, 20),    // 9: Light gray
    RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0)
};

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

static void CB2_RosterMain(void);
static void VBlankCB_Roster(void);
static void Task_RosterLoop(u8 taskId);
static void LoadCurrentGeneration(void);
static void DrawBrowserScreen(void);
static void DrawDetailBuildScreen(void);
static void UpdatePreviewSprite(u16 species, bool8 isUnlocked);
static void CleanPreviewSprite(void);
static u16 GetMonAbilityAtSlot(u16 species, u8 slot);

void OpenGachaRosterPC(void)
{
    sRoster = AllocZeroed(sizeof(struct RosterPCData));
    if (sRoster == NULL)
    {
        ScriptContext_Enable();
        return;
    }

    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);

    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    InitWindows(sWindowTemplates);

    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, 0x2A8, BG_PLTT_ID(13));
    LoadPalette(sRosterBgPalette, BG_PLTT_ID(0), sizeof(sRosterBgPalette));
    LoadPalette(GetTextWindowPalette(0), BG_PLTT_ID(15), 32);

    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetAllPicSprites();

    sRoster->headerWindowId = 0;
    sRoster->listWindowId = 1;
    sRoster->rightWindowId = 2;
    sRoster->subMenuWindowId = 3;
    sRoster->previewSpriteId = 0xFF;
    sRoster->mode = MODE_GEN_BROWSER;
    sRoster->currentGen = 0;

    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
    CopyBgTilemapBufferToVram(1);
    ShowBg(0);
    ShowBg(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_OBJ_ON);

    LoadCurrentGeneration();
    DrawBrowserScreen();

    CreateTask(Task_RosterLoop, 0);

    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);

    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_Roster);
    SetMainCallback2(CB2_RosterMain);
}

static void CB2_RosterMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_Roster(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void LoadCurrentGeneration(void)
{
    sRoster->genSpeciesCount = GetGenBaseSpecies(sRoster->currentGen, sRoster->genSpecies, MAX_SPECIES_PER_GEN);
    sRoster->selectedIndex = 0;
    sRoster->scrollOffset = 0;

    for (u16 i = 0; i < sRoster->genSpeciesCount; i++)
        sRoster->unlockedMons[i] = NULL;

    for (int box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (int slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            struct BoxPokemon *b = GetBoxedMonPtr(box, slot);
            u16 sp = GetBoxMonData(b, MON_DATA_SPECIES);
            if (sp != SPECIES_NONE && sp < NUM_SPECIES)
            {
                for (u16 i = 0; i < sRoster->genSpeciesCount; i++)
                {
                    if (sRoster->genSpecies[i] == sp)
                    {
                        sRoster->unlockedMons[i] = b;
                        break;
                    }
                }
            }
        }
    }

    if (sRoster->genSpeciesCount > 0)
        sRoster->currentSpecies = sRoster->genSpecies[0];
    else
        sRoster->currentSpecies = SPECIES_NONE;
}

static void CleanPreviewSprite(void)
{
    if (sRoster->previewSpriteId != 0xFF)
    {
        FreeAndDestroyMonPicSprite(sRoster->previewSpriteId);
        sRoster->previewSpriteId = 0xFF;
    }
}

static void UpdatePreviewSprite(u16 species, bool8 isUnlocked)
{
    CleanPreviewSprite();
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;

    s16 posX = (sRoster->mode == MODE_DETAIL_BUILD) ? 36 : 196;
    s16 posY = (sRoster->mode == MODE_DETAIL_BUILD) ? 58 : 64;

    sRoster->previewSpriteId = CreateMonPicSprite(species, FALSE, 0, TRUE, posX, posY, 14, TAG_NONE);
    if (!isUnlocked && sRoster->previewSpriteId != 0xFF)
    {
        // Dark silhouette tint
        BlendPalette(OBJ_PLTT_ID(14), 16, 16, RGB(2, 2, 4));
        CpuCopy16(&gPlttBufferFaded[OBJ_PLTT_ID(14)], &gPlttBufferUnfaded[OBJ_PLTT_ID(14)], PLTT_SIZE_4BPP);
    }
}

static void DrawBrowserScreen(void)
{
    // 1. Header Window
    FillWindowPixelBuffer(sRoster->headerWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sRoster->headerWindowId);

    // Count unlocked in current generation
    u16 unlockedCount = 0;
    for (u16 i = 0; i < sRoster->genSpeciesCount; i++)
    {
        if (sRoster->unlockedMons[i] != NULL)
            unlockedCount++;
    }

    static const u8 sText_HeaderGenOpen[]   = _("<L> < ");
    static const u8 sText_HeaderGenClose[]  = _(" > <R>  (");
    static const u8 sText_Slash[]           = _("/");
    static const u8 sText_ParenClose[]      = _(")");
    static const u8 sText_PtsCom[]          = _("Pt: Com:");
    static const u8 sText_PtsNonCom[]       = _(" NonCom:");
    static const u8 sText_PtsRare[]         = _(" Raro:");
    static const u8 sText_PtsEpic[]         = _(" Epic:");

    static const u8 sText_CursorArrow[]     = _("{COLOR RED}> ");
    static const u8 sText_CursorSpace[]     = _("  ");
    static const u8 sText_Hash[]            = _("N.");
    static const u8 sText_Space[]           = _(" ");
    static const u8 sText_StarLocked[]      = _("{COLOR DARK_GRAY}???");
    static const u8 sText_StarGold[]        = _("{COLOR GREEN} MAX");
    static const u8 sText_TierPrefix[]      = _("Tier: ");
    static const u8 sText_StarterBadge[]    = _("{COLOR GREEN}> STARTER RUN");
    static const u8 sText_PressABuild[]     = _("Premi (A) per Build");
    static const u8 sText_LockedDesc1[]     = _("{COLOR DARK_GRAY}Non sbloccato!");
    static const u8 sText_LockedDesc2[]     = _("Schiudi uovo Gacha");

    u8 title[48];
    u8 *ptr = StringCopy(title, sText_HeaderGenOpen);
    ptr = StringAppend(ptr, gGenerationTable[sRoster->currentGen].name);
    ptr = StringAppend(ptr, sText_HeaderGenClose);
    ptr = ConvertIntToDecimalStringN(ptr, unlockedCount, STR_CONV_MODE_LEFT_ALIGN, 3);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, sRoster->genSpeciesCount, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(ptr, sText_ParenClose);

    AddTextPrinterParameterized(sRoster->headerWindowId, 2, title, 2, 1, 0, NULL);

    u8 ptsStr[48];
    ptr = StringCopy(ptsStr, sText_PtsCom);
    ptr = ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_COMMON], STR_CONV_MODE_LEFT_ALIGN, 3);
    ptr = StringAppend(ptr, sText_PtsNonCom);
    ptr = ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_UNCOMMON], STR_CONV_MODE_LEFT_ALIGN, 3);
    ptr = StringAppend(ptr, sText_PtsRare);
    ptr = ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_RARE], STR_CONV_MODE_LEFT_ALIGN, 3);
    ptr = StringAppend(ptr, sText_PtsEpic);
    ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.rarityPoints[RARITY_EPIC], STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sRoster->headerWindowId, 0, ptsStr, 2, 13, 0, NULL);
    CopyWindowToVram(sRoster->headerWindowId, COPYWIN_FULL);

    // 2. Left List Window
    FillWindowPixelBuffer(sRoster->listWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sRoster->listWindowId);

    for (u8 r = 0; r < VISIBLE_ROWS; r++)
    {
        u16 itemIdx = sRoster->scrollOffset + r;
        if (itemIdx >= sRoster->genSpeciesCount)
            break;

        u16 sp = sRoster->genSpecies[itemIdx];
        struct BoxPokemon *b = sRoster->unlockedMons[itemIdx];
        bool8 isUnlocked = (b != NULL);
        bool8 isSelected = (itemIdx == sRoster->selectedIndex);

        u8 rowStr[32];
        ptr = rowStr;
        if (isSelected)
            ptr = StringAppend(ptr, sText_CursorArrow);
        else
            ptr = StringAppend(ptr, sText_CursorSpace);

        // Dex number
        ptr = StringAppend(ptr, sText_Hash);
        ptr = ConvertIntToDecimalStringN(ptr, sp, STR_CONV_MODE_LEADING_ZEROS, 3);
        ptr = StringAppend(ptr, sText_Space);

        if (isUnlocked)
        {
            ptr = StringAppend(ptr, gSpeciesInfo[sp].speciesName);
            if (sp == gSaveBlock2Ptr->metaProgression.chosenGachaSpecies)
                StringAppend(ptr, sText_StarGold);
        }
        else
        {
            StringAppend(ptr, sText_StarLocked);
        }

        AddTextPrinterParameterized(sRoster->listWindowId, 0, rowStr, 2, r * 16 + 4, 0, NULL);
    }
    CopyWindowToVram(sRoster->listWindowId, COPYWIN_FULL);

    // 3. Right Preview Window
    FillWindowPixelBuffer(sRoster->rightWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sRoster->rightWindowId);

    if (sRoster->genSpeciesCount > 0)
    {
        u16 curSp = sRoster->genSpecies[sRoster->selectedIndex];
        struct BoxPokemon *curB = sRoster->unlockedMons[sRoster->selectedIndex];
        bool8 isUnlocked = (curB != NULL);

        UpdatePreviewSprite(curSp, isUnlocked);

        u8 descStr[48];
        if (isUnlocked)
        {
            u8 tier = GetSpeciesRarityTier(curSp);
            u8 *p = StringCopy(descStr, sText_TierPrefix);
            StringAppend(p, GetRarityTierName(tier));
            AddTextPrinterParameterized(sRoster->rightWindowId, 0, descStr, 2, 70, 0, NULL);

            if (curSp == gSaveBlock2Ptr->metaProgression.chosenGachaSpecies)
                AddTextPrinterParameterized(sRoster->rightWindowId, 0, sText_StarterBadge, 2, 84, 0, NULL);
            else
                AddTextPrinterParameterized(sRoster->rightWindowId, 0, sText_PressABuild, 2, 84, 0, NULL);
        }
        else
        {
            AddTextPrinterParameterized(sRoster->rightWindowId, 0, sText_LockedDesc1, 2, 70, 0, NULL);
            AddTextPrinterParameterized(sRoster->rightWindowId, 0, sText_LockedDesc2, 2, 84, 0, NULL);
        }
    }
    CopyWindowToVram(sRoster->rightWindowId, COPYWIN_FULL);
}

static void DrawDetailBuildScreen(void)
{
    FillWindowPixelBuffer(sRoster->listWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sRoster->listWindowId);
    FillWindowPixelBuffer(sRoster->rightWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sRoster->rightWindowId);

    u16 sp = sRoster->currentSpecies;
    struct BoxPokemon *b = sRoster->selectedBoxMon;
    if (b == NULL)
        return;

    UpdatePreviewSprite(sp, TRUE);

    static const u8 sText_ParenOpen[]       = _(" (");
    static const u8 sText_ParenClose[]      = _(")");
    static const u8 sText_CursorArrow[]     = _("{COLOR RED}> ");
    static const u8 sText_CursorSpace[]     = _("  ");
    static const u8 sText_StarGold[]        = _("{COLOR GREEN} MAX");

    static const u8 sText_MonPt[]           = _("Pt: ");
    static const u8 sText_MonNat[]          = _("Nat: ");
    static const u8 sText_MonAbil[]         = _("Abil: ");
    static const u8 sText_MonMossa[]        = _("Mossa: ");

    static const u8 sText_OptNature[]       = _("Cambia Natura (5 Pt)");
    static const u8 sText_OptAbility[]      = _("Cambia Abilita' (10 Pt)");
    static const u8 sText_OptMove[]         = _("Modifica Mossa (10 Pt)");
    static const u8 sText_OptStarterRem[]   = _("Rimuovi Starter Run");
    static const u8 sText_OptStarterSet[]   = _("Imposta Starter Run");
    static const u8 sText_OptBack[]         = _("Torna alla Lista");

    static const u8 sText_GrafoIvTitle[]    = _("{COLOR BLUE}GRAFO IV");
    static const u8 sText_StatHP[]          = _("PS: ");
    static const u8 sText_StatAtk[]         = _("ATK:");
    static const u8 sText_StatDef[]         = _("DEF:");
    static const u8 sText_StatSpA[]         = _("SPA:");
    static const u8 sText_StatSpD[]         = _("SPD:");
    static const u8 sText_StatSpe[]         = _("SPE:");

    static const u8 sText_BarOpen[]         = _(" (");
    static const u8 sText_BarClose[]        = _(") ");
    static const u8 sText_BarFill[]         = _("=");
    static const u8 sText_BarEmpty[]        = _("-");
    static const u8 sText_TotLabel[]        = _("TOT: ");
    static const u8 sText_TotSlash[]        = _(" / 186");

    // Left Window: Mon Info & Build Menu
    u8 tier = GetSpeciesRarityTier(sp);
    u8 nat = GetBoxMonData(b, MON_DATA_HIDDEN_NATURE);
    if (nat >= NUM_NATURES)
        nat = GetNatureFromPersonality(GetBoxMonData(b, MON_DATA_PERSONALITY));

    u16 customAbil = GetBoxMonData(b, MON_DATA_CUSTOM_ABILITY);
    u16 abil = (customAbil != ABILITY_NONE) ? customAbil : GetAbilityBySpecies(sp, GetBoxMonData(b, MON_DATA_ABILITY_NUM));
    u16 move = GetBoxMonData(b, MON_DATA_MOVE1);

    u8 buf[40];
    u8 *ptr = StringCopy(buf, gSpeciesInfo[sp].speciesName);
    ptr = StringAppend(ptr, sText_ParenOpen);
    ptr = StringAppend(ptr, GetRarityTierName(tier));
    StringAppend(ptr, sText_ParenClose);
    AddTextPrinterParameterized(sRoster->listWindowId, 2, buf, 2, 2, 0, NULL);

    ptr = StringCopy(buf, sText_MonPt);
    ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sRoster->listWindowId, 0, buf, 2, 16, 0, NULL);

    ptr = StringCopy(buf, sText_MonNat);
    StringAppend(ptr, gNaturesInfo[nat].name);
    AddTextPrinterParameterized(sRoster->listWindowId, 0, buf, 2, 28, 0, NULL);

    ptr = StringCopy(buf, sText_MonAbil);
    StringAppend(ptr, gAbilitiesInfo[abil].name);
    AddTextPrinterParameterized(sRoster->listWindowId, 0, buf, 2, 40, 0, NULL);

    ptr = StringCopy(buf, sText_MonMossa);
    StringAppend(ptr, gMovesInfo[move].name);
    AddTextPrinterParameterized(sRoster->listWindowId, 0, buf, 2, 52, 0, NULL);

    // Build Menu Options
    const u8 *menuOpts[5] = {
        sText_OptNature,
        sText_OptAbility,
        sText_OptMove,
        (sp == gSaveBlock2Ptr->metaProgression.chosenGachaSpecies) ? sText_OptStarterRem : sText_OptStarterSet,
        sText_OptBack
    };

    for (u8 i = 0; i < 5; i++)
    {
        u8 optStr[36];
        ptr = optStr;
        if (i == sRoster->buildMenuIndex)
            ptr = StringAppend(ptr, sText_CursorArrow);
        else
            ptr = StringAppend(ptr, sText_CursorSpace);

        StringAppend(ptr, menuOpts[i]);
        AddTextPrinterParameterized(sRoster->listWindowId, 0, optStr, 2, 68 + i * 11, 0, NULL);
    }
    CopyWindowToVram(sRoster->listWindowId, COPYWIN_FULL);

    // Right Window: IV GRAPH
    AddTextPrinterParameterized(sRoster->rightWindowId, 2, sText_GrafoIvTitle, 16, 2, 0, NULL);

    u8 ivHp  = GetBoxMonData(b, MON_DATA_HP_IV);
    u8 ivAtk = GetBoxMonData(b, MON_DATA_ATK_IV);
    u8 ivDef = GetBoxMonData(b, MON_DATA_DEF_IV);
    u8 ivSpe = GetBoxMonData(b, MON_DATA_SPEED_IV);
    u8 ivSpA = GetBoxMonData(b, MON_DATA_SPATK_IV);
    u8 ivSpD = GetBoxMonData(b, MON_DATA_SPDEF_IV);
    u16 ivTot = ivHp + ivAtk + ivDef + ivSpe + ivSpA + ivSpD;

    const u8 *statLabels[6] = { sText_StatHP, sText_StatAtk, sText_StatDef, sText_StatSpA, sText_StatSpD, sText_StatSpe };
    u8 ivVals[6] = { ivHp, ivAtk, ivDef, ivSpA, ivSpD, ivSpe };

    for (u8 s = 0; s < 6; s++)
    {
        u8 statLine[32];
        ptr = StringCopy(statLine, statLabels[s]);

        // Draw IV Bar (10 segments)
        u8 filled = (ivVals[s] * 10) / 31;
        ptr = StringAppend(ptr, sText_BarOpen);
        for (u8 bIdx = 0; bIdx < 10; bIdx++)
        {
            if (bIdx < filled)
                ptr = StringAppend(ptr, sText_BarFill);
            else
                ptr = StringAppend(ptr, sText_BarEmpty);
        }
        ptr = StringAppend(ptr, sText_BarClose);

        ptr = ConvertIntToDecimalStringN(ptr, ivVals[s], STR_CONV_MODE_LEFT_ALIGN, 2);
        if (ivVals[s] == 31)
            StringAppend(ptr, sText_StarGold);

        AddTextPrinterParameterized(sRoster->rightWindowId, 0, statLine, 2, 16 + s * 14, 0, NULL);
    }

    ptr = StringCopy(buf, sText_TotLabel);
    ptr = ConvertIntToDecimalStringN(ptr, ivTot, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(ptr, sText_TotSlash);
    AddTextPrinterParameterized(sRoster->rightWindowId, 0, buf, 14, 104, 0, NULL);

    CopyWindowToVram(sRoster->rightWindowId, COPYWIN_FULL);
}

static u16 GetMonAbilityAtSlot(u16 species, u8 slot)
{
    if (slot == 0) return GetAbilityBySpecies(species, 0);
    if (slot == 1) return GetAbilityBySpecies(species, 1);
    if (slot == 2) return GetAbilityBySpecies(species, 2);
    return ABILITY_NONE;
}

static void Task_RosterLoop(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    switch (sRoster->mode)
    {
    case MODE_GEN_BROWSER:
        // Generation Switching with L and R
        if (JOY_NEW(L_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->currentGen = (sRoster->currentGen == 0) ? (NUM_GENERATIONS - 1) : (sRoster->currentGen - 1);
            LoadCurrentGeneration();
            DrawBrowserScreen();
            return;
        }
        else if (JOY_NEW(R_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->currentGen = (sRoster->currentGen + 1) % NUM_GENERATIONS;
            LoadCurrentGeneration();
            DrawBrowserScreen();
            return;
        }

        // Vertical List Scrolling
        if (JOY_NEW(DPAD_UP))
        {
            if (sRoster->selectedIndex > 0)
            {
                PlaySE(SE_SELECT);
                sRoster->selectedIndex--;
                if (sRoster->selectedIndex < sRoster->scrollOffset)
                    sRoster->scrollOffset = sRoster->selectedIndex;
                DrawBrowserScreen();
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sRoster->selectedIndex + 1 < sRoster->genSpeciesCount)
            {
                PlaySE(SE_SELECT);
                sRoster->selectedIndex++;
                if (sRoster->selectedIndex >= sRoster->scrollOffset + VISIBLE_ROWS)
                    sRoster->scrollOffset = sRoster->selectedIndex - VISIBLE_ROWS + 1;
                DrawBrowserScreen();
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            if (sRoster->genSpeciesCount > 0)
            {
                u16 sp = sRoster->genSpecies[sRoster->selectedIndex];
                struct BoxPokemon *b = sRoster->unlockedMons[sRoster->selectedIndex];
                if (b != NULL)
                {
                    PlaySE(SE_PIN);
                    sRoster->currentSpecies = sp;
                    sRoster->selectedBoxMon = b;
                    sRoster->mode = MODE_DETAIL_BUILD;
                    sRoster->buildMenuIndex = 0;
                    DrawDetailBuildScreen();
                }
                else
                {
                    PlaySE(SE_BOO);
                }
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sRoster->mode = MODE_EXIT;
        }
        break;

    case MODE_DETAIL_BUILD:
        if (JOY_NEW(DPAD_UP))
        {
            if (sRoster->buildMenuIndex > 0)
            {
                PlaySE(SE_SELECT);
                sRoster->buildMenuIndex--;
                DrawDetailBuildScreen();
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sRoster->buildMenuIndex < 4)
            {
                PlaySE(SE_SELECT);
                sRoster->buildMenuIndex++;
                DrawDetailBuildScreen();
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 tier = GetSpeciesRarityTier(sRoster->currentSpecies);
            u16 pts = gSaveBlock2Ptr->metaProgression.rarityPoints[tier];

            switch (sRoster->buildMenuIndex)
            {
            case 0: // Cambia Natura (5 Pt)
                if (pts >= 5)
                {
                    sRoster->mode = MODE_SELECT_NATURE;
                    sRoster->subMenuIndex = 0;
                    PlaySE(SE_SELECT);
                }
                else
                {
                    PlaySE(SE_BOO);
                }
                break;

            case 1: // Cambia Abilita' (10 Pt)
                if (pts >= 10)
                {
                    sRoster->mode = MODE_SELECT_ABILITY;
                    sRoster->subMenuIndex = 0;
                    PlaySE(SE_SELECT);
                }
                else
                {
                    PlaySE(SE_BOO);
                }
                break;

            case 2: // Modifica Mossa (10 Pt)
                if (pts >= 10)
                {
                    // Roll 3 matching moves
                    for (int i = 0; i < 3; i++)
                        sRoster->moveChoices[i] = GetRandomRarityMoveByTier(tier, NULL);
                    sRoster->mode = MODE_SELECT_MOVE;
                    sRoster->subMenuIndex = 0;
                    PlaySE(SE_SELECT);
                }
                else
                {
                    PlaySE(SE_BOO);
                }
                break;

            case 3: // Imposta / Rimuovi Starter Run
                if (gSaveBlock2Ptr->metaProgression.chosenGachaSpecies == sRoster->currentSpecies)
                {
                    gSaveBlock2Ptr->metaProgression.chosenGachaSpecies = SPECIES_NONE;
                    PlaySE(SE_BALL_BOUNCE_1);
                }
                else
                {
                    gSaveBlock2Ptr->metaProgression.chosenGachaSpecies = sRoster->currentSpecies;
                    PlaySE(SE_PIN);
                }
                DrawDetailBuildScreen();
                break;

            case 4: // Torna alla Lista
                PlaySE(SE_SELECT);
                sRoster->mode = MODE_GEN_BROWSER;
                DrawBrowserScreen();
                break;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->mode = MODE_GEN_BROWSER;
            DrawBrowserScreen();
        }
        break;

    case MODE_SELECT_NATURE:
        // Nature Cycling
        if (JOY_NEW(DPAD_UP))
        {
            if (sRoster->subMenuIndex > 0)
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex--;
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sRoster->subMenuIndex + 1 < ARRAY_COUNT(sCompetitiveNatures))
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex++;
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 tier = GetSpeciesRarityTier(sRoster->currentSpecies);
            if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] >= 5)
            {
                gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 5;
                u8 chosenNat = sCompetitiveNatures[sRoster->subMenuIndex];
                SetBoxMonData(sRoster->selectedBoxMon, MON_DATA_HIDDEN_NATURE, &chosenNat);
                PlaySE(SE_PIN);
            }
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        break;

    case MODE_SELECT_ABILITY:
        if (JOY_NEW(DPAD_UP))
        {
            if (sRoster->subMenuIndex > 0)
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex--;
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sRoster->subMenuIndex < 2)
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex++;
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 tier = GetSpeciesRarityTier(sRoster->currentSpecies);
            if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] >= 10)
            {
                u16 chosenAbil = GetMonAbilityAtSlot(sRoster->currentSpecies, sRoster->subMenuIndex);
                if (chosenAbil != ABILITY_NONE)
                {
                    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 10;
                    SetBoxMonData(sRoster->selectedBoxMon, MON_DATA_CUSTOM_ABILITY, &chosenAbil);
                    PlaySE(SE_PIN);
                }
            }
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        break;

    case MODE_SELECT_MOVE:
        if (JOY_NEW(DPAD_UP))
        {
            if (sRoster->subMenuIndex > 0)
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex--;
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sRoster->subMenuIndex < 2)
            {
                PlaySE(SE_SELECT);
                sRoster->subMenuIndex++;
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 tier = GetSpeciesRarityTier(sRoster->currentSpecies);
            if (gSaveBlock2Ptr->metaProgression.rarityPoints[tier] >= 10)
            {
                u16 chosenMove = sRoster->moveChoices[sRoster->subMenuIndex];
                if (chosenMove != MOVE_NONE)
                {
                    gSaveBlock2Ptr->metaProgression.rarityPoints[tier] -= 10;
                    SetBoxMonData(sRoster->selectedBoxMon, MON_DATA_MOVE1, &chosenMove);
                    u8 pp = gMovesInfo[chosenMove].pp;
                    SetBoxMonData(sRoster->selectedBoxMon, MON_DATA_PP1, &pp);
                    PlaySE(SE_PIN);
                }
            }
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sRoster->mode = MODE_DETAIL_BUILD;
            DrawDetailBuildScreen();
        }
        break;

    case MODE_EXIT:
        if (!gPaletteFade.active)
        {
            CleanPreviewSprite();
            FreeAllWindowBuffers();
            Free(sRoster);
            sRoster = NULL;
            DestroyTask(taskId);
            SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        }
        break;
    }
}
