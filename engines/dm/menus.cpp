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

#include "menus.h"
#include "gfx.h"
#include "champion.h"
#include "dungeonman.h"
#include "objectman.h"
#include "inventory.h"


namespace DM {

byte gPalChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

MenuMan::MenuMan(DMEngine *vm) : _vm(vm) {
	_refreshActionArea = false;
	_actionAreaContainsIcons = false;
	_actionDamage = 0;
}

void MenuMan::drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.getBitmap(kMovementArrowsIndice);
	Box &dest = gBoxMovementArrows;
	uint16 w = disp.getWidth(kMovementArrowsIndice);

	disp.blitToScreen(arrowsBitmap, w, 0, 0, dest._x1, dest._x2, dest._y1, dest._y2, kColorNoTransparency);
}
void MenuMan::clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._actingChampionOrdinal) {
		cm._actingChampionOrdinal--;
		cm._champions[cm._actingChampionOrdinal].setAttributeFlag(kChampionAttributeActionHand, true);
		cm.drawChampionState((ChampionIndex)cm._actingChampionOrdinal);
		cm._actingChampionOrdinal = _vm->indexToOrdinal(kChampionNone);
		_refreshActionArea = true;
	}
}

void MenuMan::drawActionIcon(ChampionIndex championIndex) {
	if (!_actionAreaContainsIcons)
		return;
	DisplayMan &dm = *_vm->_displayMan;
	Champion &champion = _vm->_championMan->_champions[championIndex];

	Box box;
	box._x1 = championIndex * 22 + 233;
	box._x2 = box._x1 + 19 + 1;
	box._y1 = 86;
	box._y2 = 120 + 1;
	dm._useByteBoxCoordinates = false;
	if (!champion._currHealth) {
		dm.clearScreenBox(kColorBlack, box);
		return;
	}
	byte *bitmapIcon = dm._tmpBitmap;
	Thing thing = champion.getSlot(kChampionSlotActionHand);
	IconIndice iconIndex;
	if (thing == Thing::_thingNone) {
		iconIndex = kIconIndiceActionEmptyHand;
	} else if (gObjectInfo[_vm->_dungeonMan->getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->getIconIndex(thing);
	} else {
		dm.clearBitmap(bitmapIcon, 16, 16, kColorCyan);
		goto T0386006;
	}
	_vm->_objectMan->extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.blitToBitmapShrinkWithPalChange(bitmapIcon, 16, 16, bitmapIcon, 16, 16, gPalChangesActionAreaObjectIcon);
T0386006:
	dm.clearScreenBox(kColorCyan, box);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2; // no need to add +1 for exclusive boundaries, box already has that
	box2._y1 = 95;
	box2._y2 = 110 + 1;
	dm.blitToScreen(bitmapIcon, 16, 0, 0, box2._x1, box2._x2, box2._y1, box2._y2);
	if (champion.getAttributes(kChampionAttributeDisableAction) || _vm->_championMan->_candidateChampionOrdinal || _vm->_championMan->_partyIsSleeping) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}
}

void MenuMan::drawDisabledMenu() {
	if (!_vm->_championMan->_partyIsSleeping) {
		warning("MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
		_vm->_displayMan->_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_inventoryChampionOrdinal) {
			warning("MISSING CODE: F0334_INVENTORY_CloseChest");
		} else {
			warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		}
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning("MISSING CODE: F0067_MOUSE_SetPointerToNormal");
	}
}

void MenuMan::refreshActionAreaAndSetChampDirMaxDamageReceived() {
	ChampionMan &champMan = *_vm->_championMan;

	if (!champMan._partyChampionCount)
		return;

	Champion *champ = nullptr;
	if (champMan._partyIsSleeping || champMan._candidateChampionOrdinal) {
		if (champMan._actingChampionOrdinal) {
			clearActingChampion();
			return;
		}
		if (!champMan._candidateChampionOrdinal)
			return;
	} else {
		champ = champMan._champions;
		int16 champIndex = kChampionFirst;

		do {
			if ((champIndex != champMan._leaderIndex)
				&& (_vm->indexToOrdinal(champIndex) != champMan._actingChampionOrdinal)
				&& (champ->_maximumDamageReceived)
				&& (champ->_dir != champ->_directionMaximumDamageReceived)) {

				champ->_dir = (direction)champ->_directionMaximumDamageReceived;
				champ->setAttributeFlag(kChampionAttributeIcon, true);
				champMan.drawChampionState((ChampionIndex)champIndex);
			}
			champ->_maximumDamageReceived = 0;
			champ++;
			champIndex++;
		} while (champIndex < champMan._partyChampionCount);
	}

	if (_refreshActionArea) {
		if (!champMan._actingChampionOrdinal) {
			if (_actionDamage) {
				warning("MISSING CODE: F0385_MENUS_DrawActionDamage");
				_actionDamage = 0;
			} else {
				_actionAreaContainsIcons = true;
				warning("MISSING CODE: F0387_MENUS_DrawActionArea");
			}
		} else {
			_actionAreaContainsIcons = false;
			champ->setAttributeFlag(kChampionAttributeActionHand, true);
			champMan.drawChampionState((ChampionIndex)_vm->ordinalToIndex(champMan._actingChampionOrdinal));
			warning("MISSING CODE: F0387_MENUS_DrawActionArea");
		}
	}
}
}
