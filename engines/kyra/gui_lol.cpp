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
#include "kyra/gui_lol.h"

namespace Kyra {

void LoLEngine::gui_drawPlayField() {
	_screen->loadBitmap("PLAYFLD.CPS", 3, 3, 0);

	if (_screen->_drawGuiFlag & 0x4000) {
		// copy compass shape
		static const int cx[] = { 112, 152, 224 };
		_screen->copyRegion(cx[_lang], 32, 288, 0, 32, 32, 2, 2, Screen::CR_NO_P_CHECK);
		_compassDirection = -1;
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
		resetLampStatus();

	gui_drawScene(2);

	gui_drawAllCharPortraitsWithStats();
	gui_drawInventory();
	gui_drawMoneyBox(_screen->_curPage);

	_screen->setCurPage(cp);
	_screen->hideMouse();
	_screen->copyPage(2, 0);
	_screen->showMouse();
}

void LoLEngine::gui_drawScene(int pageNum) {
	if (!(_updateFlags & 1) && _weaponsDisabled == false && _unkDrawLevelBool && _vcnBlocks)
		drawScene(pageNum);
}

void LoLEngine::gui_drawInventory() {
	if (!_hideControls || !_hideInventory) {
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
	if (_inventory[index])
		_screen->drawShape(_screen->_curPage, getItemIconShapePtr(_inventory[index]), x + 1, 180, 0, 0);
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

void LoLEngine::gui_highlightSelectedSpell(int unk) {

}

void LoLEngine::gui_drawAllCharPortraitsWithStats() {
	int numChars = countActiveCharacters();
	if (!numChars)
		return;

	for (int i = 0; i < numChars; i++)
		gui_drawCharPortraitWithStats(i);
}

void LoLEngine::gui_drawCharPortraitWithStats(int charNum) {
	if (!(_characters[charNum].flags & 1) || _updateFlags & 2)
		return;

	Screen::FontId tmpFid = _screen->setFont(Screen::FID_6_FNT);
	int cp = _screen->setCurPage(6);

	gui_drawBox(0, 0, 66, 34, 1, 1, -1);
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
		int handIndex = 0;
		if (_characters[charNum].items[0]) {
			if (_itemProperties[_itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex].unk8 != -1)
				handIndex = _itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex;
		}

		handIndex =  _gameShapeMap[_itemProperties[handIndex].shpIndex << 1];
		if (handIndex == 0x5a) { // draw raceClassSex specific hand shape
			handIndex = _characters[charNum].raceClassSex - 1;
			if (handIndex < 0)
				handIndex = 0;
			handIndex += 68;
		}

		// draw hand/weapon
		_screen->drawShape(_screen->_curPage, _gameShapes[handIndex], 44, 0, 0, 0);
		// draw magic symbol
		_screen->drawShape(_screen->_curPage, _gameShapes[72 + _characters[charNum].field_41], 44, 17, 0, 0);

		if (spellLevels == 0)
			_screen->drawGridBox(44, 17, 22, 15, 1);
	}

	uint16 f = _characters[charNum].flags & 0x314C;
	if ((f == 0 && _weaponsDisabled) || (f && (f != 4 || _characters[charNum].weaponHit == 0 || _weaponsDisabled)))
		_screen->drawGridBox(44, 0, 22, 34, 1);

	if (_characters[charNum].weaponHit) {
		_screen->drawShape(_screen->_curPage, _gameShapes[34], 44, 0, 0, 0);
		_screen->fprintString("%d", 57, 7, 254, 0, 1, _characters[charNum].weaponHit);
	}
	if (_characters[charNum].damageSuffered)
		_screen->fprintString("%d", 17, 28, 254, 0, 1, _characters[charNum].damageSuffered);

	if (!cp)
		_screen->hideMouse();

	uint8 col = (charNum != _selectedCharacter || countActiveCharacters() == 1) ? 1 : 212;
	_screen->drawBox(0, 0, 65, 33, col);

	_screen->copyRegion(0, 0, _activeCharsXpos[charNum], 143, 66, 34, _screen->_curPage, cp, Screen::CR_NO_P_CHECK);

	if (!cp)
		_screen->showMouse();

	_screen->setCurPage(cp);
	_screen->setFont(tmpFid);
}

void LoLEngine::gui_drawBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor) {
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

void LoLEngine::gui_highlightPortraitFrame(int charNum) {
	if (charNum != _selectedCharacter) {
		int o = _selectedCharacter;
		_selectedCharacter = charNum;
		gui_drawCharPortraitWithStats(o);
	}

	gui_drawCharPortraitWithStats(charNum);
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

	if (_compassDirection == -1) {
		_compassDirectionIndex = -1;
		_compassDirection = _currentDirection << 6;
	}

	int t = ((_compassDirection + 4) >> 3) & 0x1f;

	if (t == _compassDirectionIndex)
		return;

	_compassDirectionIndex = t;

	if (!_screen->_curPage)
		_screen->hideMouse();

	const CompassDef *c = &_compassDefs[t];

	_screen->drawShape(_screen->_curPage, _gameShapes[22 + _lang], 294, 3, 0, 0);
	_screen->drawShape(_screen->_curPage, _gameShapes[25 + c->shapeIndex], 298 + c->x, c->y + 9, 0, c->flags | 0x300, _screen->_paletteOverlay1, 1);
	_screen->drawShape(_screen->_curPage, _gameShapes[25 + c->shapeIndex], 299 + c->x, c->y + 8, 0, c->flags);

	if (!_screen->_curPage)
		_screen->showMouse();
}

int LoLEngine::gui_enableControls() {
	_floatingMouseArrowControl = 0;

	if (!_hideControls) {
		for (int i = 76; i < 85; i++)
			gui_toggleButtonDisplayMode(i, 2);
	}

	gui_toggleFightButtons(false);
	return 1;
}

int LoLEngine::gui_disableControls(int controlMode) {
	if (_hideControls)
		return 0;

	_floatingMouseArrowControl = (controlMode & 2) ? 2 : 1;

	gui_toggleFightButtons(true);

	for (int i = 76; i < 85; i++)
		gui_toggleButtonDisplayMode(i, ((controlMode & 2) && (i > 78)) ? 2 : 3);

	return 1;
}

void LoLEngine::gui_toggleButtonDisplayMode(int shapeIndex, int mode) {
	static const int16 buttonX[] = { 0x0056, 0x0128, 0x000C, 0x0021, 0x0122, 0x000C, 0x0021, 0x0036, 0x000C, 0x0021, 0x0036 };
	static const int16 buttonY[] = { 0x00B4, 0x00B4, 0x00B4, 0x00B4, 0x0020, 0x0084, 0x0084, 0x0084, 0x0096, 0x0096, 0x0096 };

	if (shapeIndex == 78 && !(_screen->_drawGuiFlag & 0x1000))
		return;

	if (_hideControls && _hideInventory)
		return;

	if (mode == 0)
		shapeIndex = _lastButtonShape;

	int pageNum = 0;

	int16 x1 = buttonX[shapeIndex - 74];
	int16 y1 = buttonY[shapeIndex - 74];
	int16 x2 = 0;
	int16 y2 = 0;
	uint32 t = 0;

	switch (mode) {
		case 1:
			mode = 0x100;
			_lastButtonShape = shapeIndex;
			break;

		case 0:
			if (!_lastButtonShape)
				return;

			t = _system->getMillis();
			if (_buttonPressTimer > t)
				delay(_buttonPressTimer - t);

		case 2:
			mode = 0;
			_lastButtonShape = 0;
			break;

		case 3:
			mode = 0;
			_lastButtonShape = 0;
			pageNum = 6;

			x2 = x1;
			y2 = y1;
			x1 = 0;
			y1 = 0;
			break;

		default:
			break;
	}

	_screen->drawShape(pageNum, _gameShapes[shapeIndex], x1, y1, 0, mode, _screen->_paletteOverlay1, 1);

	if (!pageNum)
		_screen->updateScreen();

	if (pageNum == 6) {
		int cp = _screen->setCurPage(6);

		_screen->drawGridBox(x1, y1, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], 1);
		_screen->copyRegion(x1, y1, x2, y2, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], pageNum, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		_screen->setCurPage(cp);
	}

	_buttonPressTimer = _system->getMillis() + 6 * _tickLength;
}

void LoLEngine::gui_toggleFightButtons(bool disable) {
	for (int i = 0; i < 3; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		if (disable)
			_characters[i].flags |= 0x2000;
		else
			_characters[i].flags &= 0xdfff;

		if (disable && !textEnabled()) {
			int u = _selectedCharacter;
			_selectedCharacter = 99;
			int f = _updateFlags;
			_updateFlags &= 0xfffd;

			gui_drawCharPortraitWithStats(i);

			_updateFlags = f;
			_selectedCharacter = u;
		} else {
			gui_drawCharPortraitWithStats(i);
		}
	}
}

void LoLEngine::gui_updateInput() {
	int inputFlag = checkInput(_activeButtons, true);
	removeInputTop();

	if (inputFlag && _unkCharNum != -1 && !(inputFlag & 0x8800)) {
		gui_enableDefaultPlayfieldButtons();
		_characters[_unkCharNum].flags &= 0xffef;
		gui_drawCharPortraitWithStats(_unkCharNum);
		//processMouseButtonEvent(inputFlag);
		_unkCharNum = -1;
		inputFlag = 0;
	}

	if (inputFlag == 1) {
		if (_weaponsDisabled || _availableSpells[1] == -1)
			return;

		gui_highlightSelectedSpell(0);
		if (_availableSpells[++_selectedSpell] == -1)
			_selectedSpell = 0;
		gui_highlightSelectedSpell(1);

		gui_drawAllCharPortraitsWithStats();
	} else if (inputFlag == 3) {
		// TODO
		//processPortraitGuiText
	} else {
		snd_dialogueSpeechUpdate(1);
	}
}

void LoLEngine::gui_enableDefaultPlayfieldButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList1);
	gui_initCharacterControlButtons(7, 44);
	gui_initCharacterControlButtons(11, 44);
	gui_initCharacterControlButtons(17, 0);
	gui_initCharacterControlButtons(29, 0);
	gui_initCharacterControlButtons(25, 33);

	if (_screen->_drawGuiFlag & 0x2000)
		gui_initMagicScrollButtons();
}

void LoLEngine::gui_enableSequenceButtons(int x, int y, int w, int h, int enableFlags) {
	gui_resetButtonList();

	_sceneWindowButton.x = x;
	_sceneWindowButton.y = y;
	_sceneWindowButton.w = w;
	_sceneWindowButton.h = h;

	gui_initButtonsFromList(_buttonList3);

	if (enableFlags & 1)
		gui_initButtonsFromList(_buttonList4);

	if (enableFlags & 2)
		gui_initButtonsFromList(_buttonList5);
}

void LoLEngine::gui_resetButtonList() {
	while (_activeButtons) {
		Button *n = _activeButtons->nextButton;
		delete _activeButtons;
		_activeButtons = n;
	}

	_activeButtons = 0;
}

void LoLEngine::gui_initButtonsFromList(const int16 *list) {
	while (*list != -1)
		gui_initButton(*list++);
}

void LoLEngine::gui_initCharacterControlButtons(int index, int xOffs) {
	int c = countActiveCharacters();
	for (int i = 0; i < c; i++)
		gui_initButton(index + i, _activeCharsXpos[i] + xOffs);
}

void LoLEngine::gui_initMagicScrollButtons() {

}

void LoLEngine::gui_initButton(int index, int x) {
	Button *b = new Button;
	memset (b, 0, sizeof(Button));

	int cnt = 1;

	if (_activeButtons) {
		cnt++;
		Button *n = _activeButtons;

		while (n->nextButton) {
			n = n->nextButton;
			cnt++;
		}

		n->nextButton = b;

	} else {
		_activeButtons = b;
	}

	b->data0Val2 = 0xfe;
	b->data0Val3 = 0x01;
	b->data1Val2 = 0xfe;
	b->data1Val3 = 0x01;
	b->data2Val2 = 0xfe;
	b->data2Val3 = 0x01;

	b->index = cnt;
	b->keyCode = _buttonData[index].keyCode;
	b->keyCode2 = _buttonData[index].keyCode2;
	b->dimTableIndex = _buttonData[index].screenDim;
	b->flags = _buttonData[index].buttonflags;

	b->data2Val2 = _buttonData[index].index;

	if (index == 64) {
		// scene window button
		b->x = _sceneWindowButton.x;
		b->y = _sceneWindowButton.y;
		b->width = _sceneWindowButton.w - 1;
		b->height = _sceneWindowButton.h - 1;
	} else {
		b->x = x != -1 ? x : _buttonData[index].x;
		b->y = _buttonData[index].y;
		b->width = _buttonData[index].w - 1;
		b->height = _buttonData[index].h - 1;
	}

	assignButtonCallback(b, index);
}

int LoLEngine::clickedUpArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	moveParty(_currentDirection, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 0, 80);

	return 1;
}

int LoLEngine::clickedDownArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	moveParty(_currentDirection ^ 2, 0, 1, 83);

	return 1;
}

int LoLEngine::clickedLeftArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	moveParty((_currentDirection - 1) & 3, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 2, 82);

	return 1;
}

int LoLEngine::clickedRightArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	moveParty((_currentDirection + 1) & 3, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 3, 84);

	return 1;
}

int LoLEngine::clickedTurnLeftArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	gui_toggleButtonDisplayMode(79, 1);
	_currentDirection = (--_currentDirection) & 3;

	_sceneDefaultUpdate = 1;

	runSceneScript(_currentBlock, 0x4000);
	initTextFading(2, 0);

	if (!_sceneDefaultUpdate)
		gui_drawScene(0);
	else
		movePartySmoothScrollTurnLeft(1);

	gui_toggleButtonDisplayMode(79, 0);
	runSceneScript(_currentBlock, 0x10);
	return 1;
}

int LoLEngine::clickedTurnRightArrow(Button *button) {
	if (button->data2Val2 && !(_unkGameFlag & 4))
		return 0;

	gui_toggleButtonDisplayMode(81, 1);
	_currentDirection = (++_currentDirection) & 3;

	_sceneDefaultUpdate = 1;

	runSceneScript(_currentBlock, 0x4000);
	initTextFading(2, 0);

	if (!_sceneDefaultUpdate)
		gui_drawScene(0);
	else
		movePartySmoothScrollTurnRight(1);

	gui_toggleButtonDisplayMode(81, 0);
	runSceneScript(_currentBlock, 0x10);

	return 1;
}

int LoLEngine::clickedAttackButton(Button *button) {
	return 1;
}

int LoLEngine::clickedMagicButton(Button *button) {
	if (_characters[button->data2Val2].flags & 0x314C)
		return 1;

	if (notEnoughMagic(button->data2Val2, _availableSpells[_selectedSpell], 0))
		return 1;

	_characters[button->data2Val2].flags ^= 0x10;

	gui_drawCharPortraitWithStats(button->data2Val2);
	spellsub2(button->data2Val2);
	_unkCharNum = button->data2Val2;

	return 1;
}

int LoLEngine::clickedUnk9(Button *button) {
	return 1;
}

int LoLEngine::clickedScreen(Button *button) {
	return 1;
}

int LoLEngine::clickedPortraitLeft(Button *button) {
	return 1;
}

int LoLEngine::clickedLiveMagicBarsLeft(Button *button) {
	gui_highlightPortraitFrame(button->data2Val2);
	_txt->printMessage(0, getLangString(0x4047), _characters[button->data2Val2].name, _characters[button->data2Val2].hitPointsCur,
		_characters[button->data2Val2].hitPointsMax, _characters[button->data2Val2].magicPointsCur, _characters[button->data2Val2].magicPointsMax);
	return 1;
}

int LoLEngine::clickedPortraitEtcRight(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk14(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk15(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk16(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk17(Button *button) {
	return 1;
}

int LoLEngine::clickedInventorySlot(Button *button) {
	return 1;
}

int LoLEngine::clickedInventoryScroll(Button *button) {
	int8 dir = (int8) button->data2Val2;
	int shp = (dir == 1) ? 75 : 74;

	gui_toggleButtonDisplayMode(shp, 1);

	gui_toggleButtonDisplayMode(shp, 0);
	return 1;
}

int LoLEngine::clickedUnk20(Button *button) {
	return 1;
}

int LoLEngine::clickedScene(Button *button) {
	return 1;
}

int LoLEngine::clickedScroll(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk23(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk24(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk25(Button *button) {
	return 1;
}

int LoLEngine::clickedOptions(Button *button) {
	gui_toggleButtonDisplayMode(76, 1);

	gui_toggleButtonDisplayMode(76, 0);

	return 1;
}

int LoLEngine::clickedRestParty(Button *button) {
	gui_toggleButtonDisplayMode(77, 1);

	gui_toggleButtonDisplayMode(77, 0);

	return 1;
}

int LoLEngine::clickedMoneyBox(Button *button) {
	_txt->printMessage(0, getLangString(_credits == 1 ? 0x402D : 0x402E), _credits);
	return 1;
}

int LoLEngine::clickedCompass(Button *button) {
	return 1;
}

int LoLEngine::clickedAutomap(Button *button) {
	return 1;
}

int LoLEngine::clickedLamp(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk32(Button *button) {
	return 1;
}

GUI_LoL::GUI_LoL(LoLEngine *vm) : GUI(vm), _vm(vm), _screen(vm->_screen) {
	_scrollUpFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollUp);
	_scrollDownFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollDown);
	_unknownButtonList = _backUpButtonList = 0;
	_flagsModifier = 0;
	_buttonListChanged = false;
}

void GUI_LoL::processButton(Button *button) {
	if (!button)
		return;

	if (button->flags & 8) {
		if (button->flags & 0x10) {
			// XXX
		}
		return;
	}

	int entry = button->flags2 & 5;

	byte val1 = 0, val2 = 0, val3 = 0;
	const uint8 *dataPtr = 0;
	Button::Callback callback;
	if (entry == 1) {
		val1 = button->data1Val1;
		dataPtr = button->data1ShapePtr;
		callback = button->data1Callback;
		val2 = button->data1Val2;
		val3 = button->data1Val3;
	} else if (entry == 4 || entry == 5) {
		val1 = button->data2Val1;
		dataPtr = button->data2ShapePtr;
		callback = button->data2Callback;
		val2 = button->data2Val2;
		val3 = button->data2Val3;
	} else {
		val1 = button->data0Val1;
		dataPtr = button->data0ShapePtr;
		callback = button->data0Callback;
		val2 = button->data0Val2;
		val3 = button->data0Val3;
	}

	int x = 0, y = 0, x2 = 0, y2 = 0;

	x = button->x;
	if (x < 0)
		x += _screen->getScreenDim(button->dimTableIndex)->w << 3;
	x += _screen->getScreenDim(button->dimTableIndex)->sx << 3;
	x2 = x + button->width - 1;

	y = button->y;
	if (y < 0)
		y += _screen->getScreenDim(button->dimTableIndex)->h << 3;
	y += _screen->getScreenDim(button->dimTableIndex)->sy << 3;
	y2 = y + button->height - 1;

	switch (val1 - 1) {
	case 0:
		_screen->hideMouse();
		_screen->drawShape(_screen->_curPage, dataPtr, x, y, button->dimTableIndex, 0x10);
		_screen->showMouse();
		break;

	case 1:
		_screen->hideMouse();
		_screen->printText((const char*)dataPtr, x, y, val2, val3);
		_screen->showMouse();
		break;

	case 3:
		if (callback)
			(*callback)(button);
		break;

	case 4:
		_screen->hideMouse();
		_screen->drawBox(x, y, x2, y2, val2);
		_screen->showMouse();
		break;

	case 5:
		_screen->hideMouse();
		_screen->fillRect(x, y, x2, y2, val2, -1, true);
		_screen->showMouse();
		break;

	default:
		break;
	}

	_screen->updateScreen();
}

int GUI_LoL::processButtonList(Button *buttonList, uint16 inputFlag, int8 mouseWheel) {
	if (!buttonList)
		return inputFlag & 0x7FFF;

	if (_backUpButtonList != buttonList || _buttonListChanged) {
		_unknownButtonList = 0;
		//flagsModifier |= 0x2200;
		_backUpButtonList = buttonList;
		_buttonListChanged = false;

		while (buttonList) {
			processButton(buttonList);
			buttonList = buttonList->nextButton;
		}
	}

	int mouseX = _vm->_mouseX;
	int mouseY = _vm->_mouseY;

	uint16 flags = 0;

	if (1/*!_screen_cursorDisable*/) {
		uint16 inFlags = inputFlag & 0xFF;
		uint16 temp = 0;

		// HACK: inFlags == 200 is our left button (up)
		if (inFlags == 199 || inFlags == 200)
			temp = 0x100;
		if (inFlags == 201 || inFlags == 202)
			temp = 0x1000;

		if (inputFlag & 0x800)
			temp <<= 2;

		flags |= temp;

		_flagsModifier &= ~((temp & 0x4400) >> 1);
		_flagsModifier |= (temp & 0x1100) * 2;
		flags |= _flagsModifier;
		flags |= (_flagsModifier << 2) ^ 0x8800;
	}

	buttonList = _backUpButtonList;
	if (_unknownButtonList) {
		buttonList = _unknownButtonList;
		if (_unknownButtonList->flags & 8)
			_unknownButtonList = 0;
	}

	int returnValue = 0;
	while (buttonList) {
		if (buttonList->flags & 8) {
			buttonList = buttonList->nextButton;
			continue;
		}
		buttonList->flags2 &= ~0x18;
		buttonList->flags2 |= (buttonList->flags2 & 3) << 3;

		int x = buttonList->x;
		if (x < 0)
			x += _screen->getScreenDim(buttonList->dimTableIndex)->w << 3;
		x += _screen->getScreenDim(buttonList->dimTableIndex)->sx << 3;

		int y = buttonList->y;
		if (y < 0)
			y += _screen->getScreenDim(buttonList->dimTableIndex)->h;
		y += _screen->getScreenDim(buttonList->dimTableIndex)->sy;

		bool progress = false;

		if (mouseX >= x && mouseY >= y && mouseX <= x+buttonList->width && mouseY <= y+buttonList->height)
			progress = true;

		buttonList->flags2 &= ~0x80;
		uint16 inFlags = inputFlag & 0x7FFF;
		if (inFlags) {
			if (buttonList->keyCode == inFlags) {
				progress = true;
				flags = buttonList->flags & 0x0F00;
				buttonList->flags2 |= 0x80;
				inputFlag = 0;
				_unknownButtonList = buttonList;
			} else if (buttonList->keyCode2 == inFlags) {
				flags = buttonList->flags & 0xF000;
				if (!flags)
					flags = buttonList->flags & 0x0F00;
				progress = true;
				buttonList->flags2 |= 0x80;
				inputFlag = 0;
				_unknownButtonList = buttonList;
			}
		}

		bool unk1 = false;

		if (mouseWheel && buttonList->mouseWheel == mouseWheel) {
			progress = true;
			unk1 = true;
		}

		if (!progress)
			buttonList->flags2 &= ~6;

		if ((flags & 0x3300) && (buttonList->flags & 4) && progress && (buttonList == _unknownButtonList || !_unknownButtonList)) {
			buttonList->flags |= 6;
			if (!_unknownButtonList)
				_unknownButtonList = buttonList;
		} else if ((flags & 0x8800) && !(buttonList->flags & 4) && progress) {
			buttonList->flags2 |= 6;
		} else {
			buttonList->flags2 &= ~6;
		}

		bool progressSwitch = false;
		if (!_unknownButtonList) {
			progressSwitch = progress;
		} else  {
			if (_unknownButtonList->flags & 0x40)
				progressSwitch = (_unknownButtonList == buttonList);
			else
				progressSwitch = progress;
		}

		if (progressSwitch) {
			if ((flags & 0x1100) && progress && !_unknownButtonList) {
				inputFlag = 0;
				_unknownButtonList = buttonList;
			}

			if ((buttonList->flags & flags) && (progress || !(buttonList->flags & 1))) {
				uint16 combinedFlags = (buttonList->flags & flags);
				combinedFlags = ((combinedFlags & 0xF000) >> 4) | (combinedFlags & 0x0F00);
				combinedFlags >>= 8;

				static const uint16 flagTable[] = {
					0x000, 0x100, 0x200, 0x100, 0x400, 0x100, 0x400, 0x100, 0x800, 0x100,
					0x200, 0x100, 0x400, 0x100, 0x400, 0x100
				};

				assert(combinedFlags < ARRAYSIZE(flagTable));

				switch (flagTable[combinedFlags]) {
				case 0x400:
					if (!(buttonList->flags & 1) || ((buttonList->flags & 1) && _unknownButtonList == buttonList)) {
						buttonList->flags2 ^= 1;
						returnValue = buttonList->index | 0x8000;
						unk1 = true;
					}

					if (!(buttonList->flags & 4)) {
						buttonList->flags2 &= ~4;
						buttonList->flags2 &= ~2;
					}
					break;

				case 0x800:
					if (!(buttonList->flags & 4)) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}

					if (!(buttonList->flags & 1))
						unk1 = true;
					break;

				case 0x200:
					if (buttonList->flags & 4) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}

					if (!(buttonList->flags & 1))
						unk1 = true;
					break;

				case 0x100:
				default:
					buttonList->flags2 ^= 1;
					returnValue = buttonList->index | 0x8000;
					unk1 = true;
					if (buttonList->flags & 4) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}
					_unknownButtonList = buttonList;
					break;
				}
			}
		}

		bool unk2 = false;
		if ((flags & 0x2200) && progress) {
			buttonList->flags2 |= 6;
			if (!(buttonList->flags & 4) && !(buttonList->flags2 & 1)) {
				unk2 = true;
				buttonList->flags2 |= 1;
			}
		}

		if ((flags & 0x8800) == 0x8800) {
			_unknownButtonList = 0;
			if (!progress || (buttonList->flags & 4))
				buttonList->flags2 &= ~6;
		}

		if (!progress && buttonList == _unknownButtonList && !(buttonList->flags & 0x40))
			_unknownButtonList = 0;

		if ((buttonList->flags2 & 0x18) != ((buttonList->flags2 & 3) << 3))
			processButton(buttonList);

		if (unk2)
			buttonList->flags2 &= ~1;

		if (unk1) {
			buttonList->flags2 &= 0xFF;
			buttonList->flags2 |= flags;

			if (buttonList->buttonCallback) {
				_vm->removeInputTop();
				if ((*buttonList->buttonCallback.get())(buttonList))
					break;
			}

			if (buttonList->flags & 0x20)
				break;
		}

		if (_unknownButtonList == buttonList && (buttonList->flags & 0x40))
			break;

		buttonList = buttonList->nextButton;
	}

	if (!returnValue)
		returnValue = inputFlag & 0x7FFF;
	return returnValue;
}

} // end of namespace Kyra

