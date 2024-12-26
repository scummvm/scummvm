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
	exeFile.seek(FONT_LITT_OFFSET);
	_litt.loadChr(exeFile);
	exeFile.seek(FONT_EURO_OFFSET);
	_euro.loadChr(exeFile);
	exeFile.close();
	_bios = new BiosFont();
}

GraphicsManager::~GraphicsManager() {
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
	updateScreen();
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

void GraphicsManager::changePalette(byte origin[768], byte target[768]) {
	byte palpaso[768] = {0};

	int auxpaso;
	for (int i = 0; i < 768; i++) {
		palpaso[i] = origin[i];
	}

	for (int jpal = 31; jpal >= 0; jpal--) {
		for (int ipal = 0; ipal <= 255; ipal++) {
			int indexR = 3 * ipal + 0;
			int indexG = 3 * ipal + 1;
			int indexB = 3 * ipal + 2;
			auxpaso = target[indexR] - palpaso[indexR];

			if (auxpaso > 0)
				palpaso[indexR] = palpaso[indexR] + datosfundido[auxpaso][jpal];
			else
				palpaso[indexR] = palpaso[indexR] - datosfundido[-auxpaso][jpal];

			auxpaso = target[indexG] - palpaso[indexG];
			if (auxpaso > 0)
				palpaso[indexG] = palpaso[indexG] + datosfundido[auxpaso][jpal];
			else
				palpaso[indexG] = palpaso[indexG] - datosfundido[-auxpaso][jpal];

			auxpaso = target[indexB] - palpaso[indexB];
			if (auxpaso > 0)
				palpaso[indexB] = palpaso[indexB] + datosfundido[auxpaso][jpal];
			else
				palpaso[indexB] = palpaso[indexB] - datosfundido[-auxpaso][jpal];
		}
		setPalette(palpaso);
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
	}
}

byte *GraphicsManager::loadPalette(Common::String fileName) {
	Common::File paletteFile;
	if (!paletteFile.open(Common::Path(fileName)))
		error("unable to load palette %s (310)", fileName.c_str());
	byte *palette = (byte *)malloc(768);
	paletteFile.read(palette, 768);
	paletteFile.close();
	fixPalette(palette);
	setPalette(palette);
	return palette;
}

void GraphicsManager::fadeIn(uint numeropaleta, Common::String nombrepaleta) {
	byte *palette;

	byte blackPalette[768];
	for (int i = 0; i < 768; i++) {
		blackPalette[i] = 0;
	}
	if (numeropaleta > 0) {
		palette = loadPalette("PALETAS.DAT");
	} else {
		palette = loadPalette(nombrepaleta + ".PAL");
	}
	changePalette(blackPalette, palette);
}

void GraphicsManager::fadeOut(byte rojo) {
	byte palpaso[768];

	for (int i = 0; i <= 255; i++) {
		palpaso[i * 3 + 0] = rojo;
		palpaso[i * 3 + 1] = 0;
		palpaso[i * 3 + 2] = 0;
	}
	changePalette(getPalette(), palpaso);
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

void GraphicsManager::blit(const Graphics::Surface *src, Common::Rect bounds) {
	int16 height = bounds.bottom - bounds.top;
	int16 width = bounds.right - bounds.left;
	Graphics::Surface dest = g_engine->_screen->getSubArea(bounds);

	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width - 1; j++) {
			*((byte *)dest.getBasePtr(j, i)) = *((byte *)src->getBasePtr(j, i));
		}
	}
	g_engine->_screen->addDirtyRect(bounds);
	updateScreen();
}

void GraphicsManager::copyPixels(int x, int y, byte *pixels, uint size) {
	byte *destP = (byte *)g_engine->_screen->getPixels() + y * 320 + x;
	Common::copy(pixels, pixels + size, destP);
	updateScreen();
}

void GraphicsManager::copyRect(Common::Rect rect, byte *pixels) {
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	for (int j = rect.top; j < rect.bottom; j++) {
		for (int i = rect.left; i < rect.right; i++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = pixels[(j - height) * width + (i - width)];
		}
	}
}

void GraphicsManager::effect(int effectNumber, bool toBlack) {
	if (toBlack) {
		switch (effectNumber) {
		case 1:
			for (int i = 0; i <= 69; i++) {
				g_engine->_screen->drawLine(0, (i * 2), 319, i * 2, 0);
				g_system->delayMillis(5);
			}
			for (int i = 70; i >= 1; i--) {
				g_engine->_screen->drawLine(0, (i * 2 - 1), 319, (i * 2, -1), 0);
				g_system->delayMillis(5);
			}
			break;
		case 2:
			for (int i = 70; i >= 1; i--) {
				g_engine->_screen->drawLine(0, (i * 2 - 1), 319, (i * 2 - 1), 0);
				g_system->delayMillis(5);
			}
			for (int i = 0; i <= 69; i++) {
				g_engine->_screen->drawLine(0, (i * 2), 319, (i * 2), 0);
				g_system->delayMillis(5);
			}
			break;

		default:
			break;
		}
	} else {
		switch (effectNumber) {
		case 13:
			// for (int i1 = 0; i1 <= 319; i1 ++)
			// 	for (int j1 = 0; j1 <= 139; j1 ++)

			// 		move(ptr(seg(pantalla2), (ofs(pantalla2) +4 + (j1 * 320) + i1)),
			// 		     ptr(0xa000, ((j1 * 320) + i1)), 1);
			break;

		default:
			break;
		}
	}
}

void GraphicsManager::updateScreen() {
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void GraphicsManager::updateScreen(const Common::Rect &r) {
	g_engine->_screen->addDirtyRect(r);
	g_engine->_screen->update();
}

// void GraphicsManager::restoreBackground(const Common::Rect &r) {
// 	g_engine->_screen->blitFrom(_backBuffer, r, r);
// }

} // End of namespace Tot
