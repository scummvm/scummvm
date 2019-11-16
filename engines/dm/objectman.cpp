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

#include "dm/objectman.h"
#include "dm/dungeonman.h"
#include "dm/text.h"

namespace DM {

void ObjectMan::initConstants() {
	int16 iconGraphicHeight[7] = {32, 32, 32, 32, 32, 32, 32}; // @ K0077_ai_IconGraphicHeight
	int16 iconGraphicFirstIndex[7] = { // G0026_ai_Graphic562_IconGraphicFirstIconIndex
		0,     /* First icon index in graphic #42 */
		32,    /* First icon index in graphic #43 */
		64,    /* First icon index in graphic #44 */
		96,    /* First icon index in graphic #45 */
		128,   /* First icon index in graphic #46 */
		160,   /* First icon index in graphic #47 */
		192    /* First icon index in graphic #48 */
	};

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

	for (int i = 0; i < 7; i++) {
		_iconGraphicHeight[i] = iconGraphicHeight[i];
		_iconGraphicFirstIndex[i] = iconGraphicFirstIndex[i];
	}
}

ObjectMan::ObjectMan(DMEngine *vm) : _vm(vm) {
	for (uint16 i = 0; i < kDMObjectNameCount; ++i)
		_objectNames[i] = nullptr;

	_objectIconForMousePointer = nullptr;

	initConstants();
}

ObjectMan::~ObjectMan() {
	delete[] _objectIconForMousePointer;
	delete[] _objectNames[0];
}

void ObjectMan::loadObjectNames() {
	DisplayMan &dispMan = *_vm->_displayMan;

	_objectIconForMousePointer = new byte[16 * 16];

	char *objectNames = new char[dispMan.getCompressedDataSize(kDMObjectNamesGraphicIndice) + kDMObjectNameCount];
	Common::MemoryReadStream stream = dispMan.getCompressedData(kDMObjectNamesGraphicIndice);

	for (uint16 objNameIndex = 0; objNameIndex < kDMObjectNameCount; ++objNameIndex) {
		_objectNames[objNameIndex] = objectNames;

		byte tmpByte;
		for (tmpByte = stream.readByte(); !(tmpByte & 0x80); tmpByte = stream.readByte()) // last char of object name has 7th bit on
			*objectNames++ = tmpByte; // write while not last char

		*objectNames++ = tmpByte & 0x7F; // write without the 7th bit
		*objectNames++ = '\0'; // terminate string
	}
}

IconIndice ObjectMan::getObjectType(Thing thing) {
	if (thing == _vm->_thingNone)
		return kDMIconIndiceNone;

	int16 objectInfoIndex = _vm->_dungeonMan->getObjectInfoIndex(thing);
	if (objectInfoIndex != -1)
		objectInfoIndex = _vm->_dungeonMan->_objectInfos[objectInfoIndex]._type;

	return (IconIndice)objectInfoIndex;
}

IconIndice ObjectMan::getIconIndex(Thing thing) {
	static byte chargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}; // @ G0029_auc_Graphic562_ChargeCountToTorchType

	int16 iconIndex = getObjectType(thing);
	if (iconIndex != kDMIconIndiceNone) {
		if (((iconIndex < kDMIconIndiceWeaponDagger) && (iconIndex >= kDMIconIndiceJunkCompassNorth)) ||
			((iconIndex >= kDMIconIndicePotionMaPotionMonPotion) && (iconIndex <= kDMIconIndicePotionWaterFlask)) ||
			(iconIndex == kDMIconIndicePotionEmptyFlask)) {
			Junk *junkThing = (Junk*)_vm->_dungeonMan->getThingData(thing);
			switch (iconIndex) {
			case kDMIconIndiceJunkCompassNorth:
				iconIndex += _vm->_dungeonMan->_partyDir;
				break;
			case kDMIconIndiceWeaponTorchUnlit:
				if (((Weapon*)junkThing)->isLit())
					iconIndex += chargeCountToTorchType[((Weapon*)junkThing)->getChargeCount()];
				break;
			case kDMIconIndiceScrollOpen:
				if (((Scroll*)junkThing)->getClosed())
					iconIndex++;
				break;
			case kDMIconIndiceJunkWater:
			case kDMIconIndiceJunkIllumuletUnequipped:
			case kDMIconIndiceJunkJewelSymalUnequipped:
				if (junkThing->getChargeCount())
					iconIndex++;
				break;
			case kDMIconIndiceWeaponBoltBladeStormEmpty:
			case kDMIconIndiceWeaponFlamittEmpty:
			case kDMIconIndiceWeaponStormringEmpty:
			case kDMIconIndiceWeaponFuryRaBladeEmpty:
			case kDMIconIndiceWeaponEyeOfTimeEmpty:
			case kDMIconIndiceWeaponStaffOfClawsEmpty:
				if (((Weapon*)junkThing)->getChargeCount())
					iconIndex++;
				break;
			default:
				break;
			}
		}
	}
	return (IconIndice)iconIndex;
}

void ObjectMan::extractIconFromBitmap(uint16 iconIndex, byte *destBitmap) {
	uint16 counter;
	for (counter = 0; counter < 7; counter++) {
		if (_iconGraphicFirstIndex[counter] > iconIndex)
			break;
	}
	--counter;
	byte *iconBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxObjectIcons000To031 + counter);
	iconIndex -= _iconGraphicFirstIndex[counter];
	_vm->_displayMan->_useByteBoxCoordinates = true;
	Box blitBox(0, 15, 0, 15);
	_vm->_displayMan->blitToBitmap(iconBitmap, destBitmap, blitBox, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, 128, 8, kDMColorNoTransparency, _iconGraphicHeight[counter], 16);
}

void ObjectMan::drawIconInSlotBox(uint16 slotBoxIndex, int16 iconIndex) {
	SlotBox *slotBox = &_slotBoxes[slotBoxIndex];
	slotBox->_iconIndex = iconIndex;
	if (slotBox->_iconIndex == kDMIconIndiceNone)
		return;

	Box blitBox;
	blitBox._rect.left = slotBox->_x;
	blitBox._rect.right = blitBox._rect.left + 15;
	blitBox._rect.top = slotBox->_y;
	blitBox._rect.bottom = blitBox._rect.top + 15;

	uint16 iconGraphicIndex;
	for (iconGraphicIndex = 0; iconGraphicIndex < 7; iconGraphicIndex++) {
		if (_iconGraphicFirstIndex[iconGraphicIndex] > iconIndex)
			break;
	}
	iconGraphicIndex--;
	byte *iconBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(iconGraphicIndex + kDMGraphicIdxObjectIcons000To031);
	iconIndex -= _iconGraphicFirstIndex[iconGraphicIndex];
	int16 byteWidth;
	byte* blitDestination;
	int16 destHeight;
	if (slotBoxIndex >= kDMSlotBoxInventoryFirstSlot) {
		blitDestination = _vm->_displayMan->_bitmapViewport;
		byteWidth = k112_byteWidthViewport;
		destHeight = 136;
	} else {
		blitDestination = (unsigned char*)_vm->_displayMan->_bitmapScreen;
		byteWidth = k160_byteWidthScreen;
		destHeight = 200;
	}
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->blitToBitmap(iconBitmap, blitDestination, blitBox, (iconIndex & 0x000F) << 4, iconIndex & 0x0FF0, k128_byteWidth, byteWidth, kDMColorNoTransparency, _iconGraphicHeight[iconGraphicIndex], destHeight);
}

void ObjectMan::drawLeaderObjectName(Thing thing) {
	Common::String objectName;
	int16 iconIndex = getIconIndex(thing);
	if (iconIndex == kDMIconIndiceJunkChampionBones) {
		Junk *junk = (Junk*)_vm->_dungeonMan->getThingData(thing);
		Common::String champBonesName;

		switch (_vm->getGameLanguage()) { // localized
		case Common::FR_FRA:
			// Fix original bug: strcpy was coming after strcat
			champBonesName = Common::String(_objectNames[iconIndex]);
			champBonesName += Common::String(_vm->_championMan->_champions[junk->getChargeCount()]._name);
			break;
		default: // English and German version are the same
			champBonesName = Common::String(_vm->_championMan->_champions[junk->getChargeCount()]._name);
			champBonesName += Common::String(_objectNames[iconIndex]);
			break;
		}

		objectName = champBonesName;
	} else
		objectName = Common::String(_objectNames[iconIndex]);

	_vm->_textMan->printWithTrailingSpaces(_vm->_displayMan->_bitmapScreen, k160_byteWidthScreen, 233, 37, kDMColorCyan, kDMColorBlack, objectName.c_str(), kDMObjectNameMaximumLength, k200_heightScreen);
}

IconIndice ObjectMan::getIconIndexInSlotBox(uint16 slotBoxIndex) {
	return (IconIndice)_slotBoxes[slotBoxIndex]._iconIndex;
}

void ObjectMan::clearLeaderObjectName() {
	static Box boxLeaderHandObjectName(233, 319, 33, 38); // @ G0028_s_Graphic562_Box_LeaderHandObjectName
	_vm->_displayMan->fillScreenBox(boxLeaderHandObjectName, kDMColorBlack);
}

void ObjectMan::drawIconToScreen(int16 iconIndex, int16 posX, int16 posY) {
	static byte iconBitmap[16 * 16];
	Box blitBox(posX, posX + 15, posY, posY + 15);
	extractIconFromBitmap(iconIndex, iconBitmap);
	_vm->_displayMan->blitToScreen(iconBitmap, &blitBox, k8_byteWidth, kDMColorNoTransparency, 16);
}
}
