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
	
	const char * const *strings = &_mainMenuStrings[_lang << 2];
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	int charWidthBackUp = _screen->_charWidth;
	
	_screen->_charWidth = -2;
	if (_flags.gameID == GI_KYRA2)
		_screen->setScreenDim(11);
	else
		_screen->setScreenDim(3);
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
				gui_printString(strings[selected], textPos, menuRect.top + selected * fh, 0x80, 0, 5);
				gui_printString(strings[item], textPos, menuRect.top + item * fh, 0xFF, 0, 5);

				selected = item;
			}

			if (mousePressed) {
				// TODO: Flash the text
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

void KyraEngine_v2::gui_drawMainMenu(const char * const *strings, int select) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::gui_drawMainMenu(%p)", (const void*)strings);
	static const uint16 menuTable[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0x80, 0xFF, 0x00, 0x01, 0x02, 0x03 };
	
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
	static const uint8 kyra2ColorTable[] = { 0x0, 0x19, 0x28, 0xc8 };
	
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

} // end of namespace Kyra
