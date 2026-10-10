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

#include "ultima/ultima3/data/savegame.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr byte TRANSPORT_ON_FOOT = 0x3F;
constexpr byte START_X = 44;
constexpr byte START_Y = 20;

bool Savegame::hasLivingPartyMember() {
	for (int i = 0; i < _partySize; ++i) {
		if (partyMember(i).isAlive())
			return true;
	}

	return false;
}

void Savegame::addToParty(int number) {
	entry(number)._partyMember = IN_PARTY;
	_partyEntries[_partySize++] = number;
}

void Savegame::clearPartySelection() {
	while (_partySize > 0) {
		--_partySize;
		partyMember(_partySize)._partyMember = 0;
		_partyEntries[_partySize] = 0;
	}
}

void Savegame::synchronize(Common::Serializer &s) {
	for (int i = 0; i < ROSTER_COUNT; ++i)
		_roster[i].synchronize(s);

	byte partySize = _partySize;
	s.syncBytes(_partyEntries, PARTY_MAX);
	s.syncAsByte(partySize);
	_partySize = partySize;

	s.syncAsByte(_transport);
	s.syncAsByte(_location);
	s.syncAsByte(_posX);
	s.syncAsByte(_posY);
	s.syncBytes(_moveCount, sizeof(_moveCount));
	s.syncAsByte(_mapLoaded);

	if (s.getVersion() >= 2) {
		s.syncAsByte(_allWeapons);
		s.syncAsByte(_allArmour);
		s.syncAsByte(_plusTwoWeapons);
		s.syncAsByte(_plusTwoArmour);
	}

	if (s.getVersion() >= 3) {
		s.syncAsByte(_worldX);
		s.syncAsByte(_worldY);
	}

	if (s.getVersion() >= 4)
		s.syncAsByte(_lightTurns);

	if (s.getVersion() >= 5) {
		s.syncAsByte(_dungeonLevel);
		s.syncAsByte(_facing);
	}
}

constexpr int ORIGINAL_ROSTER_SIZE = ROSTER_COUNT * 0x40;
constexpr int ORIGINAL_PARTY_SIZE = 0x12 + PARTY_MAX * 0x40;
constexpr byte FLAG_SET = 0xFF;

bool Savegame::importOriginal(Common::SeekableReadStream &roster, Common::SeekableReadStream &party) {
	if (roster.size() < ORIGINAL_ROSTER_SIZE || party.size() < ORIGINAL_PARTY_SIZE)
		return false;

	Common::Serializer rosterSer(&roster, nullptr);
	for (int i = 0; i < ROSTER_COUNT; ++i)
		_roster[i].synchronize(rosterSer, true);

	Common::Serializer ser(&party, nullptr);
	byte location, size, x, y, flags[4];
	ser.syncAsByte(_transport);
	ser.skip(1);
	ser.syncAsByte(location);
	ser.syncBytes(_moveCount, sizeof(_moveCount));
	ser.syncAsByte(size);
	ser.syncAsByte(x);
	ser.syncAsByte(y);
	ser.syncBytes(_partyEntries, PARTY_MAX);
	ser.syncBytes(flags, sizeof(flags));

	// The party's own copies of the characters are the up to date ones
	RosterEntry members[PARTY_MAX];
	for (int i = 0; i < PARTY_MAX; ++i)
		members[i].synchronize(ser, true);

	_partySize = MIN<int>(size, PARTY_MAX);
	for (int i = 0; i < _partySize; ++i) {
		if (_partyEntries[i] >= 1 && _partyEntries[i] <= ROSTER_COUNT)
			entry(_partyEntries[i]) = members[i];
	}

	// A place other than the world can't be returned to, so the party is placed back on it
	_allWeapons = flags[0] == FLAG_SET;
	_allArmour = flags[1] == FLAG_SET;
	_plusTwoWeapons = flags[2] == FLAG_SET;
	_plusTwoArmour = flags[3] == FLAG_SET;
	_location = LOCATION_SOSARIA;
	_posX = _worldX = x;
	_posY = _worldY = y;
	_dungeonLevel = 0;
	_lightTurns = 0;
	return true;
}

void Savegame::exportOriginal(Common::WriteStream &roster, Common::WriteStream &party) {
	Common::Serializer rosterSer(nullptr, &roster);
	for (int i = 0; i < ROSTER_COUNT; ++i)
		_roster[i].synchronize(rosterSer, true);

	// Where the party is elsewhere can't be put in the file, so it has the place they entered from
	Common::Serializer ser(nullptr, &party);
	byte location = LOCATION_SOSARIA, size = _partySize;
	byte x = (_location == LOCATION_SOSARIA) ? _posX : _worldX;
	byte y = (_location == LOCATION_SOSARIA) ? _posY : _worldY;
	byte flags[4] = {
		_allWeapons ? FLAG_SET : (byte)0, _allArmour ? FLAG_SET : (byte)0,
		_plusTwoWeapons ? FLAG_SET : (byte)0, _plusTwoArmour ? FLAG_SET : (byte)0
	};

	ser.syncAsByte(_transport);
	ser.skip(1);
	ser.syncAsByte(location);
	ser.syncBytes(_moveCount, sizeof(_moveCount));
	ser.syncAsByte(size);
	ser.syncAsByte(x);
	ser.syncAsByte(y);
	ser.syncBytes(_partyEntries, PARTY_MAX);
	ser.syncBytes(flags, sizeof(flags));

	for (int i = 0; i < PARTY_MAX; ++i) {
		RosterEntry blank;
		(i < _partySize ? partyMember(i) : blank).synchronize(ser, true);
	}
}

void Savegame::setupDummyParty() {
	static const struct {
		const char *_name;
		char _sex, _race, _class;
	} CHARACTERS[PARTY_MAX] = {
		{ "Anna", 'F', 'H', 'F' }, { "Boris", 'M', 'D', 'C' },
		{ "Cora", 'F', 'E', 'W' }, { "Dax", 'M', 'B', 'T' }
	};

	clearPartySelection();
	for (int i = 0; i < PARTY_MAX; ++i) {
		RosterEntry &e = _roster[i];
		e.clear();

		Common::strlcpy(e._name, CHARACTERS[i]._name, sizeof(e._name));
		e._sex = CHARACTERS[i]._sex;
		e._race = CHARACTERS[i]._race;
		e._class = CHARACTERS[i]._class;
		e._status = STATUS_GOOD;
		e._strength = e._dexterity = e._intelligence = e._wisdom = toBcd(25);
		e._magicPoints = toBcd(25);
		e._hitPoints = e._maxHitPoints = toBcdWord(150);
		e._food = e._gold = toBcdWord(200);
		e._torches = toBcd(5);
		e._weaponOwned[0] = 1;
		e._armourOwned[0] = 1;
		addToParty(i + 1);
	}

	formParty();
}

void Savegame::formParty() {
	_transport = TRANSPORT_ON_FOOT;
	_location = LOCATION_SOSARIA;
	_posX = START_X;
	_posY = START_Y;
	_dungeonLevel = 0;
	_lightTurns = 0;
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
