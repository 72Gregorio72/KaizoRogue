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
// Sweet Casino Infinite Checkerboard Pattern (BG1)
static const u16 ALIGNED(4) sCheckerBg_Pal[16] = {
    RGB(0, 0, 0),        // 0: Transparent
    RGB(31, 29, 25),     // 1: Vanilla Cream Main
    RGB(31, 31, 29),     // 2: Vanilla Cream Highlight
    RGB(28, 25, 21),     // 3: Vanilla Cream Shadow
    RGB(29, 17, 19),     // 4: Sweet Strawberry Coral Main
    RGB(31, 20, 22),     // 5: Strawberry Coral Highlight
    RGB(25, 14, 16),     // 6: Strawberry Coral Shadow
    RGB(31, 26, 10),     // 7: Warm Golden Accent (center diamond)
    RGB(31, 29, 18),     // 8: Warm Golden Highlight
    RGB(26, 21, 8),      // 9: Warm Golden Shadow
    RGB(24, 21, 18),     // 10: Grid Seam Line
    RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0)
};

static const u8 ALIGNED(4) sCheckerBg_Gfx[288] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2A, 0x22, 0x22, 0x22, 0x12, 0x11, 0x11, 0x11, 0x12, 0x11, 0x11, 0x11, 0x12, 0x11, 0x11, 0x11,
    0x12, 0x11, 0x11, 0x11, 0x12, 0x11, 0x11, 0x81, 0x12, 0x11, 0x11, 0x88, 0x12, 0x11, 0x81, 0x78,
    0x22, 0x22, 0x22, 0xA2, 0x11, 0x11, 0x11, 0x31, 0x11, 0x11, 0x11, 0x31, 0x11, 0x11, 0x11, 0x31,
    0x11, 0x11, 0x11, 0x31, 0x18, 0x11, 0x11, 0x31, 0x97, 0x11, 0x11, 0x31, 0x97, 0x19, 0x11, 0x31,
    0x12, 0x11, 0x81, 0x78, 0x12, 0x11, 0x11, 0x97, 0x12, 0x11, 0x11, 0x91, 0x12, 0x11, 0x11, 0x11,
    0x12, 0x11, 0x11, 0x11, 0x12, 0x11, 0x11, 0x11, 0x12, 0x11, 0x11, 0x11, 0x3A, 0x33, 0x33, 0x33,
    0x97, 0x19, 0x11, 0x31, 0x99, 0x11, 0x11, 0x31, 0x19, 0x11, 0x11, 0x31, 0x11, 0x11, 0x11, 0x31,
    0x11, 0x11, 0x11, 0x31, 0x11, 0x11, 0x11, 0x31, 0x11, 0x11, 0x11, 0x31, 0x33, 0x33, 0x33, 0xA3,
    0x5A, 0x55, 0x55, 0x55, 0x45, 0x44, 0x44, 0x44, 0x45, 0x44, 0x44, 0x44, 0x45, 0x44, 0x44, 0x44,
    0x45, 0x44, 0x44, 0x44, 0x45, 0x44, 0x44, 0x84, 0x45, 0x44, 0x44, 0x88, 0x45, 0x44, 0x84, 0x78,
    0x55, 0x55, 0x55, 0xA5, 0x44, 0x44, 0x44, 0x64, 0x44, 0x44, 0x44, 0x64, 0x44, 0x44, 0x44, 0x64,
    0x44, 0x44, 0x44, 0x64, 0x48, 0x44, 0x44, 0x64, 0x97, 0x44, 0x44, 0x64, 0x97, 0x49, 0x44, 0x64,
    0x45, 0x44, 0x84, 0x78, 0x45, 0x44, 0x44, 0x97, 0x45, 0x44, 0x44, 0x94, 0x45, 0x44, 0x44, 0x44,
    0x45, 0x44, 0x44, 0x44, 0x45, 0x44, 0x44, 0x44, 0x45, 0x44, 0x44, 0x44, 0x6A, 0x66, 0x66, 0x66,
    0x97, 0x49, 0x44, 0x64, 0x99, 0x44, 0x44, 0x64, 0x49, 0x44, 0x44, 0x64, 0x44, 0x44, 0x44, 0x64,
    0x44, 0x44, 0x44, 0x64, 0x44, 0x44, 0x44, 0x64, 0x44, 0x44, 0x44, 0x64, 0x66, 0x66, 0x66, 0xA6,
};

// Tags for Gacha Egg Sprites
#define GFXTAG_GACHA_EGG            0x2800
#define PALTAG_GACHA_COMMON         0x2801
#define PALTAG_GACHA_UNCOMMON       0x2802
#define PALTAG_GACHA_RARE           0x2803
#define PALTAG_GACHA_EPIC           0x2804
#define PALTAG_GACHA_SHINY_COMMON   0x2805
#define PALTAG_GACHA_SHINY_UNCOMMON 0x2806
#define PALTAG_GACHA_SHINY_RARE     0x2807
#define PALTAG_GACHA_SHINY_EPIC     0x2808

#define NUM_REELS 3
#define EGGS_PER_REEL 3

// Reel Coordinates
#define REEL_Y_CENTER 99
static const s16 sReelXCoords[NUM_REELS] = { 65, 120, 175 };

// Normal Egg Palettes (White/Cream body, Tier-colored spots)
static const u16 sEggPal_Common[16] = {
    RGB(0, 0, 0),       // transparent
    RGB(8, 9, 9),       // outline
    RGB(16, 17, 18),    // shadow
    RGB(30, 30, 28),    // highlight
    RGB(28, 28, 26),    // body (white/cream)
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

// Shiny Egg Palettes (Yellow/Gold body instead of white, Tier-colored spots)
static const u16 sEggPal_ShinyCommon[16] = {
    RGB(0, 0, 0),       // transparent
    RGB(8, 9, 9),       // outline
    RGB(16, 17, 18),    // shadow
    RGB(31, 31, 14),    // highlight (sparkling pale yellow)
    RGB(31, 26, 4),     // body (vibrant golden yellow)
    RGB(26, 20, 2),     // soft shading (rich amber gold)
    RGB(16, 17, 18),    // spots primary - Slate Grey
    RGB(10, 11, 12),    // spots shadow - Dark Grey
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_ShinyUncommon[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(31, 31, 14),
    RGB(31, 26, 4),
    RGB(26, 20, 2),
    RGB(6, 26, 8),      // spots primary - Vibrant Green
    RGB(3, 16, 4),      // spots shadow - Deep Green
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_ShinyRare[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(31, 31, 14),
    RGB(31, 26, 4),
    RGB(26, 20, 2),
    RGB(6, 16, 30),     // spots primary - Sapphire Blue
    RGB(3, 8, 20),      // spots shadow - Deep Blue
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static const u16 sEggPal_ShinyEpic[16] = {
    RGB(0, 0, 0),
    RGB(8, 9, 9),
    RGB(16, 17, 18),
    RGB(31, 31, 14),
    RGB(31, 26, 4),
    RGB(26, 20, 2),
    RGB(30, 5, 8),      // spots primary - Crimson Red
    RGB(18, 2, 4),      // spots shadow - Maroon Red
    RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(28, 0, 0), RGB(16, 28, 28)
};

static u16 GetEggPaletteTag(u8 tier, bool8 isShiny)
{
    if (tier < RARITY_COMMON || tier > RARITY_EPIC)
        tier = RARITY_COMMON;
    if (isShiny)
        return PALTAG_GACHA_SHINY_COMMON + tier - 1;
    return PALTAG_GACHA_COMMON + tier - 1;
}

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
    { sEggPal_Common,        PALTAG_GACHA_COMMON },
    { sEggPal_Uncommon,      PALTAG_GACHA_UNCOMMON },
    { sEggPal_Rare,          PALTAG_GACHA_RARE },
    { sEggPal_Epic,          PALTAG_GACHA_EPIC },
    { sEggPal_ShinyCommon,   PALTAG_GACHA_SHINY_COMMON },
    { sEggPal_ShinyUncommon, PALTAG_GACHA_SHINY_UNCOMMON },
    { sEggPal_ShinyRare,     PALTAG_GACHA_SHINY_RARE },
    { sEggPal_ShinyEpic,     PALTAG_GACHA_SHINY_EPIC },
    {}
};

static const struct SpriteTemplate sSpriteTemplate_GachaEgg =
{
    .tileTag = GFXTAG_GACHA_EGG,
    .paletteTag = PALTAG_GACHA_COMMON,
    .oam = &sOamData_Egg,
    .anims = sAnims_Egg,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

#define MULTI_PULL_COUNT 10
#define EGG_TARGET_Y     78

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
    GACHA_STATE_WAIT_NEXT,
    GACHA_STATE_MULTI_EGG_DROP,
    GACHA_STATE_MULTI_EGGS_READY,
    GACHA_STATE_MULTI_SHOW_MON,
    GACHA_STATE_MULTI_WAIT_INPUT,
    GACHA_STATE_EXIT
};

enum {
    WIN_HEADER,     // Top Credit & Payout display
    WIN_MON_INFO,   // Left: Hatched Pokemon info
    WIN_STAT_CARD,  // Right: IV Stats & Grades
    WIN_PROMPT,     // Bottom: Controls
    WIN_COUNT
};

struct GachaPullResult
{
    u8 tier;
    u16 species;
    struct Pokemon mon;
    bool8 isDuplicate;
    struct BoxPokemon existingMon;
    u8 oldIvs[6];
    u8 hatchedIvs[6];
    u8 mergedIvs[6];
};

struct GachaSlotData
{
    u8 state;
    u16 timer;
    bool8 isMultiPull;
    u8 multiIndex;
    u8 multiEggSpriteIds[MULTI_PULL_COUNT];
    s16 multiEggY[MULTI_PULL_COUNT];
    s16 multiEggSpeedY[MULTI_PULL_COUNT];
    bool8 multiEggLanded[MULTI_PULL_COUNT];
    struct GachaPullResult pulls[MULTI_PULL_COUNT];

    u16 bgScrollTimer;
    u16 bgScrollX;
    u16 bgScrollY;

    // Single pull / active display
    u8 rolledTier;
    u16 hatchedSpecies;
    struct Pokemon hatchedMon;
    u16 monSpriteId;
    u8 eggSpriteIds[NUM_REELS][EGGS_PER_REEL];
    bool8 reelStopped[NUM_REELS];
    s16 reelYOffsets[NUM_REELS];
    u8 reelTiers[NUM_REELS][EGGS_PER_REEL];
    bool8 isDuplicate;
    struct BoxPokemon existingMon;
    u8 oldIvs[6];
    u8 hatchedIvs[6];
    u8 mergedIvs[6];
};

static EWRAM_DATA struct GachaSlotData *sGacha = NULL;
static EWRAM_DATA u8 sSelectedGachaBanner = 0; // 0 = All Gens, 1 = Gen 1, ..., 9 = Gen 9
static EWRAM_DATA u8 sIsMultiPullMode = 0;     // 0 = Single (1x), 1 = Multi (10x)

void Script_Gacha_SetBanner(void)
{
    sSelectedGachaBanner = (u8)gSpecialVar_0x8004;
    sIsMultiPullMode = (u8)gSpecialVar_0x8005;
}

static const s16 sMultiEggXCoords[MULTI_PULL_COUNT] = {
    12, 36, 60, 84, 108, 132, 156, 180, 204, 228
};

static const u8 sText_BannerAll[]  = _("ALL GENERATIONS");
static const u8 sText_BannerGen1[] = _("GEN 1: KANTO");
static const u8 sText_BannerGen2[] = _("GEN 2: JOHTO");
static const u8 sText_BannerGen3[] = _("GEN 3: HOENN");
static const u8 sText_BannerGen4[] = _("GEN 4: SINNOH");
static const u8 sText_BannerGen5[] = _("GEN 5: UNOVA");
static const u8 sText_BannerGen6[] = _("GEN 6: KALOS");
static const u8 sText_BannerGen7[] = _("GEN 7: ALOLA");
static const u8 sText_BannerGen8[] = _("GEN 8: GALAR");
static const u8 sText_BannerGen9[] = _("GEN 9: PALDEA");

static const u8 *const sGachaBannerNames[10] = {
    sText_BannerAll,
    sText_BannerGen1,
    sText_BannerGen2,
    sText_BannerGen3,
    sText_BannerGen4,
    sText_BannerGen5,
    sText_BannerGen6,
    sText_BannerGen7,
    sText_BannerGen8,
    sText_BannerGen9,
};

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
    [WIN_HEADER] = {
        .bg = 0,
        .tilemapLeft = 8,
        .tilemapTop = 1,
        .width = 14,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1
    },
    [WIN_MON_INFO] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 11,
        .height = 13,
        .paletteNum = 14,
        .baseBlock = 30
    },
    [WIN_STAT_CARD] = {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 1,
        .width = 13,
        .height = 13,
        .paletteNum = 15,
        .baseBlock = 175
    },
    [WIN_PROMPT] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 16,
        .width = 26,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 350
    },
    DUMMY_WIN_TEMPLATE
};

// Custom text palette for Gacha UI and colored IV grades
static const u16 sGachaTextPalette[16] = {
    RGB(0, 0, 0),        // 0: Transparent
    RGB(31, 31, 31),     // 1: White (card background)
    RGB(0, 0, 0),        // 2: Solid Black (Main Text FG)
    RGB(20, 20, 21),     // 3: Light Gray (Main Text Shadow)
    RGB(28, 4, 4),       // 4: Red (Grade A: Rossa / Epic FG)
    RGB(16, 2, 2),       // 5: Dark Red Shadow
    RGB(2, 20, 4),       // 6: Green (Grade C: Verde / Uncommon FG)
    RGB(1, 10, 2),       // 7: Dark Green Shadow
    RGB(3, 10, 28),      // 8: Blue (Grade B: Blu / Rare FG)
    RGB(1, 4, 16),       // 9: Dark Blue Shadow
    RGB(18, 4, 28),      // 10: Purple (Grade S: Viola FG)
    RGB(10, 2, 16),      // 11: Dark Purple Shadow
    RGB(31, 26, 4),      // 12: Digital Gold FG (Slot Header Credit/Payout)
    RGB(16, 12, 0),      // 13: Digital Gold Shadow
    RGB(14, 14, 15),     // 14: Gray (Grade D: Grigio FG)
    RGB(22, 22, 23),     // 15: Soft Gray Shadow
};

// Text color trios: { bg, fg, shadow }
static const u8 sColor_Dark[3]         = { TEXT_COLOR_TRANSPARENT, 2, 3 };
static const u8 sColor_Gray[3]         = { TEXT_COLOR_TRANSPARENT, 14, 15 };
static const u8 sColor_Gold[3]         = { TEXT_COLOR_TRANSPARENT, 12, 13 };
static const u8 sColor_Purple[3]       = { TEXT_COLOR_TRANSPARENT, 10, 11 }; // S: Viola
static const u8 sColor_Red[3]          = { TEXT_COLOR_TRANSPARENT, 4, 5 };   // A: Rossa
static const u8 sColor_Blue[3]         = { TEXT_COLOR_TRANSPARENT, 8, 9 };   // B: Blu
static const u8 sColor_Green[3]        = { TEXT_COLOR_TRANSPARENT, 6, 7 };   // C: Verde
static const u8 sColor_BadgeWhite[3]   = { TEXT_COLOR_TRANSPARENT, 10, 11 }; // White text on type badge pill
static const u8 sColor_TierOnPal14[3]  = { TEXT_COLOR_TRANSPARENT, 8, 9 };   // Tier color on WIN_MON_INFO (Pal 14)

struct TypeTheme
{
    u16 tint;
    u16 accent;
};

static const struct TypeTheme sTypeThemes[NUMBER_OF_MON_TYPES] = {
    [TYPE_NONE]     = { RGB(28, 28, 28), RGB(16, 16, 16) },
    [TYPE_NORMAL]   = { RGB(28, 28, 26), RGB(18, 18, 16) },
    [TYPE_FIGHTING] = { RGB(29, 23, 21), RGB(24, 7, 5) },
    [TYPE_FLYING]   = { RGB(24, 26, 30), RGB(14, 16, 28) },
    [TYPE_POISON]   = { RGB(27, 22, 28), RGB(19, 6, 22) },
    [TYPE_GROUND]   = { RGB(29, 26, 20), RGB(21, 15, 5) },
    [TYPE_ROCK]     = { RGB(27, 25, 21), RGB(17, 14, 8) },
    [TYPE_BUG]      = { RGB(26, 28, 20), RGB(15, 18, 4) },
    [TYPE_GHOST]    = { RGB(24, 22, 28), RGB(13, 9, 21) },
    [TYPE_STEEL]    = { RGB(25, 26, 27), RGB(14, 16, 18) },
    [TYPE_MYSTERY]  = { RGB(26, 28, 26), RGB(12, 18, 14) },
    [TYPE_FIRE]     = { RGB(31, 23, 19), RGB(28, 7, 2) },
    [TYPE_WATER]    = { RGB(21, 25, 31), RGB(4, 13, 28) },
    [TYPE_GRASS]    = { RGB(22, 29, 21), RGB(5, 20, 5) },
    [TYPE_ELECTRIC] = { RGB(31, 29, 19), RGB(26, 20, 0) },
    [TYPE_PSYCHIC]  = { RGB(30, 22, 26), RGB(25, 4, 15) },
    [TYPE_ICE]      = { RGB(21, 28, 30), RGB(6, 19, 24) },
    [TYPE_DRAGON]   = { RGB(23, 22, 29), RGB(11, 6, 26) },
    [TYPE_DARK]     = { RGB(22, 21, 22), RGB(8, 7, 8) },
    [TYPE_FAIRY]    = { RGB(31, 24, 28), RGB(26, 9, 18) },
    [TYPE_STELLAR]  = { RGB(28, 27, 30), RGB(18, 14, 26) },
};

static const u8 sText_TypeNormal[]   = _("NORMAL");
static const u8 sText_TypeFighting[] = _("FIGHTING");
static const u8 sText_TypeFlying[]   = _("FLYING");
static const u8 sText_TypePoison[]   = _("POISON");
static const u8 sText_TypeGround[]   = _("GROUND");
static const u8 sText_TypeRock[]     = _("ROCK");
static const u8 sText_TypeBug[]      = _("BUG");
static const u8 sText_TypeGhost[]    = _("GHOST");
static const u8 sText_TypeSteel[]    = _("STEEL");
static const u8 sText_TypeMystery[]  = _("???");
static const u8 sText_TypeFire[]     = _("FIRE");
static const u8 sText_TypeWater[]    = _("WATER");
static const u8 sText_TypeGrass[]    = _("GRASS");
static const u8 sText_TypeElectric[] = _("ELECTRIC");
static const u8 sText_TypePsychic[]  = _("PSYCHIC");
static const u8 sText_TypeIce[]      = _("ICE");
static const u8 sText_TypeDragon[]   = _("DRAGON");
static const u8 sText_TypeDark[]     = _("DARK");
static const u8 sText_TypeFairy[]    = _("FAIRY");
static const u8 sText_TypeStellar[]  = _("STELLAR");

static const u8 *const sTypeNames[NUMBER_OF_MON_TYPES] = {
    [TYPE_NONE]     = sText_TypeNormal,
    [TYPE_NORMAL]   = sText_TypeNormal,
    [TYPE_FIGHTING] = sText_TypeFighting,
    [TYPE_FLYING]   = sText_TypeFlying,
    [TYPE_POISON]   = sText_TypePoison,
    [TYPE_GROUND]   = sText_TypeGround,
    [TYPE_ROCK]     = sText_TypeRock,
    [TYPE_BUG]      = sText_TypeBug,
    [TYPE_GHOST]    = sText_TypeGhost,
    [TYPE_STEEL]    = sText_TypeSteel,
    [TYPE_MYSTERY]  = sText_TypeMystery,
    [TYPE_FIRE]     = sText_TypeFire,
    [TYPE_WATER]    = sText_TypeWater,
    [TYPE_GRASS]    = sText_TypeGrass,
    [TYPE_ELECTRIC] = sText_TypeElectric,
    [TYPE_PSYCHIC]  = sText_TypePsychic,
    [TYPE_ICE]      = sText_TypeIce,
    [TYPE_DRAGON]   = sText_TypeDragon,
    [TYPE_DARK]     = sText_TypeDark,
    [TYPE_FAIRY]    = sText_TypeFairy,
    [TYPE_STELLAR]  = sText_TypeStellar,
};

static void FormatTypeString(u8 *dest, u8 type1, u8 type2)
{
    if (type1 >= NUMBER_OF_MON_TYPES)
        type1 = TYPE_NORMAL;

    if (type2 == TYPE_NONE || type2 == type1 || type2 >= NUMBER_OF_MON_TYPES)
    {
        StringCopy(dest, sTypeNames[type1]);
    }
    else
    {
        StringCopy(dest, sTypeNames[type1]);
        StringAppend(dest, COMPOUND_STRING("/"));
        StringAppend(dest, sTypeNames[type2]);
        if (GetStringWidth(FONT_SMALL, dest, 0) > 74)
        {
            const u8 *n1 = (type1 == TYPE_ELECTRIC) ? COMPOUND_STRING("ELEC") :
                           (type1 == TYPE_FIGHTING) ? COMPOUND_STRING("FIGHT") :
                           (type1 == TYPE_POISON)   ? COMPOUND_STRING("POIS") :
                           (type1 == TYPE_PSYCHIC)  ? COMPOUND_STRING("PSY") :
                           (type1 == TYPE_GROUND)   ? COMPOUND_STRING("GRND") :
                           (type1 == TYPE_DRAGON)   ? COMPOUND_STRING("DRAG") :
                           (type1 == TYPE_NORMAL)   ? COMPOUND_STRING("NORM") :
                           sTypeNames[type1];

            const u8 *n2 = (type2 == TYPE_ELECTRIC) ? COMPOUND_STRING("ELEC") :
                           (type2 == TYPE_FIGHTING) ? COMPOUND_STRING("FIGHT") :
                           (type2 == TYPE_POISON)   ? COMPOUND_STRING("POIS") :
                           (type2 == TYPE_PSYCHIC)  ? COMPOUND_STRING("PSY") :
                           (type2 == TYPE_GROUND)   ? COMPOUND_STRING("GRND") :
                           (type2 == TYPE_DRAGON)   ? COMPOUND_STRING("DRAG") :
                           (type2 == TYPE_NORMAL)   ? COMPOUND_STRING("NORM") :
                           sTypeNames[type2];

            StringCopy(dest, n1);
            StringAppend(dest, COMPOUND_STRING("/"));
            StringAppend(dest, n2);
        }
    }
}

static const u8 *const sGradeColors[5] = {
    sColor_Purple, // 0: S (Viola)
    sColor_Red,    // 1: A (Rossa)
    sColor_Blue,   // 2: B (Blu)
    sColor_Green,  // 3: C (Verde)
    sColor_Gray    // 4: D (Grigio)
};

static u8 GetIvGrade(u8 iv)
{
    if (iv >= 31) return 0; // S (31)
    if (iv >= 25) return 1; // A (25-30)
    if (iv >= 18) return 2; // B (18-24)
    if (iv >= 10) return 3; // C (10-17)
    return 4;               // D (0-9)
}

static u8 GetTotalIvGrade(u16 tot)
{
    if (tot >= 165) return 0; // S
    if (tot >= 140) return 1; // A
    if (tot >= 110) return 2; // B
    if (tot >= 75)  return 3; // C
    return 4;                 // D
}

static void CB2_GachaMain(void);
static void VBlankCB_Gacha(void);
static void Task_GachaLoop(u8 taskId);
static void UpdateHeaderWindow(void);
static void UpdatePromptWindow(void);
static void PrintResultWindow(void);
static void StartRoll(void);
static void StartMultiRoll(void);
static void CleanSprites(void);

void OpenGachaSlotMachine(void)
{
    InitKaizoMetaProgression();
    if (gSaveBlock2Ptr->metaProgression.gachaTickets < 1)
    {
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
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);

    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    InitWindows(sWindowTemplates);

    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(WIN_STAT_CARD, 0x2A8, BG_PLTT_ID(13));
    LoadPalette(sCheckerBg_Pal, BG_PLTT_ID(0), sizeof(sCheckerBg_Pal));
    LoadPalette(sGachaTextPalette, BG_PLTT_ID(14), sizeof(sGachaTextPalette));
    LoadPalette(sGachaTextPalette, BG_PLTT_ID(15), sizeof(sGachaTextPalette));

    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetAllPicSprites();

    LoadSpriteSheet(&sEggSpriteSheet);
    LoadSpritePalettes(sEggSpritePalettes);

    sGacha->monSpriteId = 0xFFFF;
    sGacha->bgScrollTimer = 0;
    sGacha->bgScrollX = 0;
    sGacha->bgScrollY = 0;

    // 1. Carica la grafica dei tile su BG1 (charBase 2)
    CpuCopy16(sCheckerBg_Gfx, (void *)(BG_CHAR_ADDR(2)), sizeof(sCheckerBg_Gfx));

    // 2. Mappa di tessere a scacchiera infinita su screenBase 29 (u16 per tutti i 32x32 tile di VRAM)
    u16 *vramBg1 = (u16 *)(BG_SCREEN_ADDR(29));
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 32; x++)
        {
            u8 blockX = x / 2;
            u8 blockY = y / 2;
            bool8 isSquareB = (blockX + blockY) & 1;
            u8 subX = x & 1;
            u8 subY = y & 1;
            u16 tileId = isSquareB ? (5 + subY * 2 + subX) : (1 + subY * 2 + subX);
            vramBg1[y * 32 + x] = tileId;
        }
    }

    // Make sure tile 0 of charBase 0 is completely blank (32 bytes of 0) so tile 0 in BG0 is 100% transparent
    DmaFill16(3, 0, (void *)BG_CHAR_ADDR(0), 32);

    // Clear BG0 tilemap buffer completely
    u16 *bg0Tilemap = GetBgTilemapBuffer(0);
    if (bg0Tilemap != NULL)
        CpuFill16(0, bg0Tilemap, 32 * 32 * sizeof(u16));
    CopyBgTilemapBufferToVram(0);

    // Mostra BG0 (testo/finestre) e BG1 (slot machine pattern)
    ShowBg(0);
    ShowBg(1);

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_OBJ_ON);

    CreateTask(Task_GachaLoop, 0);

    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);

    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_Gacha);
    SetMainCallback2(CB2_GachaMain);

    if (sIsMultiPullMode && gSaveBlock2Ptr->metaProgression.gachaTickets >= MULTI_PULL_COUNT)
        StartMultiRoll();
    else
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
    if (sGacha != NULL)
    {
        SetGpuReg(REG_OFFSET_BG1HOFS, sGacha->bgScrollX);
        SetGpuReg(REG_OFFSET_BG1VOFS, sGacha->bgScrollY);
    }
}

static void StartRoll(void)
{
    if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
        gSaveBlock2Ptr->metaProgression.gachaTickets--;

    CleanSprites();

    // Clear entire BG0 tilemap buffer and VRAM so no result windows or borders linger
    u16 *bg0Tilemap = GetBgTilemapBuffer(0);
    if (bg0Tilemap != NULL)
        CpuFill16(0, bg0Tilemap, 32 * 32 * sizeof(u16));
    CopyBgTilemapBufferToVram(0);

    sGacha->isMultiPull = FALSE;
    sGacha->rolledTier = RollRarityTier(CATEGORY_POKEMON, LOOT_CONTEXT_STANDARD, NULL);
    if (sGacha->rolledTier == RARITY_NONE || sGacha->rolledTier > RARITY_EPIC)
        sGacha->rolledTier = RARITY_COMMON;

    u16 species = SPECIES_NONE;

    if (sSelectedGachaBanner >= 1 && sSelectedGachaBanner <= NUM_GENERATIONS)
    {
        u8 gen = sSelectedGachaBanner - 1; // 0 to 8
        u16 genSpecies[120];
        u16 totalInGen = GetGenBaseSpecies(gen, genSpecies, 120);

        u16 tierCandidates[120];
        u16 tierCount = 0;
        for (u16 i = 0; i < totalInGen; i++)
        {
            u16 sp = genSpecies[i];
            if (GetSpeciesRarityTier(sp) == sGacha->rolledTier)
            {
                tierCandidates[tierCount++] = sp;
            }
        }

        if (tierCount > 0)
            species = tierCandidates[Random() % tierCount];
        else if (totalInGen > 0)
            species = genSpecies[Random() % totalInGen];
        else
            species = SPECIES_CHARMANDER;
    }
    else
    {
        // 0: All Generations
        species = GetRandomRaritySpeciesByTier(sGacha->rolledTier, NULL);
        if (species == SPECIES_NONE)
            species = SPECIES_CHARMANDER;

        while (GetSpeciesPreEvolution(species) != SPECIES_NONE)
            species = GetSpeciesPreEvolution(species);
    }

    sGacha->hatchedSpecies = species;

    // Generate Level 5 Mon with Scratch
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

    // 1/64 chance to be shiny in Gacha (or naturally shiny)
    if ((Random() % 64) == 0 || IsMonShiny(&sGacha->hatchedMon))
    {
        bool32 trueVal = TRUE;
        SetMonData(&sGacha->hatchedMon, MON_DATA_IS_SHINY, &trueVal);
        SetBoxMonData(&sGacha->hatchedMon.box, MON_DATA_SHINY_UNLOCKED, &trueVal);
    }

    // Check duplicate
    struct BoxPokemon *existing = GetUnlockedRosterMon(species);
    if (existing != NULL)
    {
        sGacha->isDuplicate = TRUE;
        sGacha->existingMon = *existing;

        sGacha->oldIvs[0] = GetBoxMonData(existing, MON_DATA_HP_IV);
        sGacha->oldIvs[1] = GetBoxMonData(existing, MON_DATA_ATK_IV);
        sGacha->oldIvs[2] = GetBoxMonData(existing, MON_DATA_DEF_IV);
        sGacha->oldIvs[3] = GetBoxMonData(existing, MON_DATA_SPATK_IV);
        sGacha->oldIvs[4] = GetBoxMonData(existing, MON_DATA_SPDEF_IV);
        sGacha->oldIvs[5] = GetBoxMonData(existing, MON_DATA_SPEED_IV);

        sGacha->hatchedIvs[0] = GetMonData(&sGacha->hatchedMon, MON_DATA_HP_IV);
        sGacha->hatchedIvs[1] = GetMonData(&sGacha->hatchedMon, MON_DATA_ATK_IV);
        sGacha->hatchedIvs[2] = GetMonData(&sGacha->hatchedMon, MON_DATA_DEF_IV);
        sGacha->hatchedIvs[3] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPATK_IV);
        sGacha->hatchedIvs[4] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPDEF_IV);
        sGacha->hatchedIvs[5] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPEED_IV);

        for (int i = 0; i < 6; i++)
            sGacha->mergedIvs[i] = max(sGacha->oldIvs[i], sGacha->hatchedIvs[i]);

        // Merge best IVs into existing roster mon automatically!
        u32 hpIV  = sGacha->mergedIvs[0];
        u32 atkIV = sGacha->mergedIvs[1];
        u32 defIV = sGacha->mergedIvs[2];
        u32 spaIV = sGacha->mergedIvs[3];
        u32 spdIV = sGacha->mergedIvs[4];
        u32 speIV = sGacha->mergedIvs[5];

        SetBoxMonData(existing, MON_DATA_HP_IV, &hpIV);
        SetBoxMonData(existing, MON_DATA_ATK_IV, &atkIV);
        SetBoxMonData(existing, MON_DATA_DEF_IV, &defIV);
        SetBoxMonData(existing, MON_DATA_SPATK_IV, &spaIV);
        SetBoxMonData(existing, MON_DATA_SPDEF_IV, &spdIV);
        SetBoxMonData(existing, MON_DATA_SPEED_IV, &speIV);

        if (IsMonShiny(&sGacha->hatchedMon) || GetBoxMonData(&sGacha->hatchedMon.box, MON_DATA_SHINY_UNLOCKED))
        {
            bool32 trueVal = TRUE;
            SetBoxMonData(existing, MON_DATA_SHINY_UNLOCKED, &trueVal);
        }

        AddRarityPoints(sGacha->rolledTier, 10);
    }
    else
    {
        sGacha->isDuplicate = FALSE;
        for (int i = 0; i < 6; i++)
        {
            sGacha->oldIvs[i] = 0;
            sGacha->hatchedIvs[i] = GetMonData(&sGacha->hatchedMon, MON_DATA_HP_IV + i);
            sGacha->mergedIvs[i] = sGacha->hatchedIvs[i];
        }
    }

    // Initialize 3x3 Egg Sprites inside Celadon reel windows
    for (int r = 0; r < NUM_REELS; r++)
    {
        sGacha->reelStopped[r] = FALSE;
        sGacha->reelYOffsets[r] = 0;
        for (int i = 0; i < EGGS_PER_REEL; i++)
        {
            u8 tier = (Random() % 4) + 1;
            sGacha->reelTiers[r][i] = tier;
            u8 spriteId = CreateSprite(&sSpriteTemplate_GachaEgg, sReelXCoords[r], REEL_Y_CENTER + (i - 1) * 32, 2);
            gSprites[spriteId].oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_GACHA_COMMON + tier - 1);
            sGacha->eggSpriteIds[r][i] = spriteId;
        }
    }

    UpdateHeaderWindow();
    UpdatePromptWindow();

    PlaySE(SE_ROULETTE_BALL);
    sGacha->state = GACHA_STATE_SPINNING;
    sGacha->timer = 0;
}

static void StartMultiRoll(void)
{
    if (gSaveBlock2Ptr->metaProgression.gachaTickets < MULTI_PULL_COUNT)
        return;

    gSaveBlock2Ptr->metaProgression.gachaTickets -= MULTI_PULL_COUNT;
    CleanSprites();

    // Clear entire BG0 tilemap buffer and VRAM so no result windows or borders linger
    u16 *bg0Tilemap = GetBgTilemapBuffer(0);
    if (bg0Tilemap != NULL)
        CpuFill16(0, bg0Tilemap, 32 * 32 * sizeof(u16));
    CopyBgTilemapBufferToVram(0);

    sGacha->isMultiPull = TRUE;
    sGacha->multiIndex = 0;

    for (int k = 0; k < MULTI_PULL_COUNT; k++)
    {
        struct GachaPullResult *pull = &sGacha->pulls[k];

        pull->tier = RollRarityTier(CATEGORY_POKEMON, LOOT_CONTEXT_STANDARD, NULL);
        if (pull->tier == RARITY_NONE || pull->tier > RARITY_EPIC)
            pull->tier = RARITY_COMMON;

        u16 species = SPECIES_NONE;
        if (sSelectedGachaBanner >= 1 && sSelectedGachaBanner <= NUM_GENERATIONS)
        {
            u8 gen = sSelectedGachaBanner - 1;
            u16 genSpecies[120];
            u16 totalInGen = GetGenBaseSpecies(gen, genSpecies, 120);

            u16 tierCandidates[120];
            u16 tierCount = 0;
            for (u16 i = 0; i < totalInGen; i++)
            {
                if (GetSpeciesRarityTier(genSpecies[i]) == pull->tier)
                    tierCandidates[tierCount++] = genSpecies[i];
            }
            if (tierCount > 0)
                species = tierCandidates[Random() % tierCount];
            else if (totalInGen > 0)
                species = genSpecies[Random() % totalInGen];
            else
                species = SPECIES_CHARMANDER;
        }
        else
        {
            species = GetRandomRaritySpeciesByTier(pull->tier, NULL);
            if (species == SPECIES_NONE)
                species = SPECIES_CHARMANDER;
            while (GetSpeciesPreEvolution(species) != SPECIES_NONE)
                species = GetSpeciesPreEvolution(species);
        }

        pull->species = species;

        // Generate Level 5 Mon with Scratch
        ZeroMonData(&pull->mon);
        CreateMon(&pull->mon, species, 5, Random32(), OTID_STRUCT_PLAYER_ID);
        u16 scratch = MOVE_SCRATCH;
        u16 noneMove = MOVE_NONE;
        SetMonData(&pull->mon, MON_DATA_MOVE1, &scratch);
        SetMonData(&pull->mon, MON_DATA_MOVE2, &noneMove);
        SetMonData(&pull->mon, MON_DATA_MOVE3, &noneMove);
        SetMonData(&pull->mon, MON_DATA_MOVE4, &noneMove);
        u8 pp = gMovesInfo[MOVE_SCRATCH].pp;
        u8 zeroPP = 0;
        SetMonData(&pull->mon, MON_DATA_PP1, &pp);
        SetMonData(&pull->mon, MON_DATA_PP2, &zeroPP);
        SetMonData(&pull->mon, MON_DATA_PP3, &zeroPP);
        SetMonData(&pull->mon, MON_DATA_PP4, &zeroPP);
        CalculateMonStats(&pull->mon);

        // Shiny roll (1/64)
        if ((Random() % 64) == 0 || IsMonShiny(&pull->mon))
        {
            bool32 trueVal = TRUE;
            SetMonData(&pull->mon, MON_DATA_IS_SHINY, &trueVal);
            SetBoxMonData(&pull->mon.box, MON_DATA_SHINY_UNLOCKED, &trueVal);
        }

        // Check duplicate
        struct BoxPokemon *existing = GetUnlockedRosterMon(species);
        if (existing != NULL)
        {
            pull->isDuplicate = TRUE;
            pull->existingMon = *existing;

            pull->oldIvs[0] = GetBoxMonData(existing, MON_DATA_HP_IV);
            pull->oldIvs[1] = GetBoxMonData(existing, MON_DATA_ATK_IV);
            pull->oldIvs[2] = GetBoxMonData(existing, MON_DATA_DEF_IV);
            pull->oldIvs[3] = GetBoxMonData(existing, MON_DATA_SPATK_IV);
            pull->oldIvs[4] = GetBoxMonData(existing, MON_DATA_SPDEF_IV);
            pull->oldIvs[5] = GetBoxMonData(existing, MON_DATA_SPEED_IV);

            pull->hatchedIvs[0] = GetMonData(&pull->mon, MON_DATA_HP_IV);
            pull->hatchedIvs[1] = GetMonData(&pull->mon, MON_DATA_ATK_IV);
            pull->hatchedIvs[2] = GetMonData(&pull->mon, MON_DATA_DEF_IV);
            pull->hatchedIvs[3] = GetMonData(&pull->mon, MON_DATA_SPATK_IV);
            pull->hatchedIvs[4] = GetMonData(&pull->mon, MON_DATA_SPDEF_IV);
            pull->hatchedIvs[5] = GetMonData(&pull->mon, MON_DATA_SPEED_IV);

            for (int i = 0; i < 6; i++)
                pull->mergedIvs[i] = max(pull->oldIvs[i], pull->hatchedIvs[i]);

            // Save merged IVs into existing
            u32 hpIV  = pull->mergedIvs[0];
            u32 atkIV = pull->mergedIvs[1];
            u32 defIV = pull->mergedIvs[2];
            u32 spaIV = pull->mergedIvs[3];
            u32 spdIV = pull->mergedIvs[4];
            u32 speIV = pull->mergedIvs[5];

            SetBoxMonData(existing, MON_DATA_HP_IV, &hpIV);
            SetBoxMonData(existing, MON_DATA_ATK_IV, &atkIV);
            SetBoxMonData(existing, MON_DATA_DEF_IV, &defIV);
            SetBoxMonData(existing, MON_DATA_SPATK_IV, &spaIV);
            SetBoxMonData(existing, MON_DATA_SPDEF_IV, &spdIV);
            SetBoxMonData(existing, MON_DATA_SPEED_IV, &speIV);

            if (IsMonShiny(&pull->mon) || GetBoxMonData(&pull->mon.box, MON_DATA_SHINY_UNLOCKED))
            {
                bool32 trueVal = TRUE;
                SetBoxMonData(existing, MON_DATA_SHINY_UNLOCKED, &trueVal);
            }

            AddRarityPoints(pull->tier, 10);
        }
        else
        {
            pull->isDuplicate = FALSE;
            AddUnlockedRosterMon(&pull->mon.box);
            for (int i = 0; i < 6; i++)
            {
                pull->oldIvs[i] = 0;
                pull->hatchedIvs[i] = GetMonData(&pull->mon, MON_DATA_HP_IV + i);
                pull->mergedIvs[i] = pull->hatchedIvs[i];
            }
        }

        // Egg Sprite creation
        u8 spriteId = CreateSprite(&sSpriteTemplate_GachaEgg, sMultiEggXCoords[k], -24, 2);
        gSprites[spriteId].oam.paletteNum = IndexOfSpritePaletteTag(GetEggPaletteTag(pull->tier, IsMonShiny(&pull->mon)));
        gSprites[spriteId].invisible = TRUE;
        sGacha->multiEggSpriteIds[k] = spriteId;
        sGacha->multiEggY[k] = -24;
        sGacha->multiEggSpeedY[k] = 0;
        sGacha->multiEggLanded[k] = FALSE;
    }

    UpdateHeaderWindow();
    UpdatePromptWindow();

    sGacha->state = GACHA_STATE_MULTI_EGG_DROP;
    sGacha->timer = 0;
}

static void CleanSprites(void)
{
    if (sGacha->monSpriteId != 0xFFFF)
    {
        FreeAndDestroyMonPicSprite(sGacha->monSpriteId);
        sGacha->monSpriteId = 0xFFFF;
    }

    for (int r = 0; r < NUM_REELS; r++)
    {
        for (int i = 0; i < EGGS_PER_REEL; i++)
        {
            u8 sId = sGacha->eggSpriteIds[r][i];
            if (sId != 0 && gSprites[sId].inUse)
            {
                DestroySprite(&gSprites[sId]);
                sGacha->eggSpriteIds[r][i] = 0;
            }
        }
    }

    for (int k = 0; k < MULTI_PULL_COUNT; k++)
    {
        u8 sId = sGacha->multiEggSpriteIds[k];
        if (sId != 0 && gSprites[sId].inUse)
        {
            DestroySprite(&gSprites[sId]);
            sGacha->multiEggSpriteIds[k] = 0;
        }
    }
}

static void UpdateHeaderWindow(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    DrawTextBorderOuter(WIN_HEADER, 0x2A8, 13);
    PutWindowTilemap(WIN_HEADER);

    u8 strCredit[10];
    u8 strPayout[10];
    ConvertIntToDecimalStringN(strCredit, gSaveBlock2Ptr->metaProgression.gachaTickets, STR_CONV_MODE_LEFT_ALIGN, 4);

    u8 tier = (sGacha != NULL && sGacha->rolledTier >= RARITY_COMMON) ? sGacha->rolledTier : RARITY_COMMON;
    ConvertIntToDecimalStringN(strPayout, gSaveBlock2Ptr->metaProgression.rarityPoints[tier], STR_CONV_MODE_LEFT_ALIGN, 4);

    u8 textBuf[48];
    u8 *p = StringCopy(textBuf, COMPOUND_STRING("TICKETS: "));
    p = StringAppend(p, strCredit);
    p = StringAppend(p, COMPOUND_STRING("   PTS: "));
    StringAppend(p, strPayout);

    s32 w = GetStringWidth(FONT_SMALL, textBuf, 0);
    s32 x = max(4, (112 - w) / 2);
    AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, x, 0, sColor_Dark, TEXT_SKIP_DRAW, textBuf);

    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void Task_GachaLoop(u8 taskId)
{
    sGacha->bgScrollTimer++;
    if ((sGacha->bgScrollTimer % 3) == 0)
    {
        sGacha->bgScrollX++;
        sGacha->bgScrollY++;
    }

    switch (sGacha->state)
    {
    case GACHA_STATE_SPINNING:
        sGacha->timer++;
        // Spin all reels
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
            if (i == 1)
                gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(GetEggPaletteTag(sGacha->rolledTier, IsMonShiny(&sGacha->hatchedMon)));
        }
        PlaySE(SE_BALL_BOUNCE_1);

        sGacha->state = GACHA_STATE_STOP_REEL1;
        sGacha->timer = 0;
        break;

    case GACHA_STATE_STOP_REEL1:
        sGacha->timer++;
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

        if (sGacha->timer >= 35)
        {
            sGacha->reelStopped[1] = TRUE;
            for (int i = 0; i < EGGS_PER_REEL; i++)
            {
                u8 sId = sGacha->eggSpriteIds[1][i];
                gSprites[sId].y = REEL_Y_CENTER + (i - 1) * 32;
                if (i == 1)
                    gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(GetEggPaletteTag(sGacha->rolledTier, IsMonShiny(&sGacha->hatchedMon)));
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

        // Suspense per tier
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
                    gSprites[sId].oam.paletteNum = IndexOfSpritePaletteTag(GetEggPaletteTag(sGacha->rolledTier, IsMonShiny(&sGacha->hatchedMon)));
            }

            // Fanfara e flash
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
            // Nasconde le uova laterali e prepara l'uovo centrale per la schiusa
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
            StartSpriteAnim(&gSprites[centerEgg], 1); // Prima crepa
            PlaySE(SE_BALL_BOUNCE_2);
        }
        else if (sGacha->timer == 45)
        {
            gSprites[centerEgg].x2 = -3;
            StartSpriteAnim(&gSprites[centerEgg], 2); // Seconda crepa
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
    {
        bool8 isShiny = IsMonShiny(&sGacha->hatchedMon);
        u32 personality = GetMonData(&sGacha->hatchedMon, MON_DATA_PERSONALITY);
        // Spawn Hatched Pokémon Front Sprite on left side stage (x = 60, y = 38)
        sGacha->monSpriteId = CreateMonPicSprite(sGacha->hatchedSpecies, isShiny, personality, TRUE, 60, 38, 14, TAG_NONE);
        if (sGacha->monSpriteId != 0xFFFF)
            gSprites[sGacha->monSpriteId].oam.priority = 0;
        PlayCry_Normal(sGacha->hatchedSpecies, 0);
        if (isShiny)
            PlaySE(SE_SHINY);

        sGacha->state = GACHA_STATE_SHOW_RESULT;
        sGacha->timer = 0;
        break;
    }

    case GACHA_STATE_SHOW_RESULT:
        sGacha->timer++;
        if (sGacha->timer >= 20)
        {
            PrintResultWindow();
            if (!sGacha->isDuplicate)
                AddUnlockedRosterMon(&sGacha->hatchedMon.box);
            UpdatePromptWindow();
            sGacha->state = GACHA_STATE_WAIT_NEXT;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_WAIT_NEXT:
        sGacha->timer++;
        if (sGacha->timer >= 12)
        {
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
            else if (JOY_NEW(START_BUTTON))
            {
                if (gSaveBlock2Ptr->metaProgression.gachaTickets >= MULTI_PULL_COUNT)
                {
                    PlaySE(SE_SELECT);
                    StartMultiRoll();
                }
            }
            else if (JOY_NEW(B_BUTTON))
            {
                PlaySE(SE_SELECT);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
                sGacha->state = GACHA_STATE_EXIT;
            }
        }
        break;

    case GACHA_STATE_MULTI_EGG_DROP:
    {
        sGacha->timer++;

        // Fast-forward / Skip if A button is pressed
        if (JOY_NEW(A_BUTTON | B_BUTTON) && sGacha->timer > 10)
        {
            for (int k = 0; k < MULTI_PULL_COUNT; k++)
            {
                u8 sId = sGacha->multiEggSpriteIds[k];
                gSprites[sId].invisible = FALSE;
                gSprites[sId].y = EGG_TARGET_Y;
                sGacha->multiEggY[k] = EGG_TARGET_Y;
                sGacha->multiEggLanded[k] = TRUE;
            }
            PlaySE(SE_BALL_BOUNCE_2);
            sGacha->state = GACHA_STATE_MULTI_EGGS_READY;
            sGacha->timer = 0;
            UpdatePromptWindow();
            break;
        }

        // 1 egg starts falling every 50 frames (~0.83 sec)
        for (int k = 0; k < MULTI_PULL_COUNT; k++)
        {
            if (sGacha->timer >= k * 50)
            {
                u8 sId = sGacha->multiEggSpriteIds[k];
                gSprites[sId].invisible = FALSE;

                if (!sGacha->multiEggLanded[k])
                {
                    sGacha->multiEggSpeedY[k] += 2; // gravity
                    sGacha->multiEggY[k] += sGacha->multiEggSpeedY[k];

                    if (sGacha->multiEggY[k] >= EGG_TARGET_Y)
                    {
                        sGacha->multiEggY[k] = EGG_TARGET_Y;
                        if (sGacha->multiEggSpeedY[k] > 4)
                        {
                            sGacha->multiEggSpeedY[k] = -3; // Bounce!
                            if (sGacha->pulls[k].tier == RARITY_EPIC)
                                PlaySE(SE_PIN);
                            else if (sGacha->pulls[k].tier == RARITY_RARE)
                                PlaySE(SE_DING_DONG);
                            else
                                PlaySE(SE_BALL_BOUNCE_1);
                        }
                        else
                        {
                            sGacha->multiEggSpeedY[k] = 0;
                            sGacha->multiEggLanded[k] = TRUE;
                        }
                    }
                    gSprites[sId].y = sGacha->multiEggY[k];
                }
            }
        }

        // Check if all eggs have landed and a short pause passed
        if (sGacha->timer >= (MULTI_PULL_COUNT - 1) * 50 + 40 && sGacha->multiEggLanded[MULTI_PULL_COUNT - 1])
        {
            sGacha->state = GACHA_STATE_MULTI_EGGS_READY;
            sGacha->timer = 0;
            UpdatePromptWindow();
        }
        break;
    }

    case GACHA_STATE_MULTI_EGGS_READY:
        sGacha->timer++;
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sGacha->state = GACHA_STATE_EXIT;
        }
        else if (JOY_NEW(A_BUTTON) || sGacha->timer >= 45)
        {
            PlaySE(SE_SELECT);
            sGacha->multiIndex = 0;
            sGacha->state = GACHA_STATE_MULTI_SHOW_MON;
            sGacha->timer = 0;
        }
        break;

    case GACHA_STATE_MULTI_SHOW_MON:
    {
        // Hide multi egg sprites
        for (int k = 0; k < MULTI_PULL_COUNT; k++)
        {
            u8 sId = sGacha->multiEggSpriteIds[k];
            if (sId != 0 && gSprites[sId].inUse)
                gSprites[sId].invisible = TRUE;
        }

        struct GachaPullResult *pull = &sGacha->pulls[sGacha->multiIndex];
        sGacha->hatchedSpecies = pull->species;
        sGacha->hatchedMon = pull->mon;
        sGacha->rolledTier = pull->tier;
        sGacha->isDuplicate = pull->isDuplicate;
        sGacha->existingMon = pull->existingMon;
        for (int i = 0; i < 6; i++)
        {
            sGacha->oldIvs[i] = pull->oldIvs[i];
            sGacha->hatchedIvs[i] = pull->hatchedIvs[i];
            sGacha->mergedIvs[i] = pull->mergedIvs[i];
        }

        if (sGacha->monSpriteId != 0xFFFF)
        {
            FreeAndDestroyMonPicSprite(sGacha->monSpriteId);
            sGacha->monSpriteId = 0xFFFF;
        }

        bool8 isShiny = IsMonShiny(&sGacha->hatchedMon);
        u32 personality = GetMonData(&sGacha->hatchedMon, MON_DATA_PERSONALITY);
        sGacha->monSpriteId = CreateMonPicSprite(sGacha->hatchedSpecies, isShiny, personality, TRUE, 60, 38, 14, TAG_NONE);
        if (sGacha->monSpriteId != 0xFFFF)
            gSprites[sGacha->monSpriteId].oam.priority = 0;

        PlayCry_Normal(sGacha->hatchedSpecies, 0);
        if (isShiny)
            PlaySE(SE_SHINY);

        PrintResultWindow();
        UpdatePromptWindow();

        sGacha->state = GACHA_STATE_MULTI_WAIT_INPUT;
        sGacha->timer = 0;
        break;
    }

    case GACHA_STATE_MULTI_WAIT_INPUT:
        sGacha->timer++;
        if (sGacha->timer >= 12)
        {
            if (JOY_NEW(B_BUTTON))
            {
                PlaySE(SE_SELECT);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
                sGacha->state = GACHA_STATE_EXIT;
                break;
            }

            if (sGacha->multiIndex < MULTI_PULL_COUNT - 1)
            {
                if (JOY_NEW(A_BUTTON))
                {
                    PlaySE(SE_SELECT);
                    sGacha->multiIndex++;
                    sGacha->state = GACHA_STATE_MULTI_SHOW_MON;
                    sGacha->timer = 0;
                }
            }
            else
            {
                // Last mon (10th)
                if (JOY_NEW(A_BUTTON))
                {
                    if (gSaveBlock2Ptr->metaProgression.gachaTickets >= MULTI_PULL_COUNT)
                    {
                        PlaySE(SE_SELECT);
                        StartMultiRoll();
                    }
                    else if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
                    {
                        PlaySE(SE_SELECT);
                        StartRoll();
                    }
                    else
                    {
                        PlaySE(SE_BOO);
                    }
                }
                else if (JOY_NEW(START_BUTTON))
                {
                    if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
                    {
                        PlaySE(SE_SELECT);
                        StartRoll();
                    }
                }
            }
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

static void PrintResultWindow(void)
{
    ClearWindowTilemap(WIN_HEADER);
    u16 *bg0Tilemap = GetBgTilemapBuffer(0);
    if (bg0Tilemap != NULL)
    {
        for (int y = 0; y <= 3; y++)
        {
            for (int x = 6; x <= 23; x++)
            {
                bg0Tilemap[y * 32 + x] = 0;
            }
        }
    }
    CopyBgTilemapBufferToVram(0);

    u8 type1 = gSpeciesInfo[sGacha->hatchedSpecies].types[0];
    u8 type2 = gSpeciesInfo[sGacha->hatchedSpecies].types[1];
    if (type1 >= NUMBER_OF_MON_TYPES)
        type1 = TYPE_NORMAL;

    struct TypeTheme theme = sTypeThemes[type1];

    u16 monInfoPal[16];
    monInfoPal[0]  = RGB(0, 0, 0);       // Transparent
    monInfoPal[1]  = RGB(31, 31, 31);    // White card background
    monInfoPal[2]  = RGB(4, 4, 5);       // Dark charcoal text
    monInfoPal[3]  = RGB(20, 20, 21);    // Dark text shadow
    monInfoPal[4]  = theme.accent;       // Type accent (border & badge pill)
    monInfoPal[5]  = RGB(2, 2, 2);       // Accent shadow
    monInfoPal[6]  = theme.tint;         // Type tint (stage background)
    monInfoPal[7]  = RGB(18, 18, 18);    // Highlight/border
    if (sGacha->rolledTier == RARITY_EPIC) {
        monInfoPal[8] = RGB(28, 4, 4);   monInfoPal[9] = RGB(16, 2, 2);
    } else if (sGacha->rolledTier == RARITY_RARE) {
        monInfoPal[8] = RGB(3, 10, 28);  monInfoPal[9] = RGB(1, 4, 16);
    } else if (sGacha->rolledTier == RARITY_UNCOMMON) {
        monInfoPal[8] = RGB(2, 20, 4);   monInfoPal[9] = RGB(1, 10, 2);
    } else {
        monInfoPal[8] = RGB(14, 14, 15); monInfoPal[9] = RGB(22, 22, 23);
    }
    monInfoPal[10] = RGB(31, 31, 31);    // Badge text white FG
    monInfoPal[11] = RGB(3, 3, 3);       // Badge text shadow
    monInfoPal[12] = RGB(14, 14, 15);    // Gray FG
    monInfoPal[13] = RGB(22, 22, 23);    // Gray shadow
    monInfoPal[14] = RGB(14, 14, 15);    // Gray FG
    monInfoPal[15] = RGB(22, 22, 23);    // Gray shadow

    LoadPalette(monInfoPal, BG_PLTT_ID(14), sizeof(monInfoPal));

    // 1. SCHEDA SINISTRA: INFO POKEMON & PALCO ELEMENTALE
    FillWindowPixelBuffer(WIN_MON_INFO, PIXEL_FILL(1));
    DrawTextBorderOuter(WIN_MON_INFO, 0x2A8, 13);
    PutWindowTilemap(WIN_MON_INFO);

    // Palco colorato con la tinta del tipo elementale (x=5..82, y=5..54)
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(6), 5, 5, 78, 50);
    // Bordo del palco con il colore accent del tipo elementale
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(4), 4, 4, 80, 1);
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(4), 4, 55, 80, 1);
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(4), 4, 5, 1, 50);
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(4), 83, 5, 1, 50);

    // Pill badge del tipo elementale sotto il palco (y=60)
    u8 typeStr[32];
    FormatTypeString(typeStr, type1, type2);
    s32 typeWidth = GetStringWidth(FONT_SMALL, typeStr, 0);
    s32 pillWidth = min(80, typeWidth + 8);
    s32 pillX = max(4, (88 - pillWidth) / 2);
    FillWindowPixelRect(WIN_MON_INFO, PIXEL_FILL(4), pillX, 60, pillWidth, 10);
    s32 textX = pillX + (pillWidth - typeWidth) / 2;
    AddTextPrinterParameterized3(WIN_MON_INFO, FONT_SMALL, textX, 60, sColor_BadgeWhite, TEXT_SKIP_DRAW, typeStr);

    // Nome Pokémon centrato (y=73)
    const u8 *spName = gSpeciesInfo[sGacha->hatchedSpecies].speciesName;
    u8 nameBuf[32];
    StringCopy(nameBuf, spName);
    if (IsMonShiny(&sGacha->hatchedMon))
        StringAppend(nameBuf, COMPOUND_STRING(" {COLOR RED}{STAR}"));
    s32 nameWidth = GetStringWidth(FONT_NORMAL, nameBuf, 0);
    s32 nameX = max(2, (88 - nameWidth) / 2);
    AddTextPrinterParameterized3(WIN_MON_INFO, FONT_NORMAL, nameX, 73, sColor_Dark, TEXT_SKIP_DRAW, nameBuf);

    // Rarity Tier and Lv. 5 (y=88)
    if (sGacha->isDuplicate)
    {
        static const u8 sText_DuplicateBadge[] = _("{COLOR RED}(DUPLICATE)");
        s32 dupW = GetStringWidth(FONT_SMALL, sText_DuplicateBadge, 0);
        s32 dupX = max(2, (88 - dupW) / 2);
        AddTextPrinterParameterized3(WIN_MON_INFO, FONT_SMALL, dupX, 88, sColor_Red, TEXT_SKIP_DRAW, sText_DuplicateBadge);
    }
    else
    {
        const u8 *tierName = GetRarityTierName(sGacha->rolledTier);
        static const u8 sText_Lv5[] = _("Lv. 5");
        s32 tierWidth = GetStringWidth(FONT_SMALL, tierName, 0);
        s32 lvWidth = GetStringWidth(FONT_SMALL, sText_Lv5, 0);
        s32 totalW = tierWidth + 6 + lvWidth;
        s32 startX = max(2, (88 - totalW) / 2);
        AddTextPrinterParameterized3(WIN_MON_INFO, FONT_SMALL, startX, 88, sColor_TierOnPal14, TEXT_SKIP_DRAW, tierName);
        AddTextPrinterParameterized3(WIN_MON_INFO, FONT_SMALL, startX + tierWidth + 6, 88, sColor_Gray, TEXT_SKIP_DRAW, sText_Lv5);
    }

    CopyWindowToVram(WIN_MON_INFO, COPYWIN_FULL);

    // 2. SCHEDA DESTRA: STATISTICHE & IV COLORATI
    FillWindowPixelBuffer(WIN_STAT_CARD, PIXEL_FILL(1));
    DrawTextBorderOuter(WIN_STAT_CARD, 0x2A8, 13);
    PutWindowTilemap(WIN_STAT_CARD);

    static const u8 sText_Divider[] = _("------------------");

    static const u8 sText_GradeS[] = _("S");
    static const u8 sText_GradeA[] = _("A");
    static const u8 sText_GradeB[] = _("B");
    static const u8 sText_GradeC[] = _("C");
    static const u8 sText_GradeD[] = _("D");

    static const u8 *const sGradeText[5] = {
        sText_GradeS,
        sText_GradeA,
        sText_GradeB,
        sText_GradeC,
        sText_GradeD
    };

    if (sGacha->isDuplicate)
    {
        // Header: STAT  OLD  NEW
        static const u8 sText_HeaderStat[] = _("STAT");
        static const u8 sText_HeaderOld[]  = _("OLD");
        static const u8 sText_HeaderNew[]  = _("NEW");
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, 2, sColor_Dark, TEXT_SKIP_DRAW, sText_HeaderStat);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 36, 2, sColor_Dark, TEXT_SKIP_DRAW, sText_HeaderOld);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 72, 2, sColor_Dark, TEXT_SKIP_DRAW, sText_HeaderNew);

        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 3, 11, sColor_Gray, TEXT_SKIP_DRAW, sText_Divider);

        u16 oldTot = sGacha->oldIvs[0] + sGacha->oldIvs[1] + sGacha->oldIvs[2] + sGacha->oldIvs[3] + sGacha->oldIvs[4] + sGacha->oldIvs[5];
        u16 mergedTot = sGacha->mergedIvs[0] + sGacha->mergedIvs[1] + sGacha->mergedIvs[2] + sGacha->mergedIvs[3] + sGacha->mergedIvs[4] + sGacha->mergedIvs[5];

        static const u8 sText_StatNames[6][5] = {
            _("HP :"),
            _("ATK:"),
            _("DEF:"),
            _("SPA:"),
            _("SPD:"),
            _("SPE:")
        };

        for (int i = 0; i < 6; i++)
        {
            u8 yPos = 19 + i * 11;
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, yPos, sColor_Dark, TEXT_SKIP_DRAW, sText_StatNames[i]);

            // Old val & grade
            u8 oldVal = sGacha->oldIvs[i];
            u8 oldGr = GetIvGrade(oldVal);
            u8 bufOld[8];
            ConvertIntToDecimalStringN(bufOld, oldVal, STR_CONV_MODE_LEADING_ZEROS, 2);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 32, yPos, sColor_Dark, TEXT_SKIP_DRAW, bufOld);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 48, yPos, sGradeColors[oldGr], TEXT_SKIP_DRAW, sGradeText[oldGr]);

            // Merged val & grade (Green if upgraded!)
            u8 mergedVal = sGacha->mergedIvs[i];
            u8 mergedGr = GetIvGrade(mergedVal);
            u8 bufMerged[8];
            ConvertIntToDecimalStringN(bufMerged, mergedVal, STR_CONV_MODE_LEADING_ZEROS, 2);
            const u8 *valColor = (mergedVal > oldVal) ? sColor_Green : sColor_Dark;
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 68, yPos, valColor, TEXT_SKIP_DRAW, bufMerged);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 86, yPos, sGradeColors[mergedGr], TEXT_SKIP_DRAW, sGradeText[mergedGr]);
        }

        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 3, 84, sColor_Gray, TEXT_SKIP_DRAW, sText_Divider);

        // TOT
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, 91, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("TOT:"));

        u8 bufTotOld[8];
        ConvertIntToDecimalStringN(bufTotOld, oldTot, STR_CONV_MODE_LEADING_ZEROS, 3);
        u8 oldTotGr = GetTotalIvGrade(oldTot);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 28, 91, sColor_Dark, TEXT_SKIP_DRAW, bufTotOld);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 48, 91, sGradeColors[oldTotGr], TEXT_SKIP_DRAW, sGradeText[oldTotGr]);

        u8 bufTotMerged[8];
        ConvertIntToDecimalStringN(bufTotMerged, mergedTot, STR_CONV_MODE_LEADING_ZEROS, 3);
        u8 mergedTotGr = GetTotalIvGrade(mergedTot);
        const u8 *totColor = (mergedTot > oldTot) ? sColor_Green : sColor_Dark;
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 64, 91, totColor, TEXT_SKIP_DRAW, bufTotMerged);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 86, 91, sGradeColors[mergedTotGr], TEXT_SKIP_DRAW, sGradeText[mergedTotGr]);
    }
    else
    {
        u8 nat = GetNature(&sGacha->hatchedMon);
        u16 abil = GetMonAbility(&sGacha->hatchedMon);

        u8 lineNat[32];
        u8 *ptr = StringCopy(lineNat, COMPOUND_STRING("NAT: "));
        StringAppend(ptr, gNaturesInfo[nat].name);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, 3, sColor_Dark, TEXT_SKIP_DRAW, lineNat);

        u8 lineAbil[32];
        ptr = StringCopy(lineAbil, COMPOUND_STRING("ABIL: "));
        StringAppend(ptr, gAbilitiesInfo[abil].name);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, 14, sColor_Dark, TEXT_SKIP_DRAW, lineAbil);

        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 3, 24, sColor_Gray, TEXT_SKIP_DRAW, sText_Divider);

        u8 ivs[6];
        ivs[STAT_HP]    = GetMonData(&sGacha->hatchedMon, MON_DATA_HP_IV);
        ivs[STAT_ATK]   = GetMonData(&sGacha->hatchedMon, MON_DATA_ATK_IV);
        ivs[STAT_DEF]   = GetMonData(&sGacha->hatchedMon, MON_DATA_DEF_IV);
        ivs[STAT_SPATK] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPATK_IV);
        ivs[STAT_SPDEF] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPDEF_IV);
        ivs[STAT_SPEED] = GetMonData(&sGacha->hatchedMon, MON_DATA_SPEED_IV);

        u16 ivTot = ivs[0] + ivs[1] + ivs[2] + ivs[3] + ivs[4] + ivs[5];

        static const u8 sText_StatHP[]  = _("HP ");
        static const u8 sText_StatAtk[] = _("ATK");
        static const u8 sText_StatDef[] = _("DEF");
        static const u8 sText_StatSpA[] = _("SPA");
        static const u8 sText_StatSpD[] = _("SPD");
        static const u8 sText_StatSpe[] = _("SPE");

        static const u8 *const sStatNamesCol1[3] = { sText_StatHP, sText_StatAtk, sText_StatDef };
        static const u8 sStatIdxCol1[3] = { STAT_HP, STAT_ATK, STAT_DEF };

        static const u8 *const sStatNamesCol2[3] = { sText_StatSpA, sText_StatSpD, sText_StatSpe };
        static const u8 sStatIdxCol2[3] = { STAT_SPATK, STAT_SPDEF, STAT_SPEED };

        for (int i = 0; i < 3; i++)
        {
            u8 yPos = 34 + i * 12;

            // Colonna 1 (HP / ATK / DEF)
            u8 sIdx1 = sStatIdxCol1[i];
            u8 val1 = ivs[sIdx1];
            u8 gr1 = GetIvGrade(val1);
            u8 buf1[16];
            ptr = StringCopy(buf1, sStatNamesCol1[i]);
            ptr = StringAppend(ptr, COMPOUND_STRING(":"));
            ConvertIntToDecimalStringN(ptr, val1, STR_CONV_MODE_LEADING_ZEROS, 2);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, yPos, sColor_Dark, TEXT_SKIP_DRAW, buf1);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 38, yPos, sGradeColors[gr1], TEXT_SKIP_DRAW, sGradeText[gr1]);

            // Colonna 2 (SPA / SPD / SPE)
            u8 sIdx2 = sStatIdxCol2[i];
            u8 val2 = ivs[sIdx2];
            u8 gr2 = GetIvGrade(val2);
            u8 buf2[16];
            ptr = StringCopy(buf2, sStatNamesCol2[i]);
            ptr = StringAppend(ptr, COMPOUND_STRING(":"));
            ConvertIntToDecimalStringN(ptr, val2, STR_CONV_MODE_LEADING_ZEROS, 2);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 52, yPos, sColor_Dark, TEXT_SKIP_DRAW, buf2);
            AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 86, yPos, sGradeColors[gr2], TEXT_SKIP_DRAW, sGradeText[gr2]);
        }

        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 3, 72, sColor_Gray, TEXT_SKIP_DRAW, sText_Divider);

        u8 totBuf[24];
        ptr = StringCopy(totBuf, COMPOUND_STRING("TOT: "));
        ptr = ConvertIntToDecimalStringN(ptr, ivTot, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(ptr, COMPOUND_STRING("/186"));
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 4, 82, sColor_Dark, TEXT_SKIP_DRAW, totBuf);

        u8 totGr = GetTotalIvGrade(ivTot);
        AddTextPrinterParameterized3(WIN_STAT_CARD, FONT_SMALL, 74, 82, sGradeColors[totGr], TEXT_SKIP_DRAW, sGradeText[totGr]);
    }

    CopyWindowToVram(WIN_STAT_CARD, COPYWIN_FULL);

    // 3. BARRA INFERIORE CONTROLLI
    UpdatePromptWindow();
}

static void UpdatePromptWindow(void)
{
    FillWindowPixelBuffer(WIN_PROMPT, PIXEL_FILL(1));
    DrawTextBorderOuter(WIN_PROMPT, 0x2A8, 13);
    PutWindowTilemap(WIN_PROMPT);

    if (sGacha == NULL)
        return;

    if (sGacha->state == GACHA_STATE_MULTI_EGG_DROP)
    {
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 1, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("DROPPING 10 EGGS..."));
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Skip Animation"));
    }
    else if (sGacha->state == GACHA_STATE_MULTI_EGGS_READY)
    {
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 1, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("ALL 10 EGGS READY!"));
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Reveal Pokémon      {B_BUTTON} Skip & Exit"));
    }
    else if (sGacha->state == GACHA_STATE_MULTI_WAIT_INPUT)
    {
        u8 topBuf[48];
        u8 *p = StringCopy(topBuf, COMPOUND_STRING("POKÉMON ("));
        p = ConvertIntToDecimalStringN(p, sGacha->multiIndex + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        p = StringAppend(p, COMPOUND_STRING("/10)  "));
        if (sGacha->isDuplicate)
            StringAppend(p, COMPOUND_STRING("DUPLICATE (+10 Pts)"));
        else
            StringAppend(p, COMPOUND_STRING("NEW POKÉMON"));
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 1, sColor_Dark, TEXT_SKIP_DRAW, topBuf);

        if (sGacha->multiIndex < MULTI_PULL_COUNT - 1)
        {
            AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Next Pokémon      {B_BUTTON} Skip All & Exit"));
        }
        else
        {
            if (gSaveBlock2Ptr->metaProgression.gachaTickets >= MULTI_PULL_COUNT)
                AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} 10x Roll Again      {B_BUTTON} Exit"));
            else if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
                AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Single Roll      {B_BUTTON} Exit"));
            else
                AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{B_BUTTON} Exit (Out of tickets)"));
        }
    }
    else if (sGacha->state == GACHA_STATE_WAIT_NEXT)
    {
        u8 topBuf[48];
        if (sGacha->isDuplicate)
            StringCopy(topBuf, COMPOUND_STRING("DUPLICATE MERGED! (+10 Pts)"));
        else
            StringCopy(topBuf, COMPOUND_STRING("NEW POKÉMON ADDED TO ROSTER!"));
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 1, sColor_Dark, TEXT_SKIP_DRAW, topBuf);

        if (gSaveBlock2Ptr->metaProgression.gachaTickets >= MULTI_PULL_COUNT)
            AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Roll Again  {START_BUTTON} 10x Roll  {B_BUTTON} Exit"));
        else if (gSaveBlock2Ptr->metaProgression.gachaTickets >= 1)
            AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{A_BUTTON} Roll Again      {B_BUTTON} Exit"));
        else
            AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 12, sColor_Dark, TEXT_SKIP_DRAW, COMPOUND_STRING("{B_BUTTON} Exit (Out of tickets)"));
    }
    else
    {
        u8 bannerBuf[48];
        u8 *p = StringCopy(bannerBuf, COMPOUND_STRING("Slot Banner: "));
        StringAppend(p, sGachaBannerNames[sSelectedGachaBanner]);
        AddTextPrinterParameterized3(WIN_PROMPT, FONT_SMALL, 4, 7, sColor_Dark, TEXT_SKIP_DRAW, bannerBuf);
    }

    CopyWindowToVram(WIN_PROMPT, COPYWIN_FULL);
}
