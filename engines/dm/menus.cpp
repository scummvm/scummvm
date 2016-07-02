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

Box  g499_BoxActionArea3ActionMenu = Box(224, 319, 77, 121); // @ G0499_s_Graphic560_Box_ActionArea3ActionsMenu
Box  g500_BoxActionArea2ActionMenu = Box(224, 319, 77, 109); // @ G0500_s_Graphic560_Box_ActionArea2ActionsMenu
Box  g501_BoxActionArea1ActionMenu = Box(224, 319, 77, 97); // @ G0501_s_Graphic560_Box_ActionArea1ActionMenu
Box g1_BoxActionArea = Box(224, 319, 77, 121); // @ G0001_s_Graphic562_Box_ActionArea 
byte g498_PalChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

Box gK74_BoxSpellAreaLine = Box(0, 95, 0, 11); // @ K0074_s_Box_SpellAreaLine 
Box gK75_BoxSpellAreaLine2 = Box(224, 319, 50, 61); // @ K0075_s_Box_SpellAreaLine2 
Box gK76_BoxSpellAreaLine3 = Box(224, 319, 62, 73); // @ K0076_s_Box_SpellAreaLine3 
Box g0_BoxSpellArea = Box(224, 319, 77, 121);

MenuMan::MenuMan(DMEngine *vm) : _vm(vm) {
	_g508_refreshActionArea = false;
	_g509_actionAreaContainsIcons = false;
	_g513_actionDamage = 0;
	_gK72_bitmapSpellAreaLine = new byte[96 * 12];
}

MenuMan::~MenuMan() {
	delete[] _gK72_bitmapSpellAreaLine;
}

void MenuMan::f395_drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.f489_getBitmap(k13_MovementArrowsIndice);
	Box &dest = g2_BoxMovementArrows;
	uint16 w = disp.getWidth(k13_MovementArrowsIndice);

	disp.f132_blitToBitmap(arrowsBitmap, disp._g348_bitmapScreen, dest, 0, 0, w, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
}
void MenuMan::f388_clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._g506_actingChampionOrdinal) {
		cm._g506_actingChampionOrdinal--;
		cm._gK71_champions[cm._g506_actingChampionOrdinal].setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
		cm.f292_drawChampionState((ChampionIndex)cm._g506_actingChampionOrdinal);
		cm._g506_actingChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		_g508_refreshActionArea = true;
	}
}

void MenuMan::f386_drawActionIcon(ChampionIndex championIndex) {
	if (!_g509_actionAreaContainsIcons)
		return;
	DisplayMan &dm = *_vm->_displayMan;
	Champion &champion = _vm->_championMan->_gK71_champions[championIndex];

	Box box;
	box._x1 = championIndex * 22 + 233;
	box._x2 = box._x1 + 19 + 1;
	box._y1 = 86;
	box._y2 = 120 + 1;
	dm._g578_useByteBoxCoordinates = false;
	if (!champion._currHealth) {
		dm.D24_fillScreenBox(box, k0_ColorBlack);
		return;
	}
	byte *bitmapIcon = dm._g74_tmpBitmap;
	Thing thing = champion.getSlot(k1_ChampionSlotActionHand);
	IconIndice iconIndex;
	if (thing == Thing::_none) {
		iconIndex = k201_IconIndiceActionEmptyHand;
	} else if (g237_ObjectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing);
	} else {
		dm.f134_fillBitmap(bitmapIcon, 16, 16, k4_ColorCyan);
		goto T0386006;
	}
	_vm->_objectMan->f36_extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.f129_blitToBitmapShrinkWithPalChange(bitmapIcon, 16, 16, bitmapIcon, 16, 16, g498_PalChangesActionAreaObjectIcon);
T0386006:
	dm.D24_fillScreenBox(box, k4_ColorCyan);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2; // no need to add +1 for exclusive boundaries, box already has that
	box2._y1 = 95;
	box2._y2 = 110 + 1;
	dm.f132_blitToBitmap(bitmapIcon, dm._g348_bitmapScreen, box2, 0, 0, 16, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
	if (champion.getAttributes(k0x0008_ChampionAttributeDisableAction) || _vm->_championMan->_g299_candidateChampionOrdinal || _vm->_championMan->_g300_partyIsSleeping) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}
}

void MenuMan::f456_drawDisabledMenu() {
	if (!_vm->_championMan->_g300_partyIsSleeping) {
		warning("MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
			if (_vm->_inventoryMan->_g424_panelContent == k4_PanelContentChest) {
				_vm->_inventoryMan->f334_closeChest();
			}
		} else {
			warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		}
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning("MISSING CODE: F0067_MOUSE_SetPointerToNormal");
	}
}

void MenuMan::f390_refreshActionAreaAndSetChampDirMaxDamageReceived() {
	ChampionMan &champMan = *_vm->_championMan;

	if (!champMan._g305_partyChampionCount)
		return;

	Champion *champ = nullptr;
	if (champMan._g300_partyIsSleeping || champMan._g299_candidateChampionOrdinal) {
		if (champMan._g506_actingChampionOrdinal) {
			f388_clearActingChampion();
			return;
		}
		if (!champMan._g299_candidateChampionOrdinal)
			return;
	} else {
		champ = champMan._gK71_champions;
		int16 champIndex = k0_ChampionFirst;

		do {
			if ((champIndex != champMan._g411_leaderIndex)
				&& (_vm->M0_indexToOrdinal(champIndex) != champMan._g506_actingChampionOrdinal)
				&& (champ->_maximumDamageReceived)
				&& (champ->_dir != champ->_directionMaximumDamageReceived)) {

				champ->_dir = (direction)champ->_directionMaximumDamageReceived;
				champ->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
				champMan.f292_drawChampionState((ChampionIndex)champIndex);
			}
			champ->_maximumDamageReceived = 0;
			champ++;
			champIndex++;
		} while (champIndex < champMan._g305_partyChampionCount);
	}

	if (_g508_refreshActionArea) {
		if (!champMan._g506_actingChampionOrdinal) {
			if (_g513_actionDamage) {
				warning("MISSING CODE: F0385_MENUS_DrawActionDamage");
				_g513_actionDamage = 0;
			} else {
				_g509_actionAreaContainsIcons = true;
				f387_drawActionArea();
			}
		} else {
			_g509_actionAreaContainsIcons = false;
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			champMan.f292_drawChampionState((ChampionIndex)_vm->M1_ordinalToIndex(champMan._g506_actingChampionOrdinal));
			warning("MISSING CODE: F0387_MENUS_DrawActionArea");
		}
	}
}

#define k7_ChampionNameMaximumLength 7 // @ C007_CHAMPION_NAME_MAXIMUM_LENGTH
#define k12_ActionNameMaximumLength 12 // @ C012_ACTION_NAME_MAXIMUM_LENGTH

void MenuMan::f387_drawActionArea() {
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	dispMan._g578_useByteBoxCoordinates = false;
	dispMan.D24_fillScreenBox(g1_BoxActionArea, k0_ColorBlack);
	if (_g509_actionAreaContainsIcons) {
		for (uint16 champIndex = k0_ChampionFirst; champIndex < champMan._g305_partyChampionCount; ++champIndex)
			f386_drawActionIcon((ChampionIndex)champIndex);
	} else if (champMan._g506_actingChampionOrdinal) {
		Box box = g499_BoxActionArea3ActionMenu;
		if (_g713_actionList._actionIndices[2] == k255_ChampionActionNone)
			box = g500_BoxActionArea2ActionMenu;
		if (_g713_actionList._actionIndices[1] == k255_ChampionActionNone)
			box = g501_BoxActionArea1ActionMenu;
		dispMan.f132_blitToBitmap(dispMan.f489_getBitmap(k10_MenuActionAreaIndice), dispMan._g348_bitmapScreen, box, 0, 0, 96, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
		textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen * 2, 235, 83, k0_ColorBlack, k4_ColorCyan, champMan._gK71_champions[_vm->M1_ordinalToIndex(champMan._g506_actingChampionOrdinal)]._name,
											k7_ChampionNameMaximumLength, k200_heightScreen);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen * 2, 241, 93 + actionListIndex * 12, k4_ColorCyan, k0_ColorBlack,
												f384_getActionName(_g713_actionList._actionIndices[actionListIndex]),
												k12_ActionNameMaximumLength, k200_heightScreen);
		}
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	_g508_refreshActionArea = false;
}

const char *g490_ChampionActionNames[44] = { // @ G0490_ac_Graphic560_ActionNames
	"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH",
	"KICK", "WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE",
	"HIT", "SWING", "STAB", "THRUST", "JAB", "PARRY", "HACK",
	"BERZERK", "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING",
	"DISRUPT", "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
	"BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
	"FLUXCAGE", "HEAL", "CALM", "LIGHT", "WINDOW", "SPIT",
	"BRANDISH", "THROW", "FUSE"};

const char* MenuMan::f384_getActionName(ChampionAction actionIndex) {
	return (actionIndex == k255_ChampionActionNone) ? "" : g490_ChampionActionNames[actionIndex];
}


Box g504_BoxSpellAreaControls = Box(233, 319, 42, 49); // @ G0504_s_Graphic560_Box_SpellAreaControls 

void MenuMan::f393_drawSpellAreaControls(ChampionIndex champIndex) {
	ChampionMan &champMan = *_vm->_championMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	TextMan &textMan = *_vm->_textMan;

	Champion &champ = champMan._gK71_champions[champIndex];
	int16 champCurrHealth[4];
	for (uint16 i = 0; i < 4; ++i)
		champCurrHealth[i] = champMan._gK71_champions[i]._currHealth;
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	dispMan.D24_fillScreenBox(g504_BoxSpellAreaControls, k0_ColorBlack);
	int16 champCount = champMan._g305_partyChampionCount;
	switch (champIndex) {
	case k0_ChampionFirst:
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(235, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
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
		textMan.f53_printToLogicalScreen(249, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		goto labelChamp2;
	case k2_ChampionThird:
		if (champCurrHealth[0]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning("MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(263, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
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
		textMan.f53_printToLogicalScreen(277, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		break;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

#define k2_SpellAreaAvailableSymbols 2 // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
#define k3_SpellAreaChampionSymbols 3 // @ C3_SPELL_AREA_CHAMPION_SYMBOLS

void MenuMan::f392_buildSpellAreaLine(int16 spellAreaBitmapLine) {
	DisplayMan &dispMan = *_vm->_displayMan;

	Champion &champ = _vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
	if (spellAreaBitmapLine == k2_SpellAreaAvailableSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.f132_blitToBitmap(dispMan.f489_getBitmap(k11_MenuSpellAreLinesIndice), _gK72_bitmapSpellAreaLine, gK74_BoxSpellAreaLine, 0, 12, 96, 96, k255_ColorNoTransparency);
		int16 x = 1;
		byte c = 96 + (6 * champ._symbolStep);
		char spellSymbolString[2] = {'\0', '\0'};
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			spellSymbolString[0] = c++;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 96, x += 14, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == k3_SpellAreaChampionSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.f132_blitToBitmap(dispMan.f489_getBitmap(k11_MenuSpellAreLinesIndice), _gK72_bitmapSpellAreaLine, gK74_BoxSpellAreaLine, 0, 24, 96, 96, k255_ColorNoTransparency);
		char spellSymbolString[2] = {'\0', '\0'};
		int16 x = 8;
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((spellSymbolString[0] = champ._symbols[symbolIndex]) == '\0')
				break;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 96, x += 9, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	}
}

void MenuMan::f394_setMagicCasterAndDrawSpellArea(int16 champIndex) {
	ChampionMan &champMan = *_vm->_championMan;
	DisplayMan &dispMan = *_vm->_displayMan;

	if ((champIndex == champMan._g514_magicCasterChampionIndex)
		|| ((champIndex != kM1_ChampionNone) && !champMan._gK71_champions[champIndex]._currHealth))
		return;
	if (champMan._g514_magicCasterChampionIndex == kM1_ChampionNone) {
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan.f132_blitToBitmap(dispMan.f489_getBitmap(k9_MenuSpellAreaBackground), dispMan._g348_bitmapScreen, g0_BoxSpellArea, 0, 0, 96, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	}
	if (champIndex == kM1_ChampionNone) {
		champMan._g514_magicCasterChampionIndex = kM1_ChampionNone;
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.D24_fillScreenBox(g0_BoxSpellArea, k0_ColorBlack);
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
		return;
	}

	champMan._g514_magicCasterChampionIndex = (ChampionIndex)champIndex;
	f392_buildSpellAreaLine(k2_SpellAreaAvailableSymbols);
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	f393_drawSpellAreaControls((ChampionIndex)champIndex);
	dispMan.f132_blitToBitmap(_gK72_bitmapSpellAreaLine, dispMan._g348_bitmapScreen, gK75_BoxSpellAreaLine2, 0, 0, 96, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
	f392_buildSpellAreaLine(k3_SpellAreaChampionSymbols);
	dispMan.f132_blitToBitmap(_gK72_bitmapSpellAreaLine, dispMan._g348_bitmapScreen, gK76_BoxSpellAreaLine3, 0, 0, 96, k160_byteWidthScreen * 2, k255_ColorNoTransparency);
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}
}
