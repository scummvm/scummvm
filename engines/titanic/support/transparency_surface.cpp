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

#include "titanic/support/transparency_surface.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Titanic {

CTransparencySurface::CTransparencySurface(const Graphics::Surface *surface,
		TransparencyMode transMode) : _surface(surface) {
	_pitch = 0;
	_runLength = 0;
	_flag = false;
	_flag1 = false;
	_flag2 = true;

	switch (transMode) {
	case TRANS_MASK0:
	case TRANS_ALPHA0:
		_flag2 = false;
		_flag1 = true;
		break;
	case TRANS_MASK255:
	case TRANS_ALPHA255:
		_flag2 = true;
		_flag1 = false;
		break;
	case TRANS_DEFAULT:
		error("TRANS_DEFAULT not supported in transparency surface");
	default:
		break;
	}
}

int CTransparencySurface::moveX() {
	if (++_pos.x >= _surface->w) {
		_pos.x = 0;
		++_pos.y;
	}

	return 1;
}

uint CTransparencySurface::getPixel() const {
	const byte *pixelP = (const byte *)_surface->getBasePtr(_pos.x, _pos.y);
	return _flag1 ? 0xFF - *pixelP : *pixelP;
}

bool CTransparencySurface::isPixelTransparent1() const {
	const byte *pixelP = (const byte *)_surface->getBasePtr(_pos.x, _pos.y);
	return _flag1 ? *pixelP == 0xF0 : *pixelP == 0x10;
}

bool CTransparencySurface::isPixelTransparent2() const {
	const byte *pixelP = (const byte *)_surface->getBasePtr(_pos.x, _pos.y);
	return _flag2 ? *pixelP == 0xF0 : *pixelP == 0x10;
}

} // End of namespace Titanic
