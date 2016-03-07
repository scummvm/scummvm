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

class CMouseMoveMsg : public CMouseMsg {
public:
	CLASSDEF
};

class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	CLASSDEF
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
};

class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	CLASSDEF
};

class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	CLASSDEF
};

class CMouseButtonDoubleClickMsg : public CMouseButtonMsg {
public:
	CLASSDEF
};

class CMouseDragMsg : public CMouseMsg {
public:
	CLASSDEF
};

class CMouseDragMoveMsg : public CMouseDragMsg {
public:
	CLASSDEF
};

class CMouseDragStartMsg : public CMouseDragMsg {
public:
	int _field10;
	int _field14;
public:
	CLASSDEF
	CMouseDragStartMsg() : CMouseDragMsg(), _field10(0), _field14(0) {}
};

class CMouseDragEndMsg : public CMouseDragMsg {
public:
	int _field10;
public:
	CLASSDEF
	CMouseDragEndMsg() : CMouseDragMsg(), _field10(0) {}
};

} // End of namespace Titanic

#endif /* TITANIC_MOUSE_MESSAGES_H */
