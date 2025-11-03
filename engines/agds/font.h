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

#ifndef FONT_H
#define FONT_H

#include "common/ptr.h"
#include "graphics/font.h"

namespace Graphics {
class ManagedSurface;
}

namespace AGDS {

class Font : public Graphics::Font {
	Common::ScopedPtr<Graphics::ManagedSurface> _surface;
	int _glyphW, _glyphH;
	int _cellW, _cellH;
	uint8 _width[0x100];

public:
	Font(Graphics::ManagedSurface *surface, int gw, int gh);

	virtual int getFontHeight() const {
		return _glyphH;
	}

	virtual int getMaxCharWidth() const {
		return _glyphW;
	}

	virtual int getCharWidth(uint32 chr) const {
		return chr < 0x100 ? _width[chr] : 0;
	}

	virtual void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;
};

} // End of namespace AGDS

#endif /* AGDS_FONT_H */
