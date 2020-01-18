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

#ifndef ULTIMA_SHARED_ENGINE_MESSAGES_H
#define ULTIMA_SHARED_ENGINE_MESSAGES_H

#include "common/keyboard.h"
#include "ultima/shared/core/rect.h"
#include "ultima/shared/core/base_object.h"
#include "ultima/shared/core/tree_item.h"

namespace Ultima {
namespace Shared {

enum MessageFlag {
	MSGFLAG_SCAN = 1,
	MSGFLAG_BREAK_IF_HANDLED = 2,
	MSGFLAG_CLASS_DEF = 4
};

class CMessage;
namespace Gfx {
class VisualItem;
}

#define MESSAGEDEF(theClass) \
	static ClassDef type() { return ClassDef(#theClass, &CMessage::type); } \
	virtual ClassDef getType() const { return type(); }
#define MESSAGEDEFP(theClass, baseClass) \
	static ClassDef type() { return ClassDef(#theClass, &baseClass::type); } \
	virtual ClassDef getType() const { return type(); }

#define MESSAGE0(NAME) \
	class NAME: public CMessage { \
	public: NAME() : Message() {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE1(NAME, F1, N1, V1) \
	class NAME: public CMessage { \
	public: F1 _##N1; \
	NAME() : CMessage(), _##N1(V1) {} \
	NAME(F1 N1) : CMessage(), _##N1(N1) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE2(NAME, F1, N1, V1, F2, N2, V2) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2) {} \
	NAME(F1 N1, F2 N2) : CMessage(), _##N1(N1), _##N2(N2) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE3(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2), _##N3(V3) {} \
	NAME(F1 N1, F2 N2, F3 N3) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE4(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3, F4, N4, V4) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; F4 _##N4; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2), _##N3(V3), _##N4(V4) {} \
	NAME(F1 N1, F2 N2, F3 N3, F4 N4) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3), _##N4(N4) {} \
	MESSAGEDEF(NAME); \
	}

class CMessage : public BaseObject {
private:
	/**
	 * Find a map entry that supports the given class
	 */
	static const MSGMAP_ENTRY *findMapEntry(const TreeItem *treeItem, const ClassDef &classDef);
public:
	MESSAGEDEFP(CMessage, BaseObject);
	CMessage();

	/**
	 * Executes the message, passing it on to the designated target,
	 * and optionally it's children
	 */
	bool execute(TreeItem *target, const ClassDef *classDef = nullptr,
		int flags = MSGFLAG_SCAN | MSGFLAG_BREAK_IF_HANDLED);

	/**
	 * Executes the message, passing it on to the designated target,
	 * and optionally it's children
	 */
	bool execute(const Common::String &target, const ClassDef *classDef = nullptr,
		int flags = MSGFLAG_SCAN | MSGFLAG_BREAK_IF_HANDLED);

	/**
	 * Makes the passed item execute the message
	 */
	virtual bool perform(TreeItem *treeItem);

	/**
	 * Returns true if the passed item supports the specified message class
	 */
	static bool supports(const TreeItem *treeItem, const ClassDef &classDef);

	virtual bool isMouseMsg() const;
	virtual bool isButtonDownMsg() const;
	virtual bool isButtonUpMsg() const;
	virtual bool isMouseMoveMsg() const;
	virtual bool isDoubleClickMsg() const;
};


enum CMouseButton {
	MB_LEFT = 1, MB_MIDDLE = 2, MB_RIGHT = 4
};

class CMouseMsg : public CMessage {
public:
	int _buttons;
	Point _mousePos;
public:
	MESSAGEDEF(CMouseMsg);

	CMouseMsg() : _buttons(0) {}
	CMouseMsg(const Point &pt, int buttons) :
		_mousePos(pt), _buttons(buttons) {}
};

class CMouseMoveMsg : public CMouseMsg {
public:
	MESSAGEDEFP(CMouseMoveMsg, CMouseMsg);
	CMouseMoveMsg() : CMouseMsg() {}
	CMouseMoveMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}
};

class CMouseDragMsg : public CMouseMoveMsg {
public:
	MESSAGEDEFP(CMouseDragMsg, CMouseMoveMsg);
	CMouseDragMsg() : CMouseMoveMsg() {}
	CMouseDragMsg(const Point &pt, int buttons) : CMouseMoveMsg(pt, buttons) {}
};

class CMouseButtonMsg : public CMouseMsg {
public:
	int _field10;
public:
	MESSAGEDEFP(CMouseButtonMsg, CMouseMsg);
	CMouseButtonMsg() : CMouseMsg(), _field10(0) {}
	CMouseButtonMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}
};

class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseButtonDownMsg, CMouseButtonMsg);
	CMouseButtonDownMsg() : CMouseButtonMsg() {}
	CMouseButtonDownMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseButtonUpMsg, CMouseButtonMsg);
	CMouseButtonUpMsg() : CMouseButtonMsg() {}
	CMouseButtonUpMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

class CMouseWheelMsg : public CMouseMsg {
public:
	bool _wheelUp;
public:
	MESSAGEDEFP(CMouseWheelMsg, CMouseMsg);
	CMouseWheelMsg() : CMouseMsg(), _wheelUp(false) {}
	CMouseWheelMsg(const Point &pt, bool wheelUp) :
		CMouseMsg(pt, 0), _wheelUp(wheelUp) {}
};

class CMouseDoubleClickMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseDuobleClickMsg, CMouseButtonMsg);
	CMouseDoubleClickMsg() : CMouseButtonMsg() {}
	CMouseDoubleClickMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

MESSAGE1(CKeyCharMsg, int, key, 32);
MESSAGE1(CVirtualKeyCharMsg, Common::KeyState, keyState, Common::KeyState());
MESSAGE2(CShowMsg, Gfx::VisualItem *, view, (Gfx::VisualItem *)nullptr, bool, fadeIn, false);
MESSAGE2(CHideMsg, Gfx::VisualItem *, view, (Gfx::VisualItem *)nullptr, bool, fadeOut, false);
MESSAGE1(CFrameMsg, uint, ticks, 0);

} // End of namespace Shared
} // End of namespace Ultima

#endif
