/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software(0), you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation(0), either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY(0), without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program(0), if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/pet_control/pet_control_sub12.h"

namespace Titanic {

CPetControlSub12::CPetControlSub12(int count) :
	_field18(0), _field1C(0), _field20(0), _field24(0),
	_field28(0), _field30(-1), _field34(0), _field38(-1),
	_field3C(0), _field40(0), _field44(0), _field48(0xff),
	_field4C(0xff), _field50(0xff), _field54(0), _field58(0),
	_field5C(200), _field60(0), _field64(0), _field68(0),
	_field6C(0), _field70(1), _field74(0), _field78(0),
	_field7C(0) {
	setupArrays(count);
}

void CPetControlSub12::setupArrays(int count) {
	freeArrays();
	if (count < 10 || count > 60)
		count = 10;
	_array.resize(count);
}

void CPetControlSub12::freeArrays() {
	_array.clear();
}

void CPetControlSub12::load(SimpleFile *file, int param) {
	if (!param) {
		int var1 = file->readNumber();
		int var2 = file->readNumber();
		uint count = file->readNumber();
		_field1C = file->readNumber();
		_field20 = file->readNumber();
		_field24 = file->readNumber();
		_field28 = file->readNumber();
		_field3C = file->readNumber();
		_field40 = file->readNumber();
		_field44 = file->readNumber();
		_field48 = file->readNumber();
		_field4C = file->readNumber();
		_field50 = file->readNumber();
		_field54 = file->readNumber();
		_field58 = file->readNumber();
		_field5C = file->readNumber();
		_field70 = file->readNumber();
		_field74 = file->readNumber();

		warning("TODO: CPetControlSub12::load %d,%d", var1, var2);
		assert(_array.size() >= count);
		for (uint idx = 0; idx < count; ++idx) {
			_array[idx]._string1 = file->readString();
			_array[idx]._string2 = file->readString();
			_array[idx]._string3 = file->readString();
		}	
	}
}


} // End of namespace Titanic
