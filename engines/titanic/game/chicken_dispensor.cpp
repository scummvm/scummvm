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

#include "titanic/game/chicken_dispensor.h"
#include "titanic/carry/chicken.h"
#include "titanic/core/project_item.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChickenDispensor, CBackground)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CChickenDispensor::CChickenDispensor() : CBackground(),
	_disabled(false), _dispenseMode(DISPENSE_NONE), _dispensed(false) {
}

void CChickenDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_disabled, indent);
	file->writeNumberLine(_dispenseMode, indent);
	file->writeNumberLine(_dispensed, indent);
	CBackground::save(file, indent);
}

void CChickenDispensor::load(SimpleFile *file) {
	file->readNumber();
	_disabled = file->readNumber();
	_dispenseMode = (DispenseMode)file->readNumber();
	_dispensed = file->readNumber();

	CBackground::load(file);
}

bool CChickenDispensor::StatusChangeMsg(CStatusChangeMsg *msg) {
	msg->execute("SGTRestLeverAnimation");
	DispenseMode dispenseMode = _dispensed ? DISPENSE_NONE : _dispenseMode;
	CPetControl *pet = getPetControl();
	CGameObject *obj;

	for (obj = pet->getFirstObject(); obj; obj = pet->getNextObject(obj)) {
		if (obj->isEquals("Chicken")) {
			petDisplayMessage(1, ONE_CHICKEN_PER_CUSTOMER);
			return true;
		}
	}

	for (obj = getMailManFirstObject(); obj; obj = getNextMail(obj)) {
		if (obj->isEquals("Chicken")) {
			petDisplayMessage(1, ONE_CHICKEN_PER_CUSTOMER);
			return true;
		}
	}

	switch (dispenseMode) {
	case DISPENSE_NONE:
		petDisplayMessage(1, ONE_ALLOCATED_CHICKEN_PER_CUSTOMER);
		break;

	case DISPENSE_HOT:
	case DISPENSE_COLD:
		_dispensed = true;
		setVisible(true);

		if (_disabled) {
			playMovie(0, 12, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playSound(TRANSLATE("z#400.wav", "z#145.wav"));
		} else {
			playMovie(12, 16, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
		break;

	default:
		break;
	}

	return true;
}

bool CChickenDispensor::MovieEndMsg(CMovieEndMsg *msg) {
	int movieFrame = msg->_endFrame;

	if (movieFrame == 16) {
		// Dispensed a chicken
		_cursorId = CURSOR_HAND;
		playSound(TRANSLATE("b#50.wav", "b#30.wav"), 50);
		CActMsg actMsg("Dispense Chicken");
		actMsg.execute("Chicken");

		#ifdef FIX_DISPENSOR_TEMPATURE
		if (_dispenseMode != DISPENSE_HOT) {
			// WORKAROUND: If the fuse for the dispensor is removed in Titania's fusebox,
			// make the dispensed chicken already cold
			CChicken::_temperature = 0;
		}
		#endif
	} else if (_dispensed) {
		// Chicken dispensed whilst dispensor is "disabled", which basically
		// spits the chicken out at high speed directly into the SuccUBus
		_cursorId = CURSOR_ARROW;
		loadFrame(0);
		setVisible(false);
		if (_dispenseMode == DISPENSE_COLD)
			_dispensed = false;
	} else {
		// Doors closing as the view is being left
		loadFrame(0);
		setVisible(false);
		changeView("SgtLobby.Node 1.N");
	}

	return true;
}

bool CChickenDispensor::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_disabled = false;
	else if (msg->_action == "DisableObject")
		_disabled = true;
	else if (msg->_action == "IncreaseQuantity")
		_dispenseMode = DISPENSE_COLD;
	else if (msg->_action == "DecreaseQuantity")
		_dispenseMode = DISPENSE_HOT;

	return true;
}

bool CChickenDispensor::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CChickenDispensor::EnterViewMsg(CEnterViewMsg *msg) {
	playSound(TRANSLATE("b#51.wav", "b#31.wav"));
	_dispensed = false;
	_cursorId = CURSOR_ARROW;
	return true;
}

bool CChickenDispensor::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (getMovieFrame() == 16) {
		setVisible(false);
		loadFrame(0);
		_cursorId = CURSOR_ARROW;
		_dispensed = true;

		CVisibleMsg visibleMsg;
		visibleMsg.execute("Chicken");
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
		passMsg.execute("Chicken");

		msg->_dragItem = getRoot()->findByName("Chicken");
	}

	return true;
}

bool CChickenDispensor::TurnOff(CTurnOff *msg) {
	if (getMovieFrame() != 16)
		setVisible(false);
	playMovie(16, 12, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	_dispensed = false;

	return true;
}

} // End of namespace Titanic
