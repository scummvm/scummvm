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

CStarControlSub13::CStarControlSub13(void *src) :
		_field0(0), _field4(0), _field8(0), _fieldC0(0),
		_fieldC4(0), _fieldC8(0), _fieldCC(0), _fieldD0(0) {
	if (src) {
		setup(src);
	} else {
		_fieldC = 0;
		_field10 = 0x44480000;
		_field14 = 0x461C4000;
		_field18 = 0x41A00000;
		_field1C = 0x41A00000;
		_width = 600;
		_height = 340;
		_field24 = 0;
	}

	_fieldD4 = 0;
}

CStarControlSub13::CStarControlSub13(CStarControlSub13 *src) :
		_matrix(&src->_matrix), _sub1(&src->_sub1), _sub2(&src->_sub2) {
	_field0 = src->_field0;
	_field4 = src->_field4;
	_field8 = src->_field8;
	_fieldC = src->_fieldC;
	_field10 = src->_field10;
	_field14 = src->_field14;
	_field18 = src->_field18;
	_field1C = src->_field1C;
	_width = src->_width;
	_height = src->_height;

	_fieldCC = src->_fieldCC;
	_fieldD0 = src->_fieldD0;
	_fieldC0 = src->_fieldC0;
	_fieldC4 = src->_fieldC4;
	_fieldC8 = src->_fieldC8;
	_field24 = src->_field24;

	_valArray[0] = src->_valArray[0];
	_valArray[2] = src->_valArray[2];
	_valArray[3] = src->_valArray[3];
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
	
	int widthHeight = file->readNumber();
	_width = widthHeight & 0xff;
	_height = _width >> 16;
	_field24 = file->readNumber();
	
	for (int idx = 0; idx < 5; ++idx)
		_valArray[idx] = file->readFloat();

	_matrix.load(file, param);
	_fieldD4 = 0;
}

void CStarControlSub13::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_field0, indent);
	file->writeFloatLine(_field4, indent);
	file->writeFloatLine(_field8, indent);
	file->writeFloatLine(_fieldC, indent);
	file->writeFloatLine(_field10, indent);
	file->writeFloatLine(_field14, indent);
	file->writeFloatLine(_field18, indent);
	file->writeFloatLine(_field1C, indent);
	file->writeFloatLine(_width | (_height << 16), indent);

	for (int idx = 0; idx < 5; ++idx)
		file->writeFloatLine(_valArray[idx], indent);

	_matrix.save(file, indent);
}


} // End of namespace Titanic
