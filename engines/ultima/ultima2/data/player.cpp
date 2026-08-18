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

#include "ultima/ultima2/data/player.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

const char *SEX[] = { "MALE", "FEMALE", "YES PLEASE" };
const char *RACE[] = { nullptr, "HUMAN", "ELF", "DWARF", "HOBBIT" };
const char *CLASS[] = { nullptr, "FIGHTER", "CLERIC", "WIZARD", "THIEF" };

void Player::synchronize(Common::Serializer &s) {
	// Sync name
	s.syncBytes((byte *)_name, MAX_NAME_LENGTH + 1);
	s.skip(16 - (MAX_NAME_LENGTH + 1));

	// Sex
	byte sex = (_sex == SEX_MALE) ? 'M' : 'F';
	s.syncAsByte(sex);
	if (s.isLoading())
		_sex = (sex == 'M') ? SEX_MALE : SEX_FEMALE;

	// Class and race
	s.syncAsByte(_class);
	s.syncAsByte(_race);

	// Map number is saved as two separate bytes
	byte map1 = _mapNum / 10, map2 = _mapNum % 10;
	s.syncAsByte(map1);
	s.syncAsByte(map2);
	if (s.isLoading())
		_mapNum = (map1 * 10) + map2;

	// Attributes
	syncByte(s, _strength);
	syncByte(s, _agility);
	syncByte(s, _stamina);
	syncByte(s, _charisma);
	syncByte(s, _wisdom);
	syncByte(s, _intelligence);
	syncWord(s, _hp);
	syncWord(s, _food);
	s.syncAsByte(_foodSubCtr);
	s.skip(1);
	syncWord(s, _experience);
	syncWord(s, _gold);
	syncWord(s, _food);

	syncByte(s, _mapX);
	syncByte(s, _mapY);

	s.syncAsByte(_hasRing);

	if (s.isLoading()) {
		_paralyzedFlag = 0;
	}
}

void Player::syncByte(Common::Serializer &s, byte &v) {
	byte tmp = ((v / 10) << 4) | (v % 10);
	s.syncAsByte(tmp);
	if (s.isLoading())
		v = ((tmp >> 4) * 10) + (tmp & 0xf);
}

void Player::syncWord(Common::Serializer &s, uint16 &v) {
	byte tmp[2];
	if (s.isLoading()) {
		s.syncBytes(tmp, 2);
		v = ((tmp[0] >> 4) * 1000) + ((tmp[0] & 0xf) * 100) +
			((tmp[1] >> 4) * 10) + (tmp[1] & 0xf);
	} else {
		tmp[0] = ((v / 1000) << 4) | ((v % 1000) / 100);
		tmp[1] = ((v / 10) << 4) | (v % 10);
		s.syncBytes(tmp, 2);
	}
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
