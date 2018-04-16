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

#include "titanic/game/variable_list.h"

namespace Titanic {

void CVariableListItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(3, indent);
	file->writeNumberLine(_field44, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_field18, indent);
	file->writeNumberLine(_field40, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeQuotedLine(_string4, indent);

	ListItem::save(file, indent);
}

void CVariableListItem::load(SimpleFile *file) {
	int field40 = 0, field44 = 0;

	switch (file->readNumber()) {
	case 3:
		field44 = file->readNumber();
		// Intentional fall-through

	case 2:
		_string1 = file->readString();
		_field18 = file->readNumber();
		// Intentional fall-through

	case 1:
		field40 = file->readNumber();
		// Intentional fall-through

	case 0:
		_string2 = file->readString();
		_string3 = file->readString();
		_string4 = file->readString();
		break;

	default:
		break;
	}

	_field40 = field40;
	_field44 = field44;

	ListItem::load(file);
}

} // End of namespace Titanic
