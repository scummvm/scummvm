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

#ifndef TITANIC_TRANSPARENCY_SURFACE_H
#define TITANIC_TRANSPARENCY_SURFACE_H

#include "common/rect.h"
#include "graphics/surface.h"

namespace Titanic {

enum TransparencyMode {
	TRANS_MASK0 = 0, TRANS_MASK255 = 1, TRANS_ALPHA0 = 2,
	TRANS_ALPHA255 = 3, TRANS_DEFAULT = 4
};

class CTransparencySurface {
private:
	const Graphics::Surface *_surface;
	Common::Point _pos;
	int _pitch;
	int _runLength;
	bool _flag;
	byte _transparentColor;
	byte _opaqueColor;
private:
	/**
	* Returns a a pixel from the transparency surface
	*/
	inline uint getPixel() const {
		byte pixel = *(const byte *)_surface->getBasePtr(_pos.x, _pos.y);
		return pixel;
	}
public:
	/**
	 * Constructor
	 */
	CTransparencySurface(const Graphics::Surface *surface, TransparencyMode transMode);

	/**
	 * Sets the row to get transparencies from
	 */
	inline void setRow(int yp) { _pos.y = yp; }

	/**
	 * Sets the column to get transparencies from
	 */
	inline void setCol(int xp) { _pos.x = xp; }

	/**
	 * Moves reading position horizontally by a single pixel
	 */
	inline int moveX() {
		if (++_pos.x >= _surface->w) {
			_pos.x = 0;
			++_pos.y;
		}

		return 1;
	}

	/**
	 * Returns the alpha value for the pixel (0-31)
	 */
	inline uint getAlpha() const {
		byte pixel = getPixel();
		return _opaqueColor ? 0xFF - pixel : pixel;
	}

	/**
	 * Returns true if the pixel is opaque
	 */
	inline bool isPixelOpaque() const {
		byte pixel = getPixel();
		return _opaqueColor ? pixel >= 0xf0 : pixel < 0x10;
	}

	/**
	 * Returns true if the pixel is completely transparent
	 */
	inline bool isPixelTransparent() const {
		byte pixel = getPixel();
		return _transparentColor ? pixel >= 0xf0 : pixel < 0x10;
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TRANSPARENCY_SURFACE_H */
