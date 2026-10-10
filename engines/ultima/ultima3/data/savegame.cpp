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
