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

#include "titanic/game/up_lighter.h"
#include "titanic/core/project_item.h"
#include "titanic/npcs/parrot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CUpLighter, CDropTarget)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(PumpingMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(LeaveRoomMsg)
END_MESSAGE_MAP()

CUpLighter::CUpLighter() : CDropTarget(), _field118(0),
	_field11C(0), _field120(0), _field124(0) {
}

void CUpLighter::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);

	CDropTarget::save(file, indent);
}

void CUpLighter::load(SimpleFile *file) {
	file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();

	CDropTarget::load(file);
}

bool CUpLighter::MovieEndMsg(CMovieEndMsg *msg) {
	if (_field118) {
		playSound("z#47.wav");
		_field124 = true;

		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("NoseHolder");
		CDropZoneLostObjectMsg lostMsg(nullptr);
		lostMsg.execute(this);
		_clipName.clear();
		_itemMatchName = "Nothing";
		_field118 = 0;
	}

	return true;
}

bool CUpLighter::PumpingMsg(CPumpingMsg *msg) {
	_field118 = msg->_value;
	_clipName = (_field118 && !_field124) ? "WholeSequence" : "HoseToNose";
	return true;
}

bool CUpLighter::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CTrueTalkTriggerActionMsg triggerMsg(280245, 0, 0);
	triggerMsg.execute(getRoot(), CParrot::_type,
		MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
	return true;
}

bool CUpLighter::EnterRoomMsg(CEnterRoomMsg *msg) {
	_field11C = true;
	addTimer(5000 + getRandomNumber(15000), 0);
	return true;
}

bool CUpLighter::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_field120 = msg->_season == "Spring";
	if (_field120)
		addTimer(5000 + getRandomNumber(15000), 0);
	return true;
}

bool CUpLighter::TimerMsg(CTimerMsg *msg) {
	if (_field120 && _field11C & !_field118) {
		CActMsg actMsg("Sneeze");
		actMsg.execute(findRoom()->findByName("NoseHolder"));
		addTimer(1000 + getRandomNumber(19000), 0);
	}

	return true;
}

bool CUpLighter::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	_field11C = false;
	return true;
}

} // End of namespace Titanic
