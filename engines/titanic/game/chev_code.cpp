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

#include "titanic/game/chev_code.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChevCode, CGameObject)
	ON_MESSAGE(SetChevLiftBits)
	ON_MESSAGE(SetChevClassBits)
	ON_MESSAGE(SetChevFloorBits)
	ON_MESSAGE(SetChevRoomBits)
	ON_MESSAGE(GetChevLiftNum)
	ON_MESSAGE(GetChevClassNum)
	ON_MESSAGE(GetChevFloorNum)
	ON_MESSAGE(GetChevRoomNum)
	ON_MESSAGE(CheckChevCode)
	ON_MESSAGE(GetChevCodeFromRoomNameMsg)
END_MESSAGE_MAP()

void CChevCode::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_floorBits, indent);
	CGameObject::save(file, indent);
}

void CChevCode::load(SimpleFile *file) {
	file->readNumber();
	_floorBits = file->readNumber();
	CGameObject::load(file);
}

bool CChevCode::SetChevLiftBits(CSetChevLiftBits *msg) {
	_floorBits &= ~0xC0000;
	if (msg->_liftNum > 0 && msg->_liftNum < 5)
		_floorBits = ((msg->_liftNum - 1) << 18) | _floorBits;

	return true;
}

bool CChevCode::SetChevClassBits(CSetChevClassBits *msg) {
	_floorBits &= ~0x30000;
	if (msg->_classNum > 0 && msg->_classNum < 4)
		_floorBits = (msg->_classNum << 16) | msg->_classNum;

	return true;
}

bool CChevCode::SetChevFloorBits(CSetChevFloorBits *msg) {
	int section = (msg->_floorNum + 4) / 10;
	int index = (msg->_floorNum + 4) % 10;
	_floorBits &= ~0xFF00;
	
	int val;
	switch (section) {
	case 0:
		val = 144;
		break;
	case 1:
		val = 208;
		break;
	case 2:
		val = 224;
		break;
	case 3:
		val = 240;
		break;
	default:
		break;
	}
	
	_floorBits |= ((index + val) << 8);
	return true;
}

bool CChevCode::SetChevRoomBits(CSetChevRoomBits *msg) {
	_floorBits &= ~0xff;
	if (msg->_roomNum > 0 && msg->_roomNum < 128)
		_floorBits |= msg->_roomNum * 2;

	return true;
}

bool CChevCode::GetChevLiftNum(CGetChevLiftNum *msg) {
	msg->_liftNum = (_floorBits >> 18) & 3 + 1;
	return true;
}

bool CChevCode::GetChevClassNum(CGetChevClassNum *msg) {
	msg->_classNum = (_floorBits >> 16) & 3;
	return true;
}

bool CChevCode::GetChevFloorNum(CGetChevFloorNum *msg) {
	int val1 = (_floorBits >> 8) & 0xF;
	int val2 = (_floorBits >> 12) & 0xF - 9;

	switch (val2) {
	case 0:
		val2 = 0;
		break;
	case 4:
		val2 = 1;
		break;
	case 5:
		val2 = 2;
		break;
	case 6:
		val2 = 3;
		break;
	default:
		val2 = 4;
		break;
	}

	msg->_floorNum = (val1 >= 10) ? 0 : val1 * 10;
	return true;
}

bool CChevCode::GetChevRoomNum(CGetChevRoomNum *msg) {
	msg->_roomNum = (_floorBits >> 1) & 0x7F;
	return true;
}

bool CChevCode::CheckChevCode(CCheckChevCode *msg) {
	// TODO
	return true;
}

bool CChevCode::GetChevCodeFromRoomNameMsg(CGetChevCodeFromRoomNameMsg *msg) {
	static const char *const ROOM_NAMES[13] = {
		"ParrotLobby", "sculptureChamber", "Bar", "EmbLobby", "MusicRoom",
		"Titania", "BottomOfWell", "Arboretum", "PromenadeDeck",
		"FCRestrnt", "CrtrsCham", "BilgeRoom", "Bridge"
	};
	static const uint CHEV_CODES[13] = {
		0x1D0D9, 0x465FB, 0xB3D97, 0xCC971, 0xF34DB, 0x8A397, 0x59FAD,
		0x4D6AF, 0x79C45, 0x196D9, 0x2F86D, 0x3D94B, 0x39FCB
	};

	for (int idx = 0; idx < 13; ++idx) {
		if (msg->_roomName == ROOM_NAMES[idx]) {
			msg->_chevCode = CHEV_CODES[idx];
			break;
		}
	}

	return true;
}

} // End of namespace Titanic
