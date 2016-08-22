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

#include "titanic/game/bottom_of_well_monitor.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBottomOfWellMonitor, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

int CBottomOfWellMonitor::_v1;
int CBottomOfWellMonitor::_v2;

void CBottomOfWellMonitor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CBottomOfWellMonitor::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CBottomOfWellMonitor::ActMsg(CActMsg *msg) {
	if (msg->_action == "TelevisionTaken") {
		_v1 = 0;
		_cursorId = CURSOR_ARROW;
		CVisibleMsg visibleMsg;
		visibleMsg.execute("CrushedTV2NE");
		visibleMsg.execute("CrushedTV4SW");
		_cursorId = CURSOR_ARROW;
	} else if (msg->_action == "LiftbotHeadTaken") {
		_v2 = 0;
		_cursorId = CURSOR_ARROW;
		CVisibleMsg visibleMsg;
		visibleMsg.execute("LiftbotHead2NE");
		visibleMsg.execute("LiftbotHead4SW");
		_cursorId = CURSOR_ARROW;
	} else if (msg->_action == "LiftbotHeadTaken") {
		_v2 = 1;
		CVisibleMsg visibleMsg;
		visibleMsg.execute("CrushedTV2NE");
		visibleMsg.execute("CrushedTV4SW");
		_cursorId = CURSOR_MOVE_DOWN1;
	}

	return true;
}

bool CBottomOfWellMonitor::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (isEquals("BOWTelevisionMonitor")) {
		if (_v1)
			changeView("BottomOfWell.Node 7.N", "");
	} else {
		if (_v2)
			changeView("BottomOfWell.Node 8.N", "");
	}

	return true;
}

bool CBottomOfWellMonitor::EnterViewMsg(CEnterViewMsg *msg) {
	if (_flag) {
		if (isEquals("BOWTelevisionMonitor")) {
			if (_v1) {
				changeView("BottomOfWell.Node 7.N", "");
				_flag = false;
			}
		} else {
			if (_v2) {
				changeView("BottomOfWell.Node 8.N", "");
				_flag = false;
			}
		}
	}

	return true;
}

bool CBottomOfWellMonitor::LeaveViewMsg(CLeaveViewMsg *msg) {
	_flag = true;
	return true;
}

} // End of namespace Titanic
