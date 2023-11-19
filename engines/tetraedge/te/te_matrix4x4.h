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

#ifndef TETRAEDGE_TE_TE_MATRIX4X4_H
#define TETRAEDGE_TE_TE_MATRIX4X4_H

#include "math/matrix4.h"

#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeTRS;

/**
 * A 4x4 matrix, but stored in *column-major* order to match
 * OpenGL (and the original engine)
 */
class TeMatrix4x4 {
public:
	TeMatrix4x4();
	TeMatrix4x4(const Math::Matrix<4, 4> &matrix);

	void setToIdentity();

	float &operator()(int row, int col) {
		return *(_data + col * 4 + row);
	}

	const float &operator()(int row, int col) const {
		return *(_data + col * 4 + row);
	}

	bool operator==(const TeMatrix4x4 &other) const;
	bool operator!=(const TeMatrix4x4 &other) const {
		return !operator==(other);
	}

	//TeMatrix4x4 &operator*=(const TeMatrix4x4 &mul);

	TeVector3f32 operator*(const TeVector3f32 &mul) const;

	void scale(const TeVector3f32 &vec);
	void translate(const TeVector3f32 &vec);
	void rotate(const TeQuaternion &rot);
	TeVector3f32 translation() const;
	TeVector3f32 mult3x3(const TeVector3f32 &vec) const;
	TeVector3f32 mult4x3(const TeVector3f32 &vec) const;

	Common::String toString() const;
	Math::Matrix<4, 4> toScummVMMatrix() const;

	void setValue(int row, int col, float val) {
		operator()(row, col) = val;
	}

	TeMatrix4x4 transpose() const;
	TeMatrix4x4 meshScale(float factor) const;
	void meshAdd(const TeMatrix4x4 &other);

	bool inverse();

	static TeMatrix4x4 fromTRS(const TeTRS &trs);

	const float *getData() const { return _data; }
	float *getData() { return _data; }

	void deserialize(Common::ReadStream &stream);
	void serialize(Common::WriteStream &stream) const;

private:
	float _data[16];

};

TeMatrix4x4 operator*(const TeMatrix4x4 &left, const TeMatrix4x4 &right);

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MATRIX4X4_H
