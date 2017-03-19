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

CStarControlSub12::CStarControlSub12(const CStar20Data *data) :
		_currentIndex(-1), _handlerP(nullptr), _field108(0) {
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

void CStarControlSub12::proc2(const CStarControlSub13 *src) {
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

void CStarControlSub12::proc12(StarMode mode, double v2) {
	if (!isLocked())
		_sub13.fn13(mode, v2);
}

void CStarControlSub12::proc13(CStarControlSub13 *dest) {
	*dest = _sub13;
}

void CStarControlSub12::proc14(FVector &v) {
	FMatrix matrix = _sub13.getMatrix();
	FVector vector = _sub13._position;

	_handlerP->proc9(vector, v, matrix);
}

void CStarControlSub12::proc15(CErrorCode *errorCode) {
	if (!_matrix1)
		_matrix1 = new FMatrix();
	if (!_matrix2)
		_matrix2 = new FMatrix();

	*_matrix1 = _sub13.getMatrix();
	*_matrix2 = *_matrix1;

	FVector v1 = _sub13._position;
	FVector v2 = _sub13._position;
	_handlerP->proc11(*errorCode, v2, *_matrix2);

	if (v1 != v2) {
		_sub13.setPosition(v2);
		set108();
	}

	if (_matrix1 != _matrix2) {
		_sub13.setMatrix(*_matrix2);
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

void CStarControlSub12::proc20(double factor) {
	if (!isLocked())
		_sub13.reposition(factor);
}

void CStarControlSub12::proc21(const CStarControlSub6 *sub6) {
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

void CStarControlSub12::proc28(int index, const FVector &src, FVector &dest) {
	dest._x = ((_sub13._valArray[index] + src._x) * _sub13._fieldC8)
		/ (_sub13._fieldCC * src._z);
	dest._y = src._y * _sub13._fieldC8 / (_sub13._fieldD0 * src._z);
	dest._z = src._z;
}

void CStarControlSub12::proc29(int index, const FVector &src, FVector &dest) {
	_sub13.fn16(index, src, dest);
}

FVector CStarControlSub12::proc30(int index, const FVector &v) {
	return _sub13.fn17(index, v);
}

FVector CStarControlSub12::proc31(int index, const FVector &v) {
	return _sub13.fn18(index, v);
}

void CStarControlSub12::setViewportPosition(const FPoint &angles) {
	if (isLocked())
		return;

	if (_currentIndex == -1) {
		CStarControlSub6 subX(X_AXIS, angles._x);
		CStarControlSub6 subY(Y_AXIS, angles._y);
		CStarControlSub6 sub(&subX, &subY);
		subY.copyFrom(&sub);
		proc22(subY);
	} else if (_currentIndex == 0) {
		FVector row1 = _matrix._row1;
		CStarControlSub6 subX(X_AXIS, angles._x);
		CStarControlSub6 subY(Y_AXIS, angles._y);
		CStarControlSub6 sub(&subX, &subY);
		subX.copyFrom(&sub);

		FMatrix m1 = _sub13.getMatrix();
		FVector tempV1 = _sub13._position;
		FVector tempV2, tempV3, tempV4, tempV5, tempV6;
		tempV2._y = m1._row1._y * 100000.0;
		tempV2._z = m1._row1._z * 100000.0;
		tempV3._x = m1._row1._x * 100000.0 + tempV1._x;
		tempV4._x = tempV3._x;
		tempV3._y = tempV2._y + tempV1._y;
		tempV4._y = tempV3._y;
		tempV3._z = tempV2._z + tempV1._z;
		tempV4._z = tempV3._z;
		tempV2._x = m1._row2._x * 100000.0;
		tempV2._y = m1._row2._y * 100000.0;
		tempV2._z = m1._row2._z * 100000.0;
		tempV2._x = m1._row3._x * 100000.0;
		tempV2._y = m1._row3._y * 100000.0;
		tempV2._z = m1._row3._z * 100000.0;
		tempV2._x = tempV2._x + tempV1._x;
		tempV2._y = tempV2._y + tempV1._y;
		tempV2._z = tempV2._z + tempV1._z;
		tempV3._x = tempV2._x + tempV1._x;
		tempV3._y = tempV2._y + tempV1._y;
		tempV5._x = tempV2._x;
		tempV5._y = tempV2._y;
		tempV3._z = tempV2._z + tempV1._z;
		tempV5._z = tempV2._z;
		tempV6._x = tempV3._x;
		tempV6._y = tempV3._y;
		tempV6._z = tempV3._z;
		tempV1._x = tempV1._x - row1._x;
		tempV1._y = tempV1._y - row1._y;
		tempV1._z = tempV1._z - row1._z;
		tempV4._x = tempV3._x - row1._x;
		tempV4._y = tempV4._y - row1._y;
		tempV4._z = tempV4._z - row1._z;
		tempV5._x = tempV2._x - row1._x;

		tempV5._y = tempV5._y - row1._y;
		tempV5._z = tempV5._z - row1._z;
		tempV6._x = tempV3._x - row1._x;
		tempV6._y = tempV6._y - row1._y;
		tempV6._z = tempV6._z - row1._z;

		FVector modV1 = tempV1.fn5(&subX);
		FVector modV2 = tempV4.fn5(&subX);
		FVector modV3 = tempV5.fn5(&subX);
		FVector modV4 = tempV6.fn5(&subX);
		tempV1 = modV1;
		tempV4 = modV2;
		tempV5 = modV3;
		tempV4 = modV4;

		tempV2._x = tempV4._x - tempV1._x;
		tempV2._y = tempV4._y - tempV1._y;
		tempV2._z = tempV4._z - tempV1._z;
		tempV4._x = tempV2._x;
		tempV4._y = tempV2._y;
		tempV2._x = tempV5._x - tempV1._x;
		tempV4._z = tempV2._z;
		tempV5._x = tempV2._x;
		tempV2._y = tempV5._y - tempV1._y;
		tempV5._y = tempV2._y;
		tempV2._z = tempV5._z - tempV1._z;
		tempV5._z = tempV2._z;
		tempV2._x = tempV6._x - tempV1._x;
		tempV2._y = tempV6._y - tempV1._y;
		tempV2._z = tempV6._z - tempV1._z;
		tempV6 = tempV2;

		tempV4.normalize();
		tempV5.normalize();
		tempV6.normalize();
		tempV1 += row1;

		m1.set(tempV4, tempV5, tempV6);
		_sub13.setMatrix(m1);
		_sub13.setPosition(tempV1);
	} else if (_currentIndex == 1) {
		// TODO
	}
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
	}
	else {
		return false;
	}
}

void CStarControlSub12::deleteHandler() {
	if (_handlerP) {
		delete _handlerP;
		_handlerP = nullptr;
	}
}

void CStarControlSub12::fn1(CStarControlSub13 *sub13, const FVector &v) {
	// TODO
}

void CStarControlSub12::fn2(FVector v1, FVector v2, FVector v3) {
	// TODO
}

void CStarControlSub12::fn3(CStarControlSub13 *sub13, const FVector &v) {
	// TODO
}

} // End of namespace Titanic
