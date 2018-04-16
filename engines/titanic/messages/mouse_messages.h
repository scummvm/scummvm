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

#include "titanic/support/rect.h"
#include "titanic/messages/messages.h"

namespace Titanic {

enum MouseButton { MB_LEFT = 1, MB_MIDDLE = 2, MB_RIGHT = 4 };

class CMouseMsg : public CMessage {
public:
	int _buttons;
	Point _mousePos;
public:
	CLASSDEF;
	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}

	CMouseMsg() : _buttons(0) {}
	CMouseMsg(const Point &pt, int buttons) :
		_mousePos(pt), _buttons(buttons) {}
};

class CMouseMoveMsg : public CMouseMsg {
public:
	CLASSDEF;
	CMouseMoveMsg() : CMouseMsg() {}
	CMouseMoveMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	CLASSDEF;
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
	CMouseButtonMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	CLASSDEF;
	CMouseButtonDownMsg() : CMouseButtonMsg() {}
	CMouseButtonDownMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}

	/**
	 * Generate a dummy mouse down message at the current mouse position
	 */
	static void generate();
};

class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	CLASSDEF;
	CMouseButtonUpMsg() : CMouseButtonMsg() {}
	CMouseButtonUpMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}

	/**
	 * Generate a dummy mouse up message at the current mouse position
	 */
	static void generate();
};

class CMouseWheelMsg : public CMouseMsg {
public:
	bool _wheelUp;
public:
	CLASSDEF;
	CMouseWheelMsg() : CMouseMsg(), _wheelUp(false) {}
	CMouseWheelMsg(const Point &pt, bool wheelUp) :
		CMouseMsg(pt, 0), _wheelUp(wheelUp) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseDoubleClickMsg : public CMouseButtonMsg {
public:
	CLASSDEF;
	CMouseDoubleClickMsg() : CMouseButtonMsg() {}
	CMouseDoubleClickMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseDragMsg : public CMouseMsg {
public:
	CLASSDEF;
	CMouseDragMsg() : CMouseMsg() {}
	CMouseDragMsg(const Point &pt) : CMouseMsg(pt, 0) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	CLASSDEF;
	CMouseDragMoveMsg() : CMouseDragMsg() {}
	CMouseDragMoveMsg(const Point &pt) : CMouseDragMsg(pt) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseDragStartMsg : public CMouseDragMsg {
public:
	CTreeItem *_dragItem;
	bool _handled;
public:
	CLASSDEF;
	CMouseDragStartMsg() : CMouseDragMsg(), _dragItem(nullptr), _handled(false) {}
	CMouseDragStartMsg(const Point &pt) : CMouseDragMsg(pt),
		_dragItem(nullptr), _handled(false) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CPassOnDragStartMsg : public CMessage {
public:
	Point _mousePos;
	int _value3;
	int _value4;
public:
	CLASSDEF;
	CPassOnDragStartMsg() : CMessage(), _value3(0), _value4(0) {}
	CPassOnDragStartMsg(const Point &pt, int v3 = 0, int v4 = 0) :
		CMessage(), _mousePos(pt), _value3(v3), _value4(v4) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CMouseDragEndMsg : public CMouseDragMsg {
public:
	CGameObject *_dropTarget;
public:
	CLASSDEF;
	CMouseDragEndMsg() : CMouseDragMsg(), _dropTarget(nullptr) {}
	CMouseDragEndMsg(const Point &pt, CGameObject *dropTarget = nullptr) :
		CMouseDragMsg(pt), _dropTarget(dropTarget) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CChildDragMoveMsg : public CMessage {
public:
	Point _mousePos;
public:
	CLASSDEF;
	CChildDragMoveMsg() : CMessage() {}
	CChildDragMoveMsg(const Point &pt) : CMessage(), _mousePos(pt) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CChildDragStartMsg : public CMessage {
public:
	Point _mousePos;
public:
	CLASSDEF;
	CChildDragStartMsg() : CMessage() {}
	CChildDragStartMsg(const Point &pt) : CMessage(), _mousePos(pt) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
