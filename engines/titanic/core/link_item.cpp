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

EMPTY_MESSAGE_MAP(CLinkItem, CNamedItem);

Movement CLinkItem::getMovementFromCursor(CursorId cursorId) {
	if (cursorId == CURSOR_MOVE_LEFT)
		return TURN_LEFT;
	else if (cursorId == CURSOR_MOVE_RIGHT)
		return TURN_RIGHT;
	else if (cursorId == CURSOR_MOVE_FORWARD || cursorId == CURSOR_MOVE_THROUGH ||
			cursorId == CURSOR_DOWN || cursorId == CURSOR_LOOK_UP ||
			cursorId == CURSOR_LOOK_DOWN || cursorId == CURSOR_MAGNIFIER)
		return MOVE_FORWARDS;
	else if (cursorId == CURSOR_BACKWARDS)
		return MOVE_BACKWARDS;
	else
		return MOVE_NONE;
}

CLinkItem::CLinkItem() : CNamedItem() {
	_roomNumber = -1;
	_nodeNumber = -1;
	_viewNumber = -1;
	_linkMode = 0;
	_cursorId = CURSOR_ARROW;
	_name = "Link";
}

CString CLinkItem::formName() {
	CViewItem *view = findView();
	CNodeItem *node = view->findNode();
	CRoomItem *room = node->findRoom();

	CViewItem *destView = getDestView();
	CNodeItem *destNode = destView->findNode();
	CRoomItem *destRoom = destNode->findRoom();

	switch (_linkMode) {
	case 1:
		return CString::format("_PANL,%d,%s,%s", node->_nodeNumber,
			view->getName().c_str(), destView->getName().c_str());

	case 2:
		return CString::format("_PANR,%d,%s,%s", node->_nodeNumber,
			view->getName().c_str(), destView->getName().c_str());

	case 3:
		return CString::format("_TRACK,%d,%s,%d,%s",
			node->_nodeNumber, view->getName().c_str(),
			destNode->_nodeNumber, destView->getName().c_str());

	case 4:
		return CString::format("_EXIT,%d,%d,%s,%d,%d,%s",
			room->_roomNumber, node->_nodeNumber, view->getName().c_str(),
			destRoom->_roomNumber, destNode->_nodeNumber, destView->getName().c_str());

	default:
		return getName().c_str();
	}
}

void CLinkItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(2, indent);
	file->writeQuotedLine("L", indent);
	file->writeNumberLine(_cursorId, indent + 1);
	file->writeNumberLine(_linkMode, indent + 1);
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
		// Intentional fall-through

	case 1:
		_linkMode = file->readNumber();
		// Intentional fall-through

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
		switch (_linkMode) {
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
			_cursorId = CURSOR_MOVE_THROUGH;
			break;
		}
	}
}

bool CLinkItem::connectsTo(CViewItem *destView) const {
	CNodeItem *destNode = destView->findNode();
	CRoomItem *destRoom = destNode->findRoom();

	return _viewNumber == destView->_viewNumber &&
		_nodeNumber == destNode->_nodeNumber &&
		_roomNumber == destRoom->_roomNumber;
}

void CLinkItem::setDestination(int roomNumber, int nodeNumber,
		int viewNumber, int linkMode) {
	_roomNumber = roomNumber;
	_nodeNumber = nodeNumber;
	_viewNumber = viewNumber;
	_linkMode = linkMode;

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

Movement CLinkItem::getMovement() const {
	if (_bounds.isEmpty())
		return MOVE_NONE;

	return getMovementFromCursor(_cursorId);
}

bool CLinkItem::findPoint(Quadrant quadrant, Point &pt) {
	if (_bounds.isEmpty())
		return false;

	pt = _bounds.getPoint(quadrant);
	return true;
}

} // End of namespace Titanic
