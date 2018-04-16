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

#include "bladerunner/boundingbox.h"

#include "bladerunner/savefile.h"

namespace BladeRunner {

BoundingBox::BoundingBox(float x0, float y0, float z0, float x1, float y1, float z1) {
	_vertices[0].x = x0;
	_vertices[0].y = y0;
	_vertices[0].z = z0;

	_vertices[1].x = x1;
	_vertices[1].y = y1;
	_vertices[1].z = z1;
}

void BoundingBox::expand(float x0, float y0, float z0, float x1, float y1, float z1) {
	_vertices[0].x += x0;
	_vertices[0].y += y0;
	_vertices[0].z += z0;

	_vertices[1].x += x1;
	_vertices[1].y += y1;
	_vertices[1].z += z1;
}

bool BoundingBox::inside(float x, float y, float z) const {
	return x >= _vertices[0].x && x <= _vertices[1].x
	    && y >= _vertices[0].y && y <= _vertices[1].y
	    && z >= _vertices[0].z && z <= _vertices[1].z;
}

bool BoundingBox::inside(Vector3 &position) const {
	return inside(position.x, position.y, position.z);
}

void BoundingBox::setXYZ(float x0, float y0, float z0, float x1, float y1, float z1) {
	_vertices[0].x = x0;
	_vertices[0].y = y0;
	_vertices[0].z = z0;

	_vertices[1].x = x1;
	_vertices[1].y = y1;
	_vertices[1].z = z1;
}

void BoundingBox::getXYZ(float *x0, float *y0, float *z0, float *x1, float *y1, float *z1) const {
	*x0 = _vertices[0].x;
	*y0 = _vertices[0].y;
	*z0 = _vertices[0].z;

	*x1 = _vertices[1].x;
	*y1 = _vertices[1].y;
	*z1 = _vertices[1].z;
}

float BoundingBox::getZ0() const {
	return _vertices[0].z;
}

float BoundingBox::getZ1() const {
	return _vertices[1].z;
}

} // End of namespace BladeRunner
