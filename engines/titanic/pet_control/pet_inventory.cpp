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

#include "titanic/pet_control/pet_inventory.h"

namespace Titanic {

CPetInventory::CPetInventory() : CPetSection(),
		_field28C(0), _field290(0), _field294(0), _field298(0) {
	for (int idx = 0; idx < 46; ++idx) {
		_valArray1[idx] = _valArray2[idx] = 0;
	}
}

void CPetInventory::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(_field298, indent);
}

void CPetInventory::load(SimpleFile *file, int param) {
	_field298 = file->readNumber();
}

bool CPetInventory::isValid(CPetControl *petControl) {
	// TODO
	return true;
}

} // End of namespace Titanic
