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

#include "titanic/star_control/star_field.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/titanic.h"

namespace Titanic {

CStarField::CStarField() : _points1On(false), _points2On(false), _mode(MODE_STARFIELD),
		_showCrosshairs(true), _val5(0), _isSolved(false) {
}

void CStarField::load(SimpleFile *file) {
	_sub7.load(file);
	_sub8.load(file);
	_points1On = file->readNumber();
	_points2On = file->readNumber();
	_mode = (StarMode)file->readNumber();
	_showCrosshairs = file->readNumber();
	_isSolved = file->readNumber();
}

void CStarField::save(SimpleFile *file, int indent) {
	_sub7.save(file, indent);
	_sub8.save(file, indent);
	file->writeNumberLine(_points1On, indent);
	file->writeNumberLine(_points2On, indent);
	file->writeNumberLine(_mode, indent);
	file->writeNumberLine(_showCrosshairs, indent);
	file->writeNumberLine(_isSolved, indent);
}

bool CStarField::initDocument() {
	bool valid = setup() && _points1.initialize();
	if (valid)
		valid = _sub5.setup();
	if (valid)
		valid = _points1.initialize();
	if (valid)
		valid = _points2.initialize();

	return valid;
}

void CStarField::render(CVideoSurface *surface, CStarControlSub12 *sub12) {
	CSurfaceArea surfaceArea(surface);
	draw(&surfaceArea, sub12, &_sub5);
	if (_showCrosshairs)
		drawCrosshairs(&surfaceArea);

	_sub7.draw(&surfaceArea, sub12, nullptr);
	_sub8.draw(&surfaceArea);

	if (_points2On)
		_points2.draw(&surfaceArea, sub12);
	if (_points1On)
		_points1.draw(&surfaceArea, sub12);

	fn4(&surfaceArea, sub12);
}

int CStarField::get1() const {
	return _points1On;
}

void CStarField::set1(int val) {
	_points1On = val;
}

int CStarField::get2() const {
	return _points2On;
}

void CStarField::set2(int val) {
	_points2On = val;
}

int CStarField::get54() const {
	return _sub5.get4();
}

void CStarField::set54(int val) {
	_sub5.set4(val);
}

StarMode CStarField::getMode() const {
	return _mode;
}

void CStarField::setMode(StarMode mode) {
	_mode = mode;
}

void CStarField::toggleCrosshairs() {
	_showCrosshairs = !_showCrosshairs;
}

bool CStarField::setCrosshairs(bool isVisible) {
	bool oldVal = _showCrosshairs;
	_showCrosshairs = isVisible;
	return oldVal;
}

int CStarField::get88() const {
	return _sub8._field8;
}

int CStarField::get5() const {
	return _val5;
}

void CStarField::setSolved() {
	_isSolved = _sub8._field8 == 2;
}

bool CStarField::isSolved() const {
	return _isSolved;
}

void CStarField::fn1(CErrorCode *errorCode) {
	_sub5.proc3(errorCode);
}

void CStarField::drawCrosshairs(CSurfaceArea *surfaceArea) {
	uint oldPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 0x323232;
	surfaceArea->setColorFromPixel();

	surfaceArea->fillRect(FRect(202.60417, 63.75, 397.39584, 63.75));
	surfaceArea->fillRect(FRect(202.60417, 276.25, 397.39584, 276.25));
	surfaceArea->fillRect(FRect(193.75, 72.604164, 193.75, 267.39584));
	surfaceArea->fillRect(FRect(406.25, 72.604164, 406.25, 267.39584));
	surfaceArea->fillRect(FRect(202.60417, 63.75, 202.60417, 68.177086));
	surfaceArea->fillRect(FRect(397.39584, 63.75, 397.39584, 68.177086));
	surfaceArea->fillRect(FRect(202.60417, 276.25, 202.60417, 271.82291));
	surfaceArea->fillRect(FRect(397.39584, 276.25, 397.39584, 271.82291));
	surfaceArea->fillRect(FRect(193.75, 72.604164, 198.17708, 72.604164));
	surfaceArea->fillRect(FRect(193.75, 267.39584, 198.17708, 267.39584));
	surfaceArea->fillRect(FRect(406.25, 72.604164, 401.82291, 72.604164));
	surfaceArea->fillRect(FRect(406.25, 267.39584, 401.82291, 267.39584));
	surfaceArea->fillRect(FRect(300.0, 63.75, 300.0, 54.895832));
	surfaceArea->fillRect(FRect(300.0, 276.25, 300.0, 285.10416));
	surfaceArea->fillRect(FRect(193.75, 170.0, 184.89583, 170.0));
	surfaceArea->fillRect(FRect(406.25, 170.0, 415.10416, 170.0));

	surfaceArea->_pixel = oldPixel;
	surfaceArea->setColorFromPixel();
}

void CStarField::fn4(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	FVector v1, v2, v3;
	_val5 = 0;

	if (_mode == MODE_STARFIELD) {
		if (fn5(surfaceArea, sub12, v1, v2, v3) > -1.0) {
			surfaceArea->_pixel = 0xA0A0;
			surfaceArea->setColorFromPixel();
			surfaceArea->fillRect(FRect(v1._x, v1._y, v3._x, v3._y));
		}
	}
}

double CStarField::fn5(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12,
		FVector &v1, FVector &v2, FVector &v3) {
	if (_sub8._fieldC < 0)
		return -1.0;

	const CBaseStarEntry *dataP = _sub7.getDataPtr(_sub8._fieldC);
	v2 = dataP->_position;
	FVector tv;
	sub12->proc29(2, v2, tv);

	if (sub12->proc25() >= tv._z)
		return -1.0;

	sub12->proc28(2, tv, tv);

	v1 = FVector(tv._x + surfaceArea->_centroid._x,
		tv._y + surfaceArea->_centroid._y, tv._z);
	FPoint pt = _sub8.getPosition();
	v3 = FVector(pt._x, pt._y, 1.0);

	double incr = (v1._x - pt._x) * (v1._x - pt._x);
	if (incr > 3600.0)
		return -1.0;
	if ((v1._y - pt._y) * (v1._y - pt._y) + incr > 3600.0)
		return -1.0;

	_val5 = 1;
	return v1._y - pt._y;
}

void CStarField::fn6(CVideoSurface *surface, CStarControlSub12 *sub12) {
	CSurfaceArea surfaceArea(surface);
	_sub8.fn1(this, &surfaceArea, sub12);
}

void CStarField::fn7() {
	_sub8.fn3();
	setSolved();
}

void CStarField::fn8(CVideoSurface *surface) {
	_sub8.fn2(surface, this, &_sub7);
	setSolved();
}

bool CStarField::mouseButtonDown(CVideoSurface *surface, CStarControlSub12 *sub12,
		int flags, const Common::Point &pt) {
	if (_mode == MODE_STARFIELD) {
		CSurfaceArea surfaceArea(surface);
		return selectStar(&surfaceArea, sub12, pt);
	} else {
		int starNum = _sub8.indexOf(pt);
		if (starNum >= 0) {
			_sub8.selectStar(starNum, surface, this, &_sub7);
			return true;
		}

		return false;
	}
}

const CBaseStarEntry *CStarField::getRandomStar() const {
	if (_data.empty())
		return nullptr;

	return getDataPtr(g_vm->getRandomNumber(_data.size() - 1));
}

const CBaseStarEntry *CStarField::getStar(int index) const {
	return (index < 0 || index >= (int)_data.size()) ? nullptr : getDataPtr(index);
}

} // End of namespace Titanic
