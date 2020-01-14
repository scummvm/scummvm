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
#include "titanic/star_control/camera.h"
#include "titanic/titanic.h"

namespace Titanic {

CStarField::CStarField() : _renderBoundaries(false), _renderConstMap(false), _mode(MODE_STARFIELD),
		_showBox(true), _closeToMarker(false), _isSolved(false) {
}

void CStarField::load(SimpleFile *file) {
	_markers.load(file);
	_crosshairs.load(file);
	_renderBoundaries = file->readNumber();
	_renderConstMap = file->readNumber();
	_mode = (StarMode)file->readNumber();
	_showBox = file->readNumber();
	_isSolved = file->readNumber();
}

void CStarField::save(SimpleFile *file, int indent) {
	_markers.save(file, indent);
	_crosshairs.save(file, indent);
	file->writeNumberLine(_renderBoundaries, indent);
	file->writeNumberLine(_renderConstMap, indent);
	file->writeNumberLine(_mode, indent);
	file->writeNumberLine(_showBox, indent);
	file->writeNumberLine(_isSolved, indent);
}

bool CStarField::initDocument() {
	bool valid = setup() && _constBounds.initialize();
	if (valid)
		valid = _starCloseup.setup();
	if (valid)
		valid = _constMap.initialize();

	return valid;
}

void CStarField::render(CVideoSurface *surface, CCamera *camera) {
	CSurfaceArea surfaceArea(surface);
	draw(&surfaceArea, camera, &_starCloseup);
	if (_showBox)
		drawBox(&surfaceArea);

	_markers.draw(&surfaceArea, camera, nullptr);
	_crosshairs.draw(&surfaceArea);

	if (_renderConstMap)
		_constMap.draw(&surfaceArea, camera);
	if (_renderBoundaries)
		_constBounds.draw(&surfaceArea, camera);

	renderLockLine(&surfaceArea, camera);
}

bool CStarField::getBoundaryState() const {
	return _renderBoundaries;
}

void CStarField::setBoundaryState(bool state) {
	_renderBoundaries = state;
}

bool CStarField::getConstMapState() const {
	return _renderConstMap;
}

void CStarField::setConstMapState(bool state) {
	_renderConstMap = state;
}

int CStarField::get54() const {
	return _starCloseup.get4();
}

void CStarField::set54(int val) {
	_starCloseup.set4(val);
}

StarMode CStarField::getMode() const {
	return _mode;
}

void CStarField::setMode(StarMode mode) {
	_mode = mode;
}

void CStarField::toggleBox() {
	_showBox = !_showBox;
}

bool CStarField::setBoxVisible(bool isVisible) {
	bool oldVal = _showBox;
	_showBox = isVisible;
	return oldVal;
}

int CStarField::getMatchedIndex() const {
	return _crosshairs._matchIndex;
}

bool CStarField::isCloseToMarker() const {
	return _closeToMarker;
}

void CStarField::setSolved() {
	_isSolved = _crosshairs._matchIndex >= 2;
}

bool CStarField::isSolved() const {
	return _isSolved;
}

bool CStarField::isSkipped() const {
	return _crosshairs.isSkipped();
}

void CStarField::skipPuzzle() {
	_crosshairs._matchIndex = 3;
	setSolved();
}

void CStarField::fn1(CErrorCode *errorCode) {
	_starCloseup.proc3(errorCode);
}

void CStarField::drawBox(CSurfaceArea *surfaceArea) {
	uint oldPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 0x323232;
	surfaceArea->setColorFromPixel();

	surfaceArea->drawLine(FRect(202.60417, 63.75, 397.39584, 63.75));
	surfaceArea->drawLine(FRect(202.60417, 276.25, 397.39584, 276.25));
	surfaceArea->drawLine(FRect(193.75, 72.604164, 193.75, 267.39584));
	surfaceArea->drawLine(FRect(406.25, 72.604164, 406.25, 267.39584));
	surfaceArea->drawLine(FRect(202.60417, 63.75, 202.60417, 68.177086));
	surfaceArea->drawLine(FRect(397.39584, 63.75, 397.39584, 68.177086));
	surfaceArea->drawLine(FRect(202.60417, 276.25, 202.60417, 271.82291));
	surfaceArea->drawLine(FRect(397.39584, 276.25, 397.39584, 271.82291));
	surfaceArea->drawLine(FRect(193.75, 72.604164, 198.17708, 72.604164));
	surfaceArea->drawLine(FRect(193.75, 267.39584, 198.17708, 267.39584));
	surfaceArea->drawLine(FRect(406.25, 72.604164, 401.82291, 72.604164));
	surfaceArea->drawLine(FRect(406.25, 267.39584, 401.82291, 267.39584));
	surfaceArea->drawLine(FRect(300.0, 63.75, 300.0, 54.895832));
	surfaceArea->drawLine(FRect(300.0, 276.25, 300.0, 285.10416));
	surfaceArea->drawLine(FRect(193.75, 170.0, 184.89583, 170.0));
	surfaceArea->drawLine(FRect(406.25, 170.0, 415.10416, 170.0));

	surfaceArea->_pixel = oldPixel;
	surfaceArea->setColorFromPixel();
}

void CStarField::renderLockLine(CSurfaceArea *surfaceArea, CCamera *camera) {
	FVector screenCoord, worldCoord, photoPos;
	_closeToMarker = false;

	if (_mode == MODE_STARFIELD) {
		if (lockDistance(surfaceArea, camera, screenCoord, worldCoord, photoPos) > -1.0) {
			surfaceArea->_pixel = 0xA0A0;
			surfaceArea->setColorFromPixel();
			surfaceArea->drawLine(FRect(screenCoord._x, screenCoord._y, photoPos._x, photoPos._y));
		}
	}
}

double CStarField::lockDistance(CSurfaceArea *surfaceArea, CCamera *camera,
		FVector &screenCoord, FVector &worldCoord, FVector &photoPos) {
	if (_crosshairs.isEmpty())
		// No crosshairs selection yet
		return -1.0;
	if (_crosshairs._entryIndex == _crosshairs._matchIndex)
		// Trying to re-lock on a previously locked star
		return -1.0;

	const CBaseStarEntry *dataP = _markers.getDataPtr(_crosshairs._entryIndex);
	worldCoord = dataP->_position;
	FVector tv = camera->getRelativePosNoCentering(2, worldCoord);

	if (camera->getFrontClip() >= tv._z)
		return -1.0;

	tv = camera->getRelativePos(2, tv);

	screenCoord = FVector(tv._x + surfaceArea->_centroid._x,
		tv._y + surfaceArea->_centroid._y, tv._z);
	FPoint pt = _crosshairs.getPosition();
	photoPos = FVector(pt._x, pt._y, 1.0);

	double incr = (screenCoord._x - pt._x) * (screenCoord._x - pt._x);
	if (incr > 3600.0)
		return -1.0;

	incr += (screenCoord._y - pt._y) * (screenCoord._y - pt._y);
	if (incr > 3600.0)
		return -1.0;

	_closeToMarker = true;
	return incr;
}

void CStarField::fn6(CVideoSurface *surface, CCamera *camera) {
	CSurfaceArea surfaceArea(surface);
	_crosshairs.fn1(this, &surfaceArea, camera);
}

void CStarField::incLockLevel() {
	_crosshairs.incMatches();
	setSolved();
}

void CStarField::decLockLevel(CVideoSurface *surface) {
	_crosshairs.decMatches(surface, this, &_markers);
	setSolved();
}

bool CStarField::mouseButtonDown(CVideoSurface *surface, CCamera *camera,
		int flags, const Common::Point &pt) {
	if (_mode == MODE_STARFIELD) {
		CSurfaceArea surfaceArea(surface);
		return selectStar(&surfaceArea, camera, pt);
	} else {
		int starNum = _crosshairs.indexOf(pt);
		if (starNum >= 0) {
			_crosshairs.selectStar(starNum, surface, this, &_markers);
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
