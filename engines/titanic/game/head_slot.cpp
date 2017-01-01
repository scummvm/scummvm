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

int CHeadSlot::_v1;

CHeadSlot::CHeadSlot() : CGameObject(), _string1("NotWorking"), _string2("NULL"),
	_fieldBC(0), _fieldD8(0), _fieldDC(27), _fieldE0(56),
	_fieldE4(82), _fieldE8(112), _fieldEC(false) {
}

void CHeadSlot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldD8, indent);
	file->writeNumberLine(_fieldDC, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_fieldEC, indent);

	CGameObject::save(file, indent);
}

void CHeadSlot::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_fieldD8 = file->readNumber();
	_fieldDC = file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_v1 = file->readNumber();
	_fieldEC = file->readNumber();

	CGameObject::load(file);
}

bool CHeadSlot::AddHeadPieceMsg(CAddHeadPieceMsg *msg) {
	setVisible(true);
	_fieldBC = 1;
	_string2 = msg->_value;
	playMovie(_fieldDC, _fieldE8, 0);
	_cursorId = CURSOR_HAND;
	msg->execute("TitaniaControl");
	return true;
}

bool CHeadSlot::SenseWorkingMsg(CSenseWorkingMsg *msg) {
	if (_fieldEC)
		playMovie(_fieldE4, _fieldE8, 0);

	_string1 = msg->_value;
	_fieldEC = false;
	return true;
}

bool CHeadSlot::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	if (_v1)
		_cursorId = CURSOR_ARROW;

	if (_v1 == 1 || _string1 == "Working") {
		playMovie(_fieldE0, _fieldE4, MOVIE_GAMESTATE);
		_fieldEC = true;
	} else if (_fieldBC) {
		playMovie(_fieldE0, _fieldE8, MOVIE_GAMESTATE);
		_fieldEC = false;
	} else {
		playMovie(0, _fieldDC, MOVIE_GAMESTATE);
	}

	addTimer(5000 + getRandomNumber(3000));
	return true;
}

bool CHeadSlot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (getName() == "YepItsASlot") {
		stopMovie();

		if (_fieldBC) {
			loadFrame(_fieldE0);
			playMovie(_fieldE0, _fieldE8, MOVIE_GAMESTATE);
			_fieldEC = false;
		} else {
			loadFrame(_fieldDC);
			playMovie(_fieldDC, _fieldE0, MOVIE_GAMESTATE);
		}

		_fieldEC = false;
	}

	return true;
}

bool CHeadSlot::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	return true;
}

bool CHeadSlot::TimerMsg(CTimerMsg *msg) {
	if (compareViewNameTo("Titania.Node 15.S") && CBrainSlot::_added == 5
			&& _fieldBC == 1) {
		if (_string1 == "Working" && !_fieldEC) {
			playMovie(_fieldE0, _fieldE4, 0);
			_fieldEC = true;
		} else if (_string1 == "Random") {
			playMovie(_fieldE0, _fieldE8, 0);
		}
	}

	if (compareViewNameTo("Titania.Node 15.S")) {
		_fieldD8 = 7000 + getRandomNumber(5000);
		addTimer(_fieldD8);
	}

	return true;
}

bool CHeadSlot::ActMsg(CActMsg *msg) {
	if (msg->_action == "Woken")
		_v1 = 1;
	return true;
}

bool CHeadSlot::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_fieldBC && !_v1 && checkPoint(msg->_mousePos, false, true)) {
		CPassOnDragStartMsg passMsg;
		passMsg._mousePos = msg->_mousePos;
		passMsg.execute(_string2);

		msg->_dragItem = getRoot()->findByName(_string2);
		_cursorId = CURSOR_ARROW;
		_fieldBC = 0;
		_fieldEC = false;
		_string2 = "NULL";
		stopMovie();
		loadFrame(0);
		playMovie(0, _fieldDC, 0);
	}

	return true;
}

} // End of namespace Titanic
