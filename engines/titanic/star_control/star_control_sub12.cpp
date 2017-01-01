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

CStarControlSub12::CStarControlSub12(void *val1, const CStar20Data *data) :
		_currentIndex(-1), _handlerP(nullptr), _field108(0),
		_sub13(val1) {
	setupHandler(data);
}

CStarControlSub12::CStarControlSub12(CStarControlSub13 *src) :
		_currentIndex(-1), _handlerP(nullptr), _field108(0), _sub13(src) {
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

void CStarControlSub12::proc3(const CStar20Data *src) {
	_handlerP->copyFrom(src);
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

void CStarControlSub12::proc16() {
	_handlerP->proc4();
}

void CStarControlSub12::proc17() {
	_handlerP->proc5();
}

void CStarControlSub12::proc18() {
	_handlerP->proc6();
}

void CStarControlSub12::proc19() {
	_handlerP->proc7();
}

void CStarControlSub12::proc20(double v) {
	if (!isLocked())
		_sub13.fn14(v);
}

void CStarControlSub12::proc21(CStarControlSub6 &sub6) {
	if (!isLocked()) {
		_sub13.setPosition(sub6);
		set108();
	}
}

void CStarControlSub12::proc22(FMatrix &m) {
	if (!isLocked())
		_sub13.fn15(m);
}

CStarControlSub6 CStarControlSub12::proc23() {
	return _sub13.getSub1();
}

CStarControlSub6 CStarControlSub12::proc24() {
	return _sub13.getSub2();
}

double CStarControlSub12::proc25() const {
	return _sub13._field10;
}

double CStarControlSub12::proc26() const {
	return _sub13._field14;
}

int CStarControlSub12::proc27() const {
	return _sub13._field24;
}

FVector CStarControlSub12::proc28(int index, const void *v2) {
	error("TODO: CStarControlSub12::proc28");
	return FVector();
}

FVector CStarControlSub12::proc29(const FVector &v) {
	return _sub13.fn16(v);
}

FVector CStarControlSub12::proc30(int index, const FVector &v) {
	return _sub13.fn17(index, v);
}

FVector CStarControlSub12::proc31(int index, const FVector &v) {
	return _sub13.fn18(index, v);
}

void CStarControlSub12::proc32(double v1, double v2) {
	error("TODO: CStarControlSub12::proc32");
}

bool CStarControlSub12::setArrayVector(const FVector &v) {
	if (_currentIndex >= 2)
		return false;

	error("TODO: CStarControlSub12::setArrayVector");
}

bool CStarControlSub12::proc35() {
	if (_currentIndex == -1)
		return false;

	error("TODO: CStarControlSub12::proc35");
}

void CStarControlSub12::proc36(double *v1, double *v2, double *v3, double *v4) {
	_sub13.fn19(v1, v2, v3, v4);
}

void CStarControlSub12::load(SimpleFile *file, int param) {
	_sub13.load(file, param);
}

void CStarControlSub12::save(SimpleFile *file, int indent) {
	_sub13.save(file, indent);
}

bool CStarControlSub12::setupHandler(const CStar20Data *src) {
	CStarControlSub20 *handler = nullptr;

	switch (_currentIndex) {
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
