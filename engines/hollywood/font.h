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

#ifndef HOLLYWOOD_FONT_H
#define HOLLYWOOD_FONT_H

#include "common/array.h"
#include "common/path.h"
#include "common/types.h"
#include "graphics/font.h"

namespace Hollywood {

class HollywoodFont : public Graphics::Font {
public:
	HollywoodFont();

	bool load();
	bool isLoaded() const { return _loaded; }

	void setShadowColor(byte color) { _shadowColor = color; }

	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

private:
	struct GlyphDescriptor {
		uint16 offset;
		byte yOffset;
		byte height;
		byte width;
	};

	byte mapCharacter(uint32 chr) const;
	const GlyphDescriptor *getGlyph(uint32 chr) const;
	bool loadCharacterMap(const Common::Path &exeName);
	void loadFallbackCharacterMap();
	bool isValidCharacterMap(const Common::Array<byte> &characterMap) const;
	bool readPeDataAtVa(const Common::Path &exeName, uint32 virtualAddress, Common::Array<byte> &destination) const;

	Common::Array<byte> _characterMap;
	Common::Array<byte> _glyphPixels;
	Common::Array<GlyphDescriptor> _glyphs;
	byte _shadowColor;
	int _maxCharWidth;
	bool _loaded;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_FONT_H
