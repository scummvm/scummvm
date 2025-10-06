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

#include "common/config-manager.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/paletteman.h"

#include "tot/graphics.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

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

	if (isLanguageSpanish()) {
		exeFile.seek(FONT_LITT_OFFSET_ES);
	} else {
		exeFile.seek(FONT_LITT_OFFSET_EN);
	}
	_litt = new Graphics::BgiFont();
	_litt->loadChr(exeFile);
	if (isLanguageSpanish()) {
		exeFile.seek(FONT_EURO_OFFSET_ES);
	} else {
		exeFile.seek(FONT_EURO_OFFSET_EN);
	}
	_euro = new Graphics::BgiFont();
	_euro->loadChr(exeFile);
	exeFile.close();
	_dosFont = new Graphics::DosFont();
}

GraphicsManager::~GraphicsManager() {
	delete (_dosFont);
	delete (_litt);
	delete (_euro);
	free(_textAreaBackground);
}

void GraphicsManager::restoreBackgroundArea(uint x, uint y, uint x2, uint y2) {
	for (uint j = y; j < y2; j++) {
		for (uint i = x; i < x2; i++) {
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

void GraphicsManager::setPalette(byte *palette, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(palette, start, num);
}

void GraphicsManager::fixPalette(byte *palette, uint num) {
	for (uint i = 0; i < num; i++) {
		palette[i] <<= 2;
	}
}

byte *GraphicsManager::getPalette() {
	byte *palette = (byte *)malloc(768);
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	return palette;
}

void GraphicsManager::advancePaletteAnim() {
	if (g_engine->_currentRoomData->paletteAnimationFlag && _palAnimStep >= 4) {
		_palAnimStep = 0;
		if (_paletteAnimFrame > 6)
			_paletteAnimFrame = 0;
		else
			_paletteAnimFrame += 1;
		if (g_engine->_currentRoomData->code == 4 && _paletteAnimFrame == 4)
				g_engine->_sound->playVoc();
		g_engine->_graphics->updatePalette(_paletteAnimFrame);
	} else
		_palAnimStep += 1;
}

void GraphicsManager::loadPaletteFromFile(Common::String paletteName) {

	Common::File fichero;

	if (!fichero.open(Common::Path(paletteName + ".PAL")))
		showError(310);
	fichero.read(_pal, 768);
	fichero.close();

	fixPalette(_pal, 768);
	setPalette(_pal);
}

void GraphicsManager::fadePalettes(byte *from, byte *to) {
	byte intermediate[768];
	int aux;

	copyPalette(from, intermediate);
	for (int j = 32; j >= 0; j--) {
		for (int i = 0; i < 256; i++) {
			aux = to[3 * i + 0] - intermediate[3 * i + 0];
			if (aux > 0)
				intermediate[3 * i + 0] = intermediate[3 * i + 0] + fadeData[aux][j];
			else
				intermediate[3 * i + 0] = intermediate[3 * i + 0] - fadeData[-aux][j];

			aux = to[3 * i + 1] - intermediate[3 * i + 1];
			if (aux > 0)
				intermediate[3 * i + 1] = intermediate[3 * i + 1] + fadeData[aux][j];
			else
				intermediate[3 * i + 1] = intermediate[3 * i + 1] - fadeData[-aux][j];

			aux = to[3 * i + 2] - intermediate[3 * i + 2];
			if (aux > 0)
				intermediate[3 * i + 2] = intermediate[3 * i + 2] + fadeData[aux][j];
			else
				intermediate[3 * i + 2] = intermediate[3 * i + 2] - fadeData[-aux][j];
		}
		setPalette(intermediate, 0, 256);
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
	}
}

void GraphicsManager::turnLightOn() {
	Common::File paletteFile;
	byte intermediate[768];

	if (!paletteFile.open("PALETAS.DAT")) {
		showError(311);
	}
	paletteFile.seek(1536);
	paletteFile.read(intermediate, 768);
	paletteFile.close();
	for (int i = 0; i < 256; i++) {
		if (i >= 201) {
			intermediate[3 * i + 0] = _pal[3 * i + 0];
			intermediate[3 * i + 1] = _pal[3 * i + 1];
			intermediate[3 * i + 2] = _pal[3 * i + 2];
		} else {
			intermediate[3 * i + 0] <<= 2;
			intermediate[3 * i + 1] <<= 2;
			intermediate[3 * i + 2] <<= 2;
		}
	}
	fadePalettes(_pal, intermediate);
	copyPalette(intermediate, _pal);
}

void GraphicsManager::totalFadeOut(byte redComponent) {
	byte intermediate[768];

	for (int ipal = 0; ipal <= 255; ipal++) {
		intermediate[3 * ipal + 0] = redComponent;
		intermediate[3 * ipal + 1] = 0;
		intermediate[3 * ipal + 2] = 0;
	}
	fadePalettes(_pal, intermediate);
	copyPalette(intermediate, _pal);
}

void GraphicsManager::partialFadeOut(byte numcol) {
	byte intermediate[768];

	for (int i = 0; i <= numcol; i++) {
		intermediate[3 * i + 0] = 0;
		intermediate[3 * i + 1] = 0;
		intermediate[3 * i + 2] = 0;
	}
	for (int i = (numcol + 1); i <= 255; i++) {
		intermediate[3 * i + 0] = _pal[3 * i + 0];
		intermediate[3 * i + 1] = _pal[3 * i + 1];
		intermediate[3 * i + 2] = _pal[3 * i + 2];
	}
	fadePalettes(_pal, intermediate);
	copyPalette(intermediate, _pal);
}

void GraphicsManager::partialFadeIn(byte numcol) {
	byte darkPalette[768];

	for (int i = 0; i <= numcol; i++) {
		darkPalette[3 * i + 0] = 0;
		darkPalette[3 * i + 1] = 0;
		darkPalette[3 * i + 2] = 0;
	}
	for (int i = (numcol + 1); i <= 255; i++) {
		darkPalette[3 * i + 0] = _pal[3 * i + 0];
		darkPalette[3 * i + 1] = _pal[3 * i + 1];
		darkPalette[3 * i + 2] = _pal[3 * i + 2];
	}
	fadePalettes(darkPalette, _pal);
}

void GraphicsManager::totalFadeIn(uint paletteNumber, Common::String paletteName) {
	byte intermediate[768];
	byte darkPalette[768];

	Common::File paletteFile;
	if (paletteNumber > 0) {
		if (!paletteFile.open("PALETAS.DAT"))
			showError(311);
		paletteFile.seek(paletteNumber);
		paletteFile.read(intermediate, 768);
		paletteFile.close();
	} else {
		if (!paletteFile.open(Common::Path(Common::String(paletteName + ".PAL")))) {
			showError(311);
		}
		paletteFile.read(intermediate, 768);
		paletteFile.close();
	}
	for (int i = 0; i <= 255; i++) {
		darkPalette[3 * i + 0] = 0;
		darkPalette[3 * i + 1] = 0;
		darkPalette[3 * i + 2] = 0;
		// 6-bit color correction
		intermediate[3 * i + 0] <<= 2;
		intermediate[3 * i + 1] <<= 2;
		intermediate[3 * i + 2] <<= 2;
	}

	fadePalettes(darkPalette, intermediate);
	copyPalette(intermediate, _pal);
}

void GraphicsManager::redFadeIn(byte *intermediatePalette) {
	byte dark[768];

	for (int i = 0; i < 256; i++) {
		dark[3 * i + 0] = 0;
		dark[3 * i + 1] = 0;
		dark[3 * i + 2] = 0;
	}
	fadePalettes(_pal, dark);
	fadePalettes(dark, intermediatePalette);
	copyPalette(intermediatePalette, _pal);
}

void GraphicsManager::updatePalette(byte paletteIndex) {
	int ip = 0;

	switch (g_engine->_gamePart) {
	case 1: {
		for (int i = 0; i <= 5; i++) {
			_pal[(i + 195) * 3 + 0] = _palAnimSlice[(paletteIndex * 6 + i) * 3 + 0];
			_pal[(i + 195) * 3 + 1] = _palAnimSlice[(paletteIndex * 6 + i) * 3 + 1];
			_pal[(i + 195) * 3 + 2] = _palAnimSlice[(paletteIndex * 6 + i) * 3 + 2];
		}
		setPalette(&_pal[195 * 3 + 0], 195, 6);
	} break;
	case 2: {
		switch (paletteIndex) {
		case 0:
		case 4:
			ip = 0;
			break;
		case 1:
		case 3:
			ip = 4;
			break;
		case 2:
			ip = 8;
			break;
		case 5:
		case 7:
			ip = -4;
			break;
		case 6:
			ip = -8;
			break;
		}

		for (int i = 0; i < 3; i++) {
			_pal[131 * 3 + i] = _pal[131 * 3 + i] - ip;
			_pal[134 * 3 + i] = _pal[134 * 3 + i] - ip;
			_pal[143 * 3 + i] = _pal[143 * 3 + i] - ip;
			_pal[187 * 3 + i] = _pal[187 * 3 + i] - ip;
		}
		setPalette(_pal);

	} break;
	}
}

void GraphicsManager::copyPalette(byte *from, byte *to) {
	Common::copy(from, from + 768, to);
}

void GraphicsManager::printColor(int x, int y, int color) {
	int squareHeight = 10;
	for (int k = 0; k < squareHeight; k++) {
		for (int l = 0; l < squareHeight; l++) {
			g_engine->_screen->setPixel(x + l, y + k, color);
		}
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

// Copies the rectangle delimited by getCoord** from backgroundScreen into image
void GraphicsManager::getImageArea(
	uint getcoordx1, // xframe
	uint getcoordy1, // yframe
	uint getcoordx2, // xframe + framewidth
	uint getcoordy2, // yframe + frameheight
	byte *backgroundScreen,
	byte *image) {
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

void GraphicsManager::littText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align, bool alignCenterY) {
	if (alignCenterY) {
		y = y - _litt->getFontHeight() / 2 + 2;
	}
	_litt->drawString(g_engine->_screen, str, x, y, 320, color, align);
}

void GraphicsManager::euroText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align, bool alignCenterY) {
	if (alignCenterY) {
		y = y - _euro->getFontHeight() / 2;
	}
	_euro->drawString(g_engine->_screen, str, x, y, 320, color, align);
}

int GraphicsManager::euroTextWidth(const Common::String &str) {
	return (int)_euro->getBoundingBox(str).width();
}

void GraphicsManager::biosText(const Common::String &str, int x, int y, uint32 color) {
	_dosFont->drawString(g_engine->_screen, str, x, y, 320, color, Graphics::TextAlign::kTextAlignLeft);
}

void GraphicsManager::drawFullScreen(byte *screen) {
	Common::copy(screen, screen + 64000, (byte *)g_engine->_screen->getPixels());
}

void GraphicsManager::copyFromScreen(byte *&screen) {
	byte *src = (byte *)g_engine->_screen->getPixels();
	Common::copy(src, src + 64000, screen);
}

void GraphicsManager::drawScreen(byte *screen, bool offsetSize) {
	int offset = offsetSize ? 4 : 0;
	for (int i1 = 0; i1 < 320; i1++) {
		for (int j1 = 0; j1 < 140; j1++) {
			((byte *)g_engine->_screen->getPixels())[j1 * 320 + i1] = ((byte *)screen)[offset + j1 * 320 + i1];
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
}

void GraphicsManager::updateSceneArea(int speed) {
		g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		g_engine->_screen->update();
}
void GraphicsManager::sceneTransition(bool fadeToBlack, byte *scene) {
	sceneTransition(fadeToBlack, scene, getRandom(15) + 1);
}

void updateScreenIfNeeded(uint32 &targetTime) {
	g_engine->_events->pollEvent();
	if (g_system->getMillis() > targetTime) {
		g_engine->_graphics->updateSceneArea();
		targetTime = g_system->getMillis() + 10;
	}
	g_system->delayMillis(1);
}

void effect9(bool fadeToBlack, byte *scene);
void effect7(bool fadeToBlack, byte *scene);

void effect13(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 319; i1 >= 0; i1--) {
			line(i1, 0, i1, 139, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		for (int i1 = 0; i1 < 320; i1++) {
			for (int j1 = 0; j1 < 140; j1++) { // one vertical line
				int color = scene[4 + (j1 * 320) + i1];
				*((byte *)g_engine->_screen->getBasePtr(i1, j1)) = color;
			}
			updateScreenIfNeeded(targetTime);
		}
	}
}

void effect1(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 0; i1 <= 69; i1++) {

			line(0, (i1 * 2), 319, (i1 * 2), 0);
			updateScreenIfNeeded(targetTime);
		}
		for (int i1 = 70; i1 >= 1; i1--) {

			line(0, (i1 * 2 - 1), 319, (i1 * 2 - 1), 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();
		for (int i1 = 0; i1 <= 69; i1++) {
			byte *src = scene + (i1 * 640) + 4;
			Common::copy(src, src + 320, screenBuf + (i1 * 640));
			updateScreenIfNeeded(targetTime);
		}
		for (int i1 = 70; i1 >= 1; i1--) {
			byte *src = scene + (i1 * 640) - 320 + 4;
			Common::copy(src, src + 320, screenBuf + (i1 * 640) - 320);
			updateScreenIfNeeded(targetTime);
		}
	}
}

void effect2(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 70; i1 >= 1; i1--) {
			line(0, (i1 * 2 - 1), 319, (i1 * 2 - 1), 0);
			updateScreenIfNeeded(targetTime);
		}
		for (int i1 = 0; i1 <= 69; i1++) {
			line(0, (i1 * 2), 319, (i1 * 2), 0);
			updateScreenIfNeeded(targetTime);
			// delay(5);
		}
	} else {
		effect9(fadeToBlack, scene);
	}
}

void effect9(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 319; i1 >= 0; i1--) {
			line(319, 139, i1, 0, 0);
			updateScreenIfNeeded(targetTime);
		}
		for (int i1 = 1; i1 <= 139; i1++) {
			line(319, 139, 0, i1, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();

		for (int i1 = 70; i1 >= 1; i1--) {
			byte *src = scene + ((i1 * 640) - 320) + 4;
			Common::copy(src, src + 320, screenBuf + (i1 * 640) - 320);
			updateScreenIfNeeded(targetTime);
			// delay(5);
		}
		for (int i1 = 0; i1 <= 69; i1++) {
			byte *src = scene + (i1 * 640) + 4;
			Common::copy(src, src + 320, screenBuf + (i1 * 640));
			updateScreenIfNeeded(targetTime);
			// delay(5);
		}
	}
}

void effect3(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	int i1 = 0, j1 = 0;
	if (fadeToBlack) {
		for (int i2 = 1; i2 <= 9; i2++) {
			i1 = 10;
			do {
				j1 = 10;
				do {
					bar((i1 - i2), (j1 - i2), (i1 + i2), (j1 + i2), 0);
					j1 += 20;
				} while (j1 != 150);
				i1 += 20;
				updateScreenIfNeeded(targetTime);
			} while (i1 != 330);
		}
		bar(0, 0, 319, 139, 0);
	} else {
		effect7(false, scene);
	}
}

void effect7(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	int i1 = 0, j1 = 0;
	if (fadeToBlack) {
		for (i1 = 69; i1 >= 0; i1--) {
			rectangle(i1, i1, (319 - i1), (139 - i1), 0);
			updateScreenIfNeeded(targetTime);
			g_system->delayMillis(1);
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();
		for (int i2 = 1; i2 <= 9; i2++) {
			i1 = 10;
			do {
				j1 = 10;
				do {
					for (int i3 = (j1 - i2); i3 <= (j1 + i2); i3++) {
						byte *src = scene + +4 + (i3 * 320) + +(i1 - i2);
						Common::copy(src, src + (i2 * 2), screenBuf + (i3 * 320) + (i1 - i2));
					}
					j1 += 20;
				} while (j1 != 150);
				updateScreenIfNeeded(targetTime);
				i1 += 20;
			} while (i1 != 330);
		}
		g_engine->_graphics->drawScreen(scene);
	}
}

void effect4(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i2 = 1; i2 <= 22; i2++) {
			for (int i1 = 1; i1 <= 16; i1++)
				for (int j1 = 1; j1 <= 7; j1++) {
					if ((i1 + j1) == (i2 + 1)) {
						bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
						updateScreenIfNeeded(targetTime);
						// delay(8);
					}
				}
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();
		for (int i2 = 1; i2 <= 22; i2++) {
			for (int i1 = 1; i1 <= 16; i1++)
				for (int j1 = 1; j1 <= 7; j1++) {
					if ((i1 + j1) == (i2 + 1)) {
						for (int i3 = ((j1 - 1) * 20); i3 <= ((j1 - 1) * 20 + 19); i3++) {
							byte *src = scene + +4 + (i3 * 320) + ((i1 - 1) * 20);
							Common::copy(src, src + 20, screenBuf + (i3 * 320) + ((i1 - 1) * 20));
						}
						updateScreenIfNeeded(targetTime);
						// delay(8);
					}
				}
		}
	}
}

void effect5(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int j1 = 0; j1 <= 139; j1++) {
			line(0, j1, 319, (139 - j1), 0);
			updateScreenIfNeeded(targetTime);
		}
		for (int j1 = 318; j1 >= 1; j1--) {
			line(j1, 0, (319 - j1), 139, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		effect9(false, scene);
	}
}

void effect6(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 0; i1 < 70; i1++) {
			rectangle(i1, i1, (319 - i1), (139 - i1), 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();
		for (int i1 = 70; i1 >= 0; i1--) {
			for (int j1 = i1; j1 <= (139 - i1); j1++) {
				byte *src = scene + 4 + (j1 * 320) + i1;
				Common::copy(src, src + 319 - (i1 * 2), screenBuf + (j1 * 320) + i1);
			}
			updateScreenIfNeeded(targetTime);
		}
	}
}

void effect8(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 0; i1 <= 319; i1++) {
			line(0, 139, i1, 0, 0);
			updateScreenIfNeeded(targetTime);
		}
		for (int i1 = 1; i1 <= 139; i1++) {
			line(0, 139, 319, i1, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		effect9(false, scene);
	}
}

void effect10(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int j1 = 1; j1 <= 7; j1++) {
			if ((j1 % 2) == 0)
				for (int i1 = 1; i1 <= 16; i1++) {
					bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
					updateScreenIfNeeded(targetTime);
					g_system->delayMillis(2); // original enforces delay(8);
				}
			else
				for (int i1 = 16; i1 >= 1; i1--) {
					bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
					updateScreenIfNeeded(targetTime);
					g_system->delayMillis(2); // original enforces delay(8);
				}
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();
		for (int j1 = 1; j1 <= 7; j1++) {
			if ((j1 % 2) == 0)
				for (int i1 = 1; i1 <= 16; i1++) {
					for (int j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
						byte *src = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);

						Common::copy(src, src + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					}
					updateScreenIfNeeded(targetTime);
					g_system->delayMillis(2); // original enforces delay(8);
				}
			else
				for (int i1 = 16; i1 >= 1; i1--) {
					for (int j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
						byte *src = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);
						Common::copy(src, src + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					}
					updateScreenIfNeeded(targetTime);
					g_system->delayMillis(2); // original enforces delay(8);
				}
		}
	}
}

void effect11(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	int i1 = 0, j1 = 0, j3 = 0;
	if (fadeToBlack) {
		for (int i2 = 0; i2 <= 3; i2++) {
			j1 = 1 + i2;
			for (i1 = (1 + i2); i1 <= (16 - i2); i1++) {
				bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			i1--;
			for (j1 = (2 + i2); j1 <= (7 - i2); j1++) {
				bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			j1--;
			for (i1 = (16 - i2); i1 >= (1 + i2); i1--) {
				bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			i1++;
			for (j1 = (6 - i2); j1 >= (2 + i2); j1--) {
				bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();

		for (int i2 = 0; i2 <= 3; i2++) {
			j1 = 1 + i2;
			for (i1 = (1 + i2); i1 <= (16 - i2); i1++) {
				for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
					byte *buf = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);
					Common::copy(buf, buf + 20, screenBuf + ((j3 * 320) + ((i1 - 1) * 20)));
				}
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			i1--;
			for (j1 = (2 + i2); j1 <= (7 - i2); j1++) {
				for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
					byte *buf = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);
					Common::copy(buf, buf + 20, screenBuf + ((j3 * 320) + ((i1 - 1) * 20)));
				}
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			j1--;
			for (i1 = (16 - i2); i1 >= (1 + i2); i1--) {
				for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
					byte *buf = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);
					Common::copy(buf, buf + 20, screenBuf + ((j3 * 320) + ((i1 - 1) * 20)));
				}
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
			i1++;
			for (j1 = (6 - i2); j1 >= (2 + i2); j1--) {
				for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
					byte *buf = scene + 4 + (j3 * 320) + ((i1 - 1) * 20);
					Common::copy(buf, buf + 20, screenBuf + ((j3 * 320) + ((i1 - 1) * 20)));
				}
				updateScreenIfNeeded(targetTime);
				g_system->delayMillis(1); // delay(8);
			}
		}
	}
}

void effect12(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	int i2 = 0, j2 = 0;
	if (fadeToBlack) {
		for (int i1 = 1; i1 <= 15000; i1++) {
			i2 = getRandom(318);
			j2 = getRandom(138);
			bar(i2, j2, (i2 + 2), (j2 + 2), 0);
			putpixel(getRandom(320), getRandom(139), 0);
			if (i1 % 100 == 0) {
				updateScreenIfNeeded(targetTime);
			}
		}
	} else {
		byte *screenBuf = (byte *)g_engine->_screen->getPixels();

		for (int i1 = 1; i1 <= 15000; i1++) {
			i2 = getRandom(318);
			j2 = getRandom(138);

			byte *src1 = scene + 4 + (j2 * 320) + i2;
			byte *src2 = scene + 4 + ((j2 + 1) * 320) + i2;
			Common::copy(src1, src1 + 2, screenBuf + j2 * 320 + i2);

			Common::copy(src2, src2 + 2, screenBuf + (j2 + 1) * 320 + i2);
			i2 = getRandom(320);
			j2 = getRandom(140);

			byte *src3 = scene + 4 + (j2 * 320) + i2;
			Common::copy(src3, src3 + 1, screenBuf + (j2 * 320) + i2);
			if (i1 % 100 == 0) {
				updateScreenIfNeeded(targetTime);
			}
		}
		g_engine->_graphics->drawScreen(scene);
	}
}

void effect14(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	if (fadeToBlack) {
		for (int i1 = 0; i1 <= 319; i1++) {
			line(i1, 0, i1, 139, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		for (int i1 = 320; i1 > 0; i1--) {
			for (int j1 = 0; j1 < 140; j1++) {
				int color = scene[4 + (j1 * 320) + i1];
				*((byte *)g_engine->_screen->getBasePtr(i1, j1)) = color;
			}
			updateScreenIfNeeded(targetTime);
		}
	}
}

void effect15(bool fadeToBlack, byte *scene) {
	uint32 targetTime = 0;
	byte *screenBuf = (byte *)g_engine->_screen->getPixels();

	if (fadeToBlack) {
		for (int j1 = 0; j1 <= 70; j1++) {
			Common::copy(screenBuf + (j1 * 320), screenBuf + (j1 * 320) + 320, screenBuf + ((j1 + 1) * 320));
			line(0, j1, 319, j1, 0);
			Common::copy(screenBuf + ((139 - j1) * 320), screenBuf + ((139 - j1) * 320) + 320, screenBuf + ((138 - j1) * 320));
			line(0, (139 - j1), 319, (139 - j1), 0);
			// delay(2);
			updateScreenIfNeeded(targetTime);
		}
		// delay(5);
		for (int j1 = 0; j1 <= 160; j1++) {
			line(0, 68, j1, 68, 0);
			line(319, 68, (319 - j1), 68, 0);
			updateScreenIfNeeded(targetTime);
		}
	} else {
		for (int j1 = 160; j1 > 0; j1--) {
			line(j1, 69, (319 - j1), 69, 255);
			updateScreenIfNeeded(targetTime);
		}
		// delay(5);
		for (int j1 = 70; j1 > 0; j1--) {
			byte *src1 = scene + 4 + j1 * 320;
			byte *src2 = scene + 4 + (139 - j1) * 320;
			Common::copy(src1, src1 + 320, screenBuf + j1 * 320);

			Common::copy(src2, src2 + 320, screenBuf + (139 - j1) * 320);
			updateScreenIfNeeded(targetTime);
		}
	}
}

void GraphicsManager::sceneTransition(bool fadeToBlack, byte *scene, byte effectNumber) {
	bool enabled = ConfMan.getBool("transitions_disable") == false;
	// Disable effects for now
	if (!enabled) {
		if (!fadeToBlack) {
			drawScreen(scene);
		}
	} else {
		g_engine->_cpCounter2 = g_engine->_cpCounter;
		if (g_engine->_cpCounter > 145)
			showError(274);
		if (fadeToBlack)
			switch (effectNumber) {
			case 1: {
				effect1(true, scene);
			} break;
			case 2: {
				effect2(true, scene);
			} break;
			case 3: {
				effect3(true, scene);
			} break;
			case 4: {
				effect4(true, scene);
			} break;
			case 5: {
				effect5(true, scene);
			} break;
			case 6: {
				effect6(true, scene);
			} break;
			case 7: {
				effect7(true, scene);
			} break;
			case 8: {
				effect8(true, scene);
			} break;
			case 9: {
				effect9(true, scene);
			} break;
			case 10: {
				effect10(true, scene);
			} break;
			case 11: {
				effect11(true, scene);
			} break;
			case 12: {
				effect12(true, scene);
			} break;
			case 13: {
				effect13(true, scene);
			} break;
			case 14: {
				effect14(true, scene);
			} break;
			case 15: {
				effect15(true, scene);
			} break;
			}
		else
			switch (effectNumber) {
			case 1:
				effect1(false, scene);
			 	break;
			case 2:
			case 5:
			case 8:
			case 9:
				effect9(false, scene);
			break;
			case 3:
			case 7: {
				effect7(false, scene);
			} break;
			case 4: {
				effect4(false, scene);
			} break;
			case 6: {
				effect6(false, scene);
			} break;
			case 10: {
				effect10(false, scene);
			} break;
			case 11: {
				effect11(false, scene);
			} break;
			case 12: {
				effect12(false, scene);
			} break;
			case 13:
				effect13(false, scene);
				break;
			case 14: {
				effect14(false, scene);
			} break;
			case 15: {
				effect15(false, scene);
			} break;
			}
	}
	updateSceneArea();
}

void GraphicsManager::init() {
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			fadeData[i][j] = i / (j + 1);
}

void GraphicsManager::restoreBackground() {
	Common::copy(g_engine->_backgroundCopy + 4, g_engine->_backgroundCopy + g_engine->_screenSize, g_engine->_sceneBackground + 4);
}

} // End of namespace Tot
