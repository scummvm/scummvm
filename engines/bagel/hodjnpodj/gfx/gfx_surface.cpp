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

#include "common/file.h"
#include "common/queue.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/gfx_surface.h"
#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

#define CONVERT_COLOR \
	if (IS_RGB(color)) { \
		assert(_owner); \
		color = _owner->getPaletteIndex(color); \
	}

void GfxSurface::floodFill(int x, int y, int color) {
	CONVERT_COLOR

	if (x < 0 || y < 0 || x >= this->w || y >= this->h)
		return;

	byte *pixel = (byte *)getBasePtr(x, y);
	const byte oldColor = *pixel;
	int cx, cy;
	int minX = 9999, maxX = -1, minY = 9999, maxY = -1;

	// TODO: Temporary hack for Huge, to ensure floodFill
	// isn't called for erasing a brick that isn't present
	assert(oldColor != 255);

	Common::Queue<Common::Pair<int, int>> queue;
	queue.push({ x, y });

	while (!queue.empty()) {
		cx = queue.front().first;
		cy = queue.front().second;
		queue.pop();

		// Check bounds and color match
		if (cx < 0 || cx >= this->w || cy < 0 || cy >= this->h)
			continue;
		pixel = (byte *)getBasePtr(cx, cy);
		if (*pixel != oldColor)
			continue;

		// Set new color
		*pixel = color;

		// Keep track of the modified area
		minX = MIN(minX, cx);
		maxX = MAX(maxX, cx);
		minY = MIN(minY, cy);
		maxY = MAX(maxY, cy);

		// Push neighboring pixels
		queue.push({ cx + 1, cy });
		queue.push({ cx - 1, cy });
		queue.push({ cx, cy + 1 });
		queue.push({ cx, cy - 1 });
	}

	addDirtyRect(Common::Rect(minX, minY, maxX + 1, maxY + 1));
}

void GfxSurface::loadBitmap(const char *filename) {
	Common::File f;
	Image::BitmapDecoder decoder;
	if (!f.open(filename) || !decoder.loadStream(f))
		error("Could not load bitmap - %s", filename);

	// Get the bitmap
	copyFrom(decoder.getSurface());
	setPalette(decoder.getPalette(), 0, decoder.getPaletteColorCount());

	// Convert it to the currently active game palette
	byte destPal[PALETTE_SIZE];
	g_system->getPaletteManager()->grabPalette(destPal, 0, PALETTE_COUNT);
	convertTo(destPal);
}

void GfxSurface::convertTo(const byte *palette, int count) {
	assert(hasPalette());
	const Graphics::Palette *surfacePal = grabPalette();

	// Create a lookup between the bitmap palette and game palette
	Graphics::PaletteLookup lookup(palette, count);
	uint32 *map = lookup.createMap(surfacePal->data(), PALETTE_COUNT);

	if (map) {
		// Translate the pixels using the lookup
		byte *pixel = (byte *)getPixels();
		for (int i = 0; i < this->w * this->h; ++i, ++pixel)
			*pixel = map[*pixel];

		delete[] map;
	}

	// Now we're using the game palette, don't maintain
	// the surface-specific palette any longer
	clearPalette();
}

void GfxSurface::loadCels(const char *filename, size_t numCells) {
	// First load the bitmap
	loadBitmap(filename);
	_cellsSource = *this;

	assert((this->w % numCells) == 0);
	_cellWidth = this->w / numCells;

	setCel(0);
}

void GfxSurface::setCel(size_t cellNum) {
	assert(_cellWidth != 0);
	assert((int)(cellNum * _cellWidth) < _cellsSource.w);

	_cellIndex = cellNum;
	Common::Rect r(0, 0, _cellWidth, this->h);
	r.moveTo(_cellWidth * cellNum, 0);
	Graphics::ManagedSurface::operator=(
		Graphics::ManagedSurface(_cellsSource, r));
}

void GfxSurface::writeString(const Common::String &text, const Common::Point &pos,
		int color) {
	CONVERT_COLOR

	Graphics::WinFont &font = g_engine->_fonts[_fontSize];
	font.drawString(this, text, pos.x, pos.y, this->w - pos.x,
		color);
}

void GfxSurface::writeString(const Common::String &text,
		const Common::Rect &bounds, int color,
		Graphics::TextAlign justify) {
	CONVERT_COLOR

	Graphics::WinFont &font = g_engine->_fonts[_fontSize];
	font.drawString(this, text, bounds.left, bounds.top,
		bounds.width(), color, justify);
}

void GfxSurface::writeShadowedString(const Common::String &text,
		const Common::Point &pos, int color) {
	CONVERT_COLOR

	writeString(text, Common::Point(pos.x + 1, pos.y), BLACK);
	writeString(text, pos, color);
}

void GfxSurface::writeShadowedString(const Common::String &text,
		const Common::Rect &bounds, int color,
		Graphics::TextAlign justify) {
	CONVERT_COLOR

	Common::Rect temp(bounds.left + 1, bounds.top, bounds.right + 1, bounds.bottom);
	writeString(text, temp, BLACK, justify);
	writeString(text, bounds, color, justify);
}


size_t GfxSurface::getStringWidth(const Common::String &text) const {
	Graphics::WinFont &font = g_engine->_fonts[_fontSize];
	return font.getStringWidth(text);
}

size_t GfxSurface::getStringHeight() const {
	Graphics::WinFont &font = g_engine->_fonts[_fontSize];
	return font.getFontHeight();
}

int GfxSurface::wordWrapText(const Common::String &str,
		Common::StringArray &lines) const {
	Graphics::WinFont &font = g_engine->_fonts[_fontSize];
	return font.wordWrapText(str, this->w, lines);
}

void GfxSurface::hLine(int x, int y, int x2, uint32 color) {
	CONVERT_COLOR
	Graphics::ManagedSurface::hLine(x, y, x2, color);
}

void GfxSurface::vLine(int x, int y, int y2, uint32 color) {
	CONVERT_COLOR
	Graphics::ManagedSurface::vLine(x, y, y2, color);
}

void GfxSurface::fillRect(Common::Rect r, uint32 color) {
	CONVERT_COLOR
	fillRect(r, color);
}

void GfxSurface::frameRect(const Common::Rect &r, uint32 color) {
	CONVERT_COLOR
	frameRect(r, color);
}

/*------------------------------------------------------------------------*/

bool Sprite::isLinked() const {
	for (auto it = _minigame->_linkedSprites.begin();
		it != _minigame->_linkedSprites.end(); ++it) {
		if (*it == this)
			return true;
	}

	return false;
}

void Sprite::linkSprite() {
	if (!isLinked())
		_minigame->_linkedSprites.push_back(this);
}

void Sprite::unlinkSprite() {
	_minigame->_linkedSprites.remove(this);
}

} // namespace HodjNPodj
} // namespace Bagel
