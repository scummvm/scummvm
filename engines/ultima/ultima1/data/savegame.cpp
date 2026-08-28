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

#include "ultima/ultima1/data/savegame.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *SEX_NAMES[] = { "Male", "Female" };
const char *RACE_NAMES[] = { nullptr, "Human", "Elf", "Dwarf", "Bobbit" };
const char *CLASS_NAMES[] = { nullptr, "Fighter", "Cleric", "Wizard", "Thief" };

void LocationEntity::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_type);
	s.syncAsSint16LE(_position.x);
	s.syncAsSint16LE(_position.y);
	s.syncAsSint16LE(_hitPoints);
}

void Savegame::synchronizeBasic(Common::Serializer &s) {
	s.syncBytes((byte *)_name, MAX_NAME_LENGTH + 2);
	s.syncAsByte(_race);
	s.syncAsByte(_class);
	s.syncAsByte(_sex);

	s.syncAsSint16LE(_hits);
	s.syncAsSint16LE(_strength);
	s.syncAsSint16LE(_agility);
	s.syncAsSint16LE(_stamina);
	s.syncAsSint16LE(_charisma);
	s.syncAsSint16LE(_wisdom);
	s.syncAsSint16LE(_intelligence);

	s.syncAsSint16LE(_coins);
	s.syncAsSint16LE(_experience);
	s.syncAsSint16LE(_food);

	s.syncAsSint16LE(_equippedWeapon);
	s.syncAsSint16LE(_equippedSpell);
	s.syncAsSint16LE(_equippedArmor);
	s.syncAsSint16LE(_transportType);

	s.syncAsSint16LE(_randomSeed);
	s.syncAsSint16LE(_overworldPos.x);
	s.syncAsSint16LE(_overworldPos.y);
	s.syncAsByte(_soundOn);

	for (int i = 0; i < QUEST_COUNT; ++i)
		s.syncAsSint16LE(_questStatus[i]);

	s.syncAsSint16LE(_redGems);
	s.syncAsSint16LE(_greenGems);
	s.syncAsSint16LE(_blueGem);
	s.syncAsSint16LE(_whiteGem);

	for (int i = 0; i < ARMOR_COUNT; ++i)
		s.syncAsSint16LE(_armor[i]);
	for (int i = 0; i < WEAPON_COUNT; ++i)
		s.syncAsSint16LE(_weapons[i]);
	for (int i = 0; i < SPELL_COUNT; ++i)
		s.syncAsSint16LE(_spells[i]);
	for (int i = 0; i < TRANSPORT_COUNT; ++i)
		s.syncAsSint16LE(_transports[i]);

	s.syncAsSint16LE(_enemyVessels);
	s.syncAsSint16LE(_signMarker);
	s.syncAsSint16LE(_overworldEntityCount);
	s.syncAsUint32LE(_moveCount);
	s.syncAsUint16LE(_shipFuel);
	s.syncAsUint16LE(_shipShield);
	for (int i = 0; i < OVERWORLD_ENTITY_COUNT; ++i)
		_overworldEntities[i].synchronize(s);
}

void Savegame::synchronizeExtra(Common::Serializer &s) {
	s.syncAsSint16LE(_mapNum);
	s.syncAsSint16LE(_locationPosition.x);
	s.syncAsSint16LE(_locationPosition.y);

	for (int i = 0; i < LOCATION_ENTITY_COUNT; ++i)
		_locationEntities[i].synchronize(s);
	s.syncAsSint16LE(_guardsHostile);
	s.syncAsSint16LE(_hasCastleKey);
	s.syncAsSint16LE(_freeingPrincess);
	s.syncAsSint16LE(_castleItemAllowance);
	s.syncAsSint16LE(_castleKeyVal);
}

void Savegame::synchronize(Common::Serializer &s) {
	synchronizeBasic(s);
	synchronizeExtra(s);
}

void Savegame::synchronizeOriginal(Common::Serializer &s) {
	synchronizeBasic(s);
	_mapNum = MAP_OVERWORLD;
}

int Savegame::mapTopLeftX() const {
	return (_G(map)._currentMap == MAP_OVERWORLD) ? _overworldPos.x - MAP_VISIBLE_CENTER_X : 0;
}

int Savegame::mapTopLeftY() const {
	return (_G(map)._currentMap == MAP_OVERWORLD) ? _overworldPos.y - MAP_VISIBLE_CENTER_Y : 0;
}

int Savegame::getEntityAt(int x, int y, int startingIndex) const {
	int foundIndex = -1;
	for (; startingIndex <= _G(overworldEntityCount) && foundIndex == -1; ++startingIndex) {
		const auto &e = _overworldEntities[startingIndex];
		if (e._x == x && e._y == y)
			foundIndex = startingIndex;
	}

	return foundIndex;
}

void Savegame::removeCreatureAt(int x, int y) {
	int creatureNum = getEntityAt(x, y, 1);
	if (creatureNum <= 0)
		return;

	// Shift all following entities down a slot, removing the found one
	for (int idx = creatureNum; idx < _G(overworldEntityCount); ++idx)
		_overworldEntities[idx] = _overworldEntities[idx + 1];

	--_G(overworldEntityCount);
}


} // namespace Data
} // namespace Ultima1
} // namespace Ultima
