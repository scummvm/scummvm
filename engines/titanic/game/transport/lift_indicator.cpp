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

#include "titanic/game/transport/lift_indicator.h"
#include "titanic/game/transport/lift.h"
#include "titanic/debugger.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLiftindicator, CLift)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CLiftindicator::CLiftindicator() : CLift(),
		_multiplier(0), _startY(0), _endY(0) {
}

void CLiftindicator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_multiplier, indent);
	file->writePoint(_indicatorPos, indent);
	file->writeNumberLine(_startY, indent);
	file->writeNumberLine(_endY, indent);

	CLift::save(file, indent);
}

void CLiftindicator::load(SimpleFile *file) {
	file->readNumber();
	_multiplier = file->readNumber();
	_indicatorPos = file->readPoint();
	_startY = file->readNumber();
	_endY = file->readNumber();

	CLift::load(file);
}

bool CLiftindicator::EnterViewMsg(CEnterViewMsg *msg) {
	double multiplier = _multiplier * 0.037037037;
	CPetControl *pet = getPetControl();
	int floorNum = pet->getRoomsFloorNum();
	debugC(kDebugScripts, "Lifts = %d,%d,%d,%d, %d",
		CLift::_elevator1Floor, CLift::_elevator2Floor,
		CLift::_elevator3Floor, CLift::_elevator4Floor,
		floorNum);

	if ((pet->petGetRoomsWellEntry() & 1) == (_liftNum & 1)) {
		petSetRemoteTarget();
		petSetArea(PET_REMOTE);
		petHighlightGlyph(0);

		petDisplayMessage(OUTSIDE_ELEVATOR_NUM, petGetRoomsWellEntry());

		debugC(kDebugScripts, "Claiming PET - %d, Multiplier = %f",
			_liftNum, multiplier);
	}

	switch (_liftNum) {
	case 0:
		loadFrame(pet->getRoomsFloorNum());
		break;

	case 1:
	case 3:
		switch (petGetRoomsWellEntry()) {
		case 1:
		case 2:
			setPosition(Point(_bounds.left, _indicatorPos.y +
				(int)(multiplier * CLift::_elevator1Floor)));
			_startFrame = CLift::_elevator1Floor;
			break;

		case 3:
		case 4:
			setPosition(Point(_bounds.left, _indicatorPos.y +
				(int)(multiplier * CLift::_elevator3Floor)));
			_startFrame = CLift::_elevator3Floor;
			break;

		default:
			break;
		}
		break;

	case 2:
	case 4:
		switch (petGetRoomsWellEntry()) {
		case 1:
		case 2:
			setPosition(Point(_bounds.left, _indicatorPos.y +
				(int)(multiplier * CLift::_elevator2Floor)));
			_startFrame = CLift::_elevator2Floor;
			break;

		case 3:
		case 4:
			setPosition(Point(_bounds.left, _indicatorPos.y +
				(int)(multiplier * CLift::_elevator4Floor)));
			_startFrame = CLift::_elevator4Floor;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

bool CLiftindicator::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

bool CLiftindicator::PETActivateMsg(CPETActivateMsg *msg) {
	double multiplier = _multiplier * 0.037037037;
	CPetControl *pet = getPetControl();

	if (msg->_name == "Lift") {
		if (petDoorOrBellbotPresent()) {
			petDisplayMessage(1, BOT_BLOCKING_ELEVATOR);
		} else {
			_endFrame = pet->getRoomsFloorNum();

			if (petGetRoomsWellEntry() == 4 && !CLift::_hasCorrectHead
					&& pet->getRoomsFloorNum() != CLift::_elevator4Floor) {
				petDisplayMessage(1, ELEVATOR_NON_FUNCTIONAL);
			} else {
				_startY = _indicatorPos.y + (int)(_startFrame * multiplier);
				_endY = _indicatorPos.y + (int)(_endFrame * multiplier);
				lockMouse();
				addTimer(100);

				if (petGetRoomsWellEntry() == 2) {
					CLift::_elevator4Floor = CLift::_elevator2Floor;
					CShipSettingMsg settingMsg;
					settingMsg._value = CLift::_elevator4Floor;
					settingMsg.execute("SGTStateroomTV");
				}

				switch (petGetRoomsWellEntry()) {
				case 1:
					CLift::_elevator1Floor = pet->getRoomsFloorNum();
					break;
				case 2:
					CLift::_elevator2Floor = pet->getRoomsFloorNum();
					break;
				case 3:
					CLift::_elevator3Floor = pet->getRoomsFloorNum();
					break;
				case 4:
					CLift::_elevator4Floor = pet->getRoomsFloorNum();
					break;
				default:
					break;
				}

				debugC(kDebugScripts, "Lifts = %d,%d,%d,%d %d",
					CLift::_elevator1Floor, CLift::_elevator2Floor,
					CLift::_elevator3Floor, CLift::_elevator4Floor,
					petGetRoomsWellEntry());
			}
		}
	}

	return true;
}

bool CLiftindicator::MovieEndMsg(CMovieEndMsg *msg) {
	playSound("357 gp button 1.wav");
	sleep(100);
	changeView("Lift.Node 1.N");

	unlockMouse();
	return true;
}

bool CLiftindicator::EnterRoomMsg(CEnterRoomMsg *msg) {
	return true;
}

bool CLiftindicator::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	return true;
}

bool CLiftindicator::TimerMsg(CTimerMsg *msg) {
	debugC(kDebugScripts, "Start %d, End %d", _startY, _endY);

	if (_startY > _endY) {
		setPosition(Point(_bounds.left, _bounds.top - 1));
		--_startY;
		addTimer(20);
	} else if (_startY < _endY) {
		setPosition(Point(_bounds.left, _bounds.top + 1));
		++_startY;
		addTimer(20);
	} else {
		CMovieEndMsg endMsg(0, 0);
		endMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
