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

#ifndef TETRAEDGE_TE_TE_VECTOR3F32_H
#define TETRAEDGE_TE_TE_VECTOR3F32_H

#include "common/str.h"
#include "math/vector2d.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeQuaternion;

class TeVector3f32 : public Math::Vector3d {

public:
	TeVector3f32() { }; // Note: vector3d constructor sets 0, 0, 0
	TeVector3f32(float x_, float y_, float z_) {
		set(x_, y_, z_);
	}
	TeVector3f32(const TeVector3f32 &other) : Math::Vector3d(other) {}
	TeVector3f32(const Math::Vector3d &v) : Math::Vector3d(v) {}
	TeVector3f32 &operator=(const TeVector3f32 &other) {
		Math::Vector3d::operator=(other);
		return *this;
	}

	//TeVector3f32 operator*(const TeVector3f32 &other) const {
	//	return TeVector3f32(x() * other.x(), y() * other.y(), z() * other.z());
	//}

	explicit TeVector3f32(const TeVector2s32 &vec2d) {
		set(vec2d._x, vec2d._y, 0.0);
	}

	static void deserialize(Common::ReadStream &stream, TeVector3f32 &dest) {
		dest.x() = stream.readFloatLE();
		dest.y() = stream.readFloatLE();
		dest.z() = stream.readFloatLE();
	}

	static void serialize(Common::WriteStream &stream, const TeVector3f32 &src) {
		stream.writeFloatLE(src.x());
		stream.writeFloatLE(src.y());
		stream.writeFloatLE(src.z());
	}

	float squaredLength() const {
		return (x() * x() + y() * y() + z() * z());
	}

	/** Parse comma-separated values */
	bool parse(const Common::String &val);

	Common::String dump() const {
		return Common::String::format("Vec3f(%.02f %.02f %.02f)", x(), y(), z());
	}

	void rotate(const TeQuaternion &rot);

	TeVector3f32 operator-() { return TeVector3f32(-x(), -y(), -z()); }
};

TeVector3f32 operator^(const TeVector3f32 &left, const TeVector3f32 &right);

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_VECTOR3F32_H
