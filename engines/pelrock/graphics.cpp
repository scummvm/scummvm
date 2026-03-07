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

Common::Point GraphicsManager::showOverlay(int height, Graphics::ManagedSurface &buf) {
	int overlayY = 400 - height;
	int overlayX = 0;
	for (int x = 0; x < 640; x++) {
		for (int y = overlayY; y < 400; y++) {
			byte pixel = (byte)buf.getPixel(x, y);
			buf.setPixel(x, y, g_engine->_room->_paletteRemaps[2][pixel]);
		}
	}
	return Common::Point(overlayX, overlayY);
}

byte *GraphicsManager::grabBackgroundSlice(Graphics::ManagedSurface &buf, int x, int y, int w, int h) {
	byte *bg = new byte[w * h];
	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			int idx = i * w + j;
			if (y + i < 400 && x + j < 640) {
				*(bg + idx) = (byte)buf.getPixel(x + j, y + i);
			}
		}
	}
	return bg;
}

void GraphicsManager::putBackgroundSlice(Graphics::ManagedSurface &buf, int x, int y, int w, int h, byte *slice) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = (j * w + i);
			if (x + i < 640 && y + j < 400) {
				buf.setPixel(x + i, y + j, slice[index]);
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
	g_engine->_screen->clear(0);
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

void GraphicsManager::drawColoredText(Graphics::ManagedSurface &buf, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font) {

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

	// Use transBlitFrom to blit non-zero pixels
	buf.transBlitFrom(tempSurface, Common::Point(x, y), 0);
	tempSurface.free();
}

void GraphicsManager::drawColoredTexts(Graphics::ManagedSurface *surface, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for (uint i = 0; i < text.size(); i++) {
		drawColoredText(surface, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

void GraphicsManager::drawColoredTexts(Graphics::ManagedSurface &buf, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font) {
	int currentX = x;
	byte currentColor = 255;

	for (uint i = 0; i < text.size(); i++) {
		drawColoredText(buf, text[i], currentX, y + i * (font->getFontHeight() + yPadding), w, currentColor, font);
	}
}

} // End of namespace Pelrock
