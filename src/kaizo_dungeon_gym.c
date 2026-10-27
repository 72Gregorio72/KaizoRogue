#include "global.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "kaizo_dungeon_gym.h"
#include "overworld.h"
#include "script.h"
#include "script_movement.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/region_map_sections.h"

extern const u8 EventScript_GymExitBlocked[];
extern const u8 EventScript_DungeonExitBlocked[];

static const u8 sMoveUp[]    = { MOVEMENT_ACTION_WALK_NORMAL_UP,    MOVEMENT_ACTION_STEP_END };
static const u8 sMoveDown[]  = { MOVEMENT_ACTION_WALK_NORMAL_DOWN,  MOVEMENT_ACTION_STEP_END };
static const u8 sMoveLeft[]  = { MOVEMENT_ACTION_WALK_NORMAL_LEFT,  MOVEMENT_ACTION_STEP_END };
static const u8 sMoveRight[] = { MOVEMENT_ACTION_WALK_NORMAL_RIGHT, MOVEMENT_ACTION_STEP_END };

void Script_StepPlayerAwayFromExit(void)
{
    const u8 *script = sMoveUp;
    switch (GetPlayerFacingDirection())
    {
    case DIR_SOUTH:
        script = sMoveUp;
        break;
    case DIR_NORTH:
        script = sMoveDown;
        break;
    case DIR_EAST:
        script = sMoveLeft;
        break;
    case DIR_WEST:
        script = sMoveRight;
        break;
    default:
        script = sMoveUp;
        break;
    }
    ScriptMovement_StartObjectMovementScript(LOCALID_PLAYER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, script);
}

bool8 IsMapGym(u8 mapGroup, u8 mapNum, u16 *outBadgeFlag)
{
    if (mapGroup == MAP_GROUP(MAP_PEWTER_CITY_GYM) && mapNum == MAP_NUM(MAP_PEWTER_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE01_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_CERULEAN_CITY_GYM) && mapNum == MAP_NUM(MAP_CERULEAN_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE02_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_VERMILION_CITY_GYM) && mapNum == MAP_NUM(MAP_VERMILION_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE03_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_CELADON_CITY_GYM) && mapNum == MAP_NUM(MAP_CELADON_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE04_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_FUCHSIA_CITY_GYM) && mapNum == MAP_NUM(MAP_FUCHSIA_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE05_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_SAFFRON_CITY_GYM) && mapNum == MAP_NUM(MAP_SAFFRON_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE06_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_CINNABAR_ISLAND_GYM) && mapNum == MAP_NUM(MAP_CINNABAR_ISLAND_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE07_GET;
        return TRUE;
    }
    if (mapGroup == MAP_GROUP(MAP_VIRIDIAN_CITY_GYM) && mapNum == MAP_NUM(MAP_VIRIDIAN_CITY_GYM))
    {
        if (outBadgeFlag) *outBadgeFlag = FLAG_BADGE08_GET;
        return TRUE;
    }
    return FALSE;
}

bool8 IsDungeonMap(u8 mapsec)
{
    switch (mapsec)
    {
    case MAPSEC_VIRIDIAN_FOREST:
    case MAPSEC_MT_MOON:
    case MAPSEC_S_S_ANNE:
    case MAPSEC_ROCK_TUNNEL:
    case MAPSEC_POKEMON_TOWER:
    case MAPSEC_ROCKET_HIDEOUT:
    case MAPSEC_SILPH_CO:
    case MAPSEC_POKEMON_MANSION:
    case MAPSEC_KANTO_SAFARI_ZONE:
    case MAPSEC_SEAFOAM_ISLANDS:
    case MAPSEC_POWER_PLANT:
    case MAPSEC_KANTO_VICTORY_ROAD:
    case MAPSEC_DIGLETTS_CAVE:
    case MAPSEC_CERULEAN_CAVE:
    case MAPSEC_BERRY_FOREST:
    case MAPSEC_MT_EMBER:
    case MAPSEC_ICEFALL_CAVE:
    case MAPSEC_ROCKET_WAREHOUSE:
    case MAPSEC_LOST_CAVE:
    case MAPSEC_PATTERN_BUSH:
    case MAPSEC_ALTERING_CAVE_FRLG:
        return TRUE;
    default:
        return FALSE;
    }
}

bool8 IsDungeonCleared(u8 mapsec)
{
    switch (mapsec)
    {
    case MAPSEC_VIRIDIAN_FOREST:
        return FlagGet(FLAG_CLEARED_VIRIDIAN_FOREST);
    case MAPSEC_MT_MOON:
        return FlagGet(FLAG_CLEARED_MT_MOON);
    case MAPSEC_S_S_ANNE:
        return FlagGet(FLAG_GOT_HM01);
    case MAPSEC_ROCK_TUNNEL:
        return FlagGet(FLAG_CLEARED_ROCK_TUNNEL);
    case MAPSEC_POKEMON_TOWER:
        return FlagGet(FLAG_RESCUED_MR_FUJI);
    case MAPSEC_ROCKET_HIDEOUT:
        return FlagGet(FLAG_HIDE_CELADON_ROCKETS);
    case MAPSEC_SILPH_CO:
        return FlagGet(FLAG_HIDE_SAFFRON_ROCKETS);
    case MAPSEC_POKEMON_MANSION:
        return FlagGet(FLAG_HIDE_POKEMON_MANSION_B1F_SECRET_KEY);
    case MAPSEC_KANTO_SAFARI_ZONE:
        return FlagGet(FLAG_HIDE_SAFARI_ZONE_WEST_GOLD_TEETH);
    case MAPSEC_SEAFOAM_ISLANDS:
        return FlagGet(FLAG_CLEARED_SEAFOAM_ISLANDS) || FlagGet(FLAG_FOUGHT_ARTICUNO);
    case MAPSEC_POWER_PLANT:
        return FlagGet(FLAG_FOUGHT_ZAPDOS);
    case MAPSEC_KANTO_VICTORY_ROAD:
        return FlagGet(FLAG_CLEARED_VICTORY_ROAD);
    case MAPSEC_DIGLETTS_CAVE:
        return FlagGet(FLAG_CLEARED_DIGLETTS_CAVE);
    case MAPSEC_CERULEAN_CAVE:
        return FlagGet(FLAG_FOUGHT_MEWTWO);
    default:
        return TRUE;
    }
}

bool8 IsCurrentDungeonCleared(void)
{
    u8 mapsec = gMapHeader.regionMapSectionId;
    if (!IsDungeonMap(mapsec))
        return TRUE;
    return IsDungeonCleared(mapsec);
}

bool8 IsForwardDungeonExit(u8 srcMapGroup, u8 srcMapNum, u8 destMapGroup, u8 destMapNum, u8 destWarpId)
{
    // Viridian Forest: uscita nord verso Route 2 / Pewter
    if (srcMapGroup == MAP_GROUP(MAP_VIRIDIAN_FOREST) && srcMapNum == MAP_NUM(MAP_VIRIDIAN_FOREST)
        && destMapGroup == MAP_GROUP(MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE) && destMapNum == MAP_NUM(MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE))
        return TRUE;

    // Mt. Moon: uscita da B2F verso Route 4
    if (srcMapGroup == MAP_GROUP(MAP_MT_MOON_B2F) && srcMapNum == MAP_NUM(MAP_MT_MOON_B2F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE4) && destMapNum == MAP_NUM(MAP_ROUTE4))
        return TRUE;

    // Rock Tunnel: uscita da 1F verso Route 10 sud (Lavender) - destWarpId == 1
    if (srcMapGroup == MAP_GROUP(MAP_ROCK_TUNNEL_1F) && srcMapNum == MAP_NUM(MAP_ROCK_TUNNEL_1F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE10) && destMapNum == MAP_NUM(MAP_ROUTE10) && destWarpId == 1)
        return TRUE;

    // Diglett's Cave: uscita da B1F verso North Entrance (Route 2)
    if (srcMapGroup == MAP_GROUP(MAP_DIGLETTS_CAVE_B1F) && srcMapNum == MAP_NUM(MAP_DIGLETTS_CAVE_B1F)
        && destMapGroup == MAP_GROUP(MAP_DIGLETTS_CAVE_NORTH_ENTRANCE) && destMapNum == MAP_NUM(MAP_DIGLETTS_CAVE_NORTH_ENTRANCE))
        return TRUE;

    // Victory Road: uscita da 3F verso Route 23 nord / Indigo Plateau
    if (srcMapGroup == MAP_GROUP(MAP_VICTORY_ROAD_3F) && srcMapNum == MAP_NUM(MAP_VICTORY_ROAD_3F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE23) && destMapNum == MAP_NUM(MAP_ROUTE23))
        return TRUE;

    // Seafoam Islands: uscita da 1F verso Route 20
    if (srcMapGroup == MAP_GROUP(MAP_SEAFOAM_ISLANDS_1F) && srcMapNum == MAP_NUM(MAP_SEAFOAM_ISLANDS_1F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE20) && destMapNum == MAP_NUM(MAP_ROUTE20))
        return TRUE;

    return FALSE;
}

void CheckAndMarkDungeonClearedOnWarp(u8 srcMapGroup, u8 srcMapNum, u8 destMapGroup, u8 destMapNum, u8 destWarpId)
{
    if (srcMapGroup == MAP_GROUP(MAP_VIRIDIAN_FOREST) && srcMapNum == MAP_NUM(MAP_VIRIDIAN_FOREST)
        && destMapGroup == MAP_GROUP(MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE) && destMapNum == MAP_NUM(MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE))
    {
        FlagSet(FLAG_CLEARED_VIRIDIAN_FOREST);
    }
    else if (srcMapGroup == MAP_GROUP(MAP_MT_MOON_B2F) && srcMapNum == MAP_NUM(MAP_MT_MOON_B2F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE4) && destMapNum == MAP_NUM(MAP_ROUTE4))
    {
        FlagSet(FLAG_CLEARED_MT_MOON);
    }
    else if (srcMapGroup == MAP_GROUP(MAP_ROCK_TUNNEL_1F) && srcMapNum == MAP_NUM(MAP_ROCK_TUNNEL_1F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE10) && destMapNum == MAP_NUM(MAP_ROUTE10) && destWarpId == 1)
    {
        FlagSet(FLAG_CLEARED_ROCK_TUNNEL);
    }
    else if (srcMapGroup == MAP_GROUP(MAP_DIGLETTS_CAVE_B1F) && srcMapNum == MAP_NUM(MAP_DIGLETTS_CAVE_B1F)
        && destMapGroup == MAP_GROUP(MAP_DIGLETTS_CAVE_NORTH_ENTRANCE) && destMapNum == MAP_NUM(MAP_DIGLETTS_CAVE_NORTH_ENTRANCE))
    {
        FlagSet(FLAG_CLEARED_DIGLETTS_CAVE);
    }
    else if (srcMapGroup == MAP_GROUP(MAP_VICTORY_ROAD_3F) && srcMapNum == MAP_NUM(MAP_VICTORY_ROAD_3F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE23) && destMapNum == MAP_NUM(MAP_ROUTE23))
    {
        FlagSet(FLAG_CLEARED_VICTORY_ROAD);
    }
    else if (srcMapGroup == MAP_GROUP(MAP_SEAFOAM_ISLANDS_1F) && srcMapNum == MAP_NUM(MAP_SEAFOAM_ISLANDS_1F)
        && destMapGroup == MAP_GROUP(MAP_ROUTE20) && destMapNum == MAP_NUM(MAP_ROUTE20))
    {
        FlagSet(FLAG_CLEARED_SEAFOAM_ISLANDS);
    }
}

bool8 TryBlockGymOrDungeonWarp(s8 warpEventId)
{
    if (warpEventId < 0 || warpEventId >= gMapHeader.events->warpCount)
        return FALSE;

    const struct WarpEvent *warp = &gMapHeader.events->warps[warpEventId];
    u8 curMapGroup = gSaveBlock1Ptr->location.mapGroup;
    u8 curMapNum = gSaveBlock1Ptr->location.mapNum;
    u8 destMapGroup = warp->mapGroup;
    u8 destMapNum = warp->mapNum;
    u8 destWarpId = warp->warpId;

    // 1. Controlla Palestre
    u16 badgeFlag = 0;
    if (IsMapGym(curMapGroup, curMapNum, &badgeFlag))
    {
        if (destMapGroup != curMapGroup || destMapNum != curMapNum)
        {
            if (!FlagGet(badgeFlag))
            {
                ScriptContext_SetupScript(EventScript_GymExitBlocked);
                return TRUE;
            }
        }
    }

    // 2. Dungeon: uscita sempre libera (il giocatore può tornare indietro quando vuole)
    // Segna comunque il completamento se si esce dall'uscita "avanzata"
    u8 curMapsec = gMapHeader.regionMapSectionId;
    if (IsDungeonMap(curMapsec))
    {
        const struct MapHeader *destHeader = Overworld_GetMapHeaderByGroupAndId(destMapGroup, destMapNum);
        if (destHeader != NULL && destHeader->regionMapSectionId != curMapsec)
            CheckAndMarkDungeonClearedOnWarp(curMapGroup, curMapNum, destMapGroup, destMapNum, destWarpId);
    }

    return FALSE;
}
