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

#ifndef TITANIC_RAW_SURFACE_H
#define TITANIC_RAW_SURFACE_H

#include "graphics/surface.h"

namespace Titanic {

enum TransparencyMode {
	TRANS_MASK0 = 0, TRANS_MASK255 = 1, TRANS_ALPHA0 = 2,
	TRANS_ALPHA255 = 3, TRANS_DEFAULT = 4
};

class CRawSurface {
private:
	const byte *_pixelsBaseP;
	const byte *_pixelsP;
	int _pitch;
	int _runLength;
	bool _flag;
	int _width;
	bool _flag1;
	bool _flag2;
public:
	CRawSurface(const Graphics::Surface *surface, TransparencyMode transMode);

	void setRow(int yp);

	void setCol(int xp);

	void skipPitch();

	uint getPixel() const;

	bool isPixelTransparent1() const;

	bool isPixelTransparent2() const;

	void resetPitch();

	int moveX(int xp);
};

} // End of namespace Titanic

#endif /* TITANIC_RAW_SURFACE_H */
