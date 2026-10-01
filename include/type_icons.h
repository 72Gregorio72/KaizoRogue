#ifndef GUARD_TYPE_ICONS_H
#define GUARD_TYPE_ICONS_H

#define TYPE_ICON_TAG 0x2720
#define TYPE_ICON_TAG_2 0x2721

#define tBattler          data[0]
#define tTypeNum          data[1]
#define tPosX             data[2]
#define tPosY             data[3]
#define tForceHidden      data[4]

#define TYPE_ICON_1_FRAME(monType) ((monType - 1) * 4)
#define TYPE_ICON_2_FRAME(monType) ((monType - 11) * 4)

void CreateBattlerTypeIcons(enum BattlerId battler);
void UpdateBattlerTypeIcons(enum BattlerId battler);
void DestroyBattlerTypeIcons(enum BattlerId battler);
void LoadTypeIcons(enum BattlerId battler);

#endif // GUARD_TYPE_ICONS_H
