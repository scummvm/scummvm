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
	//XXX
	
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

	//initInventoryButtonList(0);

	restorePage3();
	_screen->showMouse();
}

void KyraEngine_v3::hideInventory() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::hideInventory()");
	if (queryGameFlag(3))
		return;

	_inventoryState = false;
	updateCLState();
	//initInventoryButtonList(1);
	
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
	static const int stateTable[] = {
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

} // end of namespace Kyra

