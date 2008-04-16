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

#include "kyra/text_v3.h"

#include "kyra/screen_v3.h"

namespace Kyra {

TextDisplayer_v3::TextDisplayer_v3(KyraEngine_v3 *vm, Screen_v3 *screen)
	: TextDisplayer(vm, screen), _vm(vm), _screen(screen) {
}

void TextDisplayer_v3::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2, Screen::FontId font) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::printText('%s', %d, %d, %d, %d, %d)", str, x, y, c0, c1, c2);
	uint8 colorMap[] = { 0, 255, 240, 240 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	Screen::FontId curFont = _screen->setFont(font);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
	_screen->setFont(curFont);
}

} // end of namespace Kyra
