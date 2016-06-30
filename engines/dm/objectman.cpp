/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "objectman.h"
#include "dungeonman.h"
#include "text.h"

namespace DM {

int16 gIconGraphicHeight[7] = {32, 32, 32, 32, 32, 32, 32}; // @ K0077_ai_IconGraphicHeight

int16 gIconGraphicFirstIndex[7] = { // G0026_ai_Graphic562_IconGraphicFirstIconIndex
	0,     /* First icon index in graphic #42 */
	32,    /* First icon index in graphic #43 */
	64,    /* First icon index in graphic #44 */
	96,    /* First icon index in graphic #45 */
	128,   /* First icon index in graphic #46 */
	160,   /* First icon index in graphic #47 */
	192}; /* First icon index in graphic #48 */

ObjectMan::ObjectMan(DMEngine *vm) : _vm(vm) {
	/* 8 for champion hands in status boxes, 30 for champion inventory, 8 for chest */
	_slotBoxes[0] = SlotBox(4, 10, 0);    /* Champion Status Box 0 Ready Hand */
	_slotBoxes[1] = SlotBox(24, 10, 0);   /* Champion Status Box 0 Action Hand */
	_slotBoxes[2] = SlotBox(73, 10, 0);   /* Champion Status Box 1 Ready Hand */
	_slotBoxes[3] = SlotBox(93, 10, 0);   /* Champion Status Box 1 Action Hand */
	_slotBoxes[4] = SlotBox(142, 10, 0);  /* Champion Status Box 2 Ready Hand */
	_slotBoxes[5] = SlotBox(162, 10, 0);  /* Champion Status Box 2 Action Hand */
	_slotBoxes[6] = SlotBox(211, 10, 0);  /* Champion Status Box 3 Ready Hand */
	_slotBoxes[7] = SlotBox(231, 10, 0);  /* Champion Status Box 3 Action Hand */
	_slotBoxes[8] = SlotBox(6, 53, 0);    /* Ready Hand */
	_slotBoxes[9] = SlotBox(62, 53, 0);   /* Action Hand */
	_slotBoxes[10] = SlotBox(34, 26, 0);  /* Head */
	_slotBoxes[11] = SlotBox(34, 46, 0);  /* Torso */
	_slotBoxes[12] = SlotBox(34, 66, 0);  /* Legs */
	_slotBoxes[13] = SlotBox(34, 86, 0);  /* Feet */
	_slotBoxes[14] = SlotBox(6, 90, 0);   /* Pouch 2 */
	_slotBoxes[15] = SlotBox(79, 73, 0);  /* Quiver Line2 1 */
	_slotBoxes[16] = SlotBox(62, 90, 0);  /* Quiver Line1 2 */
	_slotBoxes[17] = SlotBox(79, 90, 0);  /* Quiver Line2 2 */
	_slotBoxes[18] = SlotBox(6, 33, 0);   /* Neck */
	_slotBoxes[19] = SlotBox(6, 73, 0);   /* Pouch 1 */
	_slotBoxes[20] = SlotBox(62, 73, 0);  /* Quiver Line1 1 */
	_slotBoxes[21] = SlotBox(66, 33, 0);  /* Backpack Line1 1 */
	_slotBoxes[22] = SlotBox(83, 16, 0);  /* Backpack Line2 2 */
	_slotBoxes[23] = SlotBox(100, 16, 0); /* Backpack Line2 3 */
	_slotBoxes[24] = SlotBox(117, 16, 0); /* Backpack Line2 4 */
	_slotBoxes[25] = SlotBox(134, 16, 0); /* Backpack Line2 5 */
	_slotBoxes[26] = SlotBox(151, 16, 0); /* Backpack Line2 6 */
	_slotBoxes[27] = SlotBox(168, 16, 0); /* Backpack Line2 7 */
	_slotBoxes[28] = SlotBox(185, 16, 0); /* Backpack Line2 8 */
	_slotBoxes[29] = SlotBox(202, 16, 0); /* Backpack Line2 9 */
	_slotBoxes[30] = SlotBox(83, 33, 0);  /* Backpack Line1 2 */
	_slotBoxes[31] = SlotBox(100, 33, 0); /* Backpack Line1 3 */
	_slotBoxes[32] = SlotBox(117, 33, 0); /* Backpack Line1 4 */
	_slotBoxes[33] = SlotBox(134, 33, 0); /* Backpack Line1 5 */
	_slotBoxes[34] = SlotBox(151, 33, 0); /* Backpack Line1 6 */
	_slotBoxes[35] = SlotBox(168, 33, 0); /* Backpack Line1 7 */
	_slotBoxes[36] = SlotBox(185, 33, 0); /* Backpack Line1 8 */
	_slotBoxes[37] = SlotBox(202, 33, 0); /* Backpack Line1 9 */
	_slotBoxes[38] = SlotBox(117, 59, 0); /* Chest 1 */
	_slotBoxes[39] = SlotBox(106, 76, 0); /* Chest 2 */
	_slotBoxes[40] = SlotBox(111, 93, 0); /* Chest 3 */
	_slotBoxes[41] = SlotBox(128, 98, 0); /* Chest 4 */
	_slotBoxes[42] = SlotBox(145, 101, 0); /* Chest 5 */
	_slotBoxes[43] = SlotBox(162, 103, 0); /* Chest 6 */
	_slotBoxes[44] = SlotBox(179, 104, 0); /* Chest 7 */
	_slotBoxes[45] = SlotBox(196, 105, 0); /* Chest 8 */

	_objectIconForMousePointer = nullptr;
}

ObjectMan::~ObjectMan() {
	delete[] _objectIconForMousePointer;
	delete[] _objectNames[0];
}

#define kObjectNamesGraphicIndice 556 // @ C556_GRAPHIC_OBJECT_NAMES

void ObjectMan::loadObjectNames() {
	DisplayMan &dispMan = *_vm->_displayMan;

	_objectIconForMousePointer = new byte[16 * 16];

	char *objectNames = new char[dispMan.getCompressedDataSize(kObjectNamesGraphicIndice) + kObjectNameCount];
	Common::MemoryReadStream stream = dispMan.getCompressedData(kObjectNamesGraphicIndice);

	for (uint16 objNameIndex = 0; objNameIndex < kObjectNameCount; ++objNameIndex) {
		_objectNames[objNameIndex] = objectNames;

		byte tmpByte;
		for (tmpByte = stream.readByte(); !(tmpByte & 0x80); tmpByte = stream.readByte()) // last char of object name has 7th bit on
			*objectNames++ = tmpByte; // write while not last char

		*objectNames++ = tmpByte & 0x7F; // write without the 7th bit
		*objectNames++ = '\0'; // terminate string
	}
}

IconIndice ObjectMan::getObjectType(Thing thing) {
	if (thing == Thing::_none)
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
		(((iconIndex < kIconIndiceWeaponDagger) && (iconIndex >= kIconIndiceJunkCompassNorth)) || // < instead of <= is no error
		((iconIndex >= kIconIndicePotionMaPotionMonPotion) && (iconIndex <= kIconIndicePotionWaterFlask)) ||
		 (iconIndex == kIconIndicePotionEmptyFlask))
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
		default:
			break;
		}
	}

	return iconIndex;
}

void ObjectMan::extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	int16 i;
	for (i = 0; i < 7; ++i) {
		if (gIconGraphicFirstIndex[i] > iconIndex)
			break;
	}

	--i;
	byte *srcBitmap = _vm->_displayMan->getBitmap(kObjectIcons_000_TO_031 + i);
	iconIndex -= gIconGraphicFirstIndex[i];
	_vm->_displayMan->_useByteBoxCoordinates = true;
	_vm->_displayMan->blitToBitmap(srcBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, destBitmap, 16, 0, 16, 0, 16, kColorNoTransparency);
}

void ObjectMan::drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex) {
	SlotBox *slotBox = &_slotBoxes[slotBoxIndex];
	slotBox->_iconIndex = iconIndex; // yes, this modifies the global array
	if (slotBox->_iconIndex == kIconIndiceNone) {
		return;
	}

	Box box;
	box._x1 = slotBox->_x;
	box._y1 = slotBox->_y;
	box._x2 = box._x1 + 15 + 1;
	box._y2 = box._y1 + 15 + 1;

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
		_vm->_displayMan->blitToScreen(iconsBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0,
									   box, kColorNoTransparency, gDungeonViewport);
	} else {
		_vm->_displayMan->blitToScreen(iconsBitmap, 256, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0,
									   box, kColorNoTransparency, gDefultViewPort);
	}
}

#define kObjectNameMaximumLength 14 // @ C014_OBJECT_NAME_MAXIMUM_LENGTH

void ObjectMan::drawLeaderObjectName(Thing thing) {
	IconIndice iconIndex = getIconIndex(thing);
	char *objName;
	char objectNameBuffer[16];
	if (iconIndex == kIconIndiceJunkChampionBones) {
		Junk *junk = (Junk*)_vm->_dungeonMan->getThingData(thing);
		strcpy(objectNameBuffer, _vm->_championMan->_champions[junk->getChargeCount()]._name);
		strcat(objectNameBuffer, _objectNames[iconIndex]);
		objName = objectNameBuffer;
	} else {
		objName = _objectNames[iconIndex];
	}
	_vm->_textMan->printWithTrailingSpacesToScreen(233, 37, kColorCyan, kColorBlack, objName, kObjectNameMaximumLength);
}

IconIndice ObjectMan::getIconIndexInSlotBox(uint16 slotBoxIndex) {
	return (IconIndice)_slotBoxes[slotBoxIndex]._iconIndex;
}
}
