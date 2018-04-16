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

#include "titanic/game/place_holder_item.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CPlaceHolderItem, CNamedItem);

CPlaceHolderItem::CPlaceHolderItem() :
	_field4C(0), _field60(0), _field64(0), _field68(0), _field7C(0) {
}

void CPlaceHolderItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(7, indent);
	file->writeNumberLine(_field7C, indent);
	file->writeQuotedLine("Movies", indent);
	_clips.save(file, indent + 1);
	file->writeNumberLine(_field68, indent);
	file->writeNumberLine(_field64, indent);
	file->writeNumberLine(_field60, indent);
	_list.save(file, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field4C, indent);
	file->writePoint(_pos1, indent);
	file->writePoint(_pos2, indent);
	file->writeQuotedLine(_string1, indent);

	CNamedItem::save(file, indent);
}

void CPlaceHolderItem::load(SimpleFile *file) {
	switch (file->readNumber()) {
	case 7:
		_field7C = file->readNumber();
		// Intentional fall-through

	case 6:
		file->readString();
		_clips.load(file);
		// Intentional fall-through

	case 5:
		_field68 = file->readNumber();
		// Intentional fall-through

	case 4:
		_field64 = file->readNumber();
		// Intentional fall-through

	case 3:
		_field60 = file->readNumber();
		// Intentional fall-through

	case 2:
		_list.load(file);
		// Intentional fall-through

	case 1:
		_string2 = file->readString();
		_field4C = file->readNumber();
		_pos1 = file->readPoint();
		_pos2 = file->readPoint();
		// Intentional fall-through

	case 0:
		_string1 = file->readString();
		break;

	default:
		break;
	}

	CNamedItem::load(file);
}

} // End of namespace Titanic
