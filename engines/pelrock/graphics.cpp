/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"

#include "pelrock/graphics.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

GraphicsManager::GraphicsManager() {
}

GraphicsManager::~GraphicsManager() {
}

Common::Point GraphicsManager::showOverlay(int height, byte *buf) {
	int overlayY = 400 - height;
	int overlayX = 0;
	for (int x = 0; x < 640; x++) {
		for (int y = overlayY; y < 400; y++) {
			int index = y * 640 + x;
			buf[index] = g_engine->_room->paletteRemaps[0][buf[index]];
		}
	}
	return Common::Point(overlayX, overlayY);
}

byte *GraphicsManager::grabBackgroundSlice(byte *buf, int x, int y, int w, int h) {
	byte *bg = new byte[w * h];
	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			int idx = i * w + j;
			if (y + i < 400 && x + j < 640) {
				*(bg + idx) = buf[(y + i) * 640 + (x + j)];
			}
		}
	}
	return bg;
}

void GraphicsManager::putBackgroundSlice(byte *buf, int x, int y, int w, int h, byte *slice) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = (j * w + i);
			if (x + i < 640 && y + j < 400) {
				buf[(y + j) * 640 + (x + i)] = slice[index];
				// *(byte *)_screen->getBasePtr(x + i, y + j) = slice[index];
			}
		}
	}
}

void GraphicsManager::clearScreen() {
	memset(g_engine->_screen->getPixels(), 0, g_engine->_screen->pitch * g_engine->_screen->h);
}

} // End of namespace Pelrock
