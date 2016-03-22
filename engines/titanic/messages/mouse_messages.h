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

class CMouseMsg : public CMessage {
public:
	int _buttons;
	Point _mousePos;
public:
	CLASSDEF
	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseMsg *>(item) != nullptr;
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
		return dynamic_cast<const CMouseMoveMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseMoveMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseMoveMsg *dest = dynamic_cast<CMouseMoveMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	CLASSDEF
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
	CMouseButtonMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonMsg *>(item) != nullptr;
	}
};

MSGTARGET(CMouseButtonDownMsg);
class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	CLASSDEF
	CMouseButtonDownMsg() : CMouseButtonMsg() {}
	CMouseButtonDownMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonDownMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseButtonDownMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) {
		CMouseButtonDownMsg *dest = dynamic_cast<CMouseButtonDownMsg *>(treeItem);
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
	virtual bool handleMessage(const CMouseButtonUpMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseButtonUpMsg *dest = dynamic_cast<CMouseButtonUpMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseButtonDoubleClickMsg);
class CMouseButtonDoubleClickMsg : public CMouseButtonMsg {
public:
	CLASSDEF
	CMouseButtonDoubleClickMsg() : CMouseButtonMsg() {}
	CMouseButtonDoubleClickMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseButtonDoubleClickMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseButtonDoubleClickMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseButtonDoubleClickMsg *dest = dynamic_cast<CMouseButtonDoubleClickMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

class CMouseDragMsg : public CMouseMsg {
public:
	CLASSDEF
	CMouseDragMsg() : CMouseMsg() {}
	CMouseDragMsg(const Point &pt) : CMouseMsg(pt, 0) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragMsg *>(item) != nullptr;
	}
};

class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	CLASSDEF
	CMouseDragMoveMsg() : CMouseDragMsg() {}
	CMouseDragMoveMsg(const Point &pt) : CMouseDragMsg(pt) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return dynamic_cast<const CMouseDragMoveMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseDragMoveMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragMoveMsg *dest = dynamic_cast<CMouseDragMoveMsg *>(treeItem);
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
		return dynamic_cast<const CMouseDragStartMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseDragStartMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragStartMsg *dest = dynamic_cast<CMouseDragStartMsg *>(treeItem);
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
		return dynamic_cast<const CMouseDragEndMsg *>(item) != nullptr;
	}
	virtual bool handleMessage(const CMouseDragEndMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) {
		CMouseDragEndMsg *dest = dynamic_cast<CMouseDragEndMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
