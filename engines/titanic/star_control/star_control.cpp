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

#include "titanic/star_control/star_control.h"
#include "titanic/core/dont_save_file_item.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/star_control/motion_control.h"
#include "titanic/star_control/error_code.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CStarControl, CGameObject)
	ON_MESSAGE(MouseMoveMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(KeyCharMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(MovementMsg)
END_MESSAGE_MAP()

CStarControl::CStarControl() : _enabled(false), _petControl(nullptr),
		_starRect(20, 10, 620, 350) {
	CCamera::init();
}

CStarControl::~CStarControl() {
	CCamera::deinit();
}

void CStarControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
	_starField.save(file, indent);
	_view.save(file, indent);
	CGameObject::save(file, indent);
}

void CStarControl::load(SimpleFile *file) {
	int val = file->readNumber();

	if (!val) {
		_starField.load(file);
		if (!_starField.initDocument())
			error("Couldn't initialise the StarField document");

		_view.load(file, 0);
		CScreenManager *screenManager = CScreenManager::setCurrent();
		if (!screenManager)
			error("There's no screen manager during loading");

		_view.setup(screenManager, &_starField, this);
		_view.takeCurrentHomePhoto();

		_enabled = true;
	}

	CGameObject::load(file);
}

void CStarControl::draw(CScreenManager *screenManager) {
	if (_visible)
		_view.draw(screenManager);
}

bool CStarControl::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_visible && _starRect.contains(msg->_mousePos)) {
		_view.MouseButtonDownMsg(0, Point(msg->_mousePos.x - 20,
			msg->_mousePos.y - 10));
		return true;
	} else {
		return false;
	}
}

bool CStarControl::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (_visible && _starRect.contains(msg->_mousePos)) {
		_view.MouseMoveMsg(0, Point(msg->_mousePos.x - 20,
			msg->_mousePos.y - 10));
		makeDirty();
		return true;
	} else {
		return false;
	}
}

bool CStarControl::KeyCharMsg(CKeyCharMsg *msg) {
	if (_visible) {
		CErrorCode errorCode;
		_view.KeyCharMsg(msg->_key, &errorCode);
		return errorCode.get();
	}

	return false;
}

bool CStarControl::FrameMsg(CFrameMsg *msg) {
	if (_visible) {
		Point pt = getMousePos();
		if (_starRect.contains(pt))
			_view.MouseMoveMsg(0, pt);

		newFrame();
		makeDirty();
		return true;
	} else {
		return false;
	}
}

void CStarControl::newFrame() {
	if (!_petControl)
		_petControl = getPetControl();

	if (_petControl) {
		int matchIndex = _starField.getMatchedIndex();
		bool isClose = false;

		if (_starField.getMode() == MODE_STARFIELD) {
			isClose = _starField.isCloseToMarker();
			if ((matchIndex + 2) != _starField.getMarkerCount())
				isClose = false;
		}

		_petControl->starsSetButtons(matchIndex, isClose);
	}
}

bool CStarControl::isStarFieldMode() {
	if (!_petControl)
		_petControl = getPetControl();

	if (_petControl) {

		if (_starField.getMode() == MODE_STARFIELD)
			return true;
	}
	return false;
}

void CStarControl::doAction(StarControlAction action) {
	if (!_enabled)
		return;

	switch (action) {
	case STAR_SHOW: {
		CGameManager *gameManager = getGameManager();
		CViewItem *view = gameManager ? gameManager->getView() : nullptr;
		if (view) {
			detach();
			addUnder(view);
			_view.resetView();
			_view.triggerFade(true);
			_visible = true;
		}
		break;
	}

	case STAR_HIDE: {
		CProjectItem *root = getRoot();
		CDontSaveFileItem *fileItem = root ? root->getDontSaveFileItem() : nullptr;
		if (fileItem) {
			detach();
			addUnder(fileItem);
			_visible = false;
		}
		break;
	}

	case STAR_VIEW_EARTH:
		_view.viewEarth();
		break;

	case STAR_VIEW_FROM_EARTH:
		_view.viewFromEarth();
		break;

	case STAR_VIEW_BOUNDARIES:
		_view.viewBoundaries();
		break;

	case STAR_VIEW_CONSTELLATIONS:
		_view.viewConstellations();
		break;

	case STAR_VIEW_RANDOM_STAR:
		_view.viewRandomStar();
		break;

	case STAR_FULL_SPEED:
		_view.fullSpeed();
		break;

	case STAR_TOGGLE_STEREO_PAIR:
		_view.toggleSteroPair();
		break;

	case STAR_TOGGLE_HOME_PHOTO:
		_view.toggleHomePhoto();
		break;

	case STAR_TOGGLE_SOLAR_RENDERING:
		_view.toggleSolarRendering();
		break;

	case STAR_TOGGLE_POS_FRAME:
		_view.TogglePosFrame();
		break;

	case STAR_STEREO_PAIR_ON:
		_view.stereoPairOn();
		break;

	case STAR_STEREO_PAIR_OFF:
		_view.stereoPairOff();
		break;

	case STAR_SET_REFERENCE: {
		_view.takeCurrentHomePhoto();
		CPetControl *pet = getPetControl();
		if (pet)
			pet->starsSetReference();
		break;
	}

	case STAR_FADE_IN:
		_view.triggerFade(true);
		break;

	case STAR_FADE_OUT:
		_view.triggerFade(false);
		break;

	case LOCK_STAR:
		_view.lockStar();
		break;

	case UNLOCK_STAR:
		_view.unlockStar();
		break;

	case STAR_CLEAR_MODIFIED:
		_view.starDestinationSet();
		break;

	default:
		break;
	}
}

bool CStarControl::isSolved() const {
	return _starField.isSolved();
}

bool CStarControl::isSkipped() const {
	return _starField.isSkipped();
}

void CStarControl::forceSolved() {
	_starField.skipPuzzle();
}

bool CStarControl::canSetStarDestination() const {
	return _view.canSetStarDestination();
}

void CStarControl::starDestinationSet() {
	_view.starDestinationSet();
}

bool CStarControl::MovementMsg(CMovementMsg *msg) {
	// The star control view has an unused turn right link hidden
	// under the star view. For cleanliness, explicitly consume any
	// movements in the star view so the link is never used
	return true;
}

} // End of namespace Titanic
