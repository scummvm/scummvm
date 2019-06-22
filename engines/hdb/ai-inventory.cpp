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

#include "hdb/hdb.h"

namespace HDB {

// Add entity to Player's Inventory
bool AI::addToInventory(AIEntity *e) {
	switch (e->type) {
	case ITEM_GEM_WHITE:
		_numGems++;
		removeEntity(e);
		return true;
	case ITEM_GEM_BLUE:
		_numGems += 5;
		removeEntity(e);
		return true;
	case ITEM_GEM_RED:
		_numGems += 10;
		removeEntity(e);
		return true;
	case ITEM_GEM_GREEN:
		_numGems += 100;
		removeEntity(e);
		return true;
	case ITEM_GOO_CUP:
		_numGooCups++;
		removeEntity(e);
		return true;
	case ITEM_MONKEYSTONE:
		_numMonkeystones++;
		removeEntity(e);
		return true;
	default:
		warning("AI-INVENTORY: addToInventory: Unintended Type");
		return false;
	}

	if (_numInventory >= kMaxInventory) {
		warning("STUB: addToInventory: Inventory full message");
		return false;
	}

	_inventory[_numInventory].ent = e;
	_numInventory++;

	// If weapon, ready it
	switch (e->type) {
	case ITEM_CLUB:
	case ITEM_ROBOSTUNNER:
	case ITEM_SLUGSLINGER:
		warning("STUB: addToInventory: Ready weapon");
	default:
		warning("AI-INVENTORY: addToInventory: Unintended Type");
	}

	warning("STUB: addToInventory: Print a 'You Got' message");
	removeEntity(e);
	return true;
}

// Clear out the Player inventory except Gems,
// Monkeystones and Goo Cups unless its marked
void AI::clearInventory() {
	int keepslot = 0;
	for (int i = 0; i < _numInventory; i++) {
		if (!_inventory[i].keep) {
			memset(&_inventory[i], 0, sizeof(InvEnt));
		} else {
			if (i != keepslot) {
				_inventory[keepslot] = _inventory[i];
				_inventory[keepslot].ent = _inventory[i].ent;
				_inventory[keepslot].keep = _inventory[i].keep;
				memset(&_inventory[i], 0, sizeof(InvEnt));
			}
			keepslot++;
		}
	}
	_numInventory = keepslot;
}
} // End of Namespace
