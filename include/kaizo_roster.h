#ifndef GUARD_KAIZO_ROSTER_H
#define GUARD_KAIZO_ROSTER_H

#include "global.h"
#include "main.h"

#define NUM_GENERATIONS 9

struct GenerationInfo
{
    const u8 *name;
    u16 startSpecies;
    u16 endSpecies;
};

extern const struct GenerationInfo gGenerationTable[NUM_GENERATIONS];

struct BoxPokemon *GetUnlockedRosterMon(u16 species);
bool8 AddUnlockedRosterMon(struct BoxPokemon *mon);
void SortRosterBoxes(void);
u16 GetGenBaseSpecies(u8 gen, u16 *outList, u16 maxCount);
u8 GetSpeciesRarityTier(u16 species);
const u8 *GetRarityTierName(u8 tier);
void InitKaizoMetaProgression(void);
void AddGachaTickets(u16 amount);
void AddRarityPoints(u8 tier, u16 amount);

void OpenGachaSlotMachine(void);
void OpenGachaRosterPC(void);
void OpenMonBuildScreen(struct BoxPokemon *mon, MainCallback exitCallback);

void Script_BuildMon_ShowPointsSummary(void);
void Script_BuildMon_Select(void);
void Script_BuildMon_RefreshInfo(void);
void Script_BuildMon_PushMainMenu(void);
void Script_BuildMon_ToggleStarter(void);
void Script_BuildMon_PushNatures(void);
void Script_BuildMon_ApplyNature(void);
void Script_BuildMon_PushAbilities(void);
void Script_BuildMon_ApplyAbility(void);
void Script_BuildMon_PushMoves(void);
void Script_BuildMon_ApplyMove(void);
void Script_BuildMon_FuseSelectedMon(void);
void Script_Meta_CheatCleanEggs(void);

#endif // GUARD_KAIZO_ROSTER_H
