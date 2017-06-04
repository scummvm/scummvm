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

#include "titanic/star_control/star_camera.h"
#include "titanic/star_control/star_control_sub21.h"
#include "titanic/star_control/star_control_sub22.h"
#include "titanic/star_control/dmatrix.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/titanic.h"

namespace Titanic {

FMatrix *CStarCamera::_priorOrientation;
FMatrix *CStarCamera::_newOrientation;

CStarCamera::CStarCamera(const CNavigationInfo *data) :
		_matrixRow(-1), _mover(nullptr), _isMoved(false) {
	setupHandler(data);
}

CStarCamera::CStarCamera(CViewport *src) :
		_matrixRow(-1), _mover(nullptr), _isMoved(false), _viewport(src) {
}

void CStarCamera::init() {
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

void CStarCamera::deinit() {
	delete _priorOrientation;
	delete _newOrientation;
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

CStarCamera::~CStarCamera() {
	deleteHandler();
}

void CStarCamera::proc2(const CViewport *src) {
	_viewport.copyFrom(src);
}

void CStarCamera::proc3(const CNavigationInfo *src) {
	_mover->copyFrom(src);
}

void CStarCamera::setPosition(const FVector &v) {
	if (!isLocked()) {
		_viewport.setPosition(v);
		setIsMoved();
	}
}

void CStarCamera::setOrientation(const FVector &v) {
	if (!isLocked())
		_viewport.setOrientation(v);
}

void CStarCamera::proc6(int v) {
	if (!isLocked())
		_viewport.setC(v);
}

void CStarCamera::proc7(int v) {
	if (!isLocked())
		_viewport.set10(v);
}

void CStarCamera::proc8(int v) {
	if (!isLocked())
		_viewport.set14(v);
}

void CStarCamera::proc9(int v) {
	if (!isLocked())
		_viewport.set18(v);
}

void CStarCamera::proc10(int v) {
	if (!isLocked())
		_viewport.set1C(v);
}

void CStarCamera::proc11() {
	if (!isLocked())
		_viewport.fn12();
}

void CStarCamera::proc12(StarMode mode, double v2) {
	if (!isLocked())
		_viewport.fn13(mode, v2);
}

void CStarCamera::proc13(CViewport *dest) {
	*dest = _viewport;
}

void CStarCamera::setDestination(const FVector &v) {
	FMatrix orientation = _viewport.getOrientation();
	FVector oldPos = _viewport._position;

	_mover->moveTo(oldPos, v, orientation);
}

void CStarCamera::updatePosition(CErrorCode *errorCode) {
	if (!_priorOrientation)
		_priorOrientation = new FMatrix();
	if (!_newOrientation)
		_newOrientation = new FMatrix();

	*_priorOrientation = _viewport.getOrientation();
	*_newOrientation = *_priorOrientation;

	FVector priorPos = _viewport._position;
	FVector newPos = _viewport._position;
	_mover->updatePosition(*errorCode, newPos, *_newOrientation);

	if (newPos != priorPos) {
		_viewport.setPosition(newPos);
		setIsMoved();
	}

	if (*_priorOrientation != *_newOrientation) {
		_viewport.setOrientation(*_newOrientation);
	}
}

void CStarCamera::increaseSpeed() {
	_mover->increaseSpeed();
}

void CStarCamera::decreaseSpeed() {
	_mover->decreaseSpeed();
}

void CStarCamera::fullSpeed() {
	_mover->fullSpeed();
}

void CStarCamera::stop() {
	_mover->stop();
}

void CStarCamera::reposition(double factor) {
	if (!isLocked())
		_viewport.reposition(factor);
}

void CStarCamera::setPosition(const FPose &pose) {
	if (!isLocked()) {
		_viewport.setPosition(pose);
		setIsMoved();
	}
}

void CStarCamera::proc22(FMatrix &m) {
	if (!isLocked())
		_viewport.fn15(m);
}

FPose CStarCamera::getPose() {
	return _viewport.getPose();
}

FPose CStarCamera::getRawPose() {
	return _viewport.getRawPose();
}

double CStarCamera::getThreshold() const {
	return _viewport._field10;
}

double CStarCamera::proc26() const {
	return _viewport._field14;
}

int CStarCamera::proc27() const {
	return _viewport._field24;
}

FVector CStarCamera::getRelativePos(int index, const FVector &src) {
	FVector dest;

	dest._x = ((_viewport._valArray[index] + src._x) * _viewport._centerVector._x)
		/ (_viewport._centerVector._y * src._z);
	dest._y = src._y * _viewport._centerVector._x / (_viewport._centerVector._z * src._z);
	dest._z = src._z;
	return dest;
}

FVector CStarCamera::proc29(int index, const FVector &src) {
	return _viewport.fn16(index, src);
}

FVector CStarCamera::proc30(int index, const FVector &v) {
	return _viewport.fn17(index, v);
}

FVector CStarCamera::proc31(int index, const FVector &v) {
	return _viewport.fn18(index, v);
}

void CStarCamera::setViewportAngle(const FPoint &angles) {
	debug(DEBUG_DETAILED, "setViewportAngle %f %f", angles._x, angles._y);

	if (isLocked())
		return;

	if (_matrixRow == -1) {
		// No locked markers
		FPose subX(X_AXIS, angles._y);
		FPose subY(Y_AXIS, angles._x);
		FPose sub(subX, subY);
		proc22(sub);
	} else if (_matrixRow == 0) {
		// 1 marker is locked in

		FVector row1 = _matrix._row1;
		FPose poseX(X_AXIS, angles._y);
		FPose poseY(Y_AXIS, angles._x);
		FPose pose(poseX, poseY);

		FMatrix m1 = _viewport.getOrientation();
		FVector tempV1 = _viewport._position;
		FVector tempV2 = m1._row1 * 100000;
		FVector tempV3 = tempV2 + tempV1;
		FVector tempV4 = tempV3;

		tempV2 = m1._row2 * 100000;
		FVector tempV5 = m1._row3 * 100000;
		FVector tempV6 = tempV2 + tempV1;

		FVector tempV7 = tempV5 + tempV1;
		tempV5 = tempV6;
		tempV6 = tempV7;

		tempV1 -= row1;
		tempV4 -= row1;
		tempV5 -= row1;
		tempV6 -= row1;

		tempV1 = tempV7 = tempV1.fn5(pose);
		tempV4 = tempV1 = tempV4.fn5(pose);
		tempV5 = tempV5.fn5(pose);
		tempV6 = tempV6.fn5(pose);

		tempV4 = tempV4 - tempV1;
		FVector tempV9 = tempV5 - tempV1;
		FVector tempV10 = tempV6 - tempV1;

		tempV4.normalize();
		tempV9.normalize();
		tempV10.normalize();

		tempV1 += row1;
		m1.set(tempV4, tempV9, tempV10);
		_viewport.setOrientation(m1);
		_viewport.setPosition(tempV1);
	} else if (_matrixRow == 1) {
		// 2 markers locked in
		FVector tempV2;
		DMatrix m1, m2, sub;
		DVector mrow1, mrow2, mrow3;
		DVector tempV1, diffV, multV, multV2, tempV3, tempV4, tempV5, tempV6, tempV7;
		DVector tempV8, tempV9, tempV10, tempV11, tempV12;
		DVector tempV13, tempV14, tempV15, tempV16;

		DMatrix subX(0, _matrix._row1);
		DMatrix subY(Y_AXIS, angles._y);

		tempV1 = _matrix._row2 - _matrix._row1;
		diffV = tempV1;
		m1 = diffV.fn5();
		m1 = m1.fn4(subX);
		subX = m1.fn1();
		subX = subX.fn4(subY);

		FMatrix m3 = _viewport.getOrientation();
		tempV2 = _viewport._position;
		multV._x = m3._row1._x * 1000000.0;
		multV._y = m3._row1._y * 1000000.0;
		multV._z = m3._row1._z * 1000000.0;
		tempV3._x = tempV2._x;
		tempV3._y = tempV2._y;
		tempV3._z = tempV2._z;
		multV2._z = m3._row2._z * 1000000.0;

		tempV1._x = multV._x + tempV3._x;
		tempV1._y = multV._y + tempV3._y;
		tempV1._z = multV._z + tempV3._z;
		mrow3._z = 0.0;
		mrow3._y = 0.0;
		mrow3._x = 0.0;
		multV2._x = m3._row2._x * 1000000.0;
		multV2._y = m3._row2._y * 1000000.0;
		mrow1 = tempV1;
		multV = multV2 + tempV3;
		mrow2 = multV;

		tempV7._z = m3._row3._z * 1000000.0 + tempV3._z;
		tempV7._y = m3._row3._y * 1000000.0 + tempV3._y;
		tempV7._x = m3._row3._x * 1000000.0 + tempV3._x;

		mrow3 = tempV8;
		DVector *v = tempV3.fn1(tempV9, subX);
		tempV3 = *v;
		v = mrow1.fn1(tempV10, subX);
		mrow1 = *v;
		v = mrow2.fn1(tempV11, subX);
		mrow2 = *v;
		v = mrow3.fn1(tempV12, subX);
		mrow3 = *v;

		v = tempV3.fn1(tempV13, m1);
		tempV3 = *v;
		v = mrow1.fn1(tempV14, m1);
		mrow1 = *v;
		v = mrow2.fn1(tempV15, m1);
		mrow2 = *v;
		v = mrow3.fn1(tempV16, m1);
		mrow3 = *v;

		mrow1 -= tempV3;
		mrow2 -= tempV3;
		mrow3 -= tempV3;
		mrow1.normalize();
		mrow2.normalize();
		mrow3.normalize();
		tempV16 = tempV3;

		m3.set(mrow1, mrow2, mrow3);
		_viewport.setOrientation(m3);
		_viewport.setPosition(tempV16);
	}
}

bool CStarCamera::addMatrixRow(const FVector &v) {
	if (_matrixRow >= 2)
		return false;

	CNavigationInfo data;
	_mover->copyTo(&data);
	deleteHandler();

	FVector &row = _matrix[++_matrixRow];
	row = v;
	setupHandler(&data);
	return true;
}

bool CStarCamera::removeMatrixRow() {
	if (_matrixRow == -1)
		return false;

	CNavigationInfo data;
	_mover->copyTo(&data);
	deleteHandler();

	--_matrixRow;
	setupHandler(&data);
	return true;
}

void CStarCamera::proc36(double *v1, double *v2, double *v3, double *v4) {
	_viewport.fn19(v1, v2, v3, v4);
}

void CStarCamera::load(SimpleFile *file, int param) {
	_viewport.load(file, param);
}

void CStarCamera::save(SimpleFile *file, int indent) {
	_viewport.save(file, indent);
}

bool CStarCamera::setupHandler(const CNavigationInfo *src) {
	CCameraMover *mover = nullptr;

	switch (_matrixRow) {
	case -1:
		mover = new CStarControlSub21(src);
		break;

	case 0:
	case 1:
	case 2:
		mover = new CStarControlSub22(src);
		break;

	default:
		break;
	}

	if (mover) {
		assert(!_mover);
		_mover = mover;
		return true;
	} else {
		return false;
	}
}

void CStarCamera::deleteHandler() {
	if (_mover) {
		delete _mover;
		_mover = nullptr;
	}
}

void CStarCamera::fn1(CViewport *viewport, const FVector &v) {
	if (_matrixRow == 1) {
		FMatrix newOr = viewport->getOrientation();
		FMatrix oldOr = _viewport.getOrientation();
		FVector newPos = viewport->_position;
		FVector oldPos = _viewport._position;

		_mover->proc8(oldPos, newPos, oldOr, newOr);
		CStarVector *sv = new CStarVector(this, v);
		_mover->setVector(sv);
	}
}

void CStarCamera::fn2(FVector v1, FVector v2, FVector v3) {
	// Calculations only done when the first marker is locked on
	if (_matrixRow == -1) {
		FVector tempV;
		double val1, val2, val3, val4, val5;
		double val6, val7, val8, val9;

		val1 = _viewport._centerVector._y * v1._x;
		tempV._z = _viewport._field10;
		val2 = _viewport._centerVector._y * tempV._z * v3._x;
		val3 = _viewport._centerVector._z * v1._y;
		val4 = _viewport._centerVector._z * tempV._z;
		val5 = val1 * v1._z / _viewport._centerVector._x;
		v3._z = v1._z;
		val6 = val4 * v3._y;
		val7 = val3 * v1._z / _viewport._centerVector._x;
		val8 = val6 / _viewport._centerVector._x;
		val9 = val2 / _viewport._centerVector._x;
		v3._x = val5 - _viewport._valArray[2];
		v3._y = val7;
		tempV._x = val9 - _viewport._valArray[2];
		tempV._y = val8;

		v3.normalize();
		tempV.normalize();

		FMatrix matrix = _viewport.getOrientation();
		const FVector &pos = _viewport._position;
		_mover->proc10(v3, tempV, pos, matrix);

		CStarVector *sv = new CStarVector(this, v2);
		_mover->setVector(sv);
	}
}

void CStarCamera::fn3(CViewport *viewport, const FVector &v) {
	if (_matrixRow != 0)
		return;

	DMatrix m1;
	DVector tempV1 = _matrix._row1;
	DMatrix m2(0, tempV1);

	tempV1 = v - _matrix._row1;
	m1 = tempV1.fn5();

	m1 = m1.fn4(m2);
	m2 = m1.fn1();

	DVector tempV2 = _viewport._position;
	DMatrix m4;
	m4._row1 = viewport->_position;
	m4._row2 = DVector(0.0, 0.0, 0.0);
	m4._row3 = DVector(0.0, 0.0, 0.0);
	m4._row4 = DVector(0.0, 0.0, 0.0);

	FMatrix m5 = viewport->getOrientation();
	DVector tempV3, tempV4;
	tempV4._x = m5._row1._x * 1000000.0 + m4._row1._x;
	tempV4._y = m5._row1._y * 1000000.0 + m4._row1._y;
	tempV4._z = m5._row1._z * 1000000.0 + m4._row1._z;
	tempV3._x = m5._row2._x * 1000000.0 + m4._row1._x;
	tempV3._y = m5._row2._y * 1000000.0 + m4._row1._y;
	tempV3._z = m5._row2._z * 1000000.0 + m4._row1._z;
	m4._row3 = tempV4;
	m4._row2 = tempV3;

	tempV4._x = m5._row3._x * 1000000.0;
	tempV4._y = m5._row3._y * 1000000.0;
	tempV3._x = tempV4._x + m4._row1._x;
	tempV3._y = tempV4._y + m4._row1._y;
	tempV3._z = m5._row3._z * 1000000.0 + m4._row1._z;
	m4._row4 = tempV3;

	DVector *dv = tempV2.fn1(tempV3, m2);
	tempV3 = *dv;
	dv = m4._row1.fn1(tempV3, m2);
	m4._row1 = *dv;
	dv = m4._row3.fn1(tempV3, m2);
	m4._row3 = *dv;
	dv = m4._row2.fn1(tempV3, m2);
	m4._row2 = *dv;
	dv = m4._row4.fn1(tempV3, m2);
	m4._row4 = *dv;

	// Find the angle that gives the minimum distance
	DVector tempV5;
	double minDistance = 1.0e20;
	int minDegree = 0;
	for (int degree = 0; degree < 360; ++degree) {
		tempV5 = m4._row1;
		tempV5.fn2((double)degree);
		double distance = tempV2.getDistance(tempV5);

		if (distance < minDistance) {
			minDistance = distance;
			minDegree = degree;
		}
	}

	m4._row1.fn2((double)minDegree);
	m4._row2.fn2((double)minDegree);
	m4._row3.fn2((double)minDegree);
	m4._row4.fn2((double)minDegree);
	dv = m4._row1.fn1(tempV3, m1);
	m4._row1 = *dv;
	dv = m4._row3.fn1(tempV3, m1);
	m4._row3 = *dv;
	dv = m4._row2.fn1(tempV3, m1);
	m4._row2 = *dv;
	dv = m4._row4.fn1(tempV3, m1);
	m4._row4 = *dv;

	m4._row3._x -= m4._row1._x;
	m4._row3._y -= m4._row1._y;
	m4._row3._z -= m4._row1._z;
	m4._row2._x -= m4._row1._x;
	m4._row2._y -= m4._row1._y;
	m4._row2._z = m4._row2._z - m4._row1._z;

	m4._row4._x = m4._row4._x - m4._row1._x;
	m4._row4._y = m4._row4._y - m4._row1._y;
	m4._row4._z = m4._row4._z - m4._row1._z;

	m4._row3.normalize();
	m4._row2.normalize();
	m4._row4.normalize();
	m5.set(m4._row3, m4._row2, m4._row4);

	FVector tempV6 = m4._row1;
	FMatrix m6 = _viewport.getOrientation();
	_mover->proc8(_viewport._position, tempV6, m6, m5);

	CStarVector *sv = new CStarVector(this, v);
	_mover->setVector(sv);
}

} // End of namespace Titanic
