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

#include "graphics/surface.h"
#include "graphics/thumbnail.h"

#include "dm/inventory.h"
#include "dm/dungeonman.h"
#include "dm/eventman.h"
#include "dm/group.h"
#include "dm/menus.h"
#include "dm/gfx.h"
#include "dm/text.h"
#include "dm/objectman.h"
#include "dm/timeline.h"
#include "dm/projexpl.h"
#include "dm/sounds.h"


namespace DM {

void InventoryMan::initConstants() {
	static const char* skillLevelNamesEN[15] = {"NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN", "CRAFTSMAN",
		"ARTISAN", "ADEPT", "EXPERT", "` MASTER", "a MASTER","b MASTER", "c MASTER", "d MASTER", "e MASTER", "ARCHMASTER"};
	static const char* skillLevelNamesDE[15] = {"ANFAENGER", "NEULING", "LEHRLING", "ARBEITER", "GESELLE", "HANDWERKR", "FACHMANN",
		"EXPERTE", "` MEISTER", "a MEISTER", "b MEISTER", "c MEISTER", "d MEISTER", "e MEISTER", "ERZMEISTR"};
	static const char* skillLevelNamesFR[15] = {"NEOPHYTE", "NOVICE", "APPRENTI", "COMPAGNON", "ARTISAN", "PATRON",
		"ADEPTE", "EXPERT", "MAITRE '", "MAITRE a", "MAITRE b", "MAITRE c", "MAITRE d", "MAITRE e", "SUR-MAITRE"};
	const char **translatedSkillLevel;
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		translatedSkillLevel = skillLevelNamesEN;
		break;
	case Common::DE_DEU:
		translatedSkillLevel = skillLevelNamesDE;
		break;
	case Common::FR_FRA:
		translatedSkillLevel = skillLevelNamesFR;
		break;
	}
	for (int i = 0; i < 15; ++i)
		_skillLevelNames[i] = translatedSkillLevel[i];

	_boxPanel = Box(80, 223, 52, 124); // @ G0032_s_Graphic562_Box_Panel
}

InventoryMan::InventoryMan(DMEngine *vm) : _vm(vm) {
	_inventoryChampionOrdinal = 0;
	_panelContent = kDMPanelContentFoodWaterPoisoned;
	for (uint16 i = 0; i < 8; ++i)
		_chestSlots[i] = Thing(0);
	_openChest = _vm->_thingNone;
	_objDescTextXpos = 0;
	_objDescTextYpos = 0;

	for (int i = 0; i < 15; i++)
		_skillLevelNames[i] = nullptr;

	initConstants();
}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	static Box boxFloppyZzzCross(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross

	DisplayMan &display = *_vm->_displayMan;
	ChampionMan &championMan = *_vm->_championMan;

	if ((championIndex != kDMChampionCloseInventory) && !championMan._champions[championIndex]._currHealth)
		return;

	if (_vm->_pressingMouth || _vm->_pressingEye)
		return;

	_vm->_stopWaitingForPlayerInput = true;
	uint16 inventoryChampionOrdinal = _inventoryChampionOrdinal;
	if (_vm->indexToOrdinal(championIndex) == inventoryChampionOrdinal)
		championIndex = kDMChampionCloseInventory;

	_vm->_eventMan->showMouse();
	if (inventoryChampionOrdinal) {
		_inventoryChampionOrdinal = _vm->indexToOrdinal(kDMChampionNone);
		closeChest();
		Champion *champion = &championMan._champions[_vm->ordinalToIndex(inventoryChampionOrdinal)];
		if (champion->_currHealth && !championMan._candidateChampionOrdinal) {
			setFlag(champion->_attributes, kDMAttributeStatusBox);
			championMan.drawChampionState((ChampionIndex)_vm->ordinalToIndex(inventoryChampionOrdinal));
		}
		if (championMan._partyIsSleeping) {
			_vm->_eventMan->hideMouse();
			return;
		}
		if (championIndex == kDMChampionCloseInventory) {
			_vm->_eventMan->_refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			_vm->_eventMan->hideMouse();
			_vm->_eventMan->_secondaryMouseInput = _vm->_eventMan->_secondaryMouseInputMovement;
			_vm->_eventMan->_secondaryKeyboardInput = _vm->_eventMan->_secondaryKeyboardInputMovement;
			_vm->_eventMan->discardAllInput();
			display.drawFloorAndCeiling();
			return;
		}
	}
	display._useByteBoxCoordinates = false;
	_inventoryChampionOrdinal = _vm->indexToOrdinal(championIndex);
	if (!inventoryChampionOrdinal)
		display.shadeScreenBox(&display._boxMovementArrows, kDMColorBlack);

	Champion *champion = &championMan._champions[championIndex];
	display.loadIntoBitmap(kDMGraphicIdxInventory, display._bitmapViewport);
	if (championMan._candidateChampionOrdinal)
		display.fillBoxBitmap(display._bitmapViewport, boxFloppyZzzCross, kDMColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		_vm->_textMan->printToViewport(5, 116, kDMColorLightestGray, "HEALTH");
		_vm->_textMan->printToViewport(5, 124, kDMColorLightestGray, "STAMINA");
		break;
	case Common::DE_DEU:
		_vm->_textMan->printToViewport(5, 116, kDMColorLightestGray, "GESUND");
		_vm->_textMan->printToViewport(5, 124, kDMColorLightestGray, "KRAFT");
		break;
	case Common::FR_FRA:
		_vm->_textMan->printToViewport(5, 116, kDMColorLightestGray, "SANTE");
		_vm->_textMan->printToViewport(5, 124, kDMColorLightestGray, "VIGUEUR");
		break;
	}

	_vm->_textMan->printToViewport(5, 132, kDMColorLightestGray, "MANA");

	for (uint16 i = kDMSlotReadyHand; i < kDMSlotChest1; i++)
		championMan.drawSlot(championIndex, i);

	setFlag(champion->_attributes, kDMAttributeViewport | kDMAttributeStatusBox | kDMAttributePanel | kDMAttributeLoad | kDMAttributeStatistics | kDMAttributeNameTitle);
	championMan.drawChampionState(championIndex);
	_vm->_eventMan->_mousePointerBitmapUpdated = true;
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->_secondaryMouseInput = _vm->_eventMan->_secondaryMouseInputChampionInventory;
	_vm->_eventMan->_secondaryKeyboardInput = nullptr;
	_vm->_eventMan->discardAllInput();
}

void InventoryMan::drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;

	dispMan._useByteBoxCoordinates = false;
	Box box;
	box._rect.top = 0;
	box._rect.bottom = 28;
	box._rect.left = championIndex * kDMChampionStatusBoxSpacing + 7;
	box._rect.right = box._rect.left + 31;
	dispMan.blitToScreen(_vm->_championMan->_champions[championIndex]._portrait, &box, k16_byteWidth, kDMColorNoTransparency, 29);
}

void InventoryMan::drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	DisplayMan &display = *_vm->_displayMan;
	Box box;
	box._rect.left = x;
	box._rect.right = box._rect.left + pixelWidth;
	box._rect.top = y;
	box._rect.bottom = box._rect.top + 6;
	display._useByteBoxCoordinates = false;
	display.fillBoxBitmap(display._bitmapViewport, box, color, k112_byteWidthViewport, k136_heightViewport);
}

void InventoryMan::drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
	if (amount < -512)
		color = kDMColorRed;
	else if (amount < 0)
		color = kDMColorYellow;

	int16 pixelWidth = amount + 1024;
	if (pixelWidth == 3072)
		pixelWidth = 3071;

	pixelWidth /= 32;
	drawPanelHorizontalBar(115, y + 2, pixelWidth, kDMColorBlack);
	drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::drawPanelFoodWaterPoisoned() {
	static Box boxFood(112, 159, 60, 68); // @ G0035_s_Graphic562_Box_Food
	static Box boxWater(112, 159, 83, 91); // @ G0036_s_Graphic562_Box_Water
	static Box boxPoisoned(112, 207, 105, 119); // @ G0037_s_Graphic562_Box_Poisoned

	Champion &champ = _vm->_championMan->_champions[_inventoryChampionOrdinal];
	closeChest();
	DisplayMan &dispMan = *_vm->_displayMan;

	dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxPanelEmpty), _boxPanel, k72_byteWidth, kDMColorRed, 73);

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxFoodLabel), boxFood, k24_byteWidth, kDMColorDarkestGray, 9);
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxWaterLabel), boxWater, k24_byteWidth, kDMColorDarkestGray, 9);
		break;
	case Common::DE_DEU:
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxFoodLabel), boxFood, k32_byteWidth, kDMColorDarkestGray, 9);
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxWaterLabel), boxWater, k32_byteWidth, kDMColorDarkestGray, 9);
		break;
	case Common::FR_FRA:
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxFoodLabel), boxFood, k48_byteWidth, kDMColorDarkestGray, 9);
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxWaterLabel), boxWater, k24_byteWidth, kDMColorDarkestGray, 9);
		break;
	}

	if (champ._poisonEventCount)
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxPoisionedLabel),
								   boxPoisoned, k48_byteWidth, kDMColorDarkestGray, 15);

	drawPanelFoodOrWaterBar(champ._food, 69, kDMColorLightBrown);
	drawPanelFoodOrWaterBar(champ._water, 92, kDMColorBlue);
}

void InventoryMan::drawPanelResurrectReincarnate() {
	DisplayMan &display = *_vm->_displayMan;

	_panelContent = kDMPanelContentResurrectReincarnate;
	display.blitToViewport(display.getNativeBitmapOrGraphic(kDMGraphicIdxPanelResurectReincarnate),
										 _boxPanel, k72_byteWidth, kDMColorDarkGreen, 73);
}

void InventoryMan::drawPanel() {
	closeChest();

	ChampionMan &cm = *_vm->_championMan;
	if (cm._candidateChampionOrdinal) {
		drawPanelResurrectReincarnate();
		return;
	}

	Thing thing = cm._champions[_vm->ordinalToIndex(_inventoryChampionOrdinal)].getSlot(kDMSlotActionHand);

	_panelContent = kDMPanelContentFoodWaterPoisoned;
	switch (thing.getType()) {
	case kDMThingTypeContainer:
		_panelContent = kDMPanelContentChest;
		break;
	case kDMThingTypeScroll:
		_panelContent = kDMPanelContentScroll;
		break;
	default:
		thing = _vm->_thingNone;
		break;
	}

	if (thing == _vm->_thingNone)
		drawPanelFoodWaterPoisoned();
	else
		drawPanelObject(thing, false);
}

void InventoryMan::closeChest() {
	DungeonMan &dunMan = *_vm->_dungeonMan;

	bool processFirstChestSlot = true;
	if (_openChest == _vm->_thingNone)
		return;
	Container *container = (Container *)dunMan.getThingData(_openChest);
	_openChest = _vm->_thingNone;
	container->getSlot() = _vm->_thingEndOfList;
	Thing prevThing;
	for (int16 chestSlotIndex = 0; chestSlotIndex < 8; ++chestSlotIndex) {
		Thing thing = _chestSlots[chestSlotIndex];
		if (thing != _vm->_thingNone) {
			_chestSlots[chestSlotIndex] = _vm->_thingNone; // CHANGE8_09_FIX

			if (processFirstChestSlot) {
				processFirstChestSlot = false;
				*dunMan.getThingData(thing) = _vm->_thingEndOfList.toUint16();
				container->getSlot() = prevThing = thing;
			} else {
				dunMan.linkThingToList(thing, prevThing, kDMMapXNotOnASquare, 0);
				prevThing = thing;
			}
		}
	}
}

void InventoryMan::drawPanelScrollTextLine(int16 yPos, char *text) {
	for (char *iter = text; *iter != '\0'; ++iter) {
		if ((*iter >= 'A') && (*iter <= 'Z'))
			*iter -= 64;
		else if (*iter >= '{') // this branch is CHANGE5_03_IMPROVEMENT
			*iter -= 96;
	}
	_vm->_textMan->printToViewport(162 - (6 * strlen(text) / 2), yPos, kDMColorBlack, text, kDMColorWhite);
}

void InventoryMan::drawPanelScroll(Scroll *scroll) {
	DisplayMan &dispMan = *_vm->_displayMan;

	char stringFirstLine[300];
	_vm->_dungeonMan->decodeText(stringFirstLine, Thing(scroll->getTextStringThingIndex()), (TextType)(kDMTextTypeScroll | kDMMaskDecodeEvenIfInvisible));
	char *charRed = stringFirstLine;
	while (*charRed && (*charRed != '\n'))
		charRed++;

	*charRed = '\0';
	dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxPanelOpenScroll),
							   _boxPanel, k72_byteWidth, kDMColorRed, 73);
	int16 lineCount = 1;
	charRed++;
	char *charGreen = charRed; // first char of the second line
	while (*charGreen) {
		/* BUG0_47 Graphical glitch when you open a scroll. If there is a single line of text in a scroll
		(with no carriage return) then charGreen points to undefined data. This may result in a graphical
		glitch and also corrupt other memory. This is not an issue in the original dungeons where all
		scrolls contain at least one carriage return character */
		if (*charGreen == '\n')
			lineCount++;

		charGreen++;
	}

	if (*(charGreen - 1) != '\n')
		lineCount++;
	else if (*(charGreen - 2) == '\n')
		lineCount--;

	int16 yPos = 92 - (7 * lineCount) / 2; // center the text vertically
	drawPanelScrollTextLine(yPos, stringFirstLine);
	charGreen = charRed;
	while (*charGreen) {
		yPos += 7;
		while (*charRed && (*charRed != '\n'))
			charRed++;

		if (!(*charRed))
			charRed[1] = '\0';

		*charRed++ = '\0';
		drawPanelScrollTextLine(yPos, charGreen);
		charGreen = charRed;
	}
}

void InventoryMan::openAndDrawChest(Thing thingToOpen, Container *chest, bool isPressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	ObjectMan &objMan = *_vm->_objectMan;

	if (_openChest == thingToOpen)
		return;

	if (_openChest != _vm->_thingNone)
		closeChest(); // CHANGE8_09_FIX

	_openChest = thingToOpen;
	if (!isPressingEye) {
		objMan.drawIconInSlotBox(kDMSlotBoxInventoryActionHand, kDMIconIndiceContainerChestOpen);
	}
	dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxPanelOpenChest),
							   _boxPanel, k72_byteWidth, kDMColorRed, 73);
	int16 chestSlotIndex = 0;
	Thing thing = chest->getSlot();
	int16 thingCount = 0;
	while (thing != _vm->_thingEndOfList) {
		if (++thingCount > 8)
			break; // CHANGE8_08_FIX, make sure that no more than the first 8 objects in a chest are drawn

		objMan.drawIconInSlotBox(chestSlotIndex + kDMSlotBoxChestFirstSlot, objMan.getIconIndex(thing));
		_chestSlots[chestSlotIndex++] = thing;
		thing = _vm->_dungeonMan->getNextThing(thing);
	}
	while (chestSlotIndex < 8) {
		objMan.drawIconInSlotBox(chestSlotIndex + kDMSlotBoxChestFirstSlot, kDMIconIndiceNone);
		_chestSlots[chestSlotIndex++] = _vm->_thingNone;
	}
}

void InventoryMan::drawIconToViewport(IconIndice iconIndex, int16 xPos, int16 yPos) {
	static byte iconBitmap[16 * 16];
	Box boxIcon(xPos, xPos + 15, yPos, yPos + 15);

	_vm->_objectMan->extractIconFromBitmap(iconIndex, iconBitmap);
	_vm->_displayMan->blitToViewport(iconBitmap, boxIcon, k8_byteWidth, kDMColorNoTransparency, 16);
}

void InventoryMan::buildObjectAttributeString(int16 potentialAttribMask, int16 actualAttribMask, const char **attribStrings, char *destString, const char *prefixString, const char *suffixString) {
	uint16 identicalBitCount = 0;
	int16 attribMask = 1;
	for (uint16 stringIndex = 0; stringIndex < 16; stringIndex++, attribMask <<= 1) {
		if (attribMask & potentialAttribMask & actualAttribMask)
			identicalBitCount++;
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

				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY: strcat(destString, " AND "); break;
				case Common::DE_DEU: strcat(destString, " UND "); break;
				case Common::FR_FRA: strcat(destString, " ET "); break;
				}
			}
		}
	}

	strcat(destString, suffixString);
}

void InventoryMan::drawPanelObjectDescriptionString(const char *descString) {
	if (descString[0] == '\f') { // form feed
		descString++;
		_objDescTextXpos = 108;
		_objDescTextYpos = 59;
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

			_vm->_textMan->printToViewport(_objDescTextXpos, _objDescTextYpos, kDMColorLightestGray, stringLine);
			_objDescTextYpos += 7;
			if (severalLines) {
				severalLines = false;
				stringLine = ++string;
			} else {
				*stringLine = '\0';
			}
		}
	}
}

void InventoryMan::drawPanelArrowOrEye(bool pressingEye) {
	static Box boxArrowOrEye(83, 98, 57, 65); // @ G0033_s_Graphic562_Box_ArrowOrEye

	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(pressingEye ? kDMGraphicIdxEyeForObjectDescription : kDMGraphicIdxArrowForChestContent),
							   boxArrowOrEye, k8_byteWidth, kDMColorRed, 9);
}

void InventoryMan::drawPanelObject(Thing thingToDraw, bool pressingEye) {
	static Box boxObjectDescCircle(105, 136, 53, 79); // @ G0034_s_Graphic562_Box_ObjectDescriptionCircle

	DungeonMan &dungeon = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	if (_vm->_pressingEye || _vm->_pressingMouth)
		closeChest();

	uint16 *rawThingPtr = dungeon.getThingData(thingToDraw);
	drawPanelObjectDescriptionString("\f"); // form feed
	ThingType thingType = thingToDraw.getType();
	if (thingType == kDMThingTypeScroll)
		drawPanelScroll((Scroll *)rawThingPtr);
	else if (thingType == kDMThingTypeContainer)
		openAndDrawChest(thingToDraw, (Container *)rawThingPtr, pressingEye);
	else {
		IconIndice iconIndex = objMan.getIconIndex(thingToDraw);
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxPanelEmpty),
								   _boxPanel, k72_byteWidth, kDMColorRed, 73);
		dispMan.blitToViewport(dispMan.getNativeBitmapOrGraphic(kDMGraphicIdxObjectDescCircle),
								   boxObjectDescCircle, k16_byteWidth, kDMColorDarkestGray, 27);

		Common::String descString;
		Common::String str;
		if (iconIndex == kDMIconIndiceJunkChampionBones) {
			switch (_vm->getGameLanguage()) { // localized
			case Common::FR_FRA:
				// Fix original bug dur to a cut&paste error: string was concatenated then overwritten by the name
				str = Common::String::format("%s %s", objMan._objectNames[iconIndex], champMan._champions[((Junk *)rawThingPtr)->getChargeCount()]._name);
				break;
			default: // German and English versions are the same
				str = Common::String::format("%s %s", champMan._champions[((Junk *)rawThingPtr)->getChargeCount()]._name, objMan._objectNames[iconIndex]);
				break;
			}

			descString = str;
		} else if ((thingType == kDMThingTypePotion)
				   && (iconIndex != kDMIconIndicePotionWaterFlask)
				   && (champMan.getSkillLevel((ChampionIndex)_vm->ordinalToIndex(_inventoryChampionOrdinal), kDMSkillPriest) > 1)) {
			str = ('_' + ((Potion *)rawThingPtr)->getPower() / 40);
			str += " ";
			str += objMan._objectNames[iconIndex];
			descString = str;
		} else {
			descString = objMan._objectNames[iconIndex];
		}

		textMan.printToViewport(134, 68, kDMColorLightestGray, descString.c_str());
		drawIconToViewport(iconIndex, 111, 59);


		_objDescTextYpos = 87;

		uint16 potentialAttribMask = 0;
		uint16 actualAttribMask = 0;
		switch (thingType) {
		case kDMThingTypeWeapon: {
			potentialAttribMask = kDMDescriptionMaskCursed | kDMDescriptionMaskPoisoned | kDMDescriptionMaskBroken;
			Weapon *weapon = (Weapon *)rawThingPtr;
			actualAttribMask = (weapon->getCursed() << 3) | (weapon->getPoisoned() << 1) | (weapon->getBroken() << 2);
			if ((iconIndex >= kDMIconIndiceWeaponTorchUnlit)
				&& (iconIndex <= kDMIconIndiceWeaponTorchLit)
				&& (weapon->getChargeCount() == 0)) {

				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY:
					drawPanelObjectDescriptionString("(BURNT OUT)");
					break;
				case Common::DE_DEU:
					drawPanelObjectDescriptionString("(AUSGEBRANNT)");
					break;
				case Common::FR_FRA:
					drawPanelObjectDescriptionString("(CONSUME)");
					break;
				}
			}
			break;
		}
		case kDMThingTypeArmour: {
			potentialAttribMask = kDMDescriptionMaskCursed | kDMDescriptionMaskBroken;
			Armour *armour = (Armour *)rawThingPtr;
			actualAttribMask = (armour->getCursed() << 3) | (armour->getBroken() << 2);
			break;
		}
		case kDMThingTypePotion: {
			potentialAttribMask = kDMDescriptionMaskConsumable;
			Potion *potion = (Potion *)rawThingPtr;
			actualAttribMask = dungeon._objectInfos[kDMObjectInfoIndexFirstPotion + potion->getType()].getAllowedSlots();
			break;
		}
		case kDMThingTypeJunk: {
			if ((iconIndex >= kDMIconIndiceJunkWater) && (iconIndex <= kDMIconIndiceJunkWaterSkin)) {
				potentialAttribMask = 0;
				const char *descStringEN[4] = {"(EMPTY)", "(ALMOST EMPTY)", "(ALMOST FULL)", "(FULL)"};
				const char *descStringDE[4] = {"(LEER)", "(FAST LEER)", "(FAST VOLL)", "(VOLL)"};
				const char *descStringFR[4] = {"(VIDE)", "(PRESQUE VIDE)", "(PRESQUE PLEINE)", "(PLEINE)"};

				Junk *junk = (Junk *)rawThingPtr;
				switch (_vm->getGameLanguage()) { // localized
				case Common::DE_DEU:
					descString = descStringDE[junk->getChargeCount()];
					break;
				case Common::FR_FRA:
					descString = descStringFR[junk->getChargeCount()];
					break;
				default:
					descString = descStringEN[junk->getChargeCount()];
					break;
				}

				drawPanelObjectDescriptionString(descString.c_str());
			} else if ((iconIndex >= kDMIconIndiceJunkCompassNorth) && (iconIndex <= kDMIconIndiceJunkCompassWest)) {
				const static char *directionNameEN[4] = {"NORTH", "EAST", "SOUTH", "WEST"};
				const static char *directionNameDE[4] = {"NORDEN", "OSTEN", "SUEDEN", "WESTEN"};
				const static char *directionNameFR[4] = {"AU NORD", "A L'EST", "AU SUD", "A L'OUEST"};

				potentialAttribMask = 0;

				switch (_vm->getGameLanguage()) { // localized
				case Common::DE_DEU:
					str = "GRUPPE BLICKT NACH ";
					str += directionNameDE[iconIndex];
					break;
				case Common::FR_FRA:
					str = "GROUPE FACE ";
					str += directionNameFR[iconIndex];
					break;
				default:
					str = "PARTY FACING ";
					str += directionNameEN[iconIndex];
					break;
				}

				drawPanelObjectDescriptionString(str.c_str());
			} else {
				Junk *junk = (Junk *)rawThingPtr;
				potentialAttribMask = kDMDescriptionMaskConsumable;
				actualAttribMask = dungeon._objectInfos[kDMObjectInfoIndexFirstJunk + junk->getType()].getAllowedSlots();
			}
			break;
		}
		default:
			break;
		} // end of switch

		if (potentialAttribMask) {
			static const char *attribStringEN[4] = {"CONSUMABLE", "POISONED", "BROKEN", "CURSED"};
			static const char *attribStringDE[4] = {"ESSBAR", "VERGIFTET", "DEFEKT", "VERFLUCHT"};
			static const char *attribStringFR[4] = {"COMESTIBLE", "EMPOISONNE", "BRISE", "MAUDIT"};
			const char **attribString = nullptr;

			switch (_vm->getGameLanguage()) { // localized
			case Common::DE_DEU:
				attribString = attribStringDE;
				break;
			case Common::FR_FRA:
				attribString = attribStringFR;
				break;
			default:
				attribString = attribStringEN;
				break;
			}

			char destString[40];
			buildObjectAttributeString(potentialAttribMask, actualAttribMask, attribString, destString, "(", ")");
			drawPanelObjectDescriptionString(destString);
		}

		uint16 weight = dungeon.getObjectWeight(thingToDraw);
		switch (_vm->getGameLanguage()) { // localized
		case Common::DE_DEU:
			str = "WIEGT " + champMan.getStringFromInteger(weight / 10, false, 3) + ",";
			break;
		case Common::FR_FRA:
			str = "PESE " + champMan.getStringFromInteger(weight / 10, false, 3) + "KG,";
			break;
		default:
			str = "WEIGHS " + champMan.getStringFromInteger(weight / 10, false, 3) + ".";
			break;
		}

		weight -= (weight / 10) * 10;
		str += champMan.getStringFromInteger(weight, false, 1);

		switch (_vm->getGameLanguage()) { // localized
		case Common::FR_FRA:
			str += ".";
			break;
		default:
			str += " KG.";
			break;
		}

		drawPanelObjectDescriptionString(str.c_str());
	}
	drawPanelArrowOrEye(pressingEye);
}

void InventoryMan::setDungeonViewPalette() {
	static const int16 palIndexToLightAmmount[6] = {99, 75, 50, 25, 1, 0}; // @ G0040_ai_Graphic562_PaletteIndexToLightAmount
	DisplayMan &display = *_vm->_displayMan;
	ChampionMan &championMan = *_vm->_championMan;
	DungeonMan &dungeon = *_vm->_dungeonMan;

	if (dungeon._currMap->_difficulty == 0) {
		display._dungeonViewPaletteIndex = 0; /* Brightest color palette index */
	} else {
		/* Get torch light power from both hands of each champion in the party */
		int16 counter = 4; /* BUG0_01 Coding error without consequence. The hands of four champions are inspected even if there are less champions in the party. No consequence as the data in unused champions is set to 0 and _vm->_objectMan->f32_getObjectType then returns -1 */
		Champion *curChampion = championMan._champions;
		int16 torchesLightPower[8];
		int16 *curTorchLightPower = torchesLightPower;
		while (counter--) {
			uint16 slotIndex = kDMSlotActionHand + 1;
			while (slotIndex--) {
				Thing slotThing = curChampion->_slots[slotIndex];
				if ((_vm->_objectMan->getObjectType(slotThing) >= kDMIconIndiceWeaponTorchUnlit) &&
					(_vm->_objectMan->getObjectType(slotThing) <= kDMIconIndiceWeaponTorchLit)) {
					Weapon *curWeapon = (Weapon *)dungeon.getThingData(slotThing);
					*curTorchLightPower = curWeapon->getChargeCount();
				} else {
					*curTorchLightPower = 0;
				}
				curTorchLightPower++;
			}
			curChampion++;
		}
		/* Sort torch light power values so that the four highest values are in the first four entries in the array L1045_ai_TorchesLightPower in decreasing order. The last four entries contain the smallest values but they are not sorted */
		curTorchLightPower = torchesLightPower;
		int16 torchIndex = 0;
		while (torchIndex != 4) {
			counter = 7 - torchIndex;
			int16 *L1041_pi_TorchLightPower = &torchesLightPower[torchIndex + 1];
			while (counter--) {
				if (*L1041_pi_TorchLightPower > *curTorchLightPower) {
					int16 AL1044_ui_TorchLightPower = *L1041_pi_TorchLightPower;
					*L1041_pi_TorchLightPower = *curTorchLightPower;
					*curTorchLightPower = AL1044_ui_TorchLightPower;
				}
				L1041_pi_TorchLightPower++;
			}
			curTorchLightPower++;
			torchIndex++;
		}
		/* Get total light amount provided by the four torches with the highest light power values and by the fifth torch in the array which may be any one of the four torches with the smallest ligh power values */
		uint16 torchLightAmountMultiplier = 6;
		torchIndex = 5;
		int16 totalLightAmount = 0;
		curTorchLightPower = torchesLightPower;
		while (torchIndex--) {
			if (*curTorchLightPower) {
				totalLightAmount += (championMan._lightPowerToLightAmount[*curTorchLightPower] << torchLightAmountMultiplier) >> 6;
				torchLightAmountMultiplier = MAX(0, torchLightAmountMultiplier - 1);
			}
			curTorchLightPower++;
		}
		totalLightAmount += championMan._party._magicalLightAmount;
		/* Select palette corresponding to the total light amount */
		const int16 *curLightAmount = palIndexToLightAmmount;
		int16 paletteIndex;
		if (totalLightAmount > 0) {
			paletteIndex = 0; /* Brightest color palette index */
			while (*curLightAmount++ > totalLightAmount)
				paletteIndex++;
		} else {
			paletteIndex = 5; /* Darkest color palette index */
		}
		display._dungeonViewPaletteIndex = paletteIndex;
	}

	display._refreshDungeonViewPaleteRequested = true;
}

void InventoryMan::decreaseTorchesLightPower() {
	ChampionMan &championMan = *_vm->_championMan;
	DungeonMan &dungeon = *_vm->_dungeonMan;

	bool torchChargeCountChanged = false;
	int16 championCount = championMan._partyChampionCount;
	if (championMan._candidateChampionOrdinal)
		championCount--;

	Champion *curChampion = championMan._champions;
	while (championCount--) {
		int16 slotIndex = kDMSlotActionHand + 1;
		while (slotIndex--) {
			int16 iconIndex = _vm->_objectMan->getIconIndex(curChampion->_slots[slotIndex]);
			if ((iconIndex >= kDMIconIndiceWeaponTorchUnlit) && (iconIndex <= kDMIconIndiceWeaponTorchLit)) {
				Weapon *curWeapon = (Weapon *)dungeon.getThingData(curChampion->_slots[slotIndex]);
				if (curWeapon->getChargeCount()) {
					if (curWeapon->setChargeCount(curWeapon->getChargeCount() - 1) == 0) {
						curWeapon->setDoNotDiscard(false);
					}
					torchChargeCountChanged = true;
				}
			}
		}
		curChampion++;
	}

	if (torchChargeCountChanged) {
		setDungeonViewPalette();
		championMan.drawChangedObjectIcons();
	}
}

void InventoryMan::drawChampionSkillsAndStatistics() {
	static const char *statisticNamesEN[7] = {"L", "STRENGTH", "DEXTERITY", "WISDOM", "VITALITY", "ANTI-MAGIC", "ANTI-FIRE"};
	static const char *statisticNamesDE[7] = {"L", "STAERKE", "FLINKHEIT", "WEISHEIT", "VITALITAET", "ANTI-MAGIE", "ANTI-FEUER"};
	static const char *statisticNamesFR[7] = {"L", "FORCE", "DEXTERITE", "SAGESSE", "VITALITE", "ANTI-MAGIE", "ANTI-FEU"};

	DisplayMan &display = *_vm->_displayMan;
	ChampionMan &championMan = *_vm->_championMan;
	const char **statisticNames;

	switch (_vm->getGameLanguage()) { // localized
	case Common::DE_DEU:
		statisticNames = statisticNamesDE;
		break;
	case Common::FR_FRA:
		statisticNames = statisticNamesFR;
		break;
	default:
		statisticNames = statisticNamesEN;
		break;
	}

	closeChest();
	uint16 championIndex = _vm->ordinalToIndex(_inventoryChampionOrdinal);
	Champion *curChampion = &championMan._champions[championIndex];
	display.blitToViewport(display.getNativeBitmapOrGraphic(kDMGraphicIdxPanelEmpty), _boxPanel, k72_byteWidth, kDMColorRed, 73);
	int16 textPosY = 58;
	for (uint16 idx = kDMSkillFighter; idx <= kDMSkillWizard; idx++) {
		int16 skillLevel = MIN((uint16)16, championMan.getSkillLevel(championIndex, idx | kDMIgnoreTemporaryExperience));
		if (skillLevel == 1)
			continue;

		Common::String displayString;

		switch (_vm->getGameLanguage()) { // localized
		case Common::FR_FRA:
			// Fix original bug: Due to a copy&paste error, the string was concatenate then overwritten be the last part
			displayString = Common::String::format("%s %s", championMan._baseSkillName[idx], _skillLevelNames[skillLevel - 2]);
			break;
		default: // English and German versions are built the same way
			displayString = Common::String::format("%s %s", _skillLevelNames[skillLevel - 2], championMan._baseSkillName[idx]);
			break;
		}
		_vm->_textMan->printToViewport(108, textPosY, kDMColorLightestGray, displayString.c_str());
		textPosY += 7;
	}
	textPosY = 86;
	for (uint16 idx = kDMStatStrength; idx <= kDMStatAntifire; idx++) {
		_vm->_textMan->printToViewport(108, textPosY, kDMColorLightestGray, statisticNames[idx]);
		int16 statisticCurrentValue = curChampion->_statistics[idx][kDMStatCurrent];
		uint16 statisticMaximumValue = curChampion->_statistics[idx][kDMStatMaximum];
		int16 statisticColor;
		if (statisticCurrentValue < statisticMaximumValue)
			statisticColor = kDMColorRed;
		else if (statisticCurrentValue > statisticMaximumValue)
			statisticColor = kDMColorLightGreen;
		else
			statisticColor = kDMColorLightestGray;

		_vm->_textMan->printToViewport(174, textPosY, (Color)statisticColor, championMan.getStringFromInteger(statisticCurrentValue, true, 3).c_str());
		Common::String displayString = "/" + championMan.getStringFromInteger(statisticMaximumValue, true, 3);
		_vm->_textMan->printToViewport(192, textPosY, kDMColorLightestGray, displayString.c_str());
		textPosY += 7;
	}
}

void InventoryMan::drawStopPressingMouth() {
	drawPanel();
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	_vm->_eventMan->_hideMousePointerRequestCount = 1;
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
}

void InventoryMan::drawStopPressingEye() {
	drawIconToViewport(kDMIconIndiceEyeNotLooking, 12, 13);
	drawPanel();
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	Thing leaderHandObject = _vm->_championMan->_leaderHandObject;
	if (leaderHandObject != _vm->_thingNone)
		_vm->_objectMan->drawLeaderObjectName(leaderHandObject);

	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
}

void InventoryMan::clickOnMouth() {
	static int16 foodAmounts[8] = {
		500,    /* Apple */
		600,    /* Corn */
		650,    /* Bread */
		820,    /* Cheese */
		550,    /* Screamer Slice */
		350,    /* Worm round */
		990,    /* Drumstick / Shank */
		1400    /* Dragon steak */
	};

	DisplayMan &display = *_vm->_displayMan;
	ChampionMan &championMan = *_vm->_championMan;
	DungeonMan &dungeon = *_vm->_dungeonMan;


	if (championMan._leaderEmptyHanded) {
		if (_panelContent == kDMPanelContentFoodWaterPoisoned)
			return;

		_vm->_eventMan->_ignoreMouseMovements = true;
		_vm->_pressingMouth = true;
		if (!_vm->_eventMan->isMouseButtonDown(kDMMouseButtonLeft)) {
			_vm->_eventMan->_ignoreMouseMovements = false;
			_vm->_pressingMouth = false;
			_vm->_stopPressingMouth = false;
		} else {
			_vm->_eventMan->showMouse();
			_vm->_eventMan->_hideMousePointerRequestCount = 1;
			drawPanelFoodWaterPoisoned();
			display.drawViewport(k0_viewportNotDungeonView);
		}
		return;
	}

	if (championMan._candidateChampionOrdinal)
		return;

	Thing handThing = championMan._leaderHandObject;
	if (!getFlag(dungeon._objectInfos[dungeon.getObjectInfoIndex(handThing)]._allowedSlots, kDMMaskMouth))
		return;

	uint16 iconIndex = _vm->_objectMan->getIconIndex(handThing);
	uint16 handThingType = handThing.getType();
	uint16 handThingWeight = dungeon.getObjectWeight(handThing);
	uint16 championIndex = _vm->ordinalToIndex(_inventoryChampionOrdinal);
	Champion *curChampion = &championMan._champions[championIndex];
	Junk *junkData = (Junk *)dungeon.getThingData(handThing);
	bool removeObjectFromLeaderHand;
	if ((iconIndex >= kDMIconIndiceJunkWater) && (iconIndex <= kDMIconIndiceJunkWaterSkin)) {
		if (!(junkData->getChargeCount()))
			return;

		curChampion->_water = MIN(curChampion->_water + 800, 2048);
		junkData->setChargeCount(junkData->getChargeCount() - 1);
		removeObjectFromLeaderHand = false;
	} else if (handThingType == kDMThingTypePotion)
		removeObjectFromLeaderHand = false;
	else {
		junkData->setNextThing(_vm->_thingNone);
		removeObjectFromLeaderHand = true;
	}
	_vm->_eventMan->showMouse();
	if (removeObjectFromLeaderHand)
		championMan.getObjectRemovedFromLeaderHand();

	if (handThingType == kDMThingTypePotion) {
		uint16 potionPower = ((Potion *)junkData)->getPower();
		uint16 counter = ((511 - potionPower) / (32 + (potionPower + 1) / 8)) >> 1;
		uint16 adjustedPotionPower = (potionPower / 25) + 8; /* Value between 8 and 18 */

		switch (((Potion *)junkData)->getType()) {
		case kDMPotionTypeRos:
			adjustStatisticCurrentValue(curChampion, kDMStatDexterity, adjustedPotionPower);
			break;
		case kDMPotionTypeKu:
			adjustStatisticCurrentValue(curChampion, kDMStatStrength, (((Potion *)junkData)->getPower() / 35) + 5); /* Value between 5 and 12 */
			break;
		case kDMPotionTypeDane:
			adjustStatisticCurrentValue(curChampion, kDMStatWisdom, adjustedPotionPower);
			break;
		case kDMPotionTypeNeta:
			adjustStatisticCurrentValue(curChampion, kDMStatVitality, adjustedPotionPower);
			break;
		case kDMPotionTypeAntivenin:
			championMan.unpoison(championIndex);
			break;
		case kDMPotionTypeMon:
			curChampion->_currStamina += MIN(curChampion->_maxStamina - curChampion->_currStamina, curChampion->_maxStamina / counter);
			break;
		case kDMPotionTypeYa: {
			adjustedPotionPower += adjustedPotionPower >> 1;
			if (curChampion->_shieldDefense > 50)
				adjustedPotionPower >>= 2;

			curChampion->_shieldDefense += adjustedPotionPower;
			TimelineEvent newEvent;
			newEvent._type = kDMEventTypeChampionShield;
			newEvent._mapTime = _vm->setMapAndTime(dungeon._partyMapIndex, _vm->_gameTime + (adjustedPotionPower * adjustedPotionPower));
			newEvent._priority = championIndex;
			newEvent._Bu._defense = adjustedPotionPower;
			_vm->_timeline->addEventGetEventIndex(&newEvent);
			setFlag(curChampion->_attributes, kDMAttributeStatusBox);
			}
			break;
		case kDMPotionTypeEe: {
			uint16 mana = MIN(900, (curChampion->_currMana + adjustedPotionPower) + (adjustedPotionPower - 8));
			if (mana > curChampion->_maxMana)
				mana -= (mana - MAX(curChampion->_currMana, curChampion->_maxMana)) >> 1;

			curChampion->_currMana = mana;
			}
			break;
		case kDMPotionTypeVi: {
			uint16 healWoundIterationCount = MAX(1, (((Potion *)junkData)->getPower() / 42));
			curChampion->_currHealth += curChampion->_maxHealth / counter;
			int16 wounds = curChampion->_wounds;
			if (wounds) { /* If the champion is wounded */
				counter = 10;
				do {
					for (uint16 i = 0; i < healWoundIterationCount; i++)
						curChampion->_wounds &= _vm->getRandomNumber(65536);

					healWoundIterationCount = 1;
				} while ((wounds == curChampion->_wounds) && --counter); /* Loop until at least one wound is healed or there are no more heal iterations */
			}
			setFlag(curChampion->_attributes, kDMAttributeLoad | kDMAttributeWounds);
			}
			break;
		case kDMPotionTypeWaterFlask:
			curChampion->_water = MIN(curChampion->_water + 1600, 2048);
			break;
		default:
			break;
		}
		((Potion *)junkData)->setType(kDMPotionTypeEmptyFlask);
	} else if ((iconIndex >= kDMIconIndiceJunkApple) && (iconIndex < kDMIconIndiceJunkIronKey))
		curChampion->_food = MIN(curChampion->_food + foodAmounts[iconIndex - kDMIconIndiceJunkApple], 2048);

	if (curChampion->_currStamina > curChampion->_maxStamina)
		curChampion->_currStamina = curChampion->_maxStamina;

	if (curChampion->_currHealth > curChampion->_maxHealth)
		curChampion->_currHealth = curChampion->_maxHealth;

	if (removeObjectFromLeaderHand) {
		for (uint16 i = 5; --i; _vm->delay(8)) { /* Animate mouth icon */
			_vm->_objectMan->drawIconToScreen(kDMIconIndiceMouthOpen + !(i & 0x0001), 56, 46);
			_vm->_eventMan->discardAllInput();
			if (_vm->_engineShouldQuit)
				return;
			display.updateScreen();
		}
	} else {
		championMan.drawChangedObjectIcons();
		championMan._champions[championMan._leaderIndex]._load += dungeon.getObjectWeight(handThing) - handThingWeight;
		setFlag(championMan._champions[championMan._leaderIndex]._attributes, kDMAttributeLoad);
	}
	_vm->_sound->requestPlay(kDMSoundIndexSwallow, dungeon._partyMapX, dungeon._partyMapY, kDMSoundModePlayImmediately);
	setFlag(curChampion->_attributes, kDMAttributeStatistics);

	if (_panelContent == kDMPanelContentFoodWaterPoisoned)
		setFlag(curChampion->_attributes, kDMAttributePanel);

	championMan.drawChampionState((ChampionIndex)championIndex);
	_vm->_eventMan->hideMouse();
}

void InventoryMan::adjustStatisticCurrentValue(Champion *champ, uint16 statIndex, int16 valueDelta) {
	int16 delta;
	if (valueDelta >= 0) {
		int16 currentValue = champ->_statistics[statIndex][kDMStatCurrent];
		if (currentValue > 120) {
			valueDelta >>= 1;
			if (currentValue > 150) {
				valueDelta >>= 1;
			}
			valueDelta++;
		}
		delta = MIN(valueDelta, (int16)(170 - currentValue));
	} else { /* BUG0_00 Useless code. The function is always called with valueDelta having a positive value */
		delta = MAX(valueDelta, int16(champ->_statistics[statIndex][kDMStatMinimum] - champ->_statistics[statIndex][kDMStatCurrent]));
	}
	champ->_statistics[statIndex][kDMStatCurrent] += delta;
}

void InventoryMan::clickOnEye() {
	ChampionMan &championMan = *_vm->_championMan;

	_vm->_eventMan->_ignoreMouseMovements = true;
	_vm->_pressingEye = true;
	if (!_vm->_eventMan->isMouseButtonDown(kDMMouseButtonLeft)) {
		_vm->_eventMan->_ignoreMouseMovements = false;
		_vm->_pressingEye = false;
		_vm->_stopPressingEye = false;
		return;
	}
	_vm->_eventMan->discardAllInput();
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->hideMouse();
	_vm->delay(8);
	drawIconToViewport(kDMIconIndiceEyeLooking, 12, 13);
	if (championMan._leaderEmptyHanded)
		drawChampionSkillsAndStatistics();
	else {
		_vm->_objectMan->clearLeaderObjectName();
		drawPanelObject(championMan._leaderHandObject, true);
	}
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
}

}
