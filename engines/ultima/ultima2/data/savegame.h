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

#ifndef ULTIMA2_DATA_SAVEGAME_H
#define ULTIMA2_DATA_SAVEGAME_H

#include "common/serializer.h"
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

/**
 * The player character's savegame data
 */
struct Savegame {
	// Identity
	char _name[MAX_NAME_LENGTH + 1] = {};
	Sex _sex = SEX_MALE;
	Race _race = RACE_HUMAN;
	CharClass _class = CLASS_FIGHTER;

	// Attributes
	int16 _strength = 0;
	int16 _agility = 0;
	int16 _stamina = 0;
	int16 _charisma = 0;
	int16 _wisdom = 0;
	int16 _intelligence = 0;

	// Resources
	int16 _hp = 0;
	int16 _food = 0;
	int16 _foodTurnCtr = 0;
	int16 _experience = 0;
	int16 _gold = 0;

	// Position - _mapEra/_mapType select the current mapX??/monX??/tlkX???
	// file; _mapX/_mapY are the player's position within it
	byte _mapEra = 0;
	byte _mapType = 0;
	byte _mapX = 0;
	byte _mapY = 0;

	// Overworld position to restore to when walking off the edge of a
	// village/town/castle/tower/dungeon map back to the surface
	byte _overworldReturnX = 0;
	byte _overworldReturnY = 0;

	// Tile of the vehicle the player is riding (horse, ship, ...), or 0 on foot
	byte _mount = 0;

	// Currently readied items
	WeaponType _readiedWeapon = WEAPON_HANDS;
	ArmorType _readiedArmor = ARMOR_SKIN;
	SpellType _readiedSpell = SPELL_NONE;

	// Consumable resources dropped by monster kills
	int16 _torches = 0;
	int16 _keys = 0;
	int16 _thievesTools = 0;

	// Outer space
	bool _inSpace = false;
	byte _launchMapX = 0;
	byte _launchMapY = 0;

	bool _saveDisabled = false;
	// -1 when not orbiting/in deep space; 0-8 selects a planet being
	// orbited; 9 gates a one-off castle NPC interaction
	int16 _orbitTarget = -1;

	bool _ringQuestFlag = false;
	bool _enilnoOwned = false;
	byte _patrolWaypoint = 0;

	// Turns until the moongate hops to its next waypoint, and the terrain
	// tile it's currently covering
	byte _patrolTimer = 0;
	byte _patrolTerrain = 0;

	byte _offerRewardItems[9] = {};

	// Inventory quantities/charges, indexed by the enums in data.h
	int16 _armorOwned[ARMOR_COUNT] = {};
	int16 _weaponOwned[WEAPON_COUNT] = {};
	int16 _spellCharges[SPELL_COUNT] = {};
	int16 _items[ITEM_COUNT] = {};

	// Turn-based status counters, decremented once per turn in Logic::endOfTurn
	int16 _legParalysisTurns = 0;
	int16 _armParalysisTurns = 0;
	int16 _sleepTurns = 0;
	int16 _negateTimeTurns = 0;
	// Torch/Light spell duration; only decremented by the (not yet implemented)
	// dungeon renderer
	int16 _lightTurns = 0;

	// Incremented on every overworld monster spawn attempt; its lowest set
	// bit selects the monster type rotation in OverworldLogic::trySpawnMonster
	byte _monsterSpawnCounter = 0;

	/**
	 * Synchronize savegame data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns true if a character has been created or loaded
	 */
	bool hasCharacter() const {
		return _name[0] != '\0';
	}

	/**
	 * Replaces the savegame with a default character on the starting
	 * overworld, for testing without going through character creation
	 */
	void setupDummyCharacter();

	/**
	 * Returns the tile the player is drawn with
	 */
	TileId playerTile() const {
		return _mount ? (TileId)_mount : (TileId)(TILE_FIGHTER + _class);
	}

	/**
	 * Deducts an amount from HP. Returns false if this was fatal.
	 * A survived hit costs one extra point beyond the amount given,
	 * matching the original's borrow-based HP subtraction.
	 */
	bool deductHP(int amount) {
		if (_hp <= amount) {
			_hp = 0;
			return false;
		}

		_hp -= amount + 1;
		return true;
	}

	/**
	 * Deducts an amount from food (stored as whole units in _food plus
	 * hundredths in _foodTurnCtr). Returns false if food ran out.
	 */
	bool deductFood(int amount) {
		int total = _food * 100 + _foodTurnCtr - amount;
		if (total < 0)
			return false;

		_food = total / 100;
		_foodTurnCtr = total % 100;
		return true;
	}

	/**
	 * Computes a shop item's price: a Fibonacci-style ramp (each step adds
	 * the previous two prices, starting from 4 and 4) whose length grows
	 * with the item index and shrinks by the bit length of the player's
	 * combined Intelligence+Charisma (a haggling mechanic)
	 */
	int computeItemPrice(int itemIndex) const {
		int sum = _intelligence + _charisma;
		int bits = 0;
		while (sum > 0) {
			sum >>= 1;
			++bits;
		}

		int steps = itemIndex + 8 - bits;
		int prev = 4, price = 4;
		for (int i = 0; i < steps; ++i) {
			int next = (prev + price) % 10000;
			prev = price;
			price = next;
		}
		return price;
	}
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
