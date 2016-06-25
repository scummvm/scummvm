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
#include "text.h"


namespace DM {

Box  gBoxActionArea3ActionMenu = Box(224, 319, 77, 121); // @ G0499_s_Graphic560_Box_ActionArea3ActionsMenu
Box  gBoxActionArea2ActionMenu = Box(224, 319, 77, 109); // @ G0500_s_Graphic560_Box_ActionArea2ActionsMenu
Box  gBoxActionArea1ActionMenu = Box(224, 319, 77, 97); // @ G0501_s_Graphic560_Box_ActionArea1ActionMenu
Box gBoxActionArea = Box(224, 319, 77, 121); // @ G0001_s_Graphic562_Box_ActionArea 
byte gPalChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

Box gBoxSpellAreaLine = Box(0, 95, 0, 11); // @ K0074_s_Box_SpellAreaLine 
Box gBoxSpellAreaLine2 = Box(224, 319, 50, 61); // @ K0075_s_Box_SpellAreaLine2 
Box gBoxSpellAreaLine3 = Box(224, 319, 62, 73); // @ K0076_s_Box_SpellAreaLine3 
Box gBoxSpellArea = Box(224, 319, 77, 121); 

MenuMan::MenuMan(DMEngine *vm) : _vm(vm) {
	_refreshActionArea = false;
	_actionAreaContainsIcons = false;
	_actionDamage = 0;
	_bitmapSpellAreaLine = new byte[96 * 12];
}

MenuMan::~MenuMan() {
	delete[] _bitmapSpellAreaLine;
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
				drawActionArea();
			}
		} else {
			_actionAreaContainsIcons = false;
			champ->setAttributeFlag(kChampionAttributeActionHand, true);
			champMan.drawChampionState((ChampionIndex)_vm->ordinalToIndex(champMan._actingChampionOrdinal));
			warning("MISSING CODE: F0387_MENUS_DrawActionArea");
		}
	}
}

#define kChampionNameMaximumLength 7 // @ C007_CHAMPION_NAME_MAXIMUM_LENGTH
#define kActionNameMaximumLength 12 // @ C012_ACTION_NAME_MAXIMUM_LENGTH

void MenuMan::drawActionArea() {
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	dispMan._useByteBoxCoordinates = false;
	dispMan.clearScreenBox(kColorBlack, gBoxActionArea);
	if (_actionAreaContainsIcons) {
		for (uint16 champIndex = kChampionFirst; champIndex < champMan._partyChampionCount; ++champIndex)
			drawActionIcon((ChampionIndex)champIndex);
	} else if (champMan._actingChampionOrdinal) {
		Box box = gBoxActionArea3ActionMenu;
		if (_actionList._actionIndices[2] == kChampionActionNone)
			box = gBoxActionArea2ActionMenu;
		if (_actionList._actionIndices[1] == kChampionActionNone)
			box = gBoxActionArea1ActionMenu;
		dispMan.blitToScreen(dispMan.getBitmap(kMenuActionAreaIndice), 96, 0, 0, box, kColorNoTransparency);
		textMan.printWithTrailingSpacesToScreen(235, 83, kColorBlack, kColorCyan, champMan._champions[_vm->ordinalToIndex(champMan._actingChampionOrdinal)]._name,
												kChampionNameMaximumLength);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.printWithTrailingSpacesToScreen(241, 93 + actionListIndex * 12, kColorCyan, kColorBlack,
													getActionName(_actionList._actionIndices[actionListIndex]),
													kActionNameMaximumLength);
		}
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	_refreshActionArea = false;
}

const char *gChampionActionNames[44] = {
	"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH",
	"KICK", "WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE",
	"HIT", "SWING", "STAB", "THRUST", "JAB", "PARRY", "HACK",
	"BERZERK", "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING",
	"DISRUPT", "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
	"BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
	"FLUXCAGE", "HEAL", "CALM", "LIGHT", "WINDOW", "SPIT",
	"BRANDISH", "THROW", "FUSE"};

const char* MenuMan::getActionName(ChampionAction actionIndex) {
	return (actionIndex == kChampionActionNone) ? "" : gChampionActionNames[actionIndex];
}


Box gBoxSpellAreaControls = Box(233, 319, 42, 49); // @ G0504_s_Graphic560_Box_SpellAreaControls 

void MenuMan::drawSpellAreaControls(ChampionIndex champIndex) {
	ChampionMan &champMan = *_vm->_championMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	TextMan &textMan = *_vm->_textMan;

	Champion &champ = champMan._champions[champIndex];
	int16 champCurrHealth[4];
	for (uint16 i = 0; i < 4; ++i)
		champCurrHealth[i] = champMan._champions[i]._currHealth;
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	dispMan.clearScreenBox(kColorBlack, gBoxSpellAreaControls);
	int16 champCount = champMan._partyChampionCount;
	switch (champIndex) {
	case kChampionFirst:
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(235, 48, kColorBlack, kColorCyan, champ._name);
		if (champCount) {
			if (champCurrHealth[1]) {
				warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
			}
labelChamp2:
			if (champCount > 2) {
				if (champCurrHealth[2]) {
					warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
				}
labelChamp3:
				if (champCount > 3) {
					if (champCurrHealth[3]) {
						warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
					}
				}
			}
		}
		break;
	case kChampionSecond:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(249, 48, kColorBlack, kColorCyan, champ._name);
		goto labelChamp2;
	case kChampionThird:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(263, 48, kColorBlack, kColorCyan, champ._name);
		goto labelChamp3;
	case kChampionFourth:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[2]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(277, 48, kColorBlack, kColorCyan, champ._name);
		break;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

#define kSpellAreaAvailableSymbols 2 // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
#define kSpellAreaChampionSymbols 3 // @ C3_SPELL_AREA_CHAMPION_SYMBOLS

void MenuMan::buildSpellAreaLine(int16 spellAreaBitmapLine) {
	DisplayMan &dispMan = *_vm->_displayMan;

	Champion &champ = _vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	if (spellAreaBitmapLine == kSpellAreaAvailableSymbols) {
		dispMan._useByteBoxCoordinates = false;
		dispMan.blitToBitmap(dispMan.getBitmap(kMenuSpellAreLinesIndice), 96, 0, 12, _bitmapSpellAreaLine, 96, gBoxSpellAreaLine, kColorNoTransparency);
		int16 x = 1;
		char c = 96 + (6 * champ._symbolStep);
		char spellSymbolString[2] = {'\0', '\0'};
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			spellSymbolString[0] = c++;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 96, x += 14, 8, kColorCyan, kColorBlack, spellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == kSpellAreaChampionSymbols) {
		dispMan._useByteBoxCoordinates = false;
		dispMan.blitToBitmap(dispMan.getBitmap(kMenuSpellAreLinesIndice), 96, 0, 24, _bitmapSpellAreaLine, 96, gBoxSpellAreaLine, kColorNoTransparency);
		int16 x = 8;
		char spellSymbolString[2] = {'\0', '\0'};
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((spellSymbolString[0] = champ._symbols[symbolIndex]) == '\0')
				break;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 96, x += 9, 8, kColorCyan, kColorBlack, spellSymbolString, 12);
		}
	}
}

	void MenuMan::setMagicCasterAndDrawSpellArea(int16 champIndex)
	{
		ChampionMan &champMan = *_vm->_championMan;
		DisplayMan &dispMan = *_vm->_displayMan;
		
		if((champIndex == champMan._magicCasterChampionIndex) 
		   || ((champIndex != kChampionNone) && !champMan._champions[champIndex]._currHealth))
			return;
		if (champMan._magicCasterChampionIndex == kChampionNone) {
			warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			dispMan.blitToScreen(dispMan.getBitmap(kMenuSpellAreaBackground), 96, 0, 0, gBoxSpellArea);
			warning("MISSING CODE: F0078_MOUSE_ShowPointer");
		}
		if (champIndex == kChampionNone) {
			champMan._magicCasterChampionIndex = kChampionNone;
			warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			dispMan._useByteBoxCoordinates = false;
			dispMan.clearScreenBox(kColorBlack, gBoxSpellArea);
			warning("MISSING CODE: F0078_MOUSE_ShowPointer");
			return;
		}

		champMan._magicCasterChampionIndex = champIndex;
		buildSpellAreaLine(kSpellAreaAvailableSymbols);
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		drawSpellAreaControls((ChampionIndex)champIndex);
		dispMan.blitToScreen(_bitmapSpellAreaLine, 96, 0, 0, gBoxSpellAreaLine2);
		buildSpellAreaLine(kSpellAreaChampionSymbols);
		dispMan.blitToScreen(_bitmapSpellAreaLine, 96, 0, 0, gBoxSpellAreaLine3);
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	}
}
