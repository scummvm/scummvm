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

#ifndef NGI_FONT_H
#define NGI_FONT_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/ptr.h"
#include "graphics/font.h"

namespace NGI {

class Texture;

class BitmapFont : public Graphics::Font {
public:
	bool load(const Common::Path &path);
	void drawCentered(Graphics::ManagedSurface &target, const Common::Rect &bounds, const Common::String &text) const;

	int getFontHeight() const override { return _lineHeight; }
	int getMaxCharWidth() const override { return _maxWidth; }
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override;
	Common::Rect getBoundingBox(uint32 chr) const override;
	void drawChar(Graphics::Surface *target, uint32 chr, int x, int y, uint32 color) const override;
	void drawChar(Graphics::ManagedSurface *target, uint32 chr, int x, int y, uint32 color) const override;

private:
	class Loader;
	struct Glyph {
		Common::Rect source;
		Common::Point offset;
		int advance = 0;
		int page = 0;
	};
	const Glyph &glyph(uint32 chr) const;
	int lineHeight(const Common::String &line) const;
	Common::HashMap<int, Glyph> _glyphs;
	Common::HashMap<int, int> _kerning;
	Common::Array<Common::SharedPtr<Texture>> _pages;
	Common::Point _spacing;
	int _outline = 0;
	int _extraLineSpacing = 0;
	int _lineHeight = 0;
	int _maxWidth = 0;
};

} // End of namespace NGI

#endif
