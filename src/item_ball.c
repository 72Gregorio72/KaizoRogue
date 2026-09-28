#include "global.h"
#include "item_ball.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "kaizo_randomizer.h"

void GetItemBallIdAndAmountFromTemplate(void)
{
    const struct ObjectEventTemplate *template = GetObjectEventTemplateByLocalIdAndMap(
        gSpecialVar_LastTalked,
        gSaveBlock1Ptr->location.mapNum,
        gSaveBlock1Ptr->location.mapGroup
    );

    u16 itemId = ITEM_NONE + 1;
    u16 quantity = 1;

    if (template != NULL)
    {
        itemId = template->trainerRange_berryTreeId;
        quantity = template->movementRangeX;
        if (quantity == 0)
            quantity = 1;
        if (quantity > MAX_BAG_ITEM_CAPACITY)
            quantity = MAX_BAG_ITEM_CAPACITY;
        if (itemId >= ITEMS_COUNT)
            itemId = ITEM_NONE + 1;
    }

    gSpecialVar_Result = GetKaizoRandomizedItem(itemId);
    gSpecialVar_0x8009 = quantity;
}
