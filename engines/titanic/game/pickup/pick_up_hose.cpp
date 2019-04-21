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

#include "titanic/game/pickup/pick_up_hose.h"
#include "titanic/game/broken_pell_base.h"
#include "titanic/core/project_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPickUpHose, CPickUp)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

bool CPickUpHose::_v1;

void CPickUpHose::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_target, indent);
	file->writeNumberLine(_v1, indent);

	CPickUp::save(file, indent);
}

void CPickUpHose::load(SimpleFile *file) {
	file->readNumber();
	_target = file->readString();
	_v1 = file->readNumber();

	CPickUp::load(file);
}

bool CPickUpHose::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return true;
	if (_v1 || !_enabled)
		return false;

	CViewItem *view = getView();
	if (view) {
		_v1 = true;
		CRoomItem *room = locateRoom("Arboretum");
		CTreeItem *hose = room ? room->findByName("Hose") : nullptr;

		if (!hose) {
			room = locateRoom("FrozenArboretum");
			if (room)
				hose = room->findByName("Hose");
		}

		if (hose) {
			CVisibleMsg visibleMsg;
			visibleMsg.execute(hose);
			hose->moveUnder(view);

			CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
			passMsg.execute("Hose");

			msg->_dragItem = hose;
			_cursorId = CURSOR_IGNORE;

			CActMsg actMsg("PlayerGetsHose");
			actMsg.execute(_target);
		}
	}

	return true;
}

bool CPickUpHose::StatusChangeMsg(CStatusChangeMsg *msg) {
	_cursorId = msg->_newStatus == 1 ? CURSOR_HAND : CURSOR_IGNORE;
	return CPickUp::StatusChangeMsg(msg);
}

bool CPickUpHose::EnterViewMsg(CEnterViewMsg *msg) {
	if (msg->_oldView)
		_cursorId = CURSOR_IGNORE;
	return true;
}

bool CPickUpHose::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return _enabled;
}

} // End of namespace Titanic
