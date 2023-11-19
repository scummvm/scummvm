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

#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_trs.h"

namespace Tetraedge {

TeMatrix4x4::TeMatrix4x4() {
	setToIdentity();
}

void TeMatrix4x4::setToIdentity() {
	_data[0] = _data[5] = _data[10] = _data[15] = 1.0f;
	_data[1] = _data[2] = _data[3] = _data[4] = 0.0f;
	_data[6] = _data[7] = _data[8] = _data[9] = 0.0f;
	_data[11] = _data[12] = _data[13] = _data[14] = 0.0f;
}

TeMatrix4x4::TeMatrix4x4(const Math::Matrix<4, 4> &matrix) {
	// Transpose  - row-major to column-major.
	float *d = getData();
	const float *s = matrix.getData();
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			d[c * 4 + r] = s[r * 4 + c];
		}
	}
}

TeMatrix4x4 operator*(const TeMatrix4x4 &left, const TeMatrix4x4 &right) {
	TeMatrix4x4 retval;
	const float *d1 = left.getData();
	const float *d2 = right.getData();
	float *res = retval.getData();

	res[0] = res[5] = res[10] = res[15] = 0.0f;

	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 16; c += 4) {
			res[c + r] = (d1[r + 0] * d2[c + 0]) +
						 (d1[r + 4] * d2[c + 1]) +
						 (d1[r + 8] * d2[c + 2]) +
						 (d1[r + 12] * d2[c + 3]);
		}
	}

	return retval;
}

/*
TeMatrix4x4 &TeMatrix4x4::operator*=(const TeMatrix4x4 &mul) {
	TeMatrix4x4 result = operator*(*this, mul);
	*this = result;
	return *this;
}*/

bool TeMatrix4x4::operator==(const TeMatrix4x4 &other) const {
	for (int i = 0; i < 16; i++) {
		if (_data[i] != other._data[i])
			return false;
	}
	return true;
}

void TeMatrix4x4::scale(const TeVector3f32 &vec) {
	TeMatrix4x4 scaleMatrix;
	scaleMatrix(0, 0) = vec.x();
	scaleMatrix(1, 1) = vec.y();
	scaleMatrix(2, 2) = vec.z();
	//scaleMatrix(3, 3) = 1.0; // default.
	*this = (*this * scaleMatrix);
}

void TeMatrix4x4::translate(const TeVector3f32 &vec) {
	TeMatrix4x4 translMatrix;
	translMatrix(0, 3) = vec.x();
	translMatrix(1, 3) = vec.y();
	translMatrix(2, 3) = vec.z();
	*this = (*this * translMatrix);
}

void TeMatrix4x4::rotate(const TeQuaternion &rot) {
	const TeMatrix4x4 rotMatrix = rot.toTeMatrix();
	*this = (*this * rotMatrix);
}

TeVector3f32 TeMatrix4x4::translation() const {
	return TeVector3f32(_data[12], _data[13], _data[14]);
}

TeVector3f32 TeMatrix4x4::mult4x3(const TeVector3f32 &vec) const {
	const float f1 = vec.x();
	const float f2 = vec.y();
	const float f3 = vec.z();
	const float *data = getData();

	return TeVector3f32(data[0] * f1 + data[4] * f2 + data[8] * f3 + data[12],
						data[1] * f1 + data[5] * f2 + data[9] * f3 + data[13],
						data[2] * f1 + data[6] * f2 + data[10] * f3 + data[14]);

}

TeVector3f32 TeMatrix4x4::mult3x3(const TeVector3f32 &vec) const {
	const float f1 = vec.x();
	const float f2 = vec.y();
	const float f3 = vec.z();
	const float *data = getData();

	return TeVector3f32(data[0] * f1 + data[4] * f2 + data[8] * f3,
						data[1] * f1 + data[5] * f2 + data[9] * f3,
						data[2] * f1 + data[6] * f2 + data[10] * f3);
}

TeMatrix4x4 TeMatrix4x4::meshScale(float factor) const {
	TeMatrix4x4 result;
	for (int i = 0; i < 16; i++) {
		result._data[i] = _data[i] * factor;
	}
	return result;
}

void TeMatrix4x4::meshAdd(const TeMatrix4x4 &other) {
	for (int i = 0; i < 16; i++) {
		_data[i] += other._data[i];
	}
}

TeVector3f32 TeMatrix4x4::operator*(const TeVector3f32 &mul) const {
	float x = mul.x();
	float y = mul.y();
	float z = mul.z();
	const float *d = getData();
	float w = d[3] * x + d[7] * y + d[11] * z + d[15];
	if (w == 0.0)
		w = 1e-09f;

	return TeVector3f32
			((d[0] * x + d[4] * y + d[8] *  z + d[12]) / w,
			 (d[1] * x + d[5] * y + d[9] *  z + d[13]) / w,
			 (d[2] * x + d[6] * y + d[10] * z + d[14]) / w);
}

Common::String TeMatrix4x4::toString() const {
	const float *data = getData();
	return Common::String::format("[[%.03f %.03f %.03f %.03f]  [%.03f %.03f %.03f %.03f]  [%.03f %.03f %.03f %.03f]  [%.03f %.03f %.03f %.03f]]",
								  data[0], data[4], data[8], data[12],
								  data[1], data[5], data[9], data[13],
								  data[2], data[6], data[10], data[14],
								  data[3], data[7], data[11], data[15]);
}

Math::Matrix<4, 4> TeMatrix4x4::toScummVMMatrix() const {
	const TeMatrix4x4 trans = transpose();
	Math::Matrix<4, 4> retval;
	retval.setData(trans.getData());
	return retval;
}

TeMatrix4x4 TeMatrix4x4::transpose() const {
	TeMatrix4x4 ret;
	const float *s = getData();
	float *d = ret.getData();
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			d[c * 4 + r] = s[r * 4 + c];
		}
	}
	return ret;
}

bool TeMatrix4x4::inverse() {
	TeMatrix4x4 invMatrix;
	float *inv = invMatrix.getData();
	TeMatrix4x4 temp = transpose();
	float *m = temp.getData();

	inv[0] = m[5]  * m[10] * m[15] -
			 m[5]  * m[11] * m[14] -
			 m[9]  * m[6]  * m[15] +
			 m[9]  * m[7]  * m[14] +
			 m[13] * m[6]  * m[11] -
			 m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
			  m[4]  * m[11] * m[14] +
			  m[8]  * m[6]  * m[15] -
			  m[8]  * m[7]  * m[14] -
			  m[12] * m[6]  * m[11] +
			  m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9]  * m[15] -
			 m[4]  * m[11] * m[13] -
			 m[8]  * m[5]  * m[15] +
			 m[8]  * m[7]  * m[13] +
			 m[12] * m[5]  * m[11] -
			 m[12] * m[7]  * m[9];

	inv[12] = -m[4]  * m[9]  * m[14] +
			   m[4]  * m[10] * m[13] +
			   m[8]  * m[5]  * m[14] -
			   m[8]  * m[6]  * m[13] -
			   m[12] * m[5]  * m[10] +
			   m[12] * m[6]  * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
			  m[1]  * m[11] * m[14] +
			  m[9]  * m[2]  * m[15] -
			  m[9]  * m[3]  * m[14] -
			  m[13] * m[2]  * m[11] +
			  m[13] * m[3]  * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
			 m[0]  * m[11] * m[14] -
			 m[8]  * m[2]  * m[15] +
			 m[8]  * m[3]  * m[14] +
			 m[12] * m[2]  * m[11] -
			 m[12] * m[3]  * m[10];

	inv[9] = -m[0]  * m[9]  * m[15] +
			  m[0]  * m[11] * m[13] +
			  m[8]  * m[1]  * m[15] -
			  m[8]  * m[3]  * m[13] -
			  m[12] * m[1]  * m[11] +
			  m[12] * m[3]  * m[9];

	inv[13] = m[0]  * m[9]  * m[14] -
			  m[0]  * m[10] * m[13] -
			  m[8]  * m[1]  * m[14] +
			  m[8]  * m[2]  * m[13] +
			  m[12] * m[1]  * m[10] -
			  m[12] * m[2]  * m[9];

	inv[2] = m[1]  * m[6] * m[15] -
			 m[1]  * m[7] * m[14] -
			 m[5]  * m[2] * m[15] +
			 m[5]  * m[3] * m[14] +
			 m[13] * m[2] * m[7] -
			 m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] +
			  m[0]  * m[7] * m[14] +
			  m[4]  * m[2] * m[15] -
			  m[4]  * m[3] * m[14] -
			  m[12] * m[2] * m[7] +
			  m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] -
			  m[0]  * m[7] * m[13] -
			  m[4]  * m[1] * m[15] +
			  m[4]  * m[3] * m[13] +
			  m[12] * m[1] * m[7] -
			  m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] +
			   m[0]  * m[6] * m[13] +
			   m[4]  * m[1] * m[14] -
			   m[4]  * m[2] * m[13] -
			   m[12] * m[1] * m[6] +
			   m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
			  m[1] * m[7] * m[10] +
			  m[5] * m[2] * m[11] -
			  m[5] * m[3] * m[10] -
			  m[9] * m[2] * m[7] +
			  m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
			 m[0] * m[7] * m[10] -
			 m[4] * m[2] * m[11] +
			 m[4] * m[3] * m[10] +
			 m[8] * m[2] * m[7] -
			 m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
			   m[0] * m[7] * m[9] +
			   m[4] * m[1] * m[11] -
			   m[4] * m[3] * m[9] -
			   m[8] * m[1] * m[7] +
			   m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
			  m[0] * m[6] * m[9] -
			  m[4] * m[1] * m[10] +
			  m[4] * m[2] * m[9] +
			  m[8] * m[1] * m[6] -
			  m[8] * m[2] * m[5];

	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (int i = 0; i < 16; i++) {
		m[i] = inv[i] * det;
	}

	*this = temp.transpose();

	return true;
}

void TeMatrix4x4::deserialize(Common::ReadStream &stream) {
	for (int i = 0; i < 16; i++) {
		_data[i] = stream.readFloatLE();
	}
}

void TeMatrix4x4::serialize(Common::WriteStream &stream) const {
	for (int i = 0; i < 16; i++) {
		stream.writeFloatLE(_data[i]);
	}
}

/*static*/
TeMatrix4x4 TeMatrix4x4::fromTRS(const TeTRS &trs) {
	TeMatrix4x4 result;
	const TeVector3f32 trans = trs.getTranslation();
	TeMatrix4x4 transm;
	transm(0, 3) = trans.x();
	transm(1, 3) = trans.y();
	transm(2, 3) = trans.z();
	result = result * transm;

	const TeMatrix4x4 rotm = trs.getRotation().toTeMatrix();
	result = result * rotm;

	const TeVector3f32 scle = trs.getScale();
	TeMatrix4x4 scalem;
	scalem(0, 0) = scle.x();
	scalem(1, 1) = scle.y();
	scalem(2, 2) = scle.z();
	result = result * scalem;

	return result;
}


} // end namespace Tetraedge
