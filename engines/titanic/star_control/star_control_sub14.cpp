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

#include "titanic/star_control/star_control_sub14.h"

namespace Titanic {

CStarControlSub14::CStarControlSub14() :
	_field0(0x3F800000), _field4(0), _field8(0), _fieldC(0),
	_field10(0x3F800000), _field14(0), _field18(0), _field1C(0),
	_field20(0x3F800000) {
}

void CStarControlSub14::load(SimpleFile *file, int param) {
	_field0 = file->readFloat();
	_field4 = file->readFloat();
	_field8 = file->readFloat();
	_fieldC = file->readFloat();
	_field10 = file->readFloat();
	_field14 = file->readFloat();
	_field18 = file->readFloat();
	_field1C = file->readFloat();
	_field20 = file->readFloat();
}

void CStarControlSub14::save(SimpleFile *file, int indent) const {

}


} // End of namespace Titanic
