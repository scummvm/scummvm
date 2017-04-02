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

#include "titanic/support/screen_manager.h"
#include "titanic/star_control/star_view.h"
#include "titanic/star_control/star_control.h"
#include "titanic/star_control/star_field.h"
#include "titanic/core/game_object.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/titanic.h"

namespace Titanic {

CStarView::CStarView() : _sub12((const CStar20Data *)nullptr), _owner(nullptr),
		_starField(nullptr), _videoSurface(nullptr), _hasReference(0),
		_videoSurface2(nullptr), _homePhotoMask(nullptr),
		_field218(false), _showingPhoto(false) {
	CStar20Data data = { 0, 0, 100000.0, 0, 20.0, 1.0, 1.0, 1.0 };

	_sub12.proc3(&data);
}

void CStarView::load(SimpleFile *file, int param) {
	if (!param) {
		_sub12.load(file, param);

		_hasReference = file->readNumber();
		if (_hasReference)
			_sub13.load(file, 0);

		_field218 = file->readNumber();
		_showingPhoto = file->readNumber();
	}
}

void CStarView::save(SimpleFile *file, int indent) {
	_sub12.save(file, indent);

	file->writeNumberLine(_hasReference, indent);
	if (_hasReference)
		_sub13.save(file, indent);

	file->writeNumberLine(_field218, indent);
	file->writeNumberLine(_showingPhoto, indent);
}

void CStarView::setup(CScreenManager *screenManager, CStarField *starField, CStarControl *starControl) {
	_starField = starField;
	_owner = starControl;
}

void CStarView::reset() {
	if (_hasReference) {
		CStarControlSub12 sub12(&_sub13);
		fn18(&sub12);
	}
}

void CStarView::draw(CScreenManager *screenManager) {
	if (!screenManager || !_videoSurface || !_starField)
		return;

	if (_fader.isActive()) {
		CVideoSurface *surface = _showingPhoto ? _videoSurface2 : _videoSurface;
		surface = _fader.draw(screenManager, surface);
		screenManager->blitFrom(SURFACE_PRIMARY, surface);
	} else {
		Point destPos(20, 10);

		if (_showingPhoto) {
			screenManager->blitFrom(SURFACE_PRIMARY, _videoSurface2, &destPos);

			if (!_homePhotoMask && _owner) {
				_homePhotoMask = _owner->getHiddenObject("HomePhotoMask");
			}

			if (_homePhotoMask)
				_homePhotoMask->draw(screenManager, Point(20, 187));
		} else {
			fn1();

			// Render the display
			_videoSurface->clear();
			_videoSurface->lock();
			_starField->render(_videoSurface, &_sub12);
			_videoSurface->unlock();

			// Blit the resulting surface to the screen
			screenManager->blitFrom(SURFACE_PRIMARY, _videoSurface, &destPos);
		}
	}
}

bool CStarView::MouseButtonDownMsg(int flags, const Point &pt) {
	if (_starField) {
		return _starField->mouseButtonDown(
			_showingPhoto ? _videoSurface2 : _videoSurface,
			&_sub12, flags, pt);
	}

	return false;
}

bool CStarView::MouseMoveMsg(int unused, const Point &pt) {
	if (!_showingPhoto && (_fader._index < 0 || _fader._count >= 0)) {
		FPoint fpt = pt;
		FPoint centerPt(300.0, 170.0);

		if (fpt != centerPt) {
			double threshold = MIN(centerPt._x, centerPt._y) * 0.5;
			FPoint tempPt = fpt - centerPt;

			double distance = tempPt.normalize();
			if (distance >= threshold) {
				distance -= threshold;

				FPoint relPt(tempPt._x * -2.0 * distance / threshold, 
					tempPt._y * -2.0 * distance / threshold);
				_sub12.setViewportPosition(relPt);
				return true;
			}
		}
	}

	return false;
}

bool CStarView::KeyCharMsg(int key, CErrorCode *errorCode) {
	CStarControlSub6 sub6;
	int v = _starField ? _starField->get88() : -1;

	switch (key) {
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
	case Common::KEYCODE_c:
		if (v == -1) {
			sub6.setRotationMatrix(key == Common::KEYCODE_z ? Y_AXIS : X_AXIS, 1.0);
			_sub12.proc22(sub6);
			_sub12.proc15(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_SEMICOLON:
		if (v == -1) {
			_sub12.proc16();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_PERIOD:
		if (v == -1) {
			_sub12.proc17();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_SPACE:
		if (v == -1) {
			_sub12.proc19();
			errorCode->set();
			return true;
		}
		break;

	case Common::KEYCODE_x:
		if (v == -1) {
			sub6.setRotationMatrix(Y_AXIS, -1.0);
			_sub12.proc22(sub6);
			_sub12.proc15(errorCode);
			return true;
		}
		break;

	case Common::KEYCODE_QUOTE:
		if (v == -1) {
			sub6.setRotationMatrix(X_AXIS, -1.0);
			_sub12.proc22(sub6);
			_sub12.proc15(errorCode);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

bool CStarView::canSetStarDestination() const {
	return _sub12.is108();
}

void CStarView::starDestinationSet() {
	_sub12.reset108();
}

void CStarView::resetPosition() {
	_sub12.setPosition(FVector(0.0, 0.0, 0.0));
}

bool CStarView::fn1() {
	if (_videoSurface) {
		CErrorCode errorCode;
		_sub12.proc15(&errorCode);

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
	_sub12.setPosition(v1);
	_sub12.proc5(v2);
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
		FVector v1, v2;
		randomizeVectors1(v1, v2);
		v2 += star->_position;
		_sub12.setPosition(v2);
		_sub12.proc5(v1);
	}
}

void CStarView::fn19(int index) {
	const CBaseStarEntry *star = _starField->getStar(index);
	if (star) {
		FVector v1, v2;
		randomizeVectors1(v1, v2);
		v1 += star->_position;
		_sub12.setPosition(v1);
		_sub12.proc5(v2);
	}
}

void CStarView::fn8() {
	_sub12.proc18();
}

void CStarView::fn9() {
	_field218 = !_field218;
	if (_field218) {
		_sub12.proc12(MODE_PHOTO, 30.0);
		_sub12.proc12(MODE_STARFIELD, 28000.0);
	} else {
		_sub12.proc12(MODE_PHOTO, 0.0);
		_sub12.proc12(MODE_STARFIELD, 0.0);
	}
}

void CStarView::toggleMode() {
	_showingPhoto = !_showingPhoto;
	if (_starField)
		_starField->setMode(_showingPhoto ? MODE_PHOTO : MODE_STARFIELD);
}

void CStarView::fn11() {
	if (_starField)
		_starField->fn9();
}

void CStarView::toggleCrosshairs() {
	if (_starField)
		_starField->toggleCrosshairs();
}

void CStarView::fn13() {
	_field218 = true;
	_sub12.proc12(MODE_PHOTO, 30.0);
	_sub12.proc12(MODE_STARFIELD, 28000.0);
}

void CStarView::fn14() {
	_field218 = false;
	_sub12.proc12(MODE_PHOTO, 0.0);
	_sub12.proc12(MODE_STARFIELD, 0.0);
}

void CStarView::setHasReference() {
	FVector v1, v2;
	randomizeVectors2(v1, v2);

	_sub13.setPosition(v1);
	_sub13.fn11(v2);
	_field218 = false;
	_sub13.fn13(MODE_PHOTO, 0.0);
	_sub13.fn13(MODE_STARFIELD, 0.0);
	_hasReference = true;
	reset();
	_field218 = true;
}

void CStarView::fn16() {
	if (_starField && !_showingPhoto) {
		CSurfaceArea surfaceArea(_videoSurface);
		FVector v1, v2, v3;
		double val = _starField->fn5(&surfaceArea, &_sub12, v1, v2, v3);

		if (val > -1.0) {
			v1 += surfaceArea._centroid;
			v3 += surfaceArea._centroid;

			switch (_starField->get88()) {
			case -1:
				_sub12.fn2(v1, v2, v3);
				_starField->fn7();
				break;

			case 0:
				_sub12.fn3(&_sub13, v2);
				_starField->fn7();
				break;

			case 1:
				_sub12.fn1(&_sub13, v2);
				_starField->fn7();
				break;

			default:
				break;
			}
		}
	}
}

void CStarView::fn17() {
	if (_starField && !_showingPhoto) {
		_sub12.removeMatrixRow();
		_starField->fn8(_videoSurface2);
	}
}

void CStarView::fn18(CStarControlSub12 *sub12) {
	if (_starField) {
		if (!_videoSurface2) {
			CScreenManager *scrManager = CScreenManager::setCurrent();
			if (scrManager)
				resizeSurface(scrManager, 600, 340, &_videoSurface2);
		}

		if (_videoSurface2) {
			int oldVal = _starField->get54();
			bool oldCrosshairs = _starField->setCrosshairs(false);

			_videoSurface2->clear();
			_videoSurface2->lock();
			_starField->render(_videoSurface2, sub12);

			_starField->setCrosshairs(oldCrosshairs);
			_starField->set54(oldVal);
			_starField->fn6(_videoSurface2, sub12);
			_videoSurface2->unlock();
		}
	}
}

void CStarView::randomizeVectors1(FVector &v1, FVector &v2) {
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
	v1 = FVector(69481544.0, 69481544.0, 69481544.0);
	v2 = FVector(-0.577350259, -0.577350259, -0.577350259);
}

void CStarView::randomizeVectors2(FVector &v1, FVector &v2) {
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
	v1 = FVector(69481544.0, 69481544.0, 69481544.0);
	v2 = FVector(0.624659300, -0.468542814, -0.624714553);
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
