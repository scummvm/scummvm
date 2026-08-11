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

#ifndef HARVESTER_CFT_FONT_H
#define HARVESTER_CFT_FONT_H

#include "graphics/font.h"
#include "harvester/text.h"

namespace Harvester {

class HarvesterCftFont : public Graphics::Font {
public:
	explicit HarvesterCftFont(const CftFontResource &resource);

	bool isValid() const { return _maxCharWidth > 0 && _fontHeight > 0 && _drawHeight > 0; }

	int getFontHeight() const override { return _fontHeight; }
	Common::String getFontName() const override { return _resource.name; }
	int getMaxCharWidth() const override { return _maxCharWidth; }
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

private:
	struct GlyphSlice {
		int x = 0;
		int width = 0;
		bool valid = false;
	};

	const GlyphSlice *findGlyph(uint32 chr) const;

	const CftFontResource &_resource;
	GlyphSlice _glyphs[256];
	int _fontHeight = 0;
	int _drawHeight = 0;
	int _spaceWidth = 0;
	int _maxCharWidth = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_CFT_FONT_H
