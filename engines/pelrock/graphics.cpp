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
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "graphics.h"
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
			buf[index] = g_engine->_room->_paletteRemaps[0][buf[index]];
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

void GraphicsManager::fadeToBlack(int stepSize) {
	byte palette[768];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	while (!g_engine->shouldQuit()) {
		g_engine->_events->pollEvent();
		g_engine->_chrono->updateChrono();
		if (g_engine->_chrono->_gameTick) {

			for (int i = 0; i < 768; i++) {
				if (palette[i] > 0) {
					palette[i] = MAX(palette[i] - stepSize, 0);
				}
			}
			g_system->getPaletteManager()->setPalette(palette, 0, 256);

			bool allBlack = true;
			for (int i = 0; i < 768; i++) {
				if (palette[i] != 0) {
					allBlack = false;
				}
			}

			if (allBlack) {
				break;
			}

			g_engine->_screen->markAllDirty();
			g_engine->_screen->update();
		}
		g_system->delayMillis(10);
	}
}

/**
 * Fades between two palettes by incrementally changing the current palette towards the target palette.
 */
void GraphicsManager::fadePaletteToTarget(byte *targetPalette, int stepSize) {
	byte currentPalette[768];
	memcpy(currentPalette, g_engine->_room->_roomPalette, 768);

	while (!g_engine->shouldQuit()) {
		g_engine->_events->pollEvent();

		bool didRender = g_engine->renderScene(OVERLAY_NONE);
		if (didRender) {
			bool changed = false;

			for (int i = 0; i < 768; i++) {
				if (currentPalette[i] < targetPalette[i]) {
					currentPalette[i] = MIN((int)currentPalette[i] + stepSize, (int)targetPalette[i]);
					changed = true;
				} else if (currentPalette[i] > targetPalette[i]) {
					currentPalette[i] = MAX((int)currentPalette[i] - stepSize, (int)targetPalette[i]);
					changed = true;
				}
			}

			if (!changed)
				break;

			g_system->getPaletteManager()->setPalette(currentPalette, 0, 256);
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	memcpy(g_engine->_room->_roomPalette, targetPalette, 768);
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
}

void GraphicsManager::clearScreen() {
	memset(g_engine->_screen->getPixels(), 0, g_engine->_screen->pitch * g_engine->_screen->h);
}

void GraphicsManager::drawColoredText(Graphics::ManagedSurface *screen, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font) {
	int currentX = x;

	Common::String segment;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '@' && i + 1 < text.size()) {
			// Draw accumulated segment
			if (!segment.empty()) {
				font->drawString(screen, segment, currentX, y, w, defaultColor);
				currentX += font->getStringWidth(segment);
				segment.clear();
			}
			defaultColor = text[i + 1];
			i++; // skip color code
		} else {
			segment += text[i];
		}
	}

	// Draw remaining segment
	if (!segment.empty()) {
		font->drawString(screen, segment, currentX, y, w, defaultColor);
	}
}

void GraphicsManager::drawColoredText(byte *buf, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font) {

	Graphics::Surface tempSurface;
	Common::Rect r = font->getBoundingBox(text); // Ensure font metrics are loaded before creating surface

	tempSurface.create(r.width(), r.height(), Graphics::PixelFormat::createFormatCLUT8());

	int currentX = x;

	Common::String segment;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '@' && i + 1 < text.size()) {
			// Draw accumulated segment
			if (!segment.empty()) {
				font->drawString(&tempSurface, segment, currentX, y, w, defaultColor);
				currentX += font->getStringWidth(segment);
				segment.clear();
			}
			defaultColor = text[i + 1];
			i++; // skip color code
		} else {
			segment += text[i];
		}
	}

	// Draw remaining segment
	if (!segment.empty()) {
		font->drawString(&tempSurface, segment, currentX, y, w, defaultColor);
	}

	for(int j = 0; j < tempSurface.h; j++) {
		for(int i = 0; i < tempSurface.w; i++) {
			int idx = j * tempSurface.w + i;
			if (y + j < 400 && x + i < 640) {
				byte pixel = *((byte *)tempSurface.getBasePtr(i, j));
				if (pixel != 0) { // Assuming 0 is transparent
					debug("Drawing pixel at (%d, %d) with color %d", x + i, y + j, pixel);
					buf[(y + j) * 640 + (x + i)] = pixel;
				}
			}
		}
	}
}

void GraphicsManager::drawColoredTexts(Graphics::ManagedSurface *surface, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for(int i =0; i < text.size(); i++) {
		drawColoredText(surface, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

void GraphicsManager::drawColoredTexts(byte *buf, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for(int i =0; i < text.size(); i++) {
		drawColoredText(buf, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

} // End of namespace Pelrock
