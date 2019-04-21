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

#include "titanic/game/long_stick_dispenser.h"
#include "titanic/core/project_item.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLongStickDispenser, CGameObject)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CLongStickDispenser::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);

	CGameObject::save(file, indent);
}

void CLongStickDispenser::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();

	CGameObject::load(file);
}

bool CLongStickDispenser::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	if (!_fieldBC && _fieldC4 == 1 && !_fieldC0) {
		CStatusChangeMsg statusMsg;
		statusMsg.execute("ShatterGlass");
		_fieldC0 = 1;
		loadFrame(19);
	} else if (_fieldC0 != 1) {
		playSound(TRANSLATE("z#63.wav", "z#594.wav"));
		petDisplayMessage(1, GLASS_IS_UNBREAKABLE);
	}

	return true;
}

bool CLongStickDispenser::MovieEndMsg(CMovieEndMsg *msg) {
	CPuzzleSolvedMsg puzzleMsg;
	puzzleMsg.execute("LongStick");
	_fieldC0 = 1;
	return true;
}

bool CLongStickDispenser::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	return true;
}

bool CLongStickDispenser::EnterRoomMsg(CEnterRoomMsg *msg) {
	_fieldC0 = 0;
	_fieldC4 = 1;
	return true;
}

bool CLongStickDispenser::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_fieldC0) {
		playSound(TRANSLATE("z#62.wav", "z#593.wav"));

		switch (_fieldBC) {
		case 0:
			petDisplayMessage(1, FOR_STICK_BREAK_GLASS);
			break;
		case 1:
			petDisplayMessage(1, DISPENSOR_HAS_UNBREAKABLE_GLASS);
			break;
		default:
			break;
		}
	}

	return true;
}

bool CLongStickDispenser::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_fieldC0 == 1) {
		if (_fieldC4) {
			playMovie(19, 38, MOVIE_WAIT_FOR_FINISH);
		} else {
			playMovie(0, 18, MOVIE_WAIT_FOR_FINISH);
			_fieldBC = 1;
		}

		_fieldC4 = 1;
		_fieldC0 = 0;
	}

	return true;
}

bool CLongStickDispenser::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	loadFrame(38);
	_cursorId = CURSOR_HAND;
	return true;
}

bool CLongStickDispenser::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg)) {
		return false;
	} else if (_fieldC0 == 1 && _fieldC4 == 1) {
		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("LongStick");
		CPassOnDragStartMsg dragMsg(msg->_mousePos, 1);
		dragMsg.execute("LongStick");

		msg->_dragItem = getRoot()->findByName("LongStick");
		loadFrame(0);
		_fieldC4 = 0;
		_cursorId = CURSOR_ARROW;
	}

	return true;
}

} // End of namespace Titanic
