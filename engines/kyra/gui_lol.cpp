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
			gui_disableArrowButton(i, 2);
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
		gui_disableArrowButton(i, ((controlMode & 2) && (i > 78)) ? 2 : 3);

	return 1;
}

void LoLEngine::gui_disableArrowButton(int shapeIndex, int mode) {
	static const int16 arrowButtonX[] = { 0x000C, 0x0021, 0x0122, 0x000C, 0x0021, 0x0036, 0x000C, 0x0021, 0x0036 };
	static const int16 arrowButtonY[] = { 0x00B4, 0x00B4, 0x0020, 0x0084, 0x0084, 0x0084, 0x0096, 0x0096, 0x0096 };

	if (shapeIndex == 78 && !(_screen->_drawGuiFlag & 0x1000))
		return;

	if (_hideControls && _hideInventory)
		return;

	if (mode == 0)
		shapeIndex = _lastArrowButtonShape;

	int pageNum = 0;

	int16 x1 = arrowButtonX[shapeIndex - 76];
	int16 y1 = arrowButtonY[shapeIndex - 76];
	int16 x2 = 0;
	int16 y2 = 0;
	uint32 t = 0;

	switch (mode) {
		case 1:
			mode = 0x100;
			_lastArrowButtonShape = shapeIndex;
			break;

		case 0:
			if (!_lastArrowButtonShape)
				return;

			t = _system->getMillis();
			if (_arrowButtonTimer > t)
				delay(_arrowButtonTimer - t);

		case 2:
			mode = 0;
			_lastArrowButtonShape = 0;
			break;

		case 3:
			mode = 0;
			_lastArrowButtonShape = 0;
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
	
	if (pageNum != 6)
		return;

	int cp = _screen->setCurPage(6);

	_screen->drawGridBox(x1, y1, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], 1);
	_screen->copyRegion(x1, y1, x2, y2, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], pageNum, 0, Screen::CR_NO_P_CHECK);

	_screen->setCurPage(cp);

	_arrowButtonTimer = _system->getMillis() + 6 * _tickLength;
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

int LoLEngine::clickedUpArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedDownArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedLeftArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedRightArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedTurnLeftArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedTurnRightArrow(Button *button) {
	return 1;
}

int LoLEngine::clickedAttackButton(Button *button) {
	return 1;
}

int LoLEngine::clickedMagicButton(Button *button) {
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
	return 1;
}

int LoLEngine::clickedUnk20(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk21(Button *button) {
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
	return 1;
}

int LoLEngine::clickedRestParty(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk28(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk29(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk30(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk31(Button *button) {
	return 1;
}

int LoLEngine::clickedUnk32(Button *button) {
	return 1;
}

GUI_LoL::GUI_LoL(LoLEngine *vm) : GUI(vm), _vm(vm), _screen(vm->_screen) {
	memset(_dialogueButtonString, 0, 3 * sizeof(const char*));
	_dialogueButtonPosX = _dialogueButtonPosY = _dialogueNumButtons = _dialogueButtonXoffs = _dialogueHighlightedButton = 0;
	_scrollUpFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollUp);
	_scrollDownFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollDown);
}

int GUI_LoL::processButtonList(Button *list, uint16 inputFlag, int8 mouseWheel) {
	if ((inputFlag & 0xFF) == 199)
		_pressFlag = true;
	else if ((inputFlag & 0xFF) == 200)
		_pressFlag = false;

	int returnValue = 0;
	while (list) {
		/*if (list->flags & 8) {
			list = list->nextButton;
			continue;
		}

		if (mouseWheel && list->mouseWheel == mouseWheel && list->buttonCallback) {
			if ((*list->buttonCallback.get())(list))
				break;
		}

		int x = list->x;
		int y = list->y;
		assert(_screen->getScreenDim(list->dimTableIndex) != 0);

		if (x < 0)
			x += _screen->getScreenDim(list->dimTableIndex)->w << 3;
		x += _screen->getScreenDim(list->dimTableIndex)->sx << 3;

		if (y < 0)
			y += _screen->getScreenDim(list->dimTableIndex)->h;
		y += _screen->getScreenDim(list->dimTableIndex)->sy;

		if (_vm->_mouseX >= x && _vm->_mouseY >= y && x + list->width >= _vm->_mouseX && y + list->height >= _vm->_mouseY) {
			int processMouseClick = 0;
			if (list->flags & 0x400) {
				if ((inputFlag & 0xFF) == 199 || _pressFlag) {
					if (!(list->flags2 & 1)) {
						list->flags2 |= 1;
						list->flags2 |= 4;
						processButton(list);
						_screen->updateScreen();
						inputFlag = 0;
					}
				} else if ((inputFlag & 0xFF) == 200) {
					if (list->flags2 & 1) {
						list->flags2 &= 0xFFFE;
						processButton(list);
						processMouseClick = 1;
						inputFlag = 0;
					}
				}
			}

			if (processMouseClick) {
				if (list->buttonCallback) {
					if ((*list->buttonCallback.get())(list))
						break;
				}
			}
		} else {
			if (list->flags2 & 1) {
				list->flags2 &= 0xFFFE;
				processButton(list);
			}

			if (list->flags2 & 4) {
				list->flags2 &= 0xFFFB;
				processButton(list);
				_screen->updateScreen();
			}
		}

		list = list->nextButton;*/
	}

	if (!returnValue)
		returnValue = inputFlag & 0xFF;

	return returnValue;
}

void GUI_LoL::drawDialogueBox(int numStr, const char *s1, const char *s2, const char *s3) {
	if (numStr == 1 && _vm->_speechFlag) {
		_screen->setScreenDim(5);
		_dialogueButtonString[0] = _dialogueButtonString[1] = _dialogueButtonString[2] = 0;
	} else {
		_screen->setScreenDim(5);
		_dialogueNumButtons = numStr;
		_dialogueButtonString[0] = s1;
		_dialogueButtonString[1] = s2;
		_dialogueButtonString[2] = s3;
		_dialogueHighlightedButton = 0;

		const ScreenDim *d = _screen->getScreenDim(5);
		_dialogueButtonPosY = d->sy + d->h - 9;

		if (numStr == 1) {
			_dialogueButtonXoffs = 0;
			_dialogueButtonPosX = d->sx + d->w - 77;			
		} else {			
			_dialogueButtonXoffs = d->w / numStr;
			_dialogueButtonPosX = d->sx + (_dialogueButtonXoffs >> 1) - 37;
		}

		drawDialogueButtons();
	}

	if (!_vm->shouldQuit())
		_vm->removeInputTop();
}

void GUI_LoL::drawDialogueButtons() {
	int cp = _screen->setCurPage(0);
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	int x = _dialogueButtonPosX;

	for (int i = 0; i < _dialogueNumButtons; i++) {
		_vm->gui_drawBox(x, _dialogueButtonPosY, 74, 9, 136, 251, -1);
		_screen->printText(_dialogueButtonString[i], x + 37 - (_screen->getTextWidth(_dialogueButtonString[i])) / 2,
			_dialogueButtonPosY + 2, _dialogueHighlightedButton == i ? 144 : 254, 0);
		x += _dialogueButtonXoffs;
	}
	_screen->setFont(of);
	_screen->setCurPage(cp);
}

uint16 GUI_LoL::processDialogue() {
	int df = _dialogueHighlightedButton;
	int res = 0;
	int x = _dialogueButtonPosX;

	for (int i = 0; i < _dialogueNumButtons; i++) {
		if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x, _dialogueButtonPosY, x + 74, _dialogueButtonPosY + 9)) {
			_dialogueHighlightedButton = i;
			break;
		}
		x += _dialogueButtonXoffs;
	}

	if (_dialogueNumButtons == 0) {
		int e = _vm->checkInput(0, false) & 0xCF;
		_vm->removeInputTop();
		
		if (e == 200) {
			_vm->snd_dialogueSpeechUpdate(1);
			//_dlgTimer = 0;
		}

		if (_vm->snd_characterSpeaking() != 2) {
			//if (_dlgTimer < _system->getMillis()) {
				res = 1;
				if (!_vm->shouldQuit())
					_vm->removeInputTop();
			//}
		}
	} else {
		int e = _vm->checkInput(0, false);
		_vm->removeInputTop();
		switch (e) {
			case 100:
			case 101:
				_vm->snd_dialogueSpeechUpdate(1);
				//_dlgTimer = 0;
				res = _dialogueHighlightedButton + 1;
				break;

			case 110:
			case 111:
				if (_dialogueNumButtons > 1 && _dialogueHighlightedButton > 0)
					_dialogueHighlightedButton--;
				break;

			case 112:
			case 113:
				if (_dialogueNumButtons > 1 && _dialogueHighlightedButton < (_dialogueNumButtons - 1))
					_dialogueHighlightedButton++;
				break;

			case 200:
			case 300:
				x = _dialogueButtonPosX;
				
				for (int i = 0; i < _dialogueNumButtons; i++) {
					if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x, _dialogueButtonPosY, x + 74, _dialogueButtonPosY + 9)) {
						_dialogueHighlightedButton = i;
						res = _dialogueHighlightedButton + 1;
						break;
					}
					x += _dialogueButtonXoffs;
				}

				break;

			default:
				break;
		}
	}

	if (df != _dialogueHighlightedButton)
		drawDialogueButtons();
	
	if (res == 0)
		return 0;

	_vm->updatePortraits();

	if (!_vm->textEnabled() && _vm->_hideControls) {
		_screen->setScreenDim(5);
		const ScreenDim *d = _screen->getScreenDim(5);
		_screen->hideMouse();
		_screen->fillRect(d->sx, d->sy + d->h - 9, d->sx + d->w - 1, d->sy + d->h - 1, d->unkA);
		_screen->showMouse();
	} else {
		const ScreenDim *d = _screen->_curDim;
		_screen->hideMouse();
		_screen->fillRect(d->sx, d->sy, d->sx + d->w - 2, d->sy + d->h - 1, d->unkA);
		_screen->clearDim(4);
		_screen->setScreenDim(4);
		_screen->showMouse();
		//_screen->setDialogueColumn(8, 0);
		//_screen->setDialogueLine(8, 0);
	}

	return res;
}

char *GUI_LoL::getTableString(int id) {
	return (char *) _vm->getLangString(id);
}

void GUI_LoL::update() {
	_vm->update();
}

} // end of namespace Kyra

