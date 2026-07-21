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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/display.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/resources.h"

namespace Ripper {

IndexedDisplaySnapshot::IndexedDisplaySnapshot() {
}

bool IndexedDisplaySnapshot::capture() {
	return capture(Common::Rect(0, 0, kRipperScreenWidth, kRipperScreenHeight));
}

bool IndexedDisplaySnapshot::capture(const Common::Rect &bounds) {
	clear();
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || bounds.isEmpty() ||
			bounds.left < 0 || bounds.top < 0 || bounds.right > screen->w ||
			bounds.bottom > screen->h) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: could not capture indexed display bounds=%d,%d,%d,%d",
			bounds.left, bounds.top, bounds.right, bounds.bottom);
		return false;
	}

	_bounds = bounds;
	_pixels.resize((uint32)bounds.width() * bounds.height());
	for (int y = 0; y < bounds.height(); ++y) {
		memcpy(_pixels.data() + y * bounds.width(),
			screen->getBasePtr(bounds.left, bounds.top + y), bounds.width());
	}
	g_system->unlockScreen();

	_palette.resize(kRipperPaletteByteCount);
	g_system->getPaletteManager()->grabPalette(_palette.data(), 0, kRipperPaletteColorCount);
	debugC(2, kDebugVideo,
		"Ripper: captured indexed display bounds=%d,%d,%d,%d pixels=%u paletteEntries=%u",
		_bounds.left, _bounds.top, _bounds.right, _bounds.bottom, _pixels.size(),
		kRipperPaletteColorCount);
	return true;
}

bool IndexedDisplaySnapshot::restore(bool restorePaletteValue, bool updateScreen) const {
	if (!restorePixels())
		return false;
	if (restorePaletteValue && !restorePalette())
		return false;
	if (updateScreen)
		g_system->updateScreen();
	debugC(2, kDebugVideo,
		"Ripper: restored indexed display bounds=%d,%d,%d,%d palette=%d update=%d",
		_bounds.left, _bounds.top, _bounds.right, _bounds.bottom,
		restorePaletteValue, updateScreen);
	return true;
}

bool IndexedDisplaySnapshot::restorePixels() const {
	if (!isValid())
		return false;
	g_system->copyRectToScreen(_pixels.data(), _bounds.width(), _bounds.left,
		_bounds.top, _bounds.width(), _bounds.height());
	return true;
}

bool IndexedDisplaySnapshot::restorePalette() const {
	if (_palette.size() != kRipperPaletteByteCount)
		return false;
	g_system->getPaletteManager()->setPalette(_palette.data(), 0, kRipperPaletteColorCount);
	return true;
}

bool IndexedDisplaySnapshot::isValid() const {
	return !_bounds.isEmpty() &&
		_pixels.size() == (uint32)_bounds.width() * _bounds.height() &&
		_palette.size() == kRipperPaletteByteCount;
}

void IndexedDisplaySnapshot::clear() {
	_bounds = Common::Rect();
	_pixels.clear();
	_palette.clear();
}

uint BitmapFontRenderer::measureText(const BitmapFontAsset &font,
		const Common::String &text) {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			width += font.spaceWidth;
			continue;
		}
		if (character < font.firstCharacter ||
				character >= font.firstCharacter + font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = font.glyphs[character - font.firstCharacter];
		width += glyph.xOffset + glyph.width + font.characterSpacing;
	}
	return width;
}

static void drawBitmapFontText(byte *pixels, uint pitch, const BitmapFontAsset &font,
		int x, int y, const Common::String &text, byte color, const Common::Rect *clip) {
	int drawX = x;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			drawX += font.spaceWidth;
			continue;
		}
		if (character < font.firstCharacter ||
				character >= font.firstCharacter + font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = font.glyphs[character - font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte source = font.pixels[
					glyph.pixelOffset + glyphY * glyph.width + glyphX];
				const int targetX = drawX + glyph.xOffset + glyphX;
				const int targetY = y + glyph.yOffset + glyphY;
				if (source != font.transparentColor &&
						(!clip || (targetX >= clip->left && targetX < clip->right &&
						targetY >= clip->top && targetY < clip->bottom)))
					pixels[targetY * pitch + targetX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + font.characterSpacing;
	}
	debugC(11, kDebugVideo,
		"Ripper: rendered NF2T text characters=%u x=%d y=%d color=%u clipped=%d",
		text.size(), x, y, color, clip != nullptr);
}

void BitmapFontRenderer::drawText(byte *pixels, uint pitch, const BitmapFontAsset &font,
		int x, int y, const Common::String &text, byte color) {
	drawBitmapFontText(pixels, pitch, font, x, y, text, color, nullptr);
}

void BitmapFontRenderer::drawTextClipped(byte *pixels, uint pitch,
		const BitmapFontAsset &font, int x, int y, const Common::String &text,
		byte color, const Common::Rect &clip) {
	drawBitmapFontText(pixels, pitch, font, x, y, text, color, &clip);
}

} // End of namespace Ripper
