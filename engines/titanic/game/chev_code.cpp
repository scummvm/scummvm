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
	file->writeNumberLine(_chevCode, indent);
	CGameObject::save(file, indent);
}

void CChevCode::load(SimpleFile *file) {
	file->readNumber();
	_chevCode = file->readNumber();
	CGameObject::load(file);
}

bool CChevCode::SetChevLiftBits(CSetChevLiftBits *msg) {
	_chevCode &= ~0xC0000;
	if (msg->_liftNum > 0 && msg->_liftNum < 5)
		_chevCode = ((msg->_liftNum - 1) << 18) | _chevCode;

	return true;
}

bool CChevCode::SetChevClassBits(CSetChevClassBits *msg) {
	_chevCode &= ~0x30000;
	if (msg->_classNum > 0 && msg->_classNum < 4)
		_chevCode = (msg->_classNum << 16) | msg->_classNum;

	return true;
}

bool CChevCode::SetChevFloorBits(CSetChevFloorBits *msg) {
	int section = (msg->_floorNum + 4) / 10;
	int index = (msg->_floorNum + 4) % 10;
	_chevCode &= ~0xFF00;

	int val = 0;
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

	_chevCode |= ((index + val) << 8);
	return true;
}

bool CChevCode::SetChevRoomBits(CSetChevRoomBits *msg) {
	_chevCode &= ~0xff;
	if (msg->_roomFlags > 0 && msg->_roomFlags < 128)
		_chevCode |= msg->_roomFlags * 2;

	return true;
}

bool CChevCode::GetChevLiftNum(CGetChevLiftNum *msg) {
	msg->_liftNum = ((_chevCode >> 18) & 3) + 1;
	return true;
}

bool CChevCode::GetChevClassNum(CGetChevClassNum *msg) {
	msg->_classNum = (_chevCode >> 16) & 3;
	return true;
}

bool CChevCode::GetChevFloorNum(CGetChevFloorNum *msg) {
	int val1 = (_chevCode >> 8) & 0xF;
	int val2 = ((_chevCode >> 12) & 0xF) - 9;

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

	msg->_floorNum = (val1 >= 10) ? 0 : val2 * 10 + val1;
	return true;
}

bool CChevCode::GetChevRoomNum(CGetChevRoomNum *msg) {
	msg->_roomNum = (_chevCode >> 1) & 0x7F;
	return true;
}

bool CChevCode::CheckChevCode(CCheckChevCode *msg) {
	CGetChevClassNum getClassMsg;
	CGetChevLiftNum getLiftMsg;
	CGetChevFloorNum getFloorMsg;
	CGetChevRoomNum getRoomMsg;
	CString roomName;
	int classNum = 0;
	uint bits = 0;

	if (_chevCode & 1) {
		switch (_chevCode) {
		case 0x1D0D9:
			roomName = "ParrLobby";
			classNum = 4;
			break;
		case 0x196D9:
			roomName = "FCRestrnt";
			classNum = 4;
			break;
		case 0x39FCB:
			roomName = "Bridge";
			classNum = 4;
			break;
		case 0x2F86D:
			roomName = "CrtrsCham";
			classNum = 4;
			break;
		case 0x465FB:
			roomName = "SculpCham";
			classNum = 4;
			break;
		case 0x3D94B:
			roomName = "BilgeRoom";
			classNum = 4;
			break;
		case 0x59FAD:
			roomName = "BoWell";
			classNum = 4;
			break;
		case 0x4D6AF:
			roomName = "Arboretum";
			classNum = 4;
			break;
		case 0x8A397:
			roomName = "TitRoom";
			classNum = 4;
			break;
		case 0x79C45:
			roomName = "PromDeck";
			classNum = 4;
			break;
		case 0xB3D97:
			roomName = "Bar";
			classNum = 4;
			break;
		case 0xCC971:
			roomName = "EmbLobby";
			classNum = 4;
			break;
		case 0xF34DB:
			roomName = "MusicRoom";
			classNum = 4;
			break;
		default:
			roomName = "BadRoom";
			classNum = 5;
			break;
		}

		bits = classNum == 5 ? 0x3D94B : _chevCode;
	} else {
		getFloorMsg.execute(this);
		getRoomMsg.execute(this);
		getClassMsg.execute(this);
		getLiftMsg.execute(this);
		if (getFloorMsg._floorNum > 37 || getRoomMsg._roomNum > 18)
			classNum = 5;

		if (classNum == 5) {
			bits = 0x3D94B;
		} else {
			switch (getClassMsg._classNum) {
			case 1:
				if (getFloorMsg._floorNum >= 2 && getFloorMsg._floorNum <= 18
					&& getRoomMsg._roomNum >= 1 && getRoomMsg._roomNum <= 3
					&& getLiftMsg._liftNum >= 1 && getLiftMsg._liftNum <= 4)
					classNum = 1;
				else
					classNum = 5;
				break;

			case 2:
				if (getFloorMsg._floorNum >= 19 && getFloorMsg._floorNum <= 26
					&& getRoomMsg._roomNum >= 1 && getRoomMsg._roomNum <= 5
					&& getLiftMsg._liftNum >= 1 && getLiftMsg._liftNum <= 4)
					classNum = 2;
				else
					classNum = 5;
				break;

			case 3:
				if (getFloorMsg._floorNum >= 27 && getFloorMsg._floorNum <= 37
					&& getRoomMsg._roomNum >= 1 && getRoomMsg._roomNum <= 18
					&& (getLiftMsg._liftNum & 1) == 1
					&& getLiftMsg._liftNum >= 1 && getLiftMsg._liftNum <= 4)
					classNum = 3;
				else
					classNum = 5;
				break;

			default:
				break;
			}
		}
	}

	msg->_classNum = classNum;
	msg->_chevCode = bits;

	// WORKAROUND: Skipped code from original that was for debugging purposes only
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
