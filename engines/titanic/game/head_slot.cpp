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

#include "titanic/game/head_slot.h"
#include "titanic/core/project_item.h"
#include "titanic/game/brain_slot.h"
#include "titanic/game_manager.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHeadSlot, CGameObject)
	ON_MESSAGE(AddHeadPieceMsg)
	ON_MESSAGE(SenseWorkingMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(LoadSuccessMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

bool CHeadSlot::_titaniaWoken;

CHeadSlot::CHeadSlot() : CGameObject(), _senseState("NotWorking"), _target("NULL"),
	_occupied(false), _timerDuration(0), _frameNum1(27), _frameNum2(56),
	_frameNum3(82), _frameNum4(112), _workingFlag(false) {
}

void CHeadSlot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_occupied, indent);
	file->writeQuotedLine(_senseState, indent);
	file->writeQuotedLine(_target, indent);
	file->writeNumberLine(_timerDuration, indent);
	file->writeNumberLine(_frameNum1, indent);
	file->writeNumberLine(_frameNum2, indent);
	file->writeNumberLine(_frameNum3, indent);
	file->writeNumberLine(_frameNum4, indent);
	file->writeNumberLine(_titaniaWoken, indent);
	file->writeNumberLine(_workingFlag, indent);

	CGameObject::save(file, indent);
}

void CHeadSlot::load(SimpleFile *file) {
	file->readNumber();
	_occupied = file->readNumber();
	_senseState = file->readString();
	_target = file->readString();
	_timerDuration = file->readNumber();
	_frameNum1 = file->readNumber();
	_frameNum2 = file->readNumber();
	_frameNum3 = file->readNumber();
	_frameNum4 = file->readNumber();
	_titaniaWoken = file->readNumber();
	_workingFlag = file->readNumber();

	CGameObject::load(file);
}

bool CHeadSlot::AddHeadPieceMsg(CAddHeadPieceMsg *msg) {
	setVisible(true);
	_occupied = true;
	_target = msg->_value;
	playMovie(_frameNum1, _frameNum4, 0);
	_cursorId = CURSOR_HAND;
	msg->execute("TitaniaControl");
	return true;
}

bool CHeadSlot::SenseWorkingMsg(CSenseWorkingMsg *msg) {
	if (_workingFlag)
		playMovie(_frameNum3, _frameNum4, 0);

	_senseState = msg->_value;
	_workingFlag = false;
	return true;
}

bool CHeadSlot::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	if (_titaniaWoken)
		_cursorId = CURSOR_ARROW;

	if (_titaniaWoken || _senseState == "Working") {
		playMovie(_frameNum2, _frameNum3, MOVIE_WAIT_FOR_FINISH);
		_workingFlag = true;
	} else if (_occupied) {
		playMovie(_frameNum2, _frameNum4, MOVIE_WAIT_FOR_FINISH);
		_workingFlag = false;
	} else {
		playMovie(0, _frameNum1, MOVIE_WAIT_FOR_FINISH);
	}

	addTimer(5000 + getRandomNumber(3000));
	return true;
}

bool CHeadSlot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (getName() == "YepItsASlot") {
		stopMovie();

		if (_occupied) {
			loadFrame(_frameNum2);
			playMovie(_frameNum2, _frameNum4, MOVIE_WAIT_FOR_FINISH);
		} else {
			loadFrame(_frameNum1);
			playMovie(_frameNum1, _frameNum2, MOVIE_WAIT_FOR_FINISH);
		}

		_workingFlag = false;
	}

	return true;
}

bool CHeadSlot::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	return true;
}

bool CHeadSlot::TimerMsg(CTimerMsg *msg) {
	if (compareViewNameTo("Titania.Node 15.S") && CBrainSlot::_numAdded == 5
			&& _occupied) {
		// WORKAROUND: Fix original bug where returning to Titania closeup when all the brain slots
		// were inserted in an incorrect order, would result in endless busy cursor
		if (getGameManager()->_gameState._mode != GSMODE_CUTSCENE) {
			if (_senseState == "Working" && !_workingFlag) {
				playMovie(_frameNum2, _frameNum3, 0);
				_workingFlag = true;
			} else if (_senseState == "Random") {
				playMovie(_frameNum2, _frameNum4, 0);
			}
		}
	}

	if (compareViewNameTo("Titania.Node 15.S")) {
		_timerDuration = 7000 + getRandomNumber(5000);
		addTimer(_timerDuration);
	}

	return true;
}

bool CHeadSlot::ActMsg(CActMsg *msg) {
	if (msg->_action == "Woken")
		_titaniaWoken = true;
	return true;
}

bool CHeadSlot::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_occupied && !_titaniaWoken && checkPoint(msg->_mousePos, false, true)) {
		CPassOnDragStartMsg passMsg;
		passMsg._mousePos = msg->_mousePos;
		passMsg.execute(_target);

		msg->_dragItem = getRoot()->findByName(_target);
		_cursorId = CURSOR_ARROW;
		_occupied = false;
		_workingFlag = false;
		_target = "NULL";
		stopMovie();
		loadFrame(0);
		playMovie(0, _frameNum1, 0);

		return true;
	}

	return false;
}

} // End of namespace Titanic
