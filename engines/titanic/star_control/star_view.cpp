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
#include "titanic/star_control/motion_control.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fvector.h"
#include "titanic/star_control/star_control.h"
#include "titanic/star_control/star_field.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/core/game_object.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

CStarView::CStarView() : _camera((const CNavigationInfo *)nullptr), _owner(nullptr),
		_starField(nullptr), _videoSurface(nullptr), _lensValid(0),
		_photoSurface(nullptr), _homePhotoMask(nullptr),
		_stereoPair(false), _showingPhoto(false) {
	CNavigationInfo data = { 0, 0, 100000.0, 0, 20.0, 1.0, 1.0, 1.0 };

	_camera.setMotion(&data);
}

CStarView::~CStarView() {
	delete _videoSurface;
	delete _photoSurface;
}

void CStarView::load(SimpleFile *file, int param) {
	if (!param) {
		_camera.load(file, param);

		_lensValid = file->readNumber();
		if (_lensValid)
			_photoViewport.load(file, 0);

		_stereoPair = file->readNumber();
		_showingPhoto = file->readNumber();
	}
}

void CStarView::save(SimpleFile *file, int indent) {
	_camera.save(file, indent);

	file->writeNumberLine(_lensValid, indent);
	if (_lensValid)
		_photoViewport.save(file, indent);

	file->writeNumberLine(_stereoPair, indent);
	file->writeNumberLine(_showingPhoto, indent);
}

void CStarView::setup(CScreenManager *screenManager, CStarField *starField, CStarControl *starControl) {
	_starField = starField;
	_owner = starControl;
}

void CStarView::takeCurrentHomePhoto() {
	if (_lensValid) {
		CCamera camera(&_photoViewport);
		takeHomePhotoHelper(&camera);
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
	int lockLevel = _starField ? _starField->getMatchedIndex() : -1;

	switch (tolower(key)) {
	case Common::KEYCODE_TAB:
		if (_starField) {
			toggleHomePhoto();
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
		if (lockLevel == -1) {
			pose.setRotationMatrix(Y_AXIS, -1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_SEMICOLON:
		if (lockLevel == -1) {
			_camera.accelerate();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_PERIOD:
		if (lockLevel == -1) {
			_camera.deccelerate();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_SPACE:
		if (lockLevel == -1) {
			_camera.stop();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_x:
		if (lockLevel == -1) {
			pose.setRotationMatrix(Y_AXIS, 1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_QUOTE:
		if (lockLevel == -1) {
			pose.setRotationMatrix(X_AXIS, 1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_SLASH:
		if (lockLevel == -1) {
			pose.setRotationMatrix(X_AXIS, -1.0);
			_camera.changeOrientation(pose);
			_camera.updatePosition(errorCode);
			return true;
		}
		break;

	// New for ScummVM to show the boundaries sphere code the original implemented,
	// but wasn't actually hooked up to any player action
	case Common::KEYCODE_b:
		viewBoundaries();
		return true;

	// New for ScummVM to show the constellations sphere code the original implemented,
	// but wasn't actually hooked up to any player action
	case Common::KEYCODE_c:
		viewConstellations();
		return true;

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

void CStarView::resetView() {
	if (!_videoSurface) {
		CScreenManager *scrManager = CScreenManager::setCurrent();
		if (scrManager)
			resizeSurface(scrManager, 600, 340, &_videoSurface);

		if (_videoSurface) {
			stereoPairOn();
			viewRequiredStar(244);
			draw(scrManager);
		}
	}
}

void CStarView::triggerFade(bool fadeIn) {
	_fader.reset();
	_fader.setFadeIn(fadeIn);
}

void CStarView::viewFromEarth() {
	_camera.setPosition(FVector(0.0, 0.0, 0.0));
}

void CStarView::viewEarth() {
	FVector pos, dir;
	getRandomViewpoint(pos, dir);
	_camera.setPosition(pos);
	_camera.setOrientation(dir);
}

void CStarView::viewBoundaries() {
	_starField->setBoundaryState(!_starField->getBoundaryState());
}

void CStarView::viewConstellations() {
	_starField->setConstMapState(!_starField->getConstMapState());
}

void CStarView::viewRandomStar() {
	const CBaseStarEntry *star = _starField->getRandomStar();
	if (star) {
		FVector pos, orientation;
		getRandomViewpoint(pos, orientation);
		pos += star->_position;
		_camera.setPosition(pos);
		_camera.setOrientation(orientation);
	}
}

void CStarView::viewRequiredStar(int index) {
	const CBaseStarEntry *star = _starField->getStar(index);
	if (star) {
		FVector pos, orientation;
		getRandomViewpoint(pos, orientation);
		pos += star->_position;
		_camera.setPosition(pos);
		_camera.setOrientation(orientation);
	}
}

void CStarView::fullSpeed() {
	_camera.fullSpeed();
}

void CStarView::toggleSteroPair() {
	_stereoPair = !_stereoPair;
	if (_stereoPair) {
		_camera.setFields(MODE_PHOTO, 30.0);
		_camera.setFields(MODE_STARFIELD, 28000.0);
	} else {
		_camera.setFields(MODE_PHOTO, 0.0);
		_camera.setFields(MODE_STARFIELD, 0.0);
	}
}

void CStarView::toggleHomePhoto() {
	if (!_photoSurface)
		return;

	_showingPhoto = !_showingPhoto;
	if (_starField)
		_starField->setMode(_showingPhoto ? MODE_PHOTO : MODE_STARFIELD);
}

void CStarView::toggleSolarRendering() {
	if (_starField)
		_starField->ToggleSolarRendering();
}

void CStarView::TogglePosFrame() {
	if (_starField)
		_starField->toggleBox();
}

void CStarView::stereoPairOn() {
	_stereoPair = true;
	_camera.setFields(MODE_PHOTO, 30.0);
	_camera.setFields(MODE_STARFIELD, 28000.0);
}

void CStarView::stereoPairOff() {
	_stereoPair = false;
	_camera.setFields(MODE_PHOTO, 0.0);
	_camera.setFields(MODE_STARFIELD, 0.0);
}

void CStarView::takeHomePhoto() {
	FVector pos, orientation;
	getRandomPhotoViewpoint(pos, orientation);

	_photoViewport.setPosition(pos);
	_photoViewport.setOrientation(orientation);
	_stereoPair = false;
	_photoViewport.changeStarColorPixel(MODE_PHOTO, 0.0);
	_photoViewport.changeStarColorPixel(MODE_STARFIELD, 0.0);

	_lensValid = true;
	takeCurrentHomePhoto();
	_stereoPair = true;
}

void CStarView::lockStar() {
	if (_starField && !_showingPhoto) {
		CSurfaceArea surfaceArea(_videoSurface);
		FVector screenCoord, worldCoord, photoPos;
		double dist = _starField->lockDistance(&surfaceArea, &_camera,
			screenCoord, worldCoord, photoPos);
		bool lockSuccess = false;

		if (dist > -1.0) {
			screenCoord -= surfaceArea._centroid;
			photoPos -= surfaceArea._centroid;

			switch (_starField->getMatchedIndex()) {
			case -1:
				// First star match
				lockSuccess = _camera.lockMarker1(screenCoord, worldCoord, photoPos);
				assert(lockSuccess); // lockMarker1 should always succeed
				_starField->incLockLevel();
				break;

			case 0:
				// Second star match
				lockSuccess = _camera.lockMarker2(&_photoViewport, worldCoord);
				if (lockSuccess) // lockMarker2 may have issues
					_starField->incLockLevel();
				break;

			case 1:
				// Third star match
				lockSuccess = _camera.lockMarker3(&_photoViewport, worldCoord);
				assert(lockSuccess); // lockMarker3 should always succeed
				_starField->incLockLevel();
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
		_starField->decLockLevel(_photoSurface);
	}
}

void CStarView::takeHomePhotoHelper(CCamera *camera) {
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

void CStarView::getRandomViewpoint(FVector &pos, FVector &orientation) {
	const double MIN_RADIUS = 3.0F * STAR_SCALE;
	const double RADIUS = 4.0F * STAR_SCALE;

	pos._x = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;
	pos._y = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;
	pos._z = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;

	orientation._x = -pos._x;
	orientation._y = -pos._y;
	orientation._z = -pos._z;
	orientation.normalize();
}

void CStarView::getRandomPhotoViewpoint(FVector &pos, FVector &orientation) {
	const double MIN_RADIUS = 3.0F * STAR_SCALE;
	const double RADIUS = 4.0F * STAR_SCALE;

	pos._x = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;
	pos._y = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;
	pos._z = MIN_RADIUS + g_vm->getRandomFloat() * RADIUS;

	orientation._x = g_vm->getRandomFloat() * (STAR_SCALE * 8) - pos._x;
	orientation._y = g_vm->getRandomFloat() * STAR_SCALE - pos._y;
	orientation._z = -pos._z;
	orientation.normalize();
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
