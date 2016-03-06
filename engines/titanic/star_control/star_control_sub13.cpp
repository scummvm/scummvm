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

#include "titanic/star_control/star_control_sub13.h"

namespace Titanic {

CStarControlSub13::CStarControlSub13(void *ptr):
		_field0(0), _field4(0), _field8(0), _fieldC0(0),
		_fieldC4(0), _fieldC8(0), _fieldCC(0), _fieldD0(0) {
	if (ptr) {
		setup(ptr);
	} else {
		_fieldC = 0;
		_field10 = 0x44480000;
		_field14 = 0x461C4000;
		_field18 = 0x41A00000;
		_field1C = 0x41A00000;
		_field20 = 600;
		_field22 = 340;
		_field24 = 0;
	}

	_fieldD4 = 0;
}

void CStarControlSub13::setup(void *ptr) {
	// TODO
}

void CStarControlSub13::load(SimpleFile *file, int param) {
	_field0 = file->readFloat();
	_field4 = file->readFloat();
	_field8 = file->readFloat();
	_fieldC = file->readFloat();
	_field10 = file->readFloat();
	_field14 = file->readFloat();
	_field18 = file->readFloat();
	_field1C = file->readFloat();
	_field20 = file->readNumber();
	_field22 = _field20 >> 16;
	_field24 = file->readNumber();
	
	for (int idx = 0; idx < 5; ++idx)
		_valArray[idx] = file->readFloat();

	_sub14.load(file, param);
	_fieldD4 = 0;
}

void CStarControlSub13::save(SimpleFile *file, int indent) const {
	_sub14.save(file, indent);
}


} // End of namespace Titanic
