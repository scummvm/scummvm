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
	_g713_actionList.resetToZero();
	_gK72_bitmapSpellAreaLine = new byte[96 * 12];
}

MenuMan::~MenuMan() {
	delete[] _gK72_bitmapSpellAreaLine;
}

void MenuMan::f395_drawMovementArrows() {
	DisplayMan &disp = *_vm->_displayMan;
	byte *arrowsBitmap = disp.f489_getNativeBitmapOrGraphic(k13_MovementArrowsIndice);
	Box &dest = g2_BoxMovementArrows;
	uint16 byteWidth = disp.getPixelWidth(k13_MovementArrowsIndice) / 2;

	disp.f132_blitToBitmap(arrowsBitmap, disp._g348_bitmapScreen, dest, 0, 0, byteWidth, k160_byteWidthScreen, kM1_ColorNoTransparency);
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
	} else if (g237_ObjectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing);
	} else {
		dm.f134_fillBitmap(bitmapIcon, k4_ColorCyan, 16, 16);
		goto T0386006;
	}
	_vm->_objectMan->f36_extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.f129_blitToBitmapShrinkWithPalChange(bitmapIcon, bitmapIcon, 16, 16, 16, 16, g498_PalChangesActionAreaObjectIcon);
T0386006:
	dm.D24_fillScreenBox(box, k4_ColorCyan);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2;
	box2._y1 = 95;
	box2._y2 = 110;
	dm.f132_blitToBitmap(bitmapIcon, dm._g348_bitmapScreen, box2, 0, 0, 8, k160_byteWidthScreen, kM1_ColorNoTransparency);
	if (champion.getAttributes(k0x0008_ChampionAttributeDisableAction) || _vm->_championMan->_g299_candidateChampionOrdinal || _vm->_championMan->_g300_partyIsSleeping) {
		warning(false, "MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}
}

void MenuMan::f456_drawDisabledMenu() {
	if (!_vm->_championMan->_g300_partyIsSleeping) {
		warning(false, "MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
			if (_vm->_inventoryMan->_g424_panelContent == k4_PanelContentChest) {
				_vm->_inventoryMan->f334_closeChest();
			}
		} else {
			warning(false, "MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		}
		warning(false, "MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning(false, "MISSING CODE: F0136_VIDEO_ShadeScreenBox");
		warning(false, "MISSING CODE: F0067_MOUSE_SetPointerToNormal");
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
				warning(false, "MISSING CODE: F0385_MENUS_DrawActionDamage");
				_g513_actionDamage = 0;
			} else {
				_g509_actionAreaContainsIcons = true;
				f387_drawActionArea();
			}
		} else {
			_g509_actionAreaContainsIcons = false;
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			champMan.f292_drawChampionState((ChampionIndex)_vm->M1_ordinalToIndex(champMan._g506_actingChampionOrdinal));
			warning(false, "MISSING CODE: F0387_MENUS_DrawActionArea");
		}
	}
}

#define k7_ChampionNameMaximumLength 7 // @ C007_CHAMPION_NAME_MAXIMUM_LENGTH
#define k12_ActionNameMaximumLength 12 // @ C012_ACTION_NAME_MAXIMUM_LENGTH

void MenuMan::f387_drawActionArea() {
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	warning(false, "MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
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
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k10_MenuActionAreaIndice), dispMan._g348_bitmapScreen,
								  box, 0, 0, 48, k160_byteWidthScreen, kM1_ColorNoTransparency);
		textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen,
											235, 83, k0_ColorBlack, k4_ColorCyan, champMan._gK71_champions[_vm->M1_ordinalToIndex(champMan._g506_actingChampionOrdinal)]._name,
											k7_ChampionNameMaximumLength, k200_heightScreen);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.f41_printWithTrailingSpaces(dispMan._g348_bitmapScreen, k160_byteWidthScreen, 241, 93 + actionListIndex * 12, k4_ColorCyan, k0_ColorBlack,
												f384_getActionName(_g713_actionList._actionIndices[actionListIndex]),
												k12_ActionNameMaximumLength, k200_heightScreen);
		}
	}
	warning(false, "MISSING CODE: F0078_MOUSE_ShowPointer");
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
	warning(false, "MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	dispMan.D24_fillScreenBox(g504_BoxSpellAreaControls, k0_ColorBlack);
	int16 champCount = champMan._g305_partyChampionCount;
	switch (champIndex) {
	case k0_ChampionFirst:
		warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(235, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		if (champCount) {
			if (champCurrHealth[1]) {
				warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
			}
labelChamp2:
			if (champCount > 2) {
				if (champCurrHealth[2]) {
					warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
				}
labelChamp3:
				if (champCount > 3) {
					if (champCurrHealth[3]) {
						warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
					}
				}
			}
		}
		break;
	case k1_ChampionSecond:
		if (champCurrHealth[0]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(249, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		goto labelChamp2;
	case k2_ChampionThird:
		if (champCurrHealth[0]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(263, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		goto labelChamp3;
	case k3_ChampionFourth:
		if (champCurrHealth[0]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[1]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		if (champCurrHealth[2]) {
			warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		}
		warning(false, "MISSING CODE: F0006_MAIN_HighlightScreenBox");
		textMan.f53_printToLogicalScreen(277, 48, k0_ColorBlack, k4_ColorCyan, champ._name);
		break;
	}
	warning(false, "MISSING CODE: F0078_MOUSE_ShowPointer");
}

#define k2_SpellAreaAvailableSymbols 2 // @ C2_SPELL_AREA_AVAILABLE_SYMBOLS
#define k3_SpellAreaChampionSymbols 3 // @ C3_SPELL_AREA_CHAMPION_SYMBOLS

void MenuMan::f392_buildSpellAreaLine(int16 spellAreaBitmapLine) {
	DisplayMan &dispMan = *_vm->_displayMan;

	Champion &champ = _vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
	if (spellAreaBitmapLine == k2_SpellAreaAvailableSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k11_MenuSpellAreLinesIndice), _gK72_bitmapSpellAreaLine,
								  gK74_BoxSpellAreaLine, 0, 12, 48, 48, kM1_ColorNoTransparency);
		int16 x = 1;
		byte c = 96 + (6 * champ._symbolStep);
		char spellSymbolString[2] = {'\0', '\0'};
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			spellSymbolString[0] = c++;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 48, x += 14, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == k3_SpellAreaChampionSymbols) {
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k11_MenuSpellAreLinesIndice), _gK72_bitmapSpellAreaLine,
								  gK74_BoxSpellAreaLine, 0, 24, 48, 48, kM1_ColorNoTransparency);
		char spellSymbolString[2] = {'\0', '\0'};
		int16 x = 8;
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((spellSymbolString[0] = champ._symbols[symbolIndex]) == '\0')
				break;
			_vm->_textMan->f40_printTextToBitmap(_gK72_bitmapSpellAreaLine, 48, x += 9, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
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
		warning(false, "MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k9_MenuSpellAreaBackground), dispMan._g348_bitmapScreen, g0_BoxSpellArea, 0, 0,
								  48, k160_byteWidthScreen, kM1_ColorNoTransparency);
		warning(false, "MISSING CODE: F0078_MOUSE_ShowPointer");
	}
	if (champIndex == kM1_ChampionNone) {
		champMan._g514_magicCasterChampionIndex = kM1_ChampionNone;
		warning(false, "MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.D24_fillScreenBox(g0_BoxSpellArea, k0_ColorBlack);
		warning(false, "MISSING CODE: F0078_MOUSE_ShowPointer");
		return;
	}

	champMan._g514_magicCasterChampionIndex = (ChampionIndex)champIndex;
	f392_buildSpellAreaLine(k2_SpellAreaAvailableSymbols);
	warning(false, "MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	f393_drawSpellAreaControls((ChampionIndex)champIndex);
	dispMan.f132_blitToBitmap(_gK72_bitmapSpellAreaLine, dispMan._g348_bitmapScreen, gK75_BoxSpellAreaLine2, 0, 0, 48, k160_byteWidthScreen, kM1_ColorNoTransparency);
	f392_buildSpellAreaLine(k3_SpellAreaChampionSymbols);
	dispMan.f132_blitToBitmap(_gK72_bitmapSpellAreaLine, dispMan._g348_bitmapScreen, gK76_BoxSpellAreaLine3, 0, 0, 48, k160_byteWidthScreen, kM1_ColorNoTransparency);
	warning(false, "MISSING CODE: F0078_MOUSE_ShowPointer");
}

void MenuMan::f457_drawEnabledMenus() {
	int16 L1462_i_Multiple;
#define AL1462_i_MagicCasterChampionIndex L1462_i_Multiple
#define AL1462_i_InventoryChampionOrdinal L1462_i_Multiple


	if (_vm->_championMan->_g300_partyIsSleeping) {
		_vm->_eventMan->f379_drawSleepScreen();
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
	} else {
		AL1462_i_MagicCasterChampionIndex = _vm->_championMan->_g514_magicCasterChampionIndex;
		_vm->_championMan->_g514_magicCasterChampionIndex = kM1_ChampionNone; /* Force next function to draw the spell area */
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(AL1462_i_MagicCasterChampionIndex);
		if (!_vm->_championMan->_g506_actingChampionOrdinal) {
			_vm->_menuMan->_g509_actionAreaContainsIcons = true;
		}
		_vm->_menuMan->f387_drawActionArea();
		if (AL1462_i_InventoryChampionOrdinal = _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
			_vm->_inventoryMan->_g432_inventoryChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
			_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)_vm->M1_ordinalToIndex(AL1462_i_InventoryChampionOrdinal));
		} else {
			_vm->_displayMan->f98_drawFloorAndCeiling();
			_vm->_menuMan->f395_drawMovementArrows();
		}
		_vm->_eventMan->f69_setMousePointer();
	}
}

int16 MenuMan::f408_getClickOnSpellCastResult() {
	int16 L1259_i_SpellCastResult;
	Champion* L1260_ps_Champion;


	L1260_ps_Champion = &_vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
	_vm->_eventMan->f78_showMouse();
	warning(false, "MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
	if ((L1259_i_SpellCastResult = f412_getChampionSpellCastResult(_vm->_championMan->_g514_magicCasterChampionIndex)) != k3_spellCastFailureNeedsFlask) {
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


	if (champIndex >= _vm->_championMan->_g305_partyChampionCount) {
		return k0_spellCastFailure;
	}
	L1270_ps_Champion = &_vm->_championMan->_gK71_champions[champIndex];
	if (!(L1270_ps_Champion->_currHealth)) {
		return k0_spellCastFailure;
	}
	if ((L1271_ps_Spell = f409_getSpellFromSymbols((unsigned char *)L1270_ps_Champion->_symbols)) == 0) {
		f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k1_spellCastSuccess, 0);
		return k0_spellCastFailure;
	}
	L1268_i_PowerSymbolOrdinal = L1270_ps_Champion->_symbols[0] - '_'; /* Values 1 to 6 */
	L1273_ui_Experience = _vm->getRandomNumber(8) + ((AL1269_ui_RequiredSkillLevel = L1271_ps_Spell->_baseRequiredSkillLevel + L1268_i_PowerSymbolOrdinal) << 4) + ((_vm->M1_ordinalToIndex(L1268_i_PowerSymbolOrdinal) * L1271_ps_Spell->_baseRequiredSkillLevel) << 3) + (AL1269_ui_RequiredSkillLevel * AL1269_ui_RequiredSkillLevel);
	AL1267_ui_SkillLevel = _vm->_championMan->f303_getSkillLevel(champIndex, L1271_ps_Spell->_skillIndex);
	if (AL1267_ui_SkillLevel < AL1269_ui_RequiredSkillLevel) {
		L1274_i_MissingSkillLevelCount = AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel;
		while (L1274_i_MissingSkillLevelCount--) {
			if (_vm->getRandomNumber(128) > MIN(L1270_ps_Champion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent] + 15, 115)) {
				_vm->_championMan->f304_addSkillExperience(champIndex, L1271_ps_Spell->_skillIndex, L1273_ui_Experience >> (AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel));
				f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k0_failureNeedsMorePractice, L1271_ps_Spell->_skillIndex);
				return k0_spellCastFailure;
			}
		}
	}
	switch (L1271_ps_Spell->M67_spellKind()) {
	case k1_spellKindPotion:
		if ((L1275_ps_Potion = f411_getEmptyFlaskInHand(L1270_ps_Champion, &L1272_T_Object)) == NULL) {
			f410_menusPrintSpellFailureMessage(L1270_ps_Champion, k10_failureNeedsFlaskInHand, 0);
			return k3_spellCastFailureNeedsFlask;
		}
		AL1269_ui_EmptyFlaskWeight = _vm->_dungeonMan->f140_getObjectWeight(L1272_T_Object);
		L1275_ps_Potion->setType((PotionType)L1271_ps_Spell->M68_spellType());
		L1275_ps_Potion->setPower(_vm->getRandomNumber(16) + (L1268_i_PowerSymbolOrdinal * 40));
		L1270_ps_Champion->_load += _vm->_dungeonMan->f140_getObjectWeight(L1272_T_Object) - AL1269_ui_EmptyFlaskWeight;
		_vm->_championMan->f296_drawChangedObjectIcons();
		if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal == _vm->M0_indexToOrdinal(champIndex)) {
			setFlag(L1270_ps_Champion->_attributes, k0x0200_ChampionAttributeLoad);
			_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
		}
		break;
	case k2_spellKindProjectile:
		if (L1270_ps_Champion->_dir != _vm->_dungeonMan->_g308_partyDir) {
			L1270_ps_Champion->_dir = _vm->_dungeonMan->_g308_partyDir;
			setFlag(L1270_ps_Champion->_attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
		}
		if (L1271_ps_Spell->M68_spellType() == k4_spellType_projectileOpenDoor) {
			AL1267_ui_SkillLevel <<= 1;
		}
		_vm->_championMan->f327_isProjectileSpellCast(champIndex, Thing(L1271_ps_Spell->M68_spellType() + Thing::_firstExplosion.toUint16()), f26_getBoundedValue(21, (L1268_i_PowerSymbolOrdinal + 2) * (4 + (AL1267_ui_SkillLevel << 1)), 255), 0);
		break;
	case k3_spellKindOther:
		L1276_s_Event._priority = 0;
		AL1267_ui_SpellPower = (L1268_i_PowerSymbolOrdinal + 1) << 2;
		switch (L1271_ps_Spell->M68_spellType()) {
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
			_vm->_championMan->_g407_party._magicalLightAmount += g39_LightPowerToLightAmount[AL1267_ui_LightPower];
			f404_createEvent70_light(-AL1267_ui_LightPower, AL1269_ui_Ticks);
			break;
		case k1_spellType_otherDarkness:
			AL1267_ui_LightPower = (AL1267_ui_SpellPower >> 2);
			_vm->_championMan->_g407_party._magicalLightAmount -= g39_LightPowerToLightAmount[AL1267_ui_LightPower];
			f404_createEvent70_light(AL1267_ui_LightPower, 98);
			break;
		case k2_spellType_otherThievesEye:
			L1276_s_Event._type = k73_TMEventTypeThievesEye;
			_vm->_championMan->_g407_party._event73Count_ThievesEye++;
			AL1267_ui_SpellPower = (AL1267_ui_SpellPower >> 1);
			goto T0412032;
		case k3_spellType_otherInvisibility:
			L1276_s_Event._type = k71_TMEventTypeInvisibility;
			_vm->_championMan->_g407_party._event71Count_Invisibility++;
			goto T0412033;
		case k4_spellType_otherPartyShield:
			L1276_s_Event._type = k74_TMEventTypePartyShield;
			L1276_s_Event._B._defense = AL1267_ui_SpellPower;
			if (_vm->_championMan->_g407_party._shieldDefense > 50) {
				L1276_s_Event._B._defense >>= 2;
			}
			_vm->_championMan->_g407_party._shieldDefense += L1276_s_Event._B._defense;
			_vm->_timeline->f260_timelineRefreshAllChampionStatusBoxes();
			goto T0412032;
		case k6_spellType_otherFootprints:
			L1276_s_Event._type = k79_TMEventTypeFootprints;
			_vm->_championMan->_g407_party._event79Count_Footprints++;
			_vm->_championMan->_g407_party._firstScentIndex = _vm->_championMan->_g407_party._scentCount;
			if (L1268_i_PowerSymbolOrdinal < 3) {
				_vm->_championMan->_g407_party._lastScentIndex = _vm->_championMan->_g407_party._firstScentIndex;
			} else {
				_vm->_championMan->_g407_party._lastScentIndex = 0;
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
				_vm->_championMan->f301_addObjectInSlot((ChampionIndex)champIndex, L1272_T_Object, AL1267_ui_SlotIndex);
				_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
			} else {
				_vm->_movsens->f267_getMoveResult(L1272_T_Object, kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
			}
			break;
		case k8_spellType_otherFireshield:
			f403_isPartySpellOrFireShieldSuccessful(L1270_ps_Champion, false, (AL1267_ui_SpellPower * AL1267_ui_SpellPower) + 100, false);
		}
	}
	_vm->_championMan->f304_addSkillExperience(champIndex, L1271_ps_Spell->_skillIndex, L1273_ui_Experience);
	_vm->_championMan->f330_disableAction(champIndex, L1271_ps_Spell->M69_spellDurration());
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
		Spell(0x006B6E76, 0,  3, 0x3C73)};


	int32 L1261_l_Symbols;
	int16 L1262_i_Multiple;
#define AL1262_i_BitShiftCount L1262_i_Multiple
#define AL1262_i_SpellIndex    L1262_i_Multiple
	Spell* L1263_ps_Spell;


	if (*(symbols + 1)) {
		AL1262_i_BitShiftCount = 24;
		L1261_l_Symbols = 0;
		do {
			L1261_l_Symbols |= (long)*symbols++ << AL1262_i_BitShiftCount;
		} while (*symbols && ((AL1262_i_BitShiftCount -= 8) >= 0));
		L1263_ps_Spell = G0487_as_Graphic560_Spells;
		AL1262_i_SpellIndex = 25;
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
	char* L1264_pc_Message = nullptr;

	if (skillIndex > k3_ChampionSkillWizard) {
		skillIndex = (skillIndex - 4) / 4;
	}
	_vm->_textMan->f51_messageAreaPrintLineFeed();
	_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, champ->_name);
	switch (failureType) {
	case k0_failureNeedsMorePractice:
		// TODO: localization
		_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, " NEEDS MORE PRACTICE WITH THIS ");
		_vm->_textMan->f47_messageAreaPrintMessage(k4_ColorCyan, g417_baseSkillName[skillIndex]);
		L1264_pc_Message = " SPELL.";
		break;
	case k1_failureMeaninglessSpell:
		L1264_pc_Message = " MUMBLES A MEANINGLESS SPELL."; // TODO: localization
		break;
	case k10_failureNeedsFlaskInHand:
		L1264_pc_Message = " NEEDS AN EMPTY FLASK IN HAND FOR POTION."; // TODO: localization
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
		if (_vm->_championMan->_g407_party._spellShieldDefense > 50) {
			L1240_s_Event._B._defense >>= 2;
		}
		_vm->_championMan->_g407_party._spellShieldDefense += L1240_s_Event._B._defense;
	} else {
		L1240_s_Event._type = k78_TMEventTypeFireShield;
		if (_vm->_championMan->_g407_party._fireShieldDefense > 50) {
			L1240_s_Event._B._defense >>= 2;
		}
		_vm->_championMan->_g407_party._fireShieldDefense += L1240_s_Event._B._defense;
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

	L1225_ps_Champion = &_vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
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
		_vm->_championMan->f292_drawChampionState(_vm->_championMan->_g514_magicCasterChampionIndex);
		_vm->_eventMan->f77_hideMouse();
	}
}

void MenuMan::f400_deleteChampionSymbol() {
	int16 L1226_ui_SymbolStep;
	Champion* L1228_ps_Champion;

	L1228_ps_Champion = &_vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex];
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
}
