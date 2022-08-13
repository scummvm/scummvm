/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TE_TE_QUATERNION_H
#define TETRAEDGE_TE_TE_QUATERNION_H

#include "math/quat.h"

#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeQuaternion: public Math::Quaternion {
public:
	TeQuaternion();
	TeQuaternion(const Math::Quaternion &q) : Math::Quaternion(q) {};
	TeQuaternion(float a, float b, float c, float d) : Math::Quaternion(a, b, c, d) {};

	static TeQuaternion fromAxisAndAngle(const TeVector3f32 &axis, float angle) {
		TeQuaternion ret;
		float f = sinf(angle * 0.5);
		ret.w() = axis.x() * f;
		ret.x() = axis.y() * f;
		ret.y() = axis.z() * f;
		ret.z() = cosf(angle * 0.5);
		return ret;
	}

	static void deserialize(Common::ReadStream &stream, TeQuaternion &dest) {
		dest.value(0) = stream.readFloatLE();
		dest.value(1) = stream.readFloatLE();
		dest.value(2) = stream.readFloatLE();
		dest.value(3) = stream.readFloatLE();
	}

	static void serialize(Common::WriteStream &stream, const TeQuaternion &src) {
		stream.writeFloatLE(src.value(0));
		stream.writeFloatLE(src.value(1));
		stream.writeFloatLE(src.value(2));
		stream.writeFloatLE(src.value(3));
	}

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_QUATERNION_H
