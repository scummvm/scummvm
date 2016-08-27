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

#include "titanic/star_control/surface_area.h"

namespace Titanic {

CSurfaceArea::CSurfaceArea(CVideoSurface *surface) {
	_width = surface->getWidth();
	_height = surface->getHeight();
	_pitch = surface->getPitch();

	// Original supported other pixel depths
	_bpp = surface->getPixelDepth();
	assert(_bpp == 2);
	_pixelsPtr = surface->getPixels();

	initialize();
}

void CSurfaceArea::initialize() {
	_bounds = Rect(0, 0, _width - 1, _height - 1);
	_centroid = Point(_width / 2, _height / 2);
	_field22 = _field21 = _field20 = 0xFF;
	_field27 = _field26 = _field25 = 0;
	_field24 = _field23 = 0;
	_field28 = _field2C = 0;
	_field38 = 0;
}

} // End of namespace Titanic
