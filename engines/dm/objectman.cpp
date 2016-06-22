#include "objectman.h"
#include "dungeonman.h"


namespace DM {

SlotBox gSlotBoxes[46] = { 
/* 8 for champion hands in status boxes, 30 for champion inventory, 8 for chest */
	SlotBox(4,  10, 0), /* Champion Status Box 0 Ready Hand */
	SlotBox(24,  10, 0), /* Champion Status Box 0 Action Hand */
	SlotBox(73,  10, 0), /* Champion Status Box 1 Ready Hand */
	SlotBox(93,  10, 0), /* Champion Status Box 1 Action Hand */
	SlotBox(142,  10, 0), /* Champion Status Box 2 Ready Hand */
	SlotBox(162,  10, 0), /* Champion Status Box 2 Action Hand */
	SlotBox(211,  10, 0), /* Champion Status Box 3 Ready Hand */
	SlotBox(231,  10, 0), /* Champion Status Box 3 Action Hand */
	SlotBox(6,  53, 0), /* Ready Hand */
	SlotBox(62,  53, 0), /* Action Hand */
	SlotBox(34,  26, 0), /* Head */
	SlotBox(34,  46, 0), /* Torso */
	SlotBox(34,  66, 0), /* Legs */
	SlotBox(34,  86, 0), /* Feet */
	SlotBox(6,  90, 0), /* Pouch 2 */
	SlotBox(79,  73, 0), /* Quiver Line2 1 */
	SlotBox(62,  90, 0), /* Quiver Line1 2 */
	SlotBox(79,  90, 0), /* Quiver Line2 2 */
	SlotBox(6,  33, 0), /* Neck */
	SlotBox(6,  73, 0), /* Pouch 1 */
	SlotBox(62,  73, 0), /* Quiver Line1 1 */
	SlotBox(66,  33, 0), /* Backpack Line1 1 */
	SlotBox(83,  16, 0), /* Backpack Line2 2 */
	SlotBox(100,  16, 0), /* Backpack Line2 3 */
	SlotBox(117,  16, 0), /* Backpack Line2 4 */
	SlotBox(134,  16, 0), /* Backpack Line2 5 */
	SlotBox(151,  16, 0), /* Backpack Line2 6 */
	SlotBox(168,  16, 0), /* Backpack Line2 7 */
	SlotBox(185,  16, 0), /* Backpack Line2 8 */
	SlotBox(202,  16, 0), /* Backpack Line2 9 */
	SlotBox(83,  33, 0), /* Backpack Line1 2 */
	SlotBox(100,  33, 0), /* Backpack Line1 3 */
	SlotBox(117,  33, 0), /* Backpack Line1 4 */
	SlotBox(134,  33, 0), /* Backpack Line1 5 */
	SlotBox(151,  33, 0), /* Backpack Line1 6 */
	SlotBox(168,  33, 0), /* Backpack Line1 7 */
	SlotBox(185,  33, 0), /* Backpack Line1 8 */
	SlotBox(202,  33, 0), /* Backpack Line1 9 */
	SlotBox(117,  59, 0), /* Chest 1 */
	SlotBox(106,  76, 0), /* Chest 2 */
	SlotBox(111,  93, 0), /* Chest 3 */
	SlotBox(128,  98, 0), /* Chest 4 */
	SlotBox(145, 101, 0), /* Chest 5 */
	SlotBox(162, 103, 0), /* Chest 6 */
	SlotBox(179, 104, 0), /* Chest 7 */
	SlotBox(196, 105, 0)}; /* Chest 8 */

int16 gIconGraphicHeight[7] = {32, 32, 32, 32, 32, 32, 32}; // @ K0077_ai_IconGraphicHeight

int16 gIconGraphicFirstIndex[7] = { // G0026_ai_Graphic562_IconGraphicFirstIconIndex
	0,     /* First icon index in graphic #42 */
	32,    /* First icon index in graphic #43 */
	64,    /* First icon index in graphic #44 */
	96,    /* First icon index in graphic #45 */
	128,   /* First icon index in graphic #46 */
	160,   /* First icon index in graphic #47 */
	192}; /* First icon index in graphic #48 */

ObjectMan::ObjectMan(DMEngine *vm) : _vm(vm) {}

IconIndice ObjectMan::getObjectType(Thing thing) {
	if (thing == Thing::_thingNone)
		return kIconIndiceNone;

	int16 objectInfoIndex = _vm->_dungeonMan->getObjectInfoIndex(thing);
	if (objectInfoIndex != -1) {
		objectInfoIndex = gObjectInfo[objectInfoIndex]._type;
	}
	return (IconIndice)objectInfoIndex;
}

byte gChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}; // @ G0029_auc_Graphic562_ChargeCountToTorchType

IconIndice ObjectMan::getIconIndex(Thing thing) {
	IconIndice iconIndex = getObjectType(thing);

	if ((iconIndex != kIconIndiceNone) &&
		((iconIndex < kIconIndiceWeaponDagger) && (iconIndex >= kIconIndiceJunkCompassNorth)) || // < instead of <= is no error
		((iconIndex >= kIconIndicePotionMaPotionMonPotion) && (iconIndex <= kIconIndicePotionWaterFlask)) ||
		(iconIndex == kIconIndicePotionEmptyFlask)
		) {
		uint16 *rawType = _vm->_dungeonMan->getThingData(thing);
		switch (iconIndex) {
		case kIconIndiceJunkCompassNorth:
			iconIndex = (IconIndice)(iconIndex + _vm->_dungeonMan->_currMap._partyDir);
			break;
		case kIconIndiceWeaponTorchUnlit: {
			Weapon weapon(rawType);
			if (weapon.isLit()) {
				iconIndex = (IconIndice)(iconIndex + gChargeCountToTorchType[weapon.getChargeCount()]);
			}
			break;
		}
		case kIconIndiceScrollOpen:
			if (Scroll(rawType).getClosed()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case kIconIndiceJunkWater:
		case kIconIndiceJunkIllumuletUnequipped:
		case kIconIndiceJunkJewelSymalUnequipped:
			if (Junk(rawType).getChargeCount()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case kIconIndiceWeaponBoltBladeStormEmpty:
		case kIconIndiceWeaponFlamittEmpty:
		case kIconIndiceWeaponStormringEmpty:
		case kIconIndiceWeaponFuryRaBladeEmpty:
		case kIconIndiceWeaponEyeOfTimeEmpty:
		case kIconIndiceWeaponStaffOfClawsEmpty:
			if (Weapon(rawType).getChargeCount()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		}
	}

	return iconIndex;
}

void ObjectMan::extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	int16 i;
	for (i = 0; i < 7; ++i)
		if (gIconGraphicFirstIndex[i] > iconIndex)
			break;
	--i;
	byte *srcBitmap = _vm->_displayMan->getBitmap(kObjectIcons_000_TO_031 + i);
	iconIndex -= gIconGraphicFirstIndex[i];
	_vm->_displayMan->_useByteBoxCoordinates = true;
	_vm->_displayMan->blitToBitmap(srcBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, destBitmap, 16, 0, 16, 0, 16, kColorNoTransparency);

}

void ObjectMan::drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex) {
	SlotBox *slotBox = &gSlotBoxes[slotBoxIndex];
	slotBox->_iconIndex = iconIndex; // yes, this modifies the global array
	if (slotBox->_iconIndex == kIconIndiceNone) {
		return;
	}
	Box box;
	box._x1 = slotBox->_x;
	box._y1 = slotBox->_y;
	box._x2 = box._x1 + 15; // no need to add +1, comes from a global array 
	box._y2 = box._y1 + 15; // no need to add +1, comes from a global array
	
	uint16 iconGraphicIndex;
	for (iconGraphicIndex = 0; iconGraphicIndex < 7; ++iconGraphicIndex) {
		if (gIconGraphicFirstIndex[iconGraphicIndex] > iconIndex) {
			break;
		}
	}
	iconGraphicIndex--;
	byte *iconsBitmap = _vm->_displayMan->getBitmap(iconGraphicIndex + kObjectIcons_000_TO_031);
	iconIndex -= gIconGraphicFirstIndex[iconGraphicIndex];

	_vm->_displayMan->_useByteBoxCoordinates = false;
	if (slotBoxIndex >= kSlotBoxInventoryFirstSlot) {
		_vm->_displayMan->blitToScreen(iconsBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, box, kColorNoTransparency, gDungeonViewport);
	} else {
		_vm->_displayMan->blitToScreen(iconsBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, box, kColorNoTransparency, gDefultViewPort);
	}
}

}
