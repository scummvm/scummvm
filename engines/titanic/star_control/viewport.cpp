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

#include "titanic/star_control/viewport.h"
#include "titanic/titanic.h"

namespace Titanic {

CViewport::CViewport() {
	_fieldC = 0;
	_field10 = 800.0;
	_field14 = 10000.0;
	_field18 = 20.0;
	_field1C = 20.0;
	_width = 600;
	_height = 340;
	_field24 = 0;
	_flag = false;
	Common::fill(&_valArray[0], &_valArray[5], 0.0);
}

CViewport::CViewport(CViewport *src) :
		_orientation(src->_orientation), _currentPose(src->_currentPose), _rawPose(src->_rawPose) {
	_position = src->_position;
	_fieldC = src->_fieldC;
	_field10 = src->_field10;
	_field14 = src->_field14;
	_field18 = src->_field18;
	_field1C = src->_field1C;
	_width = src->_width;
	_height = src->_height;

	_center = src->_center;
	_centerVector = src->_centerVector;
	_field24 = src->_field24;

	Common::copy(&src->_valArray[0], &src->_valArray[5], &_valArray[0]);
	_flag = false;
}

void CViewport::copyFrom(const CViewport *src) {
	error("Unused function");
}

void CViewport::load(SimpleFile *file, int param) {
	_position._x = file->readFloat();
	_position._y = file->readFloat();
	_position._z = file->readFloat();
	_fieldC = file->readFloat();
	_field10 = file->readFloat();
	_field14 = file->readFloat();
	_field18 = file->readFloat();
	_field1C = file->readFloat();

	int widthHeight = file->readNumber();
	_width = widthHeight & 0xffff;
	_height = widthHeight >> 16;
	_field24 = file->readNumber();

	for (int idx = 0; idx < 5; ++idx)
		_valArray[idx] = file->readFloat();

	_orientation.load(file, param);
	_flag = false;
}

void CViewport::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_position._x, indent);
	file->writeFloatLine(_position._y, indent);
	file->writeFloatLine(_position._z, indent);
	file->writeFloatLine(_fieldC, indent);
	file->writeFloatLine(_field10, indent);
	file->writeFloatLine(_field14, indent);
	file->writeFloatLine(_field18, indent);
	file->writeFloatLine(_field1C, indent);
	file->writeNumberLine(_width | (_height << 16), indent);

	for (int idx = 0; idx < 5; ++idx)
		file->writeFloatLine(_valArray[idx], indent);

	_orientation.save(file, indent);
}

void CViewport::setPosition(const FVector &v) {
	debugC(DEBUG_INTERMEDIATE, kDebugStarfield, "Setting starmap position to %s", v.toString().c_str());
	_position = v;
	_flag = false;
}

void CViewport::setPosition(const FPose &pose) {
	_position = _position.fn5(pose);
	_flag = false;
}

void CViewport::setOrientation(const FMatrix &m) {
	_orientation = m;
	_flag = false;
}

void CViewport::setOrientation(const FVector &v) {
	_orientation.set(v);
	_flag = false;
}

void CViewport::setC(double v) {
	_fieldC = v;
	_flag = false;
}

void CViewport::set10(double v) {
	_field10 = v;
	_flag = false;
}

void CViewport::set14(double v) {
	_field10 = v;
}

void CViewport::set18(double v) {
	_field18 = v;
	_flag = false;
}

void CViewport::set1C(double v) {
	_field1C = v;
	_flag = false;
}

void CViewport::fn12() {
	_orientation.identity();

	FPose m1(X_AXIS, g_vm->getRandomNumber(359));
	FPose m2(Y_AXIS, g_vm->getRandomNumber(359));
	FPose m3(Z_AXIS, g_vm->getRandomNumber(359));
	
	FPose s1(m1, m2);
	FPose s2(s1, m3);

	m1.copyFrom(s2);
	_orientation.fn2(m1);
	_flag = false;
}

void CViewport::fn13(StarMode mode, double val) {
	if (mode == MODE_PHOTO) {
		_valArray[0] = val;
		_valArray[1] = -val;
	} else {
		_valArray[3] = val;
		_valArray[4] = -val;
	}

	_valArray[2] = 0.0;
	_field24 = val ? 2 : 0;
}

void CViewport::reposition(double factor) {
	_position._x = _orientation._row3._x * factor + _position._x;
	_position._y = _orientation._row3._y * factor + _position._y;
	_position._z = _orientation._row3._z * factor + _position._z;
	_flag = false;
}

void CViewport::fn15(const FMatrix &matrix) {
	_orientation.fn3(matrix);
	_flag = false;
}

FPose CViewport::getPose() {
	if (!_flag)
		reset();

	return _currentPose;
}

FPose CViewport::getRawPose() {
	if (!_flag)
		reset();

	return _rawPose;
}

FVector CViewport::fn16(int index, const FVector &src) {
	FPose temp = getPose();

	FVector dest;
	dest._x = temp._row3._x * src._z + temp._row2._x * src._y
		+ src._x * temp._row1._x + temp._vector._x;
	dest._y = temp._row3._y * src._z + temp._row2._y * src._y
		+ src._x * temp._row1._y + temp._vector._y;
	dest._z = temp._row3._z * src._z + temp._row2._z * src._y
		+ src._x * temp._row1._z + temp._vector._z;
	return dest;
}

FVector CViewport::fn17(int index, const FVector &src) {
	FVector dest;
	FPose pose = getPose();
	FVector tv = src.fn5(pose);

	dest._x = (_valArray[index] + tv._x)
		* _centerVector._x / (_centerVector._y * tv._z);
	dest._y = (tv._y * _centerVector._x) / (_centerVector._z * tv._z);
	dest._z = tv._z;
	return dest;
}

FVector CViewport::fn18(int index, const FVector &src) {
	FVector dest;
	FPose pose = getRawPose();
	FVector tv = src.fn5(pose);

	dest._x = (_valArray[index] + tv._x)
		* _centerVector._x / (_centerVector._y * tv._z);
	dest._y = (tv._y * _centerVector._x) / (_centerVector._z * tv._z);
	dest._z = tv._z;
	return dest;
}

void CViewport::fn19(double *v1, double *v2, double *v3, double *v4) {
	*v1 = _centerVector._x / _centerVector._y;
	*v2 = _centerVector._x / _centerVector._z;
	*v3 = _valArray[3];
	*v4 = _valArray[4];
}

void CViewport::reset() {
	const double FACTOR = 2 * M_PI / 360.0;

	_rawPose.copyFrom(_orientation);
	_rawPose._vector = _position;
	_currentPose = _rawPose.fn4();

	_center = FPoint((double)_width * 0.5, (double)_height * 0.5);
	_centerVector._x = MIN(_center._x, _center._y);
	_centerVector._y = tan(_field18 * FACTOR);
	_centerVector._z = tan(_field1C * FACTOR);
	_flag = true;
}

const FMatrix &CViewport::getOrientation() const {
	return _orientation;
}

} // End of namespace Titanic
