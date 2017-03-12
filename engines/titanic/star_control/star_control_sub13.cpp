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

#include "titanic/star_control/star_control_sub13.h"
#include "titanic/titanic.h"

namespace Titanic {

CStarControlSub13::CStarControlSub13() {
	_fieldC = 0;
	_field10 = 800.0;
	_field14 = 10000.0;
	_field18 = 20.0;
	_field1C = 20.0;
	_width = 600;
	_height = 340;
	_field24 = 0;
	_fieldC0 = _fieldC4 = _fieldC8 = 0.0;
	_fieldCC = _fieldD0 = _fieldD4 = 0.0;
	Common::fill(&_valArray[0], &_valArray[5], 0.0);
}

CStarControlSub13::CStarControlSub13(CStarControlSub13 *src) :
		_matrix(src->_matrix), _sub1(&src->_sub1), _sub2(&src->_sub2) {
	_position = src->_position;
	_fieldC = src->_fieldC;
	_field10 = src->_field10;
	_field14 = src->_field14;
	_field18 = src->_field18;
	_field1C = src->_field1C;
	_width = src->_width;
	_height = src->_height;

	_fieldCC = src->_fieldCC;
	_fieldD0 = src->_fieldD0;
	_fieldC0 = src->_fieldC0;
	_fieldC4 = src->_fieldC4;
	_fieldC8 = src->_fieldC8;
	_field24 = src->_field24;

	Common::copy(&src->_valArray[0], &src->_valArray[4], &_valArray[0]);
	_fieldD4 = 0;
}

void CStarControlSub13::copyFrom(const CStarControlSub13 *src) {
	if (src) {
		// TODO: Not really certain src is a CStarControlSub13
		_position = src->_position;
		_fieldC = src->_field18;
		_field10 = src->_field1C;
	}
}

void CStarControlSub13::load(SimpleFile *file, int param) {
	_position._x = file->readFloat();
	_position._y = file->readFloat();
	_position._z = file->readFloat();
	_fieldC = file->readFloat();
	_field10 = file->readFloat();
	_field14 = file->readFloat();
	_field18 = file->readFloat();
	_field1C = file->readFloat();

	int widthHeight = file->readNumber();
	_width = widthHeight & 0xff;
	_height = _width >> 16;
	_field24 = file->readNumber();

	for (int idx = 0; idx < 5; ++idx)
		_valArray[idx] = file->readFloat();

	_matrix.load(file, param);
	_fieldD4 = 0;
}

void CStarControlSub13::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_position._x, indent);
	file->writeFloatLine(_position._y, indent);
	file->writeFloatLine(_position._z, indent);
	file->writeFloatLine(_fieldC, indent);
	file->writeFloatLine(_field10, indent);
	file->writeFloatLine(_field14, indent);
	file->writeFloatLine(_field18, indent);
	file->writeFloatLine(_field1C, indent);
	file->writeFloatLine(_width | (_height << 16), indent);

	for (int idx = 0; idx < 5; ++idx)
		file->writeFloatLine(_valArray[idx], indent);

	_matrix.save(file, indent);
}

void CStarControlSub13::setPosition(const FVector &v) {
	_position = v;
	_fieldD4 = 0;
}

void CStarControlSub13::setPosition(const CStarControlSub6 *sub6) {
	FVector vector;
	_position.fn5(&vector, sub6);
	_position = sub6->_row1;
	_fieldD4 = 0;
}

void CStarControlSub13::setMatrix(const FMatrix &m) {
	_matrix = m;
	_fieldD4 = 0;
}

void CStarControlSub13::fn11(const FVector &v) {
	_matrix.fn1(v);
	_fieldD4 = 0;
}

void CStarControlSub13::setC(double v) {
	_fieldC = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set10(double v) {
	_field10 = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set14(double v) {
	_field10 = v;
}

void CStarControlSub13::set18(double v) {
	_field18 = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set1C(double v) {
	_field1C = v;
	_fieldD4 = 0;
}

void CStarControlSub13::fn12() {
	_matrix.clear();

	CStarControlSub6 m1(0, g_vm->getRandomNumber(359));
	CStarControlSub6 m2(1, g_vm->getRandomNumber(359));
	CStarControlSub6 m3(2, g_vm->getRandomNumber(359));
	
	CStarControlSub6 s1, s2;
	CStarControlSub6 *s;
	s = CStarControlSub6::setup(&s1, &m1, &m2);
	s = CStarControlSub6::setup(&s2, s, &m3);

	m1.copyFrom(*s);
	_matrix.fn2(m1);
	_fieldD4 = 0;
}

void CStarControlSub13::fn13(StarMode mode, double v2) {
	if (mode == MODE_STARFIELD) {
		_valArray[0] = v2;
		_valArray[1] = -v2;
	} else {
		_valArray[3] = v2;
		_valArray[4] = -v2;
	}

	_valArray[2] = 0.0;
	_field24 = v2 ? 2 : 0;
}

void CStarControlSub13::reposition(double factor) {
	_position._x = _matrix._row3._x * factor + _position._x;
	_position._y = _matrix._row3._y * factor + _position._y;
	_position._z = _matrix._row3._z * factor + _position._z;
	_fieldD4 = 0;
}

void CStarControlSub13::fn15(const FMatrix &matrix) {
	_matrix.fn3(matrix);
	_fieldD4 = 0;
}

CStarControlSub6 CStarControlSub13::getSub1() {
	if (!_fieldD4)
		reset();

	return _sub1;
}

CStarControlSub6 CStarControlSub13::getSub2() {
	if (!_fieldD4)
		reset();

	return _sub2;
}

void CStarControlSub13::fn16(int index, const FVector &src, FVector &dest) {
	CStarControlSub6 temp = getSub1();

	dest._x = temp._row3._x * src._z + temp._row2._x * src._y
		+ src._x * temp._row1._x + temp._vector._x;
	dest._y = temp._row3._y * src._z + temp._row2._y * src._y
		+ src._x * temp._row1._y + temp._vector._y;
	dest._z = temp._row3._z * src._z + temp._row2._z * src._y
		+ src._x * temp._row1._z + temp._vector._z;
}

FVector CStarControlSub13::fn17(int index, const FVector &src) {
	FVector dest;
	FVector tv;
	CStarControlSub6 sub6 = getSub1();
	src.fn5(&tv, &sub6);

	dest._x = (_valArray[index] + tv._x)
		* _fieldC8 / (_fieldCC * tv._z);
	dest._y = (tv._y * _fieldC8) / (_fieldD0 * tv._z);
	dest._z = tv._z;
	return dest;
}

FVector CStarControlSub13::fn18(int index, const FVector &src) {
	FVector dest;
	FVector tv;
	CStarControlSub6 sub6 = getSub2();
	src.fn5(&tv, &sub6);

	dest._x = (_valArray[index] + tv._x)
		* _fieldC8 / (_fieldCC * tv._z);
	dest._y = (tv._y * _fieldC8) / (_fieldD0 * tv._z);
	dest._z = tv._z;
	return dest;
}

void CStarControlSub13::fn19(double *v1, double *v2, double *v3, double *v4) {
	*v1 = _fieldC8 / _fieldCC;
	*v2 = _fieldC8 / _fieldD0;
	*v3 = _valArray[3];
	*v4 = _valArray[4];
}

void CStarControlSub13::reset() {
	const double FACTOR = 3.1415927 * 0.0055555557;

	_sub2.copyFrom(_matrix);
	_sub2._vector._x = _position._x;
	_sub2._vector._y = _position._y;
	_sub2._vector._z = _position._z;
	_sub2.fn3(_sub1);

	double widthV = (double)_width * 0.5;
	double heightV = (double)_height * 0.5;
	_fieldC0 = widthV;
	_fieldC4 = heightV;
	_fieldC8 = MIN(widthV, heightV);
	_fieldCC = tan(_field18 * FACTOR);
	_fieldD4 = 1;
	_fieldD0 = tan(_field1C * FACTOR);
}

const FMatrix &CStarControlSub13::getMatrix() const {
	return _matrix;
}

} // End of namespace Titanic
