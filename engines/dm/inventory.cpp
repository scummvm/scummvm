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
	_g424_panelContent = k0_PanelContentFoodWaterPoisoned;
	for (uint16 i = 0; i < 8; ++i)
		_g425_chestSlots[i] = Thing::_none;
	_g426_openChest = Thing::_none;
	_g426_openChest = Thing::_none;
}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	ChampionMan &cm = *_vm->_championMan;
	EventManager &em = *_vm->_eventMan;
	DisplayMan &dm = *_vm->_displayMan;

	if ((championIndex != k4_ChampionCloseInventory) && !cm._champions[championIndex]._currHealth)
		return;
	if (_vm->_g331_pressingEye || _vm->_g333_pressingMouth)
		return;
	_vm->_g321_stopWaitingForPlayerInput = true;
	int16 invChampOrdinal = _g432_inventoryChampionOrdinal; // copy, as the original will be edited
	if (_vm->indexToOrdinal(championIndex) == invChampOrdinal) {
		championIndex = k4_ChampionCloseInventory;
	}

	Champion *champion;
	if (invChampOrdinal) {
		_g432_inventoryChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
		closeChest();
		champion = &cm._champions[_vm->ordinalToIndex(invChampOrdinal)];
		if (champion->_currHealth && !cm._g299_candidateChampionOrdinal) {
			champion->setAttributeFlag(k0x1000_ChampionAttributeStatusBox, true);
			cm.drawChampionState((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
		}
		if (cm._g300_partyIsSleeping) {
			return;
		}
		if (championIndex == k4_ChampionCloseInventory) {
			em._g326_refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			em._g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
			warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
			warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
			return;
		}
	}

	dm._g578_useByteBoxCoordinates = false;
	_g432_inventoryChampionOrdinal = _vm->indexToOrdinal(championIndex);
	if (!invChampOrdinal) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}

	champion = &cm._champions[championIndex];
	int16 w = dm.getWidth(k17_InventoryGraphicIndice);
	int16 h = dm.getHeight(k17_InventoryGraphicIndice);
	dm.blitToScreen(dm.getBitmap(k17_InventoryGraphicIndice), w, 0, 0, 0, w, 0, h, k255_ColorNoTransparency, g296_DungeonViewport);
	if (cm._g299_candidateChampionOrdinal) {
		dm.clearScreenBox(k12_ColorDarkestGray, g41_BoxFloppyZzzCross, g296_DungeonViewport);
	}
	_vm->_textMan->printToViewport(5, 116, k13_ColorLightestGray, "HEALTH");
	_vm->_textMan->printToViewport(5, 124, k13_ColorLightestGray, "STAMINA");
	_vm->_textMan->printToViewport(5, 132, k13_ColorLightestGray, "MANA");

	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		_vm->_championMan->drawSlot(championIndex, (ChampionSlot)slotIndex);
	}

	champion->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
	champion->setAttributeFlag(k0x1000_ChampionAttributeStatusBox, true);
	champion->setAttributeFlag(k0x0800_ChampionAttributePanel, true);
	champion->setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
	champion->setAttributeFlag(k0x0100_ChampionAttributeStatistics, true);
	champion->setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);

	cm.drawChampionState(championIndex);
	em._g598_mousePointerBitmapUpdated = true;
	em._g442_secondaryMouseInput = g449_SecondaryMouseInput_ChampionInventory;
	warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}

void InventoryMan::drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan._g578_useByteBoxCoordinates = false;
	Box box;
	box._y1 = 0;
	box._y2 = 28 + 1;
	box._x1 = championIndex * k69_ChampionStatusBoxSpacing + 7;
	box._x2 = box._x1 + 31 + 1;
	dispMan.blitToScreen(_vm->_championMan->_champions[championIndex]._portrait, 32, 0, 0, box, k255_ColorNoTransparency);
}

void InventoryMan::drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	Box box;
	box._x1 = x;
	box._x2 = box._x1 + pixelWidth + 1;
	box._y1 = y;
	box._y2 = box._y1 + 6 + 1;
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_displayMan->clearScreenBox(color, box);
}

void InventoryMan::drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
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
	drawPanelHorizontalBar(115, y + 2, pixelWidth, k0_ColorBlack);
	drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::drawPanelFoodWaterPoisoned() {
	Champion &champ = _vm->_championMan->_champions[_g432_inventoryChampionOrdinal];
	closeChest();
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToScreen(dispMan.getBitmap(k20_PanelEmptyIndice), 144, 0, 0, g32_BoxPanel, k8_ColorRed);
	dispMan.blitToScreen(dispMan.getBitmap(k30_FoodLabelIndice), 48, 0, 0, g35_BoxFood, k12_ColorDarkestGray);
	dispMan.blitToScreen(dispMan.getBitmap(k31_WaterLabelIndice), 48, 0, 0, g36_BoxWater, k12_ColorDarkestGray);
	if (champ._poisonEventCount) {
		dispMan.blitToScreen(dispMan.getBitmap(k32_PoisionedLabelIndice), 96, 0, 0, g37_BoxPoisoned, k12_ColorDarkestGray);
	}
	drawPanelFoodOrWaterBar(champ._food, 69, k5_ColorLightBrown);
	drawPanelFoodOrWaterBar(champ._water, 92, k14_ColorBlue);
}

void InventoryMan::drawPanelResurrectReincarnate() {
	_g424_panelContent = k5_PanelContentResurrectReincarnate;
	_vm->_displayMan->blitToScreen(_vm->_displayMan->getBitmap(k40_PanelResurectReincaranteIndice), 144, 0, 0, g32_BoxPanel, k6_ColorDarkGreen, g296_DungeonViewport);
}

void InventoryMan::drawPanel() {
	warning("possible reintroduction of BUG0_48");
	closeChest(); // possibility of BUG0_48

	ChampionMan &cm = *_vm->_championMan;
	if (cm._g299_candidateChampionOrdinal) {
		drawPanelResurrectReincarnate();
		return;
	}

	Thing thing = cm._champions[_vm->ordinalToIndex(_g432_inventoryChampionOrdinal)].getSlot(k1_ChampionSlotActionHand);

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
		drawPanelFoodWaterPoisoned();
	} else {
		drawPanelObject(thing, false);
	}
}

void InventoryMan::closeChest() {
	DungeonMan &dunMan = *_vm->_dungeonMan;

	bool processFirstChestSlot = true;
	if (_g426_openChest == Thing::_none)
		return;
	Container *container = (Container*)dunMan.getThingData(_g426_openChest);
	_g426_openChest = Thing::_none;
	container->getSlot() = Thing::_endOfList;
	Thing prevThing;
	for (int16 chestSlotIndex = 0; chestSlotIndex < 8; ++chestSlotIndex) {
		Thing thing = _g425_chestSlots[chestSlotIndex];
		if (thing != Thing::_none) {
			_g425_chestSlots[chestSlotIndex] = Thing::_none; // CHANGE8_09_FIX

			if (processFirstChestSlot) {
				processFirstChestSlot = false;
				*dunMan.getThingData(thing) = Thing::_endOfList.toUint16();
				container->getSlot() = prevThing = thing;
			} else {
				dunMan.linkThingToList(thing, prevThing, kM1_MapXNotOnASquare, 0);
				prevThing = thing;
			}
		}
	}
}

void InventoryMan::drawPanelScrollTextLine(int16 yPos, char* text) {
	warning("CHANGE5_03_IMPROVEMENT");
	for (char* iter = text; *iter != '\0'; ++iter) {
		if ((*iter >= 'A') && (*iter <= 'Z')) {
			*iter -= 64;
		} else if (*iter >= '{') { // this branch is CHANGE5_03_IMPROVEMENT
			*iter -= 96;
		}
	}
	_vm->_textMan->printToViewport(162 - (6 * strlen(text) / 2), yPos, k0_ColorBlack, text, k15_ColorWhite);
}

void InventoryMan::drawPanelScroll(Scroll* scroll) {
	DisplayMan &dispMan = *_vm->_displayMan;

	char stringFirstLine[300];
	_vm->_dungeonMan->decodeText(stringFirstLine, Thing(scroll->getTextStringThingIndex()), (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));
	char *charRed = stringFirstLine;
	while (*charRed && (*charRed != '\n')) {
		charRed++;
	}
	*charRed = '\0';
	dispMan.blitToScreen(dispMan.getBitmap(k23_PanelOpenScrollIndice), 144, 0, 0, g32_BoxPanel, k8_ColorRed, g296_DungeonViewport);
	int16 lineCount = 1;
	charRed++;
	char *charGreen = charRed; // first char of the second line
	while (*charGreen) {
		warning("BUG0_47");
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
	drawPanelScrollTextLine(yPos, stringFirstLine);
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
		drawPanelScrollTextLine(yPos, charGreen);
		charGreen = charRed;
	}
}

void InventoryMan::openAndDrawChest(Thing thingToOpen, Container* chest, bool isPressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	ObjectMan &objMan = *_vm->_objectMan;

	if (_g426_openChest == thingToOpen)
		return;

	warning("CHANGE8_09_FIX");
	if (_g426_openChest != Thing::_none)
		closeChest(); // CHANGE8_09_FIX

	_g426_openChest = thingToOpen;
	if (!isPressingEye) {
		objMan.drawIconInSlotBox(k9_SlotBoxInventoryActionHand, k145_IconIndiceContainerChestOpen);
	}
	dispMan.blitToScreen(dispMan.getBitmap(k25_PanelOpenChestIndice), 144, 0, 0, g32_BoxPanel, k8_ColorRed);

	int16 chestSlotIndex = 0;
	Thing thing = chest->getSlot();
	int16 thingCount = 0;
	while (thing != Thing::_endOfList) {
		warning("CHANGE8_08_FIX");
		if (++thingCount > 8)
			break; // CHANGE8_08_FIX, make sure that no more than the first 8 objects in a chest are drawn

		objMan.drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, objMan.getIconIndex(thing));
		_g425_chestSlots[chestSlotIndex++] = thing;
		thing = _vm->_dungeonMan->getNextThing(thing);
	}
	while (chestSlotIndex < 8) {
		objMan.drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, kM1_IconIndiceNone);
		_g425_chestSlots[chestSlotIndex++] = Thing::_none;
	}
}

void InventoryMan::drawIconToViewport(IconIndice iconIndex, int16 xPos, int16 yPos) {
	static byte iconBitmap[16 * 16];
	Box box;
	box._x2 = (box._x1 = xPos) + 15 + 1;
	box._y2 = (box._y1 = yPos) + 15 + 1;
	_vm->_objectMan->extractIconFromBitmap(iconIndex, iconBitmap);
	_vm->_displayMan->blitToScreen(iconBitmap, 16, 0, 0, box, k255_ColorNoTransparency, g296_DungeonViewport);
}

void InventoryMan::buildObjectAttributeString(int16 potentialAttribMask, int16 actualAttribMask, char** attribStrings, char* destString, char* prefixString, char* suffixString) {
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

void InventoryMan::drawPanelObjectDescriptionString(char* descString) {
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

			_vm->_textMan->printToViewport(_g421_objDescTextXpos, _g422_objDescTextYpos, k13_ColorLightestGray, stringLine);
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

void InventoryMan::drawPanelArrowOrEye(bool pressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToScreen(dispMan.getBitmap(pressingEye ? k19_EyeForObjectDescriptionIndice : k18_ArrowForChestContentIndice),
						 16, 0, 0, g33_BoxArrowOrEye, k8_ColorRed, g296_DungeonViewport);
}


Box g34_BoxObjectDescCircle = Box(105, 136, 53, 79); // @ G0034_s_Graphic562_Box_ObjectDescriptionCircle 

#define k0x0001_DescriptionMaskConsumable 0x0001 // @ MASK0x0001_DESCRIPTION_CONSUMABLE
#define k0x0002_DescriptionMaskPoisoned 0x0002 // @ MASK0x0002_DESCRIPTION_POISONED  
#define k0x0004_DescriptionMaskBroken 0x0004 // @ MASK0x0004_DESCRIPTION_BROKEN    
#define k0x0008_DescriptionMaskCursed 0x0008 // @ MASK0x0008_DESCRIPTION_CURSED    

void InventoryMan::drawPanelObject(Thing thingToDraw, bool pressingEye) {
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	if (_vm->_g331_pressingEye || _vm->_g333_pressingMouth) {
		warning("BUG0_48 The contents of a chest are reorganized when an object with a statistic modifier is placed or removed on a champion");
		closeChest();
	}

	uint16 *rawThingPtr = dunMan.getThingData(thingToDraw);
	drawPanelObjectDescriptionString("\f"); // form feed
	ThingType thingType = thingToDraw.getType();
	if (thingType == k7_ScrollThingType) {
		drawPanelScroll((Scroll*)rawThingPtr);
	} else if (thingType == k9_ContainerThingType) {
		openAndDrawChest(thingToDraw, (Container*)rawThingPtr, pressingEye);
	} else {
		IconIndice iconIndex = objMan.getIconIndex(thingToDraw);
		dispMan.blitToScreen(dispMan.getBitmap(k20_PanelEmptyIndice), 144, 0, 0, g32_BoxPanel, k8_ColorRed, g296_DungeonViewport);
		dispMan.blitToScreen(dispMan.getBitmap(k29_ObjectDescCircleIndice), 32, 0, 0, g34_BoxObjectDescCircle, k12_ColorDarkestGray, g296_DungeonViewport);

		char *descString = nullptr;
		char str[40];
		if (iconIndex == k147_IconIndiceJunkChampionBones) {
			strcpy(str, champMan._champions[((Junk*)rawThingPtr)->getChargeCount()]._name);  // TODO: localization
			strcat(str, " "); // TODO: localization
			strcat(str, objMan._g352_objectNames[iconIndex]);  // TODO: localization

			descString = str;
		} else if ((thingType == k8_PotionThingType)
				   && (iconIndex != k163_IconIndicePotionWaterFlask)
				   && (champMan.getSkillLevel((ChampionIndex)_vm->ordinalToIndex(_g432_inventoryChampionOrdinal), k2_ChampionSkillPriest) > 1)) {
			str[0] = '_' + ((Potion*)rawThingPtr)->getPower() / 40;
			str[1] = ' ';
			str[2] = '\0';
			strcat(str, objMan._g352_objectNames[iconIndex]);
			descString = str;
		} else {
			descString = objMan._g352_objectNames[iconIndex];
		}

		textMan.printToViewport(134, 68, k13_ColorLightestGray, descString);
		drawIconToViewport(iconIndex, 111, 59);

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
				drawPanelObjectDescriptionString("(BURNT OUT)"); // TODO: localization
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
				drawPanelObjectDescriptionString(descString);
			} else if ((iconIndex >= k0_IconIndiceJunkCompassNorth) && (iconIndex <= k3_IconIndiceJunkCompassWest)) {
				potentialAttribMask = 0;
				strcpy(str, "PARTY FACING "); // TODO: localization
				static char* directionName[4] = {"NORTH", "EAST", "SOUTH", "WEST"}; // G0430_apc_DirectionNames // TODO: localization
				strcat(str, directionName[iconIndex]);
				drawPanelObjectDescriptionString(str);
			} else {
				potentialAttribMask = k0x0001_DescriptionMaskConsumable;
				actualAttribMask = g237_ObjectInfo[k127_ObjectInfoIndexFirstJunk + junk->getType()].getAllowedSlots();
			}
			break;
		}
		} // end of switch 

		if (potentialAttribMask) {
			buildObjectAttributeString(potentialAttribMask, actualAttribMask, attribString, str, "(", ")");
			drawPanelObjectDescriptionString(str);
		}

		strcpy(str, "WEIGHS "); // TODO: localization

		uint16 weight = dunMan.getObjectWeight(thingToDraw);
		strcat(str, champMan.getStringFromInteger(weight / 10, false, 3).c_str());

		strcat(str, "."); // TODO: localization

		weight -= (weight / 10) * 10;
		strcat(str, champMan.getStringFromInteger(weight, false, 1).c_str()); 

		strcat(str, " KG."); // TODO: localization

		drawPanelObjectDescriptionString(str);
	}
	drawPanelArrowOrEye(pressingEye);

}
}
