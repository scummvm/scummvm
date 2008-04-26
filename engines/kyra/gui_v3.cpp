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

#include "kyra/gui_v3.h"
#include "kyra/kyra_v3.h"
#include "kyra/text_v3.h"
#include "kyra/wsamovie.h"

namespace Kyra {

void KyraEngine_v3::showMessage(const char *string, uint8 c0, uint8 c1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showMessage('%s', %d, %d)", string, c0, c1);
	_shownMessage = string;
	_screen->hideMouse();

	restoreCommandLine();
	_restoreCommandLine = false;

	if (string) {
		int x = _text->getCenterStringX(string, 0, 320);
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 0;
		_text->printText(string, x, _commandLineY, c0, c1, 0);
		_screen->_curPage = pageBackUp;
		_screen->updateScreen();
		setCommandLineRestoreTimer(7);
	}

	_screen->showMouse();
}

void KyraEngine_v3::showMessageFromCCode(int string, uint8 c0, int) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showMessageFromCCode(%d, %d, -)", string, c0);
	showMessage((const char*)getTableEntry(_cCodeFile, string), c0, 0xF0);
}

void KyraEngine_v3::updateItemCommand(int item, int str, uint8 c0) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateItemCommand(%d, %d, %d)", item, str, c0);
	char buffer[100];
	char *src = (char*)getTableEntry(_itemFile, item);

	while (*src != ' ')
		++src;
	++src;

	*src = toupper(*src);

	strcpy(buffer, src);
	strcat(buffer, " ");
	strcat(buffer, (const char*)getTableEntry(_cCodeFile, str));

	showMessage(buffer, c0, 0xF0);
}

void KyraEngine_v3::updateCommandLine() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCommandLine()");
	if (_restoreCommandLine) {
		restoreCommandLine();
		_restoreCommandLine = false;
	}
}

void KyraEngine_v3::restoreCommandLine() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::restoreCommandLine()");
	int y = _inventoryState ? 144 : 188;
	_screen->copyBlockToPage(0, 0, y, 320, 12, _interfaceCommandLine);
}

void KyraEngine_v3::updateCLState() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCLState()");
	if (_inventoryState)
		_commandLineY = 145;
	else
		_commandLineY = 189;
}

void KyraEngine_v3::showInventory() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showInventory()");
	if (!_screen->isMouseVisible())
		return;
	if (queryGameFlag(3))
		return;

	_screen->copyBlockToPage(3, 0, 0, 320, 56, _interface);
	drawMalcolmsMoodText();

	_inventoryState = true;
	updateCLState();

	redrawInventory(30);
	drawMalcolmsMoodPointer(-1, 30);
	drawScore(30, 215, 191);
	
	if (queryGameFlag(0x97))
		drawJestersStaff(1, 30);
	
	_screen->hideMouse();

	if (_itemInHand < 0) {
		_handItemSet = -1;
		_screen->setMouseCursor(0, 0, getShapePtr(0));
	}

	_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);

	if (_inventoryScrollSpeed == -1) {
		uint32 endTime = _system->getMillis() + _tickLength * 15;
		int times = 0;
		while (_system->getMillis() < endTime) {
			_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			++times;
		}

		times = MAX(times, 1);

		int speed = 60 / times;
		if (speed <= 1)
			_inventoryScrollSpeed = 1;
		else if (speed >= 8)
			_inventoryScrollSpeed = 8;
		else
			_inventoryScrollSpeed = speed;
	}

	int height = 12;
	int y = 188;
	int times = 0;
	uint32 waitTill = _system->getMillis() + _tickLength;

	while (y > 144) {
		_screen->copyRegion(0, 0, 0, y, 320, height, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		++times;
		if (_inventoryScrollSpeed == 1 && times == 3) {
			while (waitTill > _system->getMillis())
				_system->delayMillis(10);
			times = 0;
			waitTill = _system->getMillis() + _tickLength;
		}
		
		height += _inventoryScrollSpeed;
		y -= _inventoryScrollSpeed;
	}

	_screen->copyRegion(0, 0, 0, 144, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	initMainButtonList(false);

	restorePage3();
	_screen->showMouse();
}

void KyraEngine_v3::hideInventory() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::hideInventory()");
	if (queryGameFlag(3))
		return;

	_inventoryState = false;
	updateCLState();
	initMainButtonList(true);
	
	_screen->copyBlockToPage(3, 0, 0, 320, 56, _interface);
	_screen->hideMouse();
	
	restorePage3();
	flagAnimObjsForRefresh();
	drawAnimObjects();
	_screen->copyRegion(0, 144, 0, 0, 320, 56, 0, 2, Screen::CR_NO_P_CHECK);

	if (_inventoryScrollSpeed == -1) {
		uint32 endTime = _system->getMillis() + _tickLength * 15;
		int times = 0;
		while (_system->getMillis() < endTime) {
			_screen->copyRegion(0, 144, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(0, 144, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			++times;
		}

		times = MAX(times, 1);

		int speed = 60 / times;
		if (speed <= 1)
			_inventoryScrollSpeed = 1;
		else if (speed >= 8)
			_inventoryScrollSpeed = 8;
		else
			_inventoryScrollSpeed = speed;
	}

	int y = 144;
	int y2 = 144 + _inventoryScrollSpeed;
	uint32 waitTill = _system->getMillis() + _tickLength;
	int times = 0;

	while (y2 < 188) {
		_screen->copyRegion(0, 0, 0, y2, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, y, 0, y, 320, _inventoryScrollSpeed, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		++times;
		if (_inventoryScrollSpeed == 1 && times == 3) {
			while (waitTill > _system->getMillis())
				_system->delayMillis(10);
			times = 0;
			waitTill = _system->getMillis() + _tickLength;
		}
		
		y += _inventoryScrollSpeed;
		y2 += _inventoryScrollSpeed;
	}

	_screen->copyRegion(0, 0, 0, 188, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, y, 0, y, 320, 188-y, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->showMouse();
}

void KyraEngine_v3::drawMalcolmsMoodText() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawMalcolmsMoodText()");
	static const int stringId[] = { 0x32, 0x37, 0x3C };

	if (queryGameFlag(0x219))
		return;

	const char *string = (const char*)getTableEntry(_cCodeFile, stringId[_malcolmsMood]);

	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	int width = _screen->getTextWidth(string);

	_screen->_charWidth = 0;
	_screen->setFont(oldFont);

	int pageBackUp = _screen->_curPage;
	const int x = 280 - (width / 2);
	int y = 0;
	if (_inventoryState) {
		y = 189;
		_screen->_curPage = 0;
	} else {
		y = 45;
		_screen->_curPage = 2;
	}

	_screen->hideMouse();
	_screen->drawShape(_screen->_curPage, getShapePtr(432), 244, 189, 0, 0);
	_text->printText(string, x, y+1, 0xFF, 0xF0, 0x00);
	_screen->showMouse();
	_screen->_curPage = pageBackUp;
}

void KyraEngine_v3::drawMalcolmsMoodPointer(int frame, int page) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawMalcolmsMoodPointer(%d, %d)", frame, page);
	static const uint8 stateTable[] = {
		1, 6, 11
	};

	if (frame == -1)
		frame = stateTable[_malcolmsMood];
	if (queryGameFlag(0x219))
		frame = 13;

	if (page == 0) {
		_invWsa->setX(0);
		_invWsa->setY(0);
		_invWsa->setDrawPage(0);
		_invWsa->displayFrame(frame, 0);
		_screen->updateScreen();
	} else if (page == 30) {
		_invWsa->setX(0);
		_invWsa->setY(-144);
		_invWsa->setDrawPage(2);
		_invWsa->displayFrame(frame, 0);
	}

	_invWsaFrame = frame;
}

void KyraEngine_v3::drawJestersStaff(int type, int page) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawJestersStaff(%d, %d)", type, page);
	int y = 155;
	if (page == 30) {
		page = 2;
		y -= 144;
	}

	int shape = (type != 0) ? 454 : 453;
	_screen->drawShape(page, getShapePtr(shape), 217, y, 0, 0);
}

void KyraEngine_v3::drawScore(int page, int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawScore(%d, %d, %d)", page, x, y);
	if (page == 30) {
		page = 2;
		y -= 144;
	}

	int shape1 = _score / 100;
	int shape2 = (_score - shape1*100) / 10;
	int shape3 = _score % 10;

	_screen->drawShape(page, getShapePtr(shape1+433), x, y, 0, 0);
	x += 8;
	_screen->drawShape(page, getShapePtr(shape2+433), x, y, 0, 0);
	x += 8;
	_screen->drawShape(page, getShapePtr(shape3+433), x, y, 0, 0);
}

void KyraEngine_v3::drawScoreCounting(int oldScore, int newScore, int drawOld, const int x) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawScoreCounting(%d, %d, %d, %d)", oldScore, newScore, drawOld, x);
	int y = 189;
	if (_inventoryState)
		y -= 44;

	int old100 = oldScore / 100;
	int old010 = (oldScore - old100*100) / 10;
	int old001 = oldScore % 10;

	int new100 = newScore / 100;
	int new010 = (newScore - new100*100) / 10;
	int new001 = newScore % 10;

	if (drawOld) {
		_screen->drawShape(0, getShapePtr(old100+433), x +  0, y, 0, 0);
		_screen->drawShape(0, getShapePtr(old010+433), x +  8, y, 0, 0);
		_screen->drawShape(0, getShapePtr(old001+433), x + 16, y, 0, 0);
	}

	if (old100 != new100)
		_screen->drawShape(0, getShapePtr(old100+443), x +  0, y, 0, 0);

	if (old010 != new010)
		_screen->drawShape(0, getShapePtr(old010+443), x +  8, y, 0, 0);

	_screen->drawShape(0, getShapePtr(old001+443), x + 16, y, 0, 0);

	_screen->updateScreen();

	_screen->drawShape(0, getShapePtr(new100+433), x +  0, y, 0, 0);
	_screen->drawShape(0, getShapePtr(new010+433), x +  8, y, 0, 0);
	_screen->drawShape(0, getShapePtr(new001+433), x + 16, y, 0, 0);
}

int KyraEngine_v3::getScoreX(const char *str) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getScoreX('%s')", str);
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	int width = _screen->getTextWidth(str);
	int x = 160 + (width / 2) - 32;

	_screen->setFont(oldFont);
	_screen->_charWidth = 0;
	return x;
}

void KyraEngine_v3::redrawInventory(int page) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::redrawInventory(%d)", page);
	int yOffset = 0;

	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	int pageBackUp = _screen->_curPage;
	_screen->_curPage = page;
	_screen->hideMouse();

	for (int i = 0; i < 10; ++i) {
		clearInventorySlot(i, page);
		if (_mainCharacter.inventory[i] != 0xFFFF) {
			_screen->drawShape(page, getShapePtr(_mainCharacter.inventory[i]+248), _inventoryX[i], _inventoryY[i] + yOffset, 0, 0);
			drawInventorySlot(page, _mainCharacter.inventory[i], i);
		}
	}

	_screen->showMouse();
	_screen->_curPage = pageBackUp;
	
	if (page == 0 || page == 1)
		_screen->updateScreen();	
}

void KyraEngine_v3::clearInventorySlot(int slot, int page) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::clearInventorySlot(%d, %d)", slot, page);
	int yOffset = 0;
	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	_screen->drawShape(page, getShapePtr(slot+422), _inventoryX[slot], _inventoryY[slot] + yOffset, 0, 0);
}

void KyraEngine_v3::drawInventorySlot(int page, int item, int slot) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::drawInventorySlot(%d, %d, %d)", page, item, slot);
	int yOffset = 0;
	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	_screen->drawShape(page, getShapePtr(item+248), _inventoryX[slot], _inventoryY[slot] + yOffset, 0, 0);
}

int KyraEngine_v3::buttonInventory(Button *button) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::buttonInventory(%p)", (const void*)button);
	setNextIdleAnimTimer();
	if (!_enableInventory || !_inventoryState || !_screen->isMouseVisible())
		return 0;

	const int slot = button->index - 5;
	const int16 slotItem = (int16)_mainCharacter.inventory[slot];
	if (_itemInHand == -1) {
		if (slotItem == -1)
			return 0;

		_screen->hideMouse();
		clearInventorySlot(slot, 0);
		playSoundEffect(0x0B, 0xC8);
		setMouseCursor(slotItem);
		updateItemCommand(slotItem, (_lang == 1) ? getItemCommandStringPickUp(slotItem) : 0, 0xFF);
		_itemInHand = slotItem;
		_mainCharacter.inventory[slot] = 0xFFFF;
		_screen->showMouse();
	} else if (_itemInHand == 27) {
		if (_chatText)
			return 0;
		return buttonJesterStaff(&_mainButtonData[3]);
	} else {
		if (slotItem >= 0) {
			if (itemInventoryMagic(_itemInHand, slot))
				return 0;

			playSoundEffect(0x0B, 0xC8);

			_screen->hideMouse();
			clearInventorySlot(slot, 0);
			drawInventorySlot(0, _itemInHand, slot);
			setMouseCursor(slotItem);
			updateItemCommand(slotItem, (_lang == 1) ? getItemCommandStringPickUp(slotItem) : 0, 0xFF);
			_mainCharacter.inventory[slot] = _itemInHand;
			_itemInHand = slotItem;
			_screen->showMouse();
		} else {
			playSoundEffect(0x0C, 0xC8);
			_screen->hideMouse();
			drawInventorySlot(0, _itemInHand, slot);
			_screen->setMouseCursor(0, 0, getShapePtr(0));
			updateItemCommand(_itemInHand, (_lang == 1) ? getItemCommandStringInv(_itemInHand) : 2, 0xFF);
			_screen->showMouse();
			_mainCharacter.inventory[slot] = _itemInHand;
			_itemInHand = -1;
		}
	}

	return 0;
}

int KyraEngine_v3::buttonMoodChange(Button *button) {
	if (queryGameFlag(0x219)) {
		playSoundEffect(0x0D, 0xC8);
		return 0;
	}

	static const uint8 frameTable[] = { 1, 6, 11 };

	if (_mouseX >= 245 && _mouseX <= 267 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 0;
	else if (_mouseX >= 268 && _mouseX <= 289 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 1;
	else if (_mouseX >= 290 && _mouseX <= 312 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 2;

	int direction = (_invWsaFrame > frameTable[_malcolmsMood]) ? -1 : 1;

	if (_invWsaFrame != frameTable[_malcolmsMood]) {
		_screen->hideMouse();
		setGameFlag(3);

		playSoundEffect(0x2E, 0xC8);

		while (_invWsaFrame != frameTable[_malcolmsMood]) {
			uint32 endTime = _system->getMillis() + 2 * _tickLength;
			_invWsaFrame += direction;

			drawMalcolmsMoodPointer(_invWsaFrame, 0);
			_screen->updateScreen();

			while (endTime > _system->getMillis()) {
				update();
				_system->delayMillis(10);
			}
		}

		resetGameFlag(3);
		_screen->showMouse();

		drawMalcolmsMoodText();
		updateDlgIndex();
		
		ScriptData data;
		ScriptState state;
		memset(&data, 0, sizeof(data));
		memset(&state, 0, sizeof(state));

		_res->exists("_ACTOR.EMC", true);
		_scriptInterpreter->loadScript("_ACTOR.EMC", &data, &_opcodes);
		_scriptInterpreter->initScript(&state, &data);
		_scriptInterpreter->startScript(&state, 1);

		int vocHigh = _vocHigh;
		_vocHigh = 200;
		_useActorBuffer = true;

		while (_scriptInterpreter->validScript(&state))
			_scriptInterpreter->runScript(&state);

		_useActorBuffer = false;
		_vocHigh = vocHigh;
		_scriptInterpreter->unloadScript(&data);
	}

	return 0;
}

int KyraEngine_v3::buttonShowScore(Button *button) {
	strcpy(_stringBuffer, (const char*)getTableEntry(_cCodeFile, 18));

	char *buffer = _stringBuffer;

	while (*buffer != '%')
		++buffer;

	buffer[0] = (_score / 100) + '0';
	buffer[1] = ((_score % 100) / 10) + '0';
	buffer[2] = (_score % 10) + '0';

	while (*buffer != '%')
		++buffer;

	buffer[0] = (_scoreMax / 100) + '0';
	buffer[1] = ((_scoreMax % 100) / 10) + '0';
	buffer[2] = (_scoreMax % 10) + '0';

	showMessage(_stringBuffer, 0xFF, 0xF0);
	return 0;
}

int KyraEngine_v3::buttonJesterStaff(Button *button) {
	makeCharFacingMouse();
	if (_itemInHand == 27) {
		_screen->hideMouse();
		removeHandItem();
		playSoundEffect(0x0C, 0xC8);
		drawJestersStaff(1, 0);
		updateItemCommand(27, 2, 0xFF);
		setGameFlag(0x97);
		_screen->showMouse();
	} else if (_itemInHand == -1) {
		if (queryGameFlag(0x97)) {
			_screen->hideMouse();
			playSoundEffect(0x0B, 0xC8);
			setHandItem(27);
			drawJestersStaff(0, 0);
			updateItemCommand(27, 0, 0xFF);
			resetGameFlag(0x97);
			_screen->showMouse();
		} else {
			if (queryGameFlag(0x2F))
				objectChat((const char*)getTableEntry(_cCodeFile, 20), 0, 204, 20);
			else
				objectChat((const char*)getTableEntry(_cCodeFile, 25), 0, 204, 25);
		}
	} else {
		objectChat((const char*)getTableEntry(_cCodeFile, 30), 0, 204, 30);
	}
	return 0;
}

#pragma mark -

GUI_v3::GUI_v3(KyraEngine_v3 *vm) : GUI(vm), _vm(vm), _screen(vm->_screen) {
	_backUpButtonList = _unknownButtonList = 0;
	_buttonListChanged = false;
}

Button *GUI_v3::addButtonToList(Button *list, Button *newButton) {
	list = GUI::addButtonToList(list, newButton);
	_buttonListChanged = true;
	return list;
}

void GUI_v3::processButton(Button *button) {
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

int GUI_v3::processButtonList(Button *buttonList, uint16 inputFlag) {
	static uint16 flagsModifier = 0;

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
			temp = 0x1000;
		if (inFlags == 198)
			temp = 0x100;

		if (inputFlag & 0x800)
			temp <<= 2;

		flags |= temp;

		flagsModifier &= ~((temp & 0x4400) >> 1);
		flagsModifier |= (temp & 0x1100) * 2;
		flags |= flagsModifier;
		flags |= (flagsModifier << 2) ^ 0x8800;
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
			if (buttonList->unk6 == inFlags) {
				progress = true;
				flags = buttonList->flags & 0x0F00;
				buttonList->flags2 |= 0x80;
				inputFlag = 0;
				_unknownButtonList = buttonList;
			} else if (buttonList->unk8 == inFlags) {
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

void GUI_v3::flagButtonEnable(Button *button) {
	if (!button)
		return;

	if (button->flags & 8) {
		button->flags &= ~8;
		processButton(button);
	}
}

void GUI_v3::flagButtonDisable(Button *button) {
	if (!button)
		return;

	if (!(button->flags & 8)) {
		button->flags |= 8;
		processButton(button);
	}
}

} // end of namespace Kyra

