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

#include "titanic/pet_control/pet_rooms_glyphs.h"

namespace Titanic {

CPetRoomsGlyph::CPetRoomsGlyph() : CPetGlyph(),
	_field34(0), _field38(0), _field3C(0), _field40(0),
	_field44(0), _field48(0), _field4C(0), _field50(0),
	_field54(0), _field58(0), _field5C(0) {
}

void CPetRoomsGlyph::set34(int val) {
	_field34 = val;
}

void CPetRoomsGlyph::set38(int val) {
	_field38 = val;
}

} // End of namespace Titanic
