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
		break;
	}

	if (_numInventory >= kMaxInventory) {
		g_hdb->_window->openMessageBar("Your inventory is full.", 5);
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

	printYouGotMsg(e->printedName);
	removeEntity(e);
	return true;
}

void AI::purgeInventory() {
	memset(&_inventory, 0, sizeof(InvEnt) * kMaxInventory);
	_numInventory = 0;
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

AIEntity *AI::getInvItem(int which) {
	if (which >= _numInventory)
		return NULL;
	return _inventory[which].ent;
}

int AI::queryInventory(const char *string) {
	int		i, count;

	if (!scumm_stricmp(string, "monkeystone"))
		return getMonkeystoneAmount();
	if (!scumm_stricmp(string, "goo"))
		return getGooCupAmount();
	if (!scumm_stricmp(string, "gem"))
		return getGemAmount();

	if (!_numInventory)
		return 0;

	count = 0;
	for (i = _numInventory - 1; i >= 0; i--)
		if (_inventory[i].ent->entityName && strstr(_inventory[i].ent->entityName, string))
			count++;

	return count;
}

bool AI::removeInvItem(const char *string, int amount) {
	int		i, j;
	int		found;

	// Check specially for Gems, Monkeystones and Goo Cups
	if (!scumm_stricmp(string, "gem")) {
		_numGems -= amount;
		return true;
	} else if (!scumm_stricmp(string, "monkeystone")) {
		_numMonkeystones -= amount;
		return true;
	} else if (!scumm_stricmp(string, "goo")) {
		_numGooCups -= amount;
		return true;
	}

	if (!_numInventory)
		return false;

	do {
		found = 0;

		for (i = _numInventory - 1; i >= 0; i--)
			if (_inventory[i].ent->entityName && strstr(_inventory[i].ent->entityName, string)) {
				j = i;
				memset(&_inventory[j], 0, sizeof(InvEnt));
				while (j < _numInventory - 1) {
					memcpy(&_inventory[j], &_inventory[j + 1], sizeof(InvEnt));
					memset(&_inventory[j + 1], 0, sizeof(InvEnt));
					j++;
				}
				_numInventory--;
				amount--;
				found = 1;
				if (!amount)
					break;
			}
	} while (found && amount);

	// if we haven't removed them all, return false
	if (amount)
		return false;

	return true;
}

int AI::queryInventoryType(AIType which) {
	int		i, count;

	if (which == ITEM_MONKEYSTONE)
		return getMonkeystoneAmount();
	if (which == ITEM_GOO_CUP)
		return getGooCupAmount();
	if (which == ITEM_GEM_WHITE)
		return getGemAmount();

	if (!_numInventory)
		return 0;

	count = 0;
	for (i = 0; i < _numInventory; i++)
		if (_inventory[i].ent->type == which)
			count++;

	return count;
}

bool AI::removeInvItemType(AIType which, int amount) {
	int		i, j, found;

	// Check specially for Gems, Monkeystones and Goo Cups
	if (which == ITEM_GEM_WHITE) {
		_numGems -= amount;
		return true;
	} else if (which == ITEM_MONKEYSTONE) {
		_numMonkeystones -= amount;
		return true;
	} else if (which == ITEM_GOO_CUP) {
		_numGooCups -= amount;
		return true;
	}

	if (!_numInventory)
		return false;

	do {
		found = 0;

		for (i = 0; i < _numInventory; i++)
			if (_inventory[i].ent->type == which) {
				j = i;
				memset(&_inventory[j], 0, sizeof(InvEnt));
				while (j < _numInventory - 1) {
					memcpy(&_inventory[j], &_inventory[j + 1], sizeof(InvEnt));
					memset(&_inventory[j + 1], 0, sizeof(InvEnt));
					j++;
				}
				_numInventory--;
				amount--;
				found = 1;
				if (!amount)
					break;
			}
	} while (found && amount);

	// if we haven't removed them all, return false
	if (amount)
		return false;

	return true;
}

bool AI::addItemToInventory(AIType type, int amount, const char *funcInit, const char *funcAction, const char *funcUse) {
	int		i;
	AIEntity *e;
	for (i = 0; i < amount; i++) {
		spawn(type, DIR_UP, 0, 0, funcInit, funcAction, funcUse, DIR_UP, 1, 0, 0, 1);
		e = findEntity(0, 0);
		if (!e)
			return false;
		if (!addToInventory(e))
			return false;
	}
	return true;
}

void AI::keepInvItem(AIType type) {
	for (int i = 0; i < _numInventory; i++)
		if (_inventory[i].ent->type == type)
			_inventory[i].keep = 1;
}

void AI::printYouGotMsg(const char *name) {
	if (!name || !name[0])
		return;

	sprintf(_youGotBuffer, "Got %s", name);
	g_hdb->_window->textOut(_youGotBuffer, kYouGotX, kYouGotY, 120);
}

void AI::newDelivery(const char *itemTextName, const char *itemGfxName, const char *destTextName, const char *destGfxName, const char *id) {
	int i = _numDeliveries;

	if (i == kMaxDeliveries) {
		g_hdb->_window->openMessageBar("You have too many deliveries already!", 3);
		return;
	}

	if (itemTextName)
		strcpy(_deliveries[i].itemTextName, itemTextName);
	if (itemGfxName)
		strcpy(_deliveries[i].itemGfxName, itemGfxName);
	if (destTextName)
		strcpy(_deliveries[i].destTextName, destTextName);
	if (destGfxName)
		strcpy(_deliveries[i].destGfxName, destGfxName);

	strcpy(_deliveries[i].id, id);

	_numDeliveries++;

	g_hdb->_window->openDeliveries(true);
}

bool AI::completeDelivery(const char *id) {
	for (int i = 0; i < _numDeliveries; i++)
		if (!scumm_stricmp(_deliveries[i].id, id)) {
			for (; i < _numDeliveries; i++)
				memcpy(&_deliveries[i], &_deliveries[i + 1], sizeof(_deliveries[0]));
			_numDeliveries--;
			warning("STUB: Play Voice: GUY_COMPLETED");
			return true;
		}
	return false;
}

} // End of Namespace
