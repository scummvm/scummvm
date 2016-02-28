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
	CMouseMsg() : _buttons(0) {}
	virtual const char *getClassName() const { return "CMouseMsg"; }
};

class CMouseMoveMsg : public CMouseMsg {
public:
	virtual const char *getClassName() const { return "CMouseMoveMsg"; }
};

class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
	virtual const char *getClassName() const { return "CMouseButtonMsg"; }
};

class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	virtual const char *getClassName() const { return "CMouseButtonDownMsg"; }
};

class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	virtual const char *getClassName() const { return "CMouseButtonUpMsg"; }
};

class CMouseButtonDoubleClickMsg : public CMouseButtonMsg {
public:
	virtual const char *getClassName() const { return "CMouseButtonDoubleClickMsg"; }
};

class CMouseDragMsg : public CMouseMsg {
public:
	virtual const char *getClassName() const { return "CMouseDragMsg"; }
};

class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	virtual const char *getClassName() const { return "CMouseDragMoveMsg"; }
};

class CMouseDragStartMsg : public CMouseDragMsg {
public:
	int _field10;
	int _field14;
public:
	CMouseDragStartMsg() : CMouseDragMsg(), _field10(0), _field14(0) {}
	virtual const char *getClassName() const { return "CMouseDragStartMsg"; }
};

class CMouseDragEndMsg : public CMouseDragMsg {
public:
	int _field10;
public:
	CMouseDragEndMsg() : CMouseDragMsg(), _field10(0) {}
	virtual const char *getClassName() const { return "CMouseDragEndMsg"; }
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
