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
	_opaqueColor = 0;
	_transparentColor = 0xff;

	switch (transMode) {
	case TRANS_MASK0:
	case TRANS_ALPHA0:
		_transparentColor = 0;
		_opaqueColor = 0xff;
		break;
	case TRANS_MASK255:
	case TRANS_ALPHA255:
		_transparentColor = 0xff;
		_opaqueColor = 0;
		break;
	case TRANS_DEFAULT:
		// If top left pixel is low, then 0 is the transparent color
		if (*(const byte *)surface->getPixels() < 0x80) {
			_opaqueColor = 0xff;
			_transparentColor = 0;
		}
		break;
	default:
		break;
	}
}

} // End of namespace Titanic
