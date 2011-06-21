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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/eobcommon.h"
#include "kyra/gui_eob.h"
#include "kyra/text_eob.h"
#include "kyra/timer.h"
#include "kyra/util.h"

#include "common/system.h"

namespace Kyra {

void LolEobBaseEngine::removeInputTop() {
	if (!_eventList.empty()) {
		if (_eventList.begin()->event.type == Common::EVENT_LBUTTONDOWN)
			_mouseClick = 1;
		else if (_eventList.begin()->event.type == Common::EVENT_RBUTTONDOWN)
			_mouseClick = 2;
		else
			_mouseClick = 0;

		_eventList.erase(_eventList.begin());
	}
}

void LolEobBaseEngine::gui_drawBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor) {
	w--; h--;
	if (fillColor != -1)
		screen()->fillRect(x + 1, y + 1, x + w - 1, y + h - 1, fillColor);

	screen()->drawClippedLine(x + 1, y, x + w, y, frameColor2);
	screen()->drawClippedLine(x + w, y, x + w, y + h - 1, frameColor2);
	screen()->drawClippedLine(x, y, x, y + h, frameColor1);
	screen()->drawClippedLine(x, y + h, x + w, y + h, frameColor1);
}

void LolEobBaseEngine::gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 cur, int32 max, int col1, int col2) {
	if (max < 1)
		return;
	if (cur < 0)
		cur = 0;

	int32 e = MIN(cur, max);

	if (!--w)
		return;
	if (!--h)
		return;

	int32 t = (e * w) / max;

	if (!t && e)
		t++;

	if (t)
		screen()->fillRect(x, y, x + t - 1, y + h, col1);

	if (t < w && col2)
		screen()->fillRect(x + t, y, x + w, y + h, col2);
}

void LolEobBaseEngine::gui_initButtonsFromList(const int16 *list) {
	while (*list != -1)
		gui_initButton(*list++);
}

void LolEobBaseEngine::gui_resetButtonList() {
	for (uint i = 0; i < ARRAYSIZE(_activeButtonData); ++i)
		_activeButtonData[i].nextButton = 0;

	gui_notifyButtonListChanged();
	_activeButtons = 0;
}

void LolEobBaseEngine::gui_notifyButtonListChanged() {
	if (gui()) {
		if (!_buttonListChanged && !_preserveEvents)
			removeInputTop();
		_buttonListChanged = true;
	}
}

bool LolEobBaseEngine::clickedShape(int shapeIndex) {
	if (_clickedSpecialFlag != 0x40)
		return true;

	for (; shapeIndex; shapeIndex = _levelDecorationProperties[shapeIndex].next) {
		if (_flags.gameID != GI_LOL)
			shapeIndex--;

		uint16 s = _levelDecorationProperties[shapeIndex].shapeIndex[1];

		if (s == 0xffff)
			continue;

		int w = _flags.gameID == GI_LOL ? _levelDecorationShapes[s][3] : (_levelDecorationShapes[s][2] << 3);
		int h = _levelDecorationShapes[s][_flags.gameID == GI_LOL ? 2 : 1];
		int x = _levelDecorationProperties[shapeIndex].shapeX[1] + _clickedShapeXOffs;
		int y = _levelDecorationProperties[shapeIndex].shapeY[1] + _clickedShapeYOffs;

		if (_levelDecorationProperties[shapeIndex].flags & 1) {
			if (_flags.gameID == GI_LOL)
				w <<= 1;
			else
				x = 176 - x - w;
		}

		if (posWithinRect(_mouseX, _mouseY, x - 4, y - 4, x + w + 8, y + h + 8))
			return true;
	}

	return false;
}

#ifdef ENABLE_EOB

Button *EobCoreEngine::gui_getButton(Button *buttonList, int index) {
	while (buttonList) {
		if (buttonList->index == index)
			return buttonList;
		buttonList = buttonList->nextButton;
	}

	return 0;
}

void EobCoreEngine::gui_drawPlayField(int pageNum) {
	_screen->loadEobCpsFileToPage("PLAYFLD", 0, 5, 3, 2);
	int cp = _screen->setCurPage(2);
	gui_drawCompass(true);

	if (pageNum && !_sceneDrawPage1)
		drawScene(0);

	_screen->setCurPage(cp);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	_screen->loadEobCpsFileToPage("INVENT", 0, 5, 3, 2);
}

void EobCoreEngine::gui_restorePlayField() {
	loadVcnData(0, 0);
	_screen->_curPage = 0;
	gui_drawPlayField(1);
	gui_drawAllCharPortraitsWithStats();
}

void EobCoreEngine::gui_drawAllCharPortraitsWithStats() {
	for (int i = 0; i < 6; i++)
		gui_drawCharPortraitWithStats(i);
}

void EobCoreEngine::gui_drawCharPortraitWithStats(int index) {
	if (!testCharacter(index, 1))
		return;

	static const uint16 charPortraitPosX[] = { 8, 80, 184, 256 };
	static const uint16 charPortraitPosY[] = { 2, 54, 106 };

	EobCharacter *c = &_characters[index];
	int txtCol1 = 12;
	int txtCol2 = 15;

	if ((_flags.gameID == GI_EOB1 && c->flags & 6) || (_flags.gameID == GI_EOB2 && c->flags & 0x0e)) {
		txtCol1 = 8;
		txtCol2 = 6;
	}

	if (_currentControlMode == 0) {
		int x2 = charPortraitPosX[index & 1];
		int y2 = charPortraitPosY[index >> 1];
		Screen::FontId cf = _screen->setFont(Screen::FID_6_FNT);

		_screen->copyRegion(176, 168, x2 , y2, 64, 24, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(240, 168, x2, y2 + 24, 64, 26, 2, 2, Screen::CR_NO_P_CHECK);
		int cp = _screen->setCurPage(2);

		if (index == _exchangeCharacterId)
			_screen->printText(_characterGuiStringsSt[0], x2 + 2, y2 + 2, 8, _bkgColor_1);
		else
			_screen->printText(c->name, x2 + 2, y2 + 2, txtCol1, _bkgColor_1);

		gui_drawFaceShape(index);
		gui_drawWeaponSlot(index, 0);
		gui_drawWeaponSlot(index, 1);
		gui_drawHitpoints(index);

		if (testCharacter(index, 2))
			gui_drawCharPortraitStatusFrame(index);

		if (c->damageTaken > 0) {
			_screen->drawShape(2, _redSplatShape, x2 + 13, y2 + 30, 0);
			Common::String tmpStr = Common::String::format("%d", c->damageTaken);
			_screen->printText(tmpStr.c_str(), x2 + 34 - tmpStr.size() * 3, y2 + 42, 15, 0);
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
		_screen->printShadedText(c->name, 219, 6, txtCol2, _bkgColor_1);
		gui_drawHitpoints(index);
		gui_drawFoodStatusGraph(index);

		if (_currentControlMode == 1) {
			if (c->hitPointsCur == -10)
				_screen->printShadedText(_characterGuiStringsSt[1], 247, 158, 6, _color6);
			else if (c->hitPointsCur < 1)
				_screen->printShadedText(_characterGuiStringsSt[2], 226, 158, 6, _color6);
			else if (c->effectFlags & (_flags.gameID == GI_EOB1 ? 0x80 : 0x2000))
				_screen->printShadedText(_characterGuiStringsSt[3], 220, 158, 6, _color6);
			else if (c->flags & 2)
				_screen->printShadedText(_characterGuiStringsSt[4], 235, 158, 6, _color6);
			else if (c->flags & 4)
				_screen->printShadedText(_characterGuiStringsSt[5], 232, 158, 6, _color6);
			else if (c->flags & 8)
				_screen->printShadedText(_characterGuiStringsSt[6], 232, 158, 6, _color6);

			for (int i = 0; i < 27; i++)
				gui_drawInventoryItem(i, 0, 2);
			gui_drawInventoryItem(16, 1, 2);

		} else {
			static const uint16 cm2X1[] = { 179, 272, 301 };
			static const uint16 cm2Y1[] = { 36, 51, 51 };
			static const uint16 cm2X2[] = { 271, 300, 318 };
			static const uint16 cm2Y2[] = { 165, 165, 147 };

			for (int i = 0; i < 3; i++)
				_screen->fillRect(cm2X1[i], cm2Y1[i], cm2X2[i], cm2Y2[i], _color6);

			_screen->printShadedText(_characterGuiStringsIn[0], 183, 42, 15, _color6);
			_screen->printText(_chargenClassStrings[c->cClass], 183, 55, 12, _color6);
			_screen->printText(_chargenAlignmentStrings[c->alignment], 183, 62, 12, _color6);
			_screen->printText(_chargenRaceSexStrings[c->raceSex], 183, 69, 12, _color6);

			for (int i = 0; i < 6; i++)
				_screen->printText(_chargenStatStrings[6 + i], 183, 82 + i * 7, 12, _color6);

			_screen->printText(_characterGuiStringsIn[1], 183, 124, 12, _color6);
			_screen->printText(_characterGuiStringsIn[2], 239, 138, 12, _color6);
			_screen->printText(_characterGuiStringsIn[3], 278, 138, 12, _color6);

			_screen->printText(getCharStrength(c->strengthCur, c->strengthExtCur).c_str(), 275, 82, 15, _color6);
			_screen->printText(Common::String::format("%d", c->intelligenceCur).c_str(), 275, 89, 15, _color6);
			_screen->printText(Common::String::format("%d", c->wisdomCur).c_str(), 275, 96, 15, _color6);
			_screen->printText(Common::String::format("%d", c->dexterityCur).c_str(), 275, 103, 15, _color6);
			_screen->printText(Common::String::format("%d", c->constitutionCur).c_str(), 275, 110, 15, _color6);
			_screen->printText(Common::String::format("%d", c->charismaCur).c_str(), 275, 117, 15, _color6);
			_screen->printText(Common::String::format("%d", c->armorClass).c_str(), 275, 124, 15, _color6);

			for (int i = 0; i < 3; i++) {
				int t = getClassHpIncreaseType(c->cClass, i);
				if (t == -1)
					continue;

				_screen->printText(_chargenClassStrings[t + 15], 180, 145 + 7 * i, 12, _color6);
				Common::String tmpStr = Common::String::format("%d", c->experience[i]);
				_screen->printText(tmpStr.c_str(), 251 - tmpStr.size() * 3, 145 + 7 * i, 15, _color6);
				tmpStr = Common::String::format("%d", c->level[i]);
				_screen->printText(tmpStr.c_str(), 286 - tmpStr.size() * 3, 145 + 7 * i, 15, _color6);
			}
		}

		_screen->_curPage = 0;
		_screen->copyRegion(176, 0, 176, 0, 144, 168, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, 0, 176, 0, 144, 168, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void EobCoreEngine::gui_drawFaceShape(int index) {
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

	EobCharacter *c = &_characters[index];

	if (c->hitPointsCur == -10) {
		_screen->drawShape(_screen->_curPage, _deadCharShape, x, y, 0);
		return;
	}

	if (_flags.gameID == GI_EOB1) {
		if (c->effectFlags & 4) {
			_screen->fillRect(x, y, x + 31, y + 31, 12);
			return;
		}
	} else {
		if (c->effectFlags & 0x140) {
			_screen->setFadeTableIndex(1);
			_screen->setShapeFadeMode(1, true);
		}

		if (c->flags & 2) {
			_screen->setFadeTableIndex(0);
			_screen->setShapeFadeMode(1, true);
		}

		if (c->flags & 8) {
			_screen->setFadeTableIndex(2);
			_screen->setShapeFadeMode(1, true);
		}
	}

	_screen->drawShape(_screen->_curPage, c->faceShape, x, y, 0);

	if (c->hitPointsCur < 1)
		_screen->drawShape(_screen->_curPage, _disabledCharGrid, x, y, 0);

	//if ((c->flags & 2) || (c->flags & 8) || (c->effectFlags & 0x140)) {
		_screen->setFadeTableIndex(4);
		_screen->setShapeFadeMode(1, false);
	//}
}

void EobCoreEngine::gui_drawWeaponSlot(int charIndex, int slot) {
	static const uint8 xCoords[] = { 40, 112 };
	static const uint8 yCoords[] = { 11, 27, 63, 79, 115, 131 };

	int x = xCoords[charIndex & 1];
	int y = yCoords[(charIndex & 6) + slot];

	if (!_screen->_curPage)
		x += 176;

	int itm = _characters[charIndex].inventory[slot];
	gui_drawBox(x, y, 31, 16, _color1_1, _color2_1, _bkgColor_1);

	if (_characters[charIndex].slotStatus[slot]) {
		gui_drawWeaponSlotStatus(x, y, _characters[charIndex].slotStatus[slot]);
		return;
	}

	if (itm)
		drawItemIconShape(_screen->_curPage, itm, x + 8, y);
	else if (!slot && checkScriptFlag(0x8000))
		_screen->drawShape(_screen->_curPage, _itemIconShapes[103], x + 8, y, 0);
	else
		_screen->drawShape(_screen->_curPage, _itemIconShapes[85], x + 8, y, 0);

	if ((_characters[charIndex].disabledSlots & (1 << slot)) || !validateWeaponSlotItem(charIndex, slot) || (_characters[charIndex].hitPointsCur <= 0) || (_characters[charIndex].flags & 0x0c))
		_screen->drawShape(_screen->_curPage, _weaponSlotGrid, x, y, 0);
}

void EobCoreEngine::gui_drawWeaponSlotStatus(int x, int y, int status) {
	Common::String tmpStr;
	Common::String tmpStr2;
	
	if (status > -3 || status == -5)
		_screen->drawShape(_screen->_curPage, _greenSplatShape, x - 1, y, 0);
	else
		gui_drawBox(x, y, 31, 16, _color9, _color10, _color11);

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

	if (!tmpStr2.empty()) {
		_screen->printText(tmpStr.c_str(), x + (16 - tmpStr.size() * 3), y + 2, 15, 0);
		_screen->printText(tmpStr2.c_str(), x + (16 - tmpStr.size() * 3), y + 9, 15, 0);
	} else {
		_screen->printText(tmpStr.c_str(), x + (16 - tmpStr.size() * 3), y + 5, 15, 0);
	}
}

void EobCoreEngine::gui_drawHitpoints(int index) {
	if (!testCharacter(index, 1))
		return;

	if (_currentControlMode && (index != _updateCharNum))
		return;

	static const uint8 xCoords[] = { 23, 95 };
	static const uint8 yCoords[] = { 46, 98, 150 };
	static const uint8 barColor[] = { 3, 5, 8 };

	int x = xCoords[index & 1];
	int y = yCoords[index >> 1];
	int w = 38;
	int h = 3;

	if (!_screen->_curPage)
		x += 176;

	if (_currentControlMode) {
		x = 250;
		y = 16;
		w = 51;
		h = 5;
	}

	EobCharacter *c = &_characters[index];

	if (_configHpBarGraphs) {
		int bgCur = c->hitPointsCur + 10;
		int bgMax = c->hitPointsMax + 10;
		int col = ((bgMax / 3) > bgCur) ? 1 : 0;
		if (bgCur <= 10)
			col = 2;

		if (!_currentControlMode)
			_screen->printText(_characterGuiStringsHp[0], x - 13, y - 1, 12, 0);


		gui_drawHorizontalBarGraph(x, y, w, h, bgCur, bgMax, barColor[col], _color5);

	} else {
		Common::String tmpString = Common::String::format(_characterGuiStringsHp[1], c->hitPointsCur, c->hitPointsMax);

		if (!_currentControlMode) {
			x -= 13;
			y -= 1;
		}

		_screen->printText(tmpString.c_str(), x, y, 12, _bkgColor_1);
	}
}

void EobCoreEngine::gui_drawFoodStatusGraph(int index) {
	if (!_currentControlMode)
		return;

	if (!testCharacter(index, 1))
		return;

	EobCharacter *c = &_characters[index];
	if (!(c->flags & 1))
		return;

	if (index != _updateCharNum)
		return;

	uint8 col = c->food < 20 ? 8 : (c->food < 33 ? 5 : 3);
	gui_drawHorizontalBarGraph(250, 25, 51, 5, c->food, 100, col, _color5);
}

void EobCoreEngine::gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 curVal, int32 maxVal, int col1, int col2) {
	gui_drawBox(x - 1, y - 1, w + 3, h + 2, _color2_1, _color1_1, -1);
	LolEobBaseEngine::gui_drawHorizontalBarGraph(x, y, w + 2, h, curVal, maxVal, col1, col2);
}

void EobCoreEngine::gui_drawCharPortraitStatusFrame(int index) {
	uint8 boxColor = ((_partyEffectFlags & 0x20000) | (_partyEffectFlags & 0xffff)) ? 4 : 6;

	static const uint8 xCoords[] = { 8, 80 };
	static const uint8 yCoords[] = { 2, 54, 106 };
	int x = xCoords[index & 1];
	int y = yCoords[index >> 1];

	if (!_screen->_curPage)
		x += 176;

	EobCharacter *c = &_characters[index];

	int v8 = (_flags.gameID == GI_EOB2 && ((c->effectFlags & 0x4818) || c->effectsRemainder[0] || c->effectsRemainder[1] || ((_partyEffectFlags & 0x20000) | (_partyEffectFlags & 0xffff)))) ||
		(_flags.gameID == GI_EOB1 && ((c->effectFlags & 0x302) || c->effectsRemainder[0] || c->effectsRemainder[1])) ? 1 : 0;
	int vA = (_flags.gameID == GI_EOB2 && ((((c->effectFlags & 0x3000) | (c->effectFlags & 0x10000)) || (_partyEffectFlags & 0x8420)))) ||
		(_flags.gameID == GI_EOB1 && ((c->effectFlags & 0x4c8) || _partyEffectFlags & 300000))? 1 : 0;

	if (v8 || vA) {
		if (v8 && !vA) {
			_screen->drawBox(x, y, x + 63, y + 49, boxColor);
			return;
		}

		if (vA && !v8) {
			_screen->drawBox(x, y, x + 63, y + 49, 5);
			return;
		}

		int iX= x;
		int iY= y;

		for (int i = 0; i < 64; i += 16) {
			x = iX + i;
			if (v8) {
				_screen->drawClippedLine(x, y, x + 7, y, boxColor);
				_screen->drawClippedLine(x + 8, y + 49, x + 15, y + 49, boxColor);
			}
			if (vA) {
				_screen->drawClippedLine(x + 8, y, x + 15, y, 5);
				_screen->drawClippedLine(x, y + 49, x + 7, y + 49, 5);
			}
		}

		x = iX;

		for (int i = 1; i < 48; i += 12) {
			y = iY + i - 1;

			if (vA) {
				_screen->drawClippedLine(x, y + 1, x, y + 6, 5);
				_screen->drawClippedLine(x + 63, y + 7, x + 63, y + 12, 5);
			}
			if (v8) {
				_screen->drawClippedLine(x, y + 7, x, y + 12, boxColor);
				_screen->drawClippedLine(x + 63, y + 1, x + 63, y + 6, boxColor);
			}
		}

	} else {
		_screen->drawClippedLine(x, y, x + 62, y, _color1_1);
		_screen->drawClippedLine(x, y + 49, x + 62, y + 49, _color2_1);
		_screen->drawClippedLine(x - 1, y, x - 1, y + 50, 12);
		_screen->drawClippedLine(x + 63, y, x + 63, y + 50, 12);
	}
}

void EobCoreEngine::gui_drawInventoryItem(int slot, int special, int pageNum) {
	int x = _inventorySlotsX[slot];
	int y = _inventorySlotsY[slot];

	int item = _characters[_updateCharNum].inventory[slot];
	int cp = _screen->setCurPage(pageNum);

	if (special) {
		int wh = (slot == 25 || slot == 26) ? 10 : 18;
		gui_drawBox(x - 1, y - 1, wh, wh, _color1_1, _color2_1, slot == 16 ? -1 : _bkgColor_1);

		if (slot == 16) {
			_screen->fillRect(227, 65, 238, 69, 12);
			int cnt = countQueuedItems(_characters[_updateCharNum].inventory[slot], -1, -1, 1, 1);
			x = cnt >= 10 ? 227 : 233;
			Common::String str = Common::String::format("%d", cnt);
			_screen->printText(str.c_str(), x, 65, 15, 0);
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

void EobCoreEngine::gui_drawCompass(bool force) {
	if (_currentDirection == _compassDirection && !force)
		return;

	static const uint8 shpX[2][3] = { { 0x70, 0x4D, 0x95 }, { 0x72, 0x4F, 0x97 } };
	static const uint8 shpY[2][3] = { { 0x7F, 0x9A, 0x9A }, { 0x83, 0x9E, 0x9E } };
	int g = _flags.gameID == GI_EOB1 ? 0 : 1;

	for (int i = 0; i < 3; i++)
		_screen->drawShape(_screen->_curPage, _compassShapes[(i << 2) + _currentDirection], shpX[g][i], shpY[g][i], 0);

	_compassDirection = _currentDirection;
}

void EobCoreEngine::gui_drawDialogueBox() {
	gui_drawBox(0, 121, 320, 79, _color1_1, _color2_1, _bkgColor_1);
	txt()->clearCurDim();
}

void EobCoreEngine::gui_drawSpellbook() {
	_screen->setCurPage(2);
	int numTab = (_flags.gameID == GI_EOB1) ? 5 : 6;
	_screen->copyRegion(64, 121, 64, 121, 112, 56, 0, 2, Screen::CR_NO_P_CHECK);

	for (int i = 0; i < numTab; i++) {
		int col1 = _color14;
		int col2 = _color13;
		int col3 = _color12;

		if (i == _openBookSpellLevel) {
			col1 =  _color1_1;
			col2 =  _color2_1;
			col3 =  _bkgColor_1;
		}

		if (_flags.gameID == GI_EOB1) {
			gui_drawBox(i * 21 + 71, 122, 21, 9, col1, col2, col3);
			_screen->printText(_magicStrings7[i], i * 21 + 73, 123, 12, 0);
		} else {
			gui_drawBox(i * 18 + 68, 121, 18, 9, col1, col2, col3);
			_screen->printText(Common::String::format("%d", i + 1).c_str(), i * 18 + 75, 123, 12, 0);
		}
	}

	if (_flags.gameID == GI_EOB1)
		gui_drawBox(71, 131, 105, 44, _color1_1, _color2_1, _bkgColor_1);
	else {
		gui_drawBox(68, 130, 108, 47, _color1_1, _color2_1, _bkgColor_1);
		gui_drawBox(68, 168, 78, 9, _color8, _color7, _color6);
		gui_drawBox(146, 168, 14, 9, _color8, _color7, _color6);
		gui_drawBox(160, 168, 16, 9, _color8, _color7, _color6);
		gui_drawSpellbookScrollArrow(150, 169, 0);
		gui_drawSpellbookScrollArrow(165, 169, 1);
	}

	int textCol1 = 15;
	int textCol2 = 8;
	int textXa = 74;
	int textXs = 71;
	int textY = 170;
	int col3 = _bkgColor_1;

	if (_flags.gameID == GI_EOB1) {
		textCol2 = 11;
		textXa = textXs = 73;
		textY = 168;
	}

	for (int i = 0; i < 7; i++) {
		int d = _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + i];
		if (_openBookSpellSelectedItem == i) {
			if (d >= 0 && i < 6 && (i + _openBookSpellListOffset) < 9) {
				_screen->printText(_openBookSpellList[d], textXs, 132 + 6 * i, textCol1, textCol2);
			} else if (i == 6) {
				if (_flags.gameID == GI_EOB2)
					_screen->fillRect(69, 169, 144, 175, textCol2);
				_screen->printText(_magicStrings1[0], textXa, textY, textCol1, textCol2);
			}
		} else {
			if (d >= 0 && i < 6 && (i + _openBookSpellListOffset) < 9)
				_screen->printText(_openBookSpellList[d], textXs, 132 + 6 * i, textCol1, col3);
			else
				_screen->printText(_magicStrings1[0], textXa, textY, 12, _color6);
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

	_screen->setCurPage(0);
	_screen->copyRegion(64, 121, 64, 121, 112, 56, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

void EobCoreEngine::gui_drawSpellbookScrollArrow(int x, int y, int direction) {
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

void EobCoreEngine::gui_updateSlotAfterScrollUse() {
	_characters[_openBookChar].disabledSlots ^= (1 << (--_castScrollSlot));
	setCharEventTimer(_openBookChar, 18, _castScrollSlot + 2, 1);
	gui_drawCharPortraitWithStats(_openBookChar);
	_openBookChar = _openBookCharBackup;
	_openBookType = _openBookTypeBackup;
	_castScrollSlot = 0;
	gui_toggleButtons();
}

void EobCoreEngine::gui_updateControls() {
	Button b;
	if (_currentControlMode)
		clickedPortraitRestore(&b);
	if (_updateFlags)
		clickedSpellbookAbort(&b);
}

void EobCoreEngine::gui_toggleButtons() {
	if (_currentControlMode == 0)
		gui_setPlayFieldButtons();
	else if (_currentControlMode == 1)
		gui_setInventoryButtons();
	else if (_currentControlMode == 2)
		gui_setStatsListButtons();
}

void EobCoreEngine::gui_setPlayFieldButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList2 : _buttonList1);
}

void EobCoreEngine::gui_setInventoryButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList5 : _buttonList3);
}

void EobCoreEngine::gui_setStatsListButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_updateFlags ? _buttonList6 : _buttonList4);
}

void EobCoreEngine::gui_setSwapCharacterButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList7);
}

void EobCoreEngine::gui_setCastOnWhomButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList8);
}

void EobCoreEngine::gui_initButton(int index, int, int, int) {
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

	const EobGuiButtonDef *d = &_buttonDefs[index];

	if (_flags.gameID == GI_EOB1) {
		// EOB1 spellbook modifications
		if (index > 61 && index < 67)
			d = &_buttonDefs[index + 33];
		if (index == 88)
			d = &_buttonDefs[index + 12];
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
	b->buttonCallback = d->buttonCallback;
}

int EobCoreEngine::clickedCharPortraitDefault(Button *button) {
	if (!testCharacter(button->arg, 1))
		return 1;

	gui_processCharPortraitClick(button->arg);
	return 0;
}

int EobCoreEngine::clickedCamp(Button *button) {
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

	_screen->copyRegion(0, 120, 0, 0, 176, 24, 0, 14, Screen::CR_NO_P_CHECK);
	
	_gui->runCampMenu();	
	
	_screen->copyRegion(0, 0, 0, 120, 176, 24, 14, 2, Screen::CR_NO_P_CHECK);
	_screen->setScreenDim(cd);
	drawScene(0);

	for (int i = 0; i < 6; i++)
		cleanupCharacterSpellList(i);

	_screen->setCurPage(0);
	const ScreenDim *dm = _screen->getScreenDim(10);
	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 2, 0, Screen::CR_NO_P_CHECK);	
	
	//if (displayInv)
	//	_screen->loadEobCpsFileToPage("INVENT", 0, 5, 3, 2);
	
	_screen->updateScreen();

	enableSysTimer(2);	
	updateCharacterEvents(true);

	return button->arg;
}

int EobCoreEngine::clickedSceneDropPickupItem(Button *button) {
	uint16 block = _currentBlock;
	if (button->arg > 1) {
		block = calcNewBlockPosition(_currentBlock, _currentDirection);
		int f = _wllWallFlags[_levelBlockProperties[block].walls[_sceneDrawVarDown]];
		if (!(f & 0x0b))
			return 1;
	}
	int d = _dropItemDirIndex[(_currentDirection << 2) + button->arg];

	if (_itemInHand) {
		setItemPosition((Item*)&_levelBlockProperties[block & 0x3ff].drawObjects, block, _itemInHand, d);
		setHandItem(0);
		runLevelScript(block, 4);
	} else {
		d = getQueuedItem((Item*)&_levelBlockProperties[block].drawObjects, d, -1);
		if (!d)
			return 1;
		setHandItem(d);
		runLevelScript(block, 8);
	}

	_sceneUpdateRequired = true;
	return 1;
}

int EobCoreEngine::clickedCharPortrait2(Button *button) {
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

int EobCoreEngine::clickedWeaponSlot(Button *button) {
	if (!testCharacter(button->arg, 1))
		return 1;

	static const uint8 sY[] = { 24, 24, 80, 80, 136, 136 };
	int slot = sY[button->arg] > _mouseY ? 0 : 1;

	if ((_gui->_flagsMouseLeft & 0x7f) == 1)
		gui_processWeaponSlotClickLeft(button->arg, slot);
	else
		gui_processWeaponSlotClickRight(button->arg, slot);

	return 1;
}

int EobCoreEngine::clickedCharNameLabelRight(Button *button) {
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

		EobCharacter temp;
		memcpy(&temp, &_characters[d], sizeof(EobCharacter));
		memcpy(&_characters[d], &_characters[button->arg], sizeof(EobCharacter));
		memcpy(&_characters[button->arg], &temp, sizeof(EobCharacter));

		_timer->disable(0);
		gui_drawCharPortraitWithStats(d);
		gui_processCharPortraitClick(button->arg);
		gui_drawCharPortraitWithStats(button->arg);
		gui_setPlayFieldButtons();
		setupCharacterTimers();
	}

	return button->index;
}

int EobCoreEngine::clickedInventorySlot(Button *button) {
	gui_processInventorySlotClick(button->arg);
	return button->index;
}

int EobCoreEngine::clickedEatItem(Button *button) {
	eatItemInHand(_updateCharNum);
	return button->index;
}

int EobCoreEngine::clickedInventoryPrevChar(Button *button) {
	if (_gui->_progress == 1)
		_updateCharNum = 0;
	else if (_gui->_progress == 2)
		_updateCharNum = 1;
	else
		_updateCharNum = getNextValidCharIndex(_updateCharNum, -1);

	gui_drawCharPortraitWithStats(_updateCharNum);
	return button->index;
}

int EobCoreEngine::clickedInventoryNextChar(Button *button) {
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

int EobCoreEngine::clickedSpellbookTab(Button *button) {
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

int EobCoreEngine::clickedSpellbookList(Button *button) {
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
					_openBookSpellSelectedItem= listIndex;
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
					s += _mageSpellListSize;

				castSpell(s, 0);

			} else if ((_openBookSpellSelectedItem == 6 && listIndex == 7) || (_openBookSpellSelectedItem != 6 && listIndex == 6) ) {
				Button b;
				clickedSpellbookAbort(&b);
			}
		}
	}

	_characters[_openBookChar].slotStatus[2] = _openBookSpellSelectedItem;
	_characters[_openBookChar].slotStatus[4] = _openBookSpellListOffset;
	return button->index;
}

int EobCoreEngine::clickedCastSpellOnCharacter(Button *button) {
	_activeSpellCaster = button->arg;

	if (_activeSpellCaster == 255) {
		_txt->printMessage(_magicStrings3[1]);
		snd_playSoundEffect(79);
		if (_castScrollSlot) {
			gui_updateSlotAfterScrollUse();
		} else {
			gui_toggleButtons();
			gui_drawSpellbook();
		}
	} else {
		if (_characters[_activeSpellCaster].flags & 1)
			startSpell(_activeSpell);
	}

	return button->index;
}

int EobCoreEngine::clickedInventoryNextPage(Button *button) {
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

int EobCoreEngine::clickedPortraitRestore(Button *button) {
	_currentControlMode = 0;
	_screen->_curPage = 2;
	_screen->copyRegion(0, 0, 0, 0, 144, 168, 5, _screen->_curPage, Screen::CR_NO_P_CHECK);
	gui_drawAllCharPortraitsWithStats();
	_screen->_curPage = 0;
	_screen->copyRegion(0, 0, 176, 0, 144, 168, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	gui_setPlayFieldButtons();
	return button->index;
}

int EobCoreEngine::clickedUpArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, _currentDirection);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EobCoreEngine::clickedDownArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection + 2) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EobCoreEngine::clickedLeftArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection - 1) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EobCoreEngine::clickedRightArrow(Button *button) {
	int b = calcNewBlockPositionAndTestPassability(_currentBlock, (_currentDirection + 1) & 3);

	if (b == -1) {
		notifyBlockNotPassable();
	} else {
		moveParty(b);
		_sceneDefaultUpdate = 1;
	}

	return button->index;
}

int EobCoreEngine::clickedTurnLeftArrow(Button *button) {
	_currentDirection = (_currentDirection - 1) & 3;
	//_keybControlUnk = -1;
	_sceneDefaultUpdate = 1;
	_sceneUpdateRequired = true;
	return button->index;
}

int EobCoreEngine::clickedTurnRightArrow(Button *button) {
	_currentDirection = (_currentDirection + 1) & 3;
	//_keybControlUnk = -1;
	_sceneDefaultUpdate = 1;
	_sceneUpdateRequired = true;
	return button->index;
}

int EobCoreEngine::clickedAbortCharSwitch(Button *button) {
	_timer->disable(0);
	int c = _exchangeCharacterId;
	_exchangeCharacterId = -1;
	gui_drawCharPortraitWithStats(c);
	gui_setPlayFieldButtons();
	return button->index;
}

int EobCoreEngine::clickedSceneThrowItem(Button *button) {
	if (!_itemInHand)
		return button->index;

	if (launchObject(_updateCharNum, _itemInHand, _currentBlock, _dropItemDirIndex[(_currentDirection << 2) + button->arg], _currentDirection, _items[_itemInHand].type)) {
		setHandItem(0);
		_sceneUpdateRequired = true;
	}

	return button->index;
}

int EobCoreEngine::clickedSceneSpecial(Button *button) {
	_clickedSpecialFlag = 0x40;
	return specialWallAction(calcNewBlockPosition(_currentBlock, _currentDirection), _currentDirection);
}

int EobCoreEngine::clickedSpellbookAbort(Button *button) {
	_updateFlags = 0;
	_screen->copyRegion(0, 0, 64, 121, 112, 56, 10, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	gui_drawCompass(true);
	gui_toggleButtons();
	return button->index;
}

int EobCoreEngine::clickedSpellbookScroll(Button *button) {
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

int EobCoreEngine::clickedUnk(Button *button) {
	return button->index;
}

void EobCoreEngine::gui_processCharPortraitClick(int index) {
	if (index == _updateCharNum)
		return;

	int a = _updateCharNum;
	_updateCharNum = index;

	gui_drawCharPortraitWithStats(a);
	gui_drawCharPortraitWithStats(index);
}

void EobCoreEngine::gui_processWeaponSlotClickLeft(int charIndex, int slotIndex) {
	int itm = _characters[charIndex].inventory[slotIndex];
	if (_items[itm].flags & 0x20)
		return;

	int ih = _itemInHand;
	int t = _items[ih].type;
	uint16 v = (ih) ? _itemTypes[t].invFlags : 0xffff;

	if (v & _slotValidationFlags[slotIndex]) {
		setHandItem(itm);
		_characters[charIndex].inventory[slotIndex] = ih;
		gui_drawCharPortraitWithStats(charIndex);
	}

	recalcArmorClass(charIndex);
}

void EobCoreEngine::gui_processWeaponSlotClickRight(int charIndex, int slotIndex) {
	const char * const *strs = &_itemExtraStrings[_flags.gameID == GI_EOB1 ? 17 : (_flags.lang == Common::DE_DEU ? 26 : 22)];

	if (!testCharacter(charIndex, 0x0d))
		return;

	uint16 itm = _characters[charIndex].inventory[slotIndex];
	int wslot = slotIndex < 2 ? slotIndex : -1;

	if (slotIndex < 2 && (!validateWeaponSlotItem(charIndex, slotIndex) || (!_currentControlMode && (_characters[charIndex].disabledSlots & (1 << slotIndex)))))
		return;

	if (!itemUsableByCharacter(charIndex, itm))
		_txt->printMessage(strs[0], -1, _characters[charIndex].name);

	if (!itm && slotIndex > 1)
		return;

	int8 tp = _items[itm].type;
	int8 vl = _items[itm].value;
	uint8 ep = _itemTypes[tp].extraProperties & 0x7f;

	switch (ep) {
		case 0:
		case 16:
			// Item automatically used when worn
			_txt->printMessage(strs[1]);
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
			_txt->printMessage(strs[2]);
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
			// drop through
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
			usePotion(charIndex, wslot);
			break;

		case 18:
			ep = ep;
			break;

		case 19:
			// eob2 horn
			ep = ep;
			break;

		case 20:
			if (vl == 1)
				inflictCharacterDamage(charIndex, 200);
			else
				useMagicScroll(charIndex, 55, wslot);
			deleteInventoryItem(charIndex, wslot);
			break;

		default:
			break;
	}

	if (ep == 1 && charIndex >= 2)
		return;

	_lastUsedItem = itm;
	runLevelScript(calcNewBlockPosition(_currentBlock, _currentDirection), 0x100);
	_lastUsedItem = 0;
}

void EobCoreEngine::gui_processInventorySlotClick(int slot) {
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

GUI_Eob::GUI_Eob(EobCoreEngine *vm) : GUI(vm), _vm(vm), _screen(vm->_screen) {
	_scrollUpFunctor = _scrollDownFunctor = BUTTON_FUNCTOR(GUI_Eob, this, 0);

	_redrawButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawButtonCallback);
	_redrawShadedButtonFunctor = BUTTON_FUNCTOR(GUI, this, &GUI::redrawShadedButtonCallback);

	_menuStringsPrefsTemp = new char*[4];
	memset(_menuStringsPrefsTemp, 0, 4 * sizeof(char*));

	_specialProcessButton = _backupButtonList = 0;
	_flagsMouseLeft = _flagsMouseRight = _flagsModifier = 0;
	_backupButtonList = 0;
	_progress = 0;
	_prcButtonUnk3 = 1;
	_cflag = 0xffff;
	
	_menuLineSpacing = 0;
	_menuUnk1 = 0;
	_menuLastInFlags = 0;
	_menuCur = 0;
	_menuNumItems = 0;
	_menuButtons = 0;
}

GUI_Eob::~GUI_Eob() {
	if (_menuStringsPrefsTemp) {
		for (int i = 0; i < 4; i++)
			delete _menuStringsPrefsTemp[i];
		delete[] _menuStringsPrefsTemp;
	}
}

void GUI_Eob::processButton(Button *button) {
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
					_screen->printText((const char*) button->data1ShapePtr, sx, sy, col1, col2);
			} else if (button->data0Val1 == 3) {
				// nullsub (at least EOBII)
			} else if (button->data0Val1 == 4) {
				if (button->data1Callback)
					(*button->data1Callback.get())(button);
			}
		} else if (button->data1Val1 == 2) {
			if (!(button->flags2 & 4))
				_screen->drawBox(sx, sy, fx2, fy2, col1);
		} else if (button->data1Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data1Val1 == 4) {
			if (button->data1Callback)
				(*button->data1Callback.get())(button);
		}
	}

	if (button->flags2 & 4) {
		if (button->data2Val1 == 1) {
			if (button->data0Val1 == 1) {
				_screen->drawShape(_screen->_curPage, button->data2ShapePtr, fx, fy, sd);
			} else if (button->data0Val1 == 2) {
				if (button->flags2 & 1)
					_screen->printText((const char*) button->data2ShapePtr, sx, sy, button->data3Val2, button->data3Val3);
				else
					_screen->printText((const char*) button->data2ShapePtr, sx, sy, button->data2Val2, button->data2Val3);
			} else if (button->data0Val1 == 3) {
				// nullsub (at least EOBII)
			} else if (button->data0Val1 == 4) {
				if (button->data2Callback)
					(*button->data2Callback.get())(button);
			}
		} else if (button->data2Val1 == 2) {
			_screen->drawBox(sx, sy, fx2, fy2, (button->flags2 & 1) ? button->data3Val2 : button->data2Val2);
		} else if (button->data2Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data2Val1 == 4) {
			if (button->data2Callback)
				(*button->data2Callback.get())(button);
		}
	}

	if (!(button->flags2 & 5)) {
		if (button->data0Val1 == 1) {
			_screen->drawShape(_screen->_curPage, button->data0ShapePtr, fx, fy, sd);
		} else if (button->data0Val1 == 2) {
			_screen->printText((const char*) button->data0ShapePtr, sx, sy, button->data0Val2, button->data0Val3);
		} else if (button->data0Val1 == 3) {
			// nullsub (at least EOBII)
		} else if (button->data0Val1 == 4) {
			if (button->data0Callback)
				(*button->data0Callback.get())(button);
		} else if (button->data0Val1 == 5) {
			_screen->drawBox(sx, sy, fx2, fy2, button->data0Val2);
		} else {
			if (!button->data0Val1) {
				if(button->data1Val1 == 2 || button->data2Val1 == 2) {
					_screen->drawBox(sx, sy, fx2, fy2, button->data0Val2);
				} else {
					// nullsub (at least EOBII)
				}
			}
		}
	}
}

int GUI_Eob::processButtonList(Kyra::Button *buttonList, uint16 inputFlags, int8 mouseWheel) {
	_progress = 0;
	uint16 in = inputFlags & 0xff;
	uint16 buttonReleaseFlag = 0;
	bool clickEvt = false;
	//_vm->_processingButtons = true;
	_flagsMouseLeft = (_vm->_mouseClick == 1) ? 2 : 4;
	_flagsMouseRight = (_vm->_mouseClick == 2) ? 2 : 4;
	_vm->_mouseClick = 0;

	if (in >= 199 && in <= 202) {
		buttonReleaseFlag = (inputFlags & 0x800) ? 3 : 1;
		if (in < 201)
			_flagsMouseLeft = buttonReleaseFlag;
		else
			_flagsMouseRight = buttonReleaseFlag;

		////////////////////////////
		if (!buttonList && !(inputFlags & 0x800))
			return inputFlags & 0xff;
		////////////////////////////

		inputFlags = 0;
		clickEvt = true;
	} else if (inputFlags & 0x8000) {
		inputFlags &= 0xff;
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
				// if (_processButtonListExtraCallback)
				//	this->*_processButtonListExtraCallback(buttonList);
			//}

			if (buttonList->nextButton)
				buttonList = buttonList->nextButton;
			else
				runLoop = false;
		}

		_vm->_buttonListChanged = false;

		_specialProcessButton = 0;
		_prcButtonUnk3 = 1;
		_cflag = 0xffff;
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
			flgs2 &= 0xfff7;

		if (flgs2 & 4)
			flgs2 |= 0x10;
		else
			flgs2 &= 0xffef;

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
			vL = flgs & 0xf00;
			vR = flgs & 0xf000;

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
								flgs2 &= 0xfffb;
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
								flgs2 &= 0xfffb;
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
								flgs2 &= 0xfffe;
							break;

						case 3:
							if ((flgs & 4) || (!buttonList->data2Val1))
								flgs2 &= 0xfffb;
							else
								flgs2 |= 4;

							if (flgs & 0x800) {
								v6 = 1;
								break;
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xfffe;
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
								flgs2 &= 0xfffb;

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
								flgs2 &= 0xfffb;

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
								flgs2 &= 0xfffe;
							break;

						case 3:
							if ((flgs & 4) || (!buttonList->data2Val1))
								flgs2 &= 0xfffb;
							else
								flgs2 |= 4;

							if (flgs & 0x8000) {
								v6 = 1;
								break;
							}

							if ((flgs & 2) && (flgs2 & 1))
								flgs2 &= 0xfffe;
							break;

						default:
							break;
						}
					}
				} else { // if (_vm->_mouseX >= x2 && _vm->_mouseX <= (x2 + buttonList->width)....)
					flgs2 &= 0xfff9;

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
						flgs2 &= 0xfffe;
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
			runLoop = ((*buttonList->buttonCallback.get())(buttonList)) ? false : true;

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
		_cflag = 0xffff;

	if (!result)
		result = inputFlags;
	
	//_vm->_processingButtons=false;

	return result;
}

void GUI_Eob::setupMenu(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int unk, int lineSpacing) {
	initMenuItemsMask(sd, maxItem, menuItemsMask, unk);

	const ScreenDim *dm = _screen->getScreenDim(19 + sd);
	int x = (_screen->_curDim->sx + dm->sx) << 3;
	int y = _screen->_curDim->sy + dm->sy;

	int v = getMenuItem(_menuCur, menuItemsMask, unk);

	for (int i = 0; i < _menuNumItems; i++) {
		int item = getMenuItem(i, menuItemsMask, unk);
		int ty = y + i * (lineSpacing + _screen->getFontHeight());
		_screen->printShadedText(strings[item], x, ty, dm->unkA, 0);
		if (item == v)
			_screen->printText(strings[item], x, ty, dm->unkC, 0);
	}

	_screen->updateScreen();
	_menuLineSpacing = lineSpacing;
	_menuUnk1 = 0;
	_menuLastInFlags = 0;
	_vm->removeInputTop();
}

int GUI_Eob::handleMenu(int sd, const char *const *strings, void *b, int32 menuItemsMask, int unk) {
	const ScreenDim *dm = _screen->getScreenDim(19 + sd);
	int h = _menuNumItems - 1;
	int currentItem = _menuCur % _menuNumItems;
	int newItem = currentItem;
	int result = -1;
	int lineH = (_menuLineSpacing + _screen->getFontHeight());
	int lineS1 = _menuLineSpacing >> 1;
	int x = (_screen->_curDim->sx + dm->sx) << 3;
	int y = _screen->_curDim->sy + dm->sy;

	int inFlag = _vm->checkInput(0, false, 0) & 0x8ff;
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
		_screen->printText(strings[getMenuItem(currentItem, menuItemsMask, unk)], x, y + currentItem * lineH , dm->unkA, 0);
		_screen->printText(strings[getMenuItem(newItem,  menuItemsMask, unk)], x, y + newItem * lineH , dm->unkC, 0);
		_screen->updateScreen();
	}

	if (result != -1) {
		result = getMenuItem(result, menuItemsMask, unk);
		menuFlashSelection(strings[result], x, y + newItem * lineH, dm->unkA, dm->unkC, 0);
	}

	_menuCur = newItem;

	return result;
}

int GUI_Eob::getMenuItem(int index, int32 menuItemsMask, int unk) {
	if (menuItemsMask == -1)
		return index;

	int res = 0;
	int i = index;

	for (; i; res++) {
		if (menuItemsMask & (1 << (res + unk)))
			i--;
	}

	while (!(menuItemsMask & (1 << (res + unk))))
		res++;

	return res;
}

void GUI_Eob::menuFlashSelection(const char *str, int x, int y, int color1, int color2, int color3) {
	for (int i = 0; i < 3; i++) {
		_screen->printText(str, x, y, color2, color3);
		_screen->updateScreen();
		_vm->_system->delayMillis(32);
		_screen->printText(str, x, y, color1, color3);
		_screen->updateScreen();
		_vm->_system->delayMillis(32);
	}
}

int GUI_Eob::runCampMenu() {
	Screen::FontId of = _screen->setFont(Screen::FID_8_FNT);
	
	Button *highlightButton = 0;
	Button *prevHighlightButton = 0;
	
	int newMenu = 0;
	int lastMenu = -1;
	int e = 0;
	int menuG = 0;
	int menuH = 0;
	_menuButtons = 0;

	for (bool runLoop = true; runLoop && !_vm->shouldQuit(); ) {
		if (newMenu == 2)
			updateOptionsStrings();
		
		if (newMenu != -1) {
			releaseButtons(_menuButtons);

			_vm->_menuDefs[0].titleStrId = newMenu ? 1 : 56;
			if (newMenu == 2)
				_vm->_menuDefs[2].titleStrId = 57;
			else if (newMenu == 1)
				_vm->_menuDefs[1].titleStrId = 58;
			
			_menuButtons = initMenu(newMenu);

			if (newMenu != lastMenu) {
				highlightButton = _menuButtons;
				prevHighlightButton = 0;
			}

			lastMenu = newMenu;
			newMenu = -1;
		}
		
		int inputFlag = _vm->checkInput(_menuButtons, false, 0) & 0x80ff;
		_vm->removeInputTop();
		
		if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE])
			inputFlag = 0x8007;
		else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			inputFlag = 0x8000 + prevHighlightButton->index;
		}

		Button *clickedButton = _vm->gui_getButton(_menuButtons, inputFlag & 0x7fff);

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
			highlightButton = _vm->gui_getButton(_menuButtons, _vm->_menuDefs[lastMenu].firstButtonStrId + _vm->_menuDefs[lastMenu].numButtons);
			inputFlag = _vm->_keyMap[Common::KEYCODE_UP];
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP7] || inputFlag == _vm->_keyMap[Common::KEYCODE_HOME] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP9] || inputFlag == _vm->_keyMap[Common::KEYCODE_PAGEUP]) {
			highlightButton = _vm->gui_getButton(_menuButtons, _vm->_menuDefs[lastMenu].firstButtonStrId + 1);
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_KP8] || inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
			if (prevHighlightButton) {
				int dir = (inputFlag == _vm->_keyMap[Common::KEYCODE_UP]) ? -1 : 1;
				int s = prevHighlightButton->index + dir;
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
				} while(!_vm->shouldQuit());

				highlightButton = _vm->gui_getButton(_menuButtons, s);
			}
			
		} else if (inputFlag > 0x8001 && inputFlag < 0x8010) {
			switch (inputFlag) {
			case 0x8001:
				break;
			case 0x8002:
				break;
			case 0x8003:
				break;
			case 0x8004:
				break;
			case 0x8005:
				newMenu = 2;
				break;
			case 0x8006:
				newMenu = 1;
				break;
			case 0x8007:
				if (menuH)
					displayTextBox(44);
				// fall through
			case 0x800c:
			case 0x800f:
				if (lastMenu == 1 || lastMenu == 2)
					newMenu = 0;
				else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE])
					newMenu = 0;
				else
					runLoop = false;
				break;
			case 0x8008:
				break;
			case 0x8009:
				break;
			case 0x800a:
				break;
			case 0x800b:
				break;
			case 0x800d:
				break;
			case 0x800e:
				break;
			default:
				break;
			}
			
		} else {
			Common::Point p = _vm->getMousePos();
			for (Button *b = _menuButtons; b; b = b->nextButton) {
				if ((b->arg & 2) && _vm->posWithinRect(p.x, p.y, b->x, b->y, b->x + b->width, b->y + b->height))
					highlightButton = b;
			}
		}
		
		if (menuG || e) {
			for (int i = 0; i < 6; i++) {
				_vm->gui_drawCharPortraitWithStats(i);
				_vm->cleanupCharacterSpellList(i);
			}
		}

		menuG = e = 0;

		if (prevHighlightButton != highlightButton && newMenu == -1 && runLoop) {
			drawMenuButton(prevHighlightButton, false, false, true);
			drawMenuButton(highlightButton, false, true, true);
			_screen->updateScreen();
			prevHighlightButton = highlightButton;
		}
	}

	releaseButtons(_menuButtons);
	_menuButtons = 0;
	
	_screen->setFont(of);

	return 0;
}

int GUI_Eob::runLoadMenu(int x, int y) {
	return 0;
}

int GUI_Eob::getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor) {
	uint8 cursorState = 1;
	char sufx[] = " ";

	int len = strlen(dest);
	if (len > destMaxLen) {
		len = destMaxLen;
		dest[destMaxLen] = 0;
	}

	int pos = len;
	if (len >= destMaxLen)
		pos--;

	_screen->copyRegion((x - 1) << 3, y, 0, 191, (destMaxLen + 2) << 3, 9, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->printShadedText(dest, x << 3, y, textColor1, textColor2);

	uint32 next = _vm->_system->getMillis() + 2 * _vm->_tickLength;
	sufx[0] = (pos < len) ? dest[pos] : 32;
	_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);

	int in = 0;

	do {
		in = 0;
		_keyPressed.reset();

		while (!in) {
			if (next <= _vm->_system->getMillis()) {
				if (cursorState) {
					_screen->copyRegion((pos + 1) << 3, 191, (x + pos) << 3, y, 8, 9, 2, 0, Screen::CR_NO_P_CHECK);
					_screen->printShadedText(sufx, (x + pos) << 3, y, textColor1, textColor2);
				} else {
					_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);
				}

				_screen->updateScreen();
				cursorState ^= 1;
				next = _vm->_system->getMillis() + 2 * _vm->_tickLength;
			}

			_vm->updateInput();
			for (Common::List<KyraEngine_v1::Event>::const_iterator evt = _vm->_eventList.begin(); evt != _vm->_eventList.end(); ++evt) {
				if (evt->event.type == Common::EVENT_KEYDOWN) {
					_keyPressed = evt->event.kbd;
					in = _keyPressed.ascii;
				}
			}
			_vm->removeInputTop();
		}

		if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE) {
			if (pos >= len && len > 0) {
				dest[--len] = 0;
				pos--;

			} else if (pos > 0) {
				for (int i = pos; i < destMaxLen; i++)
					dest[i - 1] = dest[i];
				dest[--len] = 0;
				pos--;
			}

		} else if (_keyPressed.keycode == Common::KEYCODE_LEFT || _keyPressed.keycode == Common::KEYCODE_KP4) {
			if (pos > 0)
				pos--;

		} else if (_keyPressed.keycode == Common::KEYCODE_RIGHT || _keyPressed.keycode == Common::KEYCODE_KP6) {
			if (pos < len && pos < (destMaxLen - 1))
				pos++;

		} else if (in > 31 && in < 126) {
			if (!(in == 32 && pos == 0)) {
				if (in >= 97 && in <= 122)
					in -=32;

				if (pos < len) {
					for (int i = destMaxLen - 1; i >= pos; i--)
						dest[i + 1] = dest[i];

					dest[pos++] = in;

					if (len == destMaxLen)
						dest[len] = 0;

				} else {
					if (pos == destMaxLen) {
						pos--;
						len--;
					}

					dest[pos++] = in;
					dest[pos] = 0;
				}

				if (++len > destMaxLen)
					len = destMaxLen;

				if (pos > (destMaxLen - 1))
					pos = (destMaxLen - 1);
			}
		}

		_screen->copyRegion(0, 191, (x - 1) << 3, y, (destMaxLen + 2) << 3, 9, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->printShadedText(dest, x << 3, y, textColor1, textColor2);
		sufx[0] = (pos < len) ? dest[pos] : 32;

		if (cursorState)
			_screen->printText(sufx, (x + pos) << 3, y, textColor1, cursorColor);
		else
			_screen->printShadedText(sufx, (x + pos) << 3, y, textColor1, textColor2);
		_screen->updateScreen();

	} while (_keyPressed.keycode != Common::KEYCODE_RETURN && _keyPressed.keycode != Common::KEYCODE_ESCAPE);

	return _keyPressed.keycode == Common::KEYCODE_ESCAPE ? -1 : len;
}

void GUI_Eob::initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int unk) {
	if (menuItemsMask == -1) {
		_menuNumItems = _screen->getScreenDim(19 + menuId)->h;
		_menuCur = _screen->getScreenDim(19 + menuId)->unk8;
		return;
	}

	_menuNumItems = 0;

	for (int i = 0; i < maxItem; i++) {
		if (menuItemsMask & (1 << (i + unk)))
			_menuNumItems++;
	}

	_menuCur = 0;
}

Button *GUI_Eob::initMenu(int id) {
	_screen->setCurPage(2);

	EobMenuDef *m = &_vm->_menuDefs[id];

	if (m->dim) {
		const ScreenDim *dm = _screen->getScreenDim(m->dim);
		_screen->fillRect(dm->sx << 3, dm->sy, ((dm->sx + dm->w) << 3) - 1, (dm->sy + dm->h) - 1, _vm->_bkgColor_1);
		_screen->setScreenDim(m->dim);
		drawMenuButtonBox(dm->sx << 3, dm->sy, dm->w << 3, dm->h, false, false);		
	}

	_screen->printShadedText(getMenuString(m->titleStrId), 5, 5, m->titleCol, 0);

	Button *buttons = 0;
	for (int i = 0; i < m->numButtons; i++) {
		const EobMenuButtonDef *df = &_vm->_menuButtonDefs[m->firstButtonStrId + i];
		Button *b = new Button;
		b->index = m->firstButtonStrId + i + 1;		
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

void GUI_Eob::drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill) {
	if (!b)
		return;
	
	EobMenuButtonDef *d = (EobMenuButtonDef*)b->extButtonDef;

	if (d->flags & 1)
		drawMenuButtonBox(b->x, b->y, b->width, b->height, clicked, noFill);

	if (d->labelId) {
		const char *s = getMenuString(d->labelId);
		
		int xOffs = 4;
		int yOffs = 3;

		if (d->flags & 4) {
			xOffs = ((b->width - (strlen(s) << 3)) >> 1) + 1;
			yOffs = (b->height - 7) >> 1;
		}

		if (noFill || clicked)
			_screen->printText(s, b->x + xOffs, b->y + yOffs, highlight ? 6 : 15, 0);
		else
			_screen->printShadedText(s, b->x + xOffs, b->y + yOffs, highlight ? 6 : 15, 0);
	}
}

void GUI_Eob::drawMenuButtonBox(int x, int y, int w, int h, bool clicked, bool noFill) {
	uint8 col1 = _vm->_color1_1;
	uint8 col2 = _vm->_color2_1;
	
	if (clicked)
		col1 = col2 = _vm->_bkgColor_1;

	_vm->gui_drawBox(x, y, w, h, col1, col2, -1);
	_vm->gui_drawBox(x + 1, y + 1, w - 2, h - 2, _vm->_color1_1, _vm->_color2_1, noFill ? -1 : _vm->_bkgColor_1);
}

void GUI_Eob::displayTextBox(int id) {

}

void GUI_Eob::updateOptionsStrings() {
	for (int i = 0; i < 4; i++) {
		delete _menuStringsPrefsTemp[i];
		_menuStringsPrefsTemp[i] = new char[strlen(_vm->_menuStringsPrefs[i]) + 8];
	}

	Common::strlcpy(_menuStringsPrefsTemp[0], Common::String::format(_vm->_menuStringsPrefs[0], _vm->_menuStringsOnOff[_vm->_configMusic ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[0]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[1], Common::String::format(_vm->_menuStringsPrefs[1], _vm->_menuStringsOnOff[_vm->_configSounds ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[1]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[2], Common::String::format(_vm->_menuStringsPrefs[2], _vm->_menuStringsOnOff[_vm->_configHpBarGraphs ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[2]) + 8);
	Common::strlcpy(_menuStringsPrefsTemp[3], Common::String::format(_vm->_menuStringsPrefs[3], _vm->_menuStringsOnOff[_vm->_configMouse ? 0 : 1]).c_str(), strlen(_vm->_menuStringsPrefs[3]) + 8);
}

const char *GUI_Eob::getMenuString(int id) {
	if (id >= 69)
		return _vm->_menuStringsTransfer[id - 69];
	else if (id >= 67)
		return _vm->_menuStringsDefeat[id - 67];
	else if (id >= 63)
		return _vm->_menuStringsSpec[id - 63];
	else if (id >= 60)
		return _vm->_menuStringsSpellNo[id - 60];
	else if (id == 59)
		return _vm->_menuStringsPoison[0];
	else if (id >= 56)
		return _vm->_menuStringsHead[id - 56];
	else if (id >= 53)
		return _vm->_menuStringsDrop2[id - 53];
	else if (id >= 48)
		return _vm->_menuStringsScribe[id - 48];
	else if (id == 47)
		_vm->_menuStringsStarve[0];
	else if (id == 46)
		_vm->_menuStringsExit[0];
	else if (id == 45)
		_vm->_menuStringsDrop[0];
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
	return 0;
}

Button *GUI_Eob::linkButton(Button *list, Button *newbt) {
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

void GUI_Eob::releaseButtons(Button *list) {
	while (list) {
		Button *n = list->nextButton;
		delete list;
		list = n;
	}
}

#endif // ENABLE_EOB

}	// End of namespace Kyra

#endif // defined(ENABLE_EOB) || defined(ENABLE_LOL)

