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
 * $URL$
 * $Id$
 *
 */

#include "scumm/scumm_v4.h"
#include "scumm/object.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v4, x)

void ScummEngine_v4::setupOpcodes() {
	ScummEngine_v5::setupOpcodes();

	OPCODE(0x25, o5_drawObject);
	OPCODE(0x45, o5_drawObject);
	OPCODE(0x65, o5_drawObject);
	OPCODE(0xa5, o5_drawObject);
	OPCODE(0xc5, o5_drawObject);
	OPCODE(0xe5, o5_drawObject);

	OPCODE(0x50, o4_pickupObject);
	OPCODE(0xd0, o4_pickupObject);
}

void ScummEngine_v4::o4_pickupObject() {
	int obj = getVarOrDirectWord(PARAM_1);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	// Don't take an object twice
		return;

	// debug(0, "adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

} // End of namespace Scumm
