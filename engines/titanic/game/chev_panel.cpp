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

#include "titanic/game/chev_panel.h"
#include "titanic/game/chev_code.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChevPanel, CGameObject)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(SetChevPanelBitMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(ClearChevPanelBits)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(SetChevPanelButtonsMsg)
END_MESSAGE_MAP()

void CChevPanel::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_startPos.x, indent);
	file->writeNumberLine(_startPos.y, indent);
	file->writeNumberLine(_chevCode, indent);

	CGameObject::save(file, indent);
}

void CChevPanel::load(SimpleFile *file) {
	file->readNumber();
	_startPos.x = file->readNumber();
	_startPos.y = file->readNumber();
	_chevCode = file->readNumber();

	CGameObject::load(file);
}

bool CChevPanel::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkStartDragging(msg)) {
		_startPos = Point(msg->_mousePos.x - _bounds.left,
			msg->_mousePos.y - _bounds.top);
		CChildDragStartMsg dragMsg(_startPos);
		dragMsg.execute(this, nullptr, MSGFLAG_SCAN);
	}

	return true;
}

bool CChevPanel::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	CChildDragMoveMsg dragMsg(_startPos);
	dragMsg.execute(this, nullptr, MSGFLAG_SCAN);

	setPosition(msg->_mousePos - _startPos);
	return true;
}

bool CChevPanel::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	CChevCode chevCode;
	chevCode._chevCode = _chevCode;
	CCheckChevCode checkCode;
	checkCode.execute(this);
	CClearChevPanelBits panelBits;
	panelBits.execute(this, nullptr, MSGFLAG_SCAN);
	CSetChevPanelButtonsMsg setMsg;
	setMsg._chevCode = checkCode._chevCode;
	setMsg.execute(this);

	return true;
}

bool CChevPanel::SetChevPanelBitMsg(CSetChevPanelBitMsg *msg) {
	_chevCode = (_chevCode & ~(1 << msg->_value1)) | (msg->_value2 << msg->_value1);
	return true;
}

bool CChevPanel::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	setPosition(msg->_mousePos - _startPos);
	return true;
}

bool CChevPanel::ClearChevPanelBits(CClearChevPanelBits *msg) {
	CSetChevPanelButtonsMsg setMsg;
	setMsg._chevCode = 0;
	setMsg.execute(this);

	return true;
}

bool CChevPanel::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CChevPanel::SetChevPanelButtonsMsg(CSetChevPanelButtonsMsg *msg) {
	_chevCode = msg->_chevCode;
	CSetChevButtonImageMsg setMsg;
	setMsg._value2 = 1;
	setMsg.execute(this, nullptr, MSGFLAG_SCAN);

	return true;
}

} // End of namespace Titanic
