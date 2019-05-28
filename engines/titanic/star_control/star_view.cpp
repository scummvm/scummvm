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

#include "titanic/star_control/star_view.h"
#include "titanic/star_control/camera_mover.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fvector.h"
#include "titanic/star_control/star_control.h"
#include "titanic/star_control/star_field.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/core/game_object.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CStarView::CStarView() : _camera((const CNavigationInfo *)nullptr), _owner(nullptr),
		_starField(nullptr), _videoSurface(nullptr), _hasReference(0),
		_photoSurface(nullptr), _homePhotoMask(nullptr),
		_field218(false), _showingPhoto(false) {
	CNavigationInfo data = { 0, 0, 100000.0, 0, 20.0, 1.0, 1.0, 1.0 };

	_camera.proc3(&data);
}

CStarView::~CStarView() {
	delete _videoSurface;
	delete _photoSurface;
}

void CStarView::load(SimpleFile *file, int param) {
	if (!param) {
		_camera.load(file, param);

		_hasReference = file->readNumber();
		if (_hasReference)
			_photoViewport.load(file, 0);

		_field218 = file->readNumber();
		_showingPhoto = file->readNumber();
	}
}

void CStarView::save(SimpleFile *file, int indent) {
	_camera.save(file, indent);

	file->writeNumberLine(_hasReference, indent);
	if (_hasReference)
		_photoViewport.save(file, indent);

	file->writeNumberLine(_field218, indent);
	file->writeNumberLine(_showingPhoto, indent);
}

void CStarView::setup(CScreenManager *screenManager, CStarField *starField, CStarControl *starControl) {
	_starField = starField;
	_owner = starControl;
}

void CStarView::reset() {
	if (_hasReference) {
		CStarCamera camera(&_photoViewport);
		fn18(&camera);
	}
}

void CStarView::draw(CScreenManager *screenManager) {
	if (!screenManager || !_videoSurface || !_starField)
		return;

	if (_fader.isActive()) {
		CVideoSurface *surface = _showingPhoto ? _photoSurface : _videoSurface;
		surface = _fader.draw(screenManager, surface);
		screenManager->blitFrom(SURFACE_PRIMARY, surface);
	} else {
		Point destPos(20, 10);

		if (_showingPhoto) {
			if (_photoSurface)
				screenManager->blitFrom(SURFACE_PRIMARY, _photoSurface, &destPos);

			if (!_homePhotoMask && _owner) {
				_homePhotoMask = _owner->getHiddenObject("HomePhotoMask");
			}

			if (_homePhotoMask)
				_homePhotoMask->draw(screenManager, Point(20, 187));
		} else {
			updateCamera();

			// Render the display
			_videoSurface->clear();
			_videoSurface->lock();
			_starField->render(_videoSurface, &_camera);
			_videoSurface->unlock();

			// Blit the resulting surface to the screen
			screenManager->blitFrom(SURFACE_PRIMARY, _videoSurface, &destPos);
		}
	}
}

bool CStarView::MouseButtonDownMsg(int flags, const Point &pt) {
	if (_starField) {
		return _starField->mouseButtonDown(
			_showingPhoto ? _photoSurface : _videoSurface,
			&_camera, flags, pt);
	}

	return false;
}

bool CStarView::MouseMoveMsg(int unused, const Point &pt) {
	if (!_showingPhoto && (_fader._index < 0 || _fader._count >= 0)) {
		FPoint fpt = pt;
		FPoint centerPt(300.0, 170.0);

		if (fpt != centerPt) {
			float threshold = MIN(centerPt._x, centerPt._y) * 0.5;
			FPoint tempPt = fpt - centerPt;

			float distance = tempPt.normalize();
			if (distance >= threshold) {
				distance -= threshold;

				FPoint angle(tempPt._x * -2.0 * distance / threshold, 
					tempPt._y * -2.0 * distance / threshold);
				_camera.setViewportAngle(angle);
				return true;
			}
		}
	}

	return false;
}

bool CStarView::KeyCharMsg(int key, CErrorCode *errorCode) {
	FPose pose;
	int matchedIndex = _starField ? _starField->getMatchedIndex() : -1;

	switch (tolower(key)) {
	case Common::KEYCODE_TAB:
		if (_starField) {
			toggleMode();
			return true;
		}
		break;

	case Common::KEYCODE_l: {
		CPetControl *pet = _owner->getPetControl();
		if (pet && pet->_remoteTarget) {
			CPETStarFieldLockMsg lockMsg(1);
			lockMsg.execute(pet->_remoteTarget);
		}
		return true;
	}

	case Common::KEYCODE_d: {
		CPetControl *pet = _owner->getPetControl();
		if (pet && pet->_remoteTarget) {
			CPETStarFieldLockMsg lockMsg(0);
			lockMsg.execute(pet->_remoteTarget);
		}
		return true;
	}

	case Common::KEYCODE_z:
		if (matchedIndex == -1) {
			pose.setRotationMatrix(Y_AXIS, -1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_SEMICOLON:
		if (matchedIndex == -1) {
			_camera.increaseForwardSpeed();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_PERIOD:
		if (matchedIndex == -1) {
			_camera.increaseBackwardSpeed();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_SPACE:
		if (matchedIndex == -1) {
			_camera.stop();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_x:
		if (matchedIndex == -1) {
			pose.setRotationMatrix(Y_AXIS, 1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_QUOTE:
		if (matchedIndex == -1) {
			pose.setRotationMatrix(X_AXIS, 1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_SLASH:
		if (matchedIndex == -1) {
			pose.setRotationMatrix(X_AXIS, -1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

bool CStarView::canSetStarDestination() const {
	return _camera.isMoved();
}

void CStarView::starDestinationSet() {
	_camera.clearIsMoved();
}

void CStarView::resetPosition() {
	_camera.setPosition(FVector(0.0, 0.0, 0.0));
}

bool CStarView::updateCamera() {
	if (_fader.isActive() || _showingPhoto)
		return false;

	if (_videoSurface) {
		CErrorCode errorCode;
		_camera.updatePosition(&errorCode);

		if (_fader._index < 0 || _fader._index >= _fader._count)
			_starField->fn1(&errorCode);
		else
			errorCode.set();

		return errorCode.get();
	}

	return false;
}

void CStarView::fn2() {
	if (!_videoSurface) {
		CScreenManager *scrManager = CScreenManager::setCurrent();
		if (scrManager)
			resizeSurface(scrManager, 600, 340, &_videoSurface);

		if (_videoSurface) {
			fn13();
			fn19(244);
			draw(scrManager);
		}
	}
}

void CStarView::fn3(bool fadeIn) {
	_fader.reset();
	_fader.setFadeIn(fadeIn);
}

void CStarView::fn4() {
	FVector v1, v2;
	randomizeVectors1(v1, v2);
	_camera.setPosition(v1);
	_camera.setOrientation(v2);
}

void CStarView::fn5() {
	_starField->set1(!_starField->get1());
}

void CStarView::fn6() {
	_starField->set2(!_starField->get2());
}

void CStarView::fn7() {
	const CBaseStarEntry *star = _starField->getRandomStar();
	if (star) {
		FVector pos, orientation;
		randomizeVectors1(pos, orientation);
		pos += star->_position;
		_camera.setPosition(pos);
		_camera.setOrientation(orientation);
	}
}

void CStarView::fn19(int index) {
	const CBaseStarEntry *star = _starField->getStar(index);
	if (star) {
		FVector pos, orientation;
		randomizeVectors1(pos, orientation);
		pos += star->_position;
		_camera.setPosition(pos);
		_camera.setOrientation(orientation);
	}
}

void CStarView::fullSpeed() {
	_camera.fullSpeed();
}

void CStarView::fn9() {
	_field218 = !_field218;
	if (_field218) {
		_camera.proc12(MODE_PHOTO, 30.0);
		_camera.proc12(MODE_STARFIELD, 28000.0);
	} else {
		_camera.proc12(MODE_PHOTO, 0.0);
		_camera.proc12(MODE_STARFIELD, 0.0);
	}
}

void CStarView::toggleMode() {
	if (!_photoSurface)
		return;

	_showingPhoto = !_showingPhoto;
	if (_starField)
		_starField->setMode(_showingPhoto ? MODE_PHOTO : MODE_STARFIELD);
}

void CStarView::fn11() {
	if (_starField)
		_starField->fn9();
}

void CStarView::toggleBox() {
	if (_starField)
		_starField->toggleBox();
}

void CStarView::fn13() {
	_field218 = true;
	_camera.proc12(MODE_PHOTO, 30.0);
	_camera.proc12(MODE_STARFIELD, 28000.0);
}

void CStarView::fn14() {
	_field218 = false;
	_camera.proc12(MODE_PHOTO, 0.0);
	_camera.proc12(MODE_STARFIELD, 0.0);
}

void CStarView::setHasReference() {
	FVector pos, orientation;
	getRandomPhotoViewpoint(pos, orientation);

	_photoViewport.setPosition(pos);
	_photoViewport.setOrientation(orientation);
	_field218 = false;
	_photoViewport.changeStarColorPixel(MODE_PHOTO, 0.0);
	_photoViewport.changeStarColorPixel(MODE_STARFIELD, 0.0);
	_hasReference = true;
	reset();
	_field218 = true;
}

void CStarView::lockStar() {
	if (_starField && !_showingPhoto) {
		CSurfaceArea surfaceArea(_videoSurface);
		FVector v1, v2, v3;
		double val = _starField->fn5(&surfaceArea, &_camera, v1, v2, v3);
		bool lockSuccess = false;

		if (val > -1.0) {
			v1 -= surfaceArea._centroid;
			v3 -= surfaceArea._centroid;

			switch (_starField->getMatchedIndex()) {
			case -1:
				// First star match
				lockSuccess = _camera.lockMarker1(v1, v2, v3);
				assert(lockSuccess); // lockMarker1 should always succeed
				_starField->incMatches();
				break;

			case 0:
				// Second star match
				lockSuccess = _camera.lockMarker2(&_photoViewport, v2);
				if (lockSuccess) // lockMarker2 may have issues
					_starField->incMatches();
				break;

			case 1:
				// Third star match
				lockSuccess = _camera.lockMarker3(&_photoViewport, v2);
				assert(lockSuccess); // lockMarker3 should always succeed
				_starField->incMatches();
				break;

			default:
				break;
			}
		}
	}
}

void CStarView::unlockStar() {
	if (_starField && !_showingPhoto && _camera.isNotInLockingProcess()) {
		_camera.removeLockedStar();
		_starField->fn8(_photoSurface);
	}
}

void CStarView::fn18(CStarCamera *camera) {
	if (_starField) {
		if (!_photoSurface) {
			CScreenManager *scrManager = CScreenManager::setCurrent();
			if (scrManager)
				resizeSurface(scrManager, 600, 340, &_photoSurface);
		}

		if (_photoSurface) {
			int oldVal = _starField->get54();
			bool oldCrosshairs = _starField->setBoxVisible(false);

			// Render the starfield for the photograph view
			_photoSurface->clear();
			_photoSurface->lock();
			_starField->render(_photoSurface, camera);

			// Render any previously set crosshairs
			_starField->setBoxVisible(oldCrosshairs);
			_starField->set54(oldVal);
			_starField->fn6(_photoSurface, camera);
			_photoSurface->unlock();
		}
	}
}

void CStarView::randomizeVectors1(FVector &pos, FVector &orientation) {
	/* ***DEBUG***
	v1._x = 3072.0 - g_vm->getRandomFloat() * -4096.0;
	v1._y = 3072.0 - g_vm->getRandomFloat() * -4096.0;
	v1._z = 3072.0 - g_vm->getRandomFloat() * -4096.0;

	v2._x = -v1._x;
	v2._y = -v1._y;
	v2._z = -v1._z;
	v2.normalize();
	*/
	// Values temporarily hardcoded to match hacked values in original EXE
	pos = FVector((float)69481544.0, (float)69481544.0, (float)69481544.0);
	orientation = FVector((float)-0.577350259, (float)-0.577350259, (float)-0.577350259);
}

void CStarView::getRandomPhotoViewpoint(FVector &pos, FVector &orientation) {
	/* ****DEBUG***
	v1._x = 3072.0 - g_vm->getRandomFloat() * -4096.0;
	v1._y = 3072.0 - g_vm->getRandomFloat() * -4096.0;
	v1._z = 3072.0 - g_vm->getRandomFloat() * -4096.0;

	v2._x = g_vm->getRandomFloat() * 8192.0 - v1._x;
	v2._y = g_vm->getRandomFloat() * 1024.0 - v1._y;
	v2._z = -v1._z;
	v2.normalize();
	*/
	// Values temporarily hardcoded to match hacked values in original EXE
	pos = FVector((float)69481544.0, (float)69481544.0, (float)69481544.0);
	orientation = FVector((float)0.624659300, (float)-0.468542814, (float)-0.624714553);
}

void CStarView::resizeSurface(CScreenManager *scrManager, int width, int height,
		CVideoSurface **surface) {
	if (!surface)
		// Surface pointer must be provided
		return;
	if (*surface) {
		// If there's an existing surface of the correct size, re-use it
		if ((*surface)->getWidth() == width && (*surface)->getHeight() == height)
			return;

		// Delete the old surface
		delete *surface;
		*surface = nullptr;
	}

	CVideoSurface *newSurface = scrManager->createSurface(width, height);
	if (newSurface)
		*surface = newSurface;
}

} // End of namespace Titanic
