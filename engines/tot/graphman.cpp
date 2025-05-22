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
#include "common/debug.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/paletteman.h"

#include "tot/font/biosfont.h"
#include "tot/graphman.h"
#include "tot/offsets.h"
#include "tot/tot.h"

namespace Tot {

GraphicsManager::GraphicsManager() {
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			datosfundido[i][j] = i / (j + 1);

	Common::File exeFile;
	if (!exeFile.open(Common::Path("TOT.EXE"))) {
		error("Could not open executable file!");
	}

	if (g_engine->_lang == Common::ES_ESP) {
		exeFile.seek(FONT_LITT_OFFSET_ES);
	} else {
		exeFile.seek(FONT_LITT_OFFSET_EN);
	}
	_litt.loadChr(exeFile);
	if (g_engine->_lang == Common::ES_ESP) {
		exeFile.seek(FONT_EURO_OFFSET_ES);
	} else {
		exeFile.seek(FONT_EURO_OFFSET_EN);
	}
	_euro.loadChr(exeFile);
	exeFile.close();
	_bios = new BiosFont();
}

GraphicsManager::~GraphicsManager() {
	delete(_bios);
}

void GraphicsManager::restoreBackground(uint x, uint y, uint x2, uint y2) {
	for (int j = y; j < y2; j++) {
		for (int i = x; i < x2; i++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = 0;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x2, y2));
}

void GraphicsManager::clear() {
	g_engine->_screen->clear();
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void GraphicsManager::clearActionLine() {
	restoreBackground(0, 140, 319, 149);
}

void GraphicsManager::writeActionLine(const Common::String &str) {
	euroText(str, 0, 144, 255, Graphics::kTextAlignCenter, true);
}

void GraphicsManager::setPalette(byte *palette, uint num) {
	g_system->getPaletteManager()->setPalette(palette, 0, num);
}

void GraphicsManager::fixPalette(byte *palette, uint num) {
	for (int i = 0; i < num; i++) {
		palette[i] = palette[i] << 2;
	}
}

void GraphicsManager::getPalette(byte *palette) {
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
}
byte *GraphicsManager::getPalette() {
	byte *palette = (byte *)malloc(768);
	getPalette(palette);
	return palette;
}

// Debug function just to print a palette on the screen
void GraphicsManager::printPalette() {

	int x = 0;
	int y = 0;
	int width = 64;
	int height = 256 / width;
	int squareHeight = 3;
	for (uint16 i = 0; i <= height; i++) {
		for (uint16 j = 0; j <= width; j++) {
			for (int k = 0; k < squareHeight; k++) {
				for (int l = 0; l < squareHeight; l++) {
					*((byte *)g_engine->_screen->getBasePtr(x + j * squareHeight + l, y + i * squareHeight + k)) = i * j;
				}
			}
		}
	}
	// updateScreen();
}

void GraphicsManager::printColor(int x, int y, int color) {
	int squareHeight = 10;
	for (int k = 0; k < squareHeight; k++) {
		for (int l = 0; l < squareHeight; l++) {
			g_engine->_screen->setPixel(x + l, y + k, color);
		}
	}
}

void GraphicsManager::littText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align, bool alignCenterY) {
	int height = _euro.getFontHeight();
	if (alignCenterY) {
		y = y - _euro.getFontHeight() / 2 + 2;
	}
	_litt.drawString(g_engine->_screen, str, x, y, 320, color, align);
}

void GraphicsManager::euroText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align, bool alignCenterY) {
	if (alignCenterY) {
		y = y - _euro.getFontHeight() / 2;
	}
	_euro.drawString(g_engine->_screen, str, x, y, 320, color, align);
}

void GraphicsManager::biosText(const Common::String &str, int x, int y, uint32 color) {
	_bios->drawString(g_engine->_screen, str, x, y, 320, color, Graphics::TextAlign::kTextAlignLeft);
}

} // End of namespace Tot
