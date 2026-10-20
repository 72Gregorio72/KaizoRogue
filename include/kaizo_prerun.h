#ifndef GUARD_KAIZO_PRERUN_H
#define GUARD_KAIZO_PRERUN_H

#include "global.h"

#define KAIZO_PRERUN_MAGIC 0x4B5A5247 // "KZRG" in ASCII (0x47, 0x52, 0x5A, 0x4B in little endian)
#define KAIZO_PRERUN_MAX_ITEMS 16

#define KAIZO_PRERUN_FLAG_ACTIVE          (1 << 0)
#define KAIZO_PRERUN_FLAG_SHINY           (1 << 1)
#define KAIZO_PRERUN_FLAG_SHINY_SKIN_OFF  (1 << 2)
#define KAIZO_PRERUN_FLAG_MEGA_RING       (1 << 3)

struct KaizoWebPrerunConfig {
    u32 magic;                 // 0x4B5A5247
    u16 version;               // 1
    u16 flags;                 // KAIZO_PRERUN_FLAG_*
    u16 starterSpecies;        // species id (e.g. SPECIES_MEWTWO)
    u8  starterLevel;          // 5
    u8  starterNature;         // 0..24 (NATURE_HARDY..NATURE_QUIRKY)
    u8  starterAbilityNum;     // 0, 1, 2
    u8  starterHappiness;      // 255
    u8  starterIvHp;           // 31
    u8  starterIvAtk;          // 31
    u8  starterIvDef;          // 31
    u8  starterIvSpeed;        // 31
    u8  starterIvSpAtk;        // 31
    u8  starterIvSpDef;        // 31
    u16 starterHeldItem;       // held item or mega stone
    u16 starterMove1;          // custom move 1 (or MOVE_NONE)
    u16 starterMove2;          // custom move 2 (or MOVE_NONE)
    u16 bagItems[KAIZO_PRERUN_MAX_ITEMS];
    u16 bagItemQuantities[KAIZO_PRERUN_MAX_ITEMS];
    u32 runSeed;               // custom RNG seed
};

extern const volatile struct KaizoWebPrerunConfig gKaizoWebPrerunConfig;

bool32 HasActiveWebPrerunConfig(void);
void ApplyWebPrerunConfig(void);

#endif // GUARD_KAIZO_PRERUN_H
