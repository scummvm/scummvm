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



namespace DM {

Box gBoxFloppyZzzCross = Box(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross
Box gBoxPanel = Box(80, 223, 52, 124); // @ G0032_s_Graphic562_Box_Panel
Box gBoxFood = Box(112, 159, 60, 68); // @ G0035_s_Graphic562_Box_Food
Box gBoxWater = Box(112, 159, 83, 91); // @ G0036_s_Graphic562_Box_Water
Box gBoxPoisoned = Box(112, 207, 105, 119); // @ G0037_s_Graphic562_Box_Poisoned

InventoryMan::InventoryMan(DMEngine *vm) : _vm(vm) {
	_panelContent = kPanelContentFoodWaterPoisoned;
	for (uint16 i = 0; i < 8; ++i)
		_chestSlots[i] = Thing::_thingNone;
	_openChest = Thing::_thingNone;
	_openChest = Thing::_thingNone;
}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	ChampionMan &cm = *_vm->_championMan;
	EventManager &em = *_vm->_eventMan;
	DisplayMan &dm = *_vm->_displayMan;

	if ((championIndex != kChampionCloseInventory) && !cm._champions[championIndex]._currHealth)
		return;
	if (_vm->_pressingEye || _vm->_pressingMouth)
		return;
	_vm->_stopWaitingForPlayerInput = true;
	int16 invChampOrdinal = _inventoryChampionOrdinal; // copy, as the original will be edited
	if (_vm->indexToOrdinal(championIndex) == invChampOrdinal) {
		championIndex = kChampionCloseInventory;
	}

	Champion *champion;
	if (invChampOrdinal) {
		_inventoryChampionOrdinal = _vm->indexToOrdinal(kChampionNone);
		closeChest();
		champion = &cm._champions[_vm->ordinalToIndex(invChampOrdinal)];
		if (champion->_currHealth && !cm._candidateChampionOrdinal) {
			champion->setAttributeFlag(kChampionAttributeStatusBox, true);
			cm.drawChampionState((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
		}
		if (cm._partyIsSleeping) {
			return;
		}
		if (championIndex == kChampionCloseInventory) {
			em._refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			em._secondaryMouseInput = gSecondaryMouseInput_Movement;
			warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
			warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
			return;
		}
	}

	dm._useByteBoxCoordinates = false;
	_inventoryChampionOrdinal = _vm->indexToOrdinal(championIndex);
	if (!invChampOrdinal) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}

	champion = &cm._champions[championIndex];
	int16 w = dm.getWidth(kInventoryGraphicIndice);
	int16 h = dm.getHeight(kInventoryGraphicIndice);
	dm.blitToScreen(dm.getBitmap(kInventoryGraphicIndice), w, 0, 0, 0, w, 0, h, kColorNoTransparency, gDungeonViewport);
	if (cm._candidateChampionOrdinal) {
		dm.clearScreenBox(kColorDarkestGray, gBoxFloppyZzzCross, gDungeonViewport);
	}
	_vm->_textMan->printToViewport(5, 116, kColorLightestGray, "HEALTH");
	_vm->_textMan->printToViewport(5, 124, kColorLightestGray, "STAMINA");
	_vm->_textMan->printToViewport(5, 132, kColorLightestGray, "MANA");

	for (uint16 slotIndex = kChampionSlotReadyHand; slotIndex < kChampionSlotChest_1; slotIndex++) {
		_vm->_championMan->drawSlot(championIndex, (ChampionSlot)slotIndex);
	}

	champion->setAttributeFlag(kChampionAttributeViewport, true);
	champion->setAttributeFlag(kChampionAttributeStatusBox, true);
	champion->setAttributeFlag(kChampionAttributePanel, true);
	champion->setAttributeFlag(kChampionAttributeLoad, true);
	champion->setAttributeFlag(kChampionAttributeStatistics, true);
	champion->setAttributeFlag(kChampionAttributeNameTitle, true);

	cm.drawChampionState(championIndex);
	em._mousePointerBitmapUpdated = true;
	em._secondaryMouseInput = gSecondaryMouseInput_ChampionInventory;
	warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}

void InventoryMan::drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan._useByteBoxCoordinates = false;
	Box box;
	box._y1 = 0;
	box._y2 = 28 + 1;
	box._x1 = championIndex * kChampionStatusBoxSpacing + 7;
	box._x2 = box._x1 + 31 + 1;
	dispMan.blitToScreen(_vm->_championMan->_champions[championIndex]._portrait, 32, 0, 0, box, kColorNoTransparency);
}

void InventoryMan::drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	Box box;
	box._x1 = x;
	box._x2 = box._x1 + pixelWidth + 1;
	box._y1 = y;
	box._y2 = box._y1 + 6 + 1;
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->clearScreenBox(color, box);
}

void InventoryMan::drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
	if (amount < -512) {
		color = kColorRed;
	} else if (amount < 0) {
		color = kColorYellow;
	}

	int16 pixelWidth = amount + 1024;
	if (pixelWidth == 3072) {
		pixelWidth = 3071;
	}
	pixelWidth /= 32;
	drawPanelHorizontalBar(115, y + 2, pixelWidth, kColorBlack);
	drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::drawPanelFoodWaterPoisoned() {
	Champion &champ = _vm->_championMan->_champions[_inventoryChampionOrdinal];
	closeChest();
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToScreen(dispMan.getBitmap(kPanelEmptyIndice), 144, 0, 0, gBoxPanel, kColorRed);
	dispMan.blitToScreen(dispMan.getBitmap(kFoodLabelIndice), 48, 0, 0, gBoxFood, kColorDarkestGray);
	dispMan.blitToScreen(dispMan.getBitmap(kWaterLabelIndice), 48, 0, 0, gBoxWater, kColorDarkestGray);
	if (champ._poisonEventCount) {
		dispMan.blitToScreen(dispMan.getBitmap(kPoisionedLabelIndice), 96, 0, 0, gBoxPoisoned, kColorDarkestGray);
	}
	drawPanelFoodOrWaterBar(champ._food, 69, kColorLightBrown);
	drawPanelFoodOrWaterBar(champ._water, 92, kColorBlue);
}

void InventoryMan::drawPanelResurrectReincarnate() {
	_panelContent = kPanelContentResurrectReincarnate;
	_vm->_displayMan->blitToScreen(_vm->_displayMan->getBitmap(kPanelResurectReincaranteIndice), 144, 0, 0, gBoxPanel, kColorDarkGreen, gDungeonViewport);
}

void InventoryMan::drawPanel() {
	warning("possible reintroduction of BUG0_48");
	closeChest(); // possibility of BUG0_48

	ChampionMan &cm = *_vm->_championMan;
	if (cm._candidateChampionOrdinal) {
		drawPanelResurrectReincarnate();
		return;
	}

	Thing thing = cm._champions[_vm->ordinalToIndex(_inventoryChampionOrdinal)].getSlot(kChampionSlotActionHand);

	_panelContent = kPanelContentFoodWaterPoisoned;
	switch (thing.getType()) {
	case kContainerThingType:
		_panelContent = kPanelContentChest;
		break;
	case kScrollThingType:
		_panelContent = kPanelContentScroll;
		break;
	default:
		thing = Thing::_thingNone;
		break;
	}
	if (thing == Thing::_thingNone) {
		drawPanelFoodWaterPoisoned();
	} else {
		warning("MISSING CODE: F0342_INVENTORY_DrawPanel_Object(L1075_T_Thing, C0_FALSE);");
	}
}

void InventoryMan::closeChest() {
	DungeonMan &dunMan = *_vm->_dungeonMan;

	bool processFirstChestSlot = true;
	if (_openChest == Thing::_thingNone)
		return;
	Container *container = (Container*)dunMan.getThingData(_openChest);
	_openChest = Thing::_thingNone;
	container->getSlot() = Thing::_thingEndOfList;
	Thing prevThing;
	for (int16 chestSlotIndex = 0; chestSlotIndex < 8; ++chestSlotIndex) {
		Thing thing = _chestSlots[chestSlotIndex];
		if (thing != Thing::_thingNone) {
			_chestSlots[chestSlotIndex] = Thing::_thingNone; // CHANGE8_09_FIX

			if (processFirstChestSlot) {
				processFirstChestSlot = false;
				*dunMan.getThingData(thing) = Thing::_thingEndOfList.toUint16();
				container->getSlot() = prevThing = thing;
			} else {
				dunMan.linkThingToList(thing, prevThing, kMapXNotOnASquare, 0);
				prevThing = thing;
			}
		}
	}
}

}
