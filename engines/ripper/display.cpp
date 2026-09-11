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
#include "ripper/ripper.h"
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
		presentScreen();
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

bool IndexedBitmapRenderer::drawBitmap(byte *pixels, uint pitch,
		const BitmapAssetFrame &bitmap, int x, int y,
		const Common::Rect &clip) {
	const uint32 expectedPixels = (uint32)bitmap.width * bitmap.height;
	if (!pixels || pitch == 0 || bitmap.width == 0 || bitmap.height == 0 ||
			bitmap.pixels.size() != expectedPixels || clip.isEmpty() ||
			clip.left < 0 || clip.top < 0 || (uint)clip.right > pitch)
		return false;

	const int64 bitmapRight = (int64)x + bitmap.width;
	const int64 bitmapBottom = (int64)y + bitmap.height;
	if (bitmapRight <= clip.left || bitmapBottom <= clip.top ||
			x >= clip.right || y >= clip.bottom)
		return true;

	const int destinationLeft = MAX<int64>(x, clip.left);
	const int destinationTop = MAX<int64>(y, clip.top);
	const int destinationRight = MIN<int64>(bitmapRight, clip.right);
	const int destinationBottom = MIN<int64>(bitmapBottom, clip.bottom);
	for (int destinationY = destinationTop; destinationY < destinationBottom;
			++destinationY) {
		const uint sourceY = destinationY - y;
		for (int destinationX = destinationLeft;
				destinationX < destinationRight; ++destinationX) {
			const uint sourceX = destinationX - x;
			const byte pixel = bitmap.pixels[sourceY * bitmap.width + sourceX];
			if (pixel != bitmap.transparentColor)
				pixels[destinationY * pitch + destinationX] = pixel;
		}
	}
	return true;
}

bool IndexedBitmapRenderer::drawNineSlice(byte *pixels, uint pitch,
		const Common::Array<BitmapAssetFrame> &skin,
		const Common::Rect &bounds, const Common::Rect &clip) {
	if (skin.size() < 9 || skin[0].width == 0 || skin[0].height == 0 ||
			bounds.isEmpty())
		return false;

	const int columns = (bounds.width() + skin[0].width - 1) / skin[0].width;
	const int rows = (bounds.height() + skin[0].height - 1) / skin[0].height;
	int y = bounds.top;
	for (int row = 0; row < rows; ++row) {
		int x = bounds.left;
		const BitmapAssetFrame *lastTile = nullptr;
		for (int column = 0; column < columns; ++column) {
			const uint columnBand = column == 0 ? 0 :
				(column == columns - 1 ? 2 : 1);
			const uint rowBand = row == 0 ? 0 :
				(row == rows - 1 ? 2 : 1);
			const BitmapAssetFrame &tile = skin[rowBand * 3 + columnBand];
			if (!drawBitmap(pixels, pitch, tile, x, y, clip))
				return false;
			lastTile = &tile;
			// TileChooserControlFrame at 0x54fbe snaps the final column and
			// row after rendering their penultimate tiles.
			if (column == columns - 2)
				x = bounds.right - tile.width;
			else
				x += tile.width;
		}
		if (!lastTile)
			continue;
		if (row == rows - 2)
			y = bounds.bottom - lastTile->height;
		else
			y += lastTile->height;
	}
	return true;
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
