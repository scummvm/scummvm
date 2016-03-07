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

#include "titanic/messages/messages.h"

namespace Titanic {

class CMouseMsg : public CMessage {
public:
	int _buttons;
	Common::Point _mousePos;
public:
	CLASSDEF
	CMouseMsg() : _buttons(0) {}
};

MSGTARGET(CMouseMoveMsg);
class CMouseMoveMsg : public CMouseMsg {
public:
	CLASSDEF

	virtual bool handleMessage(CMouseMoveMsg &msg) { return false; }
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
};

MSGTARGET(CMouseButtonDownMsg);
class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	CLASSDEF

	virtual bool handleMessage(CMouseButtonDownMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) {
		CMouseButtonDownMsg *dest = dynamic_cast<CMouseButtonDownMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseButtonUpMsg);
class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	CLASSDEF

	virtual bool handleMessage(CMouseButtonUpMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseButtonUpMsg *dest = dynamic_cast<CMouseButtonUpMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseButtonDoubleClickMsg);
class CMouseButtonDoubleClickMsg : public CMouseButtonMsg {
public:
	CLASSDEF

	virtual bool handleMessage(CMouseButtonDoubleClickMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseButtonDoubleClickMsg *dest = dynamic_cast<CMouseButtonDoubleClickMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

class CMouseDragMsg : public CMouseMsg {
public:
	CLASSDEF
};

class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	CLASSDEF

	virtual bool handleMessage(CMouseDragMoveMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragMoveMsg *dest = dynamic_cast<CMouseDragMoveMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDragStartMsg);
class CMouseDragStartMsg : public CMouseDragMsg {
public:
	int _field10;
	int _field14;
public:
	CLASSDEF
	CMouseDragStartMsg() : CMouseDragMsg(), _field10(0), _field14(0) {}

	virtual bool handleMessage(CMouseDragStartMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) { 
		CMouseDragStartMsg *dest = dynamic_cast<CMouseDragStartMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

MSGTARGET(CMouseDragEndMsg);
class CMouseDragEndMsg : public CMouseDragMsg {
public:
	int _field10;
public:
	CLASSDEF
	CMouseDragEndMsg() : CMouseDragMsg(), _field10(0) {}

	virtual bool handleMessage(CMouseDragEndMsg &msg) { return false; }
	virtual bool perform(CTreeItem *treeItem) {
		CMouseDragEndMsg *dest = dynamic_cast<CMouseDragEndMsg *>(treeItem);
		return dest != nullptr && dest->handleMessage(*this);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
