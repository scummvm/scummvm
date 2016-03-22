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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_val.h"

namespace Titanic {

void CPetVal::proc1() {
	error("TODO");
}

void CPetVal::proc2() {
	error("TODO");
}

void CPetVal::proc3() {
	error("TODO");
}

void CPetVal::proc4() {
	error("TODO");
}

void CPetVal::proc5(Rect *rect) {
	error("TODO");
}

int CPetVal::proc16() {
	switch (_field14) {
	case 0:
		return _field18;
	case 1:
		return _field1C;
	case 2:
		return _field20;
	default:
		return 0;
	}
}

} // End of namespace Titanic
