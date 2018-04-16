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

#include "titanic/core/room_item.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CRoomItem, CNamedItem);

CRoomItem::CRoomItem() : CNamedItem(), _roomNumber(0),
		_roomDimensionX(0.0), _roomDimensionY(0.0) {
}

void CRoomItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(3, indent);
	file->writeQuotedLine("Exit Movies", indent);
	_exitMovieKey.save(file, indent);

	file->writeQuotedLine("Room dimensions x 1000", indent);
	file->writeNumberLine((int)(_roomDimensionX * 1000.0), indent + 1);
	file->writeNumberLine((int)(_roomDimensionY * 1000.0), indent + 1);

	file->writeQuotedLine("Transition Movie", indent);
	_transitionMovieKey.save(file, indent);

	file->writeQuotedLine("Movie Clip list", indent);
	_clipList.save(file, indent + 1);

	file->writeQuotedLine("Room Rect", indent);
	file->writeNumberLine(_roomRect.left, indent + 1);
	file->writeNumberLine(_roomRect.top, indent + 1);
	file->writeNumberLine(_roomRect.right, indent + 1);
	file->writeNumberLine(_roomRect.bottom, indent + 1);

	file->writeQuotedLine("Room Number", indent);
	file->writeNumberLine(_roomNumber, indent);

	CNamedItem::save(file, indent);
}

void CRoomItem::load(SimpleFile *file) {
	int val = file->readNumber();

	switch (val) {
	case 3:
		// Read exit movie
		file->readBuffer();
		_exitMovieKey.load(file);
		// Intentional fall-through

	case 2:
		// Read room dimensions
		file->readBuffer();
		_roomDimensionX = (double)file->readNumber() / 1000.0;
		_roomDimensionY = (double)file->readNumber() / 1000.0;
		// Intentional fall-through

	case 1:
		// Read transition movie key and clip list
		file->readBuffer();
		_transitionMovieKey.load(file);

		file->readBuffer();
		_clipList.load(file);
		postLoad();
		// Intentional fall-through

	case 0:
		// Read room rect
		file->readBuffer();
		_roomRect.left = file->readNumber();
		_roomRect.top = file->readNumber();
		_roomRect.right = file->readNumber();
		_roomRect.bottom = file->readNumber();
		file->readBuffer();
		_roomNumber = file->readNumber();
		break;

	default:
		break;
	}

	CNamedItem::load(file);
}

void CRoomItem::postLoad() {
	if (!_exitMovieKey.getFilename().empty())
		return;

	CString name = _transitionMovieKey.getFilename();
	if (name.right(7) == "nav.avi") {
		_exitMovieKey = CResourceKey(name.left(name.size() - 7) + "exit.avi");
	}
}

void CRoomItem::calcNodePosition(const Point &nodePos, double &xVal, double &yVal) const {
	xVal = yVal = 0.0;

	if (_roomDimensionX >= 0.0 && _roomDimensionY >= 0.0) {
		xVal = _roomRect.width() / _roomDimensionX;
		yVal = _roomRect.height() / _roomDimensionY;

		xVal = (nodePos.x - _roomRect.left) / xVal;
		yVal = (nodePos.y - _roomRect.top) / yVal;
	}
}

int CRoomItem::getScriptId() const {
	CString name = getName();
	if (name == "1stClassLobby")
		return 130;
	else if (name == "1stClassRestaurant")
		return 132;
	else if (name == "1stClassState")
		return 131;
	else if (name == "2ndClassLobby")
		return 128;
	else if (name == "Bar")
		return 112;
	else if (name == "BottomOfWell")
		return 108;
	else if (name == "Bridge")
		return 121;
	else if (name == "Dome")
		return 122;
	else if (name == "Home")
		return 100;
	else if (name == "Lift")
		return 103;
	else if (name == "MusicRoom")
		return 117;
	else if (name == "MusicRoomLobby")
		return 118;
	else if (name == "ParrotLobby")
		return 111;
	else if (name == "Pellerator")
		return 104;
	else if (name == "PromenadeDeck")
		return 114;
	else if (name == "SculptureChamber")
		return 116;
	else if (name == "secClassState")
		return 129;
	else if (name == "ServiceElevator")
		return 102;
	else if (name == "SGTLeisure")
		return 125;
	else if (name == "SGTLittleLift")
		return 105;
	else if (name == "SgtLobby")
		return 124;
	else if (name == "SGTState")
		return 126;
	else if (name == "Titania")
		return 123;
	else if (name == "TopOfWell")
		return 107;
	else if (name == "EmbLobby" || name == "MoonEmbLobby")
		return 110;
	else if (name == "CreatorsChamber" || name == "CreatorsChamberOn")
		return 113;
	else if (name == "Arboretum" || name == "FrozenArboretum")
		return 115;
	else if (name == "BilgeRoom" || name == "BilgeRoomWith")
		return 101;

	return 0;
}

CResourceKey CRoomItem::getTransitionMovieKey() {
	_transitionMovieKey.scanForFile();
	return _transitionMovieKey;
}

CResourceKey CRoomItem::getExitMovieKey() {
	return _exitMovieKey;
}

} // End of namespace Titanic
