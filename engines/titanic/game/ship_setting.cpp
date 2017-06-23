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

#include "titanic/game/ship_setting.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CShipSetting, CBackground)
	ON_MESSAGE(AddHeadPieceMsg)
	ON_MESSAGE(SetFrameMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

CShipSetting::CShipSetting() : CBackground(),
	_itemName("NULL"), _frameTarget("NULL") {
}

void CShipSetting::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_target, indent);
	file->writePoint(_pos1, indent);
	file->writeQuotedLine(_itemName, indent);
	file->writeQuotedLine(_frameTarget, indent);

	CBackground::save(file, indent);
}

void CShipSetting::load(SimpleFile *file) {
	file->readNumber();
	_target = file->readString();
	_pos1 = file->readPoint();
	_itemName = file->readString();
	_frameTarget = file->readString();

	CBackground::load(file);
}

bool CShipSetting::AddHeadPieceMsg(CAddHeadPieceMsg *msg) {
	_cursorId = CURSOR_HAND;

	if (msg->_value == "Enable") {
		CTurnOn onMsg;
		onMsg.execute(_target);

		if (isEquals("ChickenSetting")) {
			CActMsg actMsg("DecreaseQuantity");
			actMsg.execute("ChickenDispenser");
		}
	} else {
		CTurnOff offMsg;
		offMsg.execute(_target);
	}

	return true;
}

bool CShipSetting::SetFrameMsg(CSetFrameMsg *msg) {
	msg->execute(_frameTarget);
	return true;
}

bool CShipSetting::EnterRoomMsg(CEnterRoomMsg *msg) {
	CSetFrameMsg frameMsg;

	if (_itemName == "ChickenBridge")
		frameMsg._frameNumber = 1;
	else if (_itemName == "FanBridge")
		frameMsg._frameNumber = 2;
	else if (_itemName == "SeasonBridge")
		frameMsg._frameNumber = 3;
	else if (_itemName == "BeamBridge")
		frameMsg._frameNumber = 4;

	frameMsg.execute(this);
	return true;
}

bool CShipSetting::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return false;
	if (_itemName == "NULL")
		return true;

	CTurnOff offMsg;
	offMsg.execute(_target);

	if (isEquals("ChickenSetting") || _itemName == "ChickenBridge") {
		CActMsg actMsg("IncreaseQuantity");
		actMsg.execute("ChickenDispenser");
	}

	if (_itemName != "NULL") {
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
		passMsg.execute(_itemName);

		msg->_dragItem = getRoot()->findByName(_itemName);

		CVisibleMsg visibleMsg(true);
		visibleMsg.execute(_itemName);
	}

	CSetFrameMsg frameMsg(0);
	frameMsg.execute(_frameTarget);
	_itemName = "NULL";
	_cursorId = CURSOR_ARROW;
	return true;
}

} // End of namespace Titanic
