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
#include "titanic/star_control/fvector.h"
#include "titanic/debugger.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

CViewport::CViewport() {
	_spin = 0.0;
	_frontClip = 800.0;
	_backClip = 10000.0;
	_centerYAngleDegrees = 20.0;
	_centerZAngleDegrees = 20.0;
	_width = 600;
	_height = 340;
	_starColor = PINK; // default for starview
	_poseUpToDate = false;
	Common::fill(&_valArray[0], &_valArray[2], 0.0);
	_isZero = 0.0; // seems to always be zero
	_pixel1OffSetX = 0.0;
	_pixel2OffSetX = 0.0;
}

CViewport::CViewport(CViewport *src) :
		_orientation(src->_orientation), _currentPose(src->_currentPose), _rawPose(src->_rawPose) {
	_position = src->_position;
	_spin = src->_spin;
	_frontClip = src->_frontClip;
	_backClip = src->_backClip;
	_centerYAngleDegrees = src->_centerYAngleDegrees;
	_centerZAngleDegrees = src->_centerZAngleDegrees;
	_width = src->_width;
	_height = src->_height;

	_center = src->_center;
	_centerVector = src->_centerVector;
	_starColor = src->_starColor;

	Common::copy(&src->_valArray[0], &src->_valArray[2], &_valArray[0]);
	_isZero = src->_isZero;
	_pixel1OffSetX = src->_pixel1OffSetX;
	_pixel2OffSetX = src->_pixel2OffSetX;
	_poseUpToDate = false;
}

void CViewport::copyFrom(const CViewport *src) {
	error("Unused function");
}

void CViewport::load(SimpleFile *file, int param) {
	_position._x = file->readFloat();
	_position._y = file->readFloat();
	_position._z = file->readFloat();
	_spin = file->readFloat();
	_frontClip = file->readFloat();
	_backClip = file->readFloat();
	_centerYAngleDegrees = file->readFloat();
	_centerZAngleDegrees = file->readFloat();

	int widthHeight = file->readNumber();
	_width = widthHeight & 0xffff;
	_height = widthHeight >> 16;
	int field24 = file->readNumber(); //0 = White, 2 = Pink
	_starColor = (StarColor) field24;

	for (int idx = 0; idx < 2; ++idx)
		_valArray[idx] = file->readFloat();

	_isZero = file->readFloat();
	_pixel1OffSetX = file->readFloat();
	_pixel2OffSetX = file->readFloat();

	_orientation.load(file, param);
	_poseUpToDate = false;
}

void CViewport::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_position._x, indent);
	file->writeFloatLine(_position._y, indent);
	file->writeFloatLine(_position._z, indent);
	file->writeFloatLine(_spin, indent);
	file->writeFloatLine(_frontClip, indent);
	file->writeFloatLine(_backClip, indent);
	file->writeFloatLine(_centerYAngleDegrees, indent);
	file->writeFloatLine(_centerZAngleDegrees, indent);
	file->writeNumberLine(_width | (_height << 16), indent);
	int field24 = (int)_starColor;
	file->writeNumberLine(field24, indent);

	for (int idx = 0; idx < 2; ++idx)
		file->writeFloatLine(_valArray[idx], indent);

	file->writeFloatLine(_isZero, indent);
	file->writeFloatLine(_pixel1OffSetX, indent);
	file->writeFloatLine(_pixel2OffSetX, indent);

	_orientation.save(file, indent);
}

void CViewport::setPosition(const FVector &v) {
	debugC(DEBUG_INTERMEDIATE, kDebugStarfield, "Setting starmap position to %s", v.toString().c_str());
	_position = v;
	_poseUpToDate = false;
}

void CViewport::setPosition(const FPose &pose) {
	_position = _position.matProdRowVect(pose);
	_poseUpToDate = false;
}

void CViewport::setOrientation(const FMatrix &m) {
	_orientation = m;
	_poseUpToDate = false;
}

void CViewport::setOrientation(const FVector &v) {
	_orientation.set(v);
	_poseUpToDate = false;
}

void CViewport::SetRoleAngle(double angle) {
	_spin = angle;
	_poseUpToDate = false;
}

void CViewport::setFrontClip(double dist) {
	_frontClip = dist;
	_poseUpToDate = false;
}

void CViewport::setBackClip(double dist) {
	_backClip = dist;
}

void CViewport::setCenterYAngle(double angleDegrees) {
	_centerYAngleDegrees = angleDegrees;
	_poseUpToDate = false;
}

void CViewport::setCenterZAngle(double angleDegrees) {
	_centerZAngleDegrees = angleDegrees;
	_poseUpToDate = false;
}

void CViewport::randomizeOrientation() {
	_orientation.identity();

	double ranRotAngleX = g_vm->getRandomNumber(359);
	double ranRotAngleY = g_vm->getRandomNumber(359);
	double ranRotAngleZ = g_vm->getRandomNumber(359);

	FPose m1(X_AXIS, ranRotAngleX);
	FPose m2(Y_AXIS, ranRotAngleY);
	FPose m3(Z_AXIS, ranRotAngleZ);

	FPose s1(m1, m2);
	FPose s2(s1, m3);

	_orientation.matRProd(s2);
	_poseUpToDate = false;
}

void CViewport::changeStarColorPixel(StarMode mode, double pixelOffSet) {
	// pixelOffset is usually 0.0, 30.0, or 28000.0 
	if (mode == MODE_PHOTO) {
		_valArray[0] = pixelOffSet;
		_valArray[1] = -pixelOffSet;
	} else {
		_pixel1OffSetX = pixelOffSet;
		_pixel2OffSetX = -pixelOffSet;
	}

	_isZero = 0.0;
	_starColor = pixelOffSet ? PINK : WHITE;
}

void CViewport::reposition(double factor) {
	_position._x = _orientation._row3._x * factor + _position._x;
	_position._y = _orientation._row3._y * factor + _position._y;
	_position._z = _orientation._row3._z * factor + _position._z;
	_poseUpToDate = false;
}

void CViewport::changeOrientation(const FMatrix &matrix) {
	_orientation.matLProd(matrix);
	_poseUpToDate = false;
}

FPose CViewport::getPose() {
	if (!_poseUpToDate)
		reset();

	return _currentPose;
}

FPose CViewport::getRawPose() {
	if (!_poseUpToDate)
		reset();

	return _rawPose;
}


// TODO: should index be used here like 
// getRelativePosCentering/getRelativePosCentering2?
// CCamera::getRelativePosCentering is calling this with an index of
// 2 which corresponds to _isZero which has value 0.
FVector CViewport::getRelativePosNoCentering(int index, const FVector &src) {
	FPose current_pose = getPose();
	FVector dest = src.matProdRowVect(current_pose);
	return dest;
}

FVector CViewport::getRelativePosCentering(int index, const FVector &src) {
	FVector dest;
	FPose pose = getPose();
	FVector tv = src.matProdRowVect(pose);

	double val;
	if (index <2) {
		val = _valArray[index];
	} else if (index == 2) {
		val = _isZero;
	} else if (index == 3) {
		val = _pixel1OffSetX;
	} else {
		val = _pixel2OffSetX;
	}

	dest._x = (val + tv._x)
		* _centerVector._x / (_centerVector._y * tv._z);
	dest._y = (tv._y * _centerVector._x) / (_centerVector._z * tv._z);
	dest._z = tv._z;
	return dest;
}

// Similar to getRelativePosCentering, but uses the raw/transpose version of Pose
FVector CViewport::getRelativePosCenteringRaw(int index, const FVector &src) {
	FVector dest;
	FPose pose = getRawPose();
	FVector tv = src.matProdRowVect(pose);

	double val;
	if (index <2) {
		val = _valArray[index];
	} else if (index == 2) {
		val = _isZero;
	} else if (index == 3) {
		val = _pixel1OffSetX;
	} else {
		val = _pixel2OffSetX;
	}

	dest._x = (val + tv._x)
		* _centerVector._x / (_centerVector._y * tv._z);
	dest._y = (tv._y * _centerVector._x) / (_centerVector._z * tv._z);
	dest._z = tv._z;
	return dest;
}

void CViewport::getRelativeXCenterPixels(double *v1, double *v2, double *v3, double *v4) {
	*v1 = _centerVector._x / _centerVector._y;
	*v2 = _centerVector._x / _centerVector._z;
	*v3 = _pixel1OffSetX;
	*v4 = _pixel2OffSetX;
}

void CViewport::reset() {
	_rawPose.copyFrom(_orientation);
	_rawPose._vector = _position;
	_currentPose = _rawPose.inverseTransform();
	_poseUpToDate = true;

	_center = FPoint((double)_width * 0.5, (double)_height * 0.5);
	_centerVector._x = MIN(_center._x, _center._y);
	_centerVector._y = tan(Common::deg2rad<double>(_centerYAngleDegrees));
	_centerVector._z = tan(Common::deg2rad<double>(_centerZAngleDegrees));
}

const FMatrix &CViewport::getOrientation() const {
	return _orientation;
}

} // End of namespace Titanic
