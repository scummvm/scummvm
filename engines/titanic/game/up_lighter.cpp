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
#include "titanic/translation.h"

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

CUpLighter::CUpLighter() : CDropTarget(), _hosePumping(false),
	_inRoom(0), _isSpring(false), _noseDispensed(false) {
}

void CUpLighter::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_hosePumping, indent);
	file->writeNumberLine(_inRoom, indent);
	file->writeNumberLine(_isSpring, indent);
	file->writeNumberLine(_noseDispensed, indent);

	CDropTarget::save(file, indent);
}

void CUpLighter::load(SimpleFile *file) {
	file->readNumber();
	_hosePumping = file->readNumber();
	_inRoom = file->readNumber();
	_isSpring = file->readNumber();
	_noseDispensed = file->readNumber();

	CDropTarget::load(file);
}

bool CUpLighter::MovieEndMsg(CMovieEndMsg *msg) {
	if (_hosePumping) {
		playSound(TRANSLATE("z#47.wav", "z#578.wav"));
		_noseDispensed = true;

		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("NoseHolder");
		CDropZoneLostObjectMsg lostMsg(nullptr);
		lostMsg.execute(this);
		_clipName.clear();
		_itemMatchName = "Nothing";
		_hosePumping = false;
	}

	return true;
}

bool CUpLighter::PumpingMsg(CPumpingMsg *msg) {
	_hosePumping = msg->_value;
	_clipName = (_hosePumping && !_noseDispensed) ? "WholeSequence" : "HoseToNose";
	return true;
}

bool CUpLighter::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CTrueTalkTriggerActionMsg triggerMsg(280245, 0, 0);
	triggerMsg.execute(getRoot(), CParrot::_type,
		MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
	return true;
}

bool CUpLighter::EnterRoomMsg(CEnterRoomMsg *msg) {
	_inRoom = true;
	addTimer(5000 + getRandomNumber(15000), 0);
	return true;
}

bool CUpLighter::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_isSpring = msg->_season == "Spring";
	if (_isSpring)
		addTimer(5000 + getRandomNumber(15000), 0);
	return true;
}

bool CUpLighter::TimerMsg(CTimerMsg *msg) {
	if (_isSpring && _inRoom & !_hosePumping) {
		CActMsg actMsg("Sneeze");
		actMsg.execute(findRoom()->findByName("NoseHolder"));
		addTimer(1000 + getRandomNumber(19000), 0);
	}

	return true;
}

bool CUpLighter::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	_inRoom = false;
	return true;
}

} // End of namespace Titanic
