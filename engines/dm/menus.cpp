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
	byte *arrowsBitmap = disp.getBitmap(k13_MovementArrowsIndice);
	Box &dest = g2_BoxMovementArrows;
	uint16 w = disp.getWidth(k13_MovementArrowsIndice);

	disp.blitToScreen(arrowsBitmap, w, 0, 0, dest._x1, dest._x2, dest._y1, dest._y2, k255_ColorNoTransparency);
}
void MenuMan::clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._g506_actingChampionOrdinal) {
		cm._g506_actingChampionOrdinal--;
		cm._champions[cm._g506_actingChampionOrdinal].setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
		cm.drawChampionState((ChampionIndex)cm._g506_actingChampionOrdinal);
		cm._g506_actingChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
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
	dm._g578_useByteBoxCoordinates = false;
	if (!champion._currHealth) {
		dm.clearScreenBox(k0_ColorBlack, box);
		return;
	}
	byte *bitmapIcon = dm._g74_tmpBitmap;
	Thing thing = champion.getSlot(k1_ChampionSlotActionHand);
	IconIndice iconIndex;
	if (thing == Thing::_none) {
		iconIndex = k201_IconIndiceActionEmptyHand;
	} else if (gObjectInfo[_vm->_dungeonMan->getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->getIconIndex(thing);
	} else {
		dm.clearBitmap(bitmapIcon, 16, 16, k4_ColorCyan);
		goto T0386006;
	}
	_vm->_objectMan->extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.blitToBitmapShrinkWithPalChange(bitmapIcon, 16, 16, bitmapIcon, 16, 16, gPalChangesActionAreaObjectIcon);
T0386006:
	dm.clearScreenBox(k4_ColorCyan, box);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2; // no need to add +1 for exclusive boundaries, box already has that
	box2._y1 = 95;
	box2._y2 = 110 + 1;
	dm.blitToScreen(bitmapIcon, 16, 0, 0, box2._x1, box2._x2, box2._y1, box2._y2);
	if (champion.getAttributes(k0x0008_ChampionAttributeDisableAction) || _vm->_championMan->_g299_candidateChampionOrdinal || _vm->_championMan->_g300_partyIsSleeping) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}
}

void MenuMan::drawDisabledMenu() {
	if (!_vm->_championMan->_g300_partyIsSleeping) {
		warning("MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_inventoryChampionOrdinal) {
			if (_vm->_inventoryMan->_panelContent == kPanelContentChest) {
				_vm->_inventoryMan->closeChest();
			}
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

	if (!champMan._g305_partyChampionCount)
		return;

	Champion *champ = nullptr;
	if (champMan._g300_partyIsSleeping || champMan._g299_candidateChampionOrdinal) {
		if (champMan._g506_actingChampionOrdinal) {
			clearActingChampion();
			return;
		}
		if (!champMan._g299_candidateChampionOrdinal)
			return;
	} else {
		champ = champMan._champions;
		int16 champIndex = k0_ChampionFirst;

		do {
			if ((champIndex != champMan._g411_leaderIndex)
				&& (_vm->indexToOrdinal(champIndex) != champMan._g506_actingChampionOrdinal)
				&& (champ->_maximumDamageReceived)
				&& (champ->_dir != champ->_directionMaximumDamageReceived)) {

				champ->_dir = (direction)champ->_directionMaximumDamageReceived;
				champ->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
				champMan.drawChampionState((ChampionIndex)champIndex);
			}
			champ->_maximumDamageReceived = 0;
			champ++;
			champIndex++;
		} while (champIndex < champMan._g305_partyChampionCount);
	}

	if (_refreshActionArea) {
		if (!champMan._g506_actingChampionOrdinal) {
			if (_actionDamage) {
				warning("MISSING CODE: F0385_MENUS_DrawActionDamage");
				_actionDamage = 0;
			} else {
				_actionAreaContainsIcons = true;
				drawActionArea();
			}
		} else {
			_actionAreaContainsIcons = false;
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			champMan.drawChampionState((ChampionIndex)_vm->ordinalToIndex(champMan._g506_actingChampionOrdinal));
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
	dispMan._g578_useByteBoxCoordinates = false;
	dispMan.clearScreenBox(k0_ColorBlack, gBoxActionArea);
	if (_actionAreaContainsIcons) {
		for (uint16 champIndex = k0_ChampionFirst; champIndex < champMan._g305_partyChampionCount; ++champIndex)
			drawActionIcon((ChampionIndex)champIndex);
	} else if (champMan._g506_actingChampionOrdinal) {
		Box box = gBoxActionArea3ActionMenu;
		if (_actionList._actionIndices[2] == k255_ChampionActionNone)
			box = gBoxActionArea2ActionMenu;
		if (_actionList._actionIndices[1] == k255_ChampionActionNone)
			box = gBoxActionArea1ActionMenu;
		dispMan.blitToScreen(dispMan.getBitmap(k10_MenuActionAreaIndice), 96, 0, 0, box, k255_ColorNoTransparency);
		textMan.printWithTrailingSpacesToScreen(235, 83, k0_ColorBlack, k4_ColorCyan, champMan._champions[_vm->ordinalToIndex(champMan._g506_actingChampionOrdinal)]._name,
												kChampionNameMaximumLength);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.printWithTrailingSpacesToScreen(241, 93 + actionListIndex * 12, k4_ColorCyan, k0_ColorBlack,
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
	return (actionIndex == k255_ChampionActionNone) ? "" : gChampionActionNames[actionIndex];
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
	dispMan.clearScreenBox(k0_ColorBlack, gBoxSpellAreaControls);
	int16 champCount = champMan._g305_partyChampionCount;
	switch (champIndex) {
	case k0_ChampionFirst:
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(235, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
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
	case k1_ChampionSecond:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(249, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		goto labelChamp2;
	case k2_ChampionThird:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.printTextToScreen(263, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		goto labelChamp3;
	case k3_ChampionFourth:
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
		textMan.printTextToScreen(277, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		break;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

#define kSpellAreaAvailableSymbols 2 // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
#define kSpellAreaChampionSymbols 3 // @ C3_SPELL_AREA_CHAMPION_SYMBOLS

void MenuMan::buildSpellAreaLine(int16 spellAreaBitmapLine) {
	DisplayMan &dispMan = *_vm->_displayMan;

	Champion &champ = _vm->_championMan->_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
	if (spellAreaBitmapLine == kSpellAreaAvailableSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.blitToBitmap(dispMan.getBitmap(k11_MenuSpellAreLinesIndice), 96, 0, 12, _bitmapSpellAreaLine, 96, gBoxSpellAreaLine, k255_ColorNoTransparency);
		int16 x = 1;
		byte c = 96 + (6 * champ._symbolStep);
		char spellSymbolString[2] = {'\0', '\0'};
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			spellSymbolString[0] = c++;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 96, x += 14, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == kSpellAreaChampionSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.blitToBitmap(dispMan.getBitmap(k11_MenuSpellAreLinesIndice), 96, 0, 24, _bitmapSpellAreaLine, 96, gBoxSpellAreaLine, k255_ColorNoTransparency);
		char spellSymbolString[2] = {'\0', '\0'};
		int16 x = 8;
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((spellSymbolString[0] = champ._symbols[symbolIndex]) == '\0')
				break;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 96, x += 9, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	}
}

void MenuMan::setMagicCasterAndDrawSpellArea(int16 champIndex) {
	ChampionMan &champMan = *_vm->_championMan;
	DisplayMan &dispMan = *_vm->_displayMan;

	if ((champIndex == champMan._g514_magicCasterChampionIndex)
		|| ((champIndex != kM1_ChampionNone) && !champMan._champions[champIndex]._currHealth))
		return;
	if (champMan._g514_magicCasterChampionIndex == kM1_ChampionNone) {
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan.blitToScreen(dispMan.getBitmap(k9_MenuSpellAreaBackground), 96, 0, 0, gBoxSpellArea);
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	}
	if (champIndex == kM1_ChampionNone) {
		champMan._g514_magicCasterChampionIndex = kM1_ChampionNone;
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.clearScreenBox(k0_ColorBlack, gBoxSpellArea);
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
		return;
	}

	champMan._g514_magicCasterChampionIndex = (ChampionIndex)champIndex;
	buildSpellAreaLine(kSpellAreaAvailableSymbols);
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	drawSpellAreaControls((ChampionIndex)champIndex);
	dispMan.blitToScreen(_bitmapSpellAreaLine, 96, 0, 0, gBoxSpellAreaLine2);
	buildSpellAreaLine(kSpellAreaChampionSymbols);
	dispMan.blitToScreen(_bitmapSpellAreaLine, 96, 0, 0, gBoxSpellAreaLine3);
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}
}
