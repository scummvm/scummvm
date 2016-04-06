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

#include "titanic/core/link_item.h"
#include "titanic/core/node_item.h"
#include "titanic/core/project_item.h"
#include "titanic/core/view_item.h"

namespace Titanic {

CLinkItem::CLinkItem() : CNamedItem() {
	_roomNumber = -1;
	_nodeNumber = -1;
	_viewNumber = -1;
	_field30 = 0;
	_cursorId = CURSOR_ARROW;
	_name = "Link";
}

CString CLinkItem::formName() {
	warning("TODO: CLinkItem::formName");
	return "";
}

void CLinkItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(2, indent);
	file->writeQuotedLine("L", indent);
	file->writeNumberLine(_cursorId, indent + 1);
	file->writeNumberLine(_field30, indent + 1);
	file->writeNumberLine(_roomNumber, indent + 1);
	file->writeNumberLine(_nodeNumber, indent + 1);
	file->writeNumberLine(_viewNumber, indent + 1);

	file->writeQuotedLine("Hotspot", indent + 1);
	file->writeNumberLine(_bounds.left, indent + 2);
	file->writeNumberLine(_bounds.top, indent + 2);
	file->writeNumberLine(_bounds.right, indent + 2);
	file->writeNumberLine(_bounds.bottom, indent + 2);

	CNamedItem::save(file, indent);
}

void CLinkItem::load(SimpleFile *file) {
	int val = file->readNumber();
	file->readBuffer();

	switch (val) {
	case 2:
		_cursorId = (CursorId)file->readNumber();
		// Deliberate fall-through

	case 1:
		_field30 = file->readNumber();
		// Deliberate fall-through

	case 0:
		_roomNumber = file->readNumber();
		_nodeNumber = file->readNumber();
		_viewNumber = file->readNumber();

		file->readBuffer();
		_bounds.left = file->readNumber();
		_bounds.top = file->readNumber();
		_bounds.right = file->readNumber();
		_bounds.bottom = file->readNumber();
		break;

	default:
		break;
	}

	CNamedItem::load(file);

	if (val < 2) {
		switch (_field30) {
		case 2:
			_cursorId = CURSOR_MOVE_LEFT;
			break;
		case 3:
			_cursorId = CURSOR_MOVE_RIGHT;
			break;
		case 5:
			_cursorId = CURSOR_MOVE_FORWARD;
			break;
		default:
			_cursorId = CURSOR_MOVE_FORWARD2;
			break;
		}
	}
}

void CLinkItem::setDestination(int roomNumber, int nodeNumber,
		int viewNumber, int v) {
	_roomNumber = roomNumber;
	_nodeNumber = nodeNumber;
	_viewNumber = viewNumber;
	_field30 = v;

	_name = formName();
}

CViewItem *CLinkItem::getDestView() const {
	return getRoot()->findView(_roomNumber, _nodeNumber, _viewNumber);
}

CNodeItem *CLinkItem::getDestNode() const {
	return getDestView()->findNode();
}

CRoomItem *CLinkItem::getDestRoom() const {
	return getDestNode()->findRoom();
}

CMovieClip *CLinkItem::getClip() const {
	return findRoom()->findClip(getName());
}

} // End of namespace Titanic
