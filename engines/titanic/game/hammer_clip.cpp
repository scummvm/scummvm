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

#include "titanic/game/hammer_clip.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHammerClip, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CHammerClip::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value, indent);
	CGameObject::save(file, indent);
}

void CHammerClip::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readNumber();
	CGameObject::load(file);
}

bool CHammerClip::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CHammerClip::StatusChangeMsg(CStatusChangeMsg *msg) {
	_value = msg->_newStatus == 1;
	if (_value) {
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("BigHammer");
		_cursorId = CURSOR_HAND;
	}

	return true;
}

bool CHammerClip::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return false;

	if (_value) {
		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("BigHammer");
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
		passMsg.execute("BigHammer");

		msg->_dragItem = getRoot()->findByName("BigHammer");

		CActMsg actMsg("HammerTaken");
		actMsg.execute("HammerDispensor");
		actMsg.execute("HammerDispensorButton");
		_cursorId = CURSOR_ARROW;
		_value = 0;
	}

	return true;
}

} // End of namespace Titanic
