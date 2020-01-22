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
#include "common/events.h"
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
class TextInput;
class Popup;
}

#define MESSAGEDEF(theClass) \
	static ClassDef type() { return ClassDef(#theClass, &CMessage::type); } \
	virtual ClassDef getType() const { return type(); }
#define MESSAGEDEFP(theClass, baseClass) \
	static ClassDef type() { return ClassDef(#theClass, &baseClass::type); } \
	virtual ClassDef getType() const { return type(); }

#define MESSAGE0(NAME) \
	class NAME: public CMessage { \
	public: NAME() : CMessage() {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE1(NAME, F1, N1, V1) \
	class NAME: public CMessage { \
	public: F1 _##N1; \
	NAME(F1 N1 = V1) : CMessage(), _##N1(N1) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE2(NAME, F1, N1, V1, F2, N2, V2) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; \
	NAME(F1 N1 = V1, F2 N2 = V2) : CMessage(), _##N1(N1), _##N2(N2) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE3(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; \
	NAME(F1 N1 = V1, F2 N2 = V2, F3 N3 = V3) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3) {} \
	MESSAGEDEF(NAME); \
	}
#define MESSAGE4(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3, F4, N4, V4) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; F4 _##N4; \
	NAME(F1 N1 = V1, F2 N2 = V2, F3 N3 = V3, F4 N4 = V4) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3), _##N4(N4) {} \
	MESSAGEDEF(NAME); \
	}

/**
 * Base class for all messages
 */
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

/**
 * Base class for the different mouse notifications
 */
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

/**
 * Notifies a mouse movement
 *
 */
class CMouseMoveMsg : public CMouseMsg {
public:
	MESSAGEDEFP(CMouseMoveMsg, CMouseMsg);
	CMouseMoveMsg() : CMouseMsg() {}
	CMouseMoveMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}
};

/**
 * Notifies of a mouse drag operation
 */
class CMouseDragMsg : public CMouseMoveMsg {
public:
	MESSAGEDEFP(CMouseDragMsg, CMouseMoveMsg);
	CMouseDragMsg() : CMouseMoveMsg() {}
	CMouseDragMsg(const Point &pt, int buttons) : CMouseMoveMsg(pt, buttons) {}
};

/**
 * Base class for mouse button notifications
 */
class CMouseButtonMsg : public CMouseMsg {
public:
	MESSAGEDEFP(CMouseButtonMsg, CMouseMsg);
	CMouseButtonMsg() : CMouseMsg() {}
	CMouseButtonMsg(const Point &pt, int buttons) : CMouseMsg(pt, buttons) {}
};

/**
 * Notifies a mouse button down
 */
class CMouseButtonDownMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseButtonDownMsg, CMouseButtonMsg);
	CMouseButtonDownMsg() : CMouseButtonMsg() {}
	CMouseButtonDownMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

/**
 * Notifies a mouse button release
 */
class CMouseButtonUpMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseButtonUpMsg, CMouseButtonMsg);
	CMouseButtonUpMsg() : CMouseButtonMsg() {}
	CMouseButtonUpMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

/**
 * Notifies a mouse wheel action
 */
class CMouseWheelMsg : public CMouseMsg {
public:
	bool _wheelUp;
public:
	MESSAGEDEFP(CMouseWheelMsg, CMouseMsg);
	CMouseWheelMsg() : CMouseMsg(), _wheelUp(false) {}
	CMouseWheelMsg(const Point &pt, bool wheelUp) :
		CMouseMsg(pt, 0), _wheelUp(wheelUp) {}
};

/**
 * Notifies a mouse double-click
 */
class CMouseDoubleClickMsg : public CMouseButtonMsg {
public:
	MESSAGEDEFP(CMouseDuobleClickMsg, CMouseButtonMsg);
	CMouseDoubleClickMsg() : CMouseButtonMsg() {}
	CMouseDoubleClickMsg(const Point &pt, int buttons) : CMouseButtonMsg(pt, buttons) {}
};

/**
 * Used to notify that a rendering frame has finished, occurring at GAME_FRAME_RATE times every second
 */
MESSAGE1(CFrameMsg, uint, ticks, 0);

/**
 * Notifies a game view is being hidden
 */
MESSAGE2(CHideMsg, Gfx::VisualItem *, view, (Gfx::VisualItem *)nullptr, bool, fadeOut, false);

/**
 * Show a prompt in the info area, and get a keypress for a command
 */
MESSAGE1(CInfoGetCommandKeypress, TreeItem *, responder, (TreeItem *)nullptr);

/**
 * Get a keypress in the info area
 */
MESSAGE1(CInfoGetKeypress, TreeItem *, responder, (TreeItem *)nullptr);

/**
 * Get a text input in the input area
 */
MESSAGE3(CInfoGetInput, TreeItem *, responder, (TreeItem *)nullptr, bool, isNumeric, false, size_t, maxCharacters, 10);

/**
 * Adds text strings to the info area
 */
MESSAGE3(CInfoMsg, Common::String, text, "", bool, newLine, true, bool, replaceLine, false);

/**
 * Signals an unknown/unhandled keypress
 */
MESSAGE0(CHuhMsg);

/**
 * Signals a single standard ASCII keypress
 */
MESSAGE1(CKeyCharMsg, int, key, 32);

/**
 * Signals a keypress
 */
MESSAGE1(CKeypressMsg, Common::KeyState, keyState, Common::KeyState());

/**
 * Called when a popup is finally shown
 */
MESSAGE1(CPopupShownMsg, Gfx::Popup *, view, (Gfx::Popup *)nullptr);

/**
 * Called when a popup is hidden
 */
MESSAGE1(CPopupHiddenMsg, Gfx::Popup *, view, (Gfx::Popup *)nullptr);

/**
 * Called when a game view is shown
 */
MESSAGE2(CShowMsg, Gfx::VisualItem *, view, (Gfx::VisualItem *)nullptr, bool, fadeIn, false);

/**
 * Used when text input is finished, to pass the text back to the owning view
 */
MESSAGE2(CTextInputMsg, Common::String, text, "", bool, escaped, false);

/**
 * Used when character input is finished, to pass the text back to the owning view
 */
MESSAGE1(CCharacterInputMsg, Common::KeyState, keyState, Common::KeyState());

/*-------------------------------------------------------------------*/

/**
 * Used to trigger a party movement
 */
MESSAGE1(CMoveMsg, int, direction, 0);

/**
 * Used to trigger an attack
 */
MESSAGE1(CAttackMsg, int, direction, 0);

/**
 * Used to trigger a board action
 */
MESSAGE0(CBoardMsg);

/**
 * Used to trigger a cast action
 */
MESSAGE0(CCastMsg);

/**
 * Climb up or down
 */
MESSAGE0(CClimbMsg);

/**
 * Used to trigger a drop action
 */
MESSAGE0(CDropMsg);

/**
 * Used to trigger an Enter action
 */
MESSAGE0(CEnterMsg);

/**
 * Used to trigger an exit action
 */
MESSAGE0(CExitTransportMsg);

/**
 * Used to trigger a fire action
 */
MESSAGE0(CFireMsg);

/**
 * Used to trigger a Get action
 */
MESSAGE0(CGetMsg);

/**
 * Used to trigger a HyperJump action
 */
MESSAGE0(CHyperJumpMsg);

/**
 * Used to trigger an inform action
 */
MESSAGE0(CInformMsg);

/**
 * Used to trigger an open action
 */
MESSAGE0(COpenMsg);

/**
 * Pass a turn
 */
MESSAGE0(CPassMsg);

/**
 * Used to trigger a quit (save) action
 */
MESSAGE0(CQuitMsg);

/**
 * Used to trigger a ready item/weapon/spell action
 */
MESSAGE0(CReadyMsg);

/**
 * Used to trigger a stats action
 */
MESSAGE0(CStatsMsg);

/**
 * Used to trigger a steal action
 */
MESSAGE0(CStealMsg);

/**
 * Used to trigger a Transact action
 */
MESSAGE0(CTransactMsg);

/**
 * Used to trigger an Unlock action
 */
MESSAGE0(CUnlockMsg);

/**
 * Used to trigger a view change action
 */
MESSAGE0(CViewChangeMsg);

/**
 * Used to signal an end of turn
 */
MESSAGE0(CEndOfTurnMsg);

} // End of namespace Shared
} // End of namespace Ultima

#endif
