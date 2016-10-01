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

#include "titanic/support/raw_surface.h"
#include "common/algorithm.h"

namespace Titanic {

CRawSurface::CRawSurface(Graphics::Surface *surface, TransparencyMode transMode) {
	_width = surface->w;
	_pixelsBaseP = (byte *)surface->getPixels();
	_pixelsP = nullptr;
	_pitch = 0;
	_fieldC = 0;
	_field10 = false;
	_field18 = 0;
	_field1C = 0xFF;

	switch (transMode) {
	case TRANS_MASK0:
	case TRANS_ALPHA0:
		_field1C = 0;
		_field18 = 0xFF;
		break;
	case TRANS_MASK255:
	case TRANS_ALPHA255:
		_field1C = 0xFF;
		_field18 = 0;
		break;
	case TRANS_DEFAULT:
		if ((_pixelsBaseP[0] == 0 && _pixelsBaseP[2] < 0x80) ||
				(_pixelsBaseP[0] != 0 && _pixelsBaseP[1] < 0x80)) {
			_field18 = 0xFF;
			_field1C = 0;
		}
		break;
	default:
		break;
	}
}

void CRawSurface::setRow(int yp) {
	for (int y = 0; y < yp; ++yp) {
		resetPitch();
		skipPitch();
	}
}

void CRawSurface::setCol(int xp) {
	while (xp > 0)
		xp -= moveX(xp);
}

void CRawSurface::skipPitch() {
	setCol(_pitch);
}

int CRawSurface::moveX(int xp) {
	if (_fieldC) {
		if (!_field10) {
			--_fieldC;
			--_pitch;
			++_pixelsP;
			return 1;
		}
	} else {
		while (!*_pixelsBaseP) {
			_fieldC = *++_pixelsBaseP;
			++_pixelsBaseP;

			if (_fieldC) {
				_pixelsP = _pixelsBaseP;
				_pixelsBaseP += _fieldC;
				if (_fieldC & 1)
					++_pixelsBaseP;

				_field10 = 0;
				--_pitch;
				--_fieldC;
				return 1;
			}
		}

		_fieldC = *_pixelsBaseP++;
		++_pixelsBaseP;
		_field10 = true;
	}

	if (xp < 0 || xp > _pitch)
		xp = _pitch;

	int len = MIN(_fieldC, xp);
	_pitch -= len;
	_fieldC -= len;
	return len;
}

uint CRawSurface::getPixel() const {
	return _field18 ? 0xFF - *_pixelsP : *_pixelsP;
}

bool CRawSurface::isPixelTransparent1() const {
	return _field18 ? *_pixelsP == 0xF0 : *_pixelsP == 0x10;
}

bool CRawSurface::isPixelTransparent2() const {
	return _field18 ? *_pixelsP == 0xF0 : *_pixelsP == 0x10;
}

void CRawSurface::resetPitch() {
	_pitch = _width;
}

} // End of namespace Titanic
