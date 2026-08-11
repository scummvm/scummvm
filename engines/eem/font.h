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

#include "graphics/font.h"

namespace Graphics {
class MacFONTFont;
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
 * Loader for .FNT files (FONT.FNT, SYSTEM.FNT, TINY.FNT, 8PNTTHIN.FNT).
 * _LoadFont @ 1b66:023c.
 *
 * Layout: u16 numChars, then per char u8 height, u8 widthBits,
 * u8 sizeBytes, bytes[sizeBytes] bitmap.
 *
 * Char → glyph table extracted from CHR2FNT (29b6:0000).
 */
class EEMFont : public Graphics::Font {
public:
	EEMFont() = default;
	~EEMFont() override;

	bool load(const Common::Path &path);
	bool loadMacResource(const Common::Path &path, uint16 resourceId, int size);
	bool isLoaded() const { return _macFont || !_glyphs.empty(); }

	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y,
				  uint32 color) const override;
	using Graphics::Font::drawChar;  // keep ManagedSurface overload

	/// Wrap @p s to @p width via the inherited `wordWrapText` and draw
	/// each line at (x, y) downward. Returns total height drawn.
	int drawWordWrapped(Graphics::ManagedSurface *dst, int x, int y,
						int width, const Common::String &s, uint32 color) const;

private:
	void clear();

	Common::Array<FontGlyph> _glyphs;
	Graphics::MacFONTFont *_macFont = nullptr;
	uint16 _maxHeight  = 0;
	uint16 _maxWidth   = 0;
	uint16 _lineHeight = 0;  ///< First glyph height (original line stride)
};

} // End of namespace EEM

#endif
