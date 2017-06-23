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
	uint getPixel() const;
public:
	/**
	 * Constructor
	 */
	CTransparencySurface(const Graphics::Surface *surface, TransparencyMode transMode);

	/**
	 * Sets the row to get transparencies from
	 */
	void setRow(int yp) { _pos.y = yp; }

	/**
	 * Sets the column to get transparencies from
	 */
	void setCol(int xp) { _pos.x = xp; }

	/**
	 * Moves reading position horizontally by a single pixel
	 */
	int moveX();

	/**
	 * Returns the alpha value for the pixel (0-31)
	 */
	uint getAlpha() const;

	/**
	 * Returns true if the pixel is opaque
	 */
	bool isPixelOpaque() const;

	/**
	 * Returns true if the pixel is completely transparent
	 */
	bool isPixelTransparent() const;
};

} // End of namespace Titanic

#endif /* TITANIC_TRANSPARENCY_SURFACE_H */
