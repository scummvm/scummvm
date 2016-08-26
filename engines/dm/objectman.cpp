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

int16 gK77_IconGraphicHeight[7] = {32, 32, 32, 32, 32, 32, 32}; // @ K0077_ai_IconGraphicHeight

int16 g26_IconGraphicFirstIndex[7] = { // G0026_ai_Graphic562_IconGraphicFirstIconIndex
	0,     /* First icon index in graphic #42 */
	32,    /* First icon index in graphic #43 */
	64,    /* First icon index in graphic #44 */
	96,    /* First icon index in graphic #45 */
	128,   /* First icon index in graphic #46 */
	160,   /* First icon index in graphic #47 */
	192}; /* First icon index in graphic #48 */

ObjectMan::ObjectMan(DMEngine *vm) : _vm(vm) {
	/* 8 for champion hands in status boxes, 30 for champion inventory, 8 for chest */
	_g30_slotBoxes[0] = SlotBox(4, 10, 0);    /* Champion Status Box 0 Ready Hand */
	_g30_slotBoxes[1] = SlotBox(24, 10, 0);   /* Champion Status Box 0 Action Hand */
	_g30_slotBoxes[2] = SlotBox(73, 10, 0);   /* Champion Status Box 1 Ready Hand */
	_g30_slotBoxes[3] = SlotBox(93, 10, 0);   /* Champion Status Box 1 Action Hand */
	_g30_slotBoxes[4] = SlotBox(142, 10, 0);  /* Champion Status Box 2 Ready Hand */
	_g30_slotBoxes[5] = SlotBox(162, 10, 0);  /* Champion Status Box 2 Action Hand */
	_g30_slotBoxes[6] = SlotBox(211, 10, 0);  /* Champion Status Box 3 Ready Hand */
	_g30_slotBoxes[7] = SlotBox(231, 10, 0);  /* Champion Status Box 3 Action Hand */
	_g30_slotBoxes[8] = SlotBox(6, 53, 0);    /* Ready Hand */
	_g30_slotBoxes[9] = SlotBox(62, 53, 0);   /* Action Hand */
	_g30_slotBoxes[10] = SlotBox(34, 26, 0);  /* Head */
	_g30_slotBoxes[11] = SlotBox(34, 46, 0);  /* Torso */
	_g30_slotBoxes[12] = SlotBox(34, 66, 0);  /* Legs */
	_g30_slotBoxes[13] = SlotBox(34, 86, 0);  /* Feet */
	_g30_slotBoxes[14] = SlotBox(6, 90, 0);   /* Pouch 2 */
	_g30_slotBoxes[15] = SlotBox(79, 73, 0);  /* Quiver Line2 1 */
	_g30_slotBoxes[16] = SlotBox(62, 90, 0);  /* Quiver Line1 2 */
	_g30_slotBoxes[17] = SlotBox(79, 90, 0);  /* Quiver Line2 2 */
	_g30_slotBoxes[18] = SlotBox(6, 33, 0);   /* Neck */
	_g30_slotBoxes[19] = SlotBox(6, 73, 0);   /* Pouch 1 */
	_g30_slotBoxes[20] = SlotBox(62, 73, 0);  /* Quiver Line1 1 */
	_g30_slotBoxes[21] = SlotBox(66, 33, 0);  /* Backpack Line1 1 */
	_g30_slotBoxes[22] = SlotBox(83, 16, 0);  /* Backpack Line2 2 */
	_g30_slotBoxes[23] = SlotBox(100, 16, 0); /* Backpack Line2 3 */
	_g30_slotBoxes[24] = SlotBox(117, 16, 0); /* Backpack Line2 4 */
	_g30_slotBoxes[25] = SlotBox(134, 16, 0); /* Backpack Line2 5 */
	_g30_slotBoxes[26] = SlotBox(151, 16, 0); /* Backpack Line2 6 */
	_g30_slotBoxes[27] = SlotBox(168, 16, 0); /* Backpack Line2 7 */
	_g30_slotBoxes[28] = SlotBox(185, 16, 0); /* Backpack Line2 8 */
	_g30_slotBoxes[29] = SlotBox(202, 16, 0); /* Backpack Line2 9 */
	_g30_slotBoxes[30] = SlotBox(83, 33, 0);  /* Backpack Line1 2 */
	_g30_slotBoxes[31] = SlotBox(100, 33, 0); /* Backpack Line1 3 */
	_g30_slotBoxes[32] = SlotBox(117, 33, 0); /* Backpack Line1 4 */
	_g30_slotBoxes[33] = SlotBox(134, 33, 0); /* Backpack Line1 5 */
	_g30_slotBoxes[34] = SlotBox(151, 33, 0); /* Backpack Line1 6 */
	_g30_slotBoxes[35] = SlotBox(168, 33, 0); /* Backpack Line1 7 */
	_g30_slotBoxes[36] = SlotBox(185, 33, 0); /* Backpack Line1 8 */
	_g30_slotBoxes[37] = SlotBox(202, 33, 0); /* Backpack Line1 9 */
	_g30_slotBoxes[38] = SlotBox(117, 59, 0); /* Chest 1 */
	_g30_slotBoxes[39] = SlotBox(106, 76, 0); /* Chest 2 */
	_g30_slotBoxes[40] = SlotBox(111, 93, 0); /* Chest 3 */
	_g30_slotBoxes[41] = SlotBox(128, 98, 0); /* Chest 4 */
	_g30_slotBoxes[42] = SlotBox(145, 101, 0); /* Chest 5 */
	_g30_slotBoxes[43] = SlotBox(162, 103, 0); /* Chest 6 */
	_g30_slotBoxes[44] = SlotBox(179, 104, 0); /* Chest 7 */
	_g30_slotBoxes[45] = SlotBox(196, 105, 0); /* Chest 8 */

	for (uint16 i = 0; i < k199_ObjectNameCount; ++i)
		_g352_objectNames[i] = nullptr;

	_g412_objectIconForMousePointer = nullptr;
}

ObjectMan::~ObjectMan() {
	delete[] _g412_objectIconForMousePointer;
	delete[] _g352_objectNames[0];
}

#define k556_ObjectNamesGraphicIndice 556 // @ C556_GRAPHIC_OBJECT_NAMES

void ObjectMan::loadObjectNames() {
	DisplayMan &dispMan = *_vm->_displayMan;

	_g412_objectIconForMousePointer = new byte[16 * 16];

	char *objectNames = new char[dispMan.getCompressedDataSize(k556_ObjectNamesGraphicIndice) + k199_ObjectNameCount];
	Common::MemoryReadStream stream = dispMan.getCompressedData(k556_ObjectNamesGraphicIndice);

	for (uint16 objNameIndex = 0; objNameIndex < k199_ObjectNameCount; ++objNameIndex) {
		_g352_objectNames[objNameIndex] = objectNames;

		byte tmpByte;
		for (tmpByte = stream.readByte(); !(tmpByte & 0x80); tmpByte = stream.readByte()) // last char of object name has 7th bit on
			*objectNames++ = tmpByte; // write while not last char

		*objectNames++ = tmpByte & 0x7F; // write without the 7th bit
		*objectNames++ = '\0'; // terminate string
	}
}

IconIndice ObjectMan::f32_getObjectType(Thing thing) {
	if (thing == Thing::_none)
		return kM1_IconIndiceNone;

	int16 objectInfoIndex = _vm->_dungeonMan->f141_getObjectInfoIndex(thing);
	if (objectInfoIndex != -1) {
		objectInfoIndex = g237_ObjectInfo[objectInfoIndex]._type;
	}
	return (IconIndice)objectInfoIndex;
}

byte g29_ChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}; // @ G0029_auc_Graphic562_ChargeCountToTorchType

IconIndice ObjectMan::f33_getIconIndex(Thing thing) {
	IconIndice iconIndex = f32_getObjectType(thing);

	if ((iconIndex != kM1_IconIndiceNone) &&
		(((iconIndex < k32_IconIndiceWeaponDagger) && (iconIndex >= k0_IconIndiceJunkCompassNorth)) || // < instead of <= is no error
		((iconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (iconIndex <= k163_IconIndicePotionWaterFlask)) ||
		 (iconIndex == k195_IconIndicePotionEmptyFlask))
		) {
		uint16 *rawType = _vm->_dungeonMan->f156_getThingData(thing);
		switch (iconIndex) {
		case k0_IconIndiceJunkCompassNorth:
			iconIndex = (IconIndice)(iconIndex + _vm->_dungeonMan->_g308_partyDir);
			break;
		case k4_IconIndiceWeaponTorchUnlit: {
			Weapon weapon(rawType);
			if (weapon.isLit()) {
				iconIndex = (IconIndice)(iconIndex + g29_ChargeCountToTorchType[weapon.getChargeCount()]);
			}
			break;
		}
		case k30_IconIndiceScrollOpen:
			if (Scroll(rawType).getClosed()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case k8_IconIndiceJunkWater:
		case k12_IconIndiceJunkIllumuletUnequipped:
		case k10_IconIndiceJunkJewelSymalUnequipped:
			if (Junk(rawType).getChargeCount()) {
				iconIndex = (IconIndice)(iconIndex + 1);
			}
			break;
		case k23_IconIndiceWeaponBoltBladeStormEmpty:
		case k14_IconIndiceWeaponFlamittEmpty:
		case k18_IconIndiceWeaponStormringEmpty:
		case k25_IconIndiceWeaponFuryRaBladeEmpty:
		case k16_IconIndiceWeaponEyeOfTimeEmpty:
		case k20_IconIndiceWeaponStaffOfClawsEmpty:
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

void ObjectMan::f36_extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	int16 i;
	for (i = 0; i < 7; ++i) {
		if (g26_IconGraphicFirstIndex[i] > iconIndex)
			break;
	}

	--i;
	byte *srcBitmap = _vm->_displayMan->f489_getNativeBitmapOrGraphic(k42_ObjectIcons_000_TO_031 + i);
	iconIndex -= g26_IconGraphicFirstIndex[i];
	_vm->_displayMan->_g578_useByteBoxCoordinates = true;
	Box box(0, 0, 15, 15);
	_vm->_displayMan->f132_blitToBitmap(srcBitmap, destBitmap, box, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, 128, 8, k255_ColorNoTransparency);
}

void ObjectMan::f38_drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex) {
	SlotBox *slotBox = &_g30_slotBoxes[slotBoxIndex];
	slotBox->_iconIndex = iconIndex; // yes, this modifies the global array
	if (slotBox->_iconIndex == kM1_IconIndiceNone) {
		return;
	}

	Box box;
	box._x1 = slotBox->_x;
	box._y1 = slotBox->_y;
	box._x2 = box._x1 + 15;
	box._y2 = box._y1 + 15;

	uint16 iconGraphicIndex;
	for (iconGraphicIndex = 0; iconGraphicIndex < 7; ++iconGraphicIndex) {
		if (g26_IconGraphicFirstIndex[iconGraphicIndex] > iconIndex) {
			break;
		}
	}
	iconGraphicIndex--;
	byte *iconsBitmap = _vm->_displayMan->f489_getNativeBitmapOrGraphic(iconGraphicIndex + k42_ObjectIcons_000_TO_031);
	iconIndex -= g26_IconGraphicFirstIndex[iconGraphicIndex];

	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	if (slotBoxIndex >= k8_SlotBoxInventoryFirstSlot) {
		_vm->_displayMan->f132_blitToBitmap(iconsBitmap, _vm->_displayMan->_g296_bitmapViewport, box, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, 128, k112_byteWidthViewport, k255_ColorNoTransparency);

	} else {
		_vm->_displayMan->f132_blitToBitmap(iconsBitmap, _vm->_displayMan->_g348_bitmapScreen, box, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, 128, k160_byteWidthScreen, k255_ColorNoTransparency);
	}
}

#define k14_ObjectNameMaximumLength 14 // @ C014_OBJECT_NAME_MAXIMUM_LENGTH

void ObjectMan::f34_drawLeaderObjectName(Thing thing) {
	IconIndice iconIndex = f33_getIconIndex(thing);
	char *objName;
	char objectNameBuffer[16];
	if (iconIndex == k147_IconIndiceJunkChampionBones) {
		Junk *junk = (Junk*)_vm->_dungeonMan->f156_getThingData(thing);
		strcpy(objectNameBuffer, _vm->_championMan->_gK71_champions[junk->getChargeCount()]._name);
		strcat(objectNameBuffer, _g352_objectNames[iconIndex]);
		objName = objectNameBuffer;
	} else {
		objName = _g352_objectNames[iconIndex];
	}
	_vm->_textMan->f41_printWithTrailingSpaces(_vm->_displayMan->_g348_bitmapScreen, k160_byteWidthScreen, 233, 37,
											   k4_ColorCyan, k0_ColorBlack, objName, k14_ObjectNameMaximumLength, k200_heightScreen);
}

IconIndice ObjectMan::f39_getIconIndexInSlotBox(uint16 slotBoxIndex) {
	return (IconIndice)_g30_slotBoxes[slotBoxIndex]._iconIndex;
}

void ObjectMan::f35_clearLeaderObjectName() {
	static Box g28_BoxLeaderHandObjectName(233, 319, 33, 38); // @ G0028_s_Graphic562_Box_LeaderHandObjectName 
	_vm->_displayMan->D24_fillScreenBox(g28_BoxLeaderHandObjectName, k0_ColorBlack);
}

}
