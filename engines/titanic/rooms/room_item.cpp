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

#include "titanic/rooms/room_item.h"

namespace Titanic {

CRoomItem::CRoomItem() : CNamedItem(), _roomNumber(0),
		_roomDimensionX(0.0), _roomDimensionY(0.0) {
}

void CRoomItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(3, indent);
	file->writeQuotedLine("Exit Movies", indent);
	_exitMovieKey.save(file, indent);

	file->writeNumberLine(_roomDimensionX * 1000.0, indent + 1);
	file->writeNumberLine(_roomDimensionY * 1000.0, indent + 1);

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
		// Deliberate fall-through

	case 2:
		// Read room dimensions
		file->readBuffer();
		_roomDimensionX = (double)file->readNumber() / 1000.0;
		_roomDimensionY = (double)file->readNumber() / 1000.0;
		// Deliberate fall-through

	case 1:
		// Read transition movie key and clip list
		file->readBuffer();
		_transitionMovieKey.load(file);

		file->readBuffer();
		_clipList.load(file);
		loading();
		// Deliberate fall-through

	case 0:
		// Read room rect
		file->readBuffer();
		_roomRect.left = file->readNumber();
		_roomRect.top = file->readNumber();
		_roomRect.right = file->readNumber();
		_roomRect.bottom = file->readNumber();
		file->readBuffer();
		break;

	default:
		break;
	}

	CNamedItem::load(file);
}

void CRoomItem::loading() {
	// TODO
}

} // End of namespace Titanic
