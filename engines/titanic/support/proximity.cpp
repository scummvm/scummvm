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

#include "titanic/support/proximity.h"
#include "titanic/true_talk/tt_talker.h"

namespace Titanic {

CProximity::CProximity() : _field4(0), _field8(100), _fieldC(0),
		_speechHandle(-1), _field14(0), _field18(0), _field1C(0x3FF00000),
		_field20(0), _field24(10), _field28(0), _field2C(0),
		_field30(0x3F000000), _field34(0), _double1(0.0), _double2(0.0),
		_double3(0.0), _field44(0), _field48(0), _field4C(0),
		_field50(0), _field54(0), _field58(0), _field5C(0),
		_field60(0), _method1(nullptr), _talker(nullptr), _field6C(0) {
}

} // End of namespace Titanic
