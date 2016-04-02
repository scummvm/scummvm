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
	_field18(0), _field30(-1), _field34(0), _field38(-1),
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

void CPetControlSub12::setup() {
	for (int idx = 0; idx < (int)_array.size(); ++idx) {
		_array[idx]._string1.clear();
		setArrayStr2(idx, _field54, _field58, _field5C);
		_array[idx]._string3.clear();
	}

	_field34 = 0;
	_field18 = 0;
}

void CPetControlSub12::setArrayStr2(uint idx, int val1, int val2, int val3) {
	char buffer[6];
	if (!val1)
		val1 = 1;
	if (!val2)
		val2 = 1;
	if (!val3)
		val3 = 1;

	buffer[0] = 27;
	buffer[1] = val1;
	buffer[2] = val2;
	buffer[3] = val3;
	buffer[4] = 27;
	buffer[5] = '\0';
	_array[idx]._string2 = buffer;
}

void CPetControlSub12::load(SimpleFile *file, int param) {
	if (!param) {
		int var1 = file->readNumber();
		int var2 = file->readNumber();
		uint count = file->readNumber();
		_bounds.left = file->readNumber();
		_bounds.top = file->readNumber();
		_bounds.right = file->readNumber();
		_bounds.bottom = file->readNumber();
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

void CPetControlSub12::draw(CScreenManager *screenManager) {
	warning("TODO: CPetControlSub12::draw");
}

} // End of namespace Titanic
