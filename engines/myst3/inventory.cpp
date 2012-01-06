/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/inventory.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Inventory::Inventory(Myst3Engine *vm) :
	_vm(vm){
}

Inventory::~Inventory() {
}

void Inventory::reset() {
	_inventory.clear();
}

void Inventory::addItem(uint16 var, bool atEnd) {
	// Only add objects once to the inventory
	if (Common::find(_inventory.begin(), _inventory.end(), var) == _inventory.end()) {
		_vm->_vars->set(var, 1);

		if (atEnd) {
			_inventory.push_back(var);
		} else {
			_inventory.push_front(var);
		}
	}
}

void Inventory::removeItem(uint16 var) {
	_vm->_vars->set(var, 0);
	_inventory.remove(var);
}

} /* namespace Myst3 */
