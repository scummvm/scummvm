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

#include "titanic/room_flags.h"
#include "titanic/titanic.h"
#include "titanic/support/strings.h"

namespace Titanic {

#define ELEVATOR_SHIFT 18
#define ELEVATOR_MASK 3
#define PASSENGER_CLASS_SHIFT 16
#define PASSENGER_CLASS_MASK 3
#define FLOOR_SHIFT 8
#define FLOOR_MASK 0xFF
#define ROOM_SHIFT 1
#define ROOM_MASK 0x7F

struct TransportFlagsEntry {
	const char *const _roomName;
	uint _roomFlags;
};
struct SuccUBusFlagsEntry {
	const char *const _roomName;
	uint _roomFlags;
	PassengerClass _classNum;
};

#define TRANSPORT_ROOMS_SIZE 6
const TransportFlagsEntry TRANSPORT_ROOMS[TRANSPORT_ROOMS_SIZE] = {
	{ "TopOfWell", 0xDF4D1 },
	{ "Pellerator", 0xC95E9 },
	{ "Dome", 0xAD171 },
	{ "Lift", 0x96E45 },
	{ "SGTLeisure", 0x5D3AD },
	{ "ServiceElevator", 0x68797 }
};

#define SUCCUBUS_ROOMS_SIZE 17
const SuccUBusFlagsEntry SUCCUBUS_ROOMS[SUCCUBUS_ROOMS_SIZE] = {
	{ "ParrotLobby", 0x1D0D9, THIRD_CLASS },
	{ "SculptureChamber", 0x465FB, SECOND_CLASS },
	{ "Bar", 0x0B3D97, SECOND_CLASS },
	{ "EmbLobby", 0x0CC971, THIRD_CLASS },
	{ "MoonEmbLobby", 0x0CC971, THIRD_CLASS },
	{ "MusicRoom", 0x0F34DB, SECOND_CLASS },
	{ "MusicRoomLobby", 0x0F34DB, SECOND_CLASS },
	{ "Titania", 0x8A397, THIRD_CLASS },
	{ "BottomOfWell", 0x59FAD, THIRD_CLASS },
	{ "Arboretum", 0x4D6AF, FIRST_CLASS },
	{ "PromenadeDeck", 0x79C45, SECOND_CLASS },
	{ "1stClassRestaurant", 0x896B9, FIRST_CLASS },
	{ "CreatorsChamber", 0x2F86D, SECOND_CLASS },
	{ "CreatorsChamberOn", 0x2F86D, SECOND_CLASS },
	{ "BilgeRoom", 0x3D94B, THIRD_CLASS },
	{ "BilgeRoomWith", 0x3D94B, THIRD_CLASS },
	{ "Bridge", 0x39FCB, THIRD_CLASS }
};

int CRoomFlags::getConditionally() const {
	if (getRoomArea() != 5 || getRoomCategory())
		return _data;
	else
		return 5;
}

bool CRoomFlags::isTransportRoom() const {
	for (int idx = 0; idx < TRANSPORT_ROOMS_SIZE; ++idx) {
		if (TRANSPORT_ROOMS[idx]._roomFlags == _data)
			return true;
	}

	return false;
}

int CRoomFlags::getRoomCategory() const {
	if (getRoomNum() != 0)
		return false;

	CRoomFlags tempFlags = _data;
	tempFlags.setRoomBits(1);
	return tempFlags.getRoomArea() != 5;
}

int CRoomFlags::getRoomArea() const {
	if (isSuccUBusRoomFlags())
		return 4;

	if (!getBit0()) {
		uint v3 = getFloorNum();
		if (v3 <= 38) {
			uint v4 = getRoomNum();
			if (v4 <= 18) {
				uint v6 = getElevatorNum();

				if (v6 >= 1 && v6 <= 4) {
					uint v7 = (int)getPassengerClassNum() - 1;
					if (v7) {
						uint v8 = v7 - 1;
						if (v8) {
							if (v8 == 1 && is28To38(v3) && (v6 & 1) && v4 >= 1)
								return 3;
						} else if (is20To27(v3)) {
							if (v6 & 1) {
								if (v4 >= 1 && v4 <= 3)
									return 2;
							} else if (v4 >= 1 && v4 <= 4) {
								return 2;
							}
						}
					} else if (is2To19(v3) && v4 >= 1 && v4 <= 3) {
						return 1;
					}
				}
			}
		}
	}

	return 5;
}

CString CRoomFlags::getRoomDesc() const {
	Strings &str = g_vm->_strings;

	switch (getRoomArea()) {
	case 1:
	case 2:
	case 3: {
		CString result = getPassengerClassDesc();
		result += ", ";
		result += getFloorDesc();
		result += ", ";
		result += getElevatorDesc();
		result += ", ";
		result += getRoomNumDesc();
		return result;
	}

	case 4:
		switch (_data) {
		case 0x1D0D9:
			return str[THE_PARROT_LOBBY];
		case 0x2F86D:
			return str[THE_CREATORS_CHAMBER];
		case 0x39FCB:
			return str[THE_BRIDGE];
		case 0x3D94B:
			return str[THE_BILGE_ROOM];
		case 0x465FB:
			return str[THE_SCULPTURE_CHAMBER];
		case 0x4D6AF:
			return str[THE_ARBORETUM];
		case 0x59FAD:
			return str[THE_BOTTOM_OF_THE_WELL];
		case 0x79C45:
			return str[THE_PROMENADE_DECK];
		case 0x896B9:
			return str[RESTAURANT_1ST_CLASS];
		case 0x8A397:
			return str[TITANIAS_ROOM];
		case 0xB3D97:
			return str[THE_BAR];
		case 0xCC971:
			return str[THE_EMBARKATION_LOBBY];
		case 0xF34DB:
			return  str[THE_MUSIC_ROOM];
		default:
			break;
		}
		return str[UNKNOWN_ROOM];

	case 5:
		if (isTransportRoom()) {
			switch (_data) {
			case 0x68797:
				return str[THE_SERVICE_ELEVATOR];
			case 0x5D3AD:
				return str[SGT_LEISURE_LOUNGE];
			case 0x96E45:
				return str[THE_ELEVATOR];
			case 0xAD171:
				return str[THE_DOME];
			case 0xC95E9:
				return str[THE_PELLERATOR];
			case 0xDF4D1:
				return str[THE_TOP_OF_THE_WELL];
			default:
				break;
			}
		}

		if (getRoomCategory() == 0) {
			return str[NOWHERE_TO_GO];
		} else {
			CString result = getPassengerClassDesc();
			result += ", ";
			result += getFloorDesc();
			return result;
		}
		break;

	default:
		break;
	}

	return str[UNKNOWN_ROOM];
}

void CRoomFlags::setElevatorBits(uint val) {
	_data &= ~(ELEVATOR_MASK << ELEVATOR_SHIFT);
	_data |= (val & ELEVATOR_MASK) << ELEVATOR_SHIFT;
}

uint CRoomFlags::getElevatorBits() const {
	return (_data >> ELEVATOR_SHIFT) & ELEVATOR_MASK;
}

void CRoomFlags::setPassengerClassBits(uint val) {
	_data &= ~(PASSENGER_CLASS_MASK << PASSENGER_CLASS_SHIFT);
	_data |= (val & PASSENGER_CLASS_MASK) << PASSENGER_CLASS_SHIFT;
}

uint CRoomFlags::getPassengerClassBits() const {
	return (_data >> PASSENGER_CLASS_SHIFT) & PASSENGER_CLASS_MASK;
}

CString CRoomFlags::getPassengerClassDesc() const {
	PassengerClass classNum = getPassengerClassNum();
	Strings &str = g_vm->_strings;

	switch (classNum) {
	case FIRST_CLASS:
		return str[CLASS_1];
	case SECOND_CLASS:
		return str[CLASS_2];
	case THIRD_CLASS:
		return str[CLASS_3];
	default:
		return str[CLASS_NONE];
	}
}

void CRoomFlags::setFloorBits(uint val) {
	_data &= ~(FLOOR_MASK << FLOOR_SHIFT);
	_data |= (val & FLOOR_MASK) << FLOOR_SHIFT;
}

uint CRoomFlags::getFloorBits() const {
	return (_data >> FLOOR_SHIFT) & FLOOR_MASK;
}

uint CRoomFlags::decodeFloorBits(uint bits) const {
	int base = 0;
	int offset = bits & 0xF;

	switch ((bits >> 4) & 0xF) {
	case 9:
		base = 0;
		break;
	case 0xD:
		base = 10;
		break;
	case 0xE:
		base = 20;
		break;
	case 0xF:
		base = 30;
		break;
	default:
		base = 40;
		break;
	}

	return offset >= 10 ? 0 : base + offset;
}

void CRoomFlags::setFloorNum(uint floorNum) {
	uint base = 0;

	switch (floorNum / 10) {
	case 0:
		base = 0x90;
		break;
	case 1:
		base = 0xD0;
		break;
	case 2:
		base = 0xE0;
		break;
	case 3:
		base = 0xF0;
		break;
	default:
		break;
	}

	setFloorBits(base | (floorNum % 10));
}

uint CRoomFlags::getFloorNum() const {
	return decodeFloorBits(getFloorBits());
}

void CRoomFlags::setRoomBits(uint roomBits) {
	_data &= ~(ROOM_MASK << ROOM_SHIFT);
	_data |= (roomBits & ROOM_MASK) << ROOM_SHIFT;
}

uint CRoomFlags::getRoomBits() const {
	return (_data >> ROOM_SHIFT) & ROOM_MASK;
}

bool CRoomFlags::isSuccUBusRoomFlags() const {
	for (int idx = 0; idx < SUCCUBUS_ROOMS_SIZE; ++idx) {
		if (SUCCUBUS_ROOMS[idx]._roomFlags == _data)
			return true;
	}

	return false;
}

bool CRoomFlags::getBit0() const {
	return _data & 1;
}

uint CRoomFlags::getSpecialRoomFlags(const CString &roomName) {
	for (int idx = 0; idx < SUCCUBUS_ROOMS_SIZE; ++idx) {
		if (roomName == SUCCUBUS_ROOMS[idx]._roomName)
			return SUCCUBUS_ROOMS[idx]._roomFlags;
	}

	for (int idx = 0; idx < TRANSPORT_ROOMS_SIZE; ++idx) {
		if (roomName == TRANSPORT_ROOMS[idx]._roomName)
			return TRANSPORT_ROOMS[idx]._roomFlags;
	}

	return 0;
}

PassengerClass CRoomFlags::getSuccUBusClass(const CString &roomName) const {
	for (int idx = 0; idx < SUCCUBUS_ROOMS_SIZE; ++idx) {
		if (roomName == SUCCUBUS_ROOMS[idx]._roomName)
			return SUCCUBUS_ROOMS[idx]._classNum;
	}

	return NO_CLASS;
}

CString CRoomFlags::getSuccUBusRoomName() const {
	for (int idx = 0; idx < SUCCUBUS_ROOMS_SIZE; ++idx) {
		if (SUCCUBUS_ROOMS[idx]._roomFlags == _data)
			return SUCCUBUS_ROOMS[idx]._roomName;
	}

	return CString();
}

void CRoomFlags::changeClass(PassengerClass newClassNum) {
	uint floorNum = getFloorNum();
	uint roomNum = getRoomNum();
	uint elevatorNum = getElevatorNum();
	PassengerClass classNum = getPassengerClassNum();
	uint v10, v11, v12, v13;

	switch (classNum) {
	case FIRST_CLASS:
		v10 = 2;
		v11 = 19;
		v12 = 1;
		v13 = 3;
		break;

	case SECOND_CLASS:
		v10 = 20;
		v11 = 27;
		v12 = 1;
		v13 = (elevatorNum & 1) ? 3 : 4;
		break;

	case THIRD_CLASS:
		v10 = 28;
		v11 = 38;
		v12 = 1;
		v13 = 18;
		break;

	default:
		v10 = 0;
		v11 = 0;
		v12 = 0;
		v13 = 0;
		break;
	}

	// Perform action to change room or floor
	switch (newClassNum) {
	case FIRST_CLASS:
		if (--roomNum < v12)
			roomNum = v12;
		break;

	case SECOND_CLASS:
		if (++roomNum > v13)
			roomNum = v13;
		break;

	case THIRD_CLASS:
		if (--floorNum < v10)
			floorNum = v10;
		break;

	case UNCHECKED:
		if (++floorNum > v11)
			floorNum = v11;

	default:
		break;
	}

	// Set new floor and room
	setFloorNum(floorNum);
	setRoomBits(roomNum);
}

CString CRoomFlags::getElevatorDesc() const {
	return CString::format(g_vm->_strings[ELEVATOR_NUM].c_str(), getElevatorNum());
}

CString CRoomFlags::getFloorDesc() const {
	return CString::format(g_vm->_strings[FLOOR_NUM].c_str(), getFloorNum());
}

CString CRoomFlags::getRoomNumDesc() const {
	return CString::format(g_vm->_strings[ROOM_NUM].c_str(), getRoomNum());
}

bool CRoomFlags::compareClassElevator(uint flags1, uint flags2) {
	CRoomFlags f1(flags1);
	CRoomFlags f2(flags2);

	if (f1.getFloorNum() != f2.getFloorNum())
		return false;

	uint elev1 = f1.getElevatorNum();
	uint elev2 = f2.getElevatorNum();
	PassengerClass class1 = f1.getPassengerClassNum();
	PassengerClass class2 = f2.getPassengerClassNum();

	if (class1 == FIRST_CLASS || class1 == SECOND_CLASS) {
		if (elev1 == 2)
			elev1 = 1;
		else if (elev1 == 4)
			elev1 = 3;
	}
	if (class2 == FIRST_CLASS || class2 == SECOND_CLASS) {
		if (elev2 == 2)
			elev2 = 1;
		else if (elev2 == 4)
			elev2 = 3;
	}

	return elev1 == elev2;
}

bool CRoomFlags::compareLocation(uint flags1, uint flags2) {
	CRoomFlags f1(flags1);
	CRoomFlags f2(flags2);

	return f1.getElevatorNum() == f2.getElevatorNum() &&
		f1.getFloorNum() == f2.getFloorNum() &&
		f1.getRoomNum() == f2.getRoomNum();
}

bool CRoomFlags::isTitania(uint flags1, uint flags2) {
	return flags2 == 0x8A397;
}

void CRoomFlags::setRandomLocation(PassengerClass classNum, bool flag) {
	uint minRoom, elevNum, maxRoom, maxFloor, minFloor;

	do {
		switch (classNum) {
		case FIRST_CLASS:
			minFloor = 2;
			maxFloor = 19;
			minRoom = 1;
			maxRoom = 3;
			elevNum = g_vm->getRandomNumber(flag ? 2 : 3);
			break;

		case SECOND_CLASS:
			minFloor = 20;
			maxFloor = 27;
			elevNum = g_vm->getRandomNumber(flag ? 2 : 3);
			minRoom = 1;
			maxRoom = ((elevNum - 1) & 1) ? 3 : 4;
			break;

		case THIRD_CLASS:
			minRoom = 1;
			minFloor = 28;
			maxFloor = 38;
			maxRoom = 18;
			elevNum = g_vm->getRandomNumber(1) ? 2 : 0;
			break;

		default:
			return;
		}

		uint floorNum = minFloor + g_vm->getRandomNumber(maxFloor - minFloor);
		uint roomNum = minRoom + g_vm->getRandomNumber(maxRoom - minRoom);
		setElevatorBits(elevNum);
		setRoomBits(roomNum);
		setFloorNum(floorNum);
		setPassengerClassBits(classNum);
	} while (_data == 0x59706);
}

PassengerClass CRoomFlags::whatPassengerClass(int floorNum) {
	if (is2To19(floorNum))
		return FIRST_CLASS;

	return is20To27(floorNum) ? SECOND_CLASS : THIRD_CLASS;
}

} // End of namespace Titanic
