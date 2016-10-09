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

namespace Titanic {

CStarControlSub13::CStarControlSub13(void *src) :
		_fieldC0(0), _fieldC4(0), _fieldC8(0), _fieldCC(0), _fieldD0(0) {
	if (src) {
		setup(src);
	} else {
		_fieldC = 0;
		_field10 = 0x44480000;
		_field14 = 0x461C4000;
		_field18 = 0x41A00000;
		_field1C = 0x41A00000;
		_width = 600;
		_height = 340;
		_field24 = 0;
	}

	_fieldD4 = 0;
}

CStarControlSub13::CStarControlSub13(CStarControlSub13 *src) :
		_matrix(&src->_matrix), _sub1(&src->_sub1), _sub2(&src->_sub2) {
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

	_valArray[0] = src->_valArray[0];
	_valArray[2] = src->_valArray[2];
	_valArray[3] = src->_valArray[3];
	_fieldD4 = 0;
}

void CStarControlSub13::setup(void *ptr) {
	// TODO
}

void CStarControlSub13::copyFrom(const void *src) {
	if (!src)
		return;
/*
	_field0 = src->_field0;
	_field4 = src->_field4;
	_field8 = src->_field8;
	_fieldC = src->_field18;
	_field10 = src->_field1C;
	*/
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

void CStarControlSub13::setPosition(const CStarControlSub6 &sub6) {
	FVector vector;
	_position.fn5(&vector, &sub6);
	_position = sub6._matrix._row1;
	_fieldD4 = 0;
}

void CStarControlSub13::setMatrix(const FMatrix &m) {
	_matrix = m;
	_fieldD4 = 0;
}

void CStarControlSub13::fn11(const FVector &v) {
	_matrix.fn1(&v);
	_fieldD4 = 0;
}

void CStarControlSub13::setC(int v) {
	_fieldC = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set10(int v) {
	_field10 = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set14(int v) {
	_field10 = v;
}

void CStarControlSub13::set18(int v) {
	_field18 = v;
	_fieldD4 = 0;
}

void CStarControlSub13::set1C(int v) {
	_field1C = v;
	_fieldD4 = 0;
}

void CStarControlSub13::fn12() {
	_matrix.clear();
	error("TODO: CStarControlSub13::fn12");
}

void CStarControlSub13::fn13(double v1, double v2) {
	if (v1 == 0.0) {
		_valArray[0] = v2;
		_valArray[1] = -v2;
	} else {
		_valArray[3] = v2;
		_valArray[4] = -v2;
	}

	_valArray[2] = 0.0;
	_field24 = v2 ? 2 : 0;
}

void CStarControlSub13::fn14(double v) {
	error("TODO: CStarControlSub13::fn14");
}

void CStarControlSub13::fn15(FMatrix &matrix) {
	_matrix.fn3(&matrix);
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

FVector CStarControlSub13::fn16(const FVector &v) {
	error("TODO: CStarControlSub13::fn16");
}

FVector CStarControlSub13::fn17(int index, const FVector &v) {
	error("TODO: CStarControlSub13::fn17");
}

FVector CStarControlSub13::fn18(int index, const FVector &v) {
	error("TODO: CStarControlSub13::fn17");
}

void CStarControlSub13::fn19(double *v1, double *v2, double *v3, double *v4) {
	error("TODO: CStarControlSub13::fn19");
}

void CStarControlSub13::reset() {
	const double FACTOR = 3.1415927 * 0.0055555557;
	error("TODO: CStarControlSub13::reset");
}

void CStarControlSub13::getMatrix(FMatrix *matrix) {
	*matrix = _matrix;
}

} // End of namespace Titanic
