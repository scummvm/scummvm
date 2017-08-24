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

#include "titanic/carry/carry.h"
#include "titanic/debugger.h"
#include "titanic/messages/messages.h"
#include "titanic/npcs/character.h"
#include "titanic/npcs/succubus.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCarry, CGameObject)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(RemoveFromGameMsg)
	ON_MESSAGE(MoveToStartPosMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(PassOnDragStartMsg)
END_MESSAGE_MAP()

CCarry::CCarry() : CGameObject(), _unused5(0), _canTake(true),
		_unusedR(0), _unusedG(0), _unusedB(0), _itemFrame(0),
		_enterFrame(0), _enterFrameSet(false), _visibleFrame(0),
		_npcUse("None"), _fullViewName("NULL"),
		_doesNothingMsg(g_vm->_strings[DOESNT_DO_ANYTHING]),
		_doesntWantMsg(g_vm->_strings[DOESNT_WANT_THIS]) {
}

void CCarry::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_npcUse, indent);
	file->writePoint(_origPos, indent);
	file->writeQuotedLine(_fullViewName, indent);
	file->writeNumberLine(_unused5, indent);
	file->writeNumberLine(_canTake, indent);
	file->writeQuotedLine(_doesNothingMsg, indent);
	file->writeQuotedLine(_doesntWantMsg, indent);
	file->writePoint(_centroid, indent);
	file->writeNumberLine(_unusedR, indent);
	file->writeNumberLine(_unusedG, indent);
	file->writeNumberLine(_unusedB, indent);
	file->writeNumberLine(_itemFrame, indent);
	file->writeQuotedLine(_unused6, indent);
	file->writeNumberLine(_enterFrame, indent);
	file->writeNumberLine(_enterFrameSet, indent);
	file->writeNumberLine(_visibleFrame, indent);

	CGameObject::save(file, indent);
}

void CCarry::load(SimpleFile *file) {
	file->readNumber();
	_npcUse = file->readString();
	_origPos = file->readPoint();
	_fullViewName = file->readString();
	_unused5 = file->readNumber();
	_canTake = file->readNumber();
	_doesNothingMsg = file->readString();
	_doesntWantMsg = file->readString();
	_centroid = file->readPoint();
	_unusedR = file->readNumber();
	_unusedG = file->readNumber();
	_unusedB = file->readNumber();
	_itemFrame = file->readNumber();
	_unused6 = file->readString();
	_enterFrame = file->readNumber();
	_enterFrameSet = file->readNumber();
	_visibleFrame = file->readNumber();

	CGameObject::load(file);
}

bool CCarry::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	CString name = getName();
	debugC(DEBUG_BASIC, kDebugScripts, "MosueDragStartMsg - %s", name.c_str());

	if (_canTake) {
		if (checkStartDragging(msg)) {
			CPassOnDragStartMsg startMsg(msg->_mousePos);
			startMsg.execute(this);
			return true;
		}
	} else {
		if (_visible) {
			CShowTextMsg textMsg(YOU_CANT_GET_THIS);
			textMsg.execute("PET");
		}
	}

	return false;
}

bool CCarry::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	setPosition(msg->_mousePos - _centroid);
	return true;
}

bool CCarry::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	debugC(DEBUG_BASIC, kDebugScripts, "MouseDragEndMsg");
	showMouse();

	if (msg->_dropTarget) {
		if (msg->_dropTarget->isPet()) {
			petAddToInventory();
			return true;
		}

		CCharacter *npc = dynamic_cast<CCharacter *>(msg->_dropTarget);
		if (npc) {
			CUseWithCharMsg charMsg(npc);
			charMsg.execute(this, nullptr, 0);
			return true;
		}

		CDropObjectMsg dropMsg(this);
		if (dropMsg.execute(msg->_dropTarget))
			return true;

		// Fall back on a use with other message
		CUseWithOtherMsg otherMsg(msg->_dropTarget);
		if (otherMsg.execute(this, nullptr, 0))
			return true;
	}

	if (!compareViewNameTo(_fullViewName) || msg->_mousePos.y >= 360) {
		sleep(250);
		petAddToInventory();
	} else {
		setPosition(_origPos);
		loadFrame(_itemFrame);
	}

	return true;
}

bool CCarry::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = dynamic_cast<CSuccUBus *>(msg->_character);
	if (succubus) {
		CSubAcceptCCarryMsg carryMsg;
		carryMsg._item = this;
		carryMsg.execute(succubus);
	} else {
		CShowTextMsg textMsg(_doesntWantMsg);
		textMsg.execute("PET");
		petAddToInventory();
	}

	return true;
}

bool CCarry::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CCarry::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CShowTextMsg textMsg(_doesNothingMsg);
	textMsg.execute("PET");

	if (!compareViewNameTo(_fullViewName) || _bounds.top >= 360) {
		sleep(250);
		petAddToInventory();
	} else {
		setPosition(_origPos);
	}

	return true;
}

bool CCarry::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	if (msg->_visible && _visibleFrame != -1)
		loadFrame(_visibleFrame);

	return true;
}

bool CCarry::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CCarry::RemoveFromGameMsg(CRemoveFromGameMsg *msg) {
	setPosition(Point(0, 0));
	setVisible(false);

	return true;
}

bool CCarry::MoveToStartPosMsg(CMoveToStartPosMsg *msg) {
	setPosition(_origPos);
	return true;
}

bool CCarry::EnterViewMsg(CEnterViewMsg *msg) {
	if (!_enterFrameSet) {
		loadFrame(_enterFrame);
		_enterFrameSet = true;
	}

	return true;
}

bool CCarry::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	hideMouse();

	if (_visibleFrame != -1)
		loadFrame(_visibleFrame);

	if (msg->_value3) {
		_centroid.x = _bounds.width() / 2;
		_centroid.y = _bounds.height() / 2;
	} else {
		_centroid = msg->_mousePos - _bounds;
	}

	setPosition(getMousePos() - _centroid);
	return true;
}

} // End of namespace Titanic
