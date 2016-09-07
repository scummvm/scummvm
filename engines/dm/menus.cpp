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

#include "dm/menus.h"
#include "dm/gfx.h"
#include "dm/champion.h"
#include "dm/dungeonman.h"
#include "dm/objectman.h"
#include "dm/inventory.h"
#include "dm/text.h"
#include "dm/eventman.h"
#include "dm/timeline.h"
#include "dm/movesens.h"
#include "dm/group.h"
#include "dm/projexpl.h"
#include "dm/sounds.h"


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
		3   /* FUSE */
	};
	static unsigned char actionDisabledTicks[44] = {
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
		2   /* FUSE */
	};

	_boxActionArea1ActionMenu = Box(224, 319, 77, 97); // @ G0501_s_Graphic560_Box_ActionArea1ActionMenu
	_boxActionArea2ActionMenu = Box(224, 319, 77, 109); // @ G0500_s_Graphic560_Box_ActionArea2ActionsMenu
	_boxActionArea3ActionMenu = Box(224, 319, 77, 121); // @ G0499_s_Graphic560_Box_ActionArea3ActionsMenu
	_boxActionArea = Box(224, 319, 77, 121); // @ G0001_s_Graphic562_Box_ActionArea 
	_boxSpellArea = Box(224, 319, 42, 74);

	for (int i = 0; i < 44; i++) {
		_actionSkillIndex[i] = actionSkillIndex[i];
		_actionDisabledTicks[i] = actionDisabledTicks[i];
	}
}

MenuMan::MenuMan(DMEngine *vm) : _vm(vm) {
	_refreshActionArea = false;
	_actionAreaContainsIcons = false;
	_actionDamage = 0;
	_actionList.resetToZero();
	_bitmapSpellAreaLine = new byte[96 * 12];
	_bitmapSpellAreaLines = new byte[3 * 96 * 12];
	_actionTargetGroupThing = Thing(0);
	_actionCount = 0;

	initConstants();
}

MenuMan::~MenuMan() {
	delete[] _bitmapSpellAreaLine;
}

void MenuMan::drawMovementArrows() {
	_vm->_eventMan->showMouse();
	_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k13_MovementArrowsIndice),
									   &_vm->_displayMan->_boxMovementArrows, k48_byteWidth, kM1_ColorNoTransparency, 45);
	_vm->_eventMan->hideMouse();
}
void MenuMan::clearActingChampion() {
	ChampionMan &cm = *_vm->_championMan;
	if (cm._actingChampionOrdinal) {
		cm._actingChampionOrdinal--;
		cm._champions[cm._actingChampionOrdinal].setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
		cm.drawChampionState((ChampionIndex)cm._actingChampionOrdinal);
		cm._actingChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
		_refreshActionArea = true;
	}
}

void MenuMan::drawActionIcon(ChampionIndex championIndex) {
	static byte palChangesActionAreaObjectIcon[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0}; // @ G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon

	if (!_actionAreaContainsIcons)
		return;
	DisplayMan &dm = *_vm->_displayMan;
	Champion &champion = _vm->_championMan->_champions[championIndex];

	Box box;
	box._x1 = championIndex * 22 + 233;
	box._x2 = box._x1 + 19;
	box._y1 = 86;
	box._y2 = 120;
	dm._useByteBoxCoordinates = false;
	if (!champion._currHealth) {
		dm.fillScreenBox(box, k0_ColorBlack);
		return;
	}
	byte *bitmapIcon = dm._tmpBitmap;
	Thing thing = champion.getSlot(k1_ChampionSlotActionHand);
	IconIndice iconIndex;
	if (thing == Thing::_none) {
		iconIndex = k201_IconIndiceActionEmptyHand;
	} else if (_vm->_dungeonMan->_objectInfos[_vm->_dungeonMan->getObjectInfoIndex(thing)]._actionSetIndex) {
		iconIndex = _vm->_objectMan->getIconIndex(thing);
	} else {
		dm.fillBitmap(bitmapIcon, k4_ColorCyan, 16, 16);
		goto T0386006;
	}
	_vm->_objectMan->extractIconFromBitmap(iconIndex, bitmapIcon);
	dm.blitToBitmapShrinkWithPalChange(bitmapIcon, bitmapIcon, 16, 16, 16, 16, palChangesActionAreaObjectIcon);
T0386006:
	dm.fillScreenBox(box, k4_ColorCyan);
	Box box2;
	box2._x1 = box._x1 + 2;
	box2._x2 = box._x2 - 2;
	box2._y1 = 95;
	box2._y2 = 110;
	dm.blitToScreen(bitmapIcon, &box2, k8_byteWidth, kM1_ColorNoTransparency, 16);
	if (champion.getAttributes(k0x0008_ChampionAttributeDisableAction) || _vm->_championMan->_candidateChampionOrdinal || _vm->_championMan->_partyIsSleeping) {
		_vm->_displayMan->shadeScreenBox(&box, k0_ColorBlack);
	}
}

void MenuMan::drawDisabledMenu() {
	if (!_vm->_championMan->_partyIsSleeping) {
		_vm->_eventMan->highlightBoxDisable();
		_vm->_displayMan->_useByteBoxCoordinates = false;
		if (_vm->_inventoryMan->_inventoryChampionOrdinal) {
			if (_vm->_inventoryMan->_panelContent == k4_PanelContentChest) {
				_vm->_inventoryMan->closeChest();
			}
		} else {
			_vm->_displayMan->shadeScreenBox(&_vm->_displayMan->_boxMovementArrows, k0_ColorBlack);
		}
		_vm->_displayMan->shadeScreenBox(&_boxSpellArea, k0_ColorBlack);
		_vm->_displayMan->shadeScreenBox(&_boxActionArea, k0_ColorBlack);
		_vm->_eventMan->setMousePointerToNormal(k0_pointerArrow);
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
		int16 champIndex = k0_ChampionFirst;

		do {
			if ((champIndex != champMan._leaderIndex)
				&& (_vm->indexToOrdinal(champIndex) != champMan._actingChampionOrdinal)
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

	if (_refreshActionArea) {
		if (!champMan._actingChampionOrdinal) {
			if (_actionDamage) {
				drawActionDamage(_actionDamage);
				_actionDamage = 0;
			} else {
				_actionAreaContainsIcons = true;
				drawActionArea();
			}
		} else {
			_actionAreaContainsIcons = false;
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			champMan.drawChampionState((ChampionIndex)_vm->ordinalToIndex(champMan._actingChampionOrdinal));
			drawActionArea();
		}
	}
}

#define k7_ChampionNameMaximumLength 7 // @ C007_CHAMPION_NAME_MAXIMUM_LENGTH
#define k12_ActionNameMaximumLength 12 // @ C012_ACTION_NAME_MAXIMUM_LENGTH

void MenuMan::drawActionArea() {
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	_vm->_eventMan->hideMouse();
	dispMan._useByteBoxCoordinates = false;
	dispMan.fillScreenBox(_boxActionArea, k0_ColorBlack);
	if (_actionAreaContainsIcons) {
		for (uint16 champIndex = k0_ChampionFirst; champIndex < champMan._partyChampionCount; ++champIndex)
			drawActionIcon((ChampionIndex)champIndex);
	} else if (champMan._actingChampionOrdinal) {
		Box box = _boxActionArea3ActionMenu;
		if (_actionList._actionIndices[2] == k255_ChampionActionNone)
			box = _boxActionArea2ActionMenu;
		if (_actionList._actionIndices[1] == k255_ChampionActionNone)
			box = _boxActionArea1ActionMenu;
		dispMan.blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k10_MenuActionAreaIndice),
								 &box, k48_byteWidth, kM1_ColorNoTransparency, 45);
		textMan.printWithTrailingSpaces(dispMan._bitmapScreen, k160_byteWidthScreen,
											235, 83, k0_ColorBlack, k4_ColorCyan, champMan._champions[_vm->ordinalToIndex(champMan._actingChampionOrdinal)]._name,
											k7_ChampionNameMaximumLength, k200_heightScreen);
		for (uint16 actionListIndex = 0; actionListIndex < 3; actionListIndex++) {
			textMan.printWithTrailingSpaces(dispMan._bitmapScreen, k160_byteWidthScreen, 241, 93 + actionListIndex * 12, k4_ColorCyan, k0_ColorBlack,
												getActionName(_actionList._actionIndices[actionListIndex]),
												k12_ActionNameMaximumLength, k200_heightScreen);
		}
	}
	_vm->_eventMan->showMouse();
	_refreshActionArea = false;
}

const char *MenuMan::getActionName(ChampionAction actionIndex) {
	const char *championActionNames[44] = { // @ G0490_ac_Graphic560_ActionNames
		"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH",
		"KICK", "WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE",
		"HIT", "SWING", "STAB", "THRUST", "JAB", "PARRY", "HACK",
		"BERZERK", "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING",
		"DISRUPT", "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
		"BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
		"FLUXCAGE", "HEAL", "CALM", "LIGHT", "WINDOW", "SPIT",
		"BRANDISH", "THROW", "FUSE"
	};

	return (actionIndex == k255_ChampionActionNone) ? "" : championActionNames[actionIndex];
}

void MenuMan::drawSpellAreaControls(ChampionIndex champIndex) {
	static Box boxSpellAreaControls(233, 319, 42, 49); // @ G0504_s_Graphic560_Box_SpellAreaControls 

	Champion *champ = &_vm->_championMan->_champions[champIndex];
	_vm->_displayMan->_useByteBoxCoordinates = false;
	int16 champHP0 = _vm->_championMan->_champions[0]._currHealth;
	int16 champHP1 = _vm->_championMan->_champions[1]._currHealth;
	int16 champHP2 = _vm->_championMan->_champions[2]._currHealth;
	int16 champHP3 = _vm->_championMan->_champions[3]._currHealth;
	_vm->_eventMan->showMouse();
	_vm->_displayMan->fillScreenBox(boxSpellAreaControls, k0_ColorBlack);

	switch (champIndex) {
	case 0:
		_vm->_eventMan->highlightScreenBox(233, 277, 42, 49);
		_vm->_textMan->printToLogicalScreen(235, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		if (_vm->_championMan->_partyChampionCount > 1) {
			if (champHP1)
				_vm->_eventMan->highlightScreenBox(280, 291, 42, 48);

			if (_vm->_championMan->_partyChampionCount > 2) {
				if (champHP2)
					_vm->_eventMan->highlightScreenBox(294, 305, 42, 48);

				if ((_vm->_championMan->_partyChampionCount > 3) && champHP3)
					_vm->_eventMan->highlightScreenBox(308, 319, 42, 48);
			}
		}
		break;
	case 1:
		if (champHP0)
			_vm->_eventMan->highlightScreenBox(233, 244, 42, 48);

		_vm->_eventMan->highlightScreenBox(247, 291, 42, 49);
		_vm->_textMan->printToLogicalScreen(249, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		if (_vm->_championMan->_partyChampionCount > 2) {
			if (champHP2)
				_vm->_eventMan->highlightScreenBox(294, 305, 42, 48);

			if ((_vm->_championMan->_partyChampionCount > 3) && champHP3)
				_vm->_eventMan->highlightScreenBox(308, 319, 42, 48);
		}
		break;
	case 2:
		if (champHP0)
			_vm->_eventMan->highlightScreenBox(233, 244, 42, 48);

		if (champHP1)
			_vm->_eventMan->highlightScreenBox(247, 258, 42, 48);

		_vm->_eventMan->highlightScreenBox(261, 305, 42, 49);
		_vm->_textMan->printToLogicalScreen(263, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		if ((_vm->_championMan->_partyChampionCount > 3) && champHP3)
			_vm->_eventMan->highlightScreenBox(308, 319, 42, 48);
		break;

	case 3:
		if (champHP0)
			_vm->_eventMan->highlightScreenBox(233, 244, 42, 48);

		if (champHP1)
			_vm->_eventMan->highlightScreenBox(247, 258, 42, 48);

		if (champHP2)
			_vm->_eventMan->highlightScreenBox(261, 272, 42, 48);

		_vm->_eventMan->highlightScreenBox(275, 319, 42, 49);
		_vm->_textMan->printToLogicalScreen(277, 48, k0_ColorBlack, k4_ColorCyan, champ->_name);
		break;
	default:
		break;
	}
	_vm->_eventMan->hideMouse();
}

void MenuMan::buildSpellAreaLine(int16 spellAreaBitmapLine) {
	static Box boxSpellAreaLine(0, 95, 0, 11); // @ K0074_s_Box_SpellAreaLine 

	char spellSymbolString[2] = {'\0', '\0'};
	Champion *magicChampion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	if (spellAreaBitmapLine == k2_SpellAreaAvailableSymbols) {
		_vm->_displayMan->_useByteBoxCoordinates = false;
		_vm->_displayMan->blitToBitmap(_bitmapSpellAreaLines, _bitmapSpellAreaLine, boxSpellAreaLine, 0, 12, k48_byteWidth, k48_byteWidth, kM1_ColorNoTransparency, 36, 12);
		int16 x = 1;
		char character = 96 + (6 * magicChampion->_symbolStep);
		for (uint16 symbolIndex = 0; symbolIndex < 6; symbolIndex++) {
			spellSymbolString[0] = character++;
			x += 14;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 48, x, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	} else if (spellAreaBitmapLine == k3_SpellAreaChampionSymbols) {
		_vm->_displayMan->_useByteBoxCoordinates = false;
		_vm->_displayMan->blitToBitmap(_bitmapSpellAreaLines, _bitmapSpellAreaLine, boxSpellAreaLine, 0, 24, k48_byteWidth, k48_byteWidth, kM1_ColorNoTransparency, 36, 12);
		int16 x = 8;
		for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
			if ((spellSymbolString[0] = magicChampion->_symbols[symbolIndex]) == '\0')
				break;
			x += 9;
			_vm->_textMan->printTextToBitmap(_bitmapSpellAreaLine, 48, x, 8, k4_ColorCyan, k0_ColorBlack, spellSymbolString, 12);
		}
	}
}

void MenuMan::setMagicCasterAndDrawSpellArea(ChampionIndex champIndex) {
	static Box boxSpellAreaLine2(224, 319, 50, 61); // @ K0075_s_Box_SpellAreaLine2 
	static Box boxSpellAreaLine3(224, 319, 62, 73); // @ K0076_s_Box_SpellAreaLine3 

	if ((champIndex == _vm->_championMan->_magicCasterChampionIndex)
	|| ((champIndex != kM1_ChampionNone) && !_vm->_championMan->_champions[champIndex]._currHealth))
		return;

	if (_vm->_championMan->_magicCasterChampionIndex == kM1_ChampionNone) {
		_vm->_eventMan->showMouse();
		_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k9_MenuSpellAreaBackground), &_boxSpellArea, k48_byteWidth, kM1_ColorNoTransparency, 33);
		_vm->_eventMan->hideMouse();
	}
	if (champIndex == kM1_ChampionNone) {
		_vm->_championMan->_magicCasterChampionIndex = kM1_ChampionNone;
		_vm->_eventMan->showMouse();
		_vm->_displayMan->_useByteBoxCoordinates = false;
		_vm->_displayMan->fillScreenBox(_boxSpellArea, k0_ColorBlack);
		_vm->_eventMan->hideMouse();
		return;
	}
	_vm->_championMan->_magicCasterChampionIndex = champIndex;
	buildSpellAreaLine(k2_SpellAreaAvailableSymbols);
	_vm->_eventMan->showMouse();
	drawSpellAreaControls(champIndex);
	_vm->_displayMan->blitToScreen(_bitmapSpellAreaLine, &boxSpellAreaLine2, k48_byteWidth, kM1_ColorNoTransparency, 12);
	buildSpellAreaLine(k3_SpellAreaChampionSymbols);
	_vm->_displayMan->blitToScreen(_bitmapSpellAreaLine, &boxSpellAreaLine3, k48_byteWidth, kM1_ColorNoTransparency, 12);
	_vm->_eventMan->hideMouse();
}

void MenuMan::drawEnabledMenus() {
	if (_vm->_championMan->_partyIsSleeping) {
		_vm->_eventMan->drawSleepScreen();
		_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	} else {
		ChampionIndex casterChampionIndex = _vm->_championMan->_magicCasterChampionIndex;
		_vm->_championMan->_magicCasterChampionIndex = kM1_ChampionNone; /* Force next function to draw the spell area */
		setMagicCasterAndDrawSpellArea(casterChampionIndex);
		if (!_vm->_championMan->_actingChampionOrdinal)
			_actionAreaContainsIcons = true;

		drawActionArea();
		int16 AL1462_i_InventoryChampionOrdinal = _vm->_inventoryMan->_inventoryChampionOrdinal;
		if (AL1462_i_InventoryChampionOrdinal) {
			_vm->_inventoryMan->_inventoryChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
			_vm->_inventoryMan->toggleInventory((ChampionIndex)_vm->ordinalToIndex(AL1462_i_InventoryChampionOrdinal));
		} else {
			_vm->_displayMan->drawFloorAndCeiling();
			drawMovementArrows();
		}
		_vm->_eventMan->setMousePointer();
	}
}

int16 MenuMan::getClickOnSpellCastResult() {
	Champion *casterChampion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	_vm->_eventMan->showMouse();
	_vm->_eventMan->highlightBoxDisable();

	int16 spellCastResult = getChampionSpellCastResult(_vm->_championMan->_magicCasterChampionIndex);
	if (spellCastResult != k3_spellCastFailureNeedsFlask) {
		casterChampion->_symbols[0] = '\0';
		drawAvailableSymbols(casterChampion->_symbolStep = 0);
		drawChampionSymbols(casterChampion);
	} else
		spellCastResult = k0_spellCastFailure;

	_vm->_eventMan->hideMouse();
	return spellCastResult;
}

int16 MenuMan::getChampionSpellCastResult(uint16 champIndex) {
	if (champIndex >= _vm->_championMan->_partyChampionCount)
		return k0_spellCastFailure;

	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	if (!curChampion->_currHealth)
		return k0_spellCastFailure;

	Spell *curSpell = getSpellFromSymbols((unsigned char *)curChampion->_symbols);
	if (!curSpell) {
		menusPrintSpellFailureMessage(curChampion, k1_spellCastSuccess, 0);
		return k0_spellCastFailure;
	}
	int16 powerSymbolOrdinal = curChampion->_symbols[0] - '_'; /* Values 1 to 6 */
	uint16 requiredSkillLevel = curSpell->_baseRequiredSkillLevel + powerSymbolOrdinal;
	uint16 experience = _vm->getRandomNumber(8) + (requiredSkillLevel << 4) + ((_vm->ordinalToIndex(powerSymbolOrdinal) * curSpell->_baseRequiredSkillLevel) << 3) + (requiredSkillLevel * requiredSkillLevel);
	uint16 skillLevel = _vm->_championMan->getSkillLevel(champIndex, curSpell->_skillIndex);
	if (skillLevel < requiredSkillLevel) {
		int16 missingSkillLevelCount = requiredSkillLevel - skillLevel;
		while (missingSkillLevelCount--) {
			if (_vm->getRandomNumber(128) > MIN(curChampion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent] + 15, 115)) {
				_vm->_championMan->addSkillExperience(champIndex, curSpell->_skillIndex, experience >> (requiredSkillLevel - skillLevel));
				menusPrintSpellFailureMessage(curChampion, k0_failureNeedsMorePractice, curSpell->_skillIndex);
				return k0_spellCastFailure;
			}
		}
	}
	switch (curSpell->getKind()) {
	case k1_spellKindPotion: {
		Thing newObject;
		Potion *newPotion = getEmptyFlaskInHand(curChampion, &newObject);
		if (!newPotion) {
			menusPrintSpellFailureMessage(curChampion, k10_failureNeedsFlaskInHand, 0);
			return k3_spellCastFailureNeedsFlask;
		}
		uint16 emptyFlaskWeight = _vm->_dungeonMan->getObjectWeight(newObject);
		newPotion->setType((PotionType)curSpell->getType());
		newPotion->setPower(_vm->getRandomNumber(16) + (powerSymbolOrdinal * 40));
		curChampion->_load += _vm->_dungeonMan->getObjectWeight(newObject) - emptyFlaskWeight;
		_vm->_championMan->drawChangedObjectIcons();
		if (_vm->_inventoryMan->_inventoryChampionOrdinal == _vm->indexToOrdinal(champIndex)) {
			setFlag(curChampion->_attributes, k0x0200_ChampionAttributeLoad);
			_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
		}
		}
		break;
	case k2_spellKindProjectile:
		if (curChampion->_dir != _vm->_dungeonMan->_partyDir) {
			curChampion->_dir = _vm->_dungeonMan->_partyDir;
			setFlag(curChampion->_attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
		}
		if (curSpell->getType() == k4_spellType_projectileOpenDoor)
			skillLevel <<= 1;

		_vm->_championMan->isProjectileSpellCast(champIndex, Thing(curSpell->getType() + Thing::_firstExplosion.toUint16()), getBoundedValue(21, (powerSymbolOrdinal + 2) * (4 + (skillLevel << 1)), 255), 0);
		break;
	case k3_spellKindOther: {
		TimelineEvent newEvent;
		newEvent._priority = 0;
		uint16 spellPower = (powerSymbolOrdinal + 1) << 2;
		uint16 ticks;
		switch (curSpell->getType()) {
		case k0_spellType_otherLight: {
			ticks = 10000 + ((spellPower - 8) << 9);
			uint16 lightPower = (spellPower >> 1);
			lightPower--;
			_vm->_championMan->_party._magicalLightAmount += _vm->_championMan->_lightPowerToLightAmount[lightPower];
			createEvent70_light(-lightPower, ticks);
			}
			break;
		case k5_spellType_otherMagicTorch: {
			ticks = 2000 + ((spellPower - 3) << 7);
			uint16 lightPower = (spellPower >> 2);
			lightPower++;
			_vm->_championMan->_party._magicalLightAmount += _vm->_championMan->_lightPowerToLightAmount[lightPower];
			createEvent70_light(-lightPower, ticks);
			}
			break;
		case k1_spellType_otherDarkness: {
			uint16 lightPower = (spellPower >> 2);
			_vm->_championMan->_party._magicalLightAmount -= _vm->_championMan->_lightPowerToLightAmount[lightPower];
			createEvent70_light(lightPower, 98);
			}
			break;
		case k2_spellType_otherThievesEye: {
			newEvent._type = k73_TMEventTypeThievesEye;
			_vm->_championMan->_party._event73Count_ThievesEye++;
			spellPower = (spellPower >> 1);
			uint16 spellTicks = spellPower * spellPower;
			setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + spellTicks);
			_vm->_timeline->addEventGetEventIndex(&newEvent);
			}
			break;
		case k3_spellType_otherInvisibility: {
			newEvent._type = k71_TMEventTypeInvisibility;
			_vm->_championMan->_party._event71Count_Invisibility++;
			uint16 spellTicks = spellPower;
			setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + spellTicks);
			_vm->_timeline->addEventGetEventIndex(&newEvent);
			}
			break;
		case k4_spellType_otherPartyShield: {
			newEvent._type = k74_TMEventTypePartyShield;
			newEvent._B._defense = spellPower;
			if (_vm->_championMan->_party._shieldDefense > 50)
				newEvent._B._defense >>= 2;

			_vm->_championMan->_party._shieldDefense += newEvent._B._defense;
			_vm->_timeline->refreshAllChampionStatusBoxes();
			uint16 spellTicks = spellPower * spellPower;
			setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + spellTicks);
			_vm->_timeline->addEventGetEventIndex(&newEvent);
			}
			break;
		case k6_spellType_otherFootprints: {
			newEvent._type = k79_TMEventTypeFootprints;
			_vm->_championMan->_party._event79Count_Footprints++;
			_vm->_championMan->_party._firstScentIndex = _vm->_championMan->_party._scentCount;
			if (powerSymbolOrdinal < 3)
				_vm->_championMan->_party._lastScentIndex = _vm->_championMan->_party._firstScentIndex;
			else
				_vm->_championMan->_party._lastScentIndex = 0;

			uint16 spellTicks = spellPower * spellPower;
			setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + spellTicks);
			_vm->_timeline->addEventGetEventIndex(&newEvent);
			}
			break;
		case k7_spellType_otherZokathra: {
			Thing unusedObject = _vm->_dungeonMan->getUnusedThing(k10_JunkThingType);
			if (unusedObject == Thing::_none)
				break;

			Junk *junkData = (Junk *)_vm->_dungeonMan->getThingData(unusedObject);
			junkData->setType(k51_JunkTypeZokathra);
			ChampionSlot slotIndex;
			if (curChampion->_slots[k0_ChampionSlotReadyHand] == Thing::_none)
				slotIndex = k0_ChampionSlotReadyHand;
			else if (curChampion->_slots[k1_ChampionSlotActionHand] == Thing::_none)
				slotIndex = k1_ChampionSlotActionHand;
			else
				slotIndex = kM1_ChampionSlotLeaderHand;

			if ((slotIndex == k0_ChampionSlotReadyHand) || (slotIndex == k1_ChampionSlotActionHand)) {
				_vm->_championMan->addObjectInSlot((ChampionIndex)champIndex, unusedObject, slotIndex);
				_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
			} else
				_vm->_moveSens->getMoveResult(unusedObject, kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);

			}
			break;
		case k8_spellType_otherFireshield:
			isPartySpellOrFireShieldSuccessful(curChampion, false, (spellPower * spellPower) + 100, false);
			break;
		default:
			break;
		}
		}
	}
	_vm->_championMan->addSkillExperience(champIndex, curSpell->_skillIndex, experience);
	_vm->_championMan->disableAction(champIndex, curSpell->getDuration());
	return k1_spellCastSuccess;
}

Spell *MenuMan::getSpellFromSymbols(byte *symbols) {
	static Spell SpellsArray[25] = {
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
		int16 bitShiftCount = 24;
		int32 curSymbols = 0;
		do
			curSymbols |= (long)*symbols++ << bitShiftCount;
		while (*symbols && ((bitShiftCount -= 8) >= 0));
		Spell *curSpell = SpellsArray;
		int16 spellIndex = 25;
		while (spellIndex--) {
			if (curSpell->_symbols & 0xFF000000) { /* If byte 1 of spell is not 0 then the spell includes the power symbol */
				if (curSymbols == curSpell->_symbols) { /* Compare champion symbols, including power symbol, with spell (never used with actual spells) */
					return curSpell;
				}
			} else if ((curSymbols & 0x00FFFFFF) == curSpell->_symbols) /* Compare champion symbols, except power symbol, with spell */
				return curSpell;

			curSpell++;
		}
	}
	return nullptr;
}

void MenuMan::menusPrintSpellFailureMessage(Champion *champ, uint16 failureType, uint16 skillIndex) {
	const char *messagesEN[4] = {" NEEDS MORE PRACTICE WITH THIS ", " SPELL.", " MUMBLES A MEANINGLESS SPELL."," NEEDS AN EMPTY FLASK IN HAND FOR POTION."};
	const char *messagesDE[4] = {" BRAUCHT MEHR UEBUNG MIT DIESEM ", " ZAUBERSPRUCH.",
		" MURMELT EINEN SINNLOSEN ZAUBERSPRUCH.", " MUSS FUER DEN TRANK EINE LEERE FLASCHE BEREITHALTEN."};
	const char *messagesFR[5] = {" DOIT PRATIQUER DAVANTAGE SON ", "ENVOUTEMENT.", " MARMONNE UNE CONJURATION IMCOMPREHENSIBLE.",
		" DOIT AVOIR UN FLACON VIDE EN MAIN POUR LA POTION.", "EXORCISME."};

	if (skillIndex > k3_ChampionSkillWizard)
		skillIndex = (skillIndex - 4) / 4;

	_vm->_textMan->printLineFeed();
	_vm->_textMan->printMessage(k4_ColorCyan, champ->_name);

	const char **messages;
	switch (_vm->getGameLanguage()) { // localized
	case Common::DE_DEU:
		messages = messagesDE;
		break;
	case Common::FR_FRA:
		messages = messagesFR;
		break;
	default:
		messages = messagesEN;
		break;
	}

	const char *message = nullptr;
	switch (failureType) {
	case k0_failureNeedsMorePractice:
		_vm->_textMan->printMessage(k4_ColorCyan, messages[0]);
		_vm->_textMan->printMessage(k4_ColorCyan, _vm->_championMan->_baseSkillName[skillIndex]);
		if (_vm->getGameLanguage() != Common::FR_FRA || skillIndex == k3_ChampionSkillWizard)
			message = messages[1];
		else
			message = messages[4];

		break;
	case k1_failureMeaninglessSpell:
		message = messages[2];
		break;
	case k10_failureNeedsFlaskInHand:
		message = messages[3];
		break;
	}
	_vm->_textMan->printMessage(k4_ColorCyan, message);
}

Potion *MenuMan::getEmptyFlaskInHand(Champion *champ, Thing *potionThing) {
	for (int16 slotIndex = k2_ChampionSlotHead; --slotIndex >= k0_ChampionSlotReadyHand; ) {
		Thing curThing = champ->_slots[slotIndex];
		if ((curThing != Thing::_none) && (_vm->_objectMan->getIconIndex(curThing) == k195_IconIndicePotionEmptyFlask)) {
			*potionThing = curThing;
			return (Potion *)_vm->_dungeonMan->getThingData(curThing);
		}
	}
	return nullptr;
}

void MenuMan::createEvent70_light(int16 lightPower, int16 ticks) {
	TimelineEvent newEvent;
	newEvent._type = k70_TMEventTypeLight;
	newEvent._B._lightPower = lightPower;
	setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + ticks);
	newEvent._priority = 0;
	_vm->_timeline->addEventGetEventIndex(&newEvent);
	_vm->_inventoryMan->setDungeonViewPalette();
}

bool MenuMan::isPartySpellOrFireShieldSuccessful(Champion *champ, bool spellShield, uint16 ticks, bool useMana) {
	bool isPartyMagicShieldSuccessful = true;
	if (useMana) {
		if (champ->_currMana == 0)
			return false;

		if (champ->_currMana < 4) {
			ticks >>= 1;
			champ->_currMana = 0;
			isPartyMagicShieldSuccessful = false;
		} else
			champ->_currMana -= 4;
	}
	TimelineEvent newEvent;
	newEvent._B._defense = ticks >> 5;
	if (spellShield) {
		newEvent._type = k77_TMEventTypeSpellShield;
		if (_vm->_championMan->_party._spellShieldDefense > 50)
			newEvent._B._defense >>= 2;

		_vm->_championMan->_party._spellShieldDefense += newEvent._B._defense;
	} else {
		newEvent._type = k78_TMEventTypeFireShield;
		if (_vm->_championMan->_party._fireShieldDefense > 50)
			newEvent._B._defense >>= 2;

		_vm->_championMan->_party._fireShieldDefense += newEvent._B._defense;
	}
	newEvent._priority = 0;
	setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + ticks);
	_vm->_timeline->addEventGetEventIndex(&newEvent);
	_vm->_timeline->refreshAllChampionStatusBoxes();

	return isPartyMagicShieldSuccessful;
}

void MenuMan::drawAvailableSymbols(uint16 symbolStep) {
	char displayBuffer[2];
	displayBuffer[1] = '\0';
	char curCharacter = 96 + 6 * symbolStep;
	int16 textPosX = 225;
	for (uint16 L1214_ui_Counter = 0; L1214_ui_Counter < 6; L1214_ui_Counter++) {
		displayBuffer[0] = curCharacter++;
		textPosX += 14;
		_vm->_textMan->printToLogicalScreen(textPosX, 58, k4_ColorCyan, k0_ColorBlack, displayBuffer);
	}
}

void MenuMan::drawChampionSymbols(Champion *champ) {
	uint16 symbolCount = strlen(champ->_symbols);
	int16 textPosX = 232;
	char displayBuffer[2];
	displayBuffer[1] = '\0';

	for (uint16 symbolIndex = 0; symbolIndex < 4; symbolIndex++) {
		if (symbolIndex >= symbolCount)
			displayBuffer[0] = ' ';
		else
			displayBuffer[0] = champ->_symbols[symbolIndex];

		textPosX += 9;
		_vm->_textMan->printToLogicalScreen(textPosX, 70, k4_ColorCyan, k0_ColorBlack, displayBuffer);
	}
}

void MenuMan::addChampionSymbol(int16 symbolIndex) {
	static byte symbolBaseManaCost[4][6] = {
		{1, 2, 3, 4, 5, 6},   /* Power 1 */
		{2, 3, 4, 5, 6, 7},   /* Power 2 */
		{4, 5, 6, 7, 7, 9},   /* Power 3 */
		{2, 2, 3, 4, 6, 7}    /* Power 4 */
	};
	static byte symbolManaCostMultiplier[6] = {8, 12, 16, 20, 24, 28};

	Champion *casterChampion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	uint16 symbolStep = casterChampion->_symbolStep;
	uint16 manaCost = symbolBaseManaCost[symbolStep][symbolIndex];
	if (symbolStep) {
		uint16 symbolIndex = casterChampion->_symbols[0] - 96;
		manaCost = (manaCost * symbolManaCostMultiplier[symbolIndex]) >> 3;
	}

	if (manaCost <= casterChampion->_currMana) {
		casterChampion->_currMana -= manaCost;
		setFlag(casterChampion->_attributes, k0x0100_ChampionAttributeStatistics);
		casterChampion->_symbols[symbolStep] = 96 + (symbolStep * 6) + symbolIndex;
		casterChampion->_symbols[symbolStep + 1] = '\0';
		casterChampion->_symbolStep = symbolStep = returnNextVal(symbolStep);
		_vm->_eventMan->showMouse();
		drawAvailableSymbols(symbolStep);
		drawChampionSymbols(casterChampion);
		_vm->_championMan->drawChampionState(_vm->_championMan->_magicCasterChampionIndex);
		_vm->_eventMan->hideMouse();
	}
}

void MenuMan::deleteChampionSymbol() {
	Champion *casterChampion = &_vm->_championMan->_champions[_vm->_championMan->_magicCasterChampionIndex];
	if (!strlen(casterChampion->_symbols))
		return;

	int16 symbolStep = returnPrevVal(casterChampion->_symbolStep);
	casterChampion->_symbolStep = symbolStep;
	casterChampion->_symbols[symbolStep] = '\0';
	_vm->_eventMan->showMouse();
	drawAvailableSymbols(symbolStep);
	drawChampionSymbols(casterChampion);
	_vm->_eventMan->hideMouse();
}

bool MenuMan::didClickTriggerAction(int16 actionListIndex) {
	bool retVal = false;

	if (!_vm->_championMan->_actingChampionOrdinal || (actionListIndex != -1 && (_actionList._actionIndices[actionListIndex] == k255_ChampionActionNone)))
		return retVal;

	uint16 championIndex = _vm->ordinalToIndex(_vm->_championMan->_actingChampionOrdinal);
	Champion *curChampion = &_vm->_championMan->_champions[championIndex];
	if (actionListIndex == -1)
		retVal = true;
	else {
		uint16 actionIndex = _actionList._actionIndices[actionListIndex];
		// Fix original bug - When disabled ticks is equal to zero, increasing the defense leads
		// to a permanent increment.
		if (_actionDisabledTicks[actionIndex])
			curChampion->_actionDefense += _vm->_timeline->_actionDefense[actionIndex];

		setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
		retVal = isActionPerformed(championIndex, actionIndex);
		curChampion->_actionIndex = (ChampionAction)actionIndex;
	}
	clearActingChampion();
	return retVal;
}

bool MenuMan::isActionPerformed(uint16 champIndex, int16 actionIndex) {
	static unsigned char actionStaminaArray[44] = {
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
		2   /* FUSE */
	};
	static unsigned char actionExperienceGainArray[44] = {
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
		1   /* FUSE */
	};

	if (champIndex >= _vm->_championMan->_partyChampionCount)
		return false;

	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	if (!curChampion->_currHealth)
		return false;

	Weapon *weaponInHand = (Weapon *)_vm->_dungeonMan->getThingData(curChampion->_slots[k1_ChampionSlotActionHand]);

	int16 nextMapX = _vm->_dungeonMan->_partyMapX;
	int16 nextMapY = _vm->_dungeonMan->_partyMapY;
	nextMapX += _vm->_dirIntoStepCountEast[curChampion->_dir];
	nextMapY += _vm->_dirIntoStepCountNorth[curChampion->_dir];
	_actionTargetGroupThing = _vm->_groupMan->groupGetThing(nextMapX, nextMapY);
	uint16 actionDisabledTicks = _actionDisabledTicks[actionIndex];
	int16 actionSkillIndex = _actionSkillIndex[actionIndex];
	int16 actionStamina = actionStaminaArray[actionIndex] + _vm->getRandomNumber(2);
	int16 actionExperienceGain = actionExperienceGainArray[actionIndex];
	uint16 targetSquare = _vm->_dungeonMan->getSquare(nextMapX, nextMapY).toByte();

	int16 requiredManaAmount = 0;
	if (((actionSkillIndex >= k16_ChampionSkillFire) && (actionSkillIndex <= k19_ChampionSkillWater)) || (actionSkillIndex == k3_ChampionSkillWizard))
		requiredManaAmount = 7 - MIN<uint16>(6, _vm->_championMan->getSkillLevel(champIndex, actionSkillIndex));

	bool setDirectionFl = false;
	int16 kineticEnergy = 0;
	Thing explosionThing = Thing::_none;
	bool actionPerformed = true;
	switch (actionIndex) {
	case k23_ChampionActionLightning:
		kineticEnergy = 180;
		explosionThing = Thing::_explLightningBolt;
		setDirectionFl = true;
		break;
	case k21_ChampionActionDispel:
		kineticEnergy = 150;
		explosionThing = Thing::_explHarmNonMaterial;
		setDirectionFl = true;
		break;
	case k20_ChampionActionFireball:
		kineticEnergy = 150;
		explosionThing = Thing::_explFireBall;
		setDirectionFl = true;
		break;
	case k40_ChampionActionSpit:
		kineticEnergy = 250;
		explosionThing = Thing::_explFireBall;
		setDirectionFl = true;
		break;
	case k30_ChampionActionBash:
	case k18_ChampionActionHack:
	case k19_ChampionActionBerzerk:
	case k7_ChampionActionKick:
	case k13_ChampionActionSwing:
	case k2_ChampionActionChop:
		if ((Square(targetSquare).getType() == k4_DoorElemType) && (Square(targetSquare).getDoorState() == k4_doorState_CLOSED)) {
			_vm->_sound->requestPlay(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
			actionDisabledTicks = 6;
			_vm->_groupMan->groupIsDoorDestoryedByAttack(nextMapX, nextMapY, _vm->_championMan->getStrength(champIndex, k1_ChampionSlotActionHand), false, 2);
			_vm->_sound->requestPlay(k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k2_soundModePlayOneTickLater);
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
		if (!(actionPerformed = isMeleeActionPerformed(champIndex, curChampion, actionIndex, nextMapX, nextMapY, actionSkillIndex))) {
			actionExperienceGain >>= 1;
			actionDisabledTicks >>= 1;
		}
		break;
	case k22_ChampionActionConfuse:
		decrementCharges(curChampion);
		// No break on purpose
	case k8_ChampionActionWarCry:
	case k37_ChampionActionCalm:
	case k41_ChampionActionBrandish:
	case k4_ChampionActionBlowHorn:
		if (actionIndex == k8_ChampionActionWarCry)
			_vm->_sound->requestPlay(k28_soundWAR_CRY, nextMapX, nextMapY, k0_soundModePlayImmediately);
		else if (actionIndex == k4_ChampionActionBlowHorn)
			_vm->_sound->requestPlay(k25_soundBLOW_HORN, nextMapX, nextMapY, k0_soundModePlayImmediately);

		actionPerformed = isGroupFrightenedByAction(champIndex, actionIndex, nextMapX, nextMapY);
		break;
	case k32_ChampionActionShoot: {
		if (Thing(curChampion->_slots[k0_ChampionSlotReadyHand]).getType() != k5_WeaponThingType) {
			_actionDamage = kM2_damageNoAmmunition;
			actionExperienceGain = 0;
			actionPerformed = false;
			break;
		}

		WeaponInfo *weaponInfoActionHand = &_vm->_dungeonMan->_weaponInfos[weaponInHand->getType()];
		WeaponInfo *weaponInfoReadyHand = _vm->_dungeonMan->getWeaponInfo(curChampion->_slots[k0_ChampionSlotReadyHand]);
		int16 actionHandWeaponClass = weaponInfoActionHand->_class;
		int16 readyHandWeaponClass = weaponInfoReadyHand->_class;
		int16 stepEnergy = actionHandWeaponClass;
		if ((actionHandWeaponClass >= k16_WeaponClassFirstBow) && (actionHandWeaponClass <= k31_WeaponClassLastBow)) {
			if (readyHandWeaponClass != k10_WeaponClassBowAmmunition) {
				_actionDamage = kM2_damageNoAmmunition;
				actionExperienceGain = 0;
				actionPerformed = false;
				break;
			}
			stepEnergy -= k16_WeaponClassFirstBow;
		} else if ((actionHandWeaponClass >= k32_WeaponClassFirstSling) && (actionHandWeaponClass <= k47_WeaponClassLastSling)) {
			if (readyHandWeaponClass != k11_WeaponClassSlingAmmunition) {
				_actionDamage = kM2_damageNoAmmunition;
				actionExperienceGain = 0;
				actionPerformed = false;
				break;
			}
			stepEnergy -= k32_WeaponClassFirstSling;
		}

		setChampionDirectionToPartyDirection(curChampion);
		Thing removedObject = _vm->_championMan->getObjectRemovedFromSlot(champIndex, k0_ChampionSlotReadyHand);
		_vm->_sound->requestPlay(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
		_vm->_championMan->championShootProjectile(curChampion, removedObject, weaponInfoActionHand->_kineticEnergy + weaponInfoReadyHand->_kineticEnergy, (weaponInfoActionHand->getShootAttack() + _vm->_championMan->getSkillLevel(champIndex, k11_ChampionSkillShoot)) << 1, stepEnergy);
		}
		break;
	case k5_ChampionActionFlip: {
		const char *messagesEN[2] = {"IT COMES UP HEADS.", "IT COMES UP TAILS."};
		const char *messagesDE[2] = {"DIE KOPFSEITE IST OBEN.", "DIE ZAHL IST OBEN."};
		const char *messagesFR[2] = {"C'EST FACE.", "C'EST PILE."};
		const char **message;
		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
			message = messagesEN;
			break;
		case Common::DE_DEU:
			message = messagesDE;
			break;
		case Common::FR_FRA:
			message = messagesFR;
			break;
		}
		if (_vm->getRandomNumber(2))
			printMessageAfterReplacements(message[0]);
		else
			printMessageAfterReplacements(message[1]);

		}
		break;
	case k33_ChampionActionSpellshield:
	case k34_ChampionActionFireshield:
		if (!isPartySpellOrFireShieldSuccessful(curChampion, actionIndex == k33_ChampionActionSpellshield, 280, true)) {
			actionExperienceGain >>= 2;
			actionDisabledTicks >>= 1;
		} else
			decrementCharges(curChampion);

		break;
	case k27_ChampionActionInvoke:
		kineticEnergy = _vm->getRandomNumber(128) + 100;
		switch (_vm->getRandomNumber(6)) {
		case 0:
			explosionThing = Thing::_explPoisonBolt;
			break;
		case 1:
			explosionThing = Thing::_explPoisonCloud;
			break;
		case 2:
			explosionThing = Thing::_explHarmNonMaterial;
			break;
		default:
			explosionThing = Thing::_explFireBall;
			break;
		}
		setDirectionFl = true;
		break;

	case k35_ChampionActionFluxcage:
		setChampionDirectionToPartyDirection(curChampion);
		_vm->_groupMan->fluxCageAction(nextMapX, nextMapY);
		break;
	case k43_ChampionActionFuse:
		setChampionDirectionToPartyDirection(curChampion);
		nextMapX = _vm->_dungeonMan->_partyMapX;
		nextMapY = _vm->_dungeonMan->_partyMapY;
		nextMapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir], nextMapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
		_vm->_groupMan->fuseAction(nextMapX, nextMapY);
		break;
	case k36_ChampionActionHeal: {
		/* CHANGE2_17_IMPROVEMENT Heal action is much more effective
		Heal cycles occur as long as the champion has missing health and enough mana. Cycle count = Min(Current Mana / 2, Missing health / Min(10, Heal skill level))
		Healing amount is Min(Missing health, Min(10, Heal skill level)) * heal cycle count
		Mana cost is 2 * heal cycle count
		Experience gain is 2 + 2 * heal cycle count */
		int16 missingHealth = curChampion->_maxHealth - curChampion->_currHealth;
		if ((missingHealth > 0) && curChampion->_currMana) {
			int16 healingCapability = MIN((uint16)10, _vm->_championMan->getSkillLevel(champIndex, k13_ChampionSkillHeal));
			actionExperienceGain = 2;
			uint16 healingAmount;
			do {
				healingAmount = MIN(missingHealth, healingCapability);
				curChampion->_currHealth += healingAmount;
				actionExperienceGain += 2;
				curChampion->_currMana = curChampion->_currMana - 2;
				if (curChampion->_currMana > 0)
					missingHealth -= healingAmount;
			} while ((curChampion->_currMana > 0) && missingHealth);

			if (curChampion->_currMana < 0)
				curChampion->_currMana = 0;

			setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
			actionPerformed = true;
		}
		}
		break;
	case k39_ChampionActionWindow: {
		int16 windowTicks = _vm->getRandomNumber(_vm->_championMan->getSkillLevel(champIndex, actionSkillIndex) + 8) + 5;
		TimelineEvent newEvent;
		newEvent._priority = 0;
		newEvent._type = k73_TMEventTypeThievesEye;
		setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + windowTicks);
		_vm->_timeline->addEventGetEventIndex(&newEvent);
		_vm->_championMan->_party._event73Count_ThievesEye++;
		decrementCharges(curChampion);
		}
		break;
	case k10_ChampionActionClimbDown:
		nextMapX = _vm->_dungeonMan->_partyMapX;
		nextMapY = _vm->_dungeonMan->_partyMapY;
		nextMapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir];
		nextMapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
		/* CHANGE6_00_FIX The presence of a group over the pit is checked so that you cannot climb down a pit with the rope if there is a group levitating over it */
		if ((_vm->_dungeonMan->getSquare(nextMapX, nextMapY).getType() == k2_PitElemType) && (_vm->_groupMan->groupGetThing(nextMapX, nextMapY) == Thing::_endOfList)) {
			/* BUG0_77 The party moves forward when using the rope in front of a closed pit. The engine does not check whether
			   the pit is open before moving the party over the pit. This is not consistent with the behavior when using the
			   rope in front of a corridor where nothing happens */
			_vm->_moveSens->_useRopeToClimbDownPit = true;
			_vm->_moveSens->getMoveResult(Thing::_party, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, nextMapX, nextMapY);
			_vm->_moveSens->_useRopeToClimbDownPit = false;
		} else {
			actionDisabledTicks = 0;
		}
		break;
	case k11_ChampionActionFreezeLife: {
		int16 freezeTicks;
		if (weaponInHand->getType() == k42_JunkTypeMagicalBoxBlue) {
			freezeTicks = 30;
			_vm->_championMan->getObjectRemovedFromSlot(champIndex, k1_ChampionSlotActionHand);
			weaponInHand->setNextThing(Thing::_none);
		} else if (weaponInHand->getType() == k43_JunkTypeMagicalBoxGreen) {
			freezeTicks = 125;
			_vm->_championMan->getObjectRemovedFromSlot(champIndex, k1_ChampionSlotActionHand);
			weaponInHand->setNextThing(Thing::_none);
		} else {
			freezeTicks = 70;
			decrementCharges(curChampion);
		}
		_vm->_championMan->_party._freezeLifeTicks = MIN(200, _vm->_championMan->_party._freezeLifeTicks + freezeTicks);
		}
		break;
	case k38_ChampionActionLight:
		_vm->_championMan->_party._magicalLightAmount += _vm->_championMan->_lightPowerToLightAmount[2];
		createEvent70_light(-2, 2500);
		decrementCharges(curChampion);
		break;
	case k42_ChampionActionThrow:
		setChampionDirectionToPartyDirection(curChampion);
		actionPerformed = _vm->_championMan->isObjectThrown(champIndex, k1_ChampionSlotActionHand, (curChampion->_cell == returnNextVal(_vm->_dungeonMan->_partyDir)) || (curChampion->_cell == (ViewCell)returnOppositeDir(_vm->_dungeonMan->_partyDir)));
		if (actionPerformed)
			_vm->_timeline->_events[curChampion->_enableActionEventIndex]._B._slotOrdinal = _vm->indexToOrdinal(k1_ChampionSlotActionHand);
		break;
	}

	if (setDirectionFl) {
		setChampionDirectionToPartyDirection(curChampion);
		if (curChampion->_currMana < requiredManaAmount) {
			kineticEnergy = MAX(2, curChampion->_currMana * kineticEnergy / requiredManaAmount);
			requiredManaAmount = curChampion->_currMana;
		}
		actionPerformed = _vm->_championMan->isProjectileSpellCast(champIndex, explosionThing, kineticEnergy, requiredManaAmount);
		if (!actionPerformed)
			actionExperienceGain >>= 1;

		decrementCharges(curChampion);
	}
	if (actionDisabledTicks)
		_vm->_championMan->disableAction(champIndex, actionDisabledTicks);

	if (actionStamina)
		_vm->_championMan->decrementStamina(champIndex, actionStamina);

	if (actionExperienceGain)
		_vm->_championMan->addSkillExperience(champIndex, actionSkillIndex, actionExperienceGain);

	_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	return actionPerformed;
}

void MenuMan::setChampionDirectionToPartyDirection(Champion *champ) {
	if (champ->_dir != _vm->_dungeonMan->_partyDir) {
		champ->_dir = _vm->_dungeonMan->_partyDir;
		setFlag(champ->_attributes, k0x0400_ChampionAttributeIcon);
	}
}

void MenuMan::decrementCharges(Champion *champ) {
	Thing slotActionThing = champ->_slots[k1_ChampionSlotActionHand];
	Junk *slotActionData = (Junk *)_vm->_dungeonMan->getThingData(slotActionThing);
	switch (slotActionThing.getType()) {
	case k5_WeaponThingType:
		if (((Weapon *)slotActionData)->getChargeCount()) {
			((Weapon *)slotActionData)->setChargeCount(((Weapon *)slotActionData)->getChargeCount() - 1);
		}
		break;
	case k6_ArmourThingType:
		if (((Armour *)slotActionData)->getChargeCount()) {
			((Armour *)slotActionData)->setChargeCount(((Armour *)slotActionData)->getChargeCount() - 1);
		}
		break;
	case k10_JunkThingType:
		if (slotActionData->getChargeCount()) {
			slotActionData->setChargeCount(slotActionData->getChargeCount() - 1);
		}
		break;
	default:
		break;
	}
	_vm->_championMan->drawChangedObjectIcons();
}

bool MenuMan::isMeleeActionPerformed(int16 champIndex, Champion *champ, int16 actionIndex, int16 targetMapX, int16 targetMapY, int16 skillIndex) {
	static unsigned char actionDamageFactorArray[44] = {
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
		0   /* FUSE */
	};
	static unsigned char actionHitProbabilityArray[44] = {
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
		0   /* FUSE */
	};

	_vm->_sound->requestPlay(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
	if (_actionTargetGroupThing == Thing::_endOfList)
		return false;

	uint16 championCell = champ->_cell;
	int16 targetCreatureOrdinal = _vm->_groupMan->getMeleeTargetCreatureOrdinal(targetMapX, targetMapY, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, championCell);
	if (targetCreatureOrdinal) {
		uint16 viewCell = normalizeModulo4(championCell + 4 - champ->_dir);
		switch (viewCell) {
		case k2_ViewCellBackRight: /* Champion is on the back right of the square and tries to attack a creature in the front right of its square */
		case k3_ViewCellBackLeft: /* Champion is on the back left of the square and tries to attack a creature in the front left of its square */
			uint16 cellDelta = (viewCell == k2_ViewCellBackRight) ? 3 : 1;
			/* Check if there is another champion in front */
			if (_vm->_championMan->getIndexInCell(normalizeModulo4(championCell + cellDelta)) != kM1_ChampionNone) {
				_actionDamage = kM1_damageCantReach;
				return false;
			}
			break;
		}

		if ((actionIndex == k24_ChampionActionDisrupt) && !getFlag(_vm->_dungeonMan->getCreatureAttributes(_actionTargetGroupThing), k0x0040_MaskCreatureInfo_nonMaterial))
			return false;

		uint16 actionHitProbability = actionHitProbabilityArray[actionIndex];
		uint16 actionDamageFactor = actionDamageFactorArray[actionIndex];
		if ((_vm->_objectMan->getIconIndex(champ->_slots[k1_ChampionSlotActionHand]) == k40_IconIndiceWeaponVorpalBlade) || (actionIndex == k24_ChampionActionDisrupt)) {
			setFlag(actionHitProbability, k0x8000_hitNonMaterialCreatures);
		}
		_actionDamage = _vm->_groupMan->getMeleeActionDamage(champ, champIndex, (Group *)_vm->_dungeonMan->getThingData(_actionTargetGroupThing), _vm->ordinalToIndex(targetCreatureOrdinal), targetMapX, targetMapY, actionHitProbability, actionDamageFactor, skillIndex);
		return true;
	}

	return false;
}

bool MenuMan::isGroupFrightenedByAction(int16 champIndex, uint16 actionIndex, int16 mapX, int16 mapY) {
	bool isGroupFrightenedByAction = false;
	if (_actionTargetGroupThing == Thing::_endOfList)
		return isGroupFrightenedByAction;

	uint16 experience = 0;
	int16 frightAmount = 0;

	switch (actionIndex) {
	case k8_ChampionActionWarCry:
		frightAmount = 3;
		experience = 12; /* War Cry gives experience in priest skill k14_ChampionSkillInfluence below. The War Cry action also has an experience gain of 7 defined in G0497_auc_Graphic560_ActionExperienceGain in the same skill (versions 1.1 and below) or in the fighter skill k7_ChampionSkillParry (versions 1.2 and above). In versions 1.2 and above, this is the only action that gives experience in two skills */
		break;
	case k37_ChampionActionCalm:
		frightAmount = 7;
		experience = 35;
		break;
	case k41_ChampionActionBrandish:
		frightAmount = 6;
		experience = 30;
		break;
	case k4_ChampionActionBlowHorn:
		frightAmount = 6;
		experience = 20;
		break;
	case k22_ChampionActionConfuse:
		frightAmount = 12;
		experience = 45;
		break;
	}
	frightAmount += _vm->_championMan->getSkillLevel(champIndex, k14_ChampionSkillInfluence);
	Group *targetGroup = (Group *)_vm->_dungeonMan->getThingData(_actionTargetGroupThing);
	CreatureInfo *creatureInfo = &_vm->_dungeonMan->_creatureInfos[targetGroup->_type];
	uint16 fearResistance = creatureInfo->getFearResistance();
	if ((fearResistance > _vm->getRandomNumber(frightAmount)) || (fearResistance == k15_immuneToFear)) {
		experience >>= 1;
	} else {
		ActiveGroup *activeGroup = &_vm->_groupMan->_activeGroups[targetGroup->getActiveGroupIndex()];
		if (targetGroup->getBehaviour() == k6_behavior_ATTACK) {
			_vm->_groupMan->stopAttacking(activeGroup, mapX, mapY);
			_vm->_groupMan->startWandering(mapX, mapY);
		}
		targetGroup->setBehaviour(k5_behavior_FLEE);
		activeGroup->_delayFleeingFromTarget = ((16 - fearResistance) << 2) / creatureInfo->_movementTicks;
		isGroupFrightenedByAction = true;
	}
	_vm->_championMan->addSkillExperience(champIndex, k14_ChampionSkillInfluence, experience);

	return isGroupFrightenedByAction;
}

void MenuMan::printMessageAfterReplacements(const char *str) {
	char outputString[128];
	char *curCharacter = outputString;
	*curCharacter++ = '\n'; /* New line */
	char *replacementString = "";
	do {
		if (*str == '@') {
			str++;
			if (*(curCharacter - 1) != '\n') /* New line */
				*curCharacter++ = ' ';

			if (*str == 'p') /* '@p' in the source string is replaced by the champion name followed by a space */
				replacementString = _vm->_championMan->_champions[_vm->ordinalToIndex(_vm->_championMan->_actingChampionOrdinal)]._name;

			*curCharacter = '\0';
			strcat(outputString, replacementString);
			curCharacter += strlen(replacementString);
			*curCharacter++ = ' ';
		} else {
			*curCharacter++ = *str;
		}
	} while (*str++);
	*curCharacter = '\0';

	if (outputString[1]) /* If the string is not empty (the first character is a new line \n) */
		_vm->_textMan->printMessage(k4_ColorCyan, outputString);
}

void MenuMan::processCommands116To119_setActingChampion(uint16 champIndex) {
	static ActionSet actionSets[44] = {
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
		ActionSet(6,  11, 255, 0x80, 0x00)
	};

	Champion *curChampion = &_vm->_championMan->_champions[champIndex];
	if (getFlag(curChampion->_attributes, k0x0008_ChampionAttributeDisableAction) || !curChampion->_currHealth)
		return;

	uint16 actionSetIndex;
	Thing slotActionThing = curChampion->_slots[k1_ChampionSlotActionHand];

	if (slotActionThing == Thing::_none)
		actionSetIndex = 2; /* Actions Punch, Kick and War Cry */
	else {
		actionSetIndex = _vm->_dungeonMan->_objectInfos[_vm->_dungeonMan->getObjectInfoIndex(slotActionThing)]._actionSetIndex;
		if (actionSetIndex == 0)
			return;
	}

	ActionSet *actionSet = &actionSets[actionSetIndex];
	_vm->_championMan->_actingChampionOrdinal = _vm->indexToOrdinal(champIndex);
	setActionList(actionSet);
	_actionAreaContainsIcons = false;
	setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand);
	_vm->_championMan->drawChampionState((ChampionIndex)champIndex);
	drawActionArea();
	drawActionArea();
}

void MenuMan::setActionList(ActionSet *actionSet) {
	_actionList._actionIndices[0] = (ChampionAction)actionSet->_actionIndices[0];
	_actionList._minimumSkillLevel[0] = 1;
	uint16 nextAvailableActionListIndex = 1;
	for (uint16 idx = 1; idx < 3; idx++) {
		uint16 actionIndex = actionSet->_actionIndices[idx];

		if (actionIndex == k255_ChampionActionNone)
			continue;

		uint16 minimumSkillLevel = actionSet->_actionProperties[idx - 1];
		if (getFlag(minimumSkillLevel, k0x0080_actionRequiresCharge) && !getActionObjectChargeCount())
			continue;

		clearFlag(minimumSkillLevel, k0x0080_actionRequiresCharge);
		if (_vm->_championMan->getSkillLevel(_vm->ordinalToIndex(_vm->_championMan->_actingChampionOrdinal), _actionSkillIndex[actionIndex]) >= minimumSkillLevel) {
			_actionList._actionIndices[nextAvailableActionListIndex] = (ChampionAction)actionIndex;
			_actionList._minimumSkillLevel[nextAvailableActionListIndex] = minimumSkillLevel;
			nextAvailableActionListIndex++;
		}
	}
	_actionCount = nextAvailableActionListIndex;

	for (uint16 idx = nextAvailableActionListIndex; idx < 3; idx++)
		_actionList._actionIndices[idx] = k255_ChampionActionNone;
}

int16 MenuMan::getActionObjectChargeCount() {
	Thing slotActionThing = _vm->_championMan->_champions[_vm->ordinalToIndex(_vm->_championMan->_actingChampionOrdinal)]._slots[k1_ChampionSlotActionHand];
	Junk *junkData = (Junk *)_vm->_dungeonMan->getThingData(slotActionThing);
	switch (slotActionThing.getType()) {
	case k5_WeaponThingType:
		return ((Weapon *)junkData)->getChargeCount();
	case k6_ArmourThingType:
		return ((Armour *)junkData)->getChargeCount();
	case k10_JunkThingType:
		return junkData->getChargeCount();
	default:
		return 1;
	}
}

void MenuMan::drawActionDamage(int16 damage) {
	static const Box actionAreaMediumDamage(242, 305, 81, 117);
	static const Box actionAreaSmallDamage(251, 292, 81, 117);

	_vm->_eventMan->showMouse();
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->fillScreenBox(_boxActionArea, k0_ColorBlack);
	if (damage < 0) {
		static const char *messagesEN[2] = {"CAN'T REACH", "NEED AMMO"};
		static const char *messagesDE[2] = {"ZU WEIT WEG", "MEHR MUNITION"};
		static const char *messagesFR[2] = {"TROP LOIN", "SANS MUNITION"};
		static int16 posEN[2] = {242, 248};
		static int16 posDE[2] = {242, 236};
		static int16 posFR[2] = {248, 236};
		const char **message;
		int16 *pos;
		switch (_vm->getGameLanguage()) { // localized
		case Common::DE_DEU:
			message = messagesDE;
			pos = posDE;
			break;
		case Common::FR_FRA:
			message = messagesFR;
			pos = posFR;
			break;
		default:
			message = messagesEN;
			pos = posEN;
			break;
		}

		const char *displayString;
		int16 textPosX;
		if (damage == kM1_damageCantReach) {
			textPosX = pos[0];
			displayString = message[0];
		} else {
			textPosX = pos[1];
			displayString = message[1];
		}
		_vm->_textMan->printToLogicalScreen(textPosX, 100, k4_ColorCyan, k0_ColorBlack, displayString);
	} else {
		int16 byteWidth;
		byte *blitBitmap;
		const Box *blitBox;
		int16 displayHeight;
		if (damage > 40) {
			blitBox = &_boxActionArea3ActionMenu;
			blitBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(k14_damageToCreatureIndice);
			byteWidth = k48_byteWidth;
			displayHeight = 45;
		} else {
			uint16 derivedBitmapIndex;
			int16 destPixelWidth;
			if (damage > 15) {
				derivedBitmapIndex = k2_DerivedBitmapDamageToCreatureMedium;
				destPixelWidth = 64;
				byteWidth = k32_byteWidth;
				blitBox = &actionAreaMediumDamage;
			} else {
				derivedBitmapIndex = k3_DerivedBitmapDamageToCreatureSmall;
				destPixelWidth = 42;
				byteWidth = k24_byteWidth;
				blitBox = &actionAreaSmallDamage;
			}
			displayHeight = 37;
			if (!_vm->_displayMan->isDerivedBitmapInCache(derivedBitmapIndex)) {
				byte *nativeBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(k14_damageToCreatureIndice);
				blitBitmap = _vm->_displayMan->getDerivedBitmap(derivedBitmapIndex);
				_vm->_displayMan->blitToBitmapShrinkWithPalChange(nativeBitmap, blitBitmap, 96, 45, destPixelWidth, 37, _vm->_displayMan->_palChangesNoChanges);
				_vm->_displayMan->addDerivedBitmap(derivedBitmapIndex);
			} else {
				blitBitmap = _vm->_displayMan->getDerivedBitmap(derivedBitmapIndex);
			}
		}
		_vm->_displayMan->blitToScreen(blitBitmap, blitBox, byteWidth, kM1_ColorNoTransparency, displayHeight);
		/* Convert damage value to string */
		uint16 charIndex = 5;
		int16 textPosX = 274;
		char scoreString[6];
		scoreString[5] = '\0';
		do {
			scoreString[--charIndex] = '0' + (damage % 10);
			textPosX -= 3;
		} while (damage /= 10);
		_vm->_textMan->printToLogicalScreen(textPosX, 100, k4_ColorCyan, k0_ColorBlack, &scoreString[charIndex]);
	}
	_vm->_eventMan->hideMouse();
}
}
