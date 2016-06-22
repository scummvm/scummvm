#include "dm.h"
#include "champion.h"


namespace DM {

#define kSlotBoxInventoryFirstSlot 8 // @ C08_SLOT_BOX_INVENTORY_FIRST_SLOT   
#define kSlotBoxInventoryActionHand 9 // @ C09_SLOT_BOX_INVENTORY_ACTION_HAND  
#define kSlotBoxChestFirstSlot 38 // @ C38_SLOT_BOX_CHEST_FIRST_SLOT      

class SlotBox {
public:
	int16 _x;
	int16 _y;
	int16 _iconIndex;
	SlotBox(int16 x, int16 y, int16 iconIndex): _x(x), _y(y), _iconIndex(iconIndex) {}
}; // @ SLOT_BOX

extern SlotBox gSlotBoxes[46]; // @ G0030_as_Graphic562_SlotBoxes;

class ObjectMan {
	DMEngine *_vm;
public:
	explicit ObjectMan(DMEngine *vm);

	IconIndice getObjectType(Thing thing); // @ F0032_OBJECT_GetType
	IconIndice getIconIndex(Thing thing); // @ F0033_OBJECT_GetIconIndex
	void extractIconFromBitmap(uint16 iconIndex, byte *srcBitmap); // F0036_OBJECT_ExtractIconFromBitmap
	void drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex); // @ F0038_OBJECT_DrawIconInSlotBox
};

}
