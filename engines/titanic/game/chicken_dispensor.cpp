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
#include "titanic/core/project_item.h"
#include "titanic/pet_control/pet_control.h"

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
	_fieldE0(0), _fieldE4(0), _fieldE8(0) {
}

void CChickenDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	CBackground::save(file, indent);
}

void CChickenDispensor::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();

	CBackground::load(file);
}

bool CChickenDispensor::StatusChangeMsg(CStatusChangeMsg *msg) {
	msg->execute("SGTRestLeverAnimation");
	int v1 = _fieldE8 ? 0 : _fieldE4;
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

	if (v1 == 1 || v1 == 2)
		_fieldE8 = 1;

	switch (v1) {
	case 0:
		petDisplayMessage(1, ONE_ALLOCATED_CHICKEN_PER_CUSTOMER);
		break;
	case 1:
		setVisible(true);
		if (_fieldE0) {
			playMovie(0, 12, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			playSound("z#400.wav");
			_fieldE4 = 0;
		} else {
			playMovie(12, 16, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_fieldE8 = 1;
			_fieldE4 = 0;
		}
		break;

	case 2:
		setVisible(true);
		if (_fieldE0) {
			playMovie(0, 12, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			playSound("z#400.wav");
		} else {
			playMovie(12, 16, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_fieldE8 = 1;
		}
		break;

	default:
		break;
	}

	return true;
}

bool CChickenDispensor::MovieEndMsg(CMovieEndMsg *msg) {
	if (getMovieFrame() == 16) {
		playSound("b#50.wav", 50);
		CActMsg actMsg("Dispense Chicken");
		actMsg.execute("Chicken");
	} else if (_fieldE8) {
		_cursorId = CURSOR_ARROW;
		loadFrame(0);
		setVisible(false);
		if (_fieldE4 == 2)
			_fieldE8 = 0;
	} else {
		loadFrame(0);
		setVisible(false);
		changeView("SgtLobby.Node 1.N");
	}

	return true;
}

bool CChickenDispensor::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_fieldE0 = 0;
	else if (msg->_action == "DisableObject")
		_fieldE0 = 1;
	else if (msg->_action == "IncreaseQuantity")
		_fieldE4 = 2;
	else if (msg->_action == "DecreaseQuantity")
		_fieldE4 = 1;

	return true;
}

bool CChickenDispensor::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CChickenDispensor::EnterViewMsg(CEnterViewMsg *msg) {
	playSound("b#51.wav");
	_fieldE8 = 0;
	_cursorId = CURSOR_ARROW;
	return true;
}

bool CChickenDispensor::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (getMovieFrame() == 16) {
		setVisible(false);
		loadFrame(0);
		_cursorId = CURSOR_ARROW;
		_fieldE8 = 1;

		CVisibleMsg visibleMsg;
		visibleMsg.execute("Chicken");
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
		passMsg.execute("Chicken");

		msg->_dragItem = getRoot()->findByName("Chicken");
	}

	return true;
}

bool CChickenDispensor::TurnOff(CTurnOff *msg) {
	if (getMovieFrame() == 16)
		setVisible(false);
	playMovie(16, 12, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	_fieldE8 = 0;

	return true;
}

} // End of namespace Titanic
