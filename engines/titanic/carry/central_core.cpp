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

#include "titanic/carry/central_core.h"
#include "titanic/npcs/parrot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCentralCore, CBrain)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(DropZoneLostObjectMsg)
	ON_MESSAGE(DropZoneGotObjectMsg)
END_MESSAGE_MAP()

void CCentralCore::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CBrain::save(file, indent);
}

void CCentralCore::load(SimpleFile *file) {
	file->readNumber();
	CBrain::load(file);
}

bool CCentralCore::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CString name = msg->_other->getName();
	if (name == "HammerDispensorButton") {
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("BigHammer");
	} else if (name == "SpeechCentre") {
		CShowTextMsg textMsg(DOES_NOT_REACH);
		textMsg.execute("PET");
	}

	return CBrain::UseWithOtherMsg(msg);
}

bool CCentralCore::DropZoneLostObjectMsg(CDropZoneLostObjectMsg *msg) {
	CString name = msg->_object->getName();
	if (name == "PerchCoreHolder") {
		CParrot::_takeOff = true;
		if (isEquals("CentralCore"))
			CParrot::_coreReplaced = false;

		CActMsg actMsg("LosePerch");
		actMsg.execute("ParrotLobbyController");
	} else if (name == "PerchHolder") {
		CActMsg actMsg("LoseStick");
		actMsg.execute("ParrotLobbyController");
	}

	return true;
}

bool CCentralCore::DropZoneGotObjectMsg(CDropZoneGotObjectMsg *msg) {
	CString name = msg->_object->getName();
	if (name == "PerchCoreHolder") {
		CParrot::_takeOff = false;

		if (isEquals("CentralCore")) {
			CParrot::_coreReplaced = true;
			CActMsg actMsg("CoreReplaced");
			actMsg.execute("ParrotCage");
		}

		CActMsg actMsg("GainPerch");
		actMsg.execute("ParrotLobbyController");
	} else if (name == "PerchHolder") {
		CActMsg actMsg("GainStick");
		actMsg.execute("ParrotLobbyController");
	}

	return true;
}

} // End of namespace Titanic
