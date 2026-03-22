/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "access/martian/martian_inventory.h"
#include "access/access.h"
#include "access/resources.h"

namespace Access {

namespace Martian {

MartianInventory::MartianInventory(AccessEngine *vm) : InventoryManager(vm) {
}

int MartianInventory::displayInv() {
	size_t invSize = _vm->_res->INVENTORY.size();

	Common::Array<byte> invFlags(invSize + 1);
	Common::Array<const char *> invNames(invSize + 1);

	// Only show items that are in the inventory, skip "used".
	for (size_t i = 0; i < invSize; i++) {
		byte flag = (_inv[i]._value == ITEM_IN_INVENTORY) ? 1 : 0;
		invFlags[i] = flag;
		invNames[i] = _inv[i]._name.c_str();
	}
	_vm->_events->forceSetCursor(CURSOR_CROSSHAIRS);
	_vm->_invBox->getList(invNames.data(), invFlags.data());

	int btnSelected = 0;
	int boxX = _vm->_invBox->doBox_v1(_startInvItem, _startInvBox, btnSelected);
	_startInvItem = _vm->_boxDataStart;
	_startInvBox = _vm->_boxSelectY;

	if (boxX == -1)
		btnSelected = 2;

	if (btnSelected != 2)
		setUseItem(_vm->_invBox->_tempListIdx[boxX]);
	else
		setUseItem(-1);

	return 0;
}

} // end namespace Martian

} // end namespace Access
