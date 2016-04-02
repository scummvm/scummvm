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
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetInventory::CPetInventory() : CPetSection(),
		_field28C(0), _field290(0), _field294(0), _field298(0) {
	for (int idx = 0; idx < TOTAL_ITEMS; ++idx) {
		_itemBackgrounds[idx] = _itemGlyphs[idx] = nullptr;
	}
}

bool CPetInventory::setup(CPetControl *petControl) {
	return setPetControl(petControl) && setup();
}

bool CPetInventory::setup() {
	_sub10.setup();
	_sub12.setup();

	// TODO
	return true;
}

bool CPetInventory::setPetControl(CPetControl *petControl) {
	if (!petControl)
		return false;

	_petControl = petControl;
	_sub10.proc8();
	_sub10.set20(28);

	Rect tempRect(0, 0, 52, 52);
	for (uint idx = 0; idx < TOTAL_ITEMS; ++idx) {
		if (!g_vm->_itemNames[idx].empty()) {
			CString name = "3Pet" + g_vm->_itemNames[idx];
			_itemBackgrounds[idx] = petControl->getHiddenObject(name);
		}

		if (!g_vm->_itemObjects[idx].empty()) {
			_itemGlyphs[idx] = petControl->getHiddenObject(g_vm->_itemObjects[idx]);
		}
	}

	tempRect = Rect(0, 0, 580, 15);
	tempRect.translate(32, 445);
	_sub12.setBounds(tempRect);
	_sub12.set70(0);
	
	return true;
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
