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

#ifndef TITANIC_MOUSE_MESSAGES_H
#define TITANIC_MOUSE_MESSAGES_H

#include "titanic/rect.h"
#include "titanic/messages/messages.h"

namespace Titanic {

enum MouseButton { MB_LEFT = 1, MB_MIDDLE = 2, MB_RIGHT = 4 };

MSGTARGET(CMouseMsg);
class CMouseMsg : public CMessage {
public:
	int _buttons;
	Point _mousePos;
public:
	CLASSDEF
	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseMsg *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) {
		CMouseMsgTarget *dest = dynamic_cast<CMouseMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}

	CMouseMsg() : _buttons(0) {}
	CMouseMsg(const Point &pt, int buttons) :
		_mousePos(pt), _buttons(buttons) {}
};

MSGTARGET(CMouseMoveMsg);
class CMouseMoveMsg : public CMouseMsg {
public:
	CLASSDEF
	CMouseMoveMsg() : CMouseMsg() {}
	CMouseMoveMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseMoveMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseMoveMsgTarget *dest = dynamic_cast<CMouseMoveMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseButtonMsg);
class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	CLASSDEF
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
	CMouseButtonMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonMsgTarget *>(item) != nullptr;
	}
};

MSGTARGET(CMouseButtonDownMsg);
class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	CLASSDEF
	CMouseButtonDownMsg() : CMouseButtonMsg() {}
	CMouseButtonDownMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonDownMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) {
		CMouseButtonDownMsgTarget *dest = dynamic_cast<CMouseButtonDownMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseButtonUpMsg);
class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	CLASSDEF
	CMouseButtonUpMsg() : CMouseButtonMsg() {}
	CMouseButtonUpMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonUpMsg *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseButtonUpMsgTarget *dest = dynamic_cast<CMouseButtonUpMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDoubleClickMsg);
class CMouseDoubleClickMsg : public CMouseButtonMsg {
public:
	CLASSDEF
	CMouseDoubleClickMsg() : CMouseButtonMsg() {}
	CMouseDoubleClickMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDoubleClickMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDoubleClickMsgTarget *dest = dynamic_cast<CMouseDoubleClickMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDragMsg);
class CMouseDragMsg : public CMouseMsg {
public:
	CLASSDEF
	CMouseDragMsg() : CMouseMsg() {}
	CMouseDragMsg(const Point &pt) : CMouseMsg(pt, 0) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragMsg *>(item) != nullptr;
	}
};

MSGTARGET(CMouseDragMoveMsg);
class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	CLASSDEF
	CMouseDragMoveMsg() : CMouseDragMsg() {}
	CMouseDragMoveMsg(const Point &pt) : CMouseDragMsg(pt) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragMoveMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragMoveMsgTarget *dest = dynamic_cast<CMouseDragMoveMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDragStartMsg);
class CMouseDragStartMsg : public CMouseDragMsg {
public:
	CTreeItem *_dragItem;
	int _field14;
public:
	CLASSDEF
	CMouseDragStartMsg() : CMouseDragMsg(), _dragItem(nullptr), _field14(0) {}
	CMouseDragStartMsg(const Point &pt) : CMouseDragMsg(pt),
		_dragItem(nullptr), _field14(0) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragStartMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragStartMsgTarget *dest = dynamic_cast<CMouseDragStartMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDragEndMsg);
class CMouseDragEndMsg : public CMouseDragMsg {
public:
	CTreeItem *_dragItem;
public:
	CLASSDEF
	CMouseDragEndMsg() : CMouseDragMsg(), _dragItem(nullptr) {}
	CMouseDragEndMsg(const Point &pt, CTreeItem *dragItem = nullptr) :
		CMouseDragMsg(pt), _dragItem(dragItem) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragEndMsgTarget *>(item) != nullptr;
	}
	virtual bool perform(CTreeItem *treeItem) {
		CMouseDragEndMsgTarget *dest = dynamic_cast<CMouseDragEndMsgTarget *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
