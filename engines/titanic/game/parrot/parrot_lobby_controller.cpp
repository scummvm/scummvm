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

#include "titanic/game/parrot/parrot_lobby_controller.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotLobbyController, CParrotLobbyObject)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CParrotLobbyController::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CParrotLobbyObject::save(file, indent);
}

void CParrotLobbyController::load(SimpleFile *file) {
	file->readNumber();
	CParrotLobbyObject::load(file);
}

bool CParrotLobbyController::ActMsg(CActMsg *msg) {
	if (msg->_action == "Refresh")
		return false;
	else if (msg->_action == "GainParrot")
		_haveParrot = true;
	else if (msg->_action == "LoseParrot")
		_haveParrot = false;
	else if (msg->_action == "GainPerch")
		_havePerch = true;
	else if (msg->_action == "LosePerch")
		_havePerch = false;
	else if (msg->_action == "GainStick")
		_haveStick = true;
	else if (msg->_action == "LoseStick")
		_haveStick = false;

	_flags = 0;
	if (_haveParrot)
		_flags = 4;
	if (_havePerch)
		_flags |= 2;
	if (_haveStick)
		_flags |= 1;

	CActMsg actMsg("Refresh");
	actMsg.execute(findRoom(), CParrotLobbyObject::_type, MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
	actMsg.execute("ParrotLobbyUpdater_TOW");
	return true;
}

} // End of namespace Titanic
