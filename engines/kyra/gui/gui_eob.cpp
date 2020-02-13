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

#ifdef ENABLE_EOB

#include "kyra/engine/eobcommon.h"
#include "kyra/gui/gui_eob.h"
#include "kyra/script/script_eob.h"
#include "kyra/text/text_rpg.h"
#include "kyra/engine/timer.h"
#include "kyra/engine/util.h"

#include "backends/keymapper/keymapper.h"
#include "common/system.h"
#include "common/savefile.h"
#include "graphics/scaler.h"

namespace Kyra {

Button *EoBCoreEngine::gui_getButton(Button *buttonList, int index) {
	while (buttonList) {
		if (buttonList->index == index)
			return buttonList;
		buttonList = buttonList->nextButton;
	}

	return 0;
}

void EoBCoreEngine::gui_drawPlayField(bool refresh) {
	_screen->loadEoBBitmap("PLAYFLD", _cgaMappingDeco, 5, 3, 2);
	int cp = _screen->setCurPage(2);
	gui_drawCompass(true);

	if (refresh && !_sceneDrawPage2)
		drawScene(0);

	_screen->setCurPage(cp);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	if (!_loading)
		_screen->updateScreen();

	_screen->loadEoBBitmap("INVENT", _cgaMappingInv, 5, 3, 2);

	if (_flags.platform == Common::kPlatformAmiga) {
		if (_flags.gameID == GI_EOB1) {
			_screen->getPalette(0).copy(_screen->getPalette(1), 1, 5, 1);
		} else {
			_screen->setDualPalettes(_screen->getPalette(6), _screen->getPalette(1));
			_screen->getPalette(7).copy(_screen->getPalette(1), 0, 32);
		}
	}
}

void EoBCoreEngine::gui_restorePlayField() {
	loadVcnData(0, _cgaLevelMappingIndex ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
	_screen->_curPage = 0;
	gui_drawPlayField(true);
	gui_drawAllCharPortraitsWithStats();
}

void EoBCoreEngine::gui_drawAllCharPortraitsWithStats() {
	for (int i = 0; i < 6; i++)
		gui_drawCharPortraitWithStats(i);
}

void EoBCoreEngine::gui_drawCharPortraitWithStats(int index) {
	if (!testCharacter(index, 1))
		return;

	static const uint16 charPortraitPosX[] = { 8, 80, 184, 256 };
	static const uint16 charPortraitPosY[] = { 2, 54, 106 };

	EoBCharacter *c = &_characters[index];
	int txtCol1 = guiSettings()->colors.guiColorBlack;
	int txtCol2 = guiSettings()->colors.guiColorWhite;

	if ((_flags.gameID == GI_EOB1 && c->flags & 6) || (_flags.gameID == GI_EOB2 && c->flags & 0x0E)) {
		txtCol1 = guiSettings()->colors.guiColorDarkRed;
		txtCol2 = guiSettings()->colors.guiColorLightRed;
	}

	if (_currentControlMode == 0) {
		int x2 = charPortraitPosX[index & 1];
		int y2 = charPortraitPosY[index >> 1];
	
		_screen->copyRegion(176, 168, x2 , y2, 64, 24, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(240, 168, x2, y2 + 24, 64, 26, 2, 2, Screen::CR_NO_P_CHECK);
		int cp = _screen->setCurPage(2);

		Screen::FontId cf = _screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_SMALL_FNT : Screen::FID_6_FNT);

		if (index == _exchangeCharacterId)
			_screen->printText(_characterGuiStringsSt[0], x2 + 2, y2 + 2, guiSettings()->colors.guiColorDarkRed, guiSettings()->colors.fill);
		else
			_screen->printText(c->name, x2 + 2, y2 + (_flags.platform == Common::kPlatformFMTowns ? 1 : 2), txtCol1, _flags.use16ColorMode ? 0 : guiSettings()->colors.fill);

		_screen->setFont(Screen::FID_6_FNT);

		gui_drawFaceShape(index);
		gui_drawWeaponSlot(index, 0);
		gui_drawWeaponSlot(index, 1);
		gui_drawHitpoints(index);

		if (testCharacter(index, 2))
			gui_drawCharPortraitStatusFrame(index);

		if (c->damageTaken > 0) {
			_screen->drawShape(2, _redSplatShape, x2 + 13, y2 + 30, 0);
			Common::String tmpStr = Common::String::format("%d", c->damageTaken);
			_screen->printText(tmpStr.c_str(), x2 + 34 - tmpStr.size() * 3, y2 + 42, (_configRenderMode == Common::kRenderCGA) ? 12 : guiSettings()->colors.guiColorWhite, 0);
		}

		_screen->setCurPage(cp);
		_screen->setFont(cf);

		if (!cp) {
			_screen->copyRegion(x2, y2, charPortraitPosX[2 + (index & 1)], y2, 64, 50, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}
	} else if ((_currentControlMode == 1 || _currentControlMode == 2) && index == _updateCharNum) {
		_screen->copyRegion(176, 0, 0, 0, 144, 168, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->_curPage = 2;
		gui_drawFaceShape(index);
		Screen::FontId cf = _screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_SMALL_FNT : Screen::FID_6_FNT);
		_screen->printShadedText(c->name, 219, 6, txtCol2, 0, guiSettings()->colors.guiColorBlack);
		_screen->setFont(Screen::FID_6_FNT);
		gui_drawHitpoints(index);
		gui_drawFoodStatusGraph(index);

		if (_currentControlMode == 1) {
			int statusTxtY = 158;
			if (_flags.lang == Common::JA_JPN) {
				statusTxtY = 157;
				_screen->setFont(_flags.platform == Common::kPlatformFMTowns ? Screen::FID_8_FNT : Screen::FID_SJIS_FNT);
			}

			if (c->hitPointsCur == -10)
				_screen->printShadedText(_characterGuiStringsSt[1], 247, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);
			else if (c->hitPointsCur < 1)
				_screen->printShadedText(_characterGuiStringsSt[2], 226, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);
			else if (c->effectFlags & 0x2000)
				_screen->printShadedText(_characterGuiStringsSt[3], 220, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);
			else if (c->flags & 2)
				_screen->printShadedText(_characterGuiStringsSt[4], 235, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);
			else if (c->flags & 4)
				_screen->printShadedText(_characterGuiStringsSt[5], 232, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);
			else if (c->flags & 8)
				_screen->printShadedText(_characterGuiStringsSt[6], 232, statusTxtY, guiSettings()->colors.guiColorLightRed, 0, guiSettings()->colors.guiColorBlack);

			_screen->setFont(Screen::FID_6_FNT);

			for (int i = 0; i < 27; i++)
				gui_drawInventoryItem(i, 0, 2);
			gui_drawInventoryItem(16, 1, 2);

			_screen->setFont(cf);

		} else {
			static const uint16 cm2X1[] = { 179, 272, 301 };
			static const uint16 cm2Y1[] = { 36, 51, 51 };
			static const uint16 cm2X2[] = { 271, 300, 318 };
			static const uint16 cm2Y2[] = { 165, 165, 147 };

			for (int i = 0; i < 3; i++)
				_screen->fillRect(cm2X1[i], cm2Y1[i], cm2X2[i], cm2Y2[i], guiSettings()->colors.sfill);

			_screen->setFont(cf);

			int lineH = MIN(_screen->getFontHeight() + 1, 8);
			_screen->printShadedText(_characterGuiStringsIn[0], 183, 42, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill, guiSettings()->colors.guiColorBlack);
			_screen->printText(_chargenClassStrings[c->cClass], 183, 55, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
			_screen->printText(_chargenAlignmentStrings[c->alignment], 183, 55 + lineH, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
			_screen->printText(_chargenRaceSexStrings[c->raceSex], 183, 55 + 2 * lineH, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);

			lineH = _screen->getFontHeight() + 1;
			int tX = 183;
			int tY = _flags.use16ColorMode ? 87 : 82;
			for (int i = 0; i < 3; i++)
				_screen->printText(_chargenStatStrings[6 + i], tX, tY + i * lineH, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
			
			if (_flags.use16ColorMode) {
				tX += 72;
				tY -= 27;
			}			
			for (int i = 3; i < 6; i++)
				_screen->printText(_chargenStatStrings[6 + i], tX, tY + i * lineH, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);

			_screen->printText(_characterGuiStringsIn[1], 183, tY + 6 * lineH, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
			
			tY = _flags.use16ColorMode ? 127 : 138;
			_screen->printText(_characterGuiStringsIn[2], 239, tY, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
			_screen->printText(_characterGuiStringsIn[3], 278, tY, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);

			tX = _flags.use16ColorMode ? 210 : 275;
			tY = _flags.use16ColorMode ? 87 : 82;
			_screen->printText(getCharStrength(c->strengthCur, c->strengthExtCur).c_str(), tX, tY, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			_screen->printText(Common::String::format("%d", c->intelligenceCur).c_str(), tX, tY + lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			_screen->printText(Common::String::format("%d", c->wisdomCur).c_str(), tX, tY + 2 * lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			
			if (_flags.use16ColorMode) {
				tX = 285;
				tY -= 27;
			}
			_screen->printText(Common::String::format("%d", c->dexterityCur).c_str(), tX, tY + 3 * lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			_screen->printText(Common::String::format("%d", c->constitutionCur).c_str(), tX, tY + 4 * lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			_screen->printText(Common::String::format("%d", c->charismaCur).c_str(), tX, tY + 5 * lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			
			if (_flags.use16ColorMode)
				tX = 255;
			_screen->printText(Common::String::format("%d", c->armorClass).c_str(), tX, tY + 6 * lineH, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);

			tY = _flags.use16ColorMode ? 136 : 145;
			for (int i = 0; i < 3; i++) {
				int t = getCharacterClassType(c->cClass, i);
				if (t == -1)
					continue;
				tX = (_flags.use16ColorMode) ? 183 : 180;
				_screen->printText(_chargenClassStrings[t + 15], tX, tY + lineH * i, guiSettings()->colors.guiColorBlack, guiSettings()->colors.sfill);
				Common::String tmpStr = Common::String::format("%d", c->experience[i]);
				_screen->printText(tmpStr.c_str(), 251 - (_screen->getTextWidth(tmpStr.c_str()) >> 1), tY + lineH * i, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
				tmpStr = Common::String::format("%d", c->level[i]);
				_screen->printText(tmpStr.c_str(), 286 - (_screen->getTextWidth(tmpStr.c_str()) >> 1), tY + lineH * i, guiSettings()->colors.guiColorWhite, guiSettings()->colors.sfill);
			}
		}

		_screen->_curPage = 0;
		_screen->copyRegion(176, 0, 176, 0, 144, 168, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, 176, 0, 144, 168, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void EoBCoreEngine::gui_drawFaceShape(int index) {
	if (!testCharacter(index, 1))
		return;

	static const uint8 xCoords[] = { 8, 80 };
	static const uint8 yCoords[] = { 11, 63, 115 };

	int x = xCoords[index & 1];
	int y = yCoords[index >> 1];

	if (!_screen->_curPage)
		x += 176;

	if (_currentControlMode) {
		if (_updateCharNum != index)
			return;

		x = 181;
		y = 3;
	}

	EoBCharacter *c = &_characters[index];

	if (c->hitPointsCur == -10) {
		_screen->drawShape(_screen->_curPage, _deadCharShape, x, y, 0);
		return;
	}

	if (_flags.gameID == GI_EOB1) {
		if (c->effectFlags & 0x40) {
			_screen->fillRect(x, y, x + 31, y + 31, guiSettings()->colors.guiColorBlack);
			return;
		}
	} else {
		if (c->effectFlags & 0x140) {
			_screen->setFadeTable(_blackFadingTable);
			_screen->setShapeFadingLevel(1);
		}

		if (c->flags & 2) {
			_screen->setFadeTable(_greenFadingTable);
			_screen->setShapeFadingLevel(1);
		}

		if (c->flags & 8) {
			_screen->setFadeTable(_blueFadingTable);
			_screen->setShapeFadingLevel(1);
		}
	}

	_screen->drawShape(_screen->_curPage, c->faceShape, x, y, 0);

	if (c->hitPointsCur < 1)
		_screen->drawShape(_screen->_curPage, _disabledCharGrid, x, y, 0);

	if (c->flags & 8 || c->flags & 2 || c->effectFlags & 0x140) {
		_screen->setFadeTable(_greyFadingTable);
		_screen->setShapeFadingLevel(0);
	}
}

void EoBCoreEngine::gui_drawWeaponSlot(int charIndex, int slot) {
	static const uint8 xCoords[] = { 40, 112 };
	static const uint8 yCoords[] = { 11, 27, 63, 79, 115, 131 };

	int x = xCoords[charIndex & 1];
	int y = yCoords[(charIndex & 6) + slot];

	if (!_screen->_curPage)
		x += 176;

	int itm = _characters[charIndex].inventory[slot];
	gui_drawBox(x, y, 31, 16, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);

	if (_characters[charIndex].slotStatus[slot]) {
		gui_drawWeaponSlotStatus(x, y, _characters[charIndex].slotStatus[slot]);
		return;
	}

	if (itm)
		drawItemIconShape(_screen->_curPage, itm, x + 8, y);
	else if (!slot && _flags.gameID == GI_EOB2 && checkScriptFlags(0x80000000))
		_screen->drawShape(_screen->_curPage, _itemIconShapes[103], x + 8, y, 0);
	else
		_screen->drawShape(_screen->_curPage, _itemIconShapes[85 + slot], x + 8, y, 0);

	if ((_characters[charIndex].disabledSlots & (1 << slot)) || !validateWeaponSlotItem(charIndex, slot) || (_characters[charIndex].hitPointsCur <= 0) || (_characters[charIndex].flags & 0x0C))
		_screen->drawShape(_screen->_curPage, _weaponSlotGrid, x, y, 0);
}

void EoBCoreEngine::gui_drawWeaponSlotStatus(int x, int y, int status) {
	Common::String tmpStr;
	Common::String tmpStr2;

	if (status > -3 || status == -5)
		_screen->drawShape(_screen->_curPage, _greenSplatShape, x - 1, y, 0);
	else
		gui_drawBox(x, y, 31, 16, guiSettings()->colors.warningFrame1, guiSettings()->colors.warningFrame2, guiSettings()->colors.warningFill);

	switch (status + 5) {
	case 0:
		tmpStr = _characterGuiStringsWp[2];
		break;
	case 1:
		tmpStr = _characterGuiStringsWr[2];
		tmpStr2 = _characterGuiStringsWr[3];
		break;
	case 2:
		tmpStr = _characterGuiStringsWr[0];
		tmpStr2 = _characterGuiStringsWr[1];
		break;
	case 3:
		tmpStr = _characterGuiStringsWp[1];
		break;
	case 4:
		tmpStr = _characterGuiStringsWp[0];
		break;
	default:
		tmpStr = Common::String::format("%d", status);
		break;
	}

	int textColor = (_configRenderMode == Common::kRenderCGA) ? 2 : guiSettings()->colors.guiColorWhite;

	if (!tmpStr2.empty()) {
		_screen->printText(tmpStr.c_str(), x + (16 - tmpStr.size() * 3), y + 2, textColor, 0);
		_screen->printText(tmpStr2.c_str(), x + (16 - tmpStr.size() * 3), y + 9, textColor, 0);
	} else {
		_screen->printText(tmpStr.c_str(), x + (16 - tmpStr.size() * 3), y + 5, textColor, 0);
	}
}

void EoBCoreEngine::gui_drawHitpoints(int index) {
	if (!testCharacter(index, 1))
		return;

	if (_currentControlMode && (index != _updateCharNum))
		return;

	static const uint8 xCoords[] = { 23, 95 };
	static const uint8 yCoords[] = { 46, 98, 150 };

	int x = xCoords[index & 1];
	int y = yCoords[index >> 1];
	int w = 38;
	int h = 3;
	uint8 bgCol = guiSettings()->colors.fill;

	if (!_screen->_curPage)
		x += 176;

	if (_currentControlMode) {
		x = 250;
		y = 16;
		w = 51;
		h = 5;
		if (_flags.platform == Common::kPlatformAmiga && _flags.gameID == GI_EOB1)
			bgCol = guiSettings()->colors.sfill;
	}

	EoBCharacter *c = &_characters[index];

	if (_configHpBarGraphs) {
		int bgCur = c->hitPointsCur + 10;
		int bgMax = c->hitPointsMax + 10;
		int col = ((bgMax / 3) > bgCur) ? guiSettings()->colors.guiColorYellow : guiSettings()->colors.guiColorDarkGreen;
		if (bgCur <= 10)
			col = guiSettings()->colors.guiColorDarkRed;

		if (!_currentControlMode)
			_screen->printText(_characterGuiStringsHp[0], x - 13, y - 1, guiSettings()->colors.guiColorBlack, 0);


		gui_drawHorizontalBarGraph(x, y, w, h, bgCur, bgMax, col, guiSettings()->colors.barGraph);

	} else {
		Common::String tmpString = Common::String::format(_characterGuiStringsHp[1], c->hitPointsCur, c->hitPointsMax);

		if (!_currentControlMode) {
			x -= 13;
			y -= 1;
		}

		_screen->printText(tmpString.c_str(), x, y, guiSettings()->colors.guiColorBlack, bgCol);
	}
}

void EoBCoreEngine::gui_drawFoodStatusGraph(int index) {
	if (!_currentControlMode)
		return;

	if (!testCharacter(index, 1))
		return;

	EoBCharacter *c = &_characters[index];
	if (!(c->flags & 1))
		return;

	if (index != _updateCharNum)
		return;

	uint8 col = c->food < 20 ? guiSettings()->colors.guiColorDarkRed : (c->food < 33 ? guiSettings()->colors.guiColorYellow : guiSettings()->colors.guiColorDarkGreen);
	gui_drawHorizontalBarGraph(250, 25, 51, 5, c->food, 100, col, guiSettings()->colors.barGraph);
}

void EoBCoreEngine::gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 curVal, int32 maxVal, int col1, int col2) {
	gui_drawBox(x - 1, y - 1, w + 3, h + 2, guiSettings()->colors.frame2, guiSettings()->colors.frame1, -1);
	KyraRpgEngine::gui_drawHorizontalBarGraph(x, y, w + 2, h, curVal, maxVal, col1, col2);
}

void EoBCoreEngine::gui_drawCharPortraitStatusFrame(int index) {
	uint8 redGreenColor = (_partyEffectFlags & 0x20000) ? guiSettings()->colors.guiColorLightGreen : ((_configRenderMode == Common::kRenderCGA) ? 3 : guiSettings()->colors.guiColorLightRed);

	static const uint8 xCoords[] = { 8, 80 };
	static const uint8 yCoords[] = { 2, 54, 106 };
	int x = xCoords[index & 1];
	int y = yCoords[index >> 1];
	int xOffset = (_configRenderMode == Common::kRenderCGA) ? 0 : 1;

	if (!_screen->_curPage)
		x += 176;

	EoBCharacter *c = &_characters[index];

	bool redGreen = ((c->effectFlags & 0x4818) || (_partyEffectFlags & 0x20000) || c->effectsRemainder[0] || c->effectsRemainder[1]) ? true : false;
	bool yellow = ((c->effectFlags & 0x13000) || (_partyEffectFlags & 0x8420)) ? true : false;

	if (redGreen || yellow) {
		if (redGreen && !yellow) {
			_screen->drawBox(x, y, x + 63, y + 49, redGreenColor);
			return;
		}

		if (yellow && !redGreen) {
			_screen->drawBox(x, y, x + 63, y + 49, guiSettings()->colors.guiColorYellow);
			return;
		}

		int iX = x;
		int iY = y;

		for (int i = 0; i < 64; i += 16) {
			x = iX + i;
			if (redGreen) {
				_screen->drawClippedLine(x, y, x + 7, y, redGreenColor);
				_screen->drawClippedLine(x + 8, y + 49, x + 15, y + 49, redGreenColor);
			}
			if (yellow) {
				_screen->drawClippedLine(x + 8, y, x + 15, y, guiSettings()->colors.guiColorYellow);
				_screen->drawClippedLine(x, y + 49, x + 7, y + 49, guiSettings()->colors.guiColorYellow);
			}
		}

		x = iX;

		for (int i = 1; i < 48; i += 12) {
			y = iY + i - 1;

			if (yellow) {
				_screen->drawClippedLine(x, y + 1, x, y + 6, guiSettings()->colors.guiColorYellow);
				_screen->drawClippedLine(x + 63, y + 7, x + 63, y + 12, guiSettings()->colors.guiColorYellow);
			}
			if (redGreen) {
				_screen->drawClippedLine(x, y + 7, x, y + 12, redGreenColor);
				_screen->drawClippedLine(x + 63, y + 1, x + 63, y + 6, redGreenColor);
			}
		}

	} else {
		_screen->drawClippedLine(x, y, x + 62, y, guiSettings()->colors.frame2);
		_screen->drawClippedLine(x, y + 49, x + 62, y + 49, guiSettings()->colors.frame1);
		_screen->drawClippedLine(x - xOffset, y, x - xOffset, y + 50, guiSettings()->colors.guiColorBlack);
		_screen->drawClippedLine(x + 63, y, x + 63, y + 50, guiSettings()->colors.guiColorBlack);
	}
}

void EoBCoreEngine::gui_drawInventoryItem(int slot, int redraw, int pageNum) {
	int x = _inventorySlotsX[slot];
	int y = _inventorySlotsY[slot];

	int item = _characters[_updateCharNum].inventory[slot];
	int cp = _screen->setCurPage(pageNum);

	if (redraw) {
		int wh = (slot == 25 || slot == 26) ? 10 : 18;

		uint8 col1 = guiSettings()->colors.frame1;
		uint8 col2 = guiSettings()->colors.frame2;
		if (_flags.gameID == GI_EOB1 && _flags.platform == Common::kPlatformAmiga) {
			col1 = guiSettings()->colors.inactiveTabFrame1;
			col2 = guiSettings()->colors.inactiveTabFrame2;
		} else if (_configRenderMode == Common::kRenderCGA) {
			col1 = 1;
			col2 = 3;
		}

		gui_drawBox(x - 1, y - 1, wh, wh, col1, col2, slot == 16 ? -1 : guiSettings()->colors.fill);

		if (slot == 16) {
			_screen->fillRect(227, 65, 238, 69, guiSettings()->colors.guiColorBlack);
			int cnt = countQueuedItems(_characters[_updateCharNum].inventory[slot], -1, -1, 1, 1);
			x = cnt >= 10 ? 227 : 233;
			Screen::FontId cf = _screen->setFont(Screen::FID_6_FNT);
			Common::String str = Common::String::format("%d", cnt);
			_screen->printText(str.c_str(), x, 65, guiSettings()->colors.guiColorWhite, 0);
			_screen->setFont(cf);
		}
	}

	if (slot != 16 && item) {
		if (slot == 25 || slot == 26) {
			x -= 4;
			y -= 4;
		}
		drawItemIconShape(pageNum, item, x, y);
	}
	_screen->_curPage = cp;
	_screen->updateScreen();
}

void EoBCoreEngine::gui_drawCompass(bool force) {
	if (_currentDirection == _compassDirection && !force)
		return;

	static const uint8 shpX[2][3] = { { 0x70, 0x4D, 0x95 }, { 0x72, 0x4F, 0x97 } };
	static const uint8 shpY[2][3] = { { 0x7F, 0x9A, 0x9A }, { 0x83, 0x9E, 0x9E } };
	int g = _flags.gameID == GI_EOB1 ? 0 : 1;

	for (int i = 0; i < 3; i++)
		_screen->drawShape(_screen->_curPage, _compassShapes[(i << 2) + _currentDirection], shpX[g][i], shpY[g][i], 0);

	_compassDirection = _currentDirection;
}

void EoBCoreEngine::gui_drawDialogueBox() {
	_screen->set16bitShadingLevel(4);
	gui_drawBox(0, 121, 320, 79, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	txt()->clearCurDim();
	_screen->set16bitShadingLevel(0);
}

void EoBCoreEngine::gui_drawSpellbook() {
	_screen->setCurPage(2);
	int numTab = (_flags.gameID == GI_EOB1) ? 5 : 6;
	_screen->copyRegion(64, 121, 64, 121, 112, 56, 0, 2, Screen::CR_NO_P_CHECK);

	Screen::FontId of = (_flags.gameID == GI_EOB1 && _flags.platform == Common::kPlatformPC98) ? _screen->setFont(Screen::FID_SJIS_SMALL_FNT) : _screen->_currentFont;

	for (int i = 0; i < numTab; i++) {
		int col1 = 0;
		int col2 = 1;
		int col3 = 2;

		if (_configRenderMode == Common::kRenderCGA) {
			if (i == _openBookSpellLevel) {
				col1 = 1;
				col2 = 2;
				col3 = 3;
			}
		} else {
			col1 = guiSettings()->colors.inactiveTabFrame1;
			col2 = guiSettings()->colors.inactiveTabFrame2;
			col3 = guiSettings()->colors.inactiveTabFill;

			if (i == _openBookSpellLevel) {
				col1 = guiSettings()->colors.frame1;
				col2 = _flags.platform == Common::kPlatformAmiga ? 31 : guiSettings()->colors.frame2;
				col3 = _flags.platform == Common::kPlatformAmiga ? guiSettings()->colors.frame2 : guiSettings()->colors.fill;
			}
		}

		if (_flags.gameID == GI_EOB1) {
			gui_drawBox(i * 21 + 71, 122, 21, 9, col1, col2, col3);
			_screen->printText(_magicStrings7[i], i * 21 + 73, 123, guiSettings()->colors.guiColorBlack, 0);
		} else {
			_screen->set16bitShadingLevel(4);
			gui_drawBox(i * 18 + 68, 121, 18, 9, col1, col2, col3);
			_screen->set16bitShadingLevel(0);
			_screen->printText(Common::String::format("%d", i + 1).c_str(), i * 18 + 75, 123, guiSettings()->colors.guiColorBlack, 0);
		}
	}

	if (_flags.gameID == GI_EOB1)
		gui_drawBox(71, 131, 105, 44, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	else {
		_screen->set16bitShadingLevel(4);
		gui_drawBox(68, 130, 108, 47, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
		_screen->set16bitShadingLevel(0);
		gui_drawBox(68, 168, 78, 9, guiSettings()->colors.extraFrame1, guiSettings()->colors.extraFrame2, guiSettings()->colors.extraFill);
		gui_drawBox(146, 168, 14, 9, guiSettings()->colors.extraFrame1, guiSettings()->colors.extraFrame2, guiSettings()->colors.extraFill);
		gui_drawBox(160, 168, 16, 9, guiSettings()->colors.extraFrame1, guiSettings()->colors.extraFrame2, guiSettings()->colors.extraFill);
		gui_drawSpellbookScrollArrow(150, 169, 0);
		gui_drawSpellbookScrollArrow(165, 169, 1);
	}

	int textCol1 = (_configRenderMode == Common::kRenderCGA) ? 3 : guiSettings()->colors.guiColorWhite;
	int textCol2 = guiSettings()->colors.guiColorDarkRed;
	int textXa = 74;
	int textXs = 71;
	int textY = 170;
	int col3 = (_configRenderMode == Common::kRenderCGA) ? 2 : guiSettings()->colors.fill;
	int col4 = guiSettings()->colors.extraFill;
	int col5 = 12;

	if (_flags.gameID == GI_EOB1) {
		textCol2 = (_configRenderMode == Common::kRenderCGA) ? 12 : (_flags.platform == Common::kPlatformAmiga ? 16 : 11);
		textXa = textXs = 73;
		textY = 168;
		col4 = col3;
		col5 = textCol1;
	}

	int textCol3 = _flags.use16ColorMode ? 0 : textCol2;
	int textCol4 = _flags.use16ColorMode ? 0 : col3;

	for (int i = 0; i < 7; i++) {
		int d = _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + i];
		if (_openBookSpellSelectedItem == i) {
			if (d >= 0 && i < 6 && (i + _openBookSpellListOffset) < 9) {
				_screen->fillRect(textXs, 132 + 6 * i, textXs + _screen->getTextWidth(_openBookSpellList[d]) - 1, 137 + 6 * i, textCol2);
				_screen->printText(_openBookSpellList[d], textXs, 132 + 6 * i, textCol1, textCol3);
			} else if (i == 6) {
				if (_flags.gameID == GI_EOB2)
					_screen->fillRect(69, 169, 144, 175, textCol2);
				_screen->printText(_magicStrings1[0], textXa, textY, textCol1, textCol2);
			}
		} else {
			if (d >= 0 && i < 6 && (i + _openBookSpellListOffset) < 9)
				_screen->printText(_openBookSpellList[d], textXs, 132 + 6 * i, textCol1, textCol4);
			else
				_screen->printText(_magicStrings1[0], textXa, textY, col5, col4);
		}
	}

	if (_characters[_openBookChar].disabledSlots & 4) {
		static const uint8 xpos[] = { 0x44, 0x62, 0x80, 0x90 };
		static const uint8 ypos[] = { 0x82, 0x92, 0x98 };
		for (int yc = 0; yc < 3; yc++) {
			for (int xc = 0; xc < 4; xc++)
				_screen->drawShape(_screen->_curPage, _weaponSlotGrid, xpos[xc], ypos[yc], 0);
		}
	}

	if (_openBookAvailableSpells[_openBookSpellLevel * 10 + 6] <= 0)
		_screen->drawShape(2, _blackBoxWideGrid, 146, 168, 0);

	_screen->setFont(of);
	_screen->setCurPage(0);
	_screen->copyRegion(64, 121, 64, 121, 112, 56, 2, 0, Screen::CR_NO_P_CHECK);
	if (!_loading)
		_screen->updateScreen();
}

void EoBCoreEngine::gui_drawSpellbookScrollArrow(int x, int y, int direction) {
	static const uint8 x1[] = { 0, 2, 1, 0, 2, 2 };
	static const uint8 x2[] = { 2, 4, 5, 6, 4, 4 };
	if (direction) {
		_screen->setPagePixel(_screen->_curPage, x + 3, y + 5, 12);
		for (int i = 1; i < 6; i++)
			_screen->drawClippedLine(x + x1[i], (5 - i) + y, x + x2[i], (5 - i) + y, 12);
	} else {
		_screen->setPagePixel(_screen->_curPage, x + 3, y, 12);
		for (int i = 1; i < 6; i++)
			_screen->drawClippedLine(x + x1[i], y + i, x + x2[i], y + i, 12);
	}
}

void EoBCoreEngine::gui_updateSlotAfterScrollUse() {
	_characters[_openBookChar].disabledSlots ^= (1 << (--_castScrollSlot));
	setCharEventTimer(_openBookChar, 18, _castScrollSlot + 2, 1);
	gui_drawCharPortraitWithStats(_openBookChar);
	_openBookChar = _openBookCharBackup;
	_openBookType = _openBookTypeBackup;
	_castScrollSlot = 0;
	gui_toggleButtons();
}

void EoBCoreEngine::gui_updateControls() {
	Button b;
	if (_currentControlMode)
		clickedPortraitRestore(&b);
	if (_updateFlags)
		clickedSpellbookAbort(&b);
}

void EoBCoreEngine::gui_toggleButtons() {
	if (_currentControlMode == 0)
		gui_setPlayFieldButtons();
	else if (_currentControlMode == 1)
		gui_setInventoryButtons();
	else if (_currentControlMode == 2)
		gui_setStatsListButtons();
}

void EoBCoreEngine::gui_setPlayFieldButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList2 : _buttonList1);
}

void EoBCoreEngine::gui_setInventoryButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList5 : _buttonList3);
}

void EoBCoreEngine::gui_setStatsListButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList6 : _buttonList4);
}

void EoBCoreEngine::gui_setSwapCharacterButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList7);
}

void EoBCoreEngine::gui_setCastOnWhomButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList8);
}

void EoBCoreEngine::gui_initButton(int index, int, int, int) {
	Button *b = 0;
	int cnt = 1;

	if (_flags.gameID == GI_EOB1 && index > 92)
		return;

	if (_activeButtons) {
		Button *n = _activeButtons;
		while (n->nextButton) {
			++cnt;
			n = n->nextButton;
		}

		++cnt;
		b = n->nextButton = &_activeButtonData[cnt];
	} else {
		b = &_activeButtonData[0];
		_activeButtons = b;
	}

	*b = Button();
	b->data0Val2 = 12;
	b->data1Val2 = b->data2Val2 = 15;
	b->data3Val2 = 8;

	b->index = index + 1;

	const EoBGuiButtonDef *d = &_buttonDefs[index];
	b->buttonCallback = _buttonCallbacks[index];

	if (_flags.gameID == GI_EOB1) {
		// EOB1 spellbook modifications
		if (index > 60 && index < 66) {
			d = &_buttonDefs[index + 34];
			b->buttonCallback = _buttonCallbacks[index + 34];
		} else if (index == 88) {
			d = &_buttonDefs[index + 12];
			b->buttonCallback = _buttonCallbacks[index + 12];
		}
	}

	b->x = d->x;
	b->y = d->y;
	b->width = d->w;
	b->height = d->h;

	// EOB1 spellbook modifications
	if (_flags.gameID == GI_EOB1 && ((index > 66 && index < 73) || (index > 76 && index < 79)))
		b->y++;

	b->flags = d->flags;
	b->keyCode = d->keyCode;
	b->keyCode2 = d->keyCode2;
	b->arg = d->arg;
}

int EoBCoreEngine::clickedCharPortraitDefault(Button *button) {
	if (!testCharacter(button->arg, 1))
		return 1;

	gui_processCharPortraitClick(button->arg);
	return 0;
}

int EoBCoreEngine::clickedCamp(Button *button) {
	gui_updateControls();
	disableSysTimer(2);
	int cd = _screen->curDimIndex();

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		_characters[i].damageTaken = 0;
		_characters[i].slotStatus[0] = _characters[i].slotStatus[1] = 0;
		gui_drawCharPortraitWithStats(i);
	}

	_screen->copyPage(0, 7);
	
	// Create a thumbnail from the screen for a possible savegame.
	// This ensures that all special rendering (EGA dithering, 16bit rendering, Japanese font rendering) will be visible on the thumbnail.
	::createThumbnailFromScreen(&_thumbNail);

	_screen->copyRegion(0, 120, 0, 0, 176, 24, 0, 12, Screen::CR_NO_P_CHECK);

	_gui->runCampMenu();

	_screen->fillRect(0, 0, 175, 143, 0, 2);
	_screen->copyRegion(0, 0, 0, 120, 176, 24, 12, 2, Screen::CR_NO_P_CHECK);
	_screen->setScreenDim(cd);

	_thumbNail.free();
	 
	drawScene(0);

	for (int i = 0; i < 6; i++)
		sortCharacterSpellList(i);

	_screen->setCurPage(0);
	const ScreenDim *dm = _screen->getScreenDim(10);
	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->updateScreen();

	enableSysTimer(2);
	advanceTimers(_restPartyElapsedTime);
	_restPartyElapsedTime = 0;

	checkPartyStatus(true);

	return button->arg;
}

int EoBCoreEngine::clickedSceneDropPickupItem(Button *button) {
	uint16 block = _currentBlock;
	if (button->arg > 1) {
		block = calcNewBlockPosition(_currentBlock, _currentDirection);
		int f = _wllWallFlags[_levelBlockProperties[block].walls[_sceneDrawVarDown]];
		if (!(f & 0x0B))
			return 1;
	}
	int d = _dropItemDirIndex[(_currentDirection << 2) + button->arg];

	if (_itemInHand) {
		setItemPosition((Item *)&_levelBlockProperties[block & 0x3FF].drawObjects, block, _itemInHand, d);
		setHandItem(0);
		runLevelScript(block, 4);
	} else {
		d = getQueuedItem((Item *)&_levelBlockProperties[block].drawObjects, d, -1);
		if (!d)
			return 1;
		setHandItem(d);
		runLevelScript(block, 8);
	}

	_sceneUpdateRequired = true;
	return 1;
}

int EoBCoreEngine::clickedCharPortrait2(Button *button) {
	if (!_gui->_progress) {
		if (!testCharacter(button->arg, 1))
			return button->index;
	}

	_currentControlMode = 1;
	if (!_gui->_progress)
		_updateCharNum = button->arg;

	_screen->copyRegion(176, 0, 0, 0, 144, 168, 0, 5, Screen::CR_NO_P_CHECK);
	gui_drawCharPortraitWithStats(_updateCharNum);
	gui_setInventoryButtons();

	return button->index;
}

int EoBCoreEngine::clickedWeaponSlot(Button *button) {
	if (!testCharacter(button->arg, 1))
		return 1;

	// Fix this using the coordinates from gui_drawWeaponSlot().
	// The coordinates used in the original are slightly wrong
	// (most noticeable for characters 5 and 6).
	static const uint8 sY[] = { 27, 27, 79, 79, 131, 131 };
	int slot = sY[button->arg] > _mouseY ? 0 : 1;

	uint16 flags = _configMouseBtSwap ? _gui->_flagsMouseRight : _gui->_flagsMouseLeft;

	if ((flags & 0x7F) == 1)
		gui_processWeaponSlotClickLeft(button->arg, slot);
	else
		gui_processWeaponSlotClickRight(button->arg, slot);

	return 1;
}

int EoBCoreEngine::clickedCharNameLabelRight(Button *button) {
	if (!testCharacter(button->arg, 1))
		return button->index;

	if (_updateFlags) {
		Button b;
		clickedSpellbookAbort(&b);
	}

	if (_exchangeCharacterId == -1) {
		_exchangeCharacterId = button->arg;
		gui_setSwapCharacterButtons();
		gui_drawCharPortraitWithStats(_exchangeCharacterId);
		enableTimer(0);
	} else {
		int d = _exchangeCharacterId;
		_exchangeCharacterId = -1;
		exchangeCharacters(d, button->arg);

		_timer->disable(0);
		gui_drawCharPortraitWithStats(d);
		gui_processCharPortraitClick(button->arg);
		gui_drawCharPortraitWithStats(button->arg);
		gui_setPlayFieldButtons();
		setupCharacterTimers();
	}

	return button->index;
}

int EoBCoreEngine::clickedInventorySlot(Button *button) {
	gui_processInventorySlotClick(button->arg);
	return button->index;
}

int EoBCoreEngine::clickedEatItem(Button *button) {
	eatItemInHand(_updateCharNum);
	return button->index;
}

int EoBCoreEngine::clickedInventoryPrevChar(Button *button) {
	if (_gui->_progress == 1)
		_updateCharNum = 0;
	else if (_gui->_progress == 2)
		_updateCharNum = 1;
	else
		_updateCharNum = getNextValidCharIndex(_updateCharNum, -1);

	gui_drawCharPortraitWithStats(_updateCharNum);
	return button->index;
}

int EoBCoreEngine::clickedInventoryNextChar(Button *button) {
	int oldVal = _updateCharNum;
	int v = button->arg == 2 ? 2 : 0;

	if (_gui->_progress == 1)
		_updateCharNum = v + 2;
	else if (_gui->_progress == 2)
		_updateCharNum = v + 3;
	else
		_updateCharNum = getNextValidCharIndex(_updateCharNum, 1);

	if (!testCharacter(_updateCharNum, 1)) {
		_updateCharNum = oldVal;
		return 1;
	}

	gui_drawCharPortraitWithStats(_updateCharNum);
	return button->index;
}

int EoBCoreEngine::clickedSpellbookTab(Button *button) {
	_openBookSpellLevel = button->arg;
	_openBookSpellListOffset = 0;

	for (_openBookSpellSelectedItem = 0; _openBookSpellSelectedItem < 6; _openBookSpellSelectedItem++) {
		if (_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellSelectedItem] > 0)
			break;
	}

	gui_drawSpellbook();

	_characters[_openBookChar].slotStatus[3] = _openBookSpellLevel;
	_characters[_openBookChar].slotStatus[2] = _openBookSpellSelectedItem;
	_characters[_openBookChar].slotStatus[4] = _openBookSpellListOffset;

	return button->index;
}

int EoBCoreEngine::clickedSpellbookList(Button *button) {
	int listIndex = button->arg;
	bool spellLevelAvailable = false;

	if (listIndex == 6) {
		for (int i = 0; i < 10; i++) {
			if (_openBookAvailableSpells[_openBookSpellLevel * 10 + i] > 0) {
				spellLevelAvailable = true;
				break;
			}
		}
		if (!spellLevelAvailable)
			return button->index;

		int v = (_gui->_progress == 1) ? -1 : ((_gui->_progress == 2) ? 1 : 0);

		_openBookSpellSelectedItem += _openBookSpellListOffset;
		if (_openBookSpellSelectedItem == 12 || (_openBookSpellSelectedItem == 6 && _openBookSpellListOffset == 0))
			_openBookSpellSelectedItem = 9;

		do {
			_openBookSpellSelectedItem += v;
			int s = (_openBookSpellSelectedItem >= 0) ? _openBookSpellSelectedItem : 9;
			_openBookSpellSelectedItem = (s <= 9) ? s : 0;
		} while (_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellSelectedItem] <= 0 && _openBookSpellSelectedItem != 9);

		if (_openBookSpellSelectedItem >= 6) {
			_openBookSpellListOffset = 6;
			if (_openBookSpellSelectedItem == 9)
				_openBookSpellSelectedItem = 6;
			else
				_openBookSpellSelectedItem -= 6;
		} else {
			_openBookSpellListOffset = 0;
		}

		if (_openBookSpellListOffset == 6 && _openBookAvailableSpells[_openBookSpellLevel * 10 + 6] <= 0)
			_openBookSpellListOffset = 0;

		gui_drawSpellbook();

	} else {
		if (listIndex == 7 || _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + listIndex] > 0) {
			if (listIndex < 6) {
				if (_openBookSpellListOffset + listIndex < 9)
					_openBookSpellSelectedItem = listIndex;
				else if (listIndex != 7)
					return button->index;
			} else if (listIndex != 7) {
				return button->index;
			}

			if (_openBookSpellSelectedItem < 6 && ((_openBookSpellSelectedItem + _openBookSpellListOffset) < 9)) {
				if (_characters[_openBookChar].disabledSlots & 4)
					return button->index;

				gui_drawSpellbook();

				int s = _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem];
				if (_openBookType == 1)
					s += _clericSpellOffset;

				castSpell(s, 0);

			} else if ((_openBookSpellSelectedItem == 6 && listIndex == 7) || (_openBookSpellSelectedItem != 6 && listIndex == 6)) {
				Button b;
				clickedSpellbookAbort(&b);
			}
		}
	}

	_characters[_openBookChar].slotStatus[2] = _openBookSpellSelectedItem;
	_characters[_openBookChar].slotStatus[4] = _openBookSpellListOffset;
	return button->index;
}

int EoBCoreEngine::clickedCastSpellOnCharacter(Button *button) {
	_activeSpellCharId = button->arg & 0xFF;

	if (_activeSpellCharId == 0xFF) {
		printWarning(_magicStrings3[_flags.gameID == GI_EOB1 ? 2 : 1]);
		if (_castScrollSlot) {
			gui_updateSlotAfterScrollUse();
		} else {
			gui_toggleButtons();
			gui_drawSpellbook();
		}
	} else {
		if (_characters[_activeSpellCharId].flags & 1)
			startSpell(_activeSpell);
	}

	return button->index;
}

int EoBCoreEngine::clickedInventoryNextPage(Button *button) {
	if (_currentControlMode == 2) {
		gui_setInventoryButtons();
		_currentControlMode = 1;
	} else {
		gui_setStatsListButtons();
		_currentControlMode = 2;
	}

	gui_drawCharPortraitWithStats(_updateCharNum);
	return button->index;
}

int EoBCoreEngine::clickedPortraitRestore(Button *button) {
	_currentControlMode = 0;
	_screen->_curPage = 2;
	_screen->fillRect(0, 0, 143, 167, 0);
	_screen->copyRegion(0, 0, 0, 0, 144, 168, 5, _screen->_curPage, Screen::CR_NO_P_CHECK);
	gui_drawAllCharPortraitsWithStats();
	_screen->_curPage = 0;
	_screen->copyRegion(0, 0, 176, 0, 144, 168, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	gui_setPlayFieldButtons();
	return button->index;
}

int EoBCoreEngine::clickedUpArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, _currentDirection);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EoBCoreEngine::clickedDownArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection + 2) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EoBCoreEngine::clickedLeftArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection - 1) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EoBCoreEngine::clickedRightArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection + 1) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EoBCoreEngine::clickedTurnLeftArrow(Button *button) {
	_currentDirection = (_currentDirection - 1) & 3;
	//_keybControlUnk = -1;
	_sceneDefaultUpdate = 1;
	_sceneUpdateRequired = true;
	return button->index;
}

int EoBCoreEngine::clickedTurnRightArrow(Button *button) {
	_currentDirection = (_currentDirection + 1) & 3;
	//_keybControlUnk = -1;
	_sceneDefaultUpdate = 1;
	_sceneUpdateRequired = true;
	return button->index;
}

int EoBCoreEngine::clickedAbortCharSwitch(Button *button) {
	_timer->disable(0);
	int c = _exchangeCharacterId;
	_exchangeCharacterId = -1;
	gui_drawCharPortraitWithStats(c);
	gui_setPlayFieldButtons();
	return button->index;
}

int EoBCoreEngine::clickedSceneThrowItem(Button *button) {
	if (!_itemInHand)
		return button->index;

	if (launchObject(_updateCharNum, _itemInHand, _currentBlock, _dropItemDirIndex[(_currentDirection << 2) + button->arg], _currentDirection, _items[_itemInHand].type)) {
		setHandItem(0);
		_sceneUpdateRequired = true;
	}

	return button->index;
}

int EoBCoreEngine::clickedSceneSpecial(Button *button) {
	_clickedSpecialFlag = 0x40;
	return specialWallAction(calcNewBlockPosition(_currentBlock, _currentDirection), _currentDirection);
}

int EoBCoreEngine::clickedSpellbookAbort(Button *button) {
	_updateFlags = 0;
	_screen->fillRect(64, 121, 175, 176, 0, 0);
	_screen->fillRect(64, 121, 175, 176, 0, 2);
	_screen->copyRegion(0, 0, 64, 121, 112, 56, 10, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	gui_drawCompass(true);
	gui_toggleButtons();
	return button->index;
}

int EoBCoreEngine::clickedSpellbookScroll(Button *button) {
	if (_openBookAvailableSpells[_openBookSpellLevel * 10] > 0) {
		_openBookSpellListOffset ^= 6;
		_openBookSpellSelectedItem = 0;
	} else {
		_openBookSpellListOffset = 6;
	}

	_characters[_openBookChar].slotStatus[2] = _openBookSpellSelectedItem;
	_characters[_openBookChar].slotStatus[4] = _openBookSpellListOffset;

	gui_drawSpellbook();

	return button->index;
}

int EoBCoreEngine::clickedUnk(Button *button) {
	return button->index;
}

void EoBCoreEngine::gui_processCharPortraitClick(int index) {
	if (index == _updateCharNum)
		return;

	int a = _updateCharNum;
	_updateCharNum = index;

	gui_drawCharPortraitWithStats(a);
	gui_drawCharPortraitWithStats(index);
}

void EoBCoreEngine::gui_processWeaponSlotClickLeft(int charIndex, int slotIndex) {
	int itm = _characters[charIndex].inventory[slotIndex];
	if (_items[itm].flags & 0x20)
		return;

	int ih = _itemInHand;
	int t = _items[ih].type;
	uint16 v = (ih) ? _itemTypes[t].invFlags : 0xFFFF;

	if (v & _slotValidationFlags[slotIndex]) {
		setHandItem(itm);
		_characters[charIndex].inventory[slotIndex] = ih;
		gui_drawCharPortraitWithStats(charIndex);
	}

	recalcArmorClass(charIndex);
}

void EoBCoreEngine::gui_processWeaponSlotClickRight(int charIndex, int slotIndex) {
	if (!testCharacter(charIndex, 0x0D))
		return;

	Item itm = _characters[charIndex].inventory[slotIndex];
	int wslot = slotIndex < 2 ? slotIndex : -1;

	if (slotIndex < 2 && (!validateWeaponSlotItem(charIndex, slotIndex) || (!_currentControlMode && (_characters[charIndex].disabledSlots & (1 << slotIndex)))))
		return;

	if (!itemUsableByCharacter(charIndex, itm))
		_txt->printMessage(_itemMisuseStrings[0], -1, _characters[charIndex].name);

	if (!itm && slotIndex > 1)
		return;

	int8 tp = _items[itm].type;
	int8 vl = _items[itm].value;
	uint8 ep = _itemTypes[tp].extraProperties & 0x7F;

	switch (ep) {
	case 0:
	case 16:
		// Item automatically used when worn
		_txt->printMessage(_itemMisuseStrings[1]);
		break;

	case 1:
	case 2:
	case 3:
		// Weapons
		if (!_currentControlMode)
			useSlotWeapon(charIndex, slotIndex, itm);
		break;

	case 4:
	case 8:
	case 12:
	case 13:
	case 15:
		// Item not used that way
		_txt->printMessage(_itemMisuseStrings[2]);
		break;

	case 5:
	case 6:
		// Cleric holy symbol / mage spell book
		if (!_currentControlMode)
			useMagicBookOrSymbol(charIndex, ep == 6 ? 1 : 0);
		break;

	case 7:
		// Food ration
		// Don't do anything if mouse control is enabled (we don't support anything else)
		// eatItemInHand(charIndex);
		break;

	case 10:
		if (_flags.gameID == GI_EOB1)
			vl += _clericSpellOffset;
		// fall through
	case 9:
		// Mage/Cleric Scroll
		if (!_currentControlMode)
			useMagicScroll(charIndex, vl, wslot);
		break;

	case 11:
		// Letters, Notes, Maps
		displayParchment(vl);
		break;

	case 14:
		// Potion
		usePotion(charIndex, slotIndex);
		break;

	case 18:
		useWand(charIndex, slotIndex);
		break;

	case 19:
		// eob2 horn
		useHorn(charIndex, slotIndex);
		break;

	case 20:
		if (vl == 1)
			inflictCharacterDamage(charIndex, 200);
		else
			useMagicScroll(charIndex, 55, slotIndex);
		deleteInventoryItem(charIndex, slotIndex);
		break;

	default:
		break;
	}

	if (_flags.gameID == GI_EOB1 || (ep == 1 && charIndex >= 2))
		return;

	_lastUsedItem = itm;
	runLevelScript(calcNewBlockPosition(_currentBlock, _currentDirection), 0x100);
	_lastUsedItem = 0;
}

void EoBCoreEngine::gui_processInventorySlotClick(int slot) {
	int itm = _characters[_updateCharNum].inventory[slot];
	int ih = _itemInHand;
	if (!validateInventorySlotForItem(ih, _updateCharNum, slot))
		return;

	if (slot == 16) {
		if (ih) {
			setItemPosition(&_characters[_updateCharNum].inventory[16], -2, ih, 0);
			gui_drawInventoryItem(slot, 1, 0);
			setHandItem(0);

		} else {
			itm = getQueuedItem(&_characters[_updateCharNum].inventory[16], 0, -1);
			gui_drawInventoryItem(slot, 1, 0);
			setHandItem(itm);
		}

	} else {
		setHandItem(itm);
		_characters[_updateCharNum].inventory[slot] = ih;
		gui_drawInventoryItem(slot, 1, 0);
		recalcArmorClass(_updateCharNum);
	}
}

GUI_EoB::GUI_EoB(EoBCoreEngine *vm) : GUI(vm), _vm(vm), _screen(vm->_screen) {
	_menuStringsPrefsTemp = new char*[4];
	memset(_menuStringsPrefsTemp, 0, 4 * sizeof(char *));

	_saveSlotStringsTemp = new char*[6];
	for (int i = 0; i < 6; i++) {
		_saveSlotStringsTemp[i] = new char[26];
		memset(_saveSlotStringsTemp[i], 0, 26);
	}
	_saveSlotIdTemp = new int16[6];
	_savegameOffset = 0;
	_saveSlotX = _saveSlotY = 0;

	_specialProcessButton = _backupButtonList = 0;
	_flagsMouseLeft = _flagsMouseRight = _flagsModifier = 0;
	_backupButtonList = 0;
	_progress = 0;
	_prcButtonUnk3 = 1;
	_cflag = 0xFFFF;

	_menuLineSpacing = 0;
	_menuLastInFlags = 0;
	_menuCur = 0;
	_menuNumItems = 0;

	_numPages = (_vm->game() == GI_EOB2) ? 8 : 5;
	_numVisPages = (_vm->game() == GI_EOB2) ? 6 : 5;
	_clericSpellAvltyFlags = (_vm->game() == GI_EOB2) ? 0xF7FFFFFF : 0x7BFFFF;
	_paladinSpellAvltyFlags = (_vm->game() == GI_EOB2) ? 0xA9BBD1D : 0x800FF2;
	_numAssignedSpellsOfType = new int8[72];
	memset(_numAssignedSpellsOfType, 0, 72);

	_charSelectRedraw = false;

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		_highLightColorTable = _highlightColorTableAmiga;
	else if (_vm->game() == GI_EOB1 && (_vm->_configRenderMode == Common::kRenderCGA || _vm->_configRenderMode == Common::kRenderEGA))
		_highLightColorTable = _highlightColorTableEGA;
	else if (_vm->game() == GI_EOB1 && _vm->gameFlags().platform == Common::kPlatformPC98)
		_highLightColorTable = _highlightColorTablePC98;
	else
		_highLightColorTable = _highlightColorTableVGA;

	_updateBoxIndex = -1;
	_highLightBoxTimer = 0;
	_updateBoxColorIndex = 0;

	_needRest = false;
}

GUI_EoB::~GUI_EoB() {
	if (_menuStringsPrefsTemp) {
		for (int i = 0; i < 4; i++)
			delete[] _menuStringsPrefsTemp[i];
		delete[] _menuStringsPrefsTemp;
	}

	if (_saveSlotStringsTemp) {
		for (int i = 0; i < 6; i++)
			delete[] _saveSlotStringsTemp[i];
		delete[] _saveSlotStringsTemp;
	}

	delete[] _saveSlotIdTemp;

	delete[] _numAssignedSpellsOfType;
}

void GUI_EoB::processButton(Button *button) {
	if (!button->data0Val1 && !button->data2Val1 && !button->data1Val1)
		return;

	if ((button->flags & 0x18) == 0x18)
		return;

	int sd = button->dimTableIndex;
	const ScreenDim *dm = _screen->getScreenDim(sd);

	int fx = button->x;
	if (fx < 0)
		fx += (dm->w << 3);

	int sx = fx + (dm->sx << 3);

	int fy = button->y;
	if (fy < 0)
		fy += dm->h;

	int sy = fy + dm->sy;

	uint16 fw = button->width;
	uint16 fh = button->height;

	uint8 col1 = button->data1Val1;
	uint8 col2 = button->data1Val3;

	int fx2 = sx + fw - 1;
	int fy2 = sy + fh - 1;

	if (button->flags2 & 1) {
		if (button->data1Val1 == 1) {
			if (button->data0Val1 == 1) {
				_screen->drawShape(_screen->_curPage, button->data1ShapePtr, fx, fy, sd);
			} else if (button->data0Val1 == 2) {
				if (!(button->flags2 & 4))
					_screen->printText((const char *)button->data1ShapePtr, sx, sy, col1, col2);
			} else if (button->data0Val1 == 3) {
				// nullsub (at least EOBII)
			} else if (button->data0Val1 == 4) {
				if (button->data1Callback)
					(*button->data1Callback)(button);
			}
		} else if (button->data1Val1 == 2) {
			if (!(button->flags2 & 4))
				_screen->drawBox(sx, sy, fx2, fy2, col1);
		} else if (button->data1Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data1Val1 == 4) {
			if (button->data1Callback)
				(*button->data1Callback)(button);
		}
	}

	if (button->flags2 & 4) {
		if (button->data2Val1 == 1) {
			if (button->data0Val1 == 1) {
				_screen->drawShape(_screen->_curPage, button->data2ShapePtr, fx, fy, sd);
			} else if (button->data0Val1 == 2) {
				if (button->flags2 & 1)
					_screen->printText((const char *)button->data2ShapePtr, sx, sy, button->data3Val2, button->data3Val3);
				else
					_screen->printText((const char *)button->data2ShapePtr, sx, sy, button->data2Val2, button->data2Val3);
			} else if (button->data0Val1 == 3) {
				// nullsub (at least EOBII)
			} else if (button->data0Val1 == 4) {
				if (button->data2Callback)
					(*button->data2Callback)(button);
			}
		} else if (button->data2Val1 == 2) {
			_screen->drawBox(sx, sy, fx2, fy2, (button->flags2 & 1) ? button->data3Val2 : button->data2Val2);
		} else if (button->data2Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data2Val1 == 4) {
			if (button->data2Callback)
				(*button->data2Callback)(button);
		}
	}

	if (!(button->flags2 & 5)) {
		if (button->data0Val1 == 1) {
			_screen->drawShape(_screen->_curPage, button->data0ShapePtr, fx, fy, sd);
		} else if (button->data0Val1 == 2) {
			_screen->printText((const char *)button->data0ShapePtr, sx, sy, button->data0Val2, button->data0Val3);
		} else if (button->data0Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data0Val1 == 4) {
			if (button->data0Callback)
				(*button->data0Callback)(button);
		} else if (button->data0Val1 == 5) {
			_screen->drawBox(sx, sy, fx2, fy2, button->data0Val2);
		} else {
			if (!button->data0Val1) {
				if (button->data1Val1 == 2 || button->data2Val1 == 2) {
					_screen->drawBox(sx, sy, fx2, fy2, button->data0Val2);
				} else {
					// nullsub (at least EOBII)
				}
			}
		}
	}
}

int GUI_EoB::processButtonList(Kyra::Button *buttonList, uint16 inputFlags, int8 mouseWheel) {
	_progress = 0;
	uint16 in = inputFlags & 0xFF;
	uint16 buttonReleaseFlag = 0;
	bool clickEvt = false;
	//_vm->_processingButtons = true;
	_flagsMouseLeft = (_vm->_mouseClick == 1) ? 2 : 4;
	_flagsMouseRight = (_vm->_mouseClick == 2) ? 2 : 4;
	_vm->_mouseClick = 0;

	if (mouseWheel) {
		return 204 + mouseWheel;
	} else if (in >= 199 && in <= 202) {
		buttonReleaseFlag = (inputFlags & 0x800) ? 3 : 1;
		if (in < 201)
			_flagsMouseLeft = buttonReleaseFlag;
		else
			_flagsMouseRight = buttonReleaseFlag;

		////////////////////////////
		if (!buttonList && !(inputFlags & 0x800))
			return inputFlags & 0xFF;
		////////////////////////////

		inputFlags = 0;
		clickEvt = true;
	} else if (inputFlags & 0x8000) {
		inputFlags &= 0xFF;
	}

	uint16 result = 0;
	bool runLoop = true;

	if (!buttonList)
		return inputFlags;

	if (_vm->_buttonListChanged || (buttonList != _backupButtonList)) {
		_backupButtonList = buttonList;
		_flagsModifier = 0;

		while (runLoop) {
			processButton(buttonList);
			_flagsModifier |= (buttonList->flags & 0xAA04);

			// UNUSED
			//if (buttonList->flags2 & 0x20) {
			//	if (_processButtonListExtraCallback)
			//		this->*_processButtonListExtraCallback(buttonList);
			//}

			if (buttonList->nextButton)
				buttonList = buttonList->nextButton;
			else
				runLoop = false;
		}

		_vm->_buttonListChanged = false;

		_specialProcessButton = 0;
		_prcButtonUnk3 = 1;
		_cflag = 0xFFFF;
	}

	int sd = 0;
	const ScreenDim *dm = _screen->getScreenDim(sd);

	int x1 = dm->sx << 3;
	int y1 = dm->sy;
	int w1 = dm->w << 3;
	int h1 = dm->h;

	uint16 v8 = 0;
	uint16 v18 = 0;
	uint16 v16 = 0;

	if (_specialProcessButton)
		buttonList = _specialProcessButton;

	while (runLoop) {
		if (buttonList->flags & 8) {
			buttonList = buttonList->nextButton;
			runLoop = buttonList ? true : false;
			continue;
		}

		int vc = 0;
		int v6 = 0;
		uint16 iFlag = buttonList->index | 0x8000;
		uint16 flgs2 = buttonList->flags2;
		uint16 flgs = buttonList->flags;

		if (flgs2 & 1)
			flgs2 |= 8;
		else
			flgs2 &= 0xFFF7;

		if (flgs2 & 4)
			flgs2 |= 0x10;
		else
			flgs2 &= 0xFFEF;

		uint16 vL = 0;
		uint16 vR = 0;

		if (inputFlags) {
			if (buttonList->keyCode == inputFlags) {
				_progress = 1;
				_flagsMouseLeft = 1;
				flgs2 ^= 1;
				result = iFlag;
				v6 = 1;
			} else if (buttonList->keyCode2 == inputFlags) {
				_progress = 2;
				_flagsMouseRight = 1;
				result = iFlag;
				v6 = 1;
			}
		} else if (_flagsModifier || clickEvt) {
			vL = flgs & 0xF00;
			vR = flgs & 0xF000;

			if (_prcButtonUnk3) {
				if (sd != buttonList->dimTableIndex) {
					sd = buttonList->dimTableIndex;
					dm = _screen->getScreenDim(sd);
					x1 = dm->sx << 3;
					y1 = dm->sy;
					w1 = dm->w << 3;
					h1 = dm->h;
				}

				int x2 = x1;
				if (buttonList->x < 0)
					x2 += w1;
				x2 += buttonList->x;

				int y2 = y1;
				if (buttonList->y < 0)
					y2 += h1;
				y2 += buttonList->y;

				if (_vm->_mouseX >= x2 && _vm->_mouseX <= (x2 + buttonList->width) && _vm->_mouseY >= y2 && _vm->_mouseY <= (y2 + buttonList->height)) {
					flgs2 |= 2;

					if (vL) {
						switch (_flagsMouseLeft - 1) {
						case 0:
							v18 = 1;

							if ((flgs & 4) && buttonList->data2Val1) {
								flgs2 |= 4;
								vc = 1;
							} else {
								flgs2 &= 0xFFFB;
							}

							if (flgs & 0x100) {
								v6 = 1;
								if (!(flgs & 1)) {
									flgs2 ^= 1;
									result = iFlag;
								}
							}

							if (flgs & 0x40) {
								_specialProcessButton = buttonList;
								v8 = 1;
							}

							_cflag = flgs;
							break;

						case 1:
							if (flgs != _cflag)
								break;

							if ((flgs & 4) && buttonList->data2Val1) {
								flgs2 |= 4;
								vc = 1;
							} else {
								flgs2 &= 0xFFFB;
							}

							if (!(flgs & 0x200))
								break;

							v6 = 1;

							if (flgs & 1)
								break;

							flgs2 |= 1;
							result = iFlag;
							break;

						case 2:
							if (_cflag != flgs)
								break;

							if (flgs & 0x400) {
								v6 = 1;
								if (flgs & 1) {
									flgs2 ^= 1;
									result = iFlag;
								}
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xFFFE;
							break;

						case 3:
							if ((flgs & 4) || (!buttonList->data2Val1))
								flgs2 &= 0xFFFB;
							else
								flgs2 |= 4;

							if (flgs & 0x800) {
								v6 = 1;
								break;
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xFFFE;
							break;

						default:
							break;
						}
					}

					if (vR && !v6 && !vc) {
						switch (_flagsMouseRight - 1) {
						case 0:
							v18 = 1;

							if ((flgs & 4) && buttonList->data2Val1)
								flgs2 |= 4;
							else
								flgs2 &= 0xFFFB;

							if (flgs & 0x1000) {
								v6 = 1;
								if (!(flgs & 1)) {
									flgs2 ^= 1;
									result = iFlag;
								}
							}

							if (flgs & 0x40) {
								_specialProcessButton = buttonList;
								v8 = 1;
							}

							_cflag = flgs;
							break;

						case 1:
							if (flgs != _cflag)
								break;

							if ((flgs & 4) && buttonList->data2Val1)
								flgs2 |= 4;
							else
								flgs2 &= 0xFFFB;

							if (!(flgs & 0x2000))
								break;

							v6 = 1;

							if (flgs & 1)
								break;

							flgs2 |= 1;
							result = iFlag;
							break;
						case 2:
							if (_cflag != flgs)
								break;

							if (flgs & 0x4000) {
								v6 = 1;
								if (flgs & 1) {
									flgs2 ^= 1;
									result = iFlag;
								}
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xFFFE;
							break;

						case 3:
							if ((flgs & 4) || (!buttonList->data2Val1))
								flgs2 &= 0xFFFB;
							else
								flgs2 |= 4;

							if (flgs & 0x8000) {
								v6 = 1;
								break;
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xFFFE;
							break;

						default:
							break;
						}
					}
				} else { // if (_vm->_mouseX >= x2 && _vm->_mouseX <= (x2 + buttonList->width)....)
					flgs2 &= 0xFFF9;

					if ((flgs & 0x40) && (!(flgs & 0x80)) && _specialProcessButton && !v8) {
						static const uint16 flagsTable[] = { 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000 };

						if (vL) {
							v16 = flagsTable[_flagsMouseLeft - 1];
							if (v16 & flgs)
								v6 = 1;
						}

						if (vR && !v6) {
							v16 = flagsTable[_flagsMouseRight + 3];
							if (v16 & flgs)
								v6 = 1;
						}

						if (!v6) {
							_specialProcessButton = 0;
							_prcButtonUnk3 = 1;
						}
					}

					if ((flgs & 2) && (flgs2 & 1))
						flgs2 &= 0xFFFE;
				} // end if (_vm->_mouseX >= x2 && _vm->_mouseX <= (x2 + buttonList->width)....)
			} // end if (_prcButtonUnk3)
		} // end if (_flagsModifier || clickEvt)

		buttonList->flags = flgs;
		buttonList->flags2 = flgs2;

		bool f21 = (flgs2 & 8) ? true : false;
		bool f22 = (flgs2 & 1) ? true : false;
		bool f23 = (flgs2 & 0x10) ? true : false;
		bool f24 = (flgs2 & 4) ? true : false;

		if (f21 != f22 || f23 != f24)
			processButton(buttonList);

		if (v6 && buttonList->buttonCallback)
			runLoop = !(*buttonList->buttonCallback)(buttonList);

		if ((flgs2 & 2) && (flgs & 0x20))
			runLoop = false;

		if (_specialProcessButton && ((vL && _flagsMouseLeft == 3) || (vR && _flagsMouseRight == 3))) {
			_specialProcessButton = 0;
			_prcButtonUnk3 = 1;
			runLoop = false;
		}

		if (_specialProcessButton && !v8)
			runLoop = false;

		buttonList = buttonList->nextButton;
		if (!buttonList)
			runLoop = false;
	};

	if ((_flagsMouseLeft == 1 || _flagsMouseRight == 1) && !v18)
		_cflag = 0xFFFF;

	if (!result)
		result = inputFlags;

	return result;
}

void GUI_EoB::simpleMenu_setup(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int itemOffset, int lineSpacing) {
	simpleMenu_initMenuItemsMask(sd, maxItem, menuItemsMask, itemOffset);

	const ScreenDim *dm = _screen->getScreenDim(19 + sd);
	int x = (_screen->_curDim->sx + dm->sx) << 3;
	int y = _screen->_curDim->sy + dm->sy;

	int v = simpleMenu_getMenuItem(_menuCur, menuItemsMask, itemOffset);

	for (int i = 0; i < _menuNumItems; i++) {
		int item = simpleMenu_getMenuItem(i, menuItemsMask, itemOffset);
		int ty = y + i * (lineSpacing + _screen->getFontHeight());
		_screen->printShadedText(strings[item], x, ty, (_vm->_configRenderMode == Common::kRenderCGA) ? 1 : _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		if (item == v)
			_screen->printText(strings[item], x, ty, _vm->guiSettings()->colors.guiColorLightRed, 0);
	}

	_screen->updateScreen();
	_menuLineSpacing = lineSpacing;
	_menuLastInFlags = 0;
	_vm->removeInputTop();
}

int GUI_EoB::simpleMenu_process(int sd, const char *const *strings, void *b, int32 menuItemsMask, int itemOffset) {
	const ScreenDim *dm = _screen->getScreenDim(19 + sd);
	int h = _menuNumItems - 1;
	int currentItem = _menuCur % _menuNumItems;
	int newItem = currentItem;
	int result = -1;
	int lineH = (_menuLineSpacing + _screen->getFontHeight());
	int lineS1 = _menuLineSpacing >> 1;
	int x = (_screen->_curDim->sx + dm->sx) << 3;
	int y = _screen->_curDim->sy + dm->sy;

	int inFlag = _vm->checkInput(0, false, 0) & 0x8FF;
	_vm->removeInputTop();
	Common::Point mousePos = _vm->getMousePos();

	int x1 = (_screen->_curDim->sx << 3) + (dm->sx * _screen->getFontWidth());
	int y1 = _screen->_curDim->sy + dm->sy - lineS1;
	int x2 = x1 + (dm->w * _screen->getFontWidth()) - 1;
	int y2 = y1 + _menuNumItems * lineH - 1;
	if (_vm->posWithinRect(mousePos.x, mousePos.y, x1, y1, x2, y2))
		newItem = (mousePos.y - y1) / lineH;

	if (inFlag == 199 || inFlag == 201) {
		if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x1, y1, x2, y2))
			result = newItem = (_vm->_mouseY - y1) / lineH;
	} else if (inFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inFlag == _vm->_keyMap[Common::KEYCODE_KP5]) {
		result = newItem;
	} else if (inFlag == _vm->_keyMap[Common::KEYCODE_HOME] || inFlag == _vm->_keyMap[Common::KEYCODE_KP7] || inFlag == _vm->_keyMap[Common::KEYCODE_PAGEUP] || inFlag == _vm->_keyMap[Common::KEYCODE_KP9]) {
		newItem = 0;
	} else if (inFlag == _vm->_keyMap[Common::KEYCODE_END] || inFlag == _vm->_keyMap[Common::KEYCODE_KP1] || inFlag == _vm->_keyMap[Common::KEYCODE_PAGEDOWN] || inFlag == _vm->_keyMap[Common::KEYCODE_KP3]) {
		newItem = h;
	} else if (inFlag == _vm->_keyMap[Common::KEYCODE_UP] || inFlag == _vm->_keyMap[Common::KEYCODE_KP8]) {
		if (--newItem < 0)
			newItem = h;
	} else if (inFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inFlag == _vm->_keyMap[Common::KEYCODE_KP2]) {
		if (++newItem > h)
			newItem = 0;
	} else {
		_menuLastInFlags = inFlag;
	}

	if (newItem != currentItem) {
		_screen->printText(strings[simpleMenu_getMenuItem(currentItem, menuItemsMask, itemOffset)], x, y + currentItem * lineH, (_vm->_configRenderMode == Common::kRenderCGA) ? 1 : _vm->guiSettings()->colors.guiColorWhite, 0);
		_screen->printText(strings[simpleMenu_getMenuItem(newItem,  menuItemsMask, itemOffset)], x, y + newItem * lineH , _vm->guiSettings()->colors.guiColorLightRed, 0);
		_screen->updateScreen();
	}

	if (result != -1) {
		result = simpleMenu_getMenuItem(result, menuItemsMask, itemOffset);
		simpleMenu_flashSelection(strings[result], x, y + newItem * lineH, _vm->guiSettings()->colors.guiColorWhite, _vm->guiSettings()->colors.guiColorLightRed, 0);
	}

	_menuCur = newItem;

	return result;
}

int GUI_EoB::simpleMenu_getMenuItem(int index, int32 menuItemsMask, int itemOffset) {
	if (menuItemsMask == -1)
		return index;

	int res = 0;
	int i = index;

	for (; i; res++) {
		if (menuItemsMask & (1 << (res + itemOffset)))
			i--;
	}

	while (!(menuItemsMask & (1 << (res + itemOffset))))
		res++;

	return res;
}

void GUI_EoB::simpleMenu_flashSelection(const char *str, int x, int y, int color1, int color2, int color3) {
	for (int i = 0; i < 3; i++) {
		_screen->printText(str, x, y, color2, color3);
		_screen->updateScreen();
		_vm->_system->delayMillis(32);
		_screen->printText(str, x, y, color1, color3);
		_screen->updateScreen();
		_vm->_system->delayMillis(32);
	}
}

void GUI_EoB::runCampMenu() {
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	Button *highlightButton = 0;
	Button *prevHighlightButton = 0;

	int newMenu = 0;
	int lastMenu = -1;
	bool redrawPortraits = false;
	bool keepButtons = false;

	_charSelectRedraw = false;
	_needRest = false;
	Button *buttonList = 0;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		if (newMenu == 2)
			updateOptionsStrings();

		if (newMenu != -1) {
			if (!keepButtons) {
				releaseButtons(buttonList);

				_vm->_menuDefs[0].titleStrId = newMenu ? 1 : 56;
				if (newMenu == 2)
					_vm->_menuDefs[2].titleStrId = 57;
				else if (newMenu == 1)
					_vm->_menuDefs[1].titleStrId = 58;

				buttonList = initMenu(newMenu);

				if (newMenu != lastMenu) {
					highlightButton = buttonList;
					prevHighlightButton = 0;
				}
			}

			lastMenu = newMenu;
			newMenu = -1;
			keepButtons = false;
		}

		int inputFlag = _vm->checkInput(buttonList, false, 0) & 0x80FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE])
			inputFlag = 0x8007;
		else if (prevHighlightButton && (inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]))
			inputFlag = 0x8000 + prevHighlightButton->index;

		Button *clickedButton = _vm->gui_getButton(buttonList, inputFlag & 0x7FFF);

		if (clickedButton) {
			drawMenuButton(prevHighlightButton, false, false, true);
			drawMenuButton(clickedButton, true, true, true);
			_screen->updateScreen();
			_vm->_system->delayMillis(80);
			drawMenuButton(clickedButton, false, true, true);
			_screen->updateScreen();
			highlightButton = clickedButton;
			prevHighlightButton = 0;
		}

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP3] || inputFlag == _vm->_keyMap[Common::KEYCODE_PAGEDOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP1] || inputFlag == _vm->_keyMap[Common::KEYCODE_END]) {
			highlightButton = _vm->gui_getButton(buttonList, _vm->_menuDefs[lastMenu].firstButtonStrId + _vm->_menuDefs[lastMenu].numButtons);
			inputFlag = _vm->_keyMap[Common::KEYCODE_UP];
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP7] || inputFlag == _vm->_keyMap[Common::KEYCODE_HOME] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP9] || inputFlag == _vm->_keyMap[Common::KEYCODE_PAGEUP]) {
			highlightButton = _vm->gui_getButton(buttonList, _vm->_menuDefs[lastMenu].firstButtonStrId + 1);
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP8] || inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
			if (prevHighlightButton) {
				int dir = (inputFlag == _vm->_keyMap[Common::KEYCODE_UP]) ? -1 : 1;
				int s = prevHighlightButton->index + dir;
				if (lastMenu == 2)
					s += _vm->_prefMenuPlatformOffset;
				int a = _vm->_menuDefs[lastMenu].firstButtonStrId + 1;
				int b = a + _vm->_menuDefs[lastMenu].numButtons - 1;

				do {
					if (s < a)
						s = b;
					if (s > b)
						s = a;
					if (_vm->_menuButtonDefs[s - 1].flags & 2)
						break;
					s += dir;
				} while (!_vm->shouldQuit());

				if (lastMenu == 2)
					s -= _vm->_prefMenuPlatformOffset;
				highlightButton = _vm->gui_getButton(buttonList, s);
			}

		} else if (inputFlag > 0x8000 && inputFlag < 0x8011) {
			int i = 0;
			int cnt = 0;

			switch (inputFlag) {
			case 0x8001:
				if (restParty())
					runLoop = false;
				else
					_needRest = false;
				redrawPortraits = true;
				newMenu = 0;
				break;

			case 0x8002:
				runMemorizePrayMenu(selectCharacterDialogue(23), 0);
				newMenu = 0;
				break;

			case 0x8003:
				runMemorizePrayMenu(selectCharacterDialogue(26), 1);
				newMenu = 0;
				break;

			case 0x8004:
				scribeScrollDialogue();
				newMenu = 0;
				break;

			case 0x8005:
				newMenu = 2;
				break;

			case 0x8006:
				newMenu = 1;
				break;

			case 0x8007:
				if (_needRest)
					displayTextBox(44);
				// fall through

			case 0x800C:
			case 0x8010:
				if (lastMenu == 1 || lastMenu == 2)
					newMenu = 0;
				else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE])
					newMenu = 0;
				else
					runLoop = false;
				break;

			case 0x8008:
				if (runLoadMenu(0, 0))
					runLoop = false;
				else
					newMenu = 1;
				break;

			case 0x8009:
				if (runSaveMenu(0, 0))
					displayTextBox(14);
				newMenu = 1;
				break;

			case 0x800A:
				for (; i < 6; i++) {
					if (_vm->testCharacter(i, 1))
						cnt++;
				}

				if (cnt > 4) {
					_vm->dropCharacter(selectCharacterDialogue(53));
					_vm->gui_drawPlayField(false);
					_screen->copyRegion(0, 120, 0, 0, 176, 24, 0, 12, Screen::CR_NO_P_CHECK);
					Screen::FontId cfn = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);
					_vm->gui_drawAllCharPortraitsWithStats();
					_screen->setFont(cfn);
				} else {
					displayTextBox(45);
				}

				newMenu = 0;
				break;

			case 0x800B:
				if (confirmDialogue(46))
					_vm->quitGame();
				newMenu = 0;
				break;

			case 0x800D:
				if (_vm->gameFlags().platform == Common::kPlatformPC98) {
					_vm->_configMusic ^= true;
					_vm->writeSettings();
					if (_vm->_configMusic)
						_vm->snd_playSong(_vm->_currentLevel + 1);
					else
						_vm->snd_playSong(0);
				} else {
					_vm->_configSounds ^= true;
					_vm->_configMusic = _vm->_configSounds ? 1 : 0;
				}
				keepButtons = true;
				newMenu = 2;
				break;

			case 0x800E:
				if (_vm->gameFlags().platform == Common::kPlatformPC98)
					_vm->_configSounds ^= true;
				else
					_vm->_configHpBarGraphs ^= true;
				newMenu = 2;
				redrawPortraits = keepButtons = true;
				break;

			case 0x800F:
				if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98) {
					if (_vm->gameFlags().platform == Common::kPlatformFMTowns)
						_vm->_config2431 ^= true;
					else
						_vm->_configHpBarGraphs ^= true;
					newMenu = 2;
					redrawPortraits = keepButtons = true;
				} else {
					newMenu = 0;
				}
				break;

			default:
				break;
			}

			lastMenu = -1;

		} else {
			Common::Point p = _vm->getMousePos();
			for (Button *b = buttonList; b; b = b->nextButton) {
				if ((b->arg & 2) && _vm->posWithinRect(p.x, p.y, b->x, b->y, b->x + b->width, b->y + b->height))
					highlightButton = b;
			}
		}

		if (_charSelectRedraw || redrawPortraits) {
			for (int i = 0; i < 6; i++) {
				_vm->gui_drawCharPortraitWithStats(i);
				_vm->sortCharacterSpellList(i);
			}
		}

		_charSelectRedraw = redrawPortraits = false;

		if (prevHighlightButton != highlightButton && newMenu == -1 && runLoop) {
			drawMenuButton(prevHighlightButton, false, false, true);
			drawMenuButton(highlightButton, false, true, false);
			_screen->updateScreen();
			prevHighlightButton = highlightButton;
		}
	}

	_screen->setFont(of);
	releaseButtons(buttonList);
	_vm->writeSettings();
}

bool GUI_EoB::runLoadMenu(int x, int y, bool fromMainMenu) {
	const ScreenDim *dm = _screen->getScreenDim(11);
	int xo = dm->sx;
	int yo = dm->sy;
	bool result = false;

	_screen->modifyScreenDim(11, dm->sx + (x >> 3), dm->sy + y, dm->w, dm->h);

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		updateSaveSlotsList(_vm->_targetName);
		
		_vm->useMainMenuGUISettings(fromMainMenu);
		int slot = selectSaveSlotDialogue(x, y, 1);
		_vm->useMainMenuGUISettings(false);

		if (slot > 5) {
			runLoop = result = false;
		} else if (slot >= 0) {
			if (_saveSlotIdTemp[slot] == -1) {
				_vm->useMainMenuGUISettings(fromMainMenu);
				messageDialogue(11, 65, _vm->guiSettings()->colors.guiColorLightRed);
				_vm->useMainMenuGUISettings(false);
			} else {
				if (_vm->loadGameState(_saveSlotIdTemp[slot]).getCode() != Common::kNoError) {
					_vm->useMainMenuGUISettings(fromMainMenu);
					messageDialogue(11, 16, _vm->guiSettings()->colors.guiColorLightRed);
					_vm->useMainMenuGUISettings(false);
				}
				runLoop = false;
				result = true;
			}
		}
	}

	_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);

	return result;
}

bool GUI_EoB::confirmDialogue2(int dim, int id, int deflt) {
	int od = _screen->curDimIndex();
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
	_screen->setScreenDim(dim);

	drawTextBox(dim, id);

	int16 x[2];
	x[0] = (_screen->_curDim->sx << 3) + 8;
	x[1] = (_screen->_curDim->sx + _screen->_curDim->w - 5) << 3;
	int16 y = _screen->_curDim->sy + _screen->_curDim->h - 21;
	int newHighlight = deflt ^ 1;
	int lastHighlight = -1;

	for (int i = 0; i < 2; i++)
		drawMenuButtonBox(x[i], y, 32, 14, false, false);

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		Common::Point p = _vm->getMousePos();
		if (_vm->posWithinRect(p.x, p.y, x[0], y, x[0] + 32, y + 14))
			newHighlight = 0;
		else if (_vm->posWithinRect(p.x, p.y, x[1], y, x[1] + 32, y + 14))
			newHighlight = 1;

		int inputFlag = _vm->checkInput(0, false, 0) & 0x8FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP6]) {
			newHighlight ^= 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_n]) {
			newHighlight = 1;
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_y]) {
			newHighlight = 0;
			runLoop = false;
		}  else if (inputFlag == 199 || inputFlag == 201) {
			if (_vm->posWithinRect(p.x, p.y, x[0], y, x[0] + 32, y + 14)) {
				newHighlight = 0;
				runLoop = false;
			} else if (_vm->posWithinRect(p.x, p.y, x[1], y, x[1] + 32, y + 14)) {
				newHighlight = 1;
				runLoop = false;
			}
		}

		if (newHighlight != lastHighlight) {
			for (int i = 0; i < 2; i++)
				_screen->printShadedText(_vm->_menuYesNoStrings[i], x[i] + 16 - (_screen->getTextWidth(_vm->_menuYesNoStrings[i]) / 2) + 1, y + 3, i == newHighlight ? _vm->guiSettings()->colors.guiColorLightRed : _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
			_screen->updateScreen();
			lastHighlight = newHighlight;
		}
	}

	drawMenuButtonBox(x[newHighlight], y, 32, 14, true, true);
	_screen->updateScreen();
	_vm->_system->delayMillis(80);
	drawMenuButtonBox(x[newHighlight], y, 32, 14, false, true);
	_screen->updateScreen();

	_screen->copyRegion(0, _screen->_curDim->h, _screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->setFont(of);
	_screen->setScreenDim(od);

	return newHighlight == 0;
}

void GUI_EoB::messageDialogue(int dim, int id, int buttonTextCol) {
	int od = _screen->curDimIndex();
	_screen->setScreenDim(dim);
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	drawTextBox(dim, id);
	const ScreenDim *dm = _screen->getScreenDim(dim);

	int bx = ((dm->sx + dm->w) << 3) - (_screen->getTextWidth(_vm->_menuOkString) + 16);
	int by = dm->sy + dm->h - 19;
	int bw = _screen->getTextWidth(_vm->_menuOkString) + 7;

	drawMenuButtonBox(bx, by, bw, 14, false, false);
	_screen->printShadedText(_vm->_menuOkString, bx + 4, by + 3, buttonTextCol, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->updateScreen();

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		int inputFlag = _vm->checkInput(0, false, 0) & 0x8FF;
		_vm->removeInputTop();

		if (inputFlag == 199 || inputFlag == 201) {
			if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, bx, by, bx + bw, by + 14))
				runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_o]) {
			runLoop = false;
		}
	}

	drawMenuButtonBox(bx, by, bw, 14, true, true);
	_screen->updateScreen();
	_vm->_system->delayMillis(80);
	drawMenuButtonBox(bx, by, bw, 14, false, true);
	_screen->updateScreen();

	_screen->copyRegion(0, dm->h, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->setScreenDim(od);
	_screen->setFont(of);
	dm = _screen->getScreenDim(dim);
}

void GUI_EoB::messageDialogue2(int dim, int id, int buttonTextCol) {
	_screen->_curPage = 2;
	_screen->setClearScreenDim(dim);
	drawMenuButtonBox(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, false, false);
	_screen->printShadedText(getMenuString(id), (_screen->_curDim->sx << 3) + 5, _screen->_curDim->sy + 5, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->_curPage = 0;
	_screen->copyRegion(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, 2, 0, Screen::CR_NO_P_CHECK);

	int x = (_screen->_curDim->sx << 3) + (_screen->_curDim->w << 2) - (_screen->getTextWidth(_vm->_menuOkString) / 2);
	int y = _screen->_curDim->sy + _screen->_curDim->h - 21;
	int w = _screen->getTextWidth(_vm->_menuOkString) + 8;
	drawMenuButtonBox(x, y, w, 14, false, false);
	_screen->printShadedText(_vm->_menuOkString, x + 4, y + 3, buttonTextCol, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->updateScreen();

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		int inputFlag = _vm->checkInput(0, false, 0) & 0x8FF;
		_vm->removeInputTop();

		if (inputFlag == 199 || inputFlag == 201) {
			if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x, y, x + w, y + 14))
				runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_o]) {
			runLoop = false;
		}
	}

	_screen->set16bitShadingLevel(4);
	_vm->gui_drawBox(x, y, w, 14, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill, -1);
	_screen->set16bitShadingLevel(0);
	_screen->updateScreen();
	_vm->_system->delayMillis(80);
	drawMenuButtonBox(x, y, w, 14, false, false);
	_screen->printShadedText(_vm->_menuOkString, x + 4, y + 3, buttonTextCol, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->updateScreen();

}

void GUI_EoB::updateBoxFrameHighLight(int box) {
	if (_updateBoxIndex == box) {
		if (_updateBoxIndex == -1)
			return;

		if (_vm->_system->getMillis() <= _highLightBoxTimer)
			return;

		if (!_highLightColorTable[_updateBoxColorIndex])
			_updateBoxColorIndex = 0;

		const EoBRect16 *r = &_highlightFrames[_updateBoxIndex];
		_screen->drawBox(r->x1, r->y1, r->x2, r->y2, _highLightColorTable[_updateBoxColorIndex++]);
		_screen->updateScreen();

		_highLightBoxTimer = _vm->_system->getMillis() + _vm->_tickLength;

	} else {
		if (_updateBoxIndex != -1) {
			const EoBRect16 *r = &_highlightFrames[_updateBoxIndex];
			_screen->drawBox(r->x1, r->y1, r->x2, r->y2, 12);
			_screen->updateScreen();
		}

		_updateBoxColorIndex = 0;
		_updateBoxIndex = box;
		_highLightBoxTimer = _vm->_system->getMillis();
	}
}

int GUI_EoB::getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor) {
	// Disable the keymap during text input
	Common::Keymapper *const mapper = _vm->_eventMan->getKeymapper();
	Common::Keymap *const lolKeymap = mapper->getKeymap(EoBCoreEngine::kKeymapName);
	lolKeymap->setEnabled(false);

	uint8 cursorState = 1;
	char sufx[3] = " \0";

	int len = strlen(dest);
	if (len > destMaxLen) {
		len = destMaxLen;
		dest[destMaxLen] = 0;
	}

	int pos = len;
	if (len >= destMaxLen)
		pos--;

	_screen->copyRegion((x - 1) << 3, y, 0, 191, (destMaxLen + 2) << 3, 9, 0, 2, Screen::CR_NO_P_CHECK);
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns)
		_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	_screen->printShadedText(dest, x << 3, y, textColor1, textColor2, _vm->guiSettings()->colors.guiColorBlack);

	uint32 next = _vm->_system->getMillis() + 2 * _vm->_tickLength;
	sufx[0] = (pos < len) ? dest[pos] : 32;
	_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);

	_menuCur = -1;
	printKatakanaOptions(0);

	int bytesPerChar = (_vm->_flags.platform == Common::kPlatformFMTowns) ? 2 : 1;
	int in = 0;

	do {
		in = 0;
		_keyPressed.reset();

		while (!in && !_vm->shouldQuit()) {
			if (next <= _vm->_system->getMillis()) {
				if (cursorState) {
					_screen->copyRegion((pos + 1) << 3, 191, (x + pos) << 3, y, 8, 9, 2, 0, Screen::CR_NO_P_CHECK);
					_screen->printShadedText(sufx, (x + pos) << 3, y, textColor1, textColor2, _vm->guiSettings()->colors.guiColorBlack);
				} else {
					_screen->fillRect((x + pos) << 3, y, ((x + pos) << 3) + 7, y + 7, cursorColor);
					_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);
				}

				_screen->updateScreen();
				cursorState ^= 1;
				next = _vm->_system->getMillis() + 4 * _vm->_tickLength;
			}

			_vm->updateInput();
			in = checkKatakanaSelection();

			for (Common::List<KyraEngine_v1::Event>::const_iterator evt = _vm->_eventList.begin(); evt != _vm->_eventList.end(); ++evt) {
				if (evt->event.type == Common::EVENT_KEYDOWN) {
					_keyPressed = evt->event.kbd;
					in = _keyPressed.ascii;

					if (_vm->_flags.platform == Common::kPlatformFMTowns && _keyPressed.ascii > 31 && _keyPressed.ascii < 123) {
						Common::String s;
						s.insertChar(in & 0xff, 0);
						s = _vm->convertAsciiToSjis(s);
						if (s.empty()) {
							in = 0;
						} else {
							_csjis[0] = s[0];
							_csjis[1] = s[1];
							_csjis[2] = 0;
							in = 0x89;
						}
					}
				}
			}
			_vm->removeInputTop();
		}

		if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE) {
			if (pos > 0 && pos < len ) {
				for (int i = pos; i < len; i++) {
					if (bytesPerChar == 2 && dest[i * bytesPerChar] & 0x80) {
						dest[(i - 1) * bytesPerChar] = dest[i * bytesPerChar];
						dest[(i - 1) * bytesPerChar + 1] = dest[i * bytesPerChar + 1];
					} else {
						dest[i - 1] = dest[i];
					}
				}
			}

			if (pos > 0) {
				if (bytesPerChar == 2 && dest[(len - 1) * bytesPerChar] & 0x80)
					dest[--len * bytesPerChar] = 0;
				else
					dest[--len] = 0;
				pos--;
			}

		} else if (_keyPressed.keycode == Common::KEYCODE_LEFT || _keyPressed.keycode == Common::KEYCODE_KP4) {
			if (pos > 0)
				pos--;

		} else if (_keyPressed.keycode == Common::KEYCODE_RIGHT || _keyPressed.keycode == Common::KEYCODE_KP6) {
			if (pos < len && pos < (destMaxLen - 1))
				pos++;

		} else if ((in > 31 && in < 126) || (in == 0x89)) {
			if (!(in == 32 && pos == 0)) {
				// EOBI/PC-98 is the only version that allows small characters
				if (in >= 97 && in <= 122 && !_vm->_flags.use16ColorMode)
					in -= 32;

				if (pos < len) {
					for (int i = destMaxLen - 2; i >= pos; i--) {
						if (bytesPerChar == 2 && in == 0x89) {
							dest[(i + 1) * bytesPerChar] = dest[i * bytesPerChar];
							dest[(i + 1) * bytesPerChar + 1] = dest[i * bytesPerChar + 1];
						} else {
							dest[i + 1] = dest[i];
						}
					}

					if (bytesPerChar == 2 && in == 0x89) {
						dest[pos * bytesPerChar] = _csjis[0];
						dest[pos++ * bytesPerChar + 1] = _csjis[1];
						if (len == destMaxLen)
							dest[len * bytesPerChar] = 0;
					} else {
						dest[pos++] = in;
						if (len == destMaxLen)
							dest[len] = 0;
					}
				} else {
					if (pos == destMaxLen) {
						pos--;
						len--;
					}

					if (bytesPerChar == 2 && in == 0x89) {
						dest[pos * bytesPerChar] = _csjis[0];
						dest[pos * bytesPerChar + 1] = _csjis[1];
						dest[++pos * bytesPerChar] = 0;
					} else {
						dest[pos++] = in;
						dest[pos] = 0;
					}
				}

				if (++len > destMaxLen)
					len = destMaxLen;

				if (pos > (destMaxLen - 1))
					pos = (destMaxLen - 1);
			}
		}

		_screen->copyRegion(0, 191, (x - 1) << 3, y, (destMaxLen + 2) << 3, 9, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->printShadedText(dest, x << 3, y, textColor1, textColor2, _vm->guiSettings()->colors.guiColorBlack);
		
		if (_vm->_flags.platform == Common::kPlatformFMTowns) {
			if (pos < len) {
				sufx[0] = dest[pos * bytesPerChar];
				sufx[1] = dest[pos * bytesPerChar + 1];
			} else {
				sufx[0] = 32;
				sufx[1] = 0;
			}			
		} else {
			sufx[0] = (pos < len) ? dest[pos] : 32;
		}

		if (cursorState)
			_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);
		else
			_screen->printShadedText(sufx, (x + pos) << 3, y, textColor1, textColor2, _vm->guiSettings()->colors.guiColorBlack);
		_screen->updateScreen();

	} while (_keyPressed.keycode != Common::KEYCODE_RETURN && _keyPressed.keycode != Common::KEYCODE_ESCAPE && !_vm->shouldQuit());

	lolKeymap->setEnabled(true);

	return _keyPressed.keycode == Common::KEYCODE_ESCAPE ? -1 : len;
}

int GUI_EoB::checkKatakanaSelection() {
	if (_vm->_flags.platform != Common::kPlatformFMTowns)
		return 0;

	static uint16 kanaSelXCrds[] = { 224, 272, 186 };
	Common::Point mousePos = _vm->getMousePos();
	int highlight = -1;
	_csjis[0] = _csjis[2] = 0;

	for (int y = 112; y < 168; y += 16) {
		for (int x = 152; x < 288; x += 8) {
			if (!_vm->posWithinRect(mousePos.x, mousePos.y, x, y, x + 9, y + 9))
				continue;

			int lineOffs = (y - 112) >> 4;
			int column = (x - 152) >> 2;

			_csjis[0] = _vm->_katakanaLines[_currentKanaPage * 4 + lineOffs][column];
			_csjis[1] = _vm->_katakanaLines[_currentKanaPage * 4 + lineOffs][column + 1];

			if (_csjis[0] != '\x81' || _csjis[1] != '\x40') {
				highlight = lineOffs << 8 | column;
				_screen->printShadedText(_csjis, x & ~7, y & ~15, _vm->guiSettings()->colors.guiColorLightRed, 0, _vm->guiSettings()->colors.guiColorBlack);
			}

			x = 288; y = 168;
		}
	}

	if (highlight == -1) {
		for (int i = 0; i < 3; i++) {
			if (!_vm->posWithinRect(mousePos.x, mousePos.y, kanaSelXCrds[i], 176, kanaSelXCrds[i] + _screen->getTextWidth(_vm->_katakanaSelectStrings[i]), 184))
				continue;

			highlight = 0x400 | i;
			_screen->printShadedText(_vm->_katakanaSelectStrings[i], kanaSelXCrds[i], 176, _vm->guiSettings()->colors.guiColorLightRed, 0, _vm->guiSettings()->colors.guiColorBlack);
			i = 3;
		}
	}

	int in = 0;
	for (Common::List<KyraEngine_v1::Event>::const_iterator evt = _vm->_eventList.begin(); evt != _vm->_eventList.end(); ++evt) {
		if (evt->event.type == Common::EVENT_LBUTTONDOWN)
			in = 1;
	}

	if ((highlight == -1 || highlight == _menuCur) && !in)
		return 0;

	if (_menuCur != -1) {
		if (_menuCur & 0x400) {
			_screen->printShadedText(_vm->_katakanaSelectStrings[_menuCur & 3], kanaSelXCrds[_menuCur & 3], 176, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		} else {
			char osjis[3];
			osjis[0] = _vm->_katakanaLines[_currentKanaPage * 4 + (_menuCur >> 8)][_menuCur & 0xFF];
			osjis[1] = _vm->_katakanaLines[_currentKanaPage * 4 + (_menuCur >> 8)][(_menuCur & 0xFF) + 1];
			osjis[2] = 0;
			_screen->printShadedText(osjis, 152 + ((_menuCur & 0xFF) << 2), 112 + ((_menuCur >> 4) & ~0x0F), _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		}
	}

	_menuCur = highlight;

	if (in && highlight != -1) {
		if (highlight & 0x400) {
			switch (highlight & 3) {
			case 0:
				printKatakanaOptions((_currentKanaPage + 1) % 3);
				break;
			case 1:
				_keyPressed.keycode = Common::KEYCODE_RETURN;
				break;
			case 2:
				_keyPressed.keycode = Common::KEYCODE_BACKSPACE;
				break;
			default:
				break;
			}
		} else if (_csjis[0]) {
			if (_csjis[0] == '\x81' && _csjis[1] == '\x51')
				_csjis[1] = '\x40';
			return 0x89;
		}
	}

	return in;
}

void GUI_EoB::printKatakanaOptions(int page) {
	if (_vm->_flags.platform != Common::kPlatformFMTowns)
		return;

	_currentKanaPage = page;
	_screen->copyRegion(160, 44, 144, 108, 160, 84, 2, 0, Screen::CR_NO_P_CHECK);
	for (int i = 0; i < 4; i++)
		_screen->printShadedText(_vm->_katakanaLines[page * 4 + i], 152, (i << 4) + 112, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

	static uint16 kanaSelCrds[] = { 224, 272, 186 };
	for (int i = 0; i < 3; i++)
		_screen->printShadedText(_vm->_katakanaSelectStrings[i], kanaSelCrds[i], 176, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
}

void GUI_EoB::transferWaitBox() {
	const ScreenDim *dm = _screen->getScreenDim(11);
	int xo = dm->sx;
	int yo = dm->sy;
	_screen->modifyScreenDim(11, dm->sx + 9, dm->sy + 24, dm->w, dm->h);
	displayTextBox(-4);
	_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);
}

Common::String GUI_EoB::transferTargetMenu(Common::Array<Common::String> &targets) {
	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
	}

	_savegameListSize = targets.size();
	_savegameList = new char*[_savegameListSize];
	memset(_savegameList, 0, _savegameListSize * sizeof(char *));

	Common::StringArray::iterator ii = targets.begin();
	for (int i = 0; i < _savegameListSize; ++i) {
		_savegameList[i] = new char[(*ii).size() + 1];
		strcpy(_savegameList[i], (*ii++).c_str());
	}

	const ScreenDim *dm = _screen->getScreenDim(11);
	int xo = dm->sx;
	int yo = dm->sy;
	_screen->modifyScreenDim(11, dm->sx + 9, dm->sy + 14, dm->w, dm->h);

	int slot = 0;
	do {
		slot = selectSaveSlotDialogue(72, 14, 2);
		if (slot == 6)
			break;
	} while (_saveSlotIdTemp[slot] == -1);

	_screen->copyRegion(72, 14, 72, 14, 176, 144, 12, 0, Screen::CR_NO_P_CHECK);
	_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);

	return (slot < 6) ? _savegameList[_savegameOffset + slot] : Common::String();
}

bool GUI_EoB::transferFileMenu(Common::String &targetName, Common::String &selection) {
	updateSaveSlotsList(targetName, true);
	_saveSlotsListUpdateNeeded = true;
	selection.clear();

	if (!_savegameListSize)
		return false;

	const ScreenDim *dm = _screen->getScreenDim(11);
	int xo = dm->sx;
	int yo = dm->sy;
	_screen->modifyScreenDim(11, dm->sx + 9, dm->sy + 14, dm->w, dm->h);

	int slot = 0;
	do {
		slot = selectSaveSlotDialogue(72, 14, 4);
		if (slot == 6)
			break;

		if (_saveSlotIdTemp[slot] == -1)
			messageDialogue(11, 65, _vm->guiSettings()->colors.guiColorLightRed);
		else {
			_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);
			selection = _vm->getSavegameFilename(targetName, _saveSlotIdTemp[slot]);
			return true;
		}
	} while (_saveSlotIdTemp[slot] == -1);

	_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);
	return true;
}

void GUI_EoB::createScreenThumbnail(Graphics::Surface &dst) {
	dst.copyFrom(_vm->_thumbNail);
}

void GUI_EoB::simpleMenu_initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int itemOffset) {
	if (menuItemsMask == -1) {
		_menuNumItems = _screen->getScreenDim(19 + menuId)->h;
		_menuCur = _screen->getScreenDim(19 + menuId)->unk8;
		return;
	}

	_menuNumItems = 0;

	for (int i = 0; i < maxItem; i++) {
		if (menuItemsMask & (1 << (i + itemOffset)))
			_menuNumItems++;
	}

	_menuCur = 0;
}

bool GUI_EoB::runSaveMenu(int x, int y) {
	const ScreenDim *dm = _screen->getScreenDim(11);
	int xo = dm->sx;
	int yo = dm->sy;
	bool result = false;

	_screen->modifyScreenDim(11, dm->sx + (x >> 3), dm->sy + y, dm->w, dm->h);

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		updateSaveSlotsList(_vm->_targetName);
		int slot = selectSaveSlotDialogue(x, y, 0);
		if (slot > 5) {
			runLoop = result = false;
		} else if (slot >= 0) {
			bool useSlot = (_saveSlotIdTemp[slot] == -1);
			if (useSlot)
				_saveSlotStringsTemp[slot][0] = 0;
			else
				useSlot = confirmDialogue2(11, 55, 1);

			if (!useSlot)
				continue;

			int fx = (x + 1) << 3;
			int fy = y + slot * 17 + 23;
			Screen::FontId of = _screen->_currentFont;
			_screen->set16bitShadingLevel(4);

			for (int in = -1; in == -1 && !_vm->shouldQuit();) {
				_screen->fillRect(fx - 2, fy, fx + 160, fy + 8, _vm->guiSettings()->colors.fill);
				if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
					TimeDate td;
					_vm->_system->getTimeAndDate(td);					
					Common::strlcpy(_saveSlotStringsTemp[slot], Common::String::format(_vm->_saveNamePatterns[_vm->_currentLevel * 2 + _vm->_currentSub], td.tm_mon + 1, td.tm_mday, td.tm_hour, td.tm_min).c_str(), 25);
					in = strlen(_saveSlotStringsTemp[slot]);
					of = _vm->screen()->setFont(Screen::FID_6_FNT);
					y++;
				} else {
					in = getTextInput(_saveSlotStringsTemp[slot], x + 1, fy, 19, _vm->guiSettings()->colors.guiColorBlue, 0, _vm->guiSettings()->colors.guiColorDarkRed);
				}
				if (in == -1) {
					useSlot = false;
					break;
				}

				if (!strlen(_saveSlotStringsTemp[slot])) {
					messageDialogue(11, 54, _vm->guiSettings()->colors.guiColorLightRed);
					in = -1;
				}
			}

			if (!useSlot) {
				continue;
			}

			_screen->fillRect(fx - 2, fy, fx + 160, fy + 8, _vm->guiSettings()->colors.fill);
			_screen->printShadedText(_saveSlotStringsTemp[slot], (x + 1) << 3, fy, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
			_screen->set16bitShadingLevel(0);
			_screen->setFont(of);
			_screen->updateScreen();

			Graphics::Surface thumb;
			createScreenThumbnail(thumb);
			Common::Error err = _vm->saveGameStateIntern(_savegameOffset + slot, _saveSlotStringsTemp[slot], &thumb);
			thumb.free();

			if (err.getCode() == Common::kNoError)
				result = true;
			else
				messageDialogue(11, 15, _vm->guiSettings()->colors.guiColorLightRed);

			runLoop = false;
		}
	}

	_screen->modifyScreenDim(11, xo, yo, dm->w, dm->h);
	return result;
}

int GUI_EoB::selectSaveSlotDialogue(int x, int y, int id) {
	_saveSlotX = _saveSlotY = 0;
	int col1 = (_vm->_configRenderMode == Common::kRenderCGA) ? 1 : _vm->guiSettings()->colors.guiColorWhite;
	_screen->setCurPage(2);

	_savegameOffset = 0;

	drawMenuButtonBox(0, 0, 176, 144, false, false);
	const char *title = (id < 2) ? _vm->_saveLoadStrings[2 + id] : _vm->_transferStringsScummVM[id - 1];
	_screen->printShadedText(title, 52, 5, col1, 0, _vm->guiSettings()->colors.guiColorBlack);

	_screen->copyRegion(0, 0, x, y, 176, 144, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->fillRect(0, 0, 175, 143, 0, 2);

	_screen->setCurPage(0);
	_screen->updateScreen();

	_saveSlotX = x;
	_saveSlotY = y;
	int lastHighlight = -1;
	int lastOffset = -1;
	int newHighlight = 0;
	int slot = -1;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		int inputFlag = _vm->checkInput(0, false, 0) & 0x8FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
			newHighlight = 6;
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2]) {
			if (++newHighlight > 5) {
				newHighlight = 5;
				if (++_savegameOffset > 984)
					_savegameOffset = 984;
				else
					lastOffset = -1;
			}
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP8]) {
			if (--newHighlight < 0) {
				newHighlight = 0;
				if (--_savegameOffset < 0)
					_savegameOffset = 0;
				else
					lastOffset = -1;
			}
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_PAGEDOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP1]) {
			_savegameOffset += 6;
			if (_savegameOffset > 984)
				_savegameOffset = 984;
			else
				lastOffset = -1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_PAGEUP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP7]) {
			_savegameOffset -= 6;
			if (_savegameOffset < 0)
				_savegameOffset = 0;
			else
				lastOffset = -1;
		} else if (inputFlag == 205) {
			if (++_savegameOffset > 984)
				_savegameOffset = 984;
			else
				lastOffset = -1;
		} else if (inputFlag == 203) {
			if (--_savegameOffset < 0)
				_savegameOffset = 0;
			else
				lastOffset = -1;
		} else {
			slot = getHighlightSlot();
			if (slot != -1) {
				newHighlight = slot;
				if (inputFlag == 199)
					runLoop = false;
			}
		}

		if (lastOffset != _savegameOffset) {
			lastHighlight = -1;
			setupSaveMenuSlots();
			for (int i = 0; i < 7; i++)
				drawSaveSlotButton(i, 1, col1);
			lastOffset = _savegameOffset;
		}

		if (lastHighlight != newHighlight) {
			drawSaveSlotButton(lastHighlight, 0, col1);
			drawSaveSlotButton(newHighlight, 0, _vm->guiSettings()->colors.guiColorLightRed);

			// Display highlighted slot index in the bottom left corner to avoid people getting lost with the 990 save slots
			Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
			int sli = (newHighlight == 6) ? _savegameOffset : (_savegameOffset + newHighlight);
			_screen->set16bitShadingLevel(4);
			_screen->printText(Common::String::format("%03d/989", sli).c_str(), _saveSlotX + 5, _saveSlotY + 135, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);
			_screen->set16bitShadingLevel(0);
			_screen->setFont(of);

			_screen->updateScreen();
			lastHighlight = newHighlight;
		}
	}

	drawSaveSlotButton(newHighlight, 2, _vm->guiSettings()->colors.guiColorLightRed);
	_screen->updateScreen();
	_vm->_system->delayMillis(80);
	drawSaveSlotButton(newHighlight, 1, _vm->guiSettings()->colors.guiColorLightRed);
	_screen->updateScreen();

	return newHighlight;
}

void GUI_EoB::runMemorizePrayMenu(int charIndex, int spellType) {
	if (charIndex == -1)
		return;

	uint8 np[8];
	memset(np, 0, sizeof(np));
	uint32 avltyFlags = 0;
	int li = 0;
	int lv = 0;

	EoBCharacter *c = &_vm->_characters[charIndex];
	int8 wm = c->wisdomCur - 12;
	if (wm < 0)
		wm = 0;

	if (spellType) {
		li = _vm->getCharacterLevelIndex(2, c->cClass);

		if (li == -1) {
			li = _vm->getCharacterLevelIndex(4, c->cClass);

			if (li != -1) {
				lv = c->level[li] - 1;
				if (lv < 0)
					lv = 0;

				for (int i = 0; i < _numPages; i++)
					np[i] = _vm->_numSpellsPal[lv * _numPages + i];

				avltyFlags = _paladinSpellAvltyFlags;
			}

		} else {
			lv = c->level[li] - 1;
			for (int i = 0; i < _numPages; i++) {
				np[i] = _vm->_numSpellsCleric[lv * _numPages + i];
				if (np[i])
					np[i] += _vm->_numSpellsWisAdj[wm * _numPages + i];
			}
			avltyFlags = _clericSpellAvltyFlags;
		}

	} else {
		li = _vm->getCharacterLevelIndex(1, c->cClass);

		if (li == -1) {
			if (_vm->checkInventoryForRings(charIndex, 1)) {
				np[3] <<= 1;
				np[4] <<= 1;
			}

		} else {
			lv = c->level[li] - 1;
			for (int i = 0; i < _numPages; i++)
				np[i] = _vm->_numSpellsMage[lv * _numPages + i];

			avltyFlags = c->mageSpellsAvailableFlags;
		}
	}

	int8 *menuSpellMap = new int8[88];
	memset(menuSpellMap, 0, 88);
	int8 *numAssignedSpellsPerBookPage = new int8[8];
	memset(numAssignedSpellsPerBookPage, 0, 8);
	memset(_numAssignedSpellsOfType, 0, 72);
	int8 *lh = new int8[40];
	memset(lh, 0, 40);

	memcpy(lh, spellType ? _vm->_spellLevelsCleric : _vm->_spellLevelsMage, spellType ? _vm->_spellLevelsClericSize : _vm->_spellLevelsMageSize);
	int8 *charSpellList = spellType ? c->clericSpells : c->mageSpells;

	for (int i = 0; i < 80; i++) {
		int8 s = charSpellList[i];
		if (s == 0 || (_vm->game() == GI_EOB2 && s == 29))
			continue;

		if (s < 0)
			s = -s;
		else
			_numAssignedSpellsOfType[s * 2 - 1]++;

		s--;
		_numAssignedSpellsOfType[s * 2]++;
		numAssignedSpellsPerBookPage[lh[s] - 1]++;
	}

	for (int i = 0; i < 32; i++) {
		if (!(avltyFlags & (1 << i)))
			continue;

		int d = lh[i] - 1;
		if (d < 0)
			continue;

		if (!spellType || (spellType && np[d])) {
			menuSpellMap[d * 11]++;
			menuSpellMap[d * 11 + menuSpellMap[d * 11]] = i + 1;
		}
	}

	Button *buttonList = initMenu(4);

	int lastHighLightText = -1;
	int lastHighLightButton = -1;
	int newHighLightButton = 0;
	int newHighLightText = 0;
	bool updateDesc = true;
	bool updateList = true;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		updateBoxFrameHighLight(charIndex);

		if (newHighLightButton < 0)
			newHighLightButton = 7;
		if (newHighLightButton > 7)
			newHighLightButton = 0;

		Button *b = 0;

		if (lastHighLightButton != newHighLightButton) {
			if (lastHighLightButton >= 0)
				drawMenuButton(_vm->gui_getButton(buttonList, lastHighLightButton + 26), false, false, true);
			drawMenuButton(_vm->gui_getButton(buttonList, newHighLightButton + 26), false, true, true);
			newHighLightText = 0;
			lastHighLightText = -1;
			lastHighLightButton = newHighLightButton;
			updateDesc = updateList = true;
		}

		if (updateList) {
			updateList = false;
			_screen->setCurPage(2);
			for (int ii = 1; ii < 9; ii++)
				memorizePrayMenuPrintString(menuSpellMap[lastHighLightButton * 11 + ii], ii - 1, spellType, false, false);

			_screen->setCurPage(0);
			_screen->copyRegion(0, 50, 0, 50, 176, 72, 2, 0, Screen::CR_NO_P_CHECK);
			lastHighLightText = -1;
		}

		if (updateDesc) {
			updateDesc = false;
			_screen->set16bitShadingLevel(4);
			_screen->printShadedText(Common::String::format(_vm->_menuStringsMgc[1], np[lastHighLightButton] - numAssignedSpellsPerBookPage[lastHighLightButton], np[lastHighLightButton]).c_str(), 8, 38, _vm->guiSettings()->colors.guiColorLightBlue, _vm->guiSettings()->colors.fill, _vm->guiSettings()->colors.guiColorBlack);
			_screen->set16bitShadingLevel(0);
		}

		if (newHighLightText < 0)
			newHighLightText = menuSpellMap[lastHighLightButton * 11] - 1;

		if (menuSpellMap[lastHighLightButton * 11] <= newHighLightText)
			newHighLightText = 0;

		if (newHighLightText != lastHighLightText) {
			memorizePrayMenuPrintString(menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1], lastHighLightText, spellType, true, false);
			memorizePrayMenuPrintString(menuSpellMap[lastHighLightButton * 11 + newHighLightText + 1], newHighLightText, spellType, true, true);
			lastHighLightText = newHighLightText;
		}

		int inputFlag = _vm->checkInput(buttonList, false, 0) & 0x80FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
			inputFlag = 0x801A + ((lastHighLightButton + 1) % _numVisPages);
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT]) {
			inputFlag = lastHighLightButton ? 0x8019 + lastHighLightButton : 0x8019 + _numVisPages;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
			inputFlag = 0x8018;
		} else {
			Common::Point p = _vm->getMousePos();
			if (_vm->posWithinRect(p.x, p.y, 8, 50, 168, 122)) {
				newHighLightText = (p.y - 50) / 9;
				if (menuSpellMap[lastHighLightButton * 11] - 1 < newHighLightText)
					newHighLightText = menuSpellMap[lastHighLightButton * 11] - 1;
			}
		}

		if (inputFlag & 0x8000) {
			b = _vm->gui_getButton(buttonList, inputFlag & 0x7FFF);
			drawMenuButton(b, true, true, true);
			_screen->updateScreen();
			_vm->_system->delayMillis(80);
			drawMenuButton(b, false, false, true);
			_screen->updateScreen();
		}

		if (inputFlag == 0x8019 || inputFlag == _vm->_keyMap[Common::KEYCODE_KP_PLUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_PLUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			if (np[lastHighLightButton] > numAssignedSpellsPerBookPage[lastHighLightButton] && lastHighLightText != -1) {
				_numAssignedSpellsOfType[menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1] * 2 - 2]++;
				numAssignedSpellsPerBookPage[lastHighLightButton]++;
				memorizePrayMenuPrintString(menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1], lastHighLightText, spellType, false, true);
				updateDesc = true;
			}

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP_MINUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_MINUS]) {
			if (np[lastHighLightButton] && _numAssignedSpellsOfType[menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1] * 2 - 2]) {
				_numAssignedSpellsOfType[menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1] * 2 - 2]--;
				numAssignedSpellsPerBookPage[lastHighLightButton]--;
				memorizePrayMenuPrintString(menuSpellMap[lastHighLightButton * 11 + lastHighLightText + 1], lastHighLightText, spellType, false, true);
				updateDesc = true;
			}

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP8]) {
			newHighLightText = lastHighLightText - 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2]) {
			newHighLightText = lastHighLightText + 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_END] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP1]) {
			newHighLightText = menuSpellMap[lastHighLightButton * 11] - 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_HOME] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP7]) {
			newHighLightText = 0;
		} else if (inputFlag == 0x8017) {
			if (numAssignedSpellsPerBookPage[lastHighLightButton]) {
				for (int i = 1; i <= menuSpellMap[lastHighLightButton * 11]; i++) {
					numAssignedSpellsPerBookPage[lastHighLightButton] -= _numAssignedSpellsOfType[menuSpellMap[lastHighLightButton * 11 + i] * 2 - 2];
					_numAssignedSpellsOfType[menuSpellMap[lastHighLightButton * 11 + i] * 2 - 2] = 0;
				}

				updateDesc = updateList = true;
			}

		} else if (inputFlag == 0x8018) {
			_vm->gui_drawAllCharPortraitsWithStats();
			runLoop = false;

		} else if (inputFlag & 0x8000) {
			newHighLightButton = inputFlag - 0x801A;
			if (newHighLightButton == lastHighLightButton)
				drawMenuButton(_vm->gui_getButton(buttonList, inputFlag & 0x7FFF), false, true, true);
		}
	}

	releaseButtons(buttonList);
	updateBoxFrameHighLight(-1);

	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);
	_vm->gui_drawCharPortraitWithStats(charIndex);
	_screen->setFont(of);

	memset(charSpellList, 0, 80);
	if (spellType && _vm->game() == GI_EOB2)
		charSpellList[0] = 29;

	for (int i = 0; i < 32; i++) {
		if (_numAssignedSpellsOfType[i * 2] < _numAssignedSpellsOfType[i * 2 + 1])
			_numAssignedSpellsOfType[i * 2 + 1] = _numAssignedSpellsOfType[i * 2];

		if (_numAssignedSpellsOfType[i * 2 + 1]) {
			_numAssignedSpellsOfType[i * 2]--;
			_numAssignedSpellsOfType[i * 2 + 1]--;

			int pg = lh[i] - 1;
			for (int ii = 0; ii < 10; ii++) {
				if (!charSpellList[pg * 10 + ii]) {
					charSpellList[pg * 10 + ii] = i + 1;
					break;
				}
			}
			i--;

		} else if (_numAssignedSpellsOfType[i * 2]) {
			_numAssignedSpellsOfType[i * 2]--;

			_needRest = true;
			int pg = lh[i] - 1;
			for (int ii = 0; ii < 10; ii++) {
				if (!charSpellList[pg * 10 + ii]) {
					charSpellList[pg * 10 + ii] = -(i + 1);
					break;
				}
			}
			i--;
		}
	}

	delete[] menuSpellMap;
	delete[] numAssignedSpellsPerBookPage;
	delete[] lh;
}

void GUI_EoB::scribeScrollDialogue() {
	int16 *scrollInvSlot = new int16[32];
	int16 *scrollCharacter = new int16[32];
	int16 *menuItems = new int16[6];
	int numScrolls = 0;

	for (int i = 0; i < 32; i++) {
		for (int ii = 0; ii < 6; ii++) {
			scrollInvSlot[i] = _vm->checkInventoryForItem(ii, 34, i + 1) + 1;
			if (scrollInvSlot[i] > 0) {
				numScrolls++;
				scrollCharacter[i] = ii;
				break;
			}
		}
	}

	if (numScrolls) {
		int csel = selectCharacterDialogue(49);
		if (csel != -1) {

			EoBCharacter *c = &_vm->_characters[csel];
			int s = 0;

			for (int i = 0; i < 32 && s < 6; i++) {
				if (!scrollInvSlot[i])
					continue;

				if (c->mageSpellsAvailableFlags & (1 << i))
					scrollInvSlot[i] = 0;
				else
					menuItems[s++] = i + 1;
			}

			if (s) {
				Button *buttonList = 0;
				bool redraw = true;
				int lastHighLight = -1;
				int newHighLight = 0;

				while (s && !_vm->shouldQuit()) {
					if (redraw) {
						s = 0;
						for (int i = 0; i < 32 && s < 6; i++) {
							if (!scrollInvSlot[i])
								continue;
							menuItems[s++] = i + 1;
						}

						if (!s)
							break;

						releaseButtons(buttonList);
						buttonList = initMenu(6);

						for (int i = 0; i < s; i++)
							_screen->printShadedText(_vm->_mageSpellList[menuItems[i]], 8, 9 * i + 50, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

						redraw = false;
						lastHighLight = -1;
						newHighLight = 0;
					}

					if (lastHighLight != newHighLight) {
						if (lastHighLight >= 0)
							_screen->printText(_vm->_mageSpellList[menuItems[lastHighLight]], 8, 9 * lastHighLight + 50, _vm->guiSettings()->colors.guiColorWhite, 0);
						lastHighLight = newHighLight;
						_screen->printText(_vm->_mageSpellList[menuItems[lastHighLight]], 8, 9 * lastHighLight + 50, _vm->guiSettings()->colors.guiColorLightRed, 0);
						_screen->updateScreen();
					}

					int inputFlag = _vm->checkInput(buttonList, false, 0);
					_vm->removeInputTop();

					if (inputFlag == 0) {
						Common::Point p = _vm->getMousePos();
						if (_vm->posWithinRect(p.x, p.y, 8, 50, 176, s * 9 + 49))
							newHighLight = (p.y - 50) / 9;
					} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP2] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
						newHighLight = (newHighLight + 1) % s;
					} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP8] || inputFlag == _vm->_keyMap[Common::KEYCODE_UP]) {
						newHighLight = (newHighLight + s - 1) % s;
					} else if (inputFlag == 0x8023 || inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
						s = 0;
					} else if (inputFlag == 0x8024) {
						newHighLight = (_vm->_mouseY - 50) / 9;
						if (newHighLight >= 0 && newHighLight < s) {
							inputFlag = _vm->_keyMap[Common::KEYCODE_SPACE];
						} else {
							inputFlag = 0;
							newHighLight = lastHighLight;
						}
					}

					if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5])  {
						int t = menuItems[newHighLight] - 1;
						Item scItem = _vm->_characters[scrollCharacter[t]].inventory[scrollInvSlot[t] - 1];
						c->mageSpellsAvailableFlags |= (1 << t);
						_vm->_characters[scrollCharacter[t]].inventory[scrollInvSlot[t] - 1] = 0;
						_vm->gui_drawCharPortraitWithStats(_vm->_characters[scrollCharacter[t]].id);
						scrollInvSlot[t] = 0;
						_vm->_items[scItem].block = -1;
						redraw = true;
						s--;
					}
				}

				releaseButtons(buttonList);

			} else {
				displayTextBox(51);
			}
		}
	} else {
		displayTextBox(50);
	}

	delete[] menuItems;
	delete[] scrollCharacter;
	delete[] scrollInvSlot;
}

bool GUI_EoB::restParty() {
	static const int8 eob1healSpells[] = { 2, 15, 20, 24 };
	static const int8 eob2healSpells[] = { 3, 16, 20, 28 };
	const int8 *spells = _vm->game() == GI_EOB1 ? eob1healSpells : eob2healSpells;

	uint8 crs[6];
	memset(crs, 0, 6);
	int hours = 0;

	if (_vm->_inf->preventRest()) {
		assert(_vm->_menuStringsRest3[0]);
		_vm->restParty_displayWarning(_vm->_menuStringsRest3[0]);
		return true;
	}

	if (_vm->restParty_updateMonsters())
		return true;

	if (_vm->restParty_extraAbortCondition())
		return true;

	drawMenuButtonBox(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, false, false);

	bool poisoned = false;
	for (int i = 0; i < 6; i++) {
		if (!_vm->testCharacter(i, 1))
			continue;
		if (!_vm->testCharacter(i, 0x10))
			poisoned = true;
	}

	if (poisoned) {
		if (!confirmDialogue(59))
			return false;
	}

	int8 *list = 0;
	bool useHealers = false;
	bool res = false;
	bool restLoop = true;
	bool restContinue = false;
	int injured = _vm->restParty_getCharacterWithLowestHp();

	if (injured > 0) {
		for (int i = 0; i < 6; i++) {
			if (!_vm->testCharacter(i, 13))
				continue;
			if (_vm->getCharacterLevelIndex(2, _vm->_characters[i].cClass) == -1 && _vm->getCharacterLevelIndex(4, _vm->_characters[i].cClass) == -1)
				continue;
			if (_vm->checkInventoryForItem(i, 30, -1) == -1)
				continue;
			if (_vm->restParty_checkHealSpells(i)) {
				useHealers = confirmDialogue(40);
				break;
			}
		}
	}

	_screen->setClearScreenDim(7);
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);

	restParty_updateRestTime(hours, true);

	for (int l = 0; !res && restLoop && !_vm->shouldQuit();) {
		l++;

		// Regenerate spells
		for (int i = 0; i < 6; i++) {
			crs[i]++;

			if (!_vm->_characters[i].food)
				continue;
			if (!_vm->testCharacter(i, 5))
				continue;

			if (_vm->checkInventoryForItem(i, 30, -1) != -1) {
				list = _vm->_characters[i].clericSpells;

				for (int ii = 0; ii < 80; ii++) {
					if ((ii / 10 + 48) >= crs[i])
						break;

					if (*list >= 0) {
						list++;
						continue;
					}

					*list *= -1;
					crs[i] = 48;
					_vm->_txt->printMessage(Common::String::format(_vm->_menuStringsRest2[0], _vm->_characters[i].name, _vm->_spells[_vm->_clericSpellOffset + *list].name).c_str());
					_vm->delay(80);
					break;
				}
			}

			if (_vm->checkInventoryForItem(i, 29, -1) != -1) {
				list = _vm->_characters[i].mageSpells;

				for (int ii = 0; ii < 80; ii++) {
					if ((ii / 6 + 48) >= crs[i])
						break;

					if (*list >= 0) {
						list++;
						continue;
					}

					*list *= -1;
					crs[i] = 48;
					_vm->_txt->printMessage(Common::String::format(_vm->_menuStringsRest2[1], _vm->_characters[i].name, _vm->_spells[*list].name).c_str());
					_vm->delay(80);
					break;
				}
			}
		}

		// Heal party members
		if (useHealers) {
			for (int i = 0; i < 6 && injured; i++) {
				if (_vm->getCharacterLevelIndex(2, _vm->_characters[i].cClass) == -1 && _vm->getCharacterLevelIndex(4, _vm->_characters[i].cClass) == -1)
					continue;
				if (_vm->checkInventoryForItem(i, 30, -1) == -1)
					continue;

				list = 0;
				if (crs[i] >= 48) {
					for (int ii = 0; !list && ii < 3; ii++)
						list = (int8 *)memchr(_vm->_characters[i].clericSpells, -spells[ii], 80);
				}

				if (list)
					break;

				list = _vm->_characters[i].clericSpells;
				for (int ii = 0; ii < 80 && injured; ii++) {
					int healHp = 0;
					if (*list == spells[0])
						healHp = _vm->rollDice(1, 8, 0);
					else if (*list == spells[1])
						healHp = _vm->rollDice(2, 8, 1);
					else if (*list == spells[2])
						healHp = _vm->rollDice(3, 8, 3);

					if (!healHp) {
						list++;
						continue;
					}

					*list *= -1;
					list++;

					crs[i] = 0;
					injured--;

					_vm->_txt->printMessage(Common::String::format(_vm->_menuStringsRest2[2], _vm->_characters[i].name, _vm->_characters[injured].name).c_str());
					_vm->delay(80);

					_vm->_characters[injured].hitPointsCur += healHp;
					if (_vm->_characters[injured].hitPointsCur > _vm->_characters[injured].hitPointsMax)
						_vm->_characters[injured].hitPointsCur = _vm->_characters[injured].hitPointsMax;

					_vm->gui_drawCharPortraitWithStats(injured++);
				}
			}
		}

		if (l == 6) {
			l = 0;
			restParty_updateRestTime(++hours, false);
			_vm->_restPartyElapsedTime += (32760 * _vm->_tickLength);

			// Update poisoning
			for (int i = 0; i < 6; i++) {
				if (!_vm->testCharacter(i, 1))
					continue;
				if (_vm->testCharacter(i, 16))
					continue;
				_vm->inflictCharacterDamage(i, 10);
				_vm->delayWithTicks(5);
			}

			if (!(hours % 8)) {
				bool starving = false;
				for (int i = 0; i < 6; i++) {
					// Add Lay On Hands spell
					if (_vm->_characters[i].cClass == 2) {
						list = (int8 *)memchr(_vm->_characters[i].clericSpells, spells[3], 10);
						if (list) {
							*list = spells[3];
						} else {
							list = (int8 *)memchr(_vm->_characters[i].clericSpells, -spells[3], 10);
							if (list) {
								*list = spells[3];
							} else if (!memchr(_vm->_characters[i].clericSpells, spells[3], 10)) {
								list = (int8 *)memchr(_vm->_characters[i].clericSpells, 0, 10);
								*list = spells[3];
							}
						}
					}

					if (!_vm->testCharacter(i, 3))
						continue;

					// Update hitpoints and food status
					if (_vm->_characters[i].food) {
						if (_vm->_characters[i].hitPointsCur < _vm->_characters[i].hitPointsMax) {
							_vm->_characters[i].hitPointsCur++;
							_vm->gui_drawCharPortraitWithStats(i);
						}

						if (!_vm->checkInventoryForRings(i, 2)) {
							if (_vm->_characters[i].food <= 5) {
								_vm->_characters[i].food = 0;
								starving = true;
							} else {
								_vm->_characters[i].food -= 5;
							}
						}
					} else {
						if ((hours % 24) || (_vm->_characters[i].hitPointsCur <= -10))
							continue;
						_vm->inflictCharacterDamage(i, 1);
						starving = true;
						_vm->gui_drawCharPortraitWithStats(i);
					}
				}

				if (starving) {
					if (!confirmDialogue(47)) {
						restContinue = false;
						restLoop = false;
					}
					restParty_updateRestTime(hours, true);
				}
				injured = restLoop ? _vm->restParty_getCharacterWithLowestHp() : 0;
			}
		}

		if (!_vm->restParty_checkSpellsToLearn() && restLoop && !restContinue && injured) {
			restContinue = confirmDialogue(41);
			restParty_updateRestTime(hours, true);
			if (!restContinue)
				restLoop = false;
		}

		int in = _vm->checkInput(0, false, 0);
		_vm->removeInputTop();
		if (in)
			restLoop = false;

		if (restLoop) {
			res = _vm->restParty_updateMonsters();
			if (!res)
				res = _vm->checkPartyStatus(false);
		}

		if (!_vm->restParty_checkSpellsToLearn()) {
			if (!restContinue) {
				if (!useHealers)
					restLoop = false;
			}
			if (!injured)
				restLoop = false;
		}
	}

	_vm->removeInputTop();
	_screen->setScreenDim(4);
	_screen->setFont(of);

	if (!res) {
		if (!injured)
			displayTextBox(43);
		if (hours > 4)
			_vm->restParty_npc();
	}

	return res;
}

bool GUI_EoB::confirmDialogue(int id) {
	int od = _screen->curDimIndex();
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	Button *buttonList = initMenu(5);

	_screen->printShadedText(getMenuString(id), (_screen->_curDim->sx + 1) << 3, _screen->_curDim->sy + 4, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

	int newHighlight = 0;
	int lastHighlight = -1;
	bool result = false;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit();) {
		if (newHighlight != lastHighlight) {
			if (lastHighlight != -1)
				drawMenuButton(_vm->gui_getButton(buttonList, lastHighlight + 33), false, false, true);
			drawMenuButton(_vm->gui_getButton(buttonList, newHighlight + 33), false, true, true);
			_screen->updateScreen();
			lastHighlight = newHighlight;
		}

		int inputFlag = _vm->checkInput(buttonList, false, 0) & 0x80FF;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			result = lastHighlight == 0;
			inputFlag = 0x8021 + lastHighlight;
			runLoop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
			newHighlight ^= 1;
		} else if (inputFlag == 0x8021) {
			result = true;
			runLoop = false;
		} else if (inputFlag == 0x8022) {
			result = false;
			runLoop = false;
		} else {
			Common::Point p = _vm->getMousePos();
			for (Button *b = buttonList; b; b = b->nextButton) {
				if ((b->arg & 2) && _vm->posWithinRect(p.x, p.y, b->x, b->y, b->x + b->width, b->y + b->height))
					newHighlight = b->index - 33;
			}
		}

		if (!runLoop) {
			Button *b = _vm->gui_getButton(buttonList, lastHighlight + 33);
			drawMenuButton(b, true, true, true);
			_screen->updateScreen();
			_vm->_system->delayMillis(80);
			drawMenuButton(b, false, true, true);
			_screen->updateScreen();
		}
	}

	releaseButtons(buttonList);

	_screen->setFont(of);
	_screen->setScreenDim(od);

	return result;
}

int GUI_EoB::selectCharacterDialogue(int id) {
	uint8 flags = (id == 26) ? (_vm->game() == GI_EOB1 ? 0x04 : 0x14) : 0x02;
	_vm->removeInputTop();

	_charSelectRedraw = false;
	bool starvedUnconscious = false;
	int count = 0;
	int result = -1;
	int found[6];

	for (int i = 0; i < 6; i++) {
		found[i] = -1;

		if (!_vm->testCharacter(i, 1))
			continue;

		if (!(_vm->_classModifierFlags[_vm->_characters[i].cClass] & flags) && (id != 53))
			continue;

		if (id != 53 && (!_vm->_characters[i].food || !_vm->testCharacter(i, 4))) {
			starvedUnconscious = true;
		} else {
			found[i] = 0;
			result = i;
			count++;
		}
	}

	if (!count) {
		int eid = 0;
		if (id == 23)
			eid = (starvedUnconscious || _vm->game() == GI_EOB1) ? 28 : 72;
		else if (id == 26)
			eid = (starvedUnconscious || _vm->game() == GI_EOB1) ? 27 : 73;
		else if (id == 49)
			eid = 52;

		displayTextBox(eid);
		return -1;
	}

	static const uint16 selX[] = { 184, 256, 184, 256, 184, 256 };
	static const uint8 selY[] = { 2, 2, 54, 54, 106, 106};

	for (int i = 0; i < 6; i++) {
		if (found[i] != -1 || !_vm->testCharacter(i, 1))
			continue;

		_screen->drawShape(0, _vm->_blackBoxSmallGrid, selX[i], selY[i], 0);
		_screen->drawShape(0, _vm->_blackBoxSmallGrid, selX[i] + 16, selY[i], 0);
		_screen->drawShape(0, _vm->_blackBoxSmallGrid, selX[i] + 32, selY[i], 0);
		_screen->drawShape(0, _vm->_blackBoxSmallGrid, selX[i] + 48, selY[i], 0);
		_charSelectRedraw = true;
	}
	_screen->updateScreen();
	if (count == 1) {
		int l = _vm->getCharacterLevelIndex(4, _vm->_characters[result].cClass);

		if (l == -1)
			return result;

		if (_vm->_characters[result].level[l] > 8)
			return result;

		displayTextBox(24);
		return -1;
	}

	_vm->_menuDefs[3].titleStrId = id;
	Button *buttonList = initMenu(3);

	result = -2;
	int hlCur = -1;
	for (int i = 0; i < 6; ++i) {
		if (found[i] != -1) {
			hlCur = i;
			break;
		}
	}

	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);

	while (result == -2 && !_vm->shouldQuit()) {
		int inputFlag = _vm->checkInput(buttonList, false, 0);
		_vm->removeInputTop();

		updateBoxFrameHighLight(hlCur);

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP8] || inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_a] || inputFlag == _vm->_keyMap[Common::KEYCODE_w]) {
			updateBoxFrameHighLight(-1);
			_vm->gui_drawCharPortraitWithStats(hlCur--);
			if (hlCur < 0)
				hlCur = 5;
			while (found[hlCur]) {
				if (--hlCur < 0)
					hlCur = 5;
			}

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_z] || inputFlag == _vm->_keyMap[Common::KEYCODE_s]) {
			updateBoxFrameHighLight(-1);
			_vm->gui_drawCharPortraitWithStats(hlCur++);
			if (hlCur == 6)
				hlCur = 0;
			while (found[hlCur]) {
				if (++hlCur == 6)
					hlCur = 0;
			}

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			if (hlCur >= 0)
				result = hlCur;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE] || inputFlag == 0x8010) {
			_screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
			drawMenuButton(buttonList, true, true, true);
			_screen->updateScreen();
			_vm->_system->delayMillis(80);
			drawMenuButton(buttonList, false, false, true);
			_screen->updateScreen();
			_screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);
			result = -1;

		} else if (inputFlag > 0x8010 && inputFlag < 0x8017) {
			result = inputFlag - 0x8011;
			if (found[result])
				result = -2;
		}
	}

	updateBoxFrameHighLight(-1);
	if (hlCur >= 0)
		_vm->gui_drawCharPortraitWithStats(hlCur);

	_screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	if (result != -1 && id != 53) {
		if (flags & 4) {
			int lv = _vm->getCharacterLevelIndex(4, _vm->_characters[result].cClass);
			if (lv != -1) {
				if (_vm->_characters[result].level[lv] < 9) {
					displayTextBox(24);
					result = -1;
				}
			}
		} else {
			if (_vm->checkInventoryForItem(result, 29, -1) == -1) {
				displayTextBox(25);
				result = -1;
			}
		}
	}

	releaseButtons(buttonList);
	_screen->setFont(of);

	return result;
}

void GUI_EoB::displayTextBox(int id) {
	int op = _screen->setCurPage(2);
	int od = _screen->curDimIndex();
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
	_screen->setClearScreenDim(11);
	const ScreenDim *dm = _screen->getScreenDim(11);

	drawMenuButtonBox(dm->sx << 3, dm->sy, dm->w << 3, dm->h, false, false);
	_screen->setTextMarginRight((dm->sx + dm->w) << 3);
	_screen->printShadedText(getMenuString(id), (dm->sx << 3) + 5, dm->sy + 5, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->setTextMarginRight(Screen::SCREEN_W);
	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	for (uint32 timeOut = _vm->_system->getMillis() + 1440; _vm->_system->getMillis() < timeOut && !_vm->shouldQuit();) {
		int in = _vm->checkInput(0, false, 0);
		_vm->removeInputTop();
		if (in && !(in & 0x800))
			break;
		_vm->_system->delayMillis(4);
	}

	_screen->setCurPage(op);
	_screen->setFont(of);
	_screen->setScreenDim(od);
}

Button *GUI_EoB::initMenu(int id) {
	_screen->setCurPage(2);

	EoBMenuDef *m = &_vm->_menuDefs[id];

	if (m->dim) {
		const ScreenDim *dm = _screen->getScreenDim(m->dim);
		_screen->fillRect(dm->sx << 3, dm->sy, ((dm->sx + dm->w) << 3) - 1, (dm->sy + dm->h) - 1, _vm->guiSettings()->colors.fill);
		_screen->setScreenDim(m->dim);
		_screen->setTextMarginRight((dm->sx + dm->w) << 3);
		drawMenuButtonBox(dm->sx << 3, dm->sy, dm->w << 3, dm->h, false, false);
	}

	_screen->printShadedText(getMenuString(m->titleStrId), 5, 5, m->titleCol, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->setTextMarginRight(Screen::SCREEN_W);

	Button *buttons = 0;
	for (int i = 0; i < m->numButtons; i++) {
		const EoBMenuButtonDef *df = &_vm->_menuButtonDefs[m->firstButtonStrId + i];
		Button *b = new Button;
		b->index = m->firstButtonStrId + i + 1;
		if (id == 4 && _vm->game() == GI_EOB1)
			b->index -= 14;
		else if (id == 2)
			b->index -= _vm->_prefMenuPlatformOffset;

		b->data0Val2 = 12;
		b->data1Val2 = b->data2Val2 = 15;
		b->data3Val2 = 8;
		b->flags = 0x1100;
		b->keyCode = df->keyCode;
		b->keyCode2 = df->keyCode | 0x100;
		b->x = df->x;
		b->y = df->y;
		b->width = df->width;
		b->height = df->height;
		b->extButtonDef = df;
		b->arg = df->flags;

		drawMenuButton(b, false, false, false);
		buttons = linkButton(buttons, b);
	}

	_screen->copyRegion(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, 2, 0, Screen::CR_NO_P_CHECK);
	_vm->gui_notifyButtonListChanged();
	_screen->setCurPage(0);
	_screen->updateScreen();

	return buttons;
}

void GUI_EoB::drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill) {
	if (!b)
		return;

	const EoBMenuButtonDef *d = (const EoBMenuButtonDef *)b->extButtonDef;

	if (d->flags & 1)
		drawMenuButtonBox(b->x, b->y, b->width, b->height, clicked, noFill);

	if (d->labelId) {
		const char *s = getMenuString(d->labelId);

		int xOffs = 4;
		int yOffs = 3;

		if (d->flags & 4) {
			xOffs = ((b->width - _screen->getTextWidth(s)) >> 1) + 1;
			yOffs = (b->height - 7) >> 1;
		}

		int col1 = (_vm->_configRenderMode == Common::kRenderCGA) ? 1 : _vm->guiSettings()->colors.guiColorWhite;

		if (noFill || clicked)
			_screen->printText(s, b->x + xOffs, b->y + yOffs, highlight ? _vm->guiSettings()->colors.guiColorLightRed : col1, 0);
		else
			_screen->printShadedText(s, b->x + xOffs, b->y + yOffs, highlight ? _vm->guiSettings()->colors.guiColorLightRed : col1, 0, _vm->guiSettings()->colors.guiColorBlack);
	}
}

void GUI_EoB::drawMenuButtonBox(int x, int y, int w, int h, bool clicked, bool noFill) {
	uint8 col1 = _vm->guiSettings()->colors.frame1;
	uint8 col2 = _vm->guiSettings()->colors.frame2;

	if (clicked)
		col1 = col2 = _vm->guiSettings()->colors.fill;

	_screen->set16bitShadingLevel(4);
	_vm->gui_drawBox(x, y, w, h, col1, col2, -1);
	_vm->gui_drawBox(x + 1, y + 1, w - 2, h - 2, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, noFill ? -1 : _vm->guiSettings()->colors.fill);
	_screen->set16bitShadingLevel(0);
}

void GUI_EoB::drawTextBox(int dim, int id) {
	int od = _screen->curDimIndex();
	_screen->setScreenDim(dim);
	const ScreenDim *dm = _screen->getScreenDim(dim);
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	if (dm->w <= 22 && dm->h <= 84)
		_screen->copyRegion(dm->sx << 3, dm->sy, 0, dm->h, dm->w << 3, dm->h, 0, 2, Screen::CR_NO_P_CHECK);

	_screen->setCurPage(2);

	drawMenuButtonBox(0, 0, dm->w << 3, dm->h, false, false);
	_screen->printShadedText(getMenuString(id), 5, 5, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

	_screen->setCurPage(0);
	_screen->copyRegion(0, 0, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_screen->setScreenDim(od);
	_screen->setFont(of);
}

void GUI_EoB::drawSaveSlotButton(int slot, int redrawBox, int textCol) {
	if (slot < 0)
		return;

	int x = _saveSlotX + 4;
	int y = _saveSlotY + slot * 17 + 20;
	int w = 167;
	char slotString[26];
	Common::strlcpy(slotString, slot < 6 ? _saveSlotStringsTemp[slot] : _vm->_saveLoadStrings[0], _vm->gameFlags().platform == Common::kPlatformFMTowns ? 25 : 20);
	
	if (slot >= 6) {
		x = _saveSlotX + 118;
		y = _saveSlotY + 126;
		w = 53;
	}

	if (redrawBox)
		drawMenuButtonBox(x, y, w, 14, (redrawBox - 1) ? true : false, false);

	Screen::FontId fnt = _screen->_currentFont;
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		fnt = _vm->screen()->setFont(Screen::FID_6_FNT);
		y++;
	}

	_screen->printShadedText(slotString, x + 4, y + 3, textCol, 0, _vm->guiSettings()->colors.guiColorBlack);
	_vm->screen()->setFont(fnt);
}

void GUI_EoB::memorizePrayMenuPrintString(int spellId, int bookPageIndex, int spellType, bool noFill, bool highLight) {
	if (bookPageIndex < 0)
		return;

	int y = bookPageIndex * 9 + 50;
	int col1 = (_vm->_configRenderMode == Common::kRenderCGA) ? 1 : _vm->guiSettings()->colors.guiColorWhite;
	_screen->set16bitShadingLevel(4);

	if (spellId) {
		Common::String s;
		if (_vm->_flags.lang == Common::JA_JPN) {
			s = spellType ? _vm->_clericSpellList[spellId] : _vm->_mageSpellList[spellId];
			for (int i = s.size() >> 1; i < 17; ++i)
				s.insertChar(' ', s.size());
			s.insertChar((char)(_numAssignedSpellsOfType[spellId * 2 - 2] + 48), s.size());
		} else {
			s = Common::String::format(_vm->_menuStringsMgc[0], spellType ? _vm->_clericSpellList[spellId] : _vm->_mageSpellList[spellId], _numAssignedSpellsOfType[spellId * 2 - 2]);
		}

		if (noFill)
			_screen->printText(s.c_str(), 8, y, highLight ? _vm->guiSettings()->colors.guiColorLightRed : col1, 0);
		else
			_screen->printShadedText(s.c_str(), 8, y, highLight ? _vm->guiSettings()->colors.guiColorLightRed : col1, _vm->guiSettings()->colors.fill, _vm->guiSettings()->colors.guiColorBlack);
	} else {
		_screen->fillRect(6, y, 168, y + 8,  _vm->guiSettings()->colors.fill);
	}

	_screen->set16bitShadingLevel(0);
}

void GUI_EoB::updateOptionsStrings() {
	for (int i = 0; i < 4; i++) {
		delete[] _menuStringsPrefsTemp[i];
		_menuStringsPrefsTemp[i] = new char[strlen(_vm->_menuStringsPrefs[i]) + 8];
	}

	Common::strlcpy(_menuStringsPrefsTemp[0], Common::String::format(_vm->_menuStringsPrefs[0], _vm->_menuStringsOnOff[_vm->_configMusic ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[0]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[1], Common::String::format(_vm->_menuStringsPrefs[1], _vm->_menuStringsOnOff[_vm->_configSounds ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[1]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[2], Common::String::format(_vm->_menuStringsPrefs[2], _vm->_menuStringsOnOff[_vm->_configHpBarGraphs ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[2]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[3], Common::String::format(_vm->_menuStringsPrefs[3], _vm->gameFlags().platform == Common::kPlatformFMTowns ?	_vm->_2431Strings[_vm->_config2431 ? 0 : 1] : _vm->_menuStringsOnOff[_vm->_configMouse ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[3]) + 8);
}

const char *GUI_EoB::getMenuString(int id) {
	static const char empty[] = "";

	if (id >= 69)
		return _vm->_menuStringsTransfer[id - 69];
	else  if (id == 68)
		return _vm->_transferStringsScummVM[0];
	else if (id == 67)
		return _vm->_menuStringsDefeat[0];
	else if (id == 66)
		return _vm->_errorSlotEmptyString;
	else if (id == 65)
		return _vm->_errorSlotEmptyString;
	else if (id >= 63)
		return _vm->_menuStringsSpec[id - 63];
	else if (id >= 60)
		return _vm->_menuStringsSpellNo[id - 60];
	else if (id == 59)
		return _vm->_menuStringsPoison[0];
	else if (id >= 56)
		return _vm->_menuStringsHead[id - 56];
	else if (id == 55)
		return _vm->_menuStringsDrop2[_vm->game() == GI_EOB1 ? 1 : 2];
	else if (id == 54)
		return _vm->_errorSlotNoNameString;
	else if (id == 53)
		return _vm->_menuStringsDrop2[0];
	else if (id >= 48)
		return _vm->_menuStringsScribe[id - 48];
	else if (id == 47)
		return _vm->_menuStringsStarve[0];
	else if (id == 46)
		return _vm->_menuStringsExit[0];
	else if (id == 45)
		return _vm->_menuStringsDrop[0];
	else if (id >= 40)
		return _vm->_menuStringsRest[id - 40];
	else if (id >= 23)
		return _vm->_menuStringsSpells[id - 23];
	else if (id >= 21)
		return _vm->_menuStringsOnOff[id - 21];
	else if (id >= 17)
		return _menuStringsPrefsTemp[id - 17];
	else if (id >= 9)
		return _vm->_menuStringsSaveLoad[id - 9];
	else if (id >= 1)
		return _vm->_menuStringsMain[id - 1];
	else if (id < 0)
		return _vm->_transferStringsScummVM[-id];
	return empty;
}

Button *GUI_EoB::linkButton(Button *list, Button *newbt) {
	if (!list) {
		list = newbt;
		return list;
	}

	if (!newbt)
		return list;

	Button *resList = list;
	while (list->nextButton)
		list = list->nextButton;
	list->nextButton = newbt;
	newbt->nextButton = 0;

	return resList;
}

void GUI_EoB::releaseButtons(Button *list) {
	while (list) {
		Button *n = list->nextButton;
		delete list;
		list = n;
	}
	_vm->gui_notifyButtonListChanged();
}

void GUI_EoB::setupSaveMenuSlots() {
	for (int i = 0; i < 6; ++i) {
		if (_savegameOffset + i < _savegameListSize) {
			if (_savegameList[i + _savegameOffset]) {
				Common::strlcpy(_saveSlotStringsTemp[i], _savegameList[i + _savegameOffset], 25);
				_saveSlotIdTemp[i] = i + _savegameOffset;
				continue;
			}
		}
		Common::strlcpy(_saveSlotStringsTemp[i], _vm->_saveLoadStrings[1], 25);
		_saveSlotIdTemp[i] = -1;
	}
}

int GUI_EoB::getHighlightSlot() {
	int res = -1;
	Common::Point p = _vm->getMousePos();

	for (int i = 0; i < 6; i++) {
		int y = _saveSlotY + i * 17 + 20;
		if (_vm->posWithinRect(p.x, p.y, _saveSlotX + 4, y, _saveSlotX + 167, y + 14)) {
			res = i;
			break;
		}
	}

	if (_vm->posWithinRect(p.x, p.y, _saveSlotX + 118, _saveSlotY + 126, _saveSlotX + 171, _saveSlotY + 140))
		res = 6;

	return res;
}

void GUI_EoB::sortSaveSlots() {
	Common::sort(_saveSlots.begin(), _saveSlots.end(), Common::Less<int>());
}

void GUI_EoB::restParty_updateRestTime(int hours, bool init) {
	Screen::FontId of = _screen->setFont(_vm->_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
	int od = _screen->curDimIndex();
	_screen->setScreenDim(10);

	if (init) {
		_screen->setCurPage(0);
		_vm->_txt->clearCurDim();
		drawMenuButtonBox(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, false, false);
		_screen->copyRegion(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, 0, 2, Screen::CR_NO_P_CHECK);
		_screen->printShadedText(getMenuString(42), (_screen->_curDim->sx + 1) << 3, _screen->_curDim->sy + 5, _vm->guiSettings()->colors.guiColorLightBlue, 0, _vm->guiSettings()->colors.guiColorBlack);
	}

	_screen->setCurPage(0);
	_screen->set16bitShadingLevel(4);
	_screen->fillRect((_screen->_curDim->sx + 1) << 3, _screen->_curDim->sy + 20, ((_screen->_curDim->sx + 19) << 3) + 1, _screen->_curDim->sy + 29, _vm->guiSettings()->colors.fill);
	_screen->printShadedText(Common::String::format(_vm->_menuStringsRest2[3], hours).c_str(), (_screen->_curDim->sx + 1) << 3, _screen->_curDim->sy + 20, _vm->guiSettings()->colors.guiColorWhite, _vm->guiSettings()->colors.fill, _vm->guiSettings()->colors.guiColorBlack);
	_screen->set16bitShadingLevel(0);
	_screen->updateScreen();
	_vm->delay(160);
	_screen->setScreenDim(od);
	_screen->setFont(of);
}

const EoBRect16 GUI_EoB::_highlightFrames[] = {
	{ 0x00B7, 0x0001, 0x00F7, 0x0034 },
	{ 0x00FF, 0x0001, 0x013F, 0x0034 },
	{ 0x00B7, 0x0035, 0x00F7, 0x0068 },
	{ 0x00FF, 0x0035, 0x013F, 0x0068 },
	{ 0x00B7, 0x0069, 0x00F7, 0x009C },
	{ 0x00FF, 0x0069, 0x013F, 0x009C },
	{ 0x0010, 0x003F, 0x0030, 0x0060 },
	{ 0x0050, 0x003F, 0x0070, 0x0060 },
	{ 0x0010, 0x007F, 0x0030, 0x00A0 },
	{ 0x0050, 0x007F, 0x0070, 0x00A0 },
	{ 0x00B0, 0x0042, 0x00D0, 0x0061 },
	{ 0x00D0, 0x0042, 0x00F0, 0x0061 },
	{ 0x00F0, 0x0042, 0x0110, 0x0061 },
	{ 0x0110, 0x0042, 0x0130, 0x0061 },
	{ 0x0004, 0x0018, 0x0024, 0x0039 },
	{ 0x00A3, 0x0018, 0x00C3, 0x0039 },
	{ 0x0004, 0x0040, 0x0024, 0x0061 },
	{ 0x00A3, 0x0040, 0x00C3, 0x0061 },
	{ 0x0004, 0x0068, 0x0024, 0x0089 },
	{ 0x00A3, 0x0068, 0x00C3, 0x0089 }
};

const uint8 GUI_EoB::_highlightColorTableVGA[] = { 0x0F, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0x0C, 0xBC, 0xBA, 0xB8, 0xB6, 0xB4, 0xB2, 0xB0, 0x00 };

const uint8 GUI_EoB::_highlightColorTableEGA[] = { 0x0C, 0x0D, 0x0E, 0x0F, 0x0E, 0x0D, 0x00 };

const uint8 GUI_EoB::_highlightColorTableAmiga[] = { 0x13, 0x0B, 0x12, 0x0A, 0x11, 0x09, 0x11, 0x0A, 0x12, 0x0B, 0x00 };

const uint8 GUI_EoB::_highlightColorTablePC98[] = { 0x0C, 0x0D, 0x0E, 0x0F, 0x0E, 0x0D, 0x00 };

} // End of namespace Kyra

#endif // ENABLE_EOB
