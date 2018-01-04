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

#include "titanic/carry/hose.h"
#include "titanic/npcs/succubus.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHose, CCarry)
	ON_MESSAGE(DropZoneGotObjectMsg)
	ON_MESSAGE(PumpingMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(HoseConnectedMsg)
	ON_MESSAGE(DropZoneLostObjectMsg)
END_MESSAGE_MAP()

CHoseStatics *CHose::_statics;

void CHose::init() {
	_statics = new CHoseStatics();
}

void CHose::deinit() {
	delete _statics;
}

CHose::CHose() : CCarry() {
}

void CHose::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_statics->_actionVal, indent);
	file->writeQuotedLine(_statics->_actionTarget, indent);
	file->writeQuotedLine(_unused1, indent);
	CCarry::save(file, indent);
}

void CHose::load(SimpleFile *file) {
	file->readNumber();
	_statics->_actionVal = file->readNumber();
	_statics->_actionTarget = file->readString();
	_unused1 = file->readString();
	CCarry::load(file);
}

bool CHose::DropZoneGotObjectMsg(CDropZoneGotObjectMsg *msg) {
	_statics->_actionTarget = msg->_object->getName();
	CPumpingMsg pumpingMsg;
	pumpingMsg._value = _statics->_actionVal;
	pumpingMsg.execute(_statics->_actionTarget);
	CHoseConnectedMsg connectedMsg;
	connectedMsg._connected = true;
	connectedMsg.execute(this);

	return true;
}

bool CHose::PumpingMsg(CPumpingMsg *msg) {
	_statics->_actionVal = msg->_value;
	if (!_statics->_actionTarget.empty()) {
		CPumpingMsg pumpingMsg;
		pumpingMsg._value = _statics->_actionVal;
		pumpingMsg.execute(_statics->_actionTarget);
	}

	return true;
}

bool CHose::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = dynamic_cast<CSuccUBus *>(msg->_character);
	if (!_statics->_actionVal && succubus) {
		CHoseConnectedMsg connectedMsg(1, this);
		if (connectedMsg.execute(succubus))
			return true;
	}

	return CCarry::UseWithCharMsg(msg);
}

bool CHose::HoseConnectedMsg(CHoseConnectedMsg *msg) {
	if (msg->_connected) {
		CHose *hose = dynamic_cast<CHose *>(findChildInstanceOf(CHose::_type));
		if (hose) {
			hose->setVisible(true);
			hose->petAddToInventory();
		}
	}

	return true;
}

bool CHose::DropZoneLostObjectMsg(CDropZoneLostObjectMsg *msg) {
	CPumpingMsg pumpingMsg;
	pumpingMsg._value = 0;
	pumpingMsg.execute(msg->_object);

	return true;
}

} // End of namespace Titanic
