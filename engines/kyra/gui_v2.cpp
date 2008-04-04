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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra.h"
#include "kyra/kyra_v2.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/timer.h"

#include "common/savefile.h"

namespace Kyra {

void KyraEngine_v2::gui_updateMainMenuAnimation() {
	_screen->updateScreen();
}

bool KyraEngine_v2::gui_mainMenuGetInput() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_LBUTTONUP:
			return true;
		default:
			break;
		}
	}
	return false;
}

int KyraEngine_v2::gui_handleMainMenu() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::gui_handleMainMenu()");
	int command = -1;

	uint8 colorMap[16];
	memset(colorMap, 0, sizeof(colorMap));
	_screen->setTextColorMap(colorMap);

	const char * const *strings;
	const char *k2strings[4];

	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	int charWidthBackUp = _screen->_charWidth;

	_screen->_charWidth = -2;

	if (_flags.gameID == GI_KYRA2) {
		_screen->setScreenDim(11);
		k2strings[0] = _sequenceStrings[97];
		k2strings[1] = _sequenceStrings[96];
		k2strings[2] = _sequenceStrings[95];
		k2strings[3] = _sequenceStrings[98];
		strings = k2strings;
	} else {
		_screen->setScreenDim(3);
		strings = &_mainMenuStrings[_lang << 2];
	}

	int backUpX = _screen->_curDim->sx;
	int backUpY = _screen->_curDim->sy;
	int backUpWidth = _screen->_curDim->w;
	int backUpHeight = _screen->_curDim->h;
	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 0, 3);

	int x = _screen->_curDim->sx << 3;
	int y = _screen->_curDim->sy;
	int width = _screen->_curDim->w << 3;
	int height =  _screen->_curDim->h;

	gui_drawMainBox(x, y, width, height, 1);
	gui_drawMainBox(x + 1, y + 1, width - 2, height - 2, 0);

	int selected = 0;

	gui_drawMainMenu(strings, selected);

	_screen->showMouse();

	int fh = _screen->getFontHeight();
	int textPos = ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3;

	Common::Rect menuRect(x + 16, y + 4, x + width - 16, y + 4 + fh * 4);

	while (!_quitFlag) {
		gui_updateMainMenuAnimation();
		bool mousePressed = gui_mainMenuGetInput();

		Common::Point mouse = getMousePos();
		if (menuRect.contains(mouse)) {
			int item = (mouse.y - menuRect.top) / fh;

			if (item != selected) {
				gui_printString(strings[selected], textPos, menuRect.top + selected * fh, (_flags.gameID == GI_KYRA3) ? 0x80 : 0xd7, 0, 5);
				gui_printString(strings[item], textPos, menuRect.top + item * fh, (_flags.gameID == GI_KYRA3) ? 0xFF : 0xd6, 0, 5);

				selected = item;
			}

			if (mousePressed) {
				for (int i = 0; i < 3; i++) {
					gui_printString(strings[selected], textPos, menuRect.top + selected * fh, (_flags.gameID == GI_KYRA3) ? 0x80 : 0xd7, 0, 5);
					_screen->updateScreen();
					_system->delayMillis(50);
					gui_printString(strings[selected], textPos, menuRect.top + selected * fh, (_flags.gameID == GI_KYRA3) ? 0xFF : 0xd6, 0, 5);
					_screen->updateScreen();
					_system->delayMillis(50);
				}
				command = item;
				break;
			}
		}
		_system->delayMillis(10);
	}

	if (_quitFlag)
		command = -1;

	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 3, 0);
	_screen->_charWidth = charWidthBackUp;
	_screen->setFont(oldFont);

	return command;
}

void KyraEngine_v2::gui_drawMainMenu(const char *const *strings, int select) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::gui_drawMainMenu(%p)", (const void*)strings);
	static const uint16 menuTable2[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0xd7, 0xd6, 0x00, 0x01, 0x02, 0x03 };
	static const uint16 menuTable3[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0x80, 0xFF, 0x00, 0x01, 0x02, 0x03 };
	const uint16 *menuTable;

	if (_flags.gameID == GI_KYRA3)
		menuTable = menuTable3;
	else
		menuTable = menuTable2;

	int top = _screen->_curDim->sy;
	top += menuTable[1];

	for (int i = 0; i < menuTable[3]; ++i) {
		int curY = top + i * _screen->getFontHeight();
		int color = (i == select) ? menuTable[6] : menuTable[5];
		gui_printString(strings[i], ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3, curY, color, 0, 5);
	}
}

void KyraEngine_v2::gui_drawMainBox(int x, int y, int w, int h, int fill) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::gui_drawMainBox(%d, %d, %d, %d, %d)", x, y, w, h, fill);
	static const uint8 kyra3ColorTable[] = { 0x16, 0x19, 0x1A, 0x16 };
	static const uint8 kyra2ColorTable[] = { 0xd8, 0xda, 0xd9, 0xd8 };

	const uint8 *colorTable;
	if (_flags.gameID == GI_KYRA3)
		colorTable = kyra3ColorTable;
	else
		colorTable = kyra2ColorTable;

	--w; --h;

	if (fill)
		_screen->fillRect(x, y, x+w, y+h, colorTable[0]);

	_screen->drawClippedLine(x, y+h, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x+w, y, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x, y, x+w, y, colorTable[2]);
	_screen->drawClippedLine(x, y, x, y+h, colorTable[2]);

	_screen->setPagePixel(_screen->_curPage, x, y+h, colorTable[3]);
	_screen->setPagePixel(_screen->_curPage, x+w, y, colorTable[3]);
}

void KyraEngine_v2::gui_printString(const char *format, int x, int y, int col1, int col2, int flags, ...) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::gui_printString('%s', %d, %d, %d, %d, %d, ...)", format, x, y, col1, col2, flags);
	if (!format)
		return;

	char string[512];
	va_list vaList;
	va_start(vaList, flags);
	vsprintf(string, format, vaList);
	va_end(vaList);

	if (flags & 1)
		x -= _screen->getTextWidth(string) >> 1;

	if (flags & 2)
		x -= _screen->getTextWidth(string);

	if (flags & 4) {
		_screen->printText(string, x - 1, y, 240, col2);
		_screen->printText(string, x, y + 1, 240, col2);
	}

	if (flags & 8) {
		_screen->printText(string, x - 1, y, 227, col2);
		_screen->printText(string, x, y + 1, 227, col2);
	}

	_screen->printText(string, x, y, col1, col2);
}

#pragma mark -

void KyraEngine_v2::loadButtonShapes() {
	const uint8 *src = _screen->getCPagePtr(3);
	_screen->loadBitmap("_BUTTONS.CSH", 3, 3, 0);
	_gui->_scrollUpButton.data0ShapePtr = _buttonShapes[0] = _screen->makeShapeCopy(src, 0);
	_gui->_scrollUpButton.data2ShapePtr = _buttonShapes[1] = _screen->makeShapeCopy(src, 1);
	_gui->_scrollUpButton.data1ShapePtr = _buttonShapes[2] = _screen->makeShapeCopy(src, 2);
	_gui->_scrollDownButton.data0ShapePtr = _buttonShapes[3] = _screen->makeShapeCopy(src, 3);
	_gui->_scrollDownButton.data2ShapePtr = _buttonShapes[4] = _screen->makeShapeCopy(src, 4);
	_gui->_scrollDownButton.data1ShapePtr = _buttonShapes[5] = _screen->makeShapeCopy(src, 5);
	_buttonShapes[6] = _screen->makeShapeCopy(src, 6);
	_buttonShapes[7] = _screen->makeShapeCopy(src, 7);
	_buttonShapes[8] = _screen->makeShapeCopy(src, 6);
	_buttonShapes[9] = _screen->makeShapeCopy(src, 7);
	_buttonShapes[10] = _screen->makeShapeCopy(src, 10);
	_buttonShapes[11] = _screen->makeShapeCopy(src, 11);
	_buttonShapes[16] = _screen->makeShapeCopy(src, 16);
	_buttonShapes[17] = _screen->makeShapeCopy(src, 17);
	_buttonShapes[18] = _screen->makeShapeCopy(src, 18);
}

GUI_v2::GUI_v2(KyraEngine_v2 *vm) : GUI(vm), _vm(vm), _screen(vm->screen_v2()) {
	_backUpButtonList = _unknownButtonList = 0;
	initStaticData();
	_currentMenu = 0;
	_isDeathMenu = false;
	_isSaveMenu = false;
	_scrollUpFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::scrollUpButton);
	_scrollDownFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::scrollDownButton);
}

Button *GUI_v2::addButtonToList(Button *list, Button *newButton) {
	list = GUI::addButtonToList(list, newButton);
	_buttonListChanged = true;
	return list;
}

void GUI_v2::processButton(Button *button) {
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

int GUI_v2::processButtonList(Button *buttonList, uint16 inputFlag) {
	if (!buttonList)
		return inputFlag & 0x7FFF;

	if (_backUpButtonList != buttonList || _buttonListChanged) {
		_unknownButtonList = 0;
		//XXX_gui_unk2 (very strange code, maybe keyboard related? or some non interactive input...)
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

		// this is NOT like in the original
		// the original game somehow just enabled flag 0x1000 here
		// but did some other magic, which looks like it depends on how the handle
		// key input... so we just enable 0x1000 and 0x4000 here to allow
		// all GUI buttons to work (for now at least...)
		if (inFlags == 198 || inFlags == 199)
			temp = 0x1000 | 0x4000;

		//if (inputFlag & 0x800)
		//	temp <<= 2;

		// the original did some flag hackery here, this works fine too
		flags |= temp;
	}

	buttonList = _backUpButtonList;
	if (_unknownButtonList) {
		buttonList = _unknownButtonList;
		if (_unknownButtonList->flags & 8)
			_unknownButtonList = 0;
	}

	int returnValue = 0;
	while (buttonList) {
		if (buttonList->flags & 8)
			continue;
		buttonList->flags2 &= 0xFFE7;
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
			buttonList->flags2 &= 0xFFF9;

		if ((flags & 0x3300) && (buttonList->flags & 4) && progress && (buttonList == _unknownButtonList || !_unknownButtonList)) {
			buttonList->flags |= 6;
			if (!_unknownButtonList)
				_unknownButtonList = buttonList;
		} else if ((flags & 0x8800) && !(buttonList->flags & 4) && progress) {
			buttonList->flags2 |= 6;
		} else {
			buttonList->flags2 &= 0xFFF9;
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
					if ((buttonList->flags & 1) && _unknownButtonList == buttonList) {
						buttonList->flags2 ^= 1;
						returnValue = buttonList->index | 0x8000;
						unk1 = true;
					}

					if (!(buttonList->flags & 4)) {
						buttonList->flags2 &= 0xFFFB;
						buttonList->flags2 &= 0xFFFD;
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
						_unknownButtonList = buttonList;
					}
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
				buttonList->flags2 &= 0xFFF9;
		}

		if (!progress && buttonList == _unknownButtonList && !(buttonList->flags & 0x40))
			_unknownButtonList = 0;

		if ((buttonList->flags2 & 0x18) != ((buttonList->flags2 & 3) << 3))
			processButton(buttonList);

		if (unk2)
			buttonList->flags2 &= 0xFFFE;

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

const char *GUI_v2::getMenuTitle(const Menu &menu) {
	if (!menu.menuNameId)
		return 0;

	return _vm->getTableString(menu.menuNameId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 1);
}

const char *GUI_v2::getMenuItemTitle(const MenuItem &menuItem) {
	if (!menuItem.itemId)
		return 0;

	return _vm->getTableString(menuItem.itemId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 1);
}

const char *GUI_v2::getMenuItemLabel(const MenuItem &menuItem) {
	if (!menuItem.labelId)
		return 0;

	return _vm->getTableString(menuItem.labelId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 1);
}

#pragma mark -


int KyraEngine_v2::buttonInventory(Button *button) {
	if (!_screen->isMouseVisible())
		return 0;

	int inventorySlot = button->index - 6;

	uint16 item = _mainCharacter.inventory[inventorySlot];
	if (_itemInHand == -1) {
		if (item == 0xFFFF)
			return 0;
		_screen->hideMouse();
		clearInventorySlot(inventorySlot, 0);
		snd_playSoundEffect(0x0B);
		setMouseCursor(item);
		int string = (_lang == 1) ? getItemCommandStringPickUp(item) : 7;
		updateCommandLineEx(item+54, string, 0xD6);
		_itemInHand = (int16)item;
		_screen->showMouse();
		_mainCharacter.inventory[inventorySlot] = 0xFFFF;
	} else {
		if (_mainCharacter.inventory[inventorySlot] != 0xFFFF) {
			if (checkInventoryItemExchange(_itemInHand, inventorySlot))
				return 0;

			item = _mainCharacter.inventory[inventorySlot];
			snd_playSoundEffect(0x0B);
			_screen->hideMouse();
			clearInventorySlot(inventorySlot, 0);
			drawInventoryShape(0, _itemInHand, inventorySlot);
			setMouseCursor(item);
			int string = (_lang == 1) ? getItemCommandStringPickUp(item) : 7;
			updateCommandLineEx(item+54, string, 0xD6);
			_screen->showMouse();
			_mainCharacter.inventory[inventorySlot] = _itemInHand;
			setHandItem(item);
		} else {
			snd_playSoundEffect(0x0C);
			_screen->hideMouse();
			drawInventoryShape(0, _itemInHand, inventorySlot);
			_screen->setMouseCursor(0, 0, getShapePtr(0));
			int string = (_lang == 1) ? getItemCommandStringInv(_itemInHand) : 8;
			updateCommandLineEx(_itemInHand+54, string, 0xD6);
			_screen->showMouse();
			_mainCharacter.inventory[inventorySlot] = _itemInHand;
			_itemInHand = -1;
		}
	}

	return 0;
}

int KyraEngine_v2::scrollInventory(Button *button) {
	uint16 *src = _mainCharacter.inventory;
	uint16 *dst = &_mainCharacter.inventory[10];
	uint16 temp[5];

	memcpy(temp, src, sizeof(uint16)*5);
	memcpy(src, src+5, sizeof(uint16)*5);
	memcpy(src+5, dst, sizeof(uint16)*5);
	memcpy(dst, dst+5, sizeof(uint16)*5);
	memcpy(dst+5, temp, sizeof(uint16)*5);
	_screen->hideMouse();
	_screen->copyRegion(0x46, 0x90, 0x46, 0x90, 0x71, 0x2E, 0, 2);
	_screen->showMouse();
	redrawInventory(2);
	scrollInventoryWheel();
	return 0;
}

int KyraEngine_v2::getInventoryItemSlot(uint16 item) {
	for (int i = 0; i < 20; ++i) {
		if (_mainCharacter.inventory[i] == item)
			return i;
	}
	return -1;
}

int KyraEngine_v2::findFreeVisibleInventorySlot() {
	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == 0xFFFF)
			return i;
	}
	return -1;
}

void KyraEngine_v2::removeItemFromInventory(int slot) {
	_mainCharacter.inventory[slot] = 0xFFFF;
	if (slot < 10) {
		_screen->hideMouse();
		clearInventorySlot(slot, 0);
		_screen->showMouse();
	}
}

bool KyraEngine_v2::checkInventoryItemExchange(uint16 handItem, int slot) {
	bool removeItem = false;
	uint16 newItem = 0xFFFF;

	uint16 invItem = _mainCharacter.inventory[slot];

	for (const uint16 *table = _itemMagicTable; *table != 0xFFFF; table += 4) {
		if (table[0] != handItem || table[1] != invItem)
			continue;

		if (table[3] == 0xFFFF)
			continue;

		removeItem = (table[3] == 1);
		newItem = table[2];

		snd_playSoundEffect(0x68);
		_mainCharacter.inventory[slot] = newItem;
		_screen->hideMouse();
		clearInventorySlot(slot, 0);
		drawInventoryShape(0, newItem, slot);

		if (removeItem)
			removeHandItem();

		_screen->showMouse();

		if (_lang != 1)
			updateCommandLineEx(newItem+54, 0x2E, 0xD6);

		return true;
	}

	return false;
}

void KyraEngine_v2::drawInventoryShape(int page, uint16 item, int slot) {
	_screen->drawShape(page, getShapePtr(item+64), _inventoryX[slot], _inventoryY[slot], 0, 0);
	_screen->updateScreen();
}

void KyraEngine_v2::clearInventorySlot(int slot, int page) {
	_screen->drawShape(page, _defaultShapeTable[240+slot], _inventoryX[slot], _inventoryY[slot], 0, 0);
	_screen->updateScreen();
}

void KyraEngine_v2::redrawInventory(int page) {
	int pageBackUp = _screen->_curPage;
	_screen->_curPage = page;

	const uint16 *inventory = _mainCharacter.inventory;
	_screen->hideMouse();
	for (int i = 0; i < 10; ++i) {
		clearInventorySlot(i, page);
		if (inventory[i] != 0xFFFF) {
			_screen->drawShape(page, getShapePtr(inventory[i]+64), _inventoryX[i], _inventoryY[i], 0, 0);
			drawInventoryShape(page, inventory[i], i);
		}
	}
	_screen->showMouse();
	_screen->updateScreen();

	_screen->_curPage = pageBackUp;
}

void KyraEngine_v2::scrollInventoryWheel() {
	WSAMovieV2 movie(this);
	movie.open("INVWHEEL.WSA", 0, 0);
	int frames = movie.opened() ? movie.frames() : 6;
	memcpy(_screenBuffer, _screen->getCPagePtr(2), 64000);
	uint8 overlay[0x100];
	_screen->generateOverlay(_screen->getPalette(0), overlay, 0, 50);
	_screen->hideMouse();
	_screen->copyRegion(0x46, 0x90, 0x46, 0x79, 0x71, 0x17, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->showMouse();
	snd_playSoundEffect(0x25);

	movie.setDrawPage(0);
	movie.setX(0);
	movie.setY(0);

	bool breakFlag = false;
	for (int i = 0; i <= 6 && !breakFlag; ++i) {
		if (movie.opened()) {
			_screen->hideMouse();
			movie.displayFrame(i % frames, 0, 0);
			_screen->showMouse();
			_screen->updateScreen();
		}

		uint32 endTime = _system->getMillis() + _tickLength;

		int y = (i * 981) >> 8;
		if (y >= 23 || i == 6) {
			y = 23;
			breakFlag = true;
		}

		_screen->applyOverlay(0x46, 0x79, 0x71, 0x17, 2, overlay);
		_screen->copyRegion(0x46, y+0x79, 0x46, 0x90, 0x71, 0x2E, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(endTime);
	}

	_screen->copyBlockToPage(2, 0, 0, 320, 200, _screenBuffer);
	movie.close();
}

// spellbook specific code

int KyraEngine_v2::bookButton(Button *button) {
	if (!queryGameFlag(1)) {
		objectChat(getTableString(0xEB, _cCodeBuffer, 1), 0, 0x83, 0xEB); 
		return 0;
	}

	if (!_screen->isMouseVisible())
		return 0;

	if (queryGameFlag(0xE5)) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	if (_itemInHand == 72) {
		if (!queryGameFlag(0xE2)) {
			_bookMaxPage += 2;
			removeHandItem();
			snd_playSoundEffect(0x6C);
			setGameFlag(0xE2);
		}

		if (!queryGameFlag(0x18A) && queryGameFlag(0x170)) {
			_bookMaxPage += 2;
			removeHandItem();
			snd_playSoundEffect(0x6C);
			setGameFlag(0x18A);
		}

		return 0;
	}

	if (_handItemSet != -1) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	_screen->hideMouse();
	showMessage(0, 0xCF);
	displayInvWsaLastFrame();
	_bookNewPage = _bookCurPage;

	if (_screenBuffer) {
		_screen->hideMouse();
		memcpy(_screenBuffer, _screen->getCPagePtr(0), 64000);
		_screen->showMouse();
	}

	memcpy(_screen->getPalette(2), _screen->getPalette(0), 768);
	_screen->fadeToBlack(7, &_updateFunctor);
	_res->loadFileToBuf("_BOOK.COL", _screen->getPalette(0), 768);
	loadBookBkgd();
	showBookPage();
	_screen->copyRegion(0, 0, 0, 0, 0x140, 0xC8, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	int oldItemInHand = _itemInHand;
	removeHandItem();
	_screen->fadePalette(_screen->getPalette(0), 7);
	_screen->showMouse();

	bookLoop();

	_screen->fadeToBlack(7);
	_screen->hideMouse();
	setHandItem(oldItemInHand);
	updateMouse();
	restorePage3();

	if (_screenBuffer) {
		_screen->hideMouse();
		_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenBuffer);
		_screen->showMouse();
	}

	setHandItem(_itemInHand);
	memcpy(_screen->getPalette(0), _screen->getPalette(2), 768);
	_screen->fadePalette(_screen->getPalette(0), 7, &_updateFunctor);
	_screen->showMouse();

	if (!queryGameFlag(4) && !queryGameFlag(0xB8)) {
		objectChat(getTableString(0xEC, _cCodeBuffer, 1), 0, 0x83, 0xEC);
		objectChat(getTableString(0xED, _cCodeBuffer, 1), 0, 0x83, 0xED);
		objectChat(getTableString(0xEE, _cCodeBuffer, 1), 0, 0x83, 0xEE);
		objectChat(getTableString(0xEF, _cCodeBuffer, 1), 0, 0x83, 0xEF);
		setGameFlag(4);
	}

	return 0;
}

void KyraEngine_v2::loadBookBkgd() {
	char filename[16];

	if (_flags.isTalkie)
		strcpy(filename, (_bookBkgd == 0) ? "_XBOOKD.CPS" : "_XBOOKC.CPS");
	else
		strcpy(filename, (_bookBkgd == 0) ? "_BOOKD.CPS" : "_BOOKC.CPS");

	_bookBkgd ^= 1;
	
	if (_flags.isTalkie) {
		if (!_bookCurPage)
			strcpy(filename, "_XBOOKB.CPS");
		if (_bookCurPage == _bookMaxPage)
			strcpy(filename, "_XBOOKA.CPS");

		switch (_lang) {
		case 0:
			filename[1] = 'E';
			break;

		case 1:
			filename[1] = 'F';
			break;

		case 2:
			filename[1] = 'G';
			break;

		default:
			warning("loadBookBkgd unsupported language");
			filename[1] = 'E';
			break;
		}
	} else {
		if (!_bookCurPage)
			strcpy(filename, "_BOOKB.CPS");
		if (_bookCurPage == _bookMaxPage)
			strcpy(filename, "_BOOKA.CPS");
	}

	_screen->loadBitmap(filename, 3, 3, 0);
}

void KyraEngine_v2::showBookPage() {
	char filename[16];

	sprintf(filename, "PAGE%.01X.", _bookCurPage);
	strcat(filename, _languageExtension[_lang]);
	uint8 *leftPage = _res->fileData(filename, 0);
	int leftPageY = _bookPageYOffset[_bookCurPage];

	sprintf(filename, "PAGE%.01X.", _bookCurPage+1);
	strcat(filename, _languageExtension[_lang]);
	uint8 *rightPage = (_bookCurPage != _bookMaxPage) ? _res->fileData(filename, 0) : 0;
	int rightPageY = _bookPageYOffset[_bookCurPage+1];

	_screen->hideMouse();
	if (leftPage) {
		bookDecodeText(leftPage);
		bookPrintText(2, leftPage, 20, leftPageY+20, 0x31);
		delete [] leftPage;
	}

	if (rightPage) {
		bookDecodeText(rightPage);
		bookPrintText(2, rightPage, 176, rightPageY+20, 0x31);
		delete [] rightPage;
	}
	_screen->showMouse();
}

void KyraEngine_v2::bookLoop() {
	Button bookButtons[5];

	GUI_V2_BUTTON(bookButtons[0], 0x24, 0, 0, 1, 1, 1, 0x4487, 0, 0x82, 0xBE, 0x0A, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[0].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookPrevPage);
	GUI_V2_BUTTON(bookButtons[1], 0x25, 0, 0, 1, 1, 1, 0x4487, 0, 0xB1, 0xBE, 0x0A, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookNextPage);
	GUI_V2_BUTTON(bookButtons[2], 0x26, 0, 0, 1, 1, 1, 0x4487, 0, 0x8F, 0xBE, 0x21, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookClose);
	GUI_V2_BUTTON(bookButtons[3], 0x27, 0, 0, 1, 1, 1, 0x4487, 0, 0x08, 0x08, 0x90, 0xB4, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookPrevPage);
	GUI_V2_BUTTON(bookButtons[4], 0x28, 0, 0, 1, 1, 1, 0x4487, 0, 0xAA, 0x08, 0x8E, 0xB4, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[4].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookNextPage);

	Button *buttonList = 0;
	
	for (uint i = 0; i < ARRAYSIZE(bookButtons); ++i)
		buttonList = _gui->addButtonToList(buttonList, &bookButtons[i]);

	showBookPage();
	_bookShown = true;
	while (_bookShown && !_quitFlag) {
		checkInput(buttonList);
		removeInputTop();

		if (_bookCurPage != _bookNewPage) {
			_bookCurPage = _bookNewPage;
			_screen->clearPage(2);
			loadBookBkgd();
			showBookPage();
			snd_playSoundEffect(0x64);
			_screen->hideMouse();
			_screen->copyRegion(0, 0, 0, 0, 0x140, 0xC8, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			_screen->showMouse();
		}
	}
	_screen->clearPage(2);
}

void KyraEngine_v2::bookDecodeText(uint8 *str) {
	uint8 *dst = str, *op = str;
	while (*op != 0x1A) {
		while (*op != 0x1A && *op != 0x0D)
			*dst++ = *op++;
		
		if (*op == 0x1A)
			break;

		op += 2;
		*dst++ = 0x0D;
	}
	*dst = 0;
}

void KyraEngine_v2::bookPrintText(int dstPage, const uint8 *str, int x, int y, uint8 color) {
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = dstPage;

	_screen->setTextColor(_bookTextColorMap, 0, 3);
	Screen::FontId oldFont = _screen->setFont(Screen::FID_BOOKFONT_FNT);
	_screen->_charWidth = -2;

	_screen->hideMouse();
	_screen->printText((const char*)str, x, y, color, (_flags.lang == Common::JA_JPN) ? 0x80 : 0);
	_screen->showMouse();

	_screen->_charWidth = 0;
	_screen->setFont(oldFont);
	_screen->_curPage = curPageBackUp;
}

int KyraEngine_v2::bookPrevPage(Button *button) {
	_bookNewPage = MAX<int>(_bookCurPage-2, 0);
	return 0;
}

int KyraEngine_v2::bookNextPage(Button *button) {
	_bookNewPage = MIN<int>(_bookCurPage+2, _bookMaxPage);
	return 0;
}

int KyraEngine_v2::bookClose(Button *button) {
	_bookShown = false;
	return 0;
}

// cauldron specific code

int KyraEngine_v2::cauldronClearButton(Button *button) {
	if (!queryGameFlag(2)) {
		updateCharFacing();
		objectChat(getTableString(0xF0, _cCodeBuffer, 1), 0, 0x83, 0xF0);
		return 0;
	}

	if (queryGameFlag(0xE4)) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	_screen->hideMouse();
	displayInvWsaLastFrame();
	snd_playSoundEffect(0x25);
	loadInvWsa("PULL.WSA", 1, 6, 0, -1, -1, 1);
	loadInvWsa("CAULD00.WSA", 1, 7, 0, 0xD4, 0x0F, 1);
	showMessage(0, 0xCF);
	setCauldronState(0, 0);
	clearCauldronTable();
	snd_playSoundEffect(0x57);
	loadInvWsa("CAULDFIL.WSA", 1, 7, 0, -1, -1, 1);
	_screen->showMouse();
	return 0;
}

int KyraEngine_v2::cauldronButton(Button *button) {
	if (!queryGameFlag(2)) {
		objectChat(getTableString(0xF0, _cCodeBuffer, 1), 0, 0x83, 0xF0);
		return 0;
	}

	if (!_screen->isMouseVisible() || _handItemSet < -1)
		return 0;

	if (queryGameFlag(0xE4)) {
		snd_playSoundEffect(0x0D);	
		return 0;
	}

	updateCharFacing();

	for (int i = 0; _cauldronProtectedItems[i] != -1; ++i) {
		if (_itemInHand == _cauldronProtectedItems[i]) {
			objectChat(getTableString(0xF1, _cCodeBuffer, 1), 0, 0x83, 0xF1);
			return 0;
		}
	}

	if (_itemInHand == -1) {
		listItemsInCauldron();
		return 0;
	}

	for (int i = 0; _cauldronBowlTable[i] != -1; i += 2) {
		if (_itemInHand == _cauldronBowlTable[i]) {
			addFrontCauldronTable(_itemInHand);
			setHandItem(_cauldronBowlTable[i+1]);
			if (!updateCauldron()) {
				_cauldronState = 0;
				cauldronRndPaletteFade();
			}
			return 0;
		}
	}

	if (_itemInHand == 18) {
		const int16 *magicTable = (_mainCharacter.sceneId == 77) ? _cauldronMagicTableScene77 : _cauldronMagicTable;
		while (magicTable[0] != -1) {
			if (_cauldronState == magicTable[0]) {
				setHandItem(magicTable[1]);
				snd_playSoundEffect(0x6C);
				++_cauldronUseCount;
				if (_cauldronStateTable[_cauldronState] <= _cauldronUseCount && _cauldronUseCount) {
					showMessage(0, 0xCF);
					setCauldronState(0, true);
					clearCauldronTable();
				}
				return 0;
			}
			magicTable += 2;
		}
	} else if (_itemInHand >= 0) {
		int item = _itemInHand;
		cauldronItemAnim(item);
		addFrontCauldronTable(item);
		if (!updateCauldron()) {
			_cauldronState = 0;
			cauldronRndPaletteFade();
		}
	}

	return 0;
}

#pragma mark -

void GUI_v2::getInput() {
	if (!_displayMenu)
		return;

	_vm->checkInput(_menuButtonList);
	_vm->removeInputTop();
	if (_vm->quit()) {
		_displayMenu = false;
		_displaySubMenu = false;
	}
}

int GUI_v2::optionsButton(Button *button) {
	_restartGame = false;
	_reloadTemporarySave = false;

	_screen->hideMouse();
	updateButton(&_vm->_inventoryButtons[0]);
	_screen->showMouse();

	if (!_screen->isMouseVisible() && button)
		return 0;

	_vm->showMessage(0, 0xCF);

	if (_vm->_handItemSet < -1) {
		_vm->_handItemSet = -1;
		_screen->hideMouse();
		_screen->setMouseCursor(1, 1, _vm->getShapePtr(0));
		_screen->showMouse();
		return 0;
	}

	int oldHandItem = _vm->_itemInHand;
	_screen->setMouseCursor(0, 0, _vm->getShapePtr(0));
	_vm->displayInvWsaLastFrame();
	//XXX
	_displayMenu = true;

	if (!_vm->gameFlags().isTalkie) {
		//XXX
	}

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i) {
		_menuButtons[i].data0Val1 = _menuButtons[i].data1Val1 = _menuButtons[i].data2Val1 = 4;
		_menuButtons[i].data0Callback = _redrawShadedButtonFunctor;
		_menuButtons[i].data1Callback = _menuButtons[i].data2Callback = _redrawButtonFunctor;
	}

	//XXX
	_loadMenu.numberOfItems = 6;
	initMenuLayout(_loadMenu);
	//XXX
	initMenuLayout(_deathMenu);
	
	if (_vm->_menuDirectlyToLoad) {
		backUpPage1(_vm->_screenBuffer);
		setupPalette();

		_loadedSave = false;
		
		loadMenu(0);

		if (_loadedSave) {
			if (_restartGame)
				_vm->_itemInHand = -1;
		} else {
			restorePage1(_vm->_screenBuffer);
			restorePalette();
		}

		resetState(-1);
		_vm->_menuDirectlyToLoad = false;
		return 0;
	}

	if (!button) {
		_currentMenu = &_deathMenu;
		_isDeathMenu = true;
	} else {
		//XXX just fail for now
		return 0;
	}

	backUpPage1(_vm->_screenBuffer);
	setupPalette();
	initMenu(*_currentMenu);
	_madeTempSave = false;
	_loadedSave = false;
	_vm->_itemInHand = -1;
	updateAllMenuButtons();

	if (_isDeathMenu) {
		while (!_screen->isMouseVisible())
			_screen->showMouse();
	}

	while (_displayMenu) {
		processHighlights(*_currentMenu, _vm->_mouseX, _vm->_mouseY);
		getInput();
	}

	if (_vm->_runFlag && !_loadedSave && !_madeTempSave) {
		restorePalette();
		restorePage1(_vm->_screenBuffer);
	}

	if (_vm->_runFlag)
		updateMenuButton(&_vm->_inventoryButtons[0]);

	resetState(oldHandItem);

	if (!_loadedSave && _reloadTemporarySave) {
		_vm->_unkSceneScreenFlag1 = true;
		//XXX
		_vm->_unkSceneScreenFlag1 = false;
	}

	return 0;
}

#pragma mark -

void GUI_v2::setupPalette() {
	//if (_isDeathMenu)
	//	memcpy(_vm->_unkBuffer1040Bytes, _screen->getPalette(0), 768);

	memcpy(_screen->getPalette(1), _screen->getPalette(0), 768);

	uint8 *palette = _screen->getPalette(0);
	for (int i = 0; i < 768; ++i)
		palette[i] >>= 1;

	static const uint8 guiPal[] = { 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFc, 0xFD, 0xFE };

	for (uint i = 0; i < ARRAYSIZE(guiPal); ++i)
		memcpy(_screen->getPalette(0)+guiPal[i]*3, _screen->getPalette(1)+guiPal[i]*3, 3);

	if (_isDeathMenu)
		_screen->fadePalette(_screen->getPalette(0), 0x64);
	else
		_screen->setScreenPalette(_screen->getPalette(0));
}

void GUI_v2::restorePalette() {
	memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);
	_screen->setScreenPalette(_screen->getPalette(0));
}

void GUI_v2::backUpPage1(uint8 *buffer) {
	_screen->copyRegionToBuffer(1, 0, 0, 320, 200, buffer);
}

void GUI_v2::restorePage1(const uint8 *buffer) {
	_screen->copyBlockToPage(1, 0, 0, 320, 200, buffer);
}

void GUI_v2::resetState(int item) {
	_vm->_timer->resetNextRun();
	_vm->setNextIdleAnimTimer();
	_isDeathMenu = false;
	if (!_loadedSave) {
		_vm->setHandItem(item);
	} else {
		_vm->setHandItem(_vm->_itemInHand);
		_vm->setTimer1DelaySecs(7);
		_vm->_shownMessage = " ";
		_vm->_fadeMessagePalette = false;
	}
	_buttonListChanged = true;
}

void GUI_v2::setupSavegameNames(Menu &menu, int num) {
	for (int i = 0; i < num; ++i) {
		strcpy(_vm->getTableString(menu.item[i].itemId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 0), "");
		menu.item[i].saveSlot = -1;
		menu.item[i].enabled = false;
	}

	KyraEngine::SaveHeader header;
	Common::InSaveFile *in;
	for (int i = 0; i < num; ++i) {
		if ((in = _vm->openSaveForReading(_vm->getSavegameFilename(i + _savegameOffset), header)) != 0) {
			strncpy(_vm->getTableString(menu.item[i].itemId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 0), header.description.c_str(), 80);
			menu.item[i].saveSlot = i + _savegameOffset;
			menu.item[i].enabled = true;
			delete in;
		}
	}
	
	if (_savegameOffset == 0) {
		char *dst = _vm->getTableString(menu.item[0].itemId, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 0);
		const char *src = _vm->getTableString(_vm->gameFlags().isTalkie ? 34 : 42, _vm->gameFlags().isTalkie ? _vm->_optionsBuffer : _vm->_cCodeBuffer, 0);
		strcpy(dst, src);
	}
}

int GUI_v2::scrollUpButton(Button *button) {
	int startSlot = _isSaveMenu ? 1 : 0;
	updateMenuButton(button);

	if (_savegameOffset <= startSlot)
		return 0;

	--_savegameOffset;
	if (_displaySubMenu) {
		setupSavegameNames(_loadMenu, 5);
		// original calls something different here...
		initMenu(_loadMenu);
	} else if (_isSaveMenu) {
	}

	return 0;
}

int GUI_v2::scrollDownButton(Button *button) {
	updateMenuButton(button);
	++_savegameOffset;
	if (_displaySubMenu) {
		setupSavegameNames(_loadMenu, 5);
		// original calls something different here...
		initMenu(_loadMenu);
	} else if (_isSaveMenu) {
	}

	return 0;
}

#pragma mark -

int GUI_v2::loadMenu(Button *caller) {
	if (!_vm->_menuDirectlyToLoad) {
		updateMenuButton(caller);
		restorePage1(_vm->_screenBuffer);
		backUpPage1(_vm->_screenBuffer);
	}

	_savegameOffset = 0;
	setupSavegameNames(_loadMenu, 5);
	initMenu(_loadMenu);
	_displaySubMenu = true;
	_cancelSubMenu = false;
	_vm->_gameToLoad = -1;
	updateAllMenuButtons();

	_screen->updateScreen();
	while (_displaySubMenu) {
		processHighlights(_loadMenu, _vm->_mouseX, _vm->_mouseY);
		getInput();
	}

	if (_cancelSubMenu) {
		if (!_vm->_menuDirectlyToLoad) {
			restorePage1(_vm->_screenBuffer);
			backUpPage1(_vm->_screenBuffer);
			initMenu(*_currentMenu);
			updateAllMenuButtons();
		}
	} else if (_vm->_gameToLoad >= 0) {
		restorePage1(_vm->_screenBuffer);
		restorePalette();
		_vm->loadGame(_vm->getSavegameFilename(_vm->_gameToLoad));
		if (_vm->_gameToLoad == 0) {
			_restartGame = true;
			for (int i = 0; i < 23; ++i)
				_vm->resetCauldronStateTable(i);
			_vm->runStartScript(1, 1);
		}
		_displayMenu = false;
		_loadedSave = true;
	}

	return 0;
}

int GUI_v2::clickLoadSlot(Button *caller) {
	updateMenuButton(caller);
	
	assert((caller->index-0x10) >= 0 && (caller->index-0x10 <= 6));
	MenuItem &item = _loadMenu.item[caller->index-0x10];

	if (item.saveSlot >= 0) {
		_vm->_gameToLoad = item.saveSlot;
		_displaySubMenu = false;
	}

	return 0;
}

} // end of namespace Kyra

