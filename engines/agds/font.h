/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGDS_FONT_H
#define AGDS_FONT_H

#include "graphics/font.h"

namespace Graphics {
	struct TransparentSurface;
}

namespace AGDS {

class Font : public Graphics::Font {
	Graphics::TransparentSurface * _surface;
	int _gw, _gh;

public:
	Font(Graphics::TransparentSurface *surface, int gw, int gh);

	virtual int getFontHeight() const {
		return _gh;
	}

	virtual int getMaxCharWidth() const {
		return _gw;
	}

	virtual int getCharWidth(uint32 chr) const {
		return _gw;
	}

	virtual void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;

};

} // End of namespace AGDS

#endif /* AGDS_FONT_H */
