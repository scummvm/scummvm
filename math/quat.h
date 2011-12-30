/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

// Quaternion-math borrowed from plib http://plib.sourceforge.net/index.html
// Which is covered by LGPL2

#ifndef MATH_QUAT_H
#define MATH_QUAT_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector.h"
#include "math/angle.h"
#include "math/vector4d.h"
#include "matrix4.h"

namespace Math {

class Quaternion : public Vector4d {
public:
	Quaternion() : Vector4d(0, 0, 0, 0) {}
	Quaternion(float lx, float ly, float lz, float lw) : Vector4d(lx, ly, lz, lw) {}
	Quaternion(const Quaternion &q) : Vector4d(q.x(), q.y(), q.z(), q.w()) {} 

	Matrix4 toMatrix();
	void slerpQuat(Quaternion dst, const Quaternion from, const Quaternion to, const float t);
	
	inline static Quaternion get_quaternion(const char *data) {
		return Quaternion(get_float(data), get_float(data + 4), get_float(data + 8), get_float(data + 12));
	}
};

} // end of namespace Math

#endif
