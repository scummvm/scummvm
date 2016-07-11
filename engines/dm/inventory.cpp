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

#include "inventory.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "gfx.h"
#include "text.h"
#include "objectman.h"


namespace DM {

Box g41_BoxFloppyZzzCross = Box(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross
Box g32_BoxPanel = Box(80, 223, 52, 124); // @ G0032_s_Graphic562_Box_Panel
Box g35_BoxFood = Box(112, 159, 60, 68); // @ G0035_s_Graphic562_Box_Food
Box g36_BoxWater = Box(112, 159, 83, 91); // @ G0036_s_Graphic562_Box_Water
Box g37_BoxPoisoned = Box(112, 207, 105, 119); // @ G0037_s_Graphic562_Box_Poisoned

InventoryMan::InventoryMan(DMEngine *vm) : _vm(vm) {
	_g432_inventoryChampionOrdinal = 0;
	_g424_panelContent = k0_PanelContentFoodWaterPoisoned;
	for (uint16 i = 0; i < 8; ++i)
		_g425_chestSlots[i] = Thing(0);
	_g426_openChest = Thing::_none;
	_g421_objDescTextXpos = 0;
	_g422_objDescTextYpos = 0;
}

void InventoryMan::f355_toggleInventory(ChampionIndex championIndex) {
	uint16 L1102_ui_Multiple;
#define AL1102_ui_InventoryChampionOrdinal L1102_ui_Multiple
#define AL1102_ui_SlotIndex                L1102_ui_Multiple
	Champion* L1103_ps_Champion;


	if ((championIndex != k4_ChampionCloseInventory) && !_vm->_championMan->_gK71_champions[championIndex]._currHealth) {
		return;
	}
	if (_vm->_g333_pressingMouth || _vm->_g331_pressingEye) {
		return;
	}
	_vm->_g321_stopWaitingForPlayerInput = true;
	AL1102_ui_InventoryChampionOrdinal = _vm->_inventoryMan->_g432_inventoryChampionOrdinal;
	if (_vm->M0_indexToOrdinal(championIndex) == AL1102_ui_InventoryChampionOrdinal) {
		championIndex = k4_ChampionCloseInventory;
	}
	_vm->_eventMan->f78_showMouse();
	if (AL1102_ui_InventoryChampionOrdinal) {
		_vm->_inventoryMan->_g432_inventoryChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		_vm->_inventoryMan->f334_closeChest();
		L1103_ps_Champion = &_vm->_championMan->_gK71_champions[_vm->M1_ordinalToIndex(AL1102_ui_InventoryChampionOrdinal)];
		if (L1103_ps_Champion->_currHealth && !_vm->_championMan->_g299_candidateChampionOrdinal) {
			setFlag(L1103_ps_Champion->_attributes, k0x1000_ChampionAttributeStatusBox);
			_vm->_championMan->f292_drawChampionState((ChampionIndex)_vm->M1_ordinalToIndex(AL1102_ui_InventoryChampionOrdinal));
		}
		if (_vm->_championMan->_g300_partyIsSleeping) {
			_vm->_eventMan->f77_hideMouse();
			return;
		}
		if (championIndex == k4_ChampionCloseInventory) {
			_vm->_eventMan->_g326_refreshMousePointerInMainLoop = true;
			_vm->_menuMan->f395_drawMovementArrows();
			_vm->_eventMan->f77_hideMouse();
			_vm->_eventMan->_g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
			_vm->_eventMan->_g444_secondaryKeyboardInput = g459_secondaryKeyboardInput_movement;
			_vm->_eventMan->f357_discardAllInput();
			_vm->_displayMan->f98_drawFloorAndCeiling();
			return;
		}
	}
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_inventoryMan->_g432_inventoryChampionOrdinal = _vm->M0_indexToOrdinal(championIndex);
	if (!AL1102_ui_InventoryChampionOrdinal) {
		warning(false, "MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}
	L1103_ps_Champion = &_vm->_championMan->_gK71_champions[championIndex];
	_vm->_displayMan->f466_loadIntoBitmap(k17_InventoryGraphicIndice, _vm->_displayMan->_g296_bitmapViewport);
	if (_vm->_championMan->_g299_candidateChampionOrdinal) {
		_vm->_displayMan->f135_fillBoxBitmap(_vm->_displayMan->_g296_bitmapViewport, g41_BoxFloppyZzzCross, k12_ColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);
	}
	_vm->_textMan->f52_printToViewport(5, 116, k13_ColorLightestGray, "HEALTH");
	_vm->_textMan->f52_printToViewport(5, 124, k13_ColorLightestGray, "STAMINA");
	_vm->_textMan->f52_printToViewport(5, 132, k13_ColorLightestGray, "MANA");
	for (AL1102_ui_SlotIndex = k0_ChampionSlotReadyHand; AL1102_ui_SlotIndex < k30_ChampionSlotChest_1; AL1102_ui_SlotIndex++) {
		_vm->_championMan->f291_drawSlot(championIndex, AL1102_ui_SlotIndex);
	}
	setFlag(L1103_ps_Champion->_attributes, k0x4000_ChampionAttributeViewport | k0x1000_ChampionAttributeStatusBox | k0x0800_ChampionAttributePanel | k0x0200_ChampionAttributeLoad | k0x0100_ChampionAttributeStatistics | k0x0080_ChampionAttributeNameTitle);
	_vm->_championMan->f292_drawChampionState(championIndex);
	_vm->_eventMan->_g598_mousePointerBitmapUpdated = true;
	_vm->_eventMan->f77_hideMouse();
	_vm->_eventMan->_g442_secondaryMouseInput = g449_SecondaryMouseInput_ChampionInventory;
	_vm->_eventMan->_g444_secondaryKeyboardInput = nullptr;
	_vm->_eventMan->f357_discardAllInput();
}

void InventoryMan::f354_drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan._g578_useByteBoxCoordinates = false;
	Box box;
	box._y1 = 0;
	box._y2 = 28;
	box._x1 = championIndex * k69_ChampionStatusBoxSpacing + 7;
	box._x2 = box._x1 + 31;
	dispMan.f132_blitToBitmap(_vm->_championMan->_gK71_champions[championIndex]._portrait, dispMan._g348_bitmapScreen, box, 0, 0, 16, k160_byteWidthScreen, kM1_ColorNoTransparency);
}

void InventoryMan::f343_drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	Box box;
	box._x1 = x;
	box._x2 = box._x1 + pixelWidth;
	box._y1 = y;
	box._y2 = box._y1 + 6;
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_displayMan->D24_fillScreenBox(box, color);
}

void InventoryMan::f344_drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
	if (amount < -512) {
		color = k8_ColorRed;
	} else if (amount < 0) {
		color = k11_ColorYellow;
	}

	int16 pixelWidth = amount + 1024;
	if (pixelWidth == 3072) {
		pixelWidth = 3071;
	}
	pixelWidth /= 32;
	f343_drawPanelHorizontalBar(115, y + 2, pixelWidth, k0_ColorBlack);
	f343_drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::f345_drawPanelFoodWaterPoisoned() {
	Champion &champ = _vm->_championMan->_gK71_champions[_g432_inventoryChampionOrdinal];
	f334_closeChest();
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k20_PanelEmptyIndice), dispMan._g348_bitmapScreen, g32_BoxPanel, 0, 0, 72, k160_byteWidthScreen, k8_ColorRed);
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k30_FoodLabelIndice), dispMan._g348_bitmapScreen, g35_BoxFood, 0, 0, 24, k160_byteWidthScreen, k12_ColorDarkestGray);
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k31_WaterLabelIndice), dispMan._g348_bitmapScreen, g36_BoxWater, 0, 0, 24, k160_byteWidthScreen, k12_ColorDarkestGray);
	if (champ._poisonEventCount) {
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k32_PoisionedLabelIndice), dispMan._g348_bitmapScreen, g37_BoxPoisoned, 0, 0, 48, k160_byteWidthScreen, k12_ColorDarkestGray);
	}
	f344_drawPanelFoodOrWaterBar(champ._food, 69, k5_ColorLightBrown);
	f344_drawPanelFoodOrWaterBar(champ._water, 92, k14_ColorBlue);
}

void InventoryMan::f346_drawPanelResurrectReincarnate() {
	_g424_panelContent = k5_PanelContentResurrectReincarnate;
	_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k40_PanelResurectReincaranteIndice),
										_vm->_displayMan->_g296_bitmapViewport, g32_BoxPanel, 0, 0, 72, k112_byteWidthViewport, k6_ColorDarkGreen);
}

void InventoryMan::f347_drawPanel() {
	warning(false, "possible reintroduction of BUG0_48");
	f334_closeChest(); // possibility of BUG0_48

	ChampionMan &cm = *_vm->_championMan;
	if (cm._g299_candidateChampionOrdinal) {
		f346_drawPanelResurrectReincarnate();
		return;
	}

	Thing thing = cm._gK71_champions[_vm->M1_ordinalToIndex(_g432_inventoryChampionOrdinal)].getSlot(k1_ChampionSlotActionHand);

	_g424_panelContent = k0_PanelContentFoodWaterPoisoned;
	switch (thing.getType()) {
	case k9_ContainerThingType:
		_g424_panelContent = k4_PanelContentChest;
		break;
	case k7_ScrollThingType:
		_g424_panelContent = k2_PanelContentScroll;
		break;
	default:
		thing = Thing::_none;
		break;
	}
	if (thing == Thing::_none) {
		f345_drawPanelFoodWaterPoisoned();
	} else {
		f342_drawPanelObject(thing, false);
	}
}

void InventoryMan::f334_closeChest() {
	DungeonMan &dunMan = *_vm->_dungeonMan;

	bool processFirstChestSlot = true;
	if (_g426_openChest == Thing::_none)
		return;
	Container *container = (Container*)dunMan.f156_getThingData(_g426_openChest);
	_g426_openChest = Thing::_none;
	container->getSlot() = Thing::_endOfList;
	Thing prevThing;
	for (int16 chestSlotIndex = 0; chestSlotIndex < 8; ++chestSlotIndex) {
		Thing thing = _g425_chestSlots[chestSlotIndex];
		if (thing != Thing::_none) {
			_g425_chestSlots[chestSlotIndex] = Thing::_none; // CHANGE8_09_FIX

			if (processFirstChestSlot) {
				processFirstChestSlot = false;
				*dunMan.f156_getThingData(thing) = Thing::_endOfList.toUint16();
				container->getSlot() = prevThing = thing;
			} else {
				dunMan.f163_linkThingToList(thing, prevThing, kM1_MapXNotOnASquare, 0);
				prevThing = thing;
			}
		}
	}
}

void InventoryMan::f340_drawPanelScrollTextLine(int16 yPos, char* text) {
	warning(false, "CHANGE5_03_IMPROVEMENT");
	for (char* iter = text; *iter != '\0'; ++iter) {
		if ((*iter >= 'A') && (*iter <= 'Z')) {
			*iter -= 64;
		} else if (*iter >= '{') { // this branch is CHANGE5_03_IMPROVEMENT
			*iter -= 96;
		}
	}
	_vm->_textMan->f52_printToViewport(162 - (6 * strlen(text) / 2), yPos, k0_ColorBlack, text, k15_ColorWhite);
}

void InventoryMan::f341_drawPanelScroll(Scroll* scroll) {
	DisplayMan &dispMan = *_vm->_displayMan;

	char stringFirstLine[300];
	_vm->_dungeonMan->f168_decodeText(stringFirstLine, Thing(scroll->getTextStringThingIndex()), (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));
	char *charRed = stringFirstLine;
	while (*charRed && (*charRed != '\n')) {
		charRed++;
	}
	*charRed = '\0';
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k23_PanelOpenScrollIndice), dispMan._g296_bitmapViewport, g32_BoxPanel, 0, 0, 72, k112_byteWidthViewport, k8_ColorRed);
	int16 lineCount = 1;
	charRed++;
	char *charGreen = charRed; // first char of the second line
	while (*charGreen) {
		warning(false, "BUG0_47");
		/* BUG0_47 Graphical glitch when you open a scroll. If there is a single line of text in a scroll
		(with no carriage return) then charGreen points to undefined data. This may result in a graphical
		glitch and also corrupt other memory. This is not an issue in the original dungeons where all
		scrolls contain at least one carriage return character */
		if (*charGreen == '\n') {
			lineCount++;
		}
		charGreen++;
	}
	if (*(charGreen - 1) != '\n') {
		lineCount++;
	} else if (*(charGreen - 2) == '\n') {
		lineCount--;
	}
	int16 yPos = 92 - (7 * lineCount) / 2; // center the text vertically
	f340_drawPanelScrollTextLine(yPos, stringFirstLine);
	charGreen = charRed;
	while (*charGreen) {
		yPos += 7;
		while (*charRed && (*charRed != '\n')) {
			charRed++;
		}
		if (!(*charRed)) {
			charRed[1] = '\0';
		}
		*charRed++ = '\0';
		f340_drawPanelScrollTextLine(yPos, charGreen);
		charGreen = charRed;
	}
}

void InventoryMan::f333_openAndDrawChest(Thing thingToOpen, Container* chest, bool isPressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	ObjectMan &objMan = *_vm->_objectMan;

	if (_g426_openChest == thingToOpen)
		return;

	warning(false, "CHANGE8_09_FIX");
	if (_g426_openChest != Thing::_none)
		f334_closeChest(); // CHANGE8_09_FIX

	_g426_openChest = thingToOpen;
	if (!isPressingEye) {
		objMan.f38_drawIconInSlotBox(k9_SlotBoxInventoryActionHand, k145_IconIndiceContainerChestOpen);
	}
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k25_PanelOpenChestIndice), dispMan._g348_bitmapScreen, g32_BoxPanel, 0, 0, 72, k160_byteWidthScreen, k8_ColorRed);

	int16 chestSlotIndex = 0;
	Thing thing = chest->getSlot();
	int16 thingCount = 0;
	while (thing != Thing::_endOfList) {
		warning(false, "CHANGE8_08_FIX");
		if (++thingCount > 8)
			break; // CHANGE8_08_FIX, make sure that no more than the first 8 objects in a chest are drawn

		objMan.f38_drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, objMan.f33_getIconIndex(thing));
		_g425_chestSlots[chestSlotIndex++] = thing;
		thing = _vm->_dungeonMan->f159_getNextThing(thing);
	}
	while (chestSlotIndex < 8) {
		objMan.f38_drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, kM1_IconIndiceNone);
		_g425_chestSlots[chestSlotIndex++] = Thing::_none;
	}
}

void InventoryMan::f332_drawIconToViewport(IconIndice iconIndex, int16 xPos, int16 yPos) {
	static byte iconBitmap[16 * 16];
	Box box;
	box._x2 = (box._x1 = xPos) + 15;
	box._y2 = (box._y1 = yPos) + 15;
	_vm->_objectMan->f36_extractIconFromBitmap(iconIndex, iconBitmap);
	_vm->_displayMan->f132_blitToBitmap(iconBitmap, _vm->_displayMan->_g296_bitmapViewport, box, 0, 0, 8, k112_byteWidthViewport, kM1_ColorNoTransparency);
}

void InventoryMan::f336_buildObjectAttributeString(int16 potentialAttribMask, int16 actualAttribMask, char** attribStrings, char* destString, char* prefixString, char* suffixString) {
	uint16 identicalBitCount = 0;
	int16 attribMask = 1;
	for (uint16 stringIndex = 0; stringIndex < 16; stringIndex++, attribMask <<= 1) {
		if (attribMask & potentialAttribMask & actualAttribMask) {
			identicalBitCount++;
		}
	}

	if (identicalBitCount == 0) {
		*destString = '\0';
		return;
	}

	strcpy(destString, prefixString);

	attribMask = 1;
	for (uint16 stringIndex = 0; stringIndex < 16; stringIndex++, attribMask <<= 1) {
		if (attribMask & potentialAttribMask & actualAttribMask) {
			strcat(destString, attribStrings[stringIndex]);
			if (identicalBitCount-- > 2) {
				strcat(destString, ", ");
			} else if (identicalBitCount == 1) {
				strcat(destString, " AND "); // TODO: localization
			}
		}
	}

	strcat(destString, suffixString);
}

void InventoryMan::f335_drawPanelObjectDescriptionString(char* descString) {
	if (descString[0] == '\f') { // form feed
		descString++;
		_g421_objDescTextXpos = 108;
		_g422_objDescTextYpos = 59;
	}

	if (descString[0]) {
		char stringTmpBuff[128];
		strcpy(stringTmpBuff, descString);

		char *stringLine = stringTmpBuff;
		bool severalLines = false;
		char *string = nullptr;
		while (*stringLine) {
			if (strlen(stringLine) > 18) { // if string is too long to fit on one line
				string = &stringLine[17];
				while (*string != ' ') // go back to the last space character
					string--;

				*string = '\0'; // and split the string there
				severalLines = true;
			}

			_vm->_textMan->f52_printToViewport(_g421_objDescTextXpos, _g422_objDescTextYpos, k13_ColorLightestGray, stringLine);
			_g422_objDescTextYpos += 7;
			if (severalLines) {
				severalLines = false;
				stringLine = ++string;
			} else {
				*stringLine = '\0';
			}
		}
	}
}

Box g33_BoxArrowOrEye = Box(83, 98, 57, 65); // @ G0033_s_Graphic562_Box_ArrowOrEye 

void InventoryMan::f339_drawPanelArrowOrEye(bool pressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(pressingEye ? k19_EyeForObjectDescriptionIndice : k18_ArrowForChestContentIndice),
							  dispMan._g296_bitmapViewport, g33_BoxArrowOrEye, 0, 0, 8, k112_byteWidthViewport, k8_ColorRed);
}


Box g34_BoxObjectDescCircle = Box(105, 136, 53, 79); // @ G0034_s_Graphic562_Box_ObjectDescriptionCircle 

#define k0x0001_DescriptionMaskConsumable 0x0001 // @ MASK0x0001_DESCRIPTION_CONSUMABLE
#define k0x0002_DescriptionMaskPoisoned 0x0002 // @ MASK0x0002_DESCRIPTION_POISONED  
#define k0x0004_DescriptionMaskBroken 0x0004 // @ MASK0x0004_DESCRIPTION_BROKEN    
#define k0x0008_DescriptionMaskCursed 0x0008 // @ MASK0x0008_DESCRIPTION_CURSED    

void InventoryMan::f342_drawPanelObject(Thing thingToDraw, bool pressingEye) {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	if (_vm->_g331_pressingEye || _vm->_g333_pressingMouth) {
		warning(false, "BUG0_48 The contents of a chest are reorganized when an object with a statistic modifier is placed or removed on a champion");
		f334_closeChest();
	}

	uint16 *rawThingPtr = dunMan.f156_getThingData(thingToDraw);
	f335_drawPanelObjectDescriptionString("\f"); // form feed
	ThingType thingType = thingToDraw.getType();
	if (thingType == k7_ScrollThingType) {
		f341_drawPanelScroll((Scroll*)rawThingPtr);
	} else if (thingType == k9_ContainerThingType) {
		f333_openAndDrawChest(thingToDraw, (Container*)rawThingPtr, pressingEye);
	} else {
		IconIndice iconIndex = objMan.f33_getIconIndex(thingToDraw);
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k20_PanelEmptyIndice), dispMan._g296_bitmapViewport,
								  g32_BoxPanel, 0, 0, 72, k112_byteWidthViewport, k8_ColorRed);
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k29_ObjectDescCircleIndice), dispMan._g296_bitmapViewport,
								  g34_BoxObjectDescCircle, 0, 0, 16, k112_byteWidthViewport, k12_ColorDarkestGray);

		char *descString = nullptr;
		char str[40];
		if (iconIndex == k147_IconIndiceJunkChampionBones) {
			strcpy(str, champMan._gK71_champions[((Junk*)rawThingPtr)->getChargeCount()]._name);  // TODO: localization
			strcat(str, " "); // TODO: localization
			strcat(str, objMan._g352_objectNames[iconIndex]);  // TODO: localization

			descString = str;
		} else if ((thingType == k8_PotionThingType)
				   && (iconIndex != k163_IconIndicePotionWaterFlask)
				   && (champMan.f303_getSkillLevel((ChampionIndex)_vm->M1_ordinalToIndex(_g432_inventoryChampionOrdinal), k2_ChampionSkillPriest) > 1)) {
			str[0] = '_' + ((Potion*)rawThingPtr)->getPower() / 40;
			str[1] = ' ';
			str[2] = '\0';
			strcat(str, objMan._g352_objectNames[iconIndex]);
			descString = str;
		} else {
			descString = objMan._g352_objectNames[iconIndex];
		}

		textMan.f52_printToViewport(134, 68, k13_ColorLightestGray, descString);
		f332_drawIconToViewport(iconIndex, 111, 59);

		char *attribString[4] = {"CONSUMABLE", "POISONED", "BROKEN", "CURSED"}; // TODO: localization

		_g422_objDescTextYpos = 87;

		uint16 potentialAttribMask;
		uint16 actualAttribMask;
		switch (thingType) {
		case k5_WeaponThingType: {
			potentialAttribMask = k0x0008_DescriptionMaskCursed | k0x0002_DescriptionMaskPoisoned | k0x0004_DescriptionMaskBroken;
			Weapon *weapon = (Weapon*)rawThingPtr;
			actualAttribMask = (weapon->getCursed() << 3) | (weapon->getPoisoned() << 1) | (weapon->getBroken() << 2);
			if ((iconIndex >= k4_IconIndiceWeaponTorchUnlit)
				&& (iconIndex <= k7_IconIndiceWeaponTorchLit)
				&& (weapon->getChargeCount() == 0)) {
				f335_drawPanelObjectDescriptionString("(BURNT OUT)"); // TODO: localization
			}
			break;
		}
		case k6_ArmourThingType: {
			potentialAttribMask = k0x0008_DescriptionMaskCursed | k0x0004_DescriptionMaskBroken;
			Armour *armour = (Armour*)rawThingPtr;
			actualAttribMask = (armour->getCursed() << 3) | (armour->getBroken() << 2);
			break;
		}
		case k8_PotionThingType: {
			actualAttribMask = k0x0001_DescriptionMaskConsumable;
			Potion *potion = (Potion*)rawThingPtr;
			actualAttribMask = g237_ObjectInfo[k2_ObjectInfoIndexFirstPotion + potion->getType()].getAllowedSlots();
			break;
		}
		case k10_JunkThingType: {
			Junk *junk = (Junk*)rawThingPtr;
			if ((iconIndex >= k8_IconIndiceJunkWater) && (iconIndex <= k9_IconIndiceJunkWaterSkin)) {
				potentialAttribMask = 0;
				switch (junk->getChargeCount()) {
				case 0:
					descString = "(EMPTY)"; // TODO: localization
					break;
				case 1:
					descString = "(ALMOST EMPTY)"; // TODO: localization
					break;
				case 2:
					descString = "(ALMOST FULL)"; // TODO: localization
					break;
				case 3:
					descString = "(FULL)"; // TODO: localization
					break;
				}
				f335_drawPanelObjectDescriptionString(descString);
			} else if ((iconIndex >= k0_IconIndiceJunkCompassNorth) && (iconIndex <= k3_IconIndiceJunkCompassWest)) {
				potentialAttribMask = 0;
				strcpy(str, "PARTY FACING "); // TODO: localization
				static char* directionName[4] = {"NORTH", "EAST", "SOUTH", "WEST"}; // G0430_apc_DirectionNames // TODO: localization
				strcat(str, directionName[iconIndex]);
				f335_drawPanelObjectDescriptionString(str);
			} else {
				potentialAttribMask = k0x0001_DescriptionMaskConsumable;
				actualAttribMask = g237_ObjectInfo[k127_ObjectInfoIndexFirstJunk + junk->getType()].getAllowedSlots();
			}
			break;
		}
		} // end of switch 

		if (potentialAttribMask) {
			f336_buildObjectAttributeString(potentialAttribMask, actualAttribMask, attribString, str, "(", ")");
			f335_drawPanelObjectDescriptionString(str);
		}

		strcpy(str, "WEIGHS "); // TODO: localization

		uint16 weight = dunMan.f140_getObjectWeight(thingToDraw);
		strcat(str, champMan.f288_getStringFromInteger(weight / 10, false, 3).c_str());

		strcat(str, "."); // TODO: localization

		weight -= (weight / 10) * 10;
		strcat(str, champMan.f288_getStringFromInteger(weight, false, 1).c_str());

		strcat(str, " KG."); // TODO: localization

		f335_drawPanelObjectDescriptionString(str);
	}
	f339_drawPanelArrowOrEye(pressingEye);

}

void InventoryMan::f337_setDungeonViewPalette() {
	uint16 L1039_ui_Multiple;
#define AL1039_ui_SlotIndex    L1039_ui_Multiple
#define AL1039_ui_PaletteIndex L1039_ui_Multiple
#define AL1039_ui_Counter      L1039_ui_Multiple
	int16* L1040_pi_Multiple;
#define AL1040_pi_TorchLightPower L1040_pi_Multiple
#define AL1040_pi_LightAmount     L1040_pi_Multiple
	uint16 L1044_ui_Multiple;
#define AL1044_T_Thing            L1044_ui_Multiple
#define AL1044_ui_TorchLightPower L1044_ui_Multiple
	int16 L1045_ai_TorchesLightPower[8];

	int16 g40_palIndexToLightAmmount[6] = {99, 75, 50, 25, 1, 0}; // @ G0040_ai_Graphic562_PaletteIndexToLightAmount

	if (_vm->_dungeonMan->_g269_currMap->_difficulty == 0) {
		_vm->_displayMan->_g304_dungeonViewPaletteIndex = 0; /* Brightest color palette index */
	} else {
		/* Get torch light power from both hands of each champion in the party */
		int16 L1038_i_Counter = 4; /* BUG0_01 Coding error without consequence. The hands of four champions are inspected even if there are less champions in the party. No consequence as the data in unused champions is set to 0 and _vm->_objectMan->f32_getObjectType then returns -1 */
		Champion *L1043_ps_Champion = _vm->_championMan->_gK71_champions;
		AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		while (L1038_i_Counter--) {
			AL1039_ui_SlotIndex = k1_ChampionSlotActionHand + 1;
			while (AL1039_ui_SlotIndex--) {
				if ((_vm->_objectMan->f32_getObjectType(Thing(AL1044_T_Thing = L1043_ps_Champion->_slots[AL1039_ui_SlotIndex].toUint16())) >= k4_IconIndiceWeaponTorchUnlit) &&
					(_vm->_objectMan->f32_getObjectType(Thing(AL1044_T_Thing = L1043_ps_Champion->_slots[AL1039_ui_SlotIndex].toUint16())) <= k7_IconIndiceWeaponTorchLit)) {
					Weapon *L1042_ps_Weapon = (Weapon*)_vm->_dungeonMan->f156_getThingData(Thing(AL1044_T_Thing));
					*AL1040_pi_TorchLightPower = L1042_ps_Weapon->getChargeCount();
				} else {
					*AL1040_pi_TorchLightPower = 0;
				}
				AL1040_pi_TorchLightPower++;
			}
			L1043_ps_Champion++;
		}
		/* Sort torch light power values so that the four highest values are in the first four entries in the array L1045_ai_TorchesLightPower in decreasing order. The last four entries contain the smallest values but they are not sorted */
		AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		AL1039_ui_Counter = 0;
		while (AL1039_ui_Counter != 4) {
			L1038_i_Counter = 7 - AL1039_ui_Counter;
			int16 *L1041_pi_TorchLightPower = &L1045_ai_TorchesLightPower[AL1039_ui_Counter + 1];
			while (L1038_i_Counter--) {
				if (*L1041_pi_TorchLightPower > *AL1040_pi_TorchLightPower) {
					AL1044_ui_TorchLightPower = *L1041_pi_TorchLightPower;
					*L1041_pi_TorchLightPower = *AL1040_pi_TorchLightPower;
					*AL1040_pi_TorchLightPower = AL1044_ui_TorchLightPower;
				}
				L1041_pi_TorchLightPower++;
			}
			AL1040_pi_TorchLightPower++;
			AL1039_ui_Counter++;
		}
		/* Get total light amount provided by the four torches with the highest light power values and by the fifth torch in the array which may be any one of the four torches with the smallest ligh power values */
		uint16 L1037_ui_TorchLightAmountMultiplier = 6;
		AL1039_ui_Counter = 5;
		int16 L1036_i_TotalLightAmount = 0;
		AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		while (AL1039_ui_Counter--) {
			if (*AL1040_pi_TorchLightPower) {
				L1036_i_TotalLightAmount += (g39_LightPowerToLightAmount[*AL1040_pi_TorchLightPower] << L1037_ui_TorchLightAmountMultiplier) >> 6;
				L1037_ui_TorchLightAmountMultiplier = MAX(0, L1037_ui_TorchLightAmountMultiplier - 1);
			}
			AL1040_pi_TorchLightPower++;
		}
		L1036_i_TotalLightAmount += _vm->_championMan->_g407_party._magicalLightAmount;
		/* Select palette corresponding to the total light amount */
		AL1040_pi_LightAmount = g40_palIndexToLightAmmount;
		if (L1036_i_TotalLightAmount > 0) {
			AL1039_ui_PaletteIndex = 0; /* Brightest color palette index */
			while (*AL1040_pi_LightAmount++ > L1036_i_TotalLightAmount) {
				AL1039_ui_PaletteIndex++;
			}
		} else {
			AL1039_ui_PaletteIndex = 5; /* Darkest color palette index */
		}
		_vm->_displayMan->_g304_dungeonViewPaletteIndex = AL1039_ui_PaletteIndex;
	}

	_vm->_displayMan->_g342_refreshDungeonViewPaleteRequested = true;
}

void InventoryMan::f338_decreaseTorchesLightPower() {
	int16 L1046_i_ChampionCount;
	int16 L1047_i_SlotIndex;
	bool L1048_B_TorchChargeCountChanged;
	int16 L1049_i_IconIndex;
	Champion* L1050_ps_Champion;
	Weapon* L1051_ps_Weapon;


	L1048_B_TorchChargeCountChanged = false;
	L1046_i_ChampionCount = _vm->_championMan->_g305_partyChampionCount;
	if (_vm->_championMan->_g299_candidateChampionOrdinal) {
		L1046_i_ChampionCount--;
	}
	L1050_ps_Champion = _vm->_championMan->_gK71_champions;
	while (L1046_i_ChampionCount--) {
		L1047_i_SlotIndex = k1_ChampionSlotActionHand + 1;
		while (L1047_i_SlotIndex--) {
			L1049_i_IconIndex = _vm->_objectMan->f33_getIconIndex(L1050_ps_Champion->_slots[L1047_i_SlotIndex]);
			if ((L1049_i_IconIndex >= k4_IconIndiceWeaponTorchUnlit) && (L1049_i_IconIndex <= k7_IconIndiceWeaponTorchLit)) {
				L1051_ps_Weapon = (Weapon*)_vm->_dungeonMan->f156_getThingData(L1050_ps_Champion->_slots[L1047_i_SlotIndex]);
				if (L1051_ps_Weapon->getChargeCount()) {
					if (L1051_ps_Weapon->setChargeCount(L1051_ps_Weapon->getChargeCount() - 1) == 0) {
						L1051_ps_Weapon->setDoNotDiscard(false);
					}
					L1048_B_TorchChargeCountChanged = true;
				}
			}
		}
		L1050_ps_Champion++;
	}
	if (L1048_B_TorchChargeCountChanged) {
		f337_setDungeonViewPalette();
		_vm->_championMan->f296_drawChangedObjectIcons();
	}
}

void InventoryMan::f351_drawChampionSkillsAndStatistics() {
	uint16 L1090_ui_Multiple;
#define AL1090_ui_SkillIndex     L1090_ui_Multiple
#define AL1090_ui_StatisticIndex L1090_ui_Multiple
	int16 L1091_i_Y;
	int16 L1092_i_Multiple;
#define AL1092_i_SkillLevel            L1092_i_Multiple
#define AL1092_i_StatisticCurrentValue L1092_i_Multiple
	uint16 L1093_ui_ChampionIndex;
	Champion* L1094_ps_Champion;
	int16 L1095_i_StatisticColor;
	uint16 L1096_ui_StatisticMaximumValue;
	char L1097_ac_String[20];
	// TODO: localization
	static char* G0431_apc_StatisticNames[7] = {"L", "STRENGTH", "DEXTERITY", "WISDOM", "VITALITY", "ANTI-MAGIC", "ANTI-FIRE"};


	_vm->_inventoryMan->f334_closeChest();
	L1094_ps_Champion = &_vm->_championMan->_gK71_champions[L1093_ui_ChampionIndex = _vm->M1_ordinalToIndex(_vm->_inventoryMan->_g432_inventoryChampionOrdinal)];
	_vm->_displayMan->f20_blitToViewport(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k20_PanelEmptyIndice), g32_BoxPanel, k72_byteWidth, k8_ColorRed, 73);
	L1091_i_Y = 58;
	for (AL1090_ui_SkillIndex = k0_ChampionSkillFighter; AL1090_ui_SkillIndex <= k3_ChampionSkillWizard; AL1090_ui_SkillIndex++) {
		AL1092_i_SkillLevel = MIN((uint16)16, _vm->_championMan->f303_getSkillLevel(L1093_ui_ChampionIndex, AL1090_ui_SkillIndex | k0x8000_IgnoreTemporaryExperience));
		if (AL1092_i_SkillLevel == 1)
			continue;
#ifdef COMPILE17_DM10aEN_DM10bEN_DM11EN_DM12EN_CSB20EN_CSB21EN_DMDEMO20EN_DM20EN_DM21EN_DM22EN /* CHANGE4_00_LOCALIZATION Translation to German language */
		strcpy(L1097_ac_String, G0428_apc_SkillLevelNames[AL1092_i_SkillLevel - 2]);
		strcat(L1097_ac_String, " ");
		strcat(L1097_ac_String, G0417_apc_BaseSkillNames[AL1090_ui_SkillIndex]);
#endif
#ifdef COMPILE36_DM12GE_DM13aFR_DM13bFR_DM20GE_DM20FR_DM22GE /* CHANGE4_00_LOCALIZATION Translation to German language */
		strcpy(L1097_ac_String, G0417_apc_BaseSkillNames[AL1090_ui_SkillIndex]);
		strcat(L1097_ac_String, " ");
		strcat(L1097_ac_String, G0428_apc_SkillLevelNames[AL1092_i_SkillLevel - 2]);
#endif
		_vm->_textMan->f52_printToViewport(108, L1091_i_Y, k13_ColorLightestGray, L1097_ac_String);
		L1091_i_Y += 7;
	}
	L1091_i_Y = 86;
	for (AL1090_ui_StatisticIndex = k1_ChampionStatStrength; AL1090_ui_StatisticIndex <= k6_ChampionStatAntifire; AL1090_ui_StatisticIndex++) {
		_vm->_textMan->f52_printToViewport(108, L1091_i_Y, k13_ColorLightestGray, G0431_apc_StatisticNames[AL1090_ui_StatisticIndex]);
		AL1092_i_StatisticCurrentValue = L1094_ps_Champion->_statistics[AL1090_ui_StatisticIndex][k1_ChampionStatCurrent];
		L1096_ui_StatisticMaximumValue = L1094_ps_Champion->_statistics[AL1090_ui_StatisticIndex][k0_ChampionStatMaximum];
		if (AL1092_i_StatisticCurrentValue < L1096_ui_StatisticMaximumValue) {
			L1095_i_StatisticColor = k8_ColorRed;
		} else {
			if (AL1092_i_StatisticCurrentValue > L1096_ui_StatisticMaximumValue) {
				L1095_i_StatisticColor = k7_ColorLightGreen;
			} else {
				L1095_i_StatisticColor = k13_ColorLightestGray;
			}
		}
		_vm->_textMan->f52_printToViewport(174, L1091_i_Y, (Color)L1095_i_StatisticColor, _vm->_championMan->f288_getStringFromInteger(AL1092_i_StatisticCurrentValue, true, 3).c_str());
		strcpy(L1097_ac_String, "/");
		strcat(L1097_ac_String, _vm->_championMan->f288_getStringFromInteger(L1096_ui_StatisticMaximumValue, true, 3).c_str());
		_vm->_textMan->f52_printToViewport(192, L1091_i_Y, k13_ColorLightestGray, L1097_ac_String);
		L1091_i_Y += 7;
	}
}

void InventoryMan::f350_drawStopPressingMouth() {
	_vm->_inventoryMan->f347_drawPanel();
	_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
	warning(false, "Ignored code: G0587_i_HideMousePointerRequestCount");
	_vm->_eventMan->f77_hideMouse();
}

void InventoryMan::f353_drawStopPressingEye() {
	Thing L1100_T_LeaderHandObject;

	_vm->_inventoryMan->f332_drawIconToViewport(k202_IconIndiceEyeNotLooking, 12, 13);
	_vm->_inventoryMan->f347_drawPanel();
	_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
	if ((L1100_T_LeaderHandObject = _vm->_championMan->_g414_leaderHandObject) != Thing::_none) {
		_vm->_objectMan->f34_drawLeaderObjectName(L1100_T_LeaderHandObject);
	}
	_vm->_eventMan->f77_hideMouse();
}
}
