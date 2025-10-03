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

#include "tot/debug.h"
#include "tot/tot.h"
#include "tot/util.h"

 namespace Tot {

// Debug
void drawMouseGrid(RoomFileRegister *screen) {
	for (int i = 0; i < 39; i++) {
		for (int j = 0; j < 27; j++) {
			int color = screen->mouseGrid[i][j];
			if (color == 0) continue;
			int startX = i * kXGridCount + 7;
			int startY = j * kYGridCount;
			for (int i2 = 0; i2 < kXGridCount; i2 += 2) {
				for (int j2 = 0; j2 < kYGridCount; j2++) {
					int absPixel = startY + j2;
					int offsetX = (absPixel % 2 == 0) ? 1 : 0;
					*(byte *)g_engine->_screen->getBasePtr(startX + i2 + offsetX, startY + j2) = 255 - color;
				}
			}

		}
	}
	g_engine->_screen->markAllDirty();
}

void drawScreenGrid(RoomFileRegister *screen) {
	for (int i = 0; i < 39; i++) {
		for (int j = 0; j < 27; j++) {
			int color = screen->walkAreasGrid[i][j];
			if (color == 0)
				continue;

			int startX = i * kXGridCount + 7;
			int startY = j * kYGridCount;
			for (int i2 = 0; i2 < kXGridCount; i2 += 2) {
				for (int j2 = 0; j2 < kYGridCount; j2++) {
					int absPixel = startY + j2;
					int offsetX = (absPixel % 2 == 0) ? 1 : 0;
					*(byte *)g_engine->_screen->getBasePtr(startX + i2 + offsetX, startY + j2) = 255 - color;
				}
			}
		}
	}
	g_engine->_screen->markAllDirty();
}

void drawPos(uint x, uint y, byte color) {
	if (x < 320 && x > 0 && y > 0 && y < 200)
		*(byte *)g_engine->_screen->getBasePtr(x, y) = color;

	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x + 1, y + 1));
	g_engine->_screen->markAllDirty();
}

void drawLine(int x, int y, int x2, int y2, byte color) {
	g_engine->_screen->drawLine(x, y, x2, y2, color);
}

void printNiches() {
	debug("|   | %03d | %03d | %03d | %03d |", 0, 1, 2, 3);
	debug("| 0 | %03d | %03d | %03d | %03d |", g_engine->_niche[0][0], g_engine->_niche[0][1], g_engine->_niche[0][2], g_engine->_niche[0][3]);
	debug("| 1 | %03d | %03d | %03d | %03d |", g_engine->_niche[1][0], g_engine->_niche[1][1], g_engine->_niche[1][2], g_engine->_niche[1][3]);
	debug("niche[0][niche[0][3]] = %d", g_engine->_niche[0][g_engine->_niche[0][3]]);
	debug("niche[1][niche[1][3]] = %d", g_engine->_niche[1][g_engine->_niche[1][3]]);
}

void drawX(int x, int y, byte color) {
	if (x > 0 && y > 0)
		*(byte *)g_engine->_screen->getBasePtr(x, y) = color;
	if (x - 1 > 0 && y - 1 > 0)
		*(byte *)g_engine->_screen->getBasePtr(x - 1, y - 1) = color;
	if (x - 1 > 0 && y + 1 < 140)
		*(byte *)g_engine->_screen->getBasePtr(x - 1, y + 1) = color;
	if (x + 1 < 320 && y + 1 < 140)
		*(byte *)g_engine->_screen->getBasePtr(x + 1, y + 1) = color;
	if (x + 1 < 320 && y - 1 > 0)
		*(byte *)g_engine->_screen->getBasePtr(x + 1, y - 1) = color;
}

void drawCharacterPosition() {
	drawX(g_engine->_characterPosX, g_engine->_characterPosY, 210);
	drawX(g_engine->_characterPosX + kCharacterCorrectionX, g_engine->_characterPosY + kCharacerCorrectionY, 218);
}

void drawRect(byte color, int x, int y, int x2, int y2) {
	rectangle(x, y, x2, y2, color);
}

void printPos(int x, int y, int screenPosX, int screenPosY, const char *label) {
	g_engine->_graphics->restoreBackgroundArea(screenPosX, screenPosY, screenPosX + 100, screenPosY + 10);
	g_engine->_graphics->euroText(Common::String::format("%s: %d, %d", label, x, y), screenPosX, screenPosY, Graphics::kTextAlignLeft);
}

void drawGrid() {
	int horizontal = 320 / kXGridCount;
	int vertical = 140 / kYGridCount;
	for (int i = 0; i < horizontal; i++) {
		int startX = i * kXGridCount;
		g_engine->_screen->drawLine(startX, 0, startX, 140, 200);
	}

	for (int j = 0; j < vertical; j++) {
		int startY = j * kYGridCount;
		g_engine->_screen->drawLine(0, startY, 320, startY, 200);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
}

 } // End of namespace Tot
