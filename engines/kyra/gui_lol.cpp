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
 * $URL$
 * $Id$
 *
 */

#include "kyra/lol.h"
#include "kyra/screen_lol.h"

namespace Kyra {

void LoLEngine::gui_drawPlayField() {
	_screen->loadBitmap("PLAYFLD.CPS", 3, 3, 0);

	if (_screen->_drawGuiFlag & 0x4000) {
		// copy compass shape
		static const int cx[] = { 112, 152, 224 };
		_screen->copyRegion(cx[_lang], 32, 288, 0, 32, 32, 2, 2, Screen::CR_NO_P_CHECK);
		_updateUnk2 = -1;
	}

	if (_screen->_drawGuiFlag & 0x1000)
		// draw automap book
		_screen->drawShape(2, _gameShapes[78], 289, 32, 0, 0);

	int cp = _screen->setCurPage(2);

	if (_screen->_drawGuiFlag & 0x2000) {
		gui_drawScroll();
	} else {
		_selectedSpell = 0;
	}

	if (_screen->_drawGuiFlag & 0x800)
		turnOnLamp();

	//mouseDimUnk()
	gui_drawScene(2);

	gui_drawAllCharPortraitsWithStats();
	gui_drawInventory();
	gui_drawMoneyBox(_screen->_curPage);
	_screen->setCurPage(cp);
	_screen->hideMouse();
	_screen->copyPage(2, 0);
	//mouseDimUnk
	_screen->showMouse();
}

void LoLEngine::gui_drawScene(int pageNum) {
	if (/*_charFlagUnk == 1 && */_weaponsDisabled == false && _unkDrawLevelBool && _vcnBlocks)
		drawScene(pageNum);
}

void LoLEngine::gui_drawInventory() {
	if (!_unkInventFlag || !_updateCharV6) {
		for (int i = 0; i < 9; i++)
			gui_drawInventoryItem(i);
	}
}

void LoLEngine::gui_drawInventoryItem(int index) {
	static const uint16 inventoryXpos[] = { 0x6A, 0x7F, 0x94, 0xA9, 0xBE, 0xD3, 0xE8, 0xFD, 0x112 };
	int x = inventoryXpos[index];
	int item = _inventoryCurItem + index;
	if (item > 48)
		item -= 48;

	int flag = item & 1 ? 0 : 1;

	_screen->hideMouse();
	_screen->drawShape(_screen->_curPage, _gameShapes[4], x, 179, 0, flag);
	if (_inventoryItemIndex[index])
		_screen->drawShape(_screen->_curPage, getItemIconShapePtr(_inventoryItemIndex[index]), x + 1, 180, 0, 0);
	_screen->showMouse();
}

void LoLEngine::gui_drawScroll() {
	_screen->copyRegion(112, 0, 12, 0, 87, 15, 2, 2, Screen::CR_NO_P_CHECK);
	int h = 0;
	
	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] != -1)
			h += 9;
	}

	if (h == 18)
		h = 27;

	if (h) {		
		_screen->copyRegion(201, 1, 17, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(208, 1, 89, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->fillRect(21, 15, 89, h + 15, 206);
	}

	_screen->copyRegion(112, 16, 12, h + 15, 87, 14, 2, 2, Screen::CR_NO_P_CHECK);

	int y = 15;
	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] == -1)
			continue;
		uint8 col = (i == _selectedSpell) ? 132 : 1;
		_screen->fprintString(getLangString(_spellProperties[_availableSpells[i]].spellNameCode), 24, y, col, 0, 0);
		y += 9;
	}
}

void LoLEngine::gui_drawAllCharPortraitsWithStats() {
	int numChars = countActiveCharacters();
	if (!numChars)
		return;

	for (int i = 0; i < numChars; i++)
		gui_drawCharPortraitWithStats(i);
}

void LoLEngine::gui_drawCharPortraitWithStats(int charNum) {
	if (!(_characters[charNum].flags & 1) || _charFlagUnk & 2)
		return;

	Screen::FontId tmpFid = _screen->setFont(Screen::FID_6_FNT);
	int cp = _screen->setCurPage(6);

	gui_drawPortraitBox(0, 0, 66, 34, 1, 1, -1);
	gui_drawCharFaceShape(0, 0, 1, _screen->_curPage);

	gui_drawLiveMagicBar(33, 32, _characters[charNum].magicPointsCur, 0, _characters[charNum].magicPointsMax, 5, 32, 162, 1, 0);
	gui_drawLiveMagicBar(39, 32, _characters[charNum].hitPointsCur, 0, _characters[charNum].hitPointsMax, 5, 32, 154, 1, 1);

	_screen->printText(getLangString(0x4253), 33, 1, 160, 0);
	_screen->printText(getLangString(0x4254), 39, 1, 152, 0);

	int spellLevels = 0;
	if (_availableSpells[_selectedSpell] != -1) {
		for (int i = 0; i < 4; i++) {
			if (_spellProperties[_availableSpells[_selectedSpell]].mpRequired[i] <= _characters[charNum].magicPointsCur &&
				_spellProperties[_availableSpells[_selectedSpell] + 1].unkArr[i] <= _characters[charNum].hitPointsCur)
					spellLevels++;
		}
	}

	if (_characters[charNum].flags & 0x10) {
		// magic submenu open
		_screen->drawShape(_screen->_curPage, _gameShapes[73], 44, 0, 0, 0);
		if (spellLevels < 4)
			_screen->drawGridBox(44, (spellLevels << 3) + 1, 22, 32 - (spellLevels << 3), 1);
	} else {
		// magic submenu closed
		int shapeNum = -1;		
		/*if (_characters[charNum].items[0]) {
			int u8 = _itemProperties[_itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex].unk8 & 0xff;
			if (u8 > shapeNum)
				shapeNum = u8;
		}*/

		shapeNum = _gameShapeMap[_itemProperties[_itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex].shpIndex];
		if (shapeNum == 0x5a) { // draw raceClassSex specific hand shape
			shapeNum = _characters[charNum].raceClassSex - 1;
			if (shapeNum < 0)
				shapeNum = 0;
			shapeNum += 68;
		}
		// draw hand/weapon
		_screen->drawShape(_screen->_curPage, _gameShapes[shapeNum], 44, 0, 0, 0);
		// draw magic symbol
		_screen->drawShape(_screen->_curPage, _gameShapes[72 + _characters[charNum].field_41], 44, 17, 0, 0);

		if (spellLevels == 0)
			_screen->drawGridBox(44, 17, 22, 15, 1);
	}	

	uint16 f = _characters[charNum].flags & 0x314C;
	if ((f == 0 && (f != 4 || _characters[charNum].weaponHit == 0)) || _weaponsDisabled)
		_screen->drawGridBox(44, 0, 22, 34, 1);

	if (_characters[charNum].weaponHit) {
		_screen->drawShape(_screen->_curPage, _gameShapes[34], 44, 0, 0, 0);
		_screen->fprintString("%d", 57, 7, 254, 0, 1, _characters[charNum].weaponHit);
	}
	if (_characters[charNum].damageSuffered) 
		_screen->fprintString("%d", 17, 28, 254, 0, 1, _characters[charNum].damageSuffered);

	if (!cp)
		_screen->hideMouse();

	uint8 col = (charNum != _unkDrawPortraitIndex || countActiveCharacters() == 1) ? 1 : 212;
	_screen->drawBox(0, 0, 65, 33, col);

	_screen->copyRegion(0, 0, _activeCharsXpos[charNum], 143, 66, 34, _screen->_curPage, cp, Screen::CR_NO_P_CHECK);

	if (!cp)
		_screen->showMouse();

	_screen->setCurPage(cp);
	_screen->setFont(tmpFid);
}

void LoLEngine::gui_drawPortraitBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor) {
	w--; h--;
	if (fillColor != -1)
		_screen->fillRect(x + 1, y + 1, x + w - 1, y + h - 1, fillColor);

	_screen->drawClippedLine(x + 1, y, x + w, y, frameColor2);
	_screen->drawClippedLine(x + w, y, x + w, y + h - 1, frameColor2);
	_screen->drawClippedLine(x, y, x, y + h, frameColor1);
	_screen->drawClippedLine(x, y + h, x + w, y + h, frameColor1);		
}

void LoLEngine::gui_drawCharFaceShape(int charNum, int x, int y, int pageNum) {
	if (_characters[charNum].curFaceFrame < 7 && _characters[charNum].nextFaceFrame)
		_characters[charNum].curFaceFrame = _characters[charNum].nextFaceFrame;

	if (_characters[charNum].nextFaceFrame == 0 && _characters[charNum].curFaceFrame > 1 && _characters[charNum].curFaceFrame < 7)
		_characters[charNum].curFaceFrame = _characters[charNum].nextFaceFrame;

	int frm = (_characters[charNum].flags & 0x1108 && _characters[charNum].curFaceFrame < 7) ? 1 : _characters[charNum].curFaceFrame;

	if (_characters[charNum].hitPointsCur <= (_characters[charNum].hitPointsMax >> 1))
		frm += 14;

	if (!pageNum)
		_screen->hideMouse();

	_screen->drawShape(pageNum, _characterFaceShapes[frm][charNum], x, y, 0, 0x100, _screen->_paletteOverlay2, (_characters[charNum].flags & 0x80 ? 1 : 0));

	if (_characters[charNum].flags & 0x40)
		// draw spider web
		_screen->drawShape(pageNum, _gameShapes[21], x, y, 0, 0);

	if (!pageNum)
		_screen->showMouse();
}

void LoLEngine::gui_drawLiveMagicBar(int x, int y, int curPoints, int unk, int maxPoints, int w, int h, int col1, int col2, int flag) {
	w--;
	h--;

	if (maxPoints < 1)
		return;

	int t = (curPoints < 1) ? 0 : curPoints;
	curPoints = (maxPoints < t) ? maxPoints : t;

	int barHeight = (curPoints * h) / maxPoints;

	if (barHeight < 1 && curPoints < 1)
		barHeight = 1;

	_screen->drawClippedLine(x - 1, y - h, x - 1, y, 1);

	if (flag) {
		t = maxPoints >> 1;
		if (t > curPoints)
			col1 = 144;
		t = maxPoints >> 2;
		if (t > curPoints)
			col1 = 132;
	}

	if (barHeight > 0)
		_screen->fillRect(x, y - barHeight, x + w, y, col1);

	if (barHeight < h)
		_screen->fillRect(x, y - h, x + w, y - barHeight, col2);

	if (unk > 0 && unk < maxPoints)
		_screen->drawBox(x, y - barHeight, x + w, y, col1 - 2);
}

void LoLEngine::calcCharPortraitXpos() {
	int nc = countActiveCharacters();

	// TODO

	int t = (235 - (nc * 66)) / (nc + 1);
	for (int i = 0; i < nc; i++)
		_activeCharsXpos[i] = i * 66 + t * (i + 1) + 83;	
}

void LoLEngine::gui_drawMoneyBox(int pageNum) {
	static const uint16 moneyX[] = { 0x128, 0x134, 0x12b, 0x131, 0x12e};
	static const uint16 moneyY[] = { 0x73, 0x73, 0x74, 0x74, 0x75};

	int backupPage = _screen->_curPage;
	_screen->_curPage = pageNum;

	_screen->fillRect(292, 97, 316, 118, 252, pageNum);

	for (int i = 0; i < 5; i++) {
		if (!_moneyColumnHeight[i])
			continue;

		uint8 h = _moneyColumnHeight[i] - 1;
		_screen->drawClippedLine(moneyX[i], moneyY[i], moneyX[i], moneyY[i] - h, 0xd2);
		_screen->drawClippedLine(moneyX[i] + 1, moneyY[i], moneyX[i] + 1, moneyY[i] - h, 0xd1);
		_screen->drawClippedLine(moneyX[i] + 2, moneyY[i], moneyX[i] + 2, moneyY[i] - h, 0xd0);
		_screen->drawClippedLine(moneyX[i] + 3, moneyY[i], moneyX[i] + 3, moneyY[i] - h, 0xd1);
		_screen->drawClippedLine(moneyX[i] + 4, moneyY[i], moneyX[i] + 4, moneyY[i] - h, 0xd2);
	}

	Screen::FontId backupFont = _screen->setFont(Screen::FID_6_FNT);
	_screen->fprintString("%d", 305, 98, 254, 0, 1, _credits); 

	_screen->setFont(backupFont);
	_screen->_curPage = backupPage;

	if (pageNum == 6) {
		_screen->hideMouse();
		_screen->copyRegion(292, 97, 292, 97, 25, 22, 6, 0);
		_screen->showMouse();
	}
}

void LoLEngine::gui_drawCompass() {
	if (!(_screen->_drawGuiFlag & 0x4000))
		return;

	if (_updateUnk2 == -1) {
		_compassDirectionIndex = -1;
		_updateUnk2 = _currentDirection << 6;
	}

	int t = ((_updateUnk2 + 4) >> 3) & 0x1f;

	if (t == _compassDirectionIndex)
		return;

	_compassDirectionIndex = t;

	if (!_screen->_curPage)
		_screen->hideMouse();

	const CompassDef *c = &_compassDefs[t];

	_screen->drawShape(_screen->_curPage, _gameShapes[22 + _lang], 294, 3, 0, 0);
	_screen->drawShape(_screen->_curPage, _gameShapes[25 + c->shapeIndex], 298 + c->x, c->y + 9, 0, c->flags | 0x300);
	_screen->drawShape(_screen->_curPage, _gameShapes[25 + c->shapeIndex], 299 + c->x, c->y + 8, 0, c->flags);

	if (!_screen->_curPage)
		_screen->showMouse();
}

} // end of namespace Kyra

