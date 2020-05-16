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

#ifndef ULTIMA8_GRAPHICS_FONTS_RENDEREDTEXT_H
#define ULTIMA8_GRAPHICS_FONTS_RENDEREDTEXT_H

#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderSurface;

class RenderedText {
public:
	RenderedText();
	virtual ~RenderedText();

	//! Draw self to a rendersurface.
	//! \param surface The surface to draw to
	//! \param x X coordinate of target
	//! \param y Y coordinate of target. This will be the top baseline.
	virtual void draw(RenderSurface *surface, int x, int y, bool destmasked = false) = 0;

	//! Draw self to a rendersurface blended (0xAABBGGRR, alpha is blend level)
	virtual void drawBlended(RenderSurface *surface, int x, int y, uint32 col, bool destmasked = false) = 0;

	//! Get dimensions.
	//! \param x Returns the width
	//! \param y Returns the height
	virtual void getSize(int &x, int &y) const {
		x = _width;
		y = _height;
	}

	//! Get vlead
	virtual int getVlead() {
		return _vLead;
	}

protected:
	int _width, _height;
	int _vLead;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
