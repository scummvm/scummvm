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

#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_control_sub21.h"
#include "titanic/star_control/star_control_sub22.h"

namespace Titanic {

FMatrix *CStarControlSub12::_matrix1;
FMatrix *CStarControlSub12::_matrix2;

CStarControlSub12::CStarControlSub12(void *val1, void *val2) : 
		_field4(-1), _handlerP(nullptr), _field108(0),
		_sub13(val1) {
	setupHandler(val2);
}

CStarControlSub12::CStarControlSub12(CStarControlSub13 *src) :
		_field4(-1), _handlerP(nullptr), _field108(0), _sub13(src) {
}

void CStarControlSub12::init() {
	_matrix1 = nullptr;
	_matrix2 = nullptr;
}

void CStarControlSub12::deinit() {
	delete _matrix1;
	delete _matrix2;
	_matrix1 = nullptr;
	_matrix2 = nullptr;
}

CStarControlSub12::~CStarControlSub12() {
	deleteHandler();
}

void CStarControlSub12::proc2(const void *src) {
	_sub13.copyFrom(src); 
}

void CStarControlSub12::proc3(const void *src) {
	_handlerP->copyFrom1(src);
}

void CStarControlSub12::setPosition(const FVector &v) {
	if (!isLocked()) {
		_sub13.setPosition(v);
		set108();
	}
}

void CStarControlSub12::proc5(const FVector &v) {
	if (!isLocked())
		_sub13.fn11(v);
}

void CStarControlSub12::proc6(int v) {
	if (!isLocked())
		_sub13.setC(v);
}

void CStarControlSub12::proc7(int v) {
	if (!isLocked())
		_sub13.set10(v);
}

void CStarControlSub12::proc8(int v) {
	if (!isLocked())
		_sub13.set14(v);
}

void CStarControlSub12::proc9(int v) {
	if (!isLocked())
		_sub13.set18(v);
}

void CStarControlSub12::proc10(int v) {
	if (!isLocked())
		_sub13.set1C(v);
}

void CStarControlSub12::proc11() {
	if (!isLocked())
		_sub13.fn12();
}

void CStarControlSub12::proc12(double v1, double v2) {
	if (!isLocked())
		_sub13.fn13(v1, v2);
}

void CStarControlSub12::proc13(CStarControlSub13 *dest) {
	*dest = _sub13;
}

void CStarControlSub12::proc14(int v) {
	FMatrix matrix;
	_sub13.getMatrix(&matrix);
	FVector vector = _sub13._position;

	_handlerP->proc9(&vector, v, &matrix);
}

void CStarControlSub12::proc15(int v) {
	if (!_matrix1)
		_matrix1 = new FMatrix();
	if (!_matrix2)
		_matrix2 = new FMatrix();

	_sub13.getMatrix(_matrix1);
	*_matrix2 = *_matrix1;

	FVector v1 = _sub13._position;
	FVector v2 = _sub13._position;
	CErrorCode errorCode;
	_handlerP->proc11(errorCode, v2, _matrix2);

	if (v1 != v2) {
		_sub13.setPosition(v2);
		set108();
	}

	if (_matrix1 != _matrix2) {
		_sub13.setMatrix(_matrix2);
	}
}

void CStarControlSub12::load(SimpleFile *file, int param) {
	_sub13.load(file, param);
}

void CStarControlSub12::save(SimpleFile *file, int indent) {
	_sub13.save(file, indent);
}

bool CStarControlSub12::setupHandler(void *src) {
	CStarControlSub20 *handler = nullptr;

	switch (_field4) {
	case -1:
		handler = new CStarControlSub21(src);
		break;

	case 0:
	case 1:
	case 2:
		handler = new CStarControlSub22(src);
		break;

	default:
		break;
	}

	if (handler) {
		assert(!_handlerP);
		_handlerP = handler;
		return true;
	} else {
		return false;
	}
}

void CStarControlSub12::deleteHandler() {
	if (_handlerP) {
		delete _handlerP;
		_handlerP = nullptr;
	}
}

} // End of namespace Titanic
