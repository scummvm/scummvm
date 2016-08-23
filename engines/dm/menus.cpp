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
#include "eventman.h"
#include "timeline.h"
#include "movesens.h"
#include "group.h"
#include "projexpl.h"
#include "sounds.h"


namespace DM {

void MenuMan::initConstants() {
	static unsigned char actionSkillIndex[44] = { // @ G0496_auc_Graphic560_ActionSkillIndex
		0,  /* N */
		7,  /* BLOCK */
		6,  /* CHOP */
		0,  /* X */
		14, /* BLOW HORN */
		12, /* FLIP */
		9,  /* PUNCH */
		9,  /* KICK */
		7,  /* WAR CRY Atari ST Versions 1.0 1987-12-08 1987-12-11 1.1: 14 */
		9,  /* STAB */
		8,  /* CLIMB DOWN */
		14, /* FREEZE LIFE */
		9,  /* HIT */
		4,  /* SWING */
		5,  /* STAB */
		5,  /* THRUST */
		5,  /* JAB */
		7,  /* PARRY */
		4,  /* HACK */
		4,  /* BERZERK */
		16, /* FIREBALL */
		17, /* DISPELL */
		14, /* CONFUSE */
		17, /* LIGHTNING */
		17, /* DISRUPT */
		6,  /* MELEE */
		8,  /* X */
		3,  /* INVOKE */
		4,  /* SLASH */
		4,  /* CLEAVE */
		6,  /* BASH */
		6,  /* STUN */
		11, /* SHOOT */
		15, /* SPELLSHIELD */
		15, /* FIRESHIELD */
		3,  /* FLUXCAGE */
		13, /* HEAL */
		14, /* CALM */
		17, /* LIGHT */
		18, /* WINDOW */
		16, /* SPIT */
		14, /* BRANDISH */
		10, /* THROW */
		3 /* FUSE */
	};

	boxActionArea1ActionMenu = Box(224, 319, 77, 97); // @ G0501_s_Graphic560_Box_ActionArea1ActionMenu
	boxActionArea2ActionMenu = Box(224, 319, 77, 109); // @ G0500_s_Graphic560_Box_ActionArea2ActionsMenu
	boxActionArea3ActionMenu = Box(224, 319, 77, 121); // @ G0499_s_Graphic560_Box_ActionArea3ActionsMenu
	boxActionArea = Box(224, 319, 77, 121); // @ G0001_s_Graphic562_Box_ActionArea 
	boxSpellArea = Box(224, 319, 42, 74);

	for (int i = 0; i < 40; i++)
		g496_ActionSkillIndex[i] = actionSkillIndex[i];
}

MenuMan::MenuMan(DMEngine *vm) : _vm(vm) {
	_g508_refreshActionArea = false;
	_g509_actionAreaContainsIcons = false;
	_g513_actionDamage = 0;
	_g713_actionList.resetToZero();
	_gK72_bitmapSpellAreaLine = new byte[96 * 12];
	_gK73_bitmapSpellAreaLines = new byte[3 * 96 * 12];
	_g517_actionTargetGroupThing = Thing(0);
	_g507_actionCount = 0;

	initConstants();
}

MenuMan::~MenuMan() {
	delete[] _gK72_bitmapSpellAreaLine;
}

void MenuMan::f395_drawMovementArrows() {
	_vm->_eventMan->f78_showMouse();
	_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k13_MovementArrowsIndice),
									   &_vm->_displayMan->_boxMovementArrows, k48_byteWidth, kM1_ColorNoTransparency, 45);
	_vm->_eventMan->f77_hideMouse();
}
void MenuMan::f388_clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._actingChampionOrdinal) {
		cm._actingChampionOrdinal--;
		cm._champions[cm._actingChampionOrdinal].setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
		cm.drawChampionState((ChampionIndex)cm._actingChampionOrdinal);
		cm._actingChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		_g508_refreshActionArea = true;
	}
}

void MenuMan::f386_drawActionIcon(ChampionIndex championIndex) {
	static byte palChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

	if (!_g509_actionAreaContainsIcons)
		return;
	DisplayMan &dm = *_vm->_displayMan;
	Champion &champion = _vm->_championMan->_champions[championIndex];

	Box box;
	box._x1 = championIndex * 22 + 233;
	box._x2 = box._x1 + 19;
	box._y1 = 86;
	box._y2 = 120;
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
	} else if (_vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing);
	} else {
		dm.f134_fillBitmap(bitmapIcon, k4_ColorCyan, 16, 16);
		goto T0386006;
	}
	_vm->_objectMan->f36_extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.f129_blitToBitmapShrinkWithPalChange(bitmapIcon, bitmapIcon, 16, 16, 16, 16, palChangesActionAreaObjectIcon);
T0386006:
	dm.D24_fillScreenBox(box, k4_ColorCyan);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2;
	box2._y1 = 95;
	box2._y2 = 110;
	dm.f21_blitToScreen(bitmapIcon, &box2, k8_byteWidth, kM1_ColorNoTransparency, 16);
	if (champion.getAttributes(k0x0008_ChampionAttributeDisableAction) || _vm->_championMan->_candidateChampionOrdinal || _vm->_championMan->_partyIsSleeping) {
		_vm->_displayMan->f136_shadeScreenBox(&box, k0_ColorBlack);
	}
}

void MenuMan::f456_drawDisabledMenu() {
	if (!_vm->_championMan->_partyIsSleeping) {
		_vm->_eventMan->f363_highlightBoxDisable();
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
			if (_vm->_inventoryMan->_g424_panelContent == k4_PanelContentChest) {
				_vm->_inventoryMan->f334_closeChest();
			}
		} else {
			_vm->_displayMan->f136_shadeScreenBox(&_vm->_displayMan->_boxMovementArrows, k0_ColorBlack);
		}
		_vm->_displayMan->f136_shadeScreenBox(&boxSpellArea, k0_ColorBlack);
		_vm->_displayMan->f136_shadeScreenBox(&boxActionArea, k0_ColorBlack);
		_vm->_eventMan->f67_setMousePointerToNormal(k0_pointerArrow);
	}
}

void MenuMan::f390_refreshActionAreaAndSetChampDirMaxDamageReceived() {
	ChampionMan &champMan = *_vm->_championMan;

	if (!champMan._partyChampionCount)
		return;

	Champion *champ = nullptr;
	if (champMan._partyIsSleeping || champMan._candidateChampionOrdinal) {
		if (champMan._actingChampionOrdinal) {
			f388_clearActingChampion();
			return;
		}
		if (!champMan._candidateChampionOrdinal)
			return;
	} else {
		champ = champMan._champions;
		int16 champIndex = k0_ChampionFirst;

		do {
			if ((champIndex != champMan._leaderIndex)
				&& (_vm->M0_indexToOrdinal(champIndex) != champMan._actingChampionOrdinal)
				&& (champ->_maximumDamageReceived)
				&& (champ->_dir != champ->_directionMaximumDamageReceived)) {

				champ->_dir = (Direction)champ->_directionMaximumDamageReceived;
				champ->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
				champMan.drawChampionState((ChampionIndex)champIndex);
			}
			champ->_maximumDamageReceived = 0;
			champ++;
			champIndex++;
		} while (champIndex < champMan._partyChampionCount);
	}

	if (_g508_refreshActionArea) {
		if (!champMan._actingChampionOrdinal) {
			if (_g513_actionDamage) {
				f385_drawActionDamage(_g513_actionDamage);
				_g513_actionDamage = 0;
			} else {
				_g509_actionAreaContainsIcons = true;
				f387_drawActionArea();
			}
		} else {
			_g509_actionAreaContainsIcons = false;
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			champMan.drawChampionState((ChampionIndex)_vm->M1_ordinalToIndex(champMan._actingChampionOrdinal));
			f387_drawActionArea();
		}
	}
}

#define k7_ChampionNameMaximumLength 7 // @ C007_CHAMPION_NAME_MAXIMUM_LENGTH
#define k12_ActionNameMaximumLength 12 // @ C012_ACTION_NAME_MAXIMUM_LENGTH

void MenuMan::f387_drawActionArea() {
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	_vm->_eventMan->f77_hideMouse();
	dispMan._g578_useByteBoxCoordinates = false;
	dispMan.D24_fillScreenBox(boxActionArea, k0_ColorBlack);
	if (_g509_actionAreaContainsIcons) {
		for (uint16 champIndex = k0_ChampionFirst; champIndex < champMan._partyChampionCount; ++champIndex)
			f386_drawActionIcon((ChampionIndex)champIndex);
	} else if (champMan._actingChampionOrdinal) {
		Box box = boxActionArea3ActionMenu;
		if (_g713_actionList._actionIndices[2] == k255_ChampionActionNone)
			box = boxActionArea2ActionMenu;
		if (_g713_actionList._actionIndices[1] == k255_ChampionActionNone)
			box = boxActionArea1ActionMenu;
		dispMan.f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k10_MenuActionAreaIndice),
								 &box, k48_byteWidth, kM1_ColorNoTransparency, 45);
		textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen,
											235, 83, k0_ColorBlack, k4_ColorCyan, champMan._champions[_vm->M1_ordinalToIndex(champMan._actingChampionOrdinal)]._name,
											k7_ChampionNameMaximumLength, k200_heightScreen);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen, 241, 93 + actionListIndex * 12, k4_ColorCyan, k0_ColorBlack,
												f384_getActionName(_g713_actionList._actionIndices[actionListIndex]),
												k12_ActionNameMaximumLength, k200_heightScreen);
		}
	}
	_vm->_eventMan->f78_showMouse();
	_g508_refreshActionArea = false;
}

const char* MenuMan::f384_getActionName(ChampionAction actionIndex) {
	const char *g490_ChampionActionNames[44] = { // @ G0490_ac_Graphic560_ActionNames
		"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH",
		"KICK", "WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE",
		"HIT", "SWING", "STAB", "THRUST", "JAB", "PARRY", "HACK",
		"BERZERK", "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING",
		"DISRUPT", "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
		"BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
		"FLUXCAGE", "HEAL", "CALM", "LIGHT", "WINDOW", "SPIT",
		"BRANDISH", "THROW", "FUSE"
	};

	return (actionIndex == k255_ChampionActionNone) ? "" : g490_ChampionActionNames[actionIndex];
}

void MenuMan::f393_drawSpellAreaControls(ChampionIndex champIndex) {
	static Box boxSpellAreaControls(233, 319, 42, 49); // @ G0504_s_Graphic560_Box_SpellAreaControls 

	Champion *champ = &_vm->_championMan->_champions[champIndex];
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	int16 champHP0 = _vm->_championMan->_champions[0]._currHealth;
	int16 champHP1 = _vm->_championMan->_champions[1]._currHealth;
	int16 champHP2 = _vm->_championMan->_champions[2]._currHealth;
	int16 champHP3 = _vm->_championMan->_champions[3]._currHealth;
	_vm->_eventMan->f78_showMouse();
	_vm->_displayMan->D24_fillScreenBox(boxSpellAreaControls, k0_ColorBlack);
	switch (champIndex) {
	case 0:
		_vm->_eventMan->f6_highlightScreenBox(233, 277, 42, 49);
		_vm->_textMan->f53_printToLogicalScreen(235, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		if (_vm->_championMan->_partyChampionCount > 1) {
			if (champHP1) {
				_vm->_eventMan->f6_highlightScreenBox(280, 291, 42, 48);
			}
T0393002:
			if (_vm->_championMan->_partyChampionCount > 2) {
				if (champHP2) {
					_vm->_eventMan->f6_highlightScreenBox(294, 305, 42, 48);
				}
T0393003:
				if (_vm->_championMan->_partyChampionCount > 3) {
					if (champHP3) {
						_vm->_eventMan->f6_highlightScreenBox(308, 319, 42, 48);
					}
				}
			}
		}
		break;
	case 1:
		if (champHP0) {
			_vm->_eventMan->f6_highlightScreenBox(233, 244, 42, 48);
		}
		_vm->_eventMan->f6_highlightScreenBox(247, 291, 42, 49);
		_vm->_textMan->f53_printToLogicalScreen(249, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		goto T0393002;
	case 2:
		if (champHP0) {
			_vm->_eventMan->f6_highlightScreenBox(233, 244, 42, 48);
		}
		if (champHP1) {
			_vm->_eventMan->f6_highlightScreenBox(247, 258, 42, 48);
		}
		_vm->_eventMan->f6_highlightScreenBox(261, 305, 42, 49);
		_vm->_textMan->f53_printToLogicalScreen(263, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		goto T0393003;
	case 3:
		if (champHP0) {
			_vm->_eventMan->f6_highlightScreenBox(233, 244, 42, 48);
		}
		if (champHP1) {
			_vm->_eventMan->f6_highlightScreenBox(247, 258, 42, 48);
		}
		if (champHP2) {
			_vm->_eventMan->f6_highlightScreenBox(261, 272, 42, 48);
		}
		_vm->_eventMan->f6_highlightScreenBox(275, 319, 42, 49);
		_vm->_textMan->f53_printToLogicalScreen(277, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		break;
	default:
		break;
	}
	_vm->_eventMan->f77_hideMouse();
}

#define k2_SpellAreaAvailableSymbols 2 // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
#define k3_SpellAreaChampionSymbols 3 // @ C3_SPELL_AREA_CHAMPION_SYMBOLS

void MenuMan::f392_buildSpellAreaLine(int16 spellAreaBitmapLine) {
	static Box boxSpellAreaLine(0, 95, 0, 11); // @ K0074_s_Box_SpellAreaLine 

	char L1204_ac_SpellSymbolString[2] = {'\0', '\0'};
	Champion *L1203_ps_Champion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	if (spellAreaBitmapLine == k2_SpellAreaAvailableSymbols) {
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		_vm->_displayMan->f132_blitToBitmap(_gK73_bitmapSpellAreaLines, _gK72_bitmapSpellAreaLine, boxSpellAreaLine, 0, 12, k48_byteWidth, k48_byteWidth, kM1_ColorNoTransparency, 36, 12);
		int16 x = 1;
		char character = 96 + (6 * L1203_ps_Champion->_symbolStep);
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			L1204_ac_SpellSymbolString[0] = character++;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 48, x += 14, 8, k4_ColorCyan, k0_ColorBlack, L1204_ac_SpellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == k3_SpellAreaChampionSymbols) {
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		_vm->_displayMan->f132_blitToBitmap(_gK73_bitmapSpellAreaLines, _gK72_bitmapSpellAreaLine, boxSpellAreaLine, 0, 24, k48_byteWidth, k48_byteWidth, kM1_ColorNoTransparency, 36, 12);
		int16 x = 8;
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((L1204_ac_SpellSymbolString[0] = L1203_ps_Champion->_symbols[symbolIndex]) == '\0')
				break;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 48, x += 9, 8, k4_ColorCyan, k0_ColorBlack, L1204_ac_SpellSymbolString, 12);
		}
	}
}

void MenuMan::f394_setMagicCasterAndDrawSpellArea(int16 champIndex) {
	static Box boxSpellAreaLine2(224, 319, 50, 61); // @ K0075_s_Box_SpellAreaLine2 
	static Box boxSpellAreaLine3(224, 319, 62, 73); // @ K0076_s_Box_SpellAreaLine3 

	Champion* L1213_ps_Champion;

	if ((champIndex == _vm->_championMan->_magicCasterChampionIndex) || ((champIndex != kM1_ChampionNone) && !_vm->_championMan->_champions[champIndex]._currHealth)) {
		return;
	}
	if (_vm->_championMan->_magicCasterChampionIndex == kM1_ChampionNone) {
		_vm->_eventMan->f78_showMouse();
		_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k9_MenuSpellAreaBackground), &boxSpellArea, k48_byteWidth, kM1_ColorNoTransparency, 33);
		_vm->_eventMan->f77_hideMouse();
	}
	if (champIndex == kM1_ChampionNone) {
		_vm->_championMan->_magicCasterChampionIndex = kM1_ChampionNone;
		_vm->_eventMan->f78_showMouse();
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		_vm->_displayMan->D24_fillScreenBox(boxSpellArea, k0_ColorBlack);
		_vm->_eventMan->f77_hideMouse();
		return;
	}
	L1213_ps_Champion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex = (ChampionIndex)champIndex];
	f392_buildSpellAreaLine(k2_SpellAreaAvailableSymbols);
	_vm->_eventMan->f78_showMouse();
	f393_drawSpellAreaControls((ChampionIndex)champIndex);
	_vm->_displayMan->f21_blitToScreen(_gK72_bitmapSpellAreaLine, &boxSpellAreaLine2, k48_byteWidth, kM1_ColorNoTransparency, 12);
	f392_buildSpellAreaLine(k3_SpellAreaChampionSymbols);
	_vm->_displayMan->f21_blitToScreen(_gK72_bitmapSpellAreaLine, &boxSpellAreaLine3, k48_byteWidth, kM1_ColorNoTransparency, 12);
	_vm->_eventMan->f77_hideMouse();
}

void MenuMan::f457_drawEnabledMenus() {
	int16 L1462_i_Multiple;
#define AL1462_i_MagicCasterChampionIndex L1462_i_Multiple
#define AL1462_i_InventoryChampionOrdinal L1462_i_Multiple


	if (_vm->_championMan->_partyIsSleeping) {
		_vm->_eventMan->f379_drawSleepScreen();
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
	} else {
		AL1462_i_MagicCasterChampionIndex = _vm->_championMan->_magicCasterChampionIndex;
		_vm->_championMan->_magicCasterChampionIndex = kM1_ChampionNone; /* Force next function to draw the spell area */
		f394_setMagicCasterAndDrawSpellArea(AL1462_i_MagicCasterChampionIndex);
		if (!_vm->_championMan->_actingChampionOrdinal) {
			_g509_actionAreaContainsIcons = true;
		}
		f387_drawActionArea();
		AL1462_i_InventoryChampionOrdinal = _vm->_inventoryMan->_g432_inventoryChampionOrdinal;
		if (AL1462_i_InventoryChampionOrdinal) {
			_vm->_inventoryMan->_g432_inventoryChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
			_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)_vm->M1_ordinalToIndex(AL1462_i_InventoryChampionOrdinal));
		} else {
			_vm->_displayMan->f98_drawFloorAndCeiling();
			f395_drawMovementArrows();
		}
		_vm->_eventMan->f69_setMousePointer();
	}
}

int16 MenuMan::f408_getClickOnSpellCastResult() {
	int16 L1259_i_SpellCastResult;
	Champion* L1260_ps_Champion;


	L1260_ps_Champion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	_vm->_eventMan->f78_showMouse();
	_vm->_eventMan->f363_highlightBoxDisable();
	if ((L1259_i_SpellCastResult = f412_getChampionSpellCastResult(_vm->_championMan->_magicCasterChampionIndex)) != k3_spellCastFailureNeedsFlask) {
		L1260_ps_Champion->_symbols[0] = '\0';
		f397_drawAvailableSymbols(L1260_ps_Champion->_symbolStep = 0);
		f398_drawChampionSymbols(L1260_ps_Champion);
	} else {
		L1259_i_SpellCastResult = k0_spellCastFailure;
	}
	_vm->_eventMan->f77_hideMouse();
	return L1259_i_SpellCastResult;
}

int16 MenuMan::f412_getChampionSpellCastResult(uint16 champIndex) {
	uint16 L1267_ui_Multiple;
#define AL1267_ui_SkillLevel L1267_ui_Multiple
#define AL1267_ui_LightPower L1267_ui_Multiple
#define AL1267_ui_SpellPower L1267_ui_Multiple
#define AL1267_ui_Ticks      L1267_ui_Multiple
	int16 L1268_i_PowerSymbolOrdinal;
	uint16 L1269_ui_Multiple;
#define AL1269_ui_RequiredSkillLevel L1269_ui_Multiple
#define AL1269_ui_EmptyFlaskWeight   L1269_ui_Multiple
#define AL1269_ui_Ticks              L1269_ui_Multiple
	Champion* L1270_ps_Champion;
	Spell* L1271_ps_Spell;
	Thing L1272_T_Object;
	uint16 L1273_ui_Experience;
	int16 L1274_i_MissingSkillLevelCount;
	Potion* L1275_ps_Potion;
	TimelineEvent L1276_s_Event;
	Junk* L1277_ps_Junk;


	if (champIndex >= _vm->_championMan->_partyChampionCount) {
		return k0_spellCastFailure;
	}
	L1270_ps_Champion = &_vm->_championMan->_champions[champIndex];
	if (!(L1270_ps_Champion->_currHealth)) {
		return k0_spellCastFailure;
	}
	L1271_ps_Spell = f409_getSpellFromSymbols((unsigned char *)L1270_ps_Champion->_symbols);
	if (L1271_ps_Spell == 0) {
		f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k1_spellCastSuccess, 0);
		return k0_spellCastFailure;
	}
	L1268_i_PowerSymbolOrdinal = L1270_ps_Champion->_symbols[0] - '_'; /* Values 1 to 6 */
	AL1269_ui_RequiredSkillLevel = L1271_ps_Spell->_baseRequiredSkillLevel + L1268_i_PowerSymbolOrdinal;
	L1273_ui_Experience = _vm->getRandomNumber(8) + (AL1269_ui_RequiredSkillLevel << 4) + ((_vm->M1_ordinalToIndex(L1268_i_PowerSymbolOrdinal) * L1271_ps_Spell->_baseRequiredSkillLevel) << 3) + (AL1269_ui_RequiredSkillLevel * AL1269_ui_RequiredSkillLevel);
	AL1267_ui_SkillLevel = _vm->_championMan->getSkillLevel(champIndex, L1271_ps_Spell->_skillIndex);
	if (AL1267_ui_SkillLevel < AL1269_ui_RequiredSkillLevel) {
		L1274_i_MissingSkillLevelCount = AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel;
		while (L1274_i_MissingSkillLevelCount--) {
			if (_vm->getRandomNumber(128) > MIN(L1270_ps_Champion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent] + 15, 115)) {
				_vm->_championMan->addSkillExperience(champIndex, L1271_ps_Spell->_skillIndex, L1273_ui_Experience >> (AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel));
				f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k0_failureNeedsMorePractice, L1271_ps_Spell->_skillIndex);
				return k0_spellCastFailure;
			}
		}
	}
	switch (L1271_ps_Spell->getKind()) {
	case k1_spellKindPotion:
		if ((L1275_ps_Potion = f411_getEmptyFlaskInHand(L1270_ps_Champion, &L1272_T_Object)) == NULL) {
			f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k10_failureNeedsFlaskInHand, 0);
			return k3_spellCastFailureNeedsFlask;
		}
		AL1269_ui_EmptyFlaskWeight = _vm->_dungeonMan->f140_getObjectWeight(L1272_T_Object);
		L1275_ps_Potion->setType((PotionType)L1271_ps_Spell->getType());
		L1275_ps_Potion->setPower(_vm->getRandomNumber(16) + (L1268_i_PowerSymbolOrdinal * 40));
		L1270_ps_Champion->_load += _vm->_dungeonMan->f140_getObjectWeight(L1272_T_Object) - AL1269_ui_EmptyFlaskWeight;
		_vm->_championMan->drawChangedObjectIcons();
		if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal == _vm->M0_indexToOrdinal(champIndex)) {
			setFlag(L1270_ps_Champion->_attributes, k0x0200_ChampionAttributeLoad);
			_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
		}
		break;
	case k2_spellKindProjectile:
		if (L1270_ps_Champion->_dir != _vm->_dungeonMan->_g308_partyDir) {
			L1270_ps_Champion->_dir = _vm->_dungeonMan->_g308_partyDir;
			setFlag(L1270_ps_Champion->_attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
		}
		if (L1271_ps_Spell->getType() == k4_spellType_projectileOpenDoor) {
			AL1267_ui_SkillLevel <<= 1;
		}
		_vm->_championMan->isProjectileSpellCast(champIndex, Thing(L1271_ps_Spell->getType() + Thing::_firstExplosion.toUint16()), f26_getBoundedValue(21, (L1268_i_PowerSymbolOrdinal + 2) * (4 + (AL1267_ui_SkillLevel << 1)), 255), 0);
		break;
	case k3_spellKindOther:
		L1276_s_Event._priority = 0;
		AL1267_ui_SpellPower = (L1268_i_PowerSymbolOrdinal + 1) << 2;
		switch (L1271_ps_Spell->getType()) {
		case k0_spellType_otherLight:
			AL1269_ui_Ticks = 10000 + ((AL1267_ui_SpellPower - 8) << 9);
			AL1267_ui_LightPower = (AL1267_ui_SpellPower >> 1);
			AL1267_ui_LightPower--;
			goto T0412019;
		case k5_spellType_otherMagicTorch:
			AL1269_ui_Ticks = 2000 + ((AL1267_ui_SpellPower - 3) << 7);
			AL1267_ui_LightPower = (AL1267_ui_SpellPower >> 2);
			AL1267_ui_LightPower++;
T0412019:
			_vm->_championMan->_party._magicalLightAmount += _vm->_championMan->_lightPowerToLightAmount[AL1267_ui_LightPower];
			f404_createEvent70_light(-AL1267_ui_LightPower, AL1269_ui_Ticks);
			break;
		case k1_spellType_otherDarkness:
			AL1267_ui_LightPower = (AL1267_ui_SpellPower >> 2);
			_vm->_championMan->_party._magicalLightAmount -= _vm->_championMan->_lightPowerToLightAmount[AL1267_ui_LightPower];
			f404_createEvent70_light(AL1267_ui_LightPower, 98);
			break;
		case k2_spellType_otherThievesEye:
			L1276_s_Event._type = k73_TMEventTypeThievesEye;
			_vm->_championMan->_party._event73Count_ThievesEye++;
			AL1267_ui_SpellPower = (AL1267_ui_SpellPower >> 1);
			goto T0412032;
		case k3_spellType_otherInvisibility:
			L1276_s_Event._type = k71_TMEventTypeInvisibility;
			_vm->_championMan->_party._event71Count_Invisibility++;
			goto T0412033;
		case k4_spellType_otherPartyShield:
			L1276_s_Event._type = k74_TMEventTypePartyShield;
			L1276_s_Event._B._defense = AL1267_ui_SpellPower;
			if (_vm->_championMan->_party._shieldDefense > 50) {
				L1276_s_Event._B._defense >>= 2;
			}
			_vm->_championMan->_party._shieldDefense += L1276_s_Event._B._defense;
			_vm->_timeline->f260_timelineRefreshAllChampionStatusBoxes();
			goto T0412032;
		case k6_spellType_otherFootprints:
			L1276_s_Event._type = k79_TMEventTypeFootprints;
			_vm->_championMan->_party._event79Count_Footprints++;
			_vm->_championMan->_party._firstScentIndex = _vm->_championMan->_party._scentCount;
			if (L1268_i_PowerSymbolOrdinal < 3) {
				_vm->_championMan->_party._lastScentIndex = _vm->_championMan->_party._firstScentIndex;
			} else {
				_vm->_championMan->_party._lastScentIndex = 0;
			}
T0412032:
			AL1267_ui_Ticks = AL1267_ui_SpellPower * AL1267_ui_SpellPower;
T0412033:
			M33_setMapAndTime(L1276_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + AL1267_ui_Ticks);
			_vm->_timeline->f238_addEventGetEventIndex(&L1276_s_Event);
			break;
		case k7_spellType_otherZokathra:
			if ((L1272_T_Object = _vm->_dungeonMan->f166_getUnusedThing(k10_JunkThingType)) == Thing::_none)
				break;
			L1277_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(L1272_T_Object);
			L1277_ps_Junk->setType(k51_JunkTypeZokathra);
			ChampionSlot AL1267_ui_SlotIndex;
			if (L1270_ps_Champion->_slots[k0_ChampionSlotReadyHand] == Thing::_none) {
				AL1267_ui_SlotIndex = k0_ChampionSlotReadyHand;
			} else {
				if (L1270_ps_Champion->_slots[k1_ChampionSlotActionHand] == Thing::_none) {
					AL1267_ui_SlotIndex = k1_ChampionSlotActionHand;
				} else {
					AL1267_ui_SlotIndex = kM1_ChampionSlotLeaderHand;
				}
			}
			if ((AL1267_ui_SlotIndex == k0_ChampionSlotReadyHand) || (AL1267_ui_SlotIndex == k1_ChampionSlotActionHand)) {
				_vm->_championMan->addObjectInSlot((ChampionIndex)champIndex, L1272_T_Object, AL1267_ui_SlotIndex);
				_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
			} else {
				_vm->_moveSens->f267_getMoveResult(L1272_T_Object, kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
			}
			break;
		case k8_spellType_otherFireshield:
			f403_isPartySpellOrFireShieldSuccessful(L1270_ps_Champion, false, (AL1267_ui_SpellPower * AL1267_ui_SpellPower) + 100, false);
		}
	}
	_vm->_championMan->addSkillExperience(champIndex, L1271_ps_Spell->_skillIndex, L1273_ui_Experience);
	_vm->_championMan->disableAction(champIndex, L1271_ps_Spell->getDuration());
	return k1_spellCastSuccess;
}

Spell* MenuMan::f409_getSpellFromSymbols(byte* symbols) {
	static Spell G0487_as_Graphic560_Spells[25] = {
		/* { Symbols, BaseRequiredSkillLevel, SkillIndex, Attributes } */
		Spell(0x00666F00, 2, 15, 0x7843),
		Spell(0x00667073, 1, 18, 0x4863),
		Spell(0x00686D77, 3, 17, 0xB433),
		Spell(0x00686C00, 3, 19, 0x6C72),
		Spell(0x00686D76, 3, 18, 0x8423),
		Spell(0x00686E76, 4, 17, 0x7822),
		Spell(0x00686F76, 4, 17, 0x5803),
		Spell(0x00690000, 1, 16, 0x3C53),
		Spell(0x00696F00, 3, 16, 0xA802),
		Spell(0x00697072, 4, 13, 0x3C71),
		Spell(0x00697075, 4, 15, 0x7083),
		Spell(0x006A6D00, 1, 18, 0x5032),
		Spell(0x006A6C00, 1, 19, 0x4062),
		Spell(0x006A6F77, 1, 15, 0x3013),
		Spell(0x006B0000, 1, 17, 0x3C42),
		Spell(0x00667000, 2, 15, 0x64C1),
		Spell(0x00660000, 2, 13, 0x3CB1),
		Spell(0x00667074, 4, 13, 0x3C81),
		Spell(0x00667075, 4, 13, 0x3C91),
		Spell(0x00670000, 1, 13, 0x80E1),
		Spell(0x00677000, 1, 13, 0x68A1),
		Spell(0x00687073, 4, 13, 0x3C61),
		Spell(0x006B7076, 3,  2, 0xFCD1),
		Spell(0x006B6C00, 2, 19, 0x7831),
		Spell(0x006B6E76, 0,  3, 0x3C73)
	};

	if (*(symbols + 1)) {
		int16 AL1262_i_BitShiftCount = 24;
		int32 L1261_l_Symbols = 0;
		do {
			L1261_l_Symbols |= (long)*symbols++ << AL1262_i_BitShiftCount;
		} while (*symbols && ((AL1262_i_BitShiftCount -= 8) >= 0));
		Spell *L1263_ps_Spell = G0487_as_Graphic560_Spells;
		int16 AL1262_i_SpellIndex = 25;
		while (AL1262_i_SpellIndex--) {
			if (L1263_ps_Spell->_symbols & 0xFF000000) { /* If byte 1 of spell is not 0 then the spell includes the power symbol */
				if (L1261_l_Symbols == L1263_ps_Spell->_symbols) { /* Compare champion symbols, including power symbol, with spell (never used with actual spells) */
					return L1263_ps_Spell;
				}
			} else {
				if ((L1261_l_Symbols & 0x00FFFFFF) == L1263_ps_Spell->_symbols) { /* Compare champion symbols, except power symbol, with spell */
					return L1263_ps_Spell;
				}
			}
			L1263_ps_Spell++;
		}
	}
	return NULL;
}

void MenuMan::f410_menusPrintSpellFailureMessage(Champion* champ, uint16 failureType, uint16 skillIndex) {
	const char *L1264_pc_Message = nullptr;

	if (skillIndex > k3_ChampionSkillWizard)
		skillIndex = (skillIndex - 4) / 4;

	_vm->_textMan->f51_messageAreaPrintLineFeed();
	_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, champ->_name);

	const char *messages_EN_ANY[4] = {" NEEDS MORE PRACTICE WITH THIS ", " SPELL.", " MUMBLES A MEANINGLESS SPELL."," NEEDS AN EMPTY FLASK IN HAND FOR POTION."};
	const char *messages_DE_DEU[4] = {" BRAUCHT MEHR UEBUNG MIT DIESEM ", " ZAUBERSPRUCH.",
		" MURMELT EINEN SINNLOSEN ZAUBERSPRUCH.", " MUSS FUER DEN TRANK EINE LEERE FLASCHE BEREITHALTEN."};
	const char *messages_FR_FRA[5] = {" DOIT PRATIQUER DAVANTAGE SON ", "ENVOUTEMENT.", " MARMONNE UNE CONJURATION IMCOMPREHENSIBLE.",
		" DOIT AVOIR UN FLACON VIDE EN MAIN POUR LA POTION.", "EXORCISME."};
	const char **messages;
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		messages = messages_EN_ANY; break;
	case Common::DE_DEU:
		messages = messages_DE_DEU; break;
	case Common::FR_FRA:
		messages = messages_FR_FRA; break;
	}

	switch (failureType) {
	case k0_failureNeedsMorePractice:
		_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, messages[0]);
		_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, _vm->_championMan->_baseSkillName[skillIndex]);
		if (_vm->getGameLanguage() != Common::FR_FRA || skillIndex == k3_ChampionSkillWizard)
			L1264_pc_Message = messages[1];
		else
			L1264_pc_Message = messages[4];

		break;
	case k1_failureMeaninglessSpell:
		L1264_pc_Message = messages[2];
		break;
	case k10_failureNeedsFlaskInHand:
		L1264_pc_Message = messages[3];
		break;
	}
	_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, L1264_pc_Message);
}

Potion* MenuMan::f411_getEmptyFlaskInHand(Champion* champ, Thing* potionThing) {
	Thing L1265_T_Thing;
	int16 L1266_i_SlotIndex;

	for (L1266_i_SlotIndex = k2_ChampionSlotHead; --L1266_i_SlotIndex >= k0_ChampionSlotReadyHand; ) {
		if (((L1265_T_Thing = champ->_slots[L1266_i_SlotIndex]) != Thing::_none) && (_vm->_objectMan->f33_getIconIndex(L1265_T_Thing) == k195_IconIndicePotionEmptyFlask)) {
			*potionThing = L1265_T_Thing;
			return (Potion*)_vm->_dungeonMan->f156_getThingData(L1265_T_Thing);
		}
	}
	return nullptr;
}

void MenuMan::f404_createEvent70_light(int16 lightPower, int16 ticks) {
	TimelineEvent L1241_s_Event;

	L1241_s_Event._type = k70_TMEventTypeLight;
	L1241_s_Event._B._lightPower = lightPower;
	M33_setMapAndTime(L1241_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + ticks);
	L1241_s_Event._priority = 0;
	_vm->_timeline->f238_addEventGetEventIndex(&L1241_s_Event);
	_vm->_inventoryMan->f337_setDungeonViewPalette();
}

bool MenuMan::f403_isPartySpellOrFireShieldSuccessful(Champion* champ, bool spellShield, uint16 ticks, bool useMana) {
	bool L1239_B_IsPartySpellOrFireShieldSuccessful;
	TimelineEvent L1240_s_Event;


	L1239_B_IsPartySpellOrFireShieldSuccessful = true;
	if (useMana) {
		if (champ->_currMana == 0) {
			return false;
		}
		if (champ->_currMana < 4) {
			ticks >>= 1;
			champ->_currMana = 0;
			L1239_B_IsPartySpellOrFireShieldSuccessful = false;
		} else {
			champ->_currMana -= 4;
		}
	}
	L1240_s_Event._B._defense = ticks >> 5;
	if (spellShield) {
		L1240_s_Event._type = k77_TMEventTypeSpellShield;
		if (_vm->_championMan->_party._spellShieldDefense > 50) {
			L1240_s_Event._B._defense >>= 2;
		}
		_vm->_championMan->_party._spellShieldDefense += L1240_s_Event._B._defense;
	} else {
		L1240_s_Event._type = k78_TMEventTypeFireShield;
		if (_vm->_championMan->_party._fireShieldDefense > 50) {
			L1240_s_Event._B._defense >>= 2;
		}
		_vm->_championMan->_party._fireShieldDefense += L1240_s_Event._B._defense;
	}
	L1240_s_Event._priority = 0;
	M33_setMapAndTime(L1240_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + ticks);
	_vm->_timeline->f238_addEventGetEventIndex(&L1240_s_Event);
	_vm->_timeline->f260_timelineRefreshAllChampionStatusBoxes();
	return L1239_B_IsPartySpellOrFireShieldSuccessful;
}

void MenuMan::f397_drawAvailableSymbols(uint16 symbolStep) {
	uint16 L1214_ui_Counter;
	int16 L1215_i_X;
	char L1216_c_Character;
	char L1217_ac_String[2];

	L1217_ac_String[1] = '\0';
	L1216_c_Character = 96 + 6 * symbolStep;
	L1215_i_X = 225;
	for (L1214_ui_Counter = 0; L1214_ui_Counter < 6; L1214_ui_Counter++) {
		L1217_ac_String[0] = L1216_c_Character++;
		_vm->_textMan->f53_printToLogicalScreen(L1215_i_X += 14, 58, k4_ColorCyan, k0_ColorBlack, L1217_ac_String);
	}
}

void MenuMan::f398_drawChampionSymbols(Champion* champ) {
	uint16 L1218_ui_SymbolIndex;
	int16 L1219_i_X;
	uint16 L1220_ui_SymbolCount;
	char L1221_ac_String[2];


	L1220_ui_SymbolCount = strlen(champ->_symbols);
	L1219_i_X = 232;
	L1221_ac_String[1] = '\0';
	for (L1218_ui_SymbolIndex = 0; L1218_ui_SymbolIndex < 4; L1218_ui_SymbolIndex++) {
		if (L1218_ui_SymbolIndex >= L1220_ui_SymbolCount) {
			L1221_ac_String[0] = ' ';
		} else {
			L1221_ac_String[0] = champ->_symbols[L1218_ui_SymbolIndex];
		}
		_vm->_textMan->f53_printToLogicalScreen(L1219_i_X += 9, 70, k4_ColorCyan, k0_ColorBlack, L1221_ac_String);
	}
}

void MenuMan::f399_addChampionSymbol(int16 symbolIndex) {
	static byte G0485_aauc_Graphic560_SymbolBaseManaCost[4][6] = {
		{1, 2, 3, 4, 5, 6},   /* Power 1 */
		{2, 3, 4, 5, 6, 7},   /* Power 2 */
		{4, 5, 6, 7, 7, 9},   /* Power 3 */
		{2, 2, 3, 4, 6, 7}}; /* Power 4 */
	static byte G0486_auc_Graphic560_SymbolManaCostMultiplier[6] = {8, 12, 16, 20, 24, 28};

	uint16 L1222_ui_SymbolStep;
	uint16 L1223_ui_ManaCost;
	uint16 L1224_ui_SymbolIndex;
	Champion* L1225_ps_Champion;

	L1225_ps_Champion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	L1222_ui_SymbolStep = L1225_ps_Champion->_symbolStep;
	L1223_ui_ManaCost = G0485_aauc_Graphic560_SymbolBaseManaCost[L1222_ui_SymbolStep][symbolIndex];
	if (L1222_ui_SymbolStep) {
		L1223_ui_ManaCost = (L1223_ui_ManaCost * G0486_auc_Graphic560_SymbolManaCostMultiplier[L1224_ui_SymbolIndex = L1225_ps_Champion->_symbols[0] - 96]) >> 3;
	}
	if (L1223_ui_ManaCost <= L1225_ps_Champion->_currMana) {
		L1225_ps_Champion->_currMana -= L1223_ui_ManaCost;
		setFlag(L1225_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
		L1225_ps_Champion->_symbols[L1222_ui_SymbolStep] = 96 + (L1222_ui_SymbolStep * 6) + symbolIndex;
		L1225_ps_Champion->_symbols[L1222_ui_SymbolStep + 1] = '\0';
		L1225_ps_Champion->_symbolStep = L1222_ui_SymbolStep = returnNextVal(L1222_ui_SymbolStep);
		_vm->_eventMan->f78_showMouse();
		f397_drawAvailableSymbols(L1222_ui_SymbolStep);
		f398_drawChampionSymbols(L1225_ps_Champion);
		_vm->_championMan->drawChampionState(_vm->_championMan->_magicCasterChampionIndex);
		_vm->_eventMan->f77_hideMouse();
	}
}

void MenuMan::f400_deleteChampionSymbol() {
	int16 L1226_ui_SymbolStep;
	Champion* L1228_ps_Champion;

	L1228_ps_Champion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	if (!strlen(L1228_ps_Champion->_symbols)) {
		return;
	}
	L1228_ps_Champion->_symbolStep = L1226_ui_SymbolStep = returnPrevVal(L1228_ps_Champion->_symbolStep);
	L1228_ps_Champion->_symbols[L1226_ui_SymbolStep] = '\0';
	_vm->_eventMan->f78_showMouse();
	f397_drawAvailableSymbols(L1226_ui_SymbolStep);
	f398_drawChampionSymbols(L1228_ps_Champion);
	_vm->_eventMan->f77_hideMouse();
}

bool MenuMan::f391_didClickTriggerAction(int16 actionListIndex) {
	uint16 L1196_ui_ChampionIndex;
	uint16 L1197_ui_ActionIndex;
	bool L1198_B_ClickTriggeredAction;
	Champion* L1199_ps_Champion;


	if (!_vm->_championMan->_actingChampionOrdinal || (actionListIndex != -1 && (_g713_actionList._actionIndices[actionListIndex] == k255_ChampionActionNone)))
		return false;

	L1199_ps_Champion = &_vm->_championMan->_champions[L1196_ui_ChampionIndex = _vm->M1_ordinalToIndex(_vm->_championMan->_actingChampionOrdinal)];
	if (actionListIndex == -1) {
		// L1198_B_ClickTriggeredAction is set to -1 since booleans are stored in int16 in the original
		L1198_B_ClickTriggeredAction = true;
	} else {
		L1197_ui_ActionIndex = _g713_actionList._actionIndices[actionListIndex];
		L1199_ps_Champion->_actionDefense += g495_actionDefense[L1197_ui_ActionIndex]; /* BUG0_54 The defense modifier of an action is permanent.
																									 Each action has an associated defense modifier value and a number of ticks while the champion cannot perform another action because the action icon is grayed out. If an action has a non zero defense modifier and a zero value for the number of ticks then the defense modifier is applied but it is never removed. This causes no issue in the original games because there are no actions in this case but it may occur in a version where data is customized. This statement should only be executed if the value for the action in G0491_auc_Graphic560_ActionDisabledTicks is not 0 otherwise the action is not disabled at the end of F0407_MENUS_IsActionPerformed and thus not enabled later in F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction where the defense modifier is also removed */
		setFlag(L1199_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
		L1198_B_ClickTriggeredAction = f407_isActionPerformed(L1196_ui_ChampionIndex, L1197_ui_ActionIndex);
		L1199_ps_Champion->_actionIndex = (ChampionAction)L1197_ui_ActionIndex;
	}
	f388_clearActingChampion();
	return L1198_B_ClickTriggeredAction;
}

bool MenuMan::f407_isActionPerformed(uint16 champIndex, int16 actionIndex) {
	static unsigned char G0491_auc_Graphic560_ActionDisabledTicks[44] = {
		0,  /* N */
		6,  /* BLOCK */
		8,  /* CHOP */
		0,  /* X */
		6,  /* BLOW HORN */
		3,  /* FLIP */
		1,  /* PUNCH */
		5,  /* KICK */
		3,  /* WAR CRY */
		5,  /* STAB */
		35, /* CLIMB DOWN */
		20, /* FREEZE LIFE */
		4,  /* HIT */
		6,  /* SWING */
		10, /* STAB */
		16, /* THRUST */
		2,  /* JAB */
		18, /* PARRY */
		8,  /* HACK */
		30, /* BERZERK */
		42, /* FIREBALL */
		31, /* DISPELL */
		10, /* CONFUSE */
		38, /* LIGHTNING */
		9,  /* DISRUPT */
		20, /* MELEE */
		10, /* X */
		16, /* INVOKE */
		4,  /* SLASH */
		12, /* CLEAVE */
		20, /* BASH */
		7,  /* STUN */
		14, /* SHOOT */
		30, /* SPELLSHIELD */
		35, /* FIRESHIELD */
		2,  /* FLUXCAGE */
		19, /* HEAL */
		9,  /* CALM */
		10, /* LIGHT */
		15, /* WINDOW */
		22, /* SPIT */
		10, /* BRANDISH */
		0,  /* THROW */
		2}; /* FUSE */
	static unsigned char G0494_auc_Graphic560_ActionStamina[44] = {
		0,  /* N */
		4,  /* BLOCK */
		10, /* CHOP */
		0,  /* X */
		1,  /* BLOW HORN */
		0,  /* FLIP */
		1,  /* PUNCH */
		3,  /* KICK */
		1,  /* WAR CRY */
		3,  /* STAB */
		40, /* CLIMB DOWN */
		3,  /* FREEZE LIFE */
		3,  /* HIT */
		2,  /* SWING */
		4,  /* STAB */
		17, /* THRUST */
		3,  /* JAB */
		1,  /* PARRY */
		6,  /* HACK */
		40, /* BERZERK */
		5,  /* FIREBALL */
		2,  /* DISPELL */
		2,  /* CONFUSE */
		4,  /* LIGHTNING */
		5,  /* DISRUPT */
		25, /* MELEE */
		1,  /* X */
		2,  /* INVOKE */
		2,  /* SLASH */
		10, /* CLEAVE */
		9,  /* BASH */
		2,  /* STUN */
		3,  /* SHOOT */
		1,  /* SPELLSHIELD */
		2,  /* FIRESHIELD */
		6,  /* FLUXCAGE */
		1,  /* HEAL */
		1,  /* CALM */
		3,  /* LIGHT */
		2,  /* WINDOW */
		3,  /* SPIT */
		2,  /* BRANDISH */
		0,  /* THROW */
		2}; /* FUSE */
	static unsigned char G0497_auc_Graphic560_ActionExperienceGain[44] = {
		0,  /* N */
		8,  /* BLOCK */
		10, /* CHOP */
		0,  /* X */
		0,  /* BLOW HORN */
		0,  /* FLIP */
		8,  /* PUNCH */
		13, /* KICK */
		7,  /* WAR CRY */
		15, /* STAB */
		15, /* CLIMB DOWN */
		22, /* FREEZE LIFE */
		10, /* HIT */
		6,  /* SWING */
		12, /* STAB */
		19, /* THRUST */
		11, /* JAB */
		17, /* PARRY */
		9,  /* HACK */
		40, /* BERZERK */
		35, /* FIREBALL */
		25, /* DISPELL */
		0,  /* CONFUSE */
		30, /* LIGHTNING */
		10, /* DISRUPT */
		24, /* MELEE */
		0,  /* X */
		25, /* INVOKE */
		9,  /* SLASH */
		12, /* CLEAVE */
		11, /* BASH */
		10, /* STUN */
		20, /* SHOOT Atari ST Versions 1.0 1987-12-08 1987-12-11: 9 */
		20, /* SPELLSHIELD */
		20, /* FIRESHIELD */
		12, /* FLUXCAGE */
		0,  /* HEAL */
		0,  /* CALM */
		20, /* LIGHT */
		30, /* WINDOW */
		25, /* SPIT */
		0,  /* BRANDISH */
		5,  /* THROW */
		1}; /* FUSE */
	uint16 L1244_ui_Multiple;
#define AL1244_ui_TargetSquare  L1244_ui_Multiple
#define AL1244_ui_HealingAmount L1244_ui_Multiple
#define AL1244_ui_ManaCost      L1244_ui_Multiple
	int16 L1245_i_Multiple;
#define AL1245_T_ExplosionThing  L1245_i_Multiple
#define AL1245_B_ActionPerformed L1245_i_Multiple
	int16 L1246_i_Multiple = 0;
#define AL1246_i_RequiredManaAmount    L1246_i_Multiple
#define AL1246_i_ActionHandWeaponClass L1246_i_Multiple
#define AL1246_i_StepEnergy            L1246_i_Multiple
#define AL1246_i_HealingCapability     L1246_i_Multiple
#define AL1246_i_Ticks                 L1246_i_Multiple
	Champion* L1247_ps_Champion;
	Weapon* L1248_ps_Weapon;
	uint16 L1249_ui_ActionDisabledTicks;
	int16 L1250_i_Multiple;
#define AL1250_i_KineticEnergy        L1250_i_Multiple
#define AL1250_i_ReadyHandWeaponClass L1250_i_Multiple
#define AL1250_i_MissingHealth        L1250_i_Multiple
#define AL1250_i_HealingAmount        L1250_i_Multiple
	int16 L1251_i_MapX;
	int16 L1252_i_MapY;
	int16 L1253_i_ActionStamina;
	int16 L1254_i_ActionSkillIndex;
	int16 L1255_i_ActionExperienceGain;
	WeaponInfo* L1256_ps_WeaponInfoActionHand;
	WeaponInfo* L1257_ps_WeaponInfoReadyHand;
	TimelineEvent L1258_s_Event;


	if (champIndex >= _vm->_championMan->_partyChampionCount) {
		return false;
	}
	L1247_ps_Champion = &_vm->_championMan->_champions[champIndex];
	L1248_ps_Weapon = (Weapon*)_vm->_dungeonMan->f156_getThingData(L1247_ps_Champion->_slots[k1_ChampionSlotActionHand]);
	if (!L1247_ps_Champion->_currHealth) {
		return false;
	}
	L1251_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
	L1252_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
	L1251_i_MapX += _vm->_dirIntoStepCountEast[L1247_ps_Champion->_dir], L1252_i_MapY += _vm->_dirIntoStepCountNorth[L1247_ps_Champion->_dir];
	_g517_actionTargetGroupThing = _vm->_groupMan->f175_groupGetThing(L1251_i_MapX, L1252_i_MapY);
	L1249_ui_ActionDisabledTicks = G0491_auc_Graphic560_ActionDisabledTicks[actionIndex];
	L1254_i_ActionSkillIndex = g496_ActionSkillIndex[actionIndex];
	L1253_i_ActionStamina = G0494_auc_Graphic560_ActionStamina[actionIndex] + _vm->getRandomNumber(2);
	L1255_i_ActionExperienceGain = G0497_auc_Graphic560_ActionExperienceGain[actionIndex];
	AL1244_ui_TargetSquare = _vm->_dungeonMan->f151_getSquare(L1251_i_MapX, L1252_i_MapY).toByte();
	AL1245_B_ActionPerformed = true;
	if (((L1254_i_ActionSkillIndex >= k16_ChampionSkillFire) && (L1254_i_ActionSkillIndex <= k19_ChampionSkillWater)) || (L1254_i_ActionSkillIndex == k3_ChampionSkillWizard)) {
		AL1246_i_RequiredManaAmount = 7 - MIN((uint16)6, _vm->_championMan->getSkillLevel(champIndex, L1254_i_ActionSkillIndex));
	}
	switch (actionIndex) {
	case k23_ChampionActionLightning:
		AL1250_i_KineticEnergy = 180;
		AL1245_T_ExplosionThing = Thing::_explLightningBolt.toUint16();
		goto T0407014;
	case k21_ChampionActionDispel:
		AL1250_i_KineticEnergy = 150;
		AL1245_T_ExplosionThing = Thing::_explHarmNonMaterial.toUint16();
		goto T0407014;
	case k20_ChampionActionFireball:
		AL1250_i_KineticEnergy = 150;
		goto T0407013;
	case k40_ChampionActionSpit:
		AL1250_i_KineticEnergy = 250;
T0407013:
		AL1245_T_ExplosionThing = Thing::_explFireBall.toUint16();
T0407014:
		f406_setChampionDirectionToPartyDirection(L1247_ps_Champion);
		if (L1247_ps_Champion->_currMana < AL1246_i_RequiredManaAmount) {
			AL1250_i_KineticEnergy = MAX(2, L1247_ps_Champion->_currMana * AL1250_i_KineticEnergy / AL1246_i_RequiredManaAmount);
			AL1246_i_RequiredManaAmount = L1247_ps_Champion->_currMana;
		}
		if (!(AL1245_B_ActionPerformed = _vm->_championMan->isProjectileSpellCast(champIndex, Thing(AL1245_T_ExplosionThing), AL1250_i_KineticEnergy, AL1246_i_RequiredManaAmount))) {
			L1255_i_ActionExperienceGain >>= 1;
		}
		f405_decrementCharges(L1247_ps_Champion);
		break;
	case k30_ChampionActionBash:
	case k18_ChampionActionHack:
	case k19_ChampionActionBerzerk:
	case k7_ChampionActionKick:
	case k13_ChampionActionSwing:
	case k2_ChampionActionChop:
		if ((Square(AL1244_ui_TargetSquare).getType() == k4_DoorElemType) && (Square(AL1244_ui_TargetSquare).getDoorState() == k4_doorState_CLOSED)) {
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k1_soundModePlayIfPrioritized);
			L1249_ui_ActionDisabledTicks = 6;
			_vm->_groupMan->f232_groupIsDoorDestoryedByAttack(L1251_i_MapX, L1252_i_MapY, _vm->_championMan->getStrength(champIndex, k1_ChampionSlotActionHand), false, 2);
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k2_soundModePlayOneTickLater);
			break;
		}
	case k24_ChampionActionDisrupt:
	case k16_ChampionActionJab:
	case k17_ChampionActionParry:
	case k14_ChampionActionStab_C014:
	case k9_ChampionActionStab_C009:
	case k31_ChampionActionStun:
	case k15_ChampionActionThrust:
	case k25_ChampionActionMelee:
	case k28_ChampionActionSlash:
	case k29_ChampionActionCleave:
	case k6_ChampionActionPunch:
		if (!(AL1245_B_ActionPerformed = f402_isMeleeActionPerformed(champIndex, L1247_ps_Champion, actionIndex, L1251_i_MapX, L1252_i_MapY, L1254_i_ActionSkillIndex))) {
			L1255_i_ActionExperienceGain >>= 1;
			L1249_ui_ActionDisabledTicks >>= 1;
		}
		break;
	case k22_ChampionActionConfuse:
		f405_decrementCharges(L1247_ps_Champion);
	case k8_ChampionActionWarCry:
	case k37_ChampionActionCalm:
	case k41_ChampionActionBrandish:
	case k4_ChampionActionBlowHorn:
		if (actionIndex == k8_ChampionActionWarCry) {
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k28_soundWAR_CRY, L1251_i_MapX, L1252_i_MapY, k0_soundModePlayImmediately);
		}
		if (actionIndex == k4_ChampionActionBlowHorn) {
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k25_soundBLOW_HORN, L1251_i_MapX, L1252_i_MapY, k0_soundModePlayImmediately);
		}
		AL1245_B_ActionPerformed = f401_isGroupFrightenedByAction(champIndex, actionIndex, L1251_i_MapX, L1252_i_MapY);
		break;
	case k32_ChampionActionShoot:
		if (Thing(L1247_ps_Champion->_slots[k0_ChampionSlotReadyHand]).getType() != k5_WeaponThingType)
			goto T0407032;
		L1256_ps_WeaponInfoActionHand = &_vm->_dungeonMan->_weaponInfo[L1248_ps_Weapon->getType()];
		L1257_ps_WeaponInfoReadyHand = _vm->_dungeonMan->f158_getWeaponInfo(L1247_ps_Champion->_slots[k0_ChampionSlotReadyHand]);
		AL1246_i_ActionHandWeaponClass = L1256_ps_WeaponInfoActionHand->_class;
		AL1250_i_ReadyHandWeaponClass = L1257_ps_WeaponInfoReadyHand->_class;
		if ((AL1246_i_ActionHandWeaponClass >= k16_WeaponClassFirstBow) && (AL1246_i_ActionHandWeaponClass <= k31_WeaponClassLastBow)) {
			if (AL1250_i_ReadyHandWeaponClass != k10_WeaponClassBowAmmunition)
				goto T0407032;
			AL1246_i_StepEnergy -= k16_WeaponClassFirstBow;
		} else {
			if ((AL1246_i_ActionHandWeaponClass >= k32_WeaponClassFirstSling) && (AL1246_i_ActionHandWeaponClass <= k47_WeaponClassLastSling)) {
				if (AL1250_i_ReadyHandWeaponClass != k11_WeaponClassSlingAmmunition) {
T0407032:
					_g513_actionDamage = kM2_damageNoAmmunition;
					L1255_i_ActionExperienceGain = 0;
					AL1245_B_ActionPerformed = false;
					break;
				}
				AL1246_i_StepEnergy -= k32_WeaponClassFirstSling;
			}
		}
		f406_setChampionDirectionToPartyDirection(L1247_ps_Champion);
		{ // so gotos won't skip init
			Thing AL1250_T_Object = _vm->_championMan->getObjectRemovedFromSlot(champIndex, k0_ChampionSlotReadyHand);
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k1_soundModePlayIfPrioritized);
			_vm->_championMan->championShootProjectile(L1247_ps_Champion, AL1250_T_Object, L1256_ps_WeaponInfoActionHand->_kineticEnergy + L1257_ps_WeaponInfoReadyHand->_kineticEnergy, (L1256_ps_WeaponInfoActionHand->getShootAttack() + _vm->_championMan->getSkillLevel(champIndex, k11_ChampionSkillShoot)) << 1, AL1246_i_StepEnergy);
		}
		break;
	case k5_ChampionActionFlip: {
		const char *messages_EN_ANY[2] = {"IT COMES UP HEADS.", "IT COMES UP TAILS."};
		const char *messages_DE_DEU[2] = {"DIE KOPFSEITE IST OBEN.", "DIE ZAHL IST OBEN."};
		const char *messages_FR_FRA[2] = {"C'EST FACE.", "C'EST PILE."};
		const char **message;
		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: message = messages_EN_ANY; break;
		case Common::DE_DEU: message = messages_DE_DEU; break;
		case Common::FR_FRA: message = messages_FR_FRA; break;
		}
		if (_vm->getRandomNumber(2)) {
			f381_printMessageAfterReplacements(message[0]);
		} else {
			f381_printMessageAfterReplacements(message[1]);
		}
	}
								break;
	case k33_ChampionActionSpellshield:
	case k34_ChampionActionFireshield:
		if (!f403_isPartySpellOrFireShieldSuccessful(L1247_ps_Champion, actionIndex == k33_ChampionActionSpellshield, 280, true)) {
			L1255_i_ActionExperienceGain >>= 2;
			L1249_ui_ActionDisabledTicks >>= 1;
		} else {
			f405_decrementCharges(L1247_ps_Champion);
		}
		break;
	case k27_ChampionActionInvoke:
		AL1250_i_KineticEnergy = _vm->getRandomNumber(128) + 100;
		switch (_vm->getRandomNumber(6)) {
		case 0:
			AL1245_T_ExplosionThing = Thing::_explPoisonBolt.toUint16();
			goto T0407014;
		case 1:
			AL1245_T_ExplosionThing = Thing::_explPoisonCloud.toUint16();
			goto T0407014;
		case 2:
			AL1245_T_ExplosionThing = Thing::_explHarmNonMaterial.toUint16();
			goto T0407014;
		default:
			goto T0407013;
		}
	case k35_ChampionActionFluxcage:
		f406_setChampionDirectionToPartyDirection(L1247_ps_Champion);
		_vm->_groupMan->f224_fluxCageAction(L1251_i_MapX, L1252_i_MapY);
		break;
	case k43_ChampionActionFuse:
		f406_setChampionDirectionToPartyDirection(L1247_ps_Champion);
		L1251_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
		L1252_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
		L1251_i_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], L1252_i_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
		_vm->_groupMan->f225_fuseAction(L1251_i_MapX, L1252_i_MapY);
		break;
	case k36_ChampionActionHeal:
		/* CHANGE2_17_IMPROVEMENT Heal action is much more effective
		Heal cycles occur as long as the champion has missing health and enough mana. Cycle count = Min(Current Mana / 2, Missing health / Min(10, Heal skill level))
		Healing amount is Min(Missing health, Min(10, Heal skill level)) * heal cycle count
		Mana cost is 2 * heal cycle count
		Experience gain is 2 + 2 * heal cycle count */
		if (((AL1250_i_MissingHealth = L1247_ps_Champion->_maxHealth - L1247_ps_Champion->_currHealth) > 0) && L1247_ps_Champion->_currMana) {
			AL1246_i_HealingCapability = MIN((uint16)10, _vm->_championMan->getSkillLevel(champIndex, k13_ChampionSkillHeal));
			L1255_i_ActionExperienceGain = 2;
			do {
				AL1244_ui_HealingAmount = MIN(AL1250_i_MissingHealth, AL1246_i_HealingCapability);
				L1247_ps_Champion->_currHealth += AL1244_ui_HealingAmount;
				L1255_i_ActionExperienceGain += 2;
			} while (((L1247_ps_Champion->_currMana = L1247_ps_Champion->_currMana - 2) > 0) && (AL1250_i_MissingHealth = AL1250_i_MissingHealth - AL1244_ui_HealingAmount));
			if (L1247_ps_Champion->_currMana < 0) {
				L1247_ps_Champion->_currMana = 0;
			}
			setFlag(L1247_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
			AL1245_B_ActionPerformed = true;
		}
		break;
	case k39_ChampionActionWindow:
		AL1246_i_Ticks = _vm->getRandomNumber(_vm->_championMan->getSkillLevel(champIndex, L1254_i_ActionSkillIndex) + 8) + 5;
		L1258_s_Event._priority = 0;
		L1258_s_Event._type = k73_TMEventTypeThievesEye;
		M33_setMapAndTime(L1258_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + AL1246_i_Ticks);
		_vm->_timeline->f238_addEventGetEventIndex(&L1258_s_Event);
		_vm->_championMan->_party._event73Count_ThievesEye++;
		goto T0407076;
	case k10_ChampionActionClimbDown:
		L1251_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
		L1252_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
		L1251_i_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir];
		L1252_i_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
		/* CHANGE6_00_FIX The presence of a group over the pit is checked so that you cannot climb down a pit with the rope if there is a group levitating over it */
		if ((_vm->_dungeonMan->f151_getSquare(L1251_i_MapX, L1252_i_MapY).getType() == k2_PitElemType) && (_vm->_groupMan->f175_groupGetThing(L1251_i_MapX, L1252_i_MapY) == Thing::_endOfList)) {
			/* BUG0_77 The party moves forward when using the rope in front of a closed pit. The engine does not check whether the pit is open before moving the party over the pit. This is not consistent with the behavior when using the rope in front of a corridor where nothing happens */
			_vm->_moveSens->_g402_useRopeToClimbDownPit = true;
			_vm->_moveSens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, L1251_i_MapX, L1252_i_MapY);
			_vm->_moveSens->_g402_useRopeToClimbDownPit = false;
		} else {
			L1249_ui_ActionDisabledTicks = 0;
		}
		break;
	case k11_ChampionActionFreezeLife:
		if (L1248_ps_Weapon->getType() == k42_JunkTypeMagicalBoxBlue) {
			AL1246_i_Ticks = 30;
			goto T0407071;
		}
		if (L1248_ps_Weapon->getType() == k43_JunkTypeMagicalBoxGreen) {
			AL1246_i_Ticks = 125;
T0407071:
			_vm->_championMan->getObjectRemovedFromSlot(champIndex, k1_ChampionSlotActionHand);
			L1248_ps_Weapon->setNextThing(Thing::_none);
		} else {
			AL1246_i_Ticks = 70;
			f405_decrementCharges(L1247_ps_Champion);
		}
		_vm->_championMan->_party._freezeLifeTicks = MIN(200, _vm->_championMan->_party._freezeLifeTicks + AL1246_i_Ticks);
		break;
	case k38_ChampionActionLight:
		_vm->_championMan->_party._magicalLightAmount += _vm->_championMan->_lightPowerToLightAmount[2];
		f404_createEvent70_light(-2, 2500);
T0407076:
		f405_decrementCharges(L1247_ps_Champion);
		break;
	case k42_ChampionActionThrow:
		f406_setChampionDirectionToPartyDirection(L1247_ps_Champion);
		AL1245_B_ActionPerformed = _vm->_championMan->isObjectThrown(champIndex, k1_ChampionSlotActionHand, (L1247_ps_Champion->_cell == returnNextVal(_vm->_dungeonMan->_g308_partyDir)) || (L1247_ps_Champion->_cell == (ViewCell)returnOppositeDir(_vm->_dungeonMan->_g308_partyDir)));
		if (AL1245_B_ActionPerformed) {
			_vm->_timeline->_g370_events[L1247_ps_Champion->_enableActionEventIndex]._B._slotOrdinal = _vm->M0_indexToOrdinal(k1_ChampionSlotActionHand);
		}
	}
	if (L1249_ui_ActionDisabledTicks) {
		_vm->_championMan->disableAction(champIndex, L1249_ui_ActionDisabledTicks);
	}
	if (L1253_i_ActionStamina) {
		_vm->_championMan->decrementStamina(champIndex, L1253_i_ActionStamina);
	}
	if (L1255_i_ActionExperienceGain) {
		_vm->_championMan->addSkillExperience(champIndex, L1254_i_ActionSkillIndex, L1255_i_ActionExperienceGain);
	}
	_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	return AL1245_B_ActionPerformed;
}

void MenuMan::f406_setChampionDirectionToPartyDirection(Champion* champ) {
	if (champ->_dir != _vm->_dungeonMan->_g308_partyDir) {
		champ->_dir = _vm->_dungeonMan->_g308_partyDir;
		setFlag(champ->_attributes, k0x0400_ChampionAttributeIcon);
	}
}

void MenuMan::f405_decrementCharges(Champion* champ) {
	Thing L1242_T_Thing;
	Junk* L1243_ps_Junk;

	L1243_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(L1242_T_Thing = champ->_slots[k1_ChampionSlotActionHand]);
	switch (L1242_T_Thing.getType()) {
	case k5_WeaponThingType:
		if (((Weapon*)L1243_ps_Junk)->getChargeCount()) {
			((Weapon*)L1243_ps_Junk)->setChargeCount(((Weapon*)L1243_ps_Junk)->getChargeCount() - 1);
		}
		break;
	case k6_ArmourThingType:
		if (((Armour*)L1243_ps_Junk)->getChargeCount()) {
			((Armour*)L1243_ps_Junk)->setChargeCount(((Armour*)L1243_ps_Junk)->getChargeCount() - 1);
		}
		break;
	case k10_JunkThingType:
		if (L1243_ps_Junk->getChargeCount()) {
			L1243_ps_Junk->setChargeCount(L1243_ps_Junk->getChargeCount() - 1);
		}
		break;
	default:
		break;
	}
	_vm->_championMan->drawChangedObjectIcons();
}

bool MenuMan::f402_isMeleeActionPerformed(int16 champIndex, Champion* champ, int16 actionIndex, int16 targetMapX, int16 targetMapY, int16 skillIndex) {
	static unsigned char G0492_auc_Graphic560_ActionDamageFactor[44] = {
		0,  /* N */
		15, /* BLOCK */
		48, /* CHOP */
		0,  /* X */
		0,  /* BLOW HORN */
		0,  /* FLIP */
		32, /* PUNCH */
		48, /* KICK */
		0,  /* WAR CRY */
		48, /* STAB */
		0,  /* CLIMB DOWN */
		0,  /* FREEZE LIFE */
		20, /* HIT */
		16, /* SWING */
		60, /* STAB */
		66, /* THRUST */
		8,  /* JAB */
		8,  /* PARRY */
		25, /* HACK */
		96, /* BERZERK */
		0,  /* FIREBALL */
		0,  /* DISPELL */
		0,  /* CONFUSE */
		0,  /* LIGHTNING */
		55, /* DISRUPT */
		60, /* MELEE */
		0,  /* X */
		0,  /* INVOKE */
		16, /* SLASH */
		48, /* CLEAVE */
		50, /* BASH */
		16, /* STUN */
		0,  /* SHOOT */
		0,  /* SPELLSHIELD */
		0,  /* FIRESHIELD */
		0,  /* FLUXCAGE */
		0,  /* HEAL */
		0,  /* CALM */
		0,  /* LIGHT */
		0,  /* WINDOW */
		0,  /* SPIT */
		0,  /* BRANDISH */
		0,  /* THROW */
		0}; /* FUSE */
	static unsigned char G0493_auc_Graphic560_ActionHitProbability[44] = {
		0,  /* N */
		22, /* BLOCK */
		48, /* CHOP */
		0,  /* X */
		0,  /* BLOW HORN */
		0,  /* FLIP */
		38, /* PUNCH */
		28, /* KICK */
		0,  /* WAR CRY */
		30, /* STAB */
		0,  /* CLIMB DOWN */
		0,  /* FREEZE LIFE */
		20, /* HIT */
		32, /* SWING */
		42, /* STAB */
		57, /* THRUST */
		70, /* JAB */
		18, /* PARRY */
		27, /* HACK */
		46, /* BERZERK */
		0,  /* FIREBALL */
		0,  /* DISPELL */
		0,  /* CONFUSE */
		0,  /* LIGHTNING */
		46, /* DISRUPT */
		64, /* MELEE */
		0,  /* X */
		0,  /* INVOKE */
		26, /* SLASH */
		40, /* CLEAVE */
		32, /* BASH */
		50, /* STUN */
		0,  /* SHOOT */
		0,  /* SPELLSHIELD */
		0,  /* FIRESHIELD */
		0,  /* FLUXCAGE */
		0,  /* HEAL */
		0,  /* CALM */
		0,  /* LIGHT */
		0,  /* WINDOW */
		0,  /* SPIT */
		0,  /* BRANDISH */
		0,  /* THROW */
		0}; /* FUSE */

	uint16 L1236_ui_Multiple;
#define AL1236_ui_ChampionCell       L1236_ui_Multiple
#define AL1236_ui_ActionDamageFactor L1236_ui_Multiple
	uint16 L1237_ui_Multiple;
#define AL1237_ui_Direction            L1237_ui_Multiple
#define AL1237_ui_CellDelta            L1237_ui_Multiple
#define AL1237_ui_ActionHitProbability L1237_ui_Multiple
	int16 L1238_i_CreatureOrdinal;

	_vm->_sound->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k1_soundModePlayIfPrioritized);
	if (_g517_actionTargetGroupThing == Thing::_endOfList)
		goto T0402010;
	L1238_i_CreatureOrdinal = _vm->_groupMan->f177_getMeleeTargetCreatureOrdinal(targetMapX, targetMapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, AL1236_ui_ChampionCell = champ->_cell);
	if (L1238_i_CreatureOrdinal) {
		switch (M21_normalizeModulo4(AL1236_ui_ChampionCell + 4 - champ->_dir)) {
		case k2_ViewCellBackRight: /* Champion is on the back right of the square and tries to attack a creature in the front right of its square */
			AL1237_ui_CellDelta = 3;
			goto T0402005;
		case k3_ViewCellBackLeft: /* Champion is on the back left of the square and tries to attack a creature in the front left of its square */
			AL1237_ui_CellDelta = 1;
T0402005: /* Check if there is another champion in front */
			if (_vm->_championMan->getIndexInCell(M21_normalizeModulo4(AL1236_ui_ChampionCell + AL1237_ui_CellDelta)) != kM1_ChampionNone) {
				_g513_actionDamage = kM1_damageCantReach;
				goto T0402010;
			}
		}
		if ((actionIndex == k24_ChampionActionDisrupt) && !getFlag(_vm->_dungeonMan->f144_getCreatureAttributes(_g517_actionTargetGroupThing), k0x0040_MaskCreatureInfo_nonMaterial))
			goto T0402010;
		AL1237_ui_ActionHitProbability = G0493_auc_Graphic560_ActionHitProbability[actionIndex];
		AL1236_ui_ActionDamageFactor = G0492_auc_Graphic560_ActionDamageFactor[actionIndex];
		if ((_vm->_objectMan->f33_getIconIndex(champ->_slots[k1_ChampionSlotActionHand]) == k40_IconIndiceWeaponVorpalBlade) || (actionIndex == k24_ChampionActionDisrupt)) {
			setFlag(AL1237_ui_ActionHitProbability, k0x8000_hitNonMaterialCreatures);
		}
		_g513_actionDamage = _vm->_groupMan->f231_getMeleeActionDamage(champ, champIndex, (Group*)_vm->_dungeonMan->f156_getThingData(_g517_actionTargetGroupThing), _vm->M1_ordinalToIndex(L1238_i_CreatureOrdinal), targetMapX, targetMapY, AL1237_ui_ActionHitProbability, AL1236_ui_ActionDamageFactor, skillIndex);
		return true;
	}
T0402010:
	return false;
}

bool MenuMan::f401_isGroupFrightenedByAction(int16 champIndex, uint16 actionIndex, int16 mapX, int16 mapY) {
	int16 L1229_i_FrightAmount = 0;
	uint16 L1230_ui_FearResistance;
	uint16 L1231_ui_Experience = 0;
	bool L1232_B_IsGroupFrightenedByAction;
	Group* L1233_ps_Group;
	CreatureInfo* L1234_ps_CreatureInfo;
	ActiveGroup* L1235_ps_ActiveGroup;


	L1232_B_IsGroupFrightenedByAction = false;
	if (_g517_actionTargetGroupThing == Thing::_endOfList)
		goto T0401016;
	switch (actionIndex) {
	case k8_ChampionActionWarCry:
		L1229_i_FrightAmount = 3;
		L1231_ui_Experience = 12; /* War Cry gives experience in priest skill k14_ChampionSkillInfluence below. The War Cry action also has an experience gain of 7 defined in G0497_auc_Graphic560_ActionExperienceGain in the same skill (versions 1.1 and below) or in the fighter skill k7_ChampionSkillParry (versions 1.2 and above). In versions 1.2 and above, this is the only action that gives experience in two skills */
		break;
	case k37_ChampionActionCalm:
		L1229_i_FrightAmount = 7;
		L1231_ui_Experience = 35;
		break;
	case k41_ChampionActionBrandish:
		L1229_i_FrightAmount = 6;
		L1231_ui_Experience = 30;
		break;
	case k4_ChampionActionBlowHorn:
		L1229_i_FrightAmount = 6;
		L1231_ui_Experience = 20;
		break;
	case k22_ChampionActionConfuse:
		L1229_i_FrightAmount = 12;
		L1231_ui_Experience = 45;
	}
	L1229_i_FrightAmount += _vm->_championMan->getSkillLevel(champIndex, k14_ChampionSkillInfluence);
	L1233_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(_g517_actionTargetGroupThing);
	L1234_ps_CreatureInfo = &g243_CreatureInfo[L1233_ps_Group->_type];
	if (((L1230_ui_FearResistance = L1234_ps_CreatureInfo->M57_getFearResistance()) > _vm->getRandomNumber(L1229_i_FrightAmount)) || (L1230_ui_FearResistance == k15_immuneToFear)) {
		L1231_ui_Experience >>= 1;
	} else {
		L1235_ps_ActiveGroup = &_vm->_groupMan->_g375_activeGroups[L1233_ps_Group->getActiveGroupIndex()];
		if (L1233_ps_Group->getBehaviour() == k6_behavior_ATTACK) {
			_vm->_groupMan->f182_stopAttacking(L1235_ps_ActiveGroup, mapX, mapY);
			_vm->_groupMan->f180_startWanedring(mapX, mapY);
		}
		L1233_ps_Group->setBehaviour(k5_behavior_FLEE);
		L1235_ps_ActiveGroup->_delayFleeingFromTarget = ((16 - L1230_ui_FearResistance) << 2) / L1234_ps_CreatureInfo->_movementTicks;
		L1232_B_IsGroupFrightenedByAction = true;
	}
	_vm->_championMan->addSkillExperience(champIndex, k14_ChampionSkillInfluence, L1231_ui_Experience);
T0401016:
	return L1232_B_IsGroupFrightenedByAction;
}

void MenuMan::f381_printMessageAfterReplacements(const char* str) {
	char* L1164_pc_Character;
	char* L1165_pc_ReplacementString;
	char L1166_ac_OutputString[128];


	L1164_pc_Character = L1166_ac_OutputString;
	*L1164_pc_Character++ = '\n'; /* New line */
	do {
		if (*str == '@') {
			str++;
			if (*(L1164_pc_Character - 1) != '\n') { /* New line */
				*L1164_pc_Character++ = ' ';
			}
			switch (*str) {
			case 'p': /* '@p' in the source string is replaced by the champion name followed by a space */
				L1165_pc_ReplacementString = _vm->_championMan->_champions[_vm->M1_ordinalToIndex(_vm->_championMan->_actingChampionOrdinal)]._name;
			}
			*L1164_pc_Character = '\0';
			strcat(L1166_ac_OutputString, L1165_pc_ReplacementString);
			L1164_pc_Character += strlen(L1165_pc_ReplacementString);
			*L1164_pc_Character++ = ' ';
		} else {
			*L1164_pc_Character++ = *str;
		}
	} while (*str++);
	*L1164_pc_Character = '\0';
	if (L1166_ac_OutputString[1]) { /* If the string is not empty (the first character is a new line \n) */
		_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, L1166_ac_OutputString);
	}
}

void MenuMan::f389_processCommands116To119_setActingChampion(uint16 champIndex) {
	static ActionSet G0489_as_Graphic560_ActionSets[44] = {
		/* { ActionIndices[0], ActionIndices[1], ActionIndices[2], ActionProperties[0], ActionProperties[1], Useless } */
		ActionSet(255, 255, 255, 0x00, 0x00),
		ActionSet(27,  43,  35, 0x00, 0x00),
		ActionSet(6,   7,   8, 0x00, 0x00),
		ActionSet(0,   0,   0, 0x00, 0x00),
		ActionSet(0,   0,   0, 0x00, 0x00),
		ActionSet(13, 255, 255, 0x00, 0x00),
		ActionSet(13,  20, 255, 0x87, 0x00),
		ActionSet(13,  23, 255, 0x83, 0x00),
		ActionSet(28,  41,  22, 0x02, 0x83),
		ActionSet(16,   2,  23, 0x00, 0x84),
		ActionSet(2,  25,  20, 0x02, 0x86),
		ActionSet(17,  41,  34, 0x03, 0x05),
		ActionSet(42,   9,  28, 0x00, 0x02),
		ActionSet(13,  17,   2, 0x02, 0x03),
		ActionSet(16,  17,  15, 0x01, 0x05),
		ActionSet(28,  17,  25, 0x01, 0x05),
		ActionSet(2,  25,  15, 0x05, 0x06),
		ActionSet(9,   2,  29, 0x02, 0x05),
		ActionSet(16,  29,  24, 0x02, 0x04),
		ActionSet(13,  15,  19, 0x05, 0x07),
		ActionSet(13,   2,  25, 0x00, 0x05),
		ActionSet(2,  29,  19, 0x03, 0x08),
		ActionSet(13,  30,  31, 0x02, 0x04),
		ActionSet(13,  31,  25, 0x03, 0x06),
		ActionSet(42,  30, 255, 0x00, 0x00),
		ActionSet(0,   0,   0, 0x00, 0x00),
		ActionSet(42,   9, 255, 0x00, 0x00),
		ActionSet(32, 255, 255, 0x00, 0x00),
		ActionSet(37,  33,  36, 0x82, 0x03),
		ActionSet(37,  33,  34, 0x83, 0x84),
		ActionSet(17,  38,  21, 0x80, 0x83),
		ActionSet(13,  21,  34, 0x83, 0x84),
		ActionSet(36,  37,  41, 0x02, 0x03),
		ActionSet(13,  23,  39, 0x82, 0x84),
		ActionSet(13,  17,  40, 0x00, 0x83),
		ActionSet(17,  36,  38, 0x03, 0x84),
		ActionSet(4, 255, 255, 0x00, 0x00),
		ActionSet(5, 255, 255, 0x00, 0x00),
		ActionSet(11, 255, 255, 0x00, 0x00),
		ActionSet(10, 255, 255, 0x00, 0x00),
		ActionSet(42,   9, 255, 0x00, 0x00),
		ActionSet(1,  12, 255, 0x02, 0x00),
		ActionSet(42, 255, 255, 0x00, 0x00),
		ActionSet(6,  11, 255, 0x80, 0x00)};
	uint16 L1188_ui_ActionSetIndex;
	Thing L1189_T_Thing;
	Champion* L1190_ps_Champion;
	ActionSet* L1191_ps_ActionSet;


	L1190_ps_Champion = &_vm->_championMan->_champions[champIndex];
	if (getFlag(L1190_ps_Champion->_attributes, k0x0008_ChampionAttributeDisableAction) || !L1190_ps_Champion->_currHealth) {
		return;
	}
	if ((L1189_T_Thing = L1190_ps_Champion->_slots[k1_ChampionSlotActionHand]) == Thing::_none) {
		L1188_ui_ActionSetIndex = 2; /* Actions Punch, Kick and War Cry */
	} else if ((L1188_ui_ActionSetIndex = _vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(L1189_T_Thing)]._actionSetIndex) == 0) {
		return;
	}
	L1191_ps_ActionSet = &G0489_as_Graphic560_ActionSets[L1188_ui_ActionSetIndex];
	_vm->_championMan->_actingChampionOrdinal = _vm->M0_indexToOrdinal(champIndex);
	f383_setActionList(L1191_ps_ActionSet);
	_g509_actionAreaContainsIcons = false;
	setFlag(L1190_ps_Champion->_attributes, k0x8000_ChampionAttributeActionHand);
	_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	f387_drawActionArea();
	f387_drawActionArea();
}

void MenuMan::f383_setActionList(ActionSet* actionSet) {

#define k0x0080_actionRequiresCharge 0x0080 // @ MASK0x0080_ACTION_REQUIRES_CHARGE 

	uint16 L1169_ui_ActionListIndex;
	uint16 L1170_ui_NextAvailableActionListIndex;
	uint16 L1171_ui_ActionIndex;
	uint16 L1172_ui_MinimumSkillLevel;

	_g713_actionList._actionIndices[0] = (ChampionAction)actionSet->_actionIndices[0];
	_g713_actionList._minimumSkillLevel[0] = 1;
	L1170_ui_NextAvailableActionListIndex = 1;
	for (L1169_ui_ActionListIndex = 1; L1169_ui_ActionListIndex < 3; L1169_ui_ActionListIndex++) {
		if ((L1171_ui_ActionIndex = actionSet->_actionIndices[L1169_ui_ActionListIndex]) == k255_ChampionActionNone)
			continue;
		if (getFlag(L1172_ui_MinimumSkillLevel = actionSet->_actionProperties[L1169_ui_ActionListIndex - 1], k0x0080_actionRequiresCharge) && !f382_getActionObjectChargeCount())
			continue;
		clearFlag(L1172_ui_MinimumSkillLevel, k0x0080_actionRequiresCharge);
		if (_vm->_championMan->getSkillLevel(_vm->M1_ordinalToIndex(_vm->_championMan->_actingChampionOrdinal), g496_ActionSkillIndex[L1171_ui_ActionIndex]) >= L1172_ui_MinimumSkillLevel) {
			_g713_actionList._actionIndices[L1170_ui_NextAvailableActionListIndex] = (ChampionAction)L1171_ui_ActionIndex;
			_g713_actionList._minimumSkillLevel[L1170_ui_NextAvailableActionListIndex] = L1172_ui_MinimumSkillLevel;
			L1170_ui_NextAvailableActionListIndex++;
		}
	}
	_g507_actionCount = L1170_ui_NextAvailableActionListIndex;
	for (L1169_ui_ActionListIndex = L1170_ui_NextAvailableActionListIndex; L1169_ui_ActionListIndex < 3; L1169_ui_ActionListIndex++) {
		_g713_actionList._actionIndices[L1169_ui_ActionListIndex] = k255_ChampionActionNone;
	}
}

int16 MenuMan::f382_getActionObjectChargeCount() {
	Thing L1167_T_Thing;
	Junk* L1168_ps_Junk;


	L1168_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(L1167_T_Thing = _vm->_championMan->_champions[_vm->M1_ordinalToIndex(_vm->_championMan->_actingChampionOrdinal)]._slots[k1_ChampionSlotActionHand]);
	switch (L1167_T_Thing.getType()) {
	case k5_WeaponThingType:
		return ((Weapon*)L1168_ps_Junk)->getChargeCount();
	case k6_ArmourThingType:
		return ((Armour*)L1168_ps_Junk)->getChargeCount();
	case k10_JunkThingType:
		return L1168_ps_Junk->getChargeCount();
	default:
		return 1;
	}
}

void MenuMan::f385_drawActionDamage(int16 damage) {
	static const Box G0502_s_Graphic560_Box_ActionAreaMediumDamage = Box(242, 305, 81, 117);
	static const Box G0503_s_Graphic560_Box_ActionAreaSmallDamage = Box(251, 292, 81, 117);

	uint16 L1174_ui_Multiple;
#define AL1174_ui_DerivedBitmapIndex L1174_ui_Multiple
#define AL1174_ui_CharacterIndex     L1174_ui_Multiple
	int16 L1175_i_ByteWidth;
	int16 L1176_i_Multiple;
#define AL1176_i_X          L1176_i_Multiple
#define AL1176_i_PixelWidth L1176_i_Multiple
	byte* L1177_puc_Bitmap;
	unsigned char* L1178_puc_Multiple;
#define AL1178_puc_String L1178_puc_Multiple
#define AL1178_puc_Bitmap L1178_puc_Multiple
	char L1179_ac_String[6];
	const Box* L1180_ps_Box;
	int16 L1643_i_Width;

	_vm->_eventMan->f78_showMouse();
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_displayMan->D24_fillScreenBox(boxActionArea, k0_ColorBlack);
	if (damage < 0) {
		static const char *messages_EN_ANY[2] = {"CAN'T REACH", "NEED AMMO"};
		static const char *messages_DE_DEU[2] = {"ZU WEIT WEG", "MEHR MUNITION"};
		static const char *messages_FR_FRA[2] = {"TROP LOIN", "SANS MUNITION"};
		static int16  pos_EN_ANY[2] = {242, 248};
		static int16  pos_DE_DEU[2] = {242, 236};
		static int16  pos_FR_FRA[2] = {248, 236};
		const char **message;
		int16 *pos;
		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: message = messages_EN_ANY; pos = pos_EN_ANY;  break;
		case Common::DE_DEU: message = messages_DE_DEU; pos = pos_DE_DEU; break;
		case Common::FR_FRA: message = messages_FR_FRA; pos = pos_FR_FRA; break;
		}

		if (damage == kM1_damageCantReach) {
			AL1176_i_X = pos[0];
			AL1178_puc_String = (byte*)message[0];
		} else {
			AL1176_i_X = pos[1];
			AL1178_puc_String = (byte*)message[1];
		}
		_vm->_textMan->f53_printToLogicalScreen(AL1176_i_X, 100, k4_ColorCyan, k0_ColorBlack, (char *)AL1178_puc_String);
	} else {
		if (damage > 40) {
			L1180_ps_Box = &boxActionArea3ActionMenu;
			L1177_puc_Bitmap = _vm->_displayMan->f489_getNativeBitmapOrGraphic(k14_damageToCreatureIndice);
			L1175_i_ByteWidth = k48_byteWidth;
			L1643_i_Width = 45;
		} else {
			if (damage > 15) {
				AL1174_ui_DerivedBitmapIndex = k2_DerivedBitmapDamageToCreatureMedium;
				AL1176_i_PixelWidth = 64;
				L1175_i_ByteWidth = k32_byteWidth;
				L1180_ps_Box = &G0502_s_Graphic560_Box_ActionAreaMediumDamage;
			} else {
				AL1174_ui_DerivedBitmapIndex = k3_DerivedBitmapDamageToCreatureSmall;
				AL1176_i_PixelWidth = 42;
				L1175_i_ByteWidth = k24_byteWidth;
				L1180_ps_Box = &G0503_s_Graphic560_Box_ActionAreaSmallDamage;
			}
			L1643_i_Width = 37;
			if (!_vm->_displayMan->f491_isDerivedBitmapInCache(AL1174_ui_DerivedBitmapIndex)) {
				AL1178_puc_Bitmap = _vm->_displayMan->f489_getNativeBitmapOrGraphic(k14_damageToCreatureIndice);
				L1177_puc_Bitmap = _vm->_displayMan->f492_getDerivedBitmap(AL1174_ui_DerivedBitmapIndex);
				_vm->_displayMan->f129_blitToBitmapShrinkWithPalChange(AL1178_puc_Bitmap, L1177_puc_Bitmap, 96, 45, AL1176_i_PixelWidth, 37, _vm->_displayMan->_palChangesNoChanges);
				_vm->_displayMan->f493_addDerivedBitmap(AL1174_ui_DerivedBitmapIndex);
			} else {
				L1177_puc_Bitmap = _vm->_displayMan->f492_getDerivedBitmap(AL1174_ui_DerivedBitmapIndex);
			}
		}
		_vm->_displayMan->f21_blitToScreen(L1177_puc_Bitmap, (int16 *)L1180_ps_Box, L1175_i_ByteWidth, kM1_ColorNoTransparency, L1643_i_Width);
		/* Convert damage value to string */
		AL1174_ui_CharacterIndex = 5;
		AL1176_i_X = 274;
		L1179_ac_String[5] = '\0';
		do {
			L1179_ac_String[--AL1174_ui_CharacterIndex] = '0' + (damage % 10);
			AL1176_i_X -= 3;
		} while (damage /= 10);
		_vm->_textMan->f53_printToLogicalScreen(AL1176_i_X, 100, k4_ColorCyan, k0_ColorBlack, &L1179_ac_String[AL1174_ui_CharacterIndex]);
	}
	_vm->_eventMan->f77_hideMouse();
}
}
