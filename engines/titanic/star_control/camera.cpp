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

#include "titanic/star_control/camera.h"
#include "titanic/debugger.h"
#include "titanic/star_control/motion_control.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/fpoint.h"
#include "titanic/star_control/motion_control_marked.h"
#include "titanic/star_control/motion_control_unmarked.h"
#include "titanic/star_control/error_code.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"

namespace Titanic {

const double rowScale1 = 100000.0;
const double rowScale2 = 1000000.0;

FMatrix *CCamera::_priorOrientation;
FMatrix *CCamera::_newOrientation;

CCamera::CCamera(const CNavigationInfo *data) :
		_lockLevel(ZERO_LOCKED), _motion(nullptr), _isMoved(false), _isInLockingProcess(false) {
	createMotionControl(data);
}

CCamera::CCamera(CViewport *src) :
		_lockLevel(ZERO_LOCKED), _motion(nullptr), _isMoved(false), _isInLockingProcess(false), _viewport(src) {
}

void CCamera::init() {
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

void CCamera::deinit() {
	delete _priorOrientation;
	delete _newOrientation;
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

bool CCamera::isLocked() { 
	return _motion->isLocked();
}

bool CCamera::isNotInLockingProcess() { 
	return !_isInLockingProcess;
}

CCamera::~CCamera() {
	deleteMotionController();
}

void CCamera::setViewport(const CViewport *src) {
	_viewport.copyFrom(src);
}

void CCamera::setMotion(const CNavigationInfo *src) {
	_motion->setMotion(src);
}

void CCamera::setPosition(const FVector &v) {
	if (!isLocked()) {
		_viewport.setPosition(v);
		setIsMoved();
	}
}

void CCamera::setOrientation(const FVector &v) {
	if (!isLocked())
		_viewport.setOrientation(v);
}

// This never gets called
void CCamera::setRoleAngle(double angle) {
	if (!isLocked())
		_viewport.SetRoleAngle(angle);
}

// This never gets called
void CCamera::setFrontClip(double n) {
	if (!isLocked())
		_viewport.setFrontClip(n);
}

// This never gets called
void CCamera::SetBackClip(double f) {
	if (!isLocked())
		_viewport.setBackClip(f);
}

// This never gets called
void CCamera::setCenterYAngle(int v) {
	if (!isLocked())
		_viewport.setCenterYAngle(v);
}

// This never gets called
void CCamera::setCenterZAngle(int v) {
	if (!isLocked())
		_viewport.setCenterZAngle(v);
}

void CCamera::randomizeOrientation() {
	if (!isLocked())
		_viewport.randomizeOrientation();
}

void CCamera::setFields(StarMode mode, double val) {
	if (!isLocked())
		_viewport.changeStarColorPixel(mode, val);
}

void CCamera::setDestination(const FVector &v) {
	FMatrix orientation = _viewport.getOrientation();
	FVector oldPos = _viewport._position;

	_motion->moveTo(oldPos, v, orientation);
}

void CCamera::updatePosition(CErrorCode *errorCode) {
	if (!_priorOrientation)
		_priorOrientation = new FMatrix();
	if (!_newOrientation)
		_newOrientation = new FMatrix();

	*_priorOrientation = _viewport.getOrientation();
	*_newOrientation = *_priorOrientation;

	FVector priorPos = _viewport._position;
	FVector newPos = _viewport._position;
	_motion->updatePosition(*errorCode, newPos, *_newOrientation);

	if (newPos != priorPos) {
		_viewport.setPosition(newPos);
		setIsMoved();
	}

	if (*_priorOrientation != *_newOrientation) {
		_viewport.setOrientation(*_newOrientation);
	}
}

void CCamera::accelerate() {
	_motion->accelerate();
}

void CCamera::deccelerate() {
	_motion->deccelerate();
}

void CCamera::fullSpeed() {
	_motion->fullSpeed();
}

void CCamera::stop() {
	_motion->stop();
}

void CCamera::reposition(double factor) {
	if (!isLocked())
		_viewport.reposition(factor);
}

void CCamera::setPosition(const FPose &pose) {
	if (!isLocked()) {
		_viewport.setPosition(pose);
		setIsMoved();
	}
}

void CCamera::changeOrientation(FMatrix &m) {
	if (!isLocked())
		_viewport.changeOrientation(m);
}

FPose CCamera::getPose() {
	return _viewport.getPose();
}

FPose CCamera::getRawPose() {
	return _viewport.getRawPose();
}

double CCamera::getFrontClip() const {
	return _viewport._frontClip;
}

double CCamera::getBackClip() const {
	return _viewport._backClip;
}

StarColor CCamera::getStarColor() const {
	return _viewport._starColor;
}

FVector CCamera::getRelativePos(int index, const FVector &src) {
	FVector dest;

	double val;
	if (index == 2) {
		val = _viewport._isZero;
	} else {
		val = _viewport._valArray[index];
	}

	dest._x = ((val + src._x) * _viewport._centerVector._x)
		/ (_viewport._centerVector._y * src._z);
	dest._y = src._y * _viewport._centerVector._x / (_viewport._centerVector._z * src._z);
	dest._z = src._z;
	return dest;
}

FVector CCamera::getRelativePosNoCentering(int index, const FVector &src) {
	return _viewport.getRelativePosNoCentering(index, src);
}

FVector CCamera::getRelativePosCentering(int index, const FVector &v) {
	return _viewport.getRelativePosCentering(index, v);
}

FVector CCamera::getRelativePosCenteringRaw(int index, const FVector &v) {
	return _viewport.getRelativePosCenteringRaw(index, v);
}

void CCamera::setViewportAngle(const FPoint &angles) {
	debug(DEBUG_DETAILED, "setViewportAngle %f %f", angles._x, angles._y);

	if (isLocked())
		return;

	switch(_lockLevel) {
	case ZERO_LOCKED: {
		FPose subX(X_AXIS, angles._y);
		FPose subY(Y_AXIS, -angles._x); // needs to be negative or looking left will cause the view to go right
		FPose sub(subX, subY);
		changeOrientation(sub);
		break;
	}

	case ONE_LOCKED: {
		FVector row1 = _lockedStarsPos._row1;
		FPose poseX(X_AXIS, angles._y);
		FPose poseY(Y_AXIS, -angles._x); // needs to be negative or looking left will cause the view to go right
		FPose pose(poseX, poseY);

		FMatrix m1 = _viewport.getOrientation();
		FVector tempV1 = _viewport._position;
		FVector tempV2 = m1._row1 * rowScale1;
		FVector tempV3 = tempV2 + tempV1;
		FVector tempV4 = tempV3;

		tempV2 = m1._row2 * rowScale1;
		FVector tempV5 = m1._row3 * rowScale1;
		FVector tempV6 = tempV2 + tempV1;

		FVector tempV7 = tempV5 + tempV1;
		tempV5 = tempV6;
		tempV6 = tempV7;

		tempV1 -= row1;
		tempV4 -= row1;
		tempV5 -= row1;
		tempV6 -= row1;

		tempV1 = tempV1.matProdRowVect(pose);
		tempV4 = tempV4.matProdRowVect(pose);
		tempV5 = tempV5.matProdRowVect(pose);
		tempV6 = tempV6.matProdRowVect(pose);

		tempV4 -= tempV1;
		tempV5 -= tempV1;
		tempV6 -= tempV1;

		float unusedScale = 0.0;
		if (!tempV4.normalize(unusedScale) ||
				!tempV5.normalize(unusedScale) ||
				!tempV6.normalize(unusedScale)) {
			// Do the normalization, put the scale amount in unusedScale,
			// but if it is unsuccessful, crash
			assert(unusedScale);
		}

		tempV1 += row1;
		m1.set(tempV4, tempV5, tempV6);
		_viewport.setOrientation(m1);
		_viewport.setPosition(tempV1);
		break;
	}

	case TWO_LOCKED: {
		FVector tempV2;
		FPose m1;
		FVector mrow1, mrow2, mrow3;
		FVector tempV1, diffV, multV, multV2, tempV3, tempV7;

		FPose subX(0, _lockedStarsPos._row1);
		FPose subY(Y_AXIS, angles._y);

		tempV1 = _lockedStarsPos._row2 - _lockedStarsPos._row1;
		diffV = tempV1;
		m1 = diffV.formRotXY();
		FPose m11;
		fposeProd(m1, subX, m11);

		subX = m11.inverseTransform();
		FPose m12;
		fposeProd(subX, subY, m12);

		FMatrix m3 = _viewport.getOrientation();
		tempV2 = _viewport._position;
		multV._x = m3._row1._x * rowScale2;
		multV._y = m3._row1._y * rowScale2;
		multV._z = m3._row1._z * rowScale2;
		tempV3._x = tempV2._x;
		tempV3._y = tempV2._y;
		tempV3._z = tempV2._z;
		multV2._z = m3._row2._z * rowScale2;

		tempV1._x = multV._x + tempV3._x;
		tempV1._y = multV._y + tempV3._y;
		tempV1._z = multV._z + tempV3._z;
		mrow3._z = 0.0;
		mrow3._y = 0.0;
		mrow3._x = 0.0;
		multV2._x = m3._row2._x * rowScale2;
		multV2._y = m3._row2._y * rowScale2;
		mrow1 = tempV1;
		multV = multV2 + tempV3;
		mrow2 = multV;

		tempV7._z = m3._row3._z * rowScale2 + tempV3._z;
		tempV7._y = m3._row3._y * rowScale2 + tempV3._y;
		tempV7._x = m3._row3._x * rowScale2 + tempV3._x;

		mrow3 = tempV7;
		tempV3 = tempV3.matProdRowVect(m12);
		mrow1 = mrow1.matProdRowVect(m12);
		mrow2 = mrow2.matProdRowVect(m12);
		mrow3 = mrow3.matProdRowVect(m12);

		tempV3 = tempV3.matProdRowVect(m11);
		mrow1 = mrow1.matProdRowVect(m11);
		mrow2 = mrow2.matProdRowVect(m11);
		mrow3 = mrow3.matProdRowVect(m11);

		mrow1 -= tempV3;
		mrow2 -= tempV3;
		mrow3 -= tempV3;

		float unusedScale=0.0;
		if (!mrow1.normalize(unusedScale) ||
				!mrow2.normalize(unusedScale) ||
				!mrow3.normalize(unusedScale)) {
			// Do the normalization, put the scale amount in unusedScale,
			// but if it is unsuccessful, crash
			assert(unusedScale);
		}

		m3.set(mrow1, mrow2, mrow3);
		_viewport.setOrientation(m3);
		_viewport.setPosition(tempV3);
		break;
	}

	// All three stars are locked on in this case so the camera does not move
	// in response to the users mouse movements
	case THREE_LOCKED:
	default:
		break;
	}
}

bool CCamera::addLockedStar(const FVector v) {
	if (_lockLevel == THREE_LOCKED)
		return false;

	CNavigationInfo data;
	_motion->getMotion(&data);
	deleteMotionController();

	FVector &row = _lockedStarsPos[(int)_lockLevel];
	_lockLevel = (StarLockLevel)((int)_lockLevel + 1);
	row = v;
	createMotionControl(&data);
	return true;
}

bool CCamera::removeLockedStar() {
	if (_lockLevel == ZERO_LOCKED)
		return false;

	CNavigationInfo data;
	_motion->getMotion(&data);
	deleteMotionController();

	_lockLevel = (StarLockLevel)((int)_lockLevel - 1);
	createMotionControl(&data);
	return true;
}

void CCamera::getRelativeXCenterPixels(double *v1, double *v2, double *v3, double *v4) {
	_viewport.getRelativeXCenterPixels(v1, v2, v3, v4);
}

void CCamera::load(SimpleFile *file, int param) {
	_viewport.load(file, param);
}

void CCamera::save(SimpleFile *file, int indent) {
	_viewport.save(file, indent);
}

bool CCamera::createMotionControl(const CNavigationInfo *src) {
	CMotionControl *motion = nullptr;

	switch (_lockLevel) {
	case ZERO_LOCKED:
		motion = new CMotionControlUnmarked(src);
		break;

	case ONE_LOCKED:
	case TWO_LOCKED:
	case THREE_LOCKED:
		motion = new CMotionControlMarked(src);
		break;

	default:
		break;
	}

	if (motion) {
		assert(!_motion);
		_motion = motion;
		return true;
	} else {
		return false;
	}
}

void CCamera::deleteMotionController() {
	if (_motion) {
		delete _motion;
		_motion = nullptr;
		_isInLockingProcess = false;
	}
}

bool CCamera::lockMarker1(FVector v1, FVector firstStarPosition, FVector v3) {
	if (_lockLevel != ZERO_LOCKED)
		return true;

	_isInLockingProcess = true;
	FVector tempV;
	double val1, val2, val3, val4, val5;
	double val6, val7, val8, val9;

	val1 = _viewport._centerVector._y * v1._x;
	tempV._z = _viewport._frontClip;
	val2 = _viewport._centerVector._y * tempV._z * v3._x;
	val3 = _viewport._centerVector._z * v1._y;
	val4 = _viewport._centerVector._z * tempV._z;
	val5 = val1 * v1._z / _viewport._centerVector._x;
	v3._z = v1._z;
	val6 = val4 * v3._y;
	val7 = val3 * v1._z / _viewport._centerVector._x;
	val8 = val6 / _viewport._centerVector._x;
	val9 = val2 / _viewport._centerVector._x;
	v3._x = val5 - _viewport._isZero; // TODO: _viewport._isZero is always zero
	v3._y = val7;
	tempV._x = val9 - _viewport._isZero; // TODO: _viewport._isZero is always zero
	tempV._y = val8;

	float unusedScale = 0.0;
	if (!v3.normalize(unusedScale) || !tempV.normalize(unusedScale)) {
		// Do the normalization, put the scale amount in unusedScale,
		// but if it is unsuccessful, crash
		assert(unusedScale);
	}

	FMatrix matrix = _viewport.getOrientation();
	const FVector &pos = _viewport._position;
	_motion->transitionBetweenOrientations(v3, tempV, pos, matrix);

	CCallbackHandler *callback = new CCallbackHandler(this, firstStarPosition);
	_motion->setCallback(callback);

	return	true;
}

bool CCamera::lockMarker2(CViewport *viewport, const FVector &secondStarPosition) {
	if (_lockLevel != ONE_LOCKED)
		return true;

	_isInLockingProcess = true;
	FVector firstStarPosition = _lockedStarsPos._row1;
	FPose m3(0, firstStarPosition); // Identity matrix and row4 as the 1st stars position
	FVector starDelta = secondStarPosition - firstStarPosition;
	FPose m10 = starDelta.formRotXY();
	FPose m11;
	fposeProd(m10, m3, m11);

	m10 = m11.inverseTransform();

	FVector oldPos = _viewport._position;

	FPose m4;
	m4._row1 = viewport->_position;
	m4._row2 = FVector(0.0, 0.0, 0.0);
	m4._row3 = FVector(0.0, 0.0, 0.0);
	m4._vector = FVector(0.0, 0.0, 0.0);

	FMatrix newOr = viewport->getOrientation();
	float yVal1 = newOr._row1._y * rowScale2;
	float zVal1 = newOr._row1._z * rowScale2;
	float xVal1 = newOr._row2._x * rowScale2;
	float yVal2 = newOr._row2._y * rowScale2;
	float zVal2 = newOr._row2._z * rowScale2;
	float zVal3 = zVal1 + m4._row1._z;
	float yVal3 = yVal1 + m4._row1._y;
	float xVal2 = newOr._row1._x * rowScale2 + m4._row1._x;
	float zVal4 = zVal2 + m4._row1._z;
	float yVal4 = yVal2 + m4._row1._y;
	float xVal3 = xVal1 + m4._row1._x;

	FVector tempV4(xVal2, yVal3, zVal3);
	FVector tempV3(xVal3, yVal4, zVal4);
	m4._row3 = tempV4;

	FVector tempV5;
	tempV5._x = newOr._row3._x * rowScale2;
	tempV5._y = newOr._row3._y * rowScale2;
	m4._row2 = tempV3;

	tempV3._x = tempV5._x + m4._row1._x;
	tempV3._y = tempV5._y + m4._row1._y;
	tempV3._z = newOr._row3._z * rowScale2 + m4._row1._z;
	m4._vector = tempV3;


	FVector viewPosition2 = oldPos.matProdRowVect(m10);
	m3 = m4.compose2(m10);

	float minDistance;
	FVector x1(viewPosition2);
	FVector x2(m3._row1);
	// Find the angle of rotation for m4._row1 that gives the minimum distance to viewPosition
	float minDegree = calcAngleForMinDist(x1, x2, minDistance);

	m3.rotVectAxisY((double)minDegree);
	FPose m13;
	m13 = m3.compose2(m11);

	m13._row3 -= m13._row1;
	m13._row2 -= m13._row1;
	m13._vector -= m13._row1;



	float unusedScale=0.0;
	if (!m13._row2.normalize(unusedScale) ||
			!m13._row3.normalize(unusedScale) ||
			!m13._vector.normalize(unusedScale) ) {
		// Do the normalizations, put the scale amount in unusedScale,
		// but if any of the normalizations are unsuccessful, crash
		assert(unusedScale);
	}

	newOr.set(m13._row3, m13._row2, m13._vector);

	FVector newPos = m13._row1;
	FMatrix oldOr = _viewport.getOrientation();

	// WORKAROUND: set old position to new position (1st argument), this prevents 
	// locking issues when locking the 2nd star. Fixes #9961.
	_motion->transitionBetweenPosOrients(newPos, newPos, oldOr, newOr);
	CCallbackHandler *callback = new CCallbackHandler(this, secondStarPosition);
	_motion->setCallback(callback);

	return	true;
}

bool CCamera::lockMarker3(CViewport *viewport, const FVector &thirdStarPosition) {
	if (_lockLevel != TWO_LOCKED)
		return true;

	_isInLockingProcess = true;
	FMatrix newOr = viewport->getOrientation();
	FMatrix oldOr = _viewport.getOrientation();
	FVector newPos = viewport->_position;
	//FVector oldPos = _viewport._position;

	// WORKAROUND: set old position to new position (1st argument), this prevents 
	// locking issues when locking the 3rd star. Fixes #9961.
	_motion->transitionBetweenPosOrients(newPos, newPos, oldOr, newOr);

	CCallbackHandler *callback = new CCallbackHandler(this, thirdStarPosition);
	_motion->setCallback(callback);

	return true;
}

float CCamera::calcAngleForMinDist(FVector &x, FVector &y, float &minDistance) {
	FVector tempPos;
	minDistance = (float)1.0e20;
	float minDegree = 0.0;
	float degInc = 1.0; // one degree steps
	int nDegrees = floor(360.0/degInc);
	for (int i = 0; i < nDegrees; ++i) {
		tempPos = y;
		tempPos.rotVectAxisY((float)degInc*i);
		float distance = x.getDistance(tempPos);

		if (distance < minDistance) {
			minDistance = distance;
			minDegree = (float) degInc*i;
		}
	}
	return minDegree;
}

} // End of namespace Titanic
