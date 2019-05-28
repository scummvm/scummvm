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

#ifndef BLADERUNNER_BOUNDING_BOX_H
#define BLADERUNNER_BOUNDING_BOX_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class SaveFileWriteStream;

class BoundingBox {
	Vector3 _vertices[2];

public:
	BoundingBox() {}
	BoundingBox(float x0, float y0, float z0, float x1, float y1, float z1);

	void expand(float x0, float y0, float z0, float x1, float y1, float z1);
	bool inside(float x, float y, float z) const;
	bool inside(Vector3 &position) const;

	void setXYZ(float x0, float y0, float z0, float x1, float y1, float z1);
	void getXYZ(float *x0, float *y0, float *z0, float *x1, float *y1, float *z1) const;

	float getZ0() const;
	float getZ1() const;
};

} // End of namespace BladeRunner

#endif
