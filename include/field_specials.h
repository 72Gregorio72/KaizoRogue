#ifndef GUARD_FIELD_SPECIALS_H
#define GUARD_FIELD_SPECIALS_H

#include "constants/species.h"

extern bool8 gBikeCyclingChallenge;
extern u8 gBikeCollisions;
extern u16 gScrollableMultichoice_ScrollOffset;

u8 GetLeadMonIndex(void);
bool8 IsDestinationBoxFull(void);
u16 GetPCBoxToSendMon(void);
bool8 InMultiPartnerRoom(void);
void UpdateTrainerFansAfterLinkBattle(void);
void IncrementBirthIslandRockStepCount(void);
bool8 AbnormalWeatherHasExpired(void);
bool8 ShouldDoBrailleRegicePuzzle(void);
bool32 ShouldDoWallyCall(void);
bool32 ShouldDoScottFortreeCall(void);
bool32 ShouldDoScottBattleFrontierCall(void);
bool32 ShouldDoRoxanneCall(void);
bool32 ShouldDoRivalRayquazaCall(void);
bool32 CountSSTidalStep(u16 delta);
enum SSTidalLocation GetSSTidalLocation(s8 *mapGroup, s8 *mapNum, s16 *x, s16 *y);
void ShowScrollableMultichoice(void);
void FrontierGamblerSetWonOrLost(bool8 won);
u8 TryGainNewFanFromCounter(u8 incrementId);
bool8 InPokemonCenter(void);
void SetShoalItemFlag(u16 unused);
void UpdateFrontierManiac(u16 daysSince);
void UpdateFrontierGambler(u16 daysSince);
void ResetCyclingRoadChallengeData(void);
bool8 UsedPokemonCenterWarp(void);
void ResetFanClub(void);
bool8 ShouldShowBoxWasFullMessage(void);
void SetPCBoxToSendMon(u8 boxId);
void PreparePartyForSkyBattle(void);
void GetObjectPosition(u16*, u16*, u32, u32);
bool32 CheckObjectAtXY(u32, u32);
bool32 CheckPartyHasSpecies(enum Species);
bool8 CutMoveRuinValleyCheck(void);
void CutMoveOpenDottedHoleDoor(void);

void GenerateNewKaizoSeedAndStarters(void);
void GiveSelectedStarter(void);
void BufferKaizoStarterName(void);
void Script_GetKaizoStarterSpecies(void);
void Script_GiveKaizoStarter(void);
void Script_SetupOaksLabStarter(void);
void Script_RandomizeFindItemResult(void);
void Script_ToggleWildEncounters(void);
u32 GetCurrentExpMultiplier(void);
void BufferCurrentExpMultiplierName(void);


// IV Booster NPC
void Script_IVBooster_Init(void);
void Script_IVBooster_GetRemainingPoints(void);
void Script_IVBooster_PushStatChoices(void);
void Script_IVBooster_SelectStat(void);
void Script_IVBooster_PushPointChoices(void);
void Script_IVBooster_ApplyPoints(void);

// EV Manager NPC
void Script_EVManager_Init(void);
void Script_EVManager_GetRemainingPoints(void);
void Script_EVManager_PushStatChoices(void);
void Script_EVManager_SelectStat(void);
void Script_EVManager_PushPointChoices(void);
void Script_EVManager_ApplyPoints(void);
void Script_EVManager_ResetAllEVs(void);

// Nature Changer NPC
void Script_NatureChanger_Init(void);
void Script_NatureChanger_Apply(void);

// Ability Changer NPC
void Script_AbilityChanger_Init(void);
void Script_AbilityChanger_Apply(void);

// Poké Mart Build Hub & Stat Points
void Script_MartBuild_CheckCanUse(void);
void Script_MartBuild_Consume(void);
void Script_MartBuild_InitSession(void);
void Script_MartBuild_PushMainMenuChoices(void);
void Script_MartBuild_SelectOption(void);
void Script_MartBuild_MarkOptionUsed(void);
void Script_MartBuild_ExitSession(void);
void Script_StatPoints_Init(void);
void Script_StatPoints_GetRemainingPoints(void);
void Script_StatPoints_PushStatChoices(void);
void Script_StatPoints_SelectStat(void);
void Script_StatPoints_PushAmountChoices(void);
void Script_StatPoints_Apply(void);
void Script_MoveDraft_Roll(void);
void Script_MoveDraft_Select(void);
void Script_Starter_RollAttackMoves(void);
void Script_Starter_SelectAttackMove(void);

#endif // GUARD_FIELD_SPECIALS_H
