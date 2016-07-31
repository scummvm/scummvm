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
#include "titanic/messages/messages.h"
#include "titanic/npcs/character.h"
#include "titanic/npcs/succubus.h"
#include "titanic/pet_control/pet_control.h"

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

CCarry::CCarry() : CGameObject(), _fieldDC(0), _fieldE0(1),
		_field100(0), _field104(0), _field108(0), _field10C(0),
		_itemFrame(0), _enterFrame(0), _enterFrameSet(false), _visibleFrame(0),
	_string1("None"),
	_fullViewName("NULL"),
	_string3("That doesn't seem to do anything."),
	_string4("It doesn't seem to want this.") {
}

void CCarry::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string1, indent);
	file->writePoint(_origPos, indent);
	file->writeQuotedLine(_fullViewName, indent);
	file->writeNumberLine(_fieldDC, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeQuotedLine(_string4, indent);
	file->writePoint(_tempPos, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_itemFrame, indent);
	file->writeQuotedLine(_string5, indent);
	file->writeNumberLine(_enterFrame, indent);
	file->writeNumberLine(_enterFrameSet, indent);
	file->writeNumberLine(_visibleFrame, indent);

	CGameObject::save(file, indent);
}

void CCarry::load(SimpleFile *file) {
	file->readNumber();
	_string1 = file->readString();
	_origPos = file->readPoint();
	_fullViewName = file->readString();
	_fieldDC = file->readNumber();
	_fieldE0 = file->readNumber();
	_string3 = file->readString();
	_string4 = file->readString();
	_tempPos = file->readPoint();
	_field104 = file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_itemFrame = file->readNumber();
	_string5 = file->readString();
	_enterFrame = file->readNumber();
	_enterFrameSet = file->readNumber();
	_visibleFrame = file->readNumber();

	CGameObject::load(file);
}

bool CCarry::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	CString name = getName();

	if (_fieldE0) {
		if (_visible) {
			CShowTextMsg textMsg("You can't get this.");
			textMsg.execute("PET");
		}
	} else {
		if (checkStartDragging(msg)) {
			CPassOnDragStartMsg startMsg(msg->_mousePos);
			startMsg.execute(this);
			return true;
		}
	}

	return false;
}

bool CCarry::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	setPosition(msg->_mousePos - _tempPos);
	return true;
}

bool CCarry::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	if (msg->_dropTarget) {
		if (msg->_dropTarget->isPet()) {
			petAddToInventory();
			return true;
		}

		CCharacter *npc = static_cast<CCharacter *>(msg->_dropTarget);
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

	CString viewName = getViewFullName();
	if (viewName.empty() || msg->_mousePos.y >= 360) {
		sleep(250);
		petAddToInventory();
	} else {
		setPosition(_origPos);
		loadFrame(_itemFrame);
	}

	return true;
}

bool CCarry::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = static_cast<CSuccUBus *>(msg->_character);
	if (succubus) {
		CSubAcceptCCarryMsg carryMsg;
		carryMsg._item = this;
		carryMsg.execute(succubus);
	} else {
		CShowTextMsg textMsg(_string4);
		textMsg.execute("PET");
		petAddToInventory();
	}

	return true;
}

bool CCarry::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CCarry::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CShowTextMsg textMsg(_string3);
	textMsg.execute("PET");

	_fullViewName = getViewFullName();
	if (_fullViewName.empty() || _bounds.top >= 360) {
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
	if (_visibleFrame != -1)
		loadFrame(_visibleFrame);

	if (msg->_value3) {
		_tempPos.x = _bounds.width() / 2;
		_tempPos.y = _bounds.height() / 2;
	} else {
		_tempPos = msg->_mousePos - _bounds;
	}

	setPosition(_tempPos - getMousePos());
	return true;
}

} // End of namespace Titanic
