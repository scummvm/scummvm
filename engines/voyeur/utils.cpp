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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/utils.h"
#include "common/savefile.h"

namespace Voyeur {

LockTime::LockTime() { 
	field0 = field1 = field2 = field3 = 0; 
}

void LockClass::getSysDate() {
	// May not be needed for ScummVM
}

void LockClass::getThePassword() {
	field0 = 26;
	_password = "3333";
	fieldE = field16;
	field12 = field1A;
	fieldC = -1;

	// TODO: Original loaded 'VOYEUR.DAT' here to get most recent game's password.
	// but since we allow seperate savegames in ScummVM, this is somewhat deprecated.
}

void LockClass::saveThePassword() {
	// May not be needed for ScummVM
}

Common::String LockClass::getDateString() {
	return Common::String("ScummVM");
}

} // End of namespace Voyeur
