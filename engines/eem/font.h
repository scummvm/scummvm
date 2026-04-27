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

#ifndef EEM_FONT_H
#define EEM_FONT_H

#include "common/array.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
class ManagedSurface;
}

namespace EEM {

/// One bitmap glyph, 1 bit per pixel, MSB first.
struct FontGlyph {
	uint8 height    = 0;
	uint8 widthBits = 0;
	uint8 sizeBytes = 0;
	Common::Array<byte> bitmap;
};

/**
 * Loader for the engine's `.FNT` files (FONT.FNT, SYSTEM.FNT, TINY.FNT,
 * 8PNTTHIN.FNT). Mirrors `_LoadFont` @ 1b66:023c.
 *
 * File layout:
 *   - u16 numChars
 *   - per char: u8 height, u8 widthBits, u8 sizeBytes, bytes[sizeBytes] bitmap
 *
 * Drawing mirrors `_ShowChar` @ 1b66:0346: each set bit becomes `fontColor`
 * on the destination surface; clear bits are transparent.
 */
class EEMFont {
public:
	EEMFont() = default;

	bool load(const Common::Path &path);

	uint16 height() const { return _maxHeight; }

	int charWidth(byte c) const;

	/// Total pixel width of @p s when rendered (no shadow).
	int stringWidth(const Common::String &s) const;

	/// Draw @p c at (@p x, @p y) on @p dst with foreground @p color.
	/// Returns the advance width.
	int drawChar(Graphics::ManagedSurface *dst, int x, int y, byte c, byte color) const;

	/// Draw @p s at (@p x, @p y) and return total advance width.
	int drawString(Graphics::ManagedSurface *dst, int x, int y,
				   const Common::String &s, byte color) const;

	/// Word-wrap @p s into the rect (x..x+width, y..) and draw line by line.
	/// Mirrors the data flow of `_DoWordWrap` @ 1b66:04a7.
	int drawWordWrapped(Graphics::ManagedSurface *dst, int x, int y, int width,
						const Common::String &s, byte color) const;

	bool isLoaded() const { return !_glyphs.empty(); }

private:
	Common::Array<FontGlyph> _glyphs;
	uint16 _maxHeight = 0;
};

} // End of namespace EEM

#endif
