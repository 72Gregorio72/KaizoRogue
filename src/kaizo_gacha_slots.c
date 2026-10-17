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
#include "window.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Tags for Gacha Egg Sprites
#define GFXTAG_GACHA_EGG        0x2800
#define PALTAG_GACHA_COMMON     0x2801
#define PALTAG_GACHA_UNCOMMON   0x2802
#define PALTAG_GACHA_RARE       0x2803
#define PALTAG_GACHA_EPIC       0x2804

#define NUM_REELS 3
#define EGGS_PER_REEL 3

// Reel Center Coordinates
#define REEL_Y_CENTER 56
static const s16 sReelXCoords[NUM_REELS] = { 60, 120, 180 };

// Palettes for the 4 Egg Tiers
static const u16 sEggPal_Common[16] = {
    RGB(0, 0, 0),       // transparent
    RGB(8, 9, 9),       // outline
    RGB(16, 17, 18),    // shadow
    RGB(30, 30, 28),    // highlight
    RGB(28, 28, 26),    // body
    RGB(23, 23, 22),    // soft shading
    RGB(16, 17, 18),    // spots primary - Slate Grey
    RGB(10, 11, 12),    // spots shadow - Dark Grey
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_Uncommon[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(30, 30, 28),
    RGB(28, 28, 26),
    RGB(23, 23, 22),
    RGB(6, 26, 8),      // spots primary - Vibrant Green
    RGB(3, 16, 4),      // spots shadow - Deep Green
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_Rare[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(30, 30, 28),
    RGB(28, 28, 26),
    RGB(23, 23, 22),
    RGB(6, 16, 30),     // spots primary - Sapphire Blue
    RGB(3, 8, 20),      // spots shadow - Deep Blue
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_Epic[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(30, 30, 28),
    RGB(28, 28, 26),
    RGB(23, 23, 22),
    RGB(30, 5, 8),      // spots primary - Crimson Red
    RGB(18, 2, 4),      // spots shadow - Maroon Red
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u8 sEggHatchTiles[] = INCGFX_U8("graphics/pokemon/egg/hatch.png", ".4bpp");

static const struct OamData sOamData_Egg =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sAnim_EggNormal[] =
{
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_EggCrack1[] =
{
    ANIMCMD_FRAME(16, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_EggCrack2[] =
{
    ANIMCMD_FRAME(32, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_Egg[] =
{
    sAnim_EggNormal,
    sAnim_EggCrack1,
    sAnim_EggCrack2,
};

static const struct SpriteSheet sEggSpriteSheet =
{
    .data = sEggHatchTiles,
    .size = sizeof(sEggHatchTiles),
    .tag = GFXTAG_GACHA_EGG
};

static const struct SpritePalette sEggSpritePalettes[] =
{
    { sEggPal_Common,   PALTAG_GACHA_COMMON },
    { sEggPal_Uncommon, PALTAG_GACHA_UNCOMMON },
    { sEggPal_Rare,     PALTAG_GACHA_RARE },
    { sEggPal_Epic,     PALTAG_GACHA_EPIC },
    {}
};

static const struct SpriteTemplate sSpriteTemplate_Egg =
{
    .tileTag = GFXTAG_GACHA_EGG,
    .paletteTag = PALTAG_GACHA_COMMON,
    .oam = &sOamData_Egg,
    .anims = sAnims_Egg,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

enum GachaState
{
    GACHA_STATE_START_ROLL,
    GACHA_STATE_SPINNING,
    GACHA_STATE_STOP_REEL0,
    GACHA_STATE_STOP_REEL1,
    GACHA_STATE_STOP_REEL2,
    GACHA_STATE_ALIGN_WAIT,
    GACHA_STATE_HATCH_SHAKE,
    GACHA_STATE_HATCH_BURST,
    GACHA_STATE_SHOW_RESULT,
    GACHA_STATE_WAIT_DUPLICATE_CHOICE,
    GACHA_STATE_WAIT_NEXT,
    GACHA_STATE_EXIT
};

struct GachaSlotData
{
    u8 state;
    u16 timer;
    u8 rolledTier;
    u16 hatchedSpecies;
    struct Pokemon hatchedMon;
    u8 monSpriteId;
    u8 eggSpriteIds[NUM_REELS][EGGS_PER_REEL];
    bool8 reelStopped[NUM_REELS];
    s16 reelYOffsets[NUM_REELS];
    u8 reelTiers[NUM_REELS][EGGS_PER_REEL];
    u8 textWindowId;
    u8 headerWindowId;
    bool8 isDuplicate;
    struct BoxPokemon existingMon;
};

static EWRAM_DATA struct GachaSlotData *sGacha = NULL;

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
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 28,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 1
    },
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 12,
        .width = 28,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 85
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sSlotBgPalette[] =
{
    RGB(2, 2, 6),       // 0: Deep casino purple
    RGB(31, 31, 31),    // 1: White
    RGB(28, 22, 8),     // 2: Gold highlight
    RGB(18, 12, 4),     // 3: Gold shadow
    RGB(10, 8, 20),     // 4: Indigo border
    RGB(15, 0, 0),      // 5: Red banner
    RGB(0, 15, 0),      // 6: Green
    RGB(0, 10, 25),     // 7: Blue
    RGB(5, 5, 12),      // 8: Dark slot frame
    RGB(20, 20, 24),    // 9: Light gray
    RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0)
};

static void CB2_GachaMain(void);
static void VBlankCB_Gacha(void);
static void Task_GachaLoop(u8 taskId);
static void UpdateHeaderWindow(void);
static void PrintResultWindow(void);
static void StartRoll(void);
static void CleanSprites(void);

void OpenGachaSlotMachine(void)
{
    InitKaizoMetaProgression();
    if (gSaveBlock2Ptr->metaProgression.gachaTickets < 1)
    {
        // Not enough tickets: return to overworld immediately
        ScriptContext_Enable();
        return;
    }

    sGacha = AllocZeroed(sizeof(struct GachaSlotData));
    if (sGacha == NULL)
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
    LoadPalette(sSlotBgPalette, BG_PLTT_ID(0), sizeof(sSlotBgPalette));
    LoadPalette(GetTextWindowPalette(0), BG_PLTT_ID(15), 32);

    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetAllPicSprites();

    LoadSpriteSheet(&sEggSpriteSheet);
    LoadSpritePalettes(sEggSpritePalettes);

    sGacha->headerWindowId = 0;
    sGacha->textWindowId = 1;
    sGacha->monSpriteId = 0xFF;

    // Draw dark slot frame on BG1
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
    CopyBgTilemapBufferToVram(1);
    ShowBg(0);
    ShowBg(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_OBJ_ON);

    CreateTask(Task_GachaLoop, 0);

    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);

    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_Gacha);
    SetMainCallback2(CB2_GachaMain);

    StartRoll();
}

static void CB2_GachaMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_Gacha(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void StartRoll(void)
{
    if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
        gSaveBlock2Ptr->metaProgression.gachaTickets--;

    CleanSprites();

    sGacha->rolledTier = RollRarityTier(CATEGORY_POKEMON, LOOT_CONTEXT_STANDARD, NULL);
    if (sGacha->rolledTier == RARITY_NONE || sGacha->rolledTier > RARITY_EPIC)
        sGacha->rolledTier = RARITY_COMMON;

    u16 species = GetRandomRaritySpeciesByTier(sGacha->rolledTier, NULL);
    if (species == SPECIES_NONE)
        species = SPECIES_CHARMANDER;

    while (GetSpeciesPreEvolution(species) != SPECIES_NONE)
        species = GetSpeciesPreEvolution(species);

    sGacha->hatchedSpecies = species;

    // Generate Level 5 Mon
    ZeroMonData(&sGacha->hatchedMon);
    CreateMon(&sGacha->hatchedMon, species, 5, Random32(), OTID_STRUCT_PLAYER_ID);
    u16 scratch = MOVE_SCRATCH;
    u16 noneMove = MOVE_NONE;
    SetMonData(&sGacha->hatchedMon, MON_DATA_MOVE1, &scratch);
    SetMonData(&sGacha->hatchedMon, MON_DATA_MOVE2, &noneMove);
    SetMonData(&sGacha->hatchedMon, MON_DATA_MOVE3, &noneMove);
    SetMonData(&sGacha->hatchedMon, MON_DATA_MOVE4, &noneMove);
    u8 pp = gMovesInfo[MOVE_SCRATCH].pp;
    u8 zeroPP = 0;
    SetMonData(&sGacha->hatchedMon, MON_DATA_PP1, &pp);
    SetMonData(&sGacha->hatchedMon, MON_DATA_PP2, &zeroPP);
    SetMonData(&sGacha->hatchedMon, MON_DATA_PP3, &zeroPP);
    SetMonData(&sGacha->hatchedMon, MON_DATA_PP4, &zeroPP);
    CalculateMonStats(&sGacha->hatchedMon);

    // Check Duplicate in PC Roster
    struct BoxPokemon *existing = GetUnlockedRosterMon(species);
    if (existing != NULL)
    {
        sGacha->isDuplicate = TRUE;
        sGacha->existingMon = *existing;
    }
    else
    {
        sGacha->isDuplicate = FALSE;
    }

    // Spawn 3 reels of eggs
    for (int r = 0; r < NUM_REELS; r++)
    {
        sGacha->reelStopped[r] = FALSE;
        sGacha->reelYOffsets[r] = 0;
        for (int i = 0; i < EGGS_PER_REEL; i++)
        {
            u8 palIndex = (r + i) % 4;
            u8 spriteId = CreateSprite(&sSpriteTemplate_Egg, sReelXCoords[r], REEL_Y_CENTER + (i - 1) * 32, 2);
            gSprites[spriteId].oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_GACHA_COMMON + palIndex);
            sGacha->eggSpriteIds[r][i] = spriteId;
            sGacha->reelTiers[r][i] = palIndex + 1;
        }
    }

    sGacha->state = GACHA_STATE_SPINNING;
    sGacha->timer = 0;
    UpdateHeaderWindow();

    // Clear Text Window
    FillWindowPixelBuffer(sGacha->textWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sGacha->textWindowId);
    CopyWindowToVram(sGacha->textWindowId, COPYWIN_FULL);
}

static void CleanSprites(void)
{
    if (sGacha->monSpriteId != 0xFF)
    {
        FreeAndDestroyMonPicSprite(sGacha->monSpriteId);
        sGacha->monSpriteId = 0xFF;
    }
    for (int r = 0; r < NUM_REELS; r++)
    {
        for (int i = 0; i < EGGS_PER_REEL; i++)
        {
            if (sGacha->eggSpriteIds[r][i] != 0 && gSprites[sGacha->eggSpriteIds[r][i]].inUse)
            {
                DestroySprite(&gSprites[sGacha->eggSpriteIds[r][i]]);
                sGacha->eggSpriteIds[r][i] = 0;
            }
        }
    }
}

static void UpdateHeaderWindow(void)
{
    FillWindowPixelBuffer(sGacha->headerWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sGacha->headerWindowId);

    static const u8 sTextTitle[] = _("{COLOR BLUE}GACHA MACHINE SLOTS");
    static const u8 sTextTickets[] = _("Biglietti: ");

    AddTextPrinterParameterized(sGacha->headerWindowId, 2, sTextTitle, 4, 1, 0, NULL);

    u8 buf[32];
    u8 *ptr = StringCopy(buf, sTextTickets);
    ConvertIntToDecimalStringN(ptr, gSaveBlock2Ptr->metaProgression.gachaTickets, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sGacha->headerWindowId, 2, buf, 150, 1, 0, NULL);

    CopyWindowToVram(sGacha->headerWindowId, COPYWIN_FULL);
}

static void Task_GachaLoop(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    switch (sGacha->state)
    {
    case GACHA_STATE_SPINNING:
        sGacha->timer++;

        // Rapid vertical reel animation
        for (int r = 0; r < NUM_REELS; r++)
        {
            if (!sGacha->reelStopped[r])
            {
                sGacha->reelYOffsets[r] = (sGacha->reelYOffsets[r] + 8) % 32;
                for (int i = 0; i < EGGS_PER_REEL; i++)
                {
                    u8 sId = sGacha->eggSpriteIds[r][i];
                    gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32 + sGacha->reelYOffsets[r];
                }
            }
        }

        if ((sGacha->timer % 8) == 0)
            PlaySE(SE_ROULETTE_BALL);

        // Reel 0 stops after 45 frames
        if (sGacha->timer >= 45)
        {
            sGacha->state = GACHA_STATE_STOP_REEL0;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_STOP_REEL0:
        sGacha->reelStopped[0] = TRUE;
        for (int i = 0; i < EGGS_PER_REEL; i++)
        {
            u8 sId = sGacha->eggSpriteIds[0][i];
            gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32;
            // Center egg on rolled tier
            if (i == 1)
                gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_GACHA_COMMON + sGacha->rolledTier - 1);
        }
        PlaySE(SE_BALL_BOUNCE_1);

        sGacha->state = GACHA_STATE_STOP_REEL1;
        sGacha->timer = 0;
        break;

    case GACHA_STATE_STOP_REEL1:
        sGacha->timer++;
        // Spin remaining reels
        for (int r = 1; r < NUM_REELS; r++)
        {
            if (!sGacha->reelStopped[r])
            {
                sGacha->reelYOffsets[r] = (sGacha->reelYOffsets[r] + 8) % 32;
                for (int i = 0; i < EGGS_PER_REEL; i++)
                {
                    u8 sId = sGacha->eggSpriteIds[r][i];
                    gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32 + sGacha->reelYOffsets[r];
                }
            }
        }
        if ((sGacha->timer % 8) == 0)
            PlaySE(SE_ROULETTE_BALL);

        // Reel 1 stops after 35 frames
        if (sGacha->timer >= 35)
        {
            sGacha->reelStopped[1] = TRUE;
            for (int i = 0; i < EGGS_PER_REEL; i++)
            {
                u8 sId = sGacha->eggSpriteIds[1][i];
                gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32;
                if (i == 1)
                    gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_GACHA_COMMON + sGacha->rolledTier - 1);
            }
            PlaySE(SE_BALL_BOUNCE_2);

            sGacha->state = GACHA_STATE_STOP_REEL2;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_STOP_REEL2:
        sGacha->timer++;
        if (!sGacha->reelStopped[2])
        {
            u8 speed = (sGacha->rolledTier >= RARITY_RARE && sGacha->timer > 30) ? 4 : 8;
            sGacha->reelYOffsets[2] = (sGacha->reelYOffsets[2] + speed) % 32;
            for (int i = 0; i < EGGS_PER_REEL; i++)
            {
                u8 sId = sGacha->eggSpriteIds[2][i];
                gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32 + sGacha->reelYOffsets[2];
            }
            if ((sGacha->timer % 6) == 0)
                PlaySE(SE_ROULETTE_BALL2);
        }

        // Rarity dependent suspense & stopping time!
        if ((sGacha->rolledTier == RARITY_COMMON && sGacha->timer >= 25) ||
            (sGacha->rolledTier == RARITY_UNCOMMON && sGacha->timer >= 45) ||
            (sGacha->rolledTier == RARITY_RARE && sGacha->timer >= 75) ||
            (sGacha->rolledTier == RARITY_EPIC && sGacha->timer >= 110))
        {
            sGacha->reelStopped[2] = TRUE;
            for (int i = 0; i < EGGS_PER_REEL; i++)
            {
                u8 sId = sGacha->eggSpriteIds[2][i];
                gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32;
                if (i == 1)
                    gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_GACHA_COMMON + sGacha->rolledTier - 1);
            }

            // Play jingle / fanfare according to tier!
            if (sGacha->rolledTier == RARITY_EPIC)
            {
                PlayFanfare(MUS_SLOTS_JACKPOT);
                BlendPalettes(PALETTES_ALL, 10, RGB(31, 0, 0));
                BeginNormalPaletteFade(PALETTES_ALL, 0, 10, 0, RGB_BLACK);
            }
            else if (sGacha->rolledTier == RARITY_RARE)
            {
                PlaySE(SE_DING_DONG);
                BlendPalettes(PALETTES_ALL, 8, RGB(0, 20, 31));
                BeginNormalPaletteFade(PALETTES_ALL, 0, 8, 0, RGB_BLACK);
            }
            else if (sGacha->rolledTier == RARITY_UNCOMMON)
            {
                PlaySE(SE_PIN);
            }
            else
            {
                PlaySE(SE_BALL_BOUNCE_3);
            }

            sGacha->state = GACHA_STATE_ALIGN_WAIT;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_ALIGN_WAIT:
        sGacha->timer++;
        if (sGacha->timer >= 40)
        {
            // Hide side eggs, prepare center egg for hatch
            for (int i = 0; i < EGGS_PER_REEL; i++)
            {
                gSprites[sGacha->eggSpriteIds[0][i]].invisible = TRUE;
                gSprites[sGacha->eggSpriteIds[2][i]].invisible = TRUE;
                if (i != 1)
                    gSprites[sGacha->eggSpriteIds[1][i]].invisible = TRUE;
            }
            sGacha->state = GACHA_STATE_HATCH_SHAKE;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_HATCH_SHAKE:
        sGacha->timer++;
        u8 centerEgg = sGacha->eggSpriteIds[1][1];

        if (sGacha->timer == 10)
        {
            gSprites[centerEgg].x2 = -2;
            PlaySE(SE_BALL_BOUNCE_1);
        }
        else if (sGacha->timer == 18)
        {
            gSprites[centerEgg].x2 = 2;
        }
        else if (sGacha->timer == 26)
        {
            gSprites[centerEgg].x2 = 0;
            StartSpriteAnim(&gSprites[centerEgg], 1); // Crack 1
            PlaySE(SE_BALL_BOUNCE_2);
        }
        else if (sGacha->timer == 45)
        {
            gSprites[centerEgg].x2 = -3;
            StartSpriteAnim(&gSprites[centerEgg], 2); // Crack 2
            PlaySE(SE_BALL_BOUNCE_3);
        }
        else if (sGacha->timer == 55)
        {
            gSprites[centerEgg].x2 = 3;
        }
        else if (sGacha->timer >= 70)
        {
            gSprites[centerEgg].x2 = 0;
            gSprites[centerEgg].invisible = TRUE;
            PlaySE(SE_BALL_OPEN);

            sGacha->state = GACHA_STATE_HATCH_BURST;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_HATCH_BURST:
        // Spawn Hatched Pokémon Front Sprite
        sGacha->monSpriteId = CreateMonPicSprite(sGacha->hatchedSpecies, FALSE, 0, TRUE, 120, 56, 14, TAG_NONE);
        PlayCry_Normal(sGacha->hatchedSpecies, 0);

        sGacha->state = GACHA_STATE_SHOW_RESULT;
        sGacha->timer = 0;
        break;

    case GACHA_STATE_SHOW_RESULT:
        sGacha->timer++;
        if (sGacha->timer >= 20)
        {
            PrintResultWindow();
            if (sGacha->isDuplicate)
            {
                sGacha->state = GACHA_STATE_WAIT_DUPLICATE_CHOICE;
            }
            else
            {
                AddUnlockedRosterMon(&sGacha->hatchedMon.box);
                sGacha->state = GACHA_STATE_WAIT_NEXT;
            }
        }
        break;

    case GACHA_STATE_WAIT_DUPLICATE_CHOICE:
        if (JOY_NEW(A_BUTTON))
        {
            // Keep NEW, fuse OLD for +10 points
            AddUnlockedRosterMon(&sGacha->hatchedMon.box);
            AddRarityPoints(sGacha->rolledTier, 10);
            PlaySE(SE_PIN);
            UpdateHeaderWindow();
            PrintResultWindow();
            sGacha->state = GACHA_STATE_WAIT_NEXT;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            // Keep OLD, fuse NEW for +10 points
            AddRarityPoints(sGacha->rolledTier, 10);
            PlaySE(SE_PIN);
            UpdateHeaderWindow();
            PrintResultWindow();
            sGacha->state = GACHA_STATE_WAIT_NEXT;
        }
        break;

    case GACHA_STATE_WAIT_NEXT:
        if (JOY_NEW(A_BUTTON))
        {
            if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
            {
                PlaySE(SE_SELECT);
                StartRoll();
            }
            else
            {
                PlaySE(SE_BOO);
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sGacha->state = GACHA_STATE_EXIT;
        }
        break;

    case GACHA_STATE_EXIT:
        if (!gPaletteFade.active)
        {
            CleanSprites();
            FreeAllWindowBuffers();
            Free(sGacha);
            sGacha = NULL;
            DestroyTask(taskId);
            SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        }
        break;
    }
}

static const u8 sText_HaiSchiuso[] = _("HAI SCHIUSO ");
static const u8 sText_NatLabel[]   = _("Nat: ");
static const u8 sText_AbilLabel[]  = _(" / Abil: ");
static const u8 sText_IvLabel[]    = _("IV: ");
static const u8 sText_Slash[]      = _("/");
static const u8 sText_TotOpen[]    = _(" (Tot: ");
static const u8 sText_TotClose[]   = _("/186)");
static const u8 sText_ParenOpen[]  = _(" (");
static const u8 sText_ParenClose[] = _(")");
static const u8 sText_PromptDup[]  = _("(A) Tieni Nuovo (+10Pt)  (B) Tieni Vecchio");
static const u8 sText_PromptMore[] = _("(A) Roll Ancora   (B) Esci");
static const u8 sText_PromptDone[] = _("(B) Esci (Biglietti esauriti)");

static void PrintResultWindow(void)
{
    FillWindowPixelBuffer(sGacha->textWindowId, PIXEL_FILL(1));
    PutWindowTilemap(sGacha->textWindowId);

    u8 line1[40];
    u8 line2[40];
    u8 line3[48];
    u8 line4[48];

    u8 *ptr = StringCopy(line1, sText_HaiSchiuso);
    ptr = StringAppend(ptr, gSpeciesInfo[sGacha->hatchedSpecies].speciesName);
    ptr = StringAppend(ptr, sText_ParenOpen);
    ptr = StringAppend(ptr, GetRarityTierName(sGacha->rolledTier));
    StringAppend(ptr, sText_ParenClose);

    u8 nat = GetNature(&sGacha->hatchedMon);
    u16 abil = GetMonAbility(&sGacha->hatchedMon);
    ptr = StringCopy(line2, sText_NatLabel);
    ptr = StringAppend(ptr, gNaturesInfo[nat].name);
    ptr = StringAppend(ptr, sText_AbilLabel);
    StringAppend(ptr, gAbilitiesInfo[abil].name);

    u8 ivHp  = GetMonData(&sGacha->hatchedMon, MON_DATA_HP_IV);
    u8 ivAtk = GetMonData(&sGacha->hatchedMon, MON_DATA_ATK_IV);
    u8 ivDef = GetMonData(&sGacha->hatchedMon, MON_DATA_DEF_IV);
    u8 ivSpe = GetMonData(&sGacha->hatchedMon, MON_DATA_SPEED_IV);
    u8 ivSpA = GetMonData(&sGacha->hatchedMon, MON_DATA_SPATK_IV);
    u8 ivSpD = GetMonData(&sGacha->hatchedMon, MON_DATA_SPDEF_IV);
    u16 ivTot = ivHp + ivAtk + ivDef + ivSpe + ivSpA + ivSpD;

    ptr = StringCopy(line3, sText_IvLabel);
    ptr = ConvertIntToDecimalStringN(ptr, ivHp, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, ivAtk, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, ivDef, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, ivSpA, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, ivSpD, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_Slash);
    ptr = ConvertIntToDecimalStringN(ptr, ivSpe, STR_CONV_MODE_LEFT_ALIGN, 2);
    ptr = StringAppend(ptr, sText_TotOpen);
    ptr = ConvertIntToDecimalStringN(ptr, ivTot, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(ptr, sText_TotClose);

    if (sGacha->isDuplicate && sGacha->state == GACHA_STATE_WAIT_DUPLICATE_CHOICE)
        StringCopy(line4, sText_PromptDup);
    else if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
        StringCopy(line4, sText_PromptMore);
    else
        StringCopy(line4, sText_PromptDone);

    AddTextPrinterParameterized(sGacha->textWindowId, 2, line1, 2, 1, 0, NULL);
    AddTextPrinterParameterized(sGacha->textWindowId, 2, line2, 2, 13, 0, NULL);
    AddTextPrinterParameterized(sGacha->textWindowId, 2, line3, 2, 25, 0, NULL);
    AddTextPrinterParameterized(sGacha->textWindowId, 2, line4, 2, 38, 0, NULL);

    CopyWindowToVram(sGacha->textWindowId, COPYWIN_FULL);
}
