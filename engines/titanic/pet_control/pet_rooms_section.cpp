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

#include "titanic/pet_control/pet_rooms_section.h"

namespace Titanic {

CPetRoomsSection::CPetRoomsSection() :
	_field100(0), _field104(0), _field108(0), _field10C(0),
	_field110(0), _field114(0), _field118(0), _field11C(0),
	_field1C0(0), _field1C4(0), _field1C8(0), _field1CC(0),
	_field1D0(0), _field1D4(0) {
}

void CPetRoomsSection::save(SimpleFile *file, int indent) const {

}

void CPetRoomsSection::load(SimpleFile *file, int param) {
	if (!param) {
		int count = file->readNumber();

		for (int idx = 0; idx < count; ++idx) {
			int v1 = file->readNumber();
			int v2 = file->readNumber();
			warning("TODO: CPetRoomsSection::load - %d,%d", v1, v2);
		}

		_listItem.setField34(file->readNumber());
		file->readNumber();
		_field1C0 = file->readNumber();
		_field1C4 = file->readNumber();
		_field1C8 = file->readNumber();
		_field1CC = file->readNumber();
		_field1D0 = file->readNumber();
		_field1D4 = file->readNumber();
	}
}

bool CPetRoomsSection::isValid(CPetControl *petControl) {
	// TODO
	return true;
}


} // End of namespace Titanic
