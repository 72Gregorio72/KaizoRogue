#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "battle_interface.h"
#include "decompress.h"
#include "graphics.h"
#include "pokedex.h"
#include "sprite.h"
#include "type_icons.h"

static EWRAM_DATA u8 sBattlerTypeIconSpriteIds[MAX_BATTLERS_COUNT][2] = {0};

static void LoadTypeSpritesAndPalettes(void);
static enum Type GetMonPublicType(enum BattlerId, u32);
static enum Type GetMonDefensiveTeraType(struct Pokemon *, struct Pokemon *, enum BattlerId, u32, enum Species, enum Species);
static bool32 IsIllusionActiveAndTypeUnchanged(struct Pokemon *, enum Species, enum BattlerId);
static void SpriteCB_TypeIcon(struct Sprite*);

const union AnimCmd sSpriteAnim_TypeIcon_Normal[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_NORMAL), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Fighting[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_FIGHTING), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Flying[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_FLYING), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Poison[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_POISON), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ground[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_GROUND), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Rock[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_ROCK), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Bug[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_BUG), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ghost[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_GHOST), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Steel[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_STEEL), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Mystery[] =
{
    ANIMCMD_FRAME(TYPE_ICON_1_FRAME(TYPE_MYSTERY), 0),
    ANIMCMD_END
};

const union AnimCmd sSpriteAnim_TypeIcon_Fire[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_FIRE), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Water[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_WATER), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Grass[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_GRASS), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Electric[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_ELECTRIC), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Psychic[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_PSYCHIC), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Ice[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_ICE), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Dragon[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_DRAGON), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Dark[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_DARK), 0),
    ANIMCMD_END
};
const union AnimCmd sSpriteAnim_TypeIcon_Fairy[] =
{
    ANIMCMD_FRAME(TYPE_ICON_2_FRAME(TYPE_FAIRY), 0),
    ANIMCMD_END
};

const union AnimCmd *const sSpriteAnimTable_TypeIcons[] =
{
    [TYPE_NONE] =       sSpriteAnim_TypeIcon_Mystery,
    [TYPE_NORMAL] =     sSpriteAnim_TypeIcon_Normal,
    [TYPE_FIGHTING] =   sSpriteAnim_TypeIcon_Fighting,
    [TYPE_FLYING] =     sSpriteAnim_TypeIcon_Flying,
    [TYPE_POISON] =     sSpriteAnim_TypeIcon_Poison,
    [TYPE_GROUND] =     sSpriteAnim_TypeIcon_Ground,
    [TYPE_ROCK] =       sSpriteAnim_TypeIcon_Rock,
    [TYPE_BUG] =        sSpriteAnim_TypeIcon_Bug,
    [TYPE_GHOST] =      sSpriteAnim_TypeIcon_Ghost,
    [TYPE_STEEL] =      sSpriteAnim_TypeIcon_Steel,
    [TYPE_MYSTERY] =    sSpriteAnim_TypeIcon_Mystery,
    [TYPE_FIRE] =       sSpriteAnim_TypeIcon_Fire,
    [TYPE_WATER] =      sSpriteAnim_TypeIcon_Water,
    [TYPE_GRASS] =      sSpriteAnim_TypeIcon_Grass,
    [TYPE_ELECTRIC] =   sSpriteAnim_TypeIcon_Electric,
    [TYPE_PSYCHIC] =    sSpriteAnim_TypeIcon_Psychic,
    [TYPE_ICE] =        sSpriteAnim_TypeIcon_Ice,
    [TYPE_DRAGON] =     sSpriteAnim_TypeIcon_Dragon,
    [TYPE_DARK] =       sSpriteAnim_TypeIcon_Dark,
    [TYPE_FAIRY] =      sSpriteAnim_TypeIcon_Fairy,
    [TYPE_STELLAR] =    sSpriteAnim_TypeIcon_Mystery,
};

const struct SpritePalette sTypeIconPal1 =
{
    .data = gBattleIcons_Pal1,
    .tag = TYPE_ICON_TAG
};

const struct SpritePalette sTypeIconPal2 =
{
    .data = gBattleIcons_Pal2,
    .tag = TYPE_ICON_TAG_2
};

const struct OamData sOamData_TypeIcons =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

const struct CompressedSpriteSheet sSpriteSheet_TypeIcons2 =
{
    .data = gBattleIcons_Gfx2,
    .size = (16*16) * 10 / 2,
    .tag = TYPE_ICON_TAG_2,
};

const struct CompressedSpriteSheet sSpriteSheet_TypeIcons1 =
{
    .data = gBattleIcons_Gfx1,
    .size = (16*16) * 10 / 2,
    .tag = TYPE_ICON_TAG,
};

const struct SpriteTemplate sSpriteTemplate_TypeIcons1 =
{
    .tileTag = TYPE_ICON_TAG,
    .paletteTag = TYPE_ICON_TAG,
    .oam = &sOamData_TypeIcons,
    .anims = sSpriteAnimTable_TypeIcons,
    .callback = SpriteCB_TypeIcon
};

const struct SpriteTemplate sSpriteTemplate_TypeIcons2 =
{
    .tileTag = TYPE_ICON_TAG_2,
    .paletteTag = TYPE_ICON_TAG_2,
    .oam = &sOamData_TypeIcons,
    .anims = sSpriteAnimTable_TypeIcons,
    .callback = SpriteCB_TypeIcon
};

static void LoadTypeSpritesAndPalettes(void)
{
    if (IndexOfSpritePaletteTag(TYPE_ICON_TAG) == 0xFF)
        LoadSpritePalette(&sTypeIconPal1);
    if (IndexOfSpritePaletteTag(TYPE_ICON_TAG_2) == 0xFF)
        LoadSpritePalette(&sTypeIconPal2);

    if (GetSpriteTileStartByTag(TYPE_ICON_TAG) == 0xFFFF)
        LoadCompressedSpriteSheet(&sSpriteSheet_TypeIcons1);
    if (GetSpriteTileStartByTag(TYPE_ICON_TAG_2) == 0xFFFF)
        LoadCompressedSpriteSheet(&sSpriteSheet_TypeIcons2);
}

void CreateBattlerTypeIcons(enum BattlerId battler)
{
    u32 i;
    LoadTypeSpritesAndPalettes();

    for (i = 0; i < 2; i++)
    {
        u8 spriteId = CreateSpriteAtEndUnchecked(&sSpriteTemplate_TypeIcons1, 0, 0, 0);
        sBattlerTypeIconSpriteIds[battler][i] = spriteId;

        if (spriteId != MAX_SPRITES)
        {
            gSprites[spriteId].tBattler = battler;
            gSprites[spriteId].tTypeNum = i;
            gSprites[spriteId].tPosX = 0;
            gSprites[spriteId].tPosY = 0;
            gSprites[spriteId].tForceHidden = TRUE;
            gSprites[spriteId].invisible = TRUE;
            gSprites[spriteId].callback = SpriteCB_TypeIcon;
        }
    }

    UpdateBattlerTypeIcons(battler);
}

void DestroyBattlerTypeIcons(enum BattlerId battler)
{
    u32 i;
    for (i = 0; i < 2; i++)
    {
        u8 spriteId = sBattlerTypeIconSpriteIds[battler][i];
        if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse)
        {
            DestroySprite(&gSprites[spriteId]);
            sBattlerTypeIconSpriteIds[battler][i] = MAX_SPRITES;
        }
    }
}

static bool32 ShouldFlipTypeIcon(enum BattlerId battler, enum Type typeId)
{
    return FALSE;
}

void UpdateBattlerTypeIcons(enum BattlerId battler)
{
    struct Pokemon *mon;
    enum Species species;
    enum Type types[2];
    bool32 isDualType;
    s16 xOffset, yBase;

    if (battler >= gBattlersCount)
        return;

    mon = GetBattlerMon(battler);
    if (mon == NULL)
        return;

    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
    {
        for (u32 i = 0; i < 2; i++)
        {
            u8 spriteId = sBattlerTypeIconSpriteIds[battler][i];
            if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse)
            {
                gSprites[spriteId].tForceHidden = TRUE;
                gSprites[spriteId].invisible = TRUE;
            }
        }
        return;
    }

    if (B_SHOW_TYPES == SHOW_TYPES_NEVER)
    {
        for (u32 i = 0; i < 2; i++)
        {
            u8 spriteId = sBattlerTypeIconSpriteIds[battler][i];
            if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse)
            {
                gSprites[spriteId].tForceHidden = TRUE;
                gSprites[spriteId].invisible = TRUE;
            }
        }
        return;
    }

    types[0] = GetMonPublicType(battler, 0);
    types[1] = GetMonPublicType(battler, 1);
    isDualType = (types[0] != types[1] && types[1] != TYPE_NONE);

    if (IsOnPlayerSide(battler))
    {
        xOffset = -26;
        yBase = 0;
    }
    else
    {
        xOffset = 69;
        yBase = 0;
    }

    // Type 0
    {
        u8 spriteId = sBattlerTypeIconSpriteIds[battler][0];
        if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse)
        {
            struct Sprite *sprite = &gSprites[spriteId];
            enum Type t = types[0];
            const struct SpriteTemplate *template = gTypesInfo[t].useSecondTypeIconPalette ? &sSpriteTemplate_TypeIcons2 : &sSpriteTemplate_TypeIcons1;
            u8 palNum = IndexOfSpritePaletteTag(template->paletteTag);
            u16 tileStart = GetSpriteTileStartByTag(template->tileTag);

            sprite->template = template;
            if (palNum != 0xFF)
                sprite->oam.paletteNum = palNum;
            if (tileStart != 0xFFFF)
            {
                sprite->sheetTileStart = tileStart;
                sprite->oam.tileNum = tileStart;
            }

            sprite->tPosX = xOffset;
            sprite->tPosY = isDualType ? (yBase - 6) : yBase;
            sprite->tForceHidden = FALSE;
            sprite->hFlip = ShouldFlipTypeIcon(battler, t);
            StartSpriteAnim(sprite, t);
        }
    }

    // Type 1
    {
        u8 spriteId = sBattlerTypeIconSpriteIds[battler][1];
        if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse)
        {
            struct Sprite *sprite = &gSprites[spriteId];
            if (!isDualType)
            {
                sprite->tForceHidden = TRUE;
                sprite->invisible = TRUE;
            }
            else
            {
                enum Type t = types[1];
                const struct SpriteTemplate *template = gTypesInfo[t].useSecondTypeIconPalette ? &sSpriteTemplate_TypeIcons2 : &sSpriteTemplate_TypeIcons1;
                u8 palNum = IndexOfSpritePaletteTag(template->paletteTag);
                u16 tileStart = GetSpriteTileStartByTag(template->tileTag);

                sprite->template = template;
                if (palNum != 0xFF)
                    sprite->oam.paletteNum = palNum;
                if (tileStart != 0xFFFF)
                {
                    sprite->sheetTileStart = tileStart;
                    sprite->oam.tileNum = tileStart;
                }

                sprite->tPosX = xOffset;
                sprite->tPosY = yBase + 6;
                sprite->tForceHidden = FALSE;
                sprite->hFlip = ShouldFlipTypeIcon(battler, t);
                StartSpriteAnim(sprite, t);
            }
        }
    }
}

void LoadTypeIcons(enum BattlerId battler)
{
    UpdateBattlerTypeIcons(battler);
}

static enum Type GetMonPublicType(enum BattlerId battlerId, u32 typeNum)
{
    struct Pokemon *mon = GetBattlerMon(battlerId);
    enum Species monSpecies = GetMonData(mon, MON_DATA_SPECIES, NULL);
    struct Pokemon *monIllusion;
    enum Species illusionSpecies;

    if (monSpecies == SPECIES_NONE)
        return TYPE_NONE;

    monIllusion = GetIllusionMonPtr(battlerId);
    illusionSpecies = GetMonData(monIllusion, MON_DATA_SPECIES, NULL);

    if (GetActiveGimmick(battlerId) == GIMMICK_TERA)
        return GetMonDefensiveTeraType(mon, monIllusion, battlerId, typeNum, illusionSpecies, monSpecies);

    if (IsIllusionActiveAndTypeUnchanged(monIllusion, monSpecies, battlerId))
        return GetSpeciesType(illusionSpecies, typeNum);

    if (gBattleMons[battlerId].types[typeNum] != TYPE_NONE)
        return gBattleMons[battlerId].types[typeNum];

    return GetSpeciesType(monSpecies, typeNum);
}


static enum Type GetMonDefensiveTeraType(struct Pokemon *mon, struct Pokemon *monIllusion, enum BattlerId battlerId, u32 typeNum, enum Species illusionSpecies, enum Species monSpecies)
{
    enum Type teraType = GetBattlerTeraType(battlerId);
    enum Species targetSpecies;

    if (teraType != TYPE_STELLAR)
        return teraType;

    targetSpecies = (monIllusion != NULL) ? illusionSpecies : monSpecies;

    return GetSpeciesType(targetSpecies, typeNum);
}

static bool32 IsIllusionActiveAndTypeUnchanged(struct Pokemon *monIllusion, enum Species monSpecies, enum BattlerId battlerId)
{
    u32 typeNum;

    if (monIllusion == NULL)
        return FALSE;

    for (typeNum = 0; typeNum < 2; typeNum++)
        if (GetSpeciesType(monSpecies, typeNum) != gBattleMons[battlerId].types[typeNum])
            return FALSE;

    return TRUE;
}

static void SpriteCB_TypeIcon(struct Sprite *sprite)
{
    u8 battler = sprite->tBattler;
    u8 healthboxId = gHealthboxSpriteIds[battler];

    if (healthboxId < MAX_SPRITES && gSprites[healthboxId].inUse)
    {
        sprite->x = gSprites[healthboxId].x + sprite->tPosX;
        sprite->y = gSprites[healthboxId].y + sprite->tPosY;
        sprite->x2 = gSprites[healthboxId].x2;
        sprite->y2 = gSprites[healthboxId].y2;
        sprite->invisible = gSprites[healthboxId].invisible || sprite->tForceHidden;
        sprite->oam.priority = gSprites[healthboxId].oam.priority;
    }
    else
    {
        sprite->invisible = TRUE;
    }
}

