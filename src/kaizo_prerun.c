#include "global.h"
#include "kaizo_prerun.h"
#include "pokemon.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/map_groups.h"
#include "item.h"
#include "event_data.h"
#include "random.h"
#include "battle_main.h"

extern u8 GetSpeciesMegaStones(u16 species, u16 *outStone1, u16 *outStone2);

// Defined with explicit section so it can be found and patched by the web runner
__attribute__((section(".rodata.kaizo_prerun"), used))
const volatile struct KaizoWebPrerunConfig gKaizoWebPrerunConfig = {
    .magic = KAIZO_PRERUN_MAGIC,
    .version = 1,
    .flags = 0,
    .starterSpecies = SPECIES_NONE,
    .starterLevel = 5,
    .starterNature = 0,
    .starterAbilityNum = 0,
    .starterHappiness = 255,
    .starterIvHp = 31,
    .starterIvAtk = 31,
    .starterIvDef = 31,
    .starterIvSpeed = 31,
    .starterIvSpAtk = 31,
    .starterIvSpDef = 31,
    .starterHeldItem = ITEM_NONE,
    .starterMove1 = MOVE_NONE,
    .starterMove2 = MOVE_NONE,
    .bagItems = {0},
    .bagItemQuantities = {0},
    .runSeed = 0
};

bool32 HasActiveWebPrerunConfig(void)
{
    return (gKaizoWebPrerunConfig.magic == KAIZO_PRERUN_MAGIC 
         && (gKaizoWebPrerunConfig.flags & KAIZO_PRERUN_FLAG_ACTIVE));
}

void ApplyWebPrerunConfig(void)
{
    if (!HasActiveWebPrerunConfig())
        return;

    u16 species = gKaizoWebPrerunConfig.starterSpecies;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        species = SPECIES_BULBASAUR;

    u8 level = gKaizoWebPrerunConfig.starterLevel ? gKaizoWebPrerunConfig.starterLevel : 5;

    struct Pokemon mon;
    CreateMon(&mon, species, level, Random32(), OTID_STRUCT_PLAYER_ID);

    // Nature
    u8 nature = gKaizoWebPrerunConfig.starterNature;
    if (nature < NUM_NATURES)
        SetMonData(&mon, MON_DATA_HIDDEN_NATURE, &nature);

    // Ability
    u8 abilityNum = gKaizoWebPrerunConfig.starterAbilityNum;
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);

    // Shiny
    bool32 isShiny = (gKaizoWebPrerunConfig.flags & KAIZO_PRERUN_FLAG_SHINY) ? TRUE : FALSE;
    bool32 shinyUnlocked = isShiny;
    bool32 shinySkinOff = (gKaizoWebPrerunConfig.flags & KAIZO_PRERUN_FLAG_SHINY_SKIN_OFF) ? TRUE : FALSE;

    SetMonData(&mon, MON_DATA_SHINY_UNLOCKED, &shinyUnlocked);
    // If shiny skin is off, isShiny flag for rendering can be false, but shinyUnlocked is true
    bool32 renderShiny = (isShiny && !shinySkinOff);
    SetMonData(&mon, MON_DATA_IS_SHINY, &renderShiny);

    // IVs (all 31)
    u8 ivHp = gKaizoWebPrerunConfig.starterIvHp ? gKaizoWebPrerunConfig.starterIvHp : 31;
    u8 ivAtk = gKaizoWebPrerunConfig.starterIvAtk ? gKaizoWebPrerunConfig.starterIvAtk : 31;
    u8 ivDef = gKaizoWebPrerunConfig.starterIvDef ? gKaizoWebPrerunConfig.starterIvDef : 31;
    u8 ivSpeed = gKaizoWebPrerunConfig.starterIvSpeed ? gKaizoWebPrerunConfig.starterIvSpeed : 31;
    u8 ivSpAtk = gKaizoWebPrerunConfig.starterIvSpAtk ? gKaizoWebPrerunConfig.starterIvSpAtk : 31;
    u8 ivSpDef = gKaizoWebPrerunConfig.starterIvSpDef ? gKaizoWebPrerunConfig.starterIvSpDef : 31;

    SetMonData(&mon, MON_DATA_HP_IV, &ivHp);
    SetMonData(&mon, MON_DATA_ATK_IV, &ivAtk);
    SetMonData(&mon, MON_DATA_DEF_IV, &ivDef);
    SetMonData(&mon, MON_DATA_SPEED_IV, &ivSpeed);
    SetMonData(&mon, MON_DATA_SPATK_IV, &ivSpAtk);
    SetMonData(&mon, MON_DATA_SPDEF_IV, &ivSpDef);

    // Friendship / Happiness locked at 255
    u8 friendship = 255;
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);

    // Held Item
    u16 heldItem = gKaizoWebPrerunConfig.starterHeldItem;
    if (heldItem != ITEM_NONE && heldItem < ITEMS_COUNT)
        SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);

    // Moves
    u16 moveScratch = MOVE_SCRATCH;
    u8 ppScratch = gMovesInfo[MOVE_SCRATCH].pp;
    u16 moveNone = MOVE_NONE;
    u8 ppZero = 0;

    SetMonData(&mon, MON_DATA_MOVE1, &moveScratch);
    SetMonData(&mon, MON_DATA_PP1, &ppScratch);

    if (gKaizoWebPrerunConfig.starterMove1 != MOVE_NONE && gKaizoWebPrerunConfig.starterMove1 < MOVES_COUNT)
    {
        u16 m1 = gKaizoWebPrerunConfig.starterMove1;
        u8 pp1 = gMovesInfo[m1].pp;
        SetMonData(&mon, MON_DATA_MOVE2, &m1);
        SetMonData(&mon, MON_DATA_PP2, &pp1);
    }
    else
    {
        SetMonData(&mon, MON_DATA_MOVE2, &moveNone);
        SetMonData(&mon, MON_DATA_PP2, &ppZero);
    }

    if (gKaizoWebPrerunConfig.starterMove2 != MOVE_NONE && gKaizoWebPrerunConfig.starterMove2 < MOVES_COUNT)
    {
        u16 m2 = gKaizoWebPrerunConfig.starterMove2;
        u8 pp2 = gMovesInfo[m2].pp;
        SetMonData(&mon, MON_DATA_MOVE3, &m2);
        SetMonData(&mon, MON_DATA_PP3, &pp2);
    }
    else
    {
        SetMonData(&mon, MON_DATA_MOVE3, &moveNone);
        SetMonData(&mon, MON_DATA_PP3, &ppZero);
    }

    SetMonData(&mon, MON_DATA_MOVE4, &moveNone);
    SetMonData(&mon, MON_DATA_PP4, &ppZero);

    CalculateMonStats(&mon);

    ZeroPlayerPartyMons();
    GiveCapturedMonToPlayer(&mon);
    gPartiesCount[B_TRAINER_PLAYER] = 1;

    // Bag items purchased in Web Shop
    for (int i = 0; i < KAIZO_PRERUN_MAX_ITEMS; i++)
    {
        u16 item = gKaizoWebPrerunConfig.bagItems[i];
        u16 qty = gKaizoWebPrerunConfig.bagItemQuantities[i];
        if (item != ITEM_NONE && item < ITEMS_COUNT && qty > 0)
        {
            AddBagItem(item, qty);
        }
    }

    // Mega Ring if mega stone is held or flag set
    u16 m1 = ITEM_NONE, m2 = ITEM_NONE;
    u8 megaCount = GetSpeciesMegaStones(species, &m1, &m2);
    if ((gKaizoWebPrerunConfig.flags & KAIZO_PRERUN_FLAG_MEGA_RING) 
     || (megaCount > 0 && (heldItem == m1 || heldItem == m2)))
    {
        if (!CheckBagHasItem(ITEM_MEGA_RING, 1))
            AddBagItem(ITEM_MEGA_RING, 1);
    }

    // Essential adventure items
    AddBagItem(ITEM_TOWN_MAP, 1);
    AddBagItem(ITEM_LIFT_KEY, 1);
    AddBagItem(ITEM_SILPH_SCOPE, 1);
    AddBagItem(ITEM_WILD_ENCOUNTERS, 1);

    // Story flags so Oak lab is cleared and player is free to start the adventure
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_PALLET_LADY_NOT_BLOCKING_SIGN);
    FlagSet(FLAG_CAN_USE_ROCKET_HIDEOUT_LIFT);

    FlagSet(FLAG_HIDE_BULBASAUR_BALL);
    FlagSet(FLAG_HIDE_SQUIRTLE_BALL);
    FlagSet(FLAG_HIDE_CHARMANDER_BALL);
    FlagSet(FLAG_HIDE_RIVAL_IN_LAB);
    FlagSet(FLAG_BEAT_RIVAL_IN_OAKS_LAB);
    FlagSet(FLAG_HIDE_POKEDEX);
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);

    VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 2);
    VarSet(VAR_MAP_SCENE_ROUTE22, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 1);
    VarSet(VAR_MAP_SCENE_POKEMON_CENTER_TEALA, 1);

    // Custom RNG Seed if provided
    if (gKaizoWebPrerunConfig.runSeed != 0)
    {
        gSaveBlock2Ptr->randomizerSeed = gKaizoWebPrerunConfig.runSeed;
    }
}
