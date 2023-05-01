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
#include "tetraedge/te/te_matrix4x4.h"

namespace Tetraedge {

class TeQuaternion: public Math::Quaternion {
public:
	TeQuaternion();
	TeQuaternion(const Math::Quaternion &q) : Math::Quaternion(q) {};
	TeQuaternion(float a, float b, float c, float d) : Math::Quaternion(a, b, c, d) {};

	static TeQuaternion fromAxisAndAngle(const TeVector3f32 &axis, float angle) {
		TeQuaternion ret;
		float f = sinf(angle * 0.5);
		ret.x() = axis.x() * f;
		ret.y() = axis.y() * f;
		ret.z() = axis.z() * f;
		ret.w() = cosf(angle * 0.5);
		return ret;
	}

	static TeQuaternion fromEuler(const TeVector3f32 &euler) {
		TeQuaternion rot;

		rot.x() = sinf(euler.x() / 2.0);
		rot.y() = 0.0;
		rot.z() = 0.0;
		rot.w() = cosf(euler.x() / 2.0);
		TeQuaternion retval = rot;

		rot.x() = 0.0;
		rot.y() = sinf(euler.y() / 2.0);
		rot.z() = 0.0;
		rot.w() = cosf(euler.y() / 2.0);
		retval *= rot;

		rot.x() = 0.0;
		rot.y() = 0.0;
		rot.z() = sinf(euler.z() / 2.0);
		rot.w() = cosf(euler.z() / 2.0);
		retval *= rot;

		return retval;
	}

	static TeQuaternion fromEulerDegrees(const TeVector3f32 &euler) {
		const float xdeg = (float)(euler.x() * M_PI / 180.0);
		const float ydeg = (float)(euler.y() * M_PI / 180.0);
		const float zdeg = (float)(euler.z() * M_PI / 180.0);
		return fromEuler(TeVector3f32(xdeg, ydeg, zdeg));
	}

	TeMatrix4x4 toTeMatrix() const {
		const TeMatrix4x4 retval = toMatrix();
		return retval.transpose();
	}

	static void deserialize(Common::ReadStream &stream, TeQuaternion &dest) {
		dest.x() = stream.readFloatLE();
		dest.y() = stream.readFloatLE();
		dest.z() = stream.readFloatLE();
		dest.w() = stream.readFloatLE();
	}

	static void serialize(Common::WriteStream &stream, const TeQuaternion &src) {
		stream.writeFloatLE(src.x());
		stream.writeFloatLE(src.y());
		stream.writeFloatLE(src.z());
		stream.writeFloatLE(src.w());
	}

	Common::String dump() const;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_QUATERNION_H
