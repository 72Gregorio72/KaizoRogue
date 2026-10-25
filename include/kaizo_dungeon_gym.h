#ifndef GUARD_KAIZO_DUNGEON_GYM_H
#define GUARD_KAIZO_DUNGEON_GYM_H

#include "global.h"

bool8 IsMapGym(u8 mapGroup, u8 mapNum, u16 *outBadgeFlag);
bool8 IsDungeonMap(u8 mapsec);
bool8 IsDungeonCleared(u8 mapsec);
bool8 IsCurrentDungeonCleared(void);
bool8 IsForwardDungeonExit(u8 srcMapGroup, u8 srcMapNum, u8 destMapGroup, u8 destMapNum, u8 destWarpId);
void CheckAndMarkDungeonClearedOnWarp(u8 srcMapGroup, u8 srcMapNum, u8 destMapGroup, u8 destMapNum, u8 destWarpId);
bool8 TryBlockGymOrDungeonWarp(s8 warpEventId);
void Script_StepPlayerAwayFromExit(void);

#endif // GUARD_KAIZO_DUNGEON_GYM_H
