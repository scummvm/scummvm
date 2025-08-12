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
#include "tot/statics.h"
#include "tot/tot.h"

namespace Tot {

GraphicsManager::GraphicsManager() {
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			fadeData[i][j] = i / (j + 1);

	// loads fonts
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

void GraphicsManager::restoreBackgroundArea(uint x, uint y, uint x2, uint y2) {
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
	restoreBackgroundArea(0, 140, 319, 149);
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


void GraphicsManager::getImg(uint coordx1, uint coordy1, uint coordx2, uint coordy2, byte *image) {

	uint16 width = coordx2 - coordx1;
	uint16 height = coordy2 - coordy1;

	WRITE_LE_UINT16(image, width);
	WRITE_LE_UINT16(image + 2, height);

	width++;
	height++;

	for (int j = 0; j < width; j++) {
		for (int i = 0; i < height; i++) {
			int idx = 4 + i * width + j;
			*(image + idx) = *(byte *)g_engine->_screen->getBasePtr(coordx1 + j, coordy1 + i);
		}
	}
}

void GraphicsManager::putShape(uint coordx, uint coordy, byte *image) {
	putImg(coordx, coordy, image, true);
}

void GraphicsManager::putImg(uint coordx, uint coordy, byte *image, bool transparency) {
	uint16 w, h;

	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = 4 + (j * w + i);
			if (!transparency || image[index] != 0) {
				*(byte *)g_engine->_screen->getBasePtr(coordx + i, coordy + j) = image[index];
			}
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(coordx, coordy, coordx + w, coordy + h));
}

byte *GraphicsManager::getPalette() {
	byte *palette = (byte *)malloc(768);
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	return palette;
}


// Copies the rectangle delimited by getCoord** from backgroundScreen into image
void GraphicsManager::getImageArea(
	uint getcoordx1,        // xframe
	uint getcoordy1,        // yframe
	uint getcoordx2,        // xframe + framewidth
	uint getcoordy2,        // yframe + frameheight
	byte *backgroundScreen,
	byte *image
) {
	uint16 w = getcoordx2 - getcoordx1;
	uint16 h = getcoordy2 - getcoordy1;

	WRITE_UINT16(image, w);
	WRITE_UINT16(image + 2, h);
	w++;
	h++;
	int posAbs = 4 + getcoordx1 + (getcoordy1 * 320);

	int sourcePtr = 0;
	byte *destPtr = 4 + image; // Start writing after width and height

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			*destPtr++ = backgroundScreen[posAbs + sourcePtr++];
		}
		sourcePtr += (320 - w); // Move to the beginning of the next row in backgroundScreen
	}
}

// puts an image into a buffer in the given position, asuming 320 width
void GraphicsManager::putImageArea(uint putcoordx, uint putcoordy, byte *backgroundScreen, byte *image) {
	uint16 w, h;

	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	w++;
	h++;
	int posAbs = 4 + putcoordx + (putcoordy * 320);

	int sourcePtr = 0;
	byte *destPtr = 4 + image; // Start writing after width and height

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			backgroundScreen[posAbs + sourcePtr++] = *destPtr++;
		}
		sourcePtr += (320 - w); // Move to the beginning of the next row in backgroundScreen
	}
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
