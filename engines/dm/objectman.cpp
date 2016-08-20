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
		objectInfoIndex = _vm->_dungeonMan->_objectInfo[objectInfoIndex]._type;
	}
	return (IconIndice)objectInfoIndex;
}

byte g29_ChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}; // @ G0029_auc_Graphic562_ChargeCountToTorchType

IconIndice ObjectMan::f33_getIconIndex(Thing thing) {
	int16 L0005_i_IconIndex = f32_getObjectType(thing);
	if (L0005_i_IconIndex != kM1_IconIndiceNone) {
		if (((L0005_i_IconIndex < k32_IconIndiceWeaponDagger) && (L0005_i_IconIndex >= k0_IconIndiceJunkCompassNorth)) ||
			((L0005_i_IconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (L0005_i_IconIndex <= k163_IconIndicePotionWaterFlask)) ||
			(L0005_i_IconIndex == k195_IconIndicePotionEmptyFlask)) {
			Junk *L0006_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(thing);
			switch (L0005_i_IconIndex) {
			case k0_IconIndiceJunkCompassNorth:
				L0005_i_IconIndex += _vm->_dungeonMan->_g308_partyDir;
				break;
			case k4_IconIndiceWeaponTorchUnlit:
				if (((Weapon*)L0006_ps_Junk)->isLit()) {
					L0005_i_IconIndex += g29_ChargeCountToTorchType[((Weapon*)L0006_ps_Junk)->getChargeCount()];
				}
				break;
			case k30_IconIndiceScrollOpen:
				if (((Scroll*)L0006_ps_Junk)->getClosed()) {
					L0005_i_IconIndex++;
				}
				break;
			case k8_IconIndiceJunkWater:
			case k12_IconIndiceJunkIllumuletUnequipped:
			case k10_IconIndiceJunkJewelSymalUnequipped:
				if (L0006_ps_Junk->getChargeCount()) {
					L0005_i_IconIndex++;
				}
				break;
			case k23_IconIndiceWeaponBoltBladeStormEmpty:
			case k14_IconIndiceWeaponFlamittEmpty:
			case k18_IconIndiceWeaponStormringEmpty:
			case k25_IconIndiceWeaponFuryRaBladeEmpty:
			case k16_IconIndiceWeaponEyeOfTimeEmpty:
			case k20_IconIndiceWeaponStaffOfClawsEmpty:
				if (((Weapon*)L0006_ps_Junk)->getChargeCount()) {
					L0005_i_IconIndex++;
				}
			}
		}
	}
	return (IconIndice)L0005_i_IconIndex;
}

void ObjectMan::f36_extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	uint16 L0011_ui_Counter;
	byte* L0012_pl_Bitmap_Icon;
	Box L1568_s_Box;

	for (L0011_ui_Counter = 0; L0011_ui_Counter < 7; L0011_ui_Counter++) {
		if (g26_IconGraphicFirstIndex[L0011_ui_Counter] > iconIndex)
			break;
	}
	L0012_pl_Bitmap_Icon = _vm->_displayMan->f489_getNativeBitmapOrGraphic(k42_ObjectIcons_000_TO_031 + --L0011_ui_Counter);
	iconIndex -= g26_IconGraphicFirstIndex[L0011_ui_Counter];
	_vm->_displayMan->_g578_useByteBoxCoordinates = true;
	L1568_s_Box._y1 = 0;
	L1568_s_Box._x1 = 0;
	L1568_s_Box._y2 = 15;
	L1568_s_Box._x2 = 15;
	_vm->_displayMan->f132_blitToBitmap(L0012_pl_Bitmap_Icon, destBitmap, L1568_s_Box, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, 128, 8, kM1_ColorNoTransparency, gK77_IconGraphicHeight[L0011_ui_Counter], 16);
}

void ObjectMan::f38_drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex) {
	uint16 L0015_ui_IconGraphicIndex;
	int16 L0016_i_ByteWidth;
	SlotBox* L0017_ps_SlotBox;
	byte* L0018_puc_Bitmap_Icons;
	Box L0019_s_Box;
	byte* L0020_puc_Bitmap_Destination;
	int16 L1569_i_Width;

	L0017_ps_SlotBox = &_g30_slotBoxes[slotBoxIndex];
	if ((L0017_ps_SlotBox->_iconIndex = iconIndex) == kM1_IconIndiceNone) {
		return;
	}
	L0019_s_Box._x2 = (L0019_s_Box._x1 = L0017_ps_SlotBox->_x) + 15;
	L0019_s_Box._y2 = (L0019_s_Box._y1 = L0017_ps_SlotBox->_y) + 15;
	for (L0015_ui_IconGraphicIndex = 0; L0015_ui_IconGraphicIndex < 7; L0015_ui_IconGraphicIndex++) {
		if (g26_IconGraphicFirstIndex[L0015_ui_IconGraphicIndex] > iconIndex)
			break;
	}
	L0015_ui_IconGraphicIndex--;
	L0018_puc_Bitmap_Icons = _vm->_displayMan->f489_getNativeBitmapOrGraphic(L0015_ui_IconGraphicIndex + k42_ObjectIcons_000_TO_031);
	iconIndex -= g26_IconGraphicFirstIndex[L0015_ui_IconGraphicIndex];
	if (slotBoxIndex >= k8_SlotBoxInventoryFirstSlot) {
		L0020_puc_Bitmap_Destination = _vm->_displayMan->_g296_bitmapViewport;
		L0016_i_ByteWidth = k112_byteWidthViewport;
		L1569_i_Width = 136;
	} else {
		L0020_puc_Bitmap_Destination = (unsigned char*)_vm->_displayMan->_g348_bitmapScreen;
		L0016_i_ByteWidth = k160_byteWidthScreen;
		L1569_i_Width = 200;
	}
	_vm->_displayMan->_g578_useByteBoxCoordinates = false, _vm->_displayMan->f132_blitToBitmap(L0018_puc_Bitmap_Icons, L0020_puc_Bitmap_Destination, L0019_s_Box, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, k128_byteWidth, L0016_i_ByteWidth, kM1_ColorNoTransparency, gK77_IconGraphicHeight[L0015_ui_IconGraphicIndex], L1569_i_Width);
}

#define k14_ObjectNameMaximumLength 14 // @ C014_OBJECT_NAME_MAXIMUM_LENGTH

void ObjectMan::f34_drawLeaderObjectName(Thing thing) {
	char* objectName = nullptr;
	int16 L0007_i_IconIndex = f33_getIconIndex(thing);
	if (L0007_i_IconIndex == k147_IconIndiceJunkChampionBones) {
		Junk *junk = (Junk*)_vm->_dungeonMan->f156_getThingData(thing);
		char champBonesName[16];

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
		case Common::DE_DEU: // english and german version are the same
			strcpy(champBonesName, _vm->_championMan->_gK71_champions[junk->getChargeCount()]._name);
			strcat(champBonesName, _g352_objectNames[L0007_i_IconIndex]);
			break;
		case Common::FR_FRA:
			strcat(champBonesName, _g352_objectNames[L0007_i_IconIndex]);
			strcpy(champBonesName, _vm->_championMan->_gK71_champions[junk->getChargeCount()]._name);
			break;
		}

		objectName = champBonesName;
	} else {
		objectName = _g352_objectNames[L0007_i_IconIndex];
	}
	_vm->_textMan->f41_printWithTrailingSpaces(_vm->_displayMan->_g348_bitmapScreen, k160_byteWidthScreen, 233, 37, k4_ColorCyan, k0_ColorBlack, objectName, k14_ObjectNameMaximumLength, k200_heightScreen);
}

IconIndice ObjectMan::f39_getIconIndexInSlotBox(uint16 slotBoxIndex) {
	return (IconIndice)_g30_slotBoxes[slotBoxIndex]._iconIndex;
}

void ObjectMan::f35_clearLeaderObjectName() {
	static Box g28_BoxLeaderHandObjectName(233, 319, 33, 38); // @ G0028_s_Graphic562_Box_LeaderHandObjectName 
	_vm->_displayMan->D24_fillScreenBox(g28_BoxLeaderHandObjectName, k0_ColorBlack);
}

void ObjectMan::f37_drawIconToScreen(int16 iconIndex, int16 posX, int16 posY) {
	static byte L0013_puc_Bitmap_Icon[16 * 16];
	Box L0014_s_Box;

	L0014_s_Box._x2 = (L0014_s_Box._x1 = posX) + 15;
	L0014_s_Box._y2 = (L0014_s_Box._y1 = posY) + 15;
	f36_extractIconFromBitmap(iconIndex, L0013_puc_Bitmap_Icon);
	_vm->_displayMan->f21_blitToScreen(L0013_puc_Bitmap_Icon, &L0014_s_Box, k8_byteWidth, kM1_ColorNoTransparency, 16);
}
}
