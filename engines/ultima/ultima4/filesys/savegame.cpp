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

#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/filesys/io.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

void SaveGame::synchronize(Common::Serializer &s) {
	int i;

	s.syncAsUint32LE(_unknown1);
	s.syncAsUint32LE(_moves);

	for (i = 0; i < 8; ++i)
		_players[i].synchronize(s);

	s.syncAsUint32LE(_food);
	s.syncAsUint16LE(_gold);

	for (i = 0; i < 8; ++i)
		s.syncAsUint16LE(_karma[i]);

	s.syncAsUint16LE(_torches);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_keys);
	s.syncAsUint16LE(_sextants);

	for (i = 0; i < ARMR_MAX; ++i)
		s.syncAsUint16LE(_armor[i]);

	for (i = 0; i < WEAP_MAX; ++i)
		s.syncAsUint16LE(_weapons[i]);

	for (i = 0; i < REAG_MAX; ++i)
		s.syncAsUint16LE(_reagents[i]);

	for (i = 0; i < SPELL_MAX; ++i)
		s.syncAsUint16LE(_mixtures[i]);

	s.syncAsUint16LE(_items);
	s.syncAsByte(_x);
	s.syncAsByte(_y);
	s.syncAsByte(_stones);
	s.syncAsByte(_runes);
	s.syncAsUint16LE(_members);
	s.syncAsUint16LE(_transport);
	s.syncAsUint16LE(_balloonState);
	s.syncAsUint16LE(_trammelPhase);
	s.syncAsUint16LE(_feluccaPhase);
	s.syncAsUint16LE(_shipHull);
	s.syncAsUint16LE(_lbIntro);
	s.syncAsUint16LE(_lastCamp);
	s.syncAsUint16LE(_lastReagent);
	s.syncAsUint16LE(_lastMeditation);
	s.syncAsUint16LE(_lastVirtue);
	s.syncAsByte(_dngX);
	s.syncAsByte(_dngY);
	s.syncAsUint16LE(_orientation);
	s.syncAsUint16LE(_dngLevel);
	s.syncAsUint16LE(_location);
}

void SaveGame::init(const SaveGamePlayerRecord *avatarInfo) {
	int i;

	_unknown1 = 0;
	_moves = 0;

	_players[0] = *avatarInfo;
	for (i = 1; i < 8; ++i)
		_players[i].init();

	_food = 0;
	_gold = 0;

	for (i = 0; i < 8; ++i)
		_karma[i] = 20;

	_torches = 0;
	_gems = 0;
	_keys = 0;
	_sextants = 0;

	for (i = 0; i < ARMR_MAX; ++i)
		_armor[i] = 0;

	for (i = 0; i < WEAP_MAX; ++i)
		_weapons[i] = 0;

	for (i = 0; i < REAG_MAX; ++i)
		_reagents[i] = 0;

	for (i = 0; i < SPELL_MAX; ++i)
		_mixtures[i] = 0;

	_items = 0;
	_x = 0;
	_y = 0;
	_stones = 0;
	_runes = 0;
	_members = 1;
	_transport = 0x1f;
	_balloonState = 0;
	_trammelPhase = 0;
	_feluccaPhase = 0;
	_shipHull = 50;
	_lbIntro = 0;
	_lastCamp = 0;
	_lastReagent = 0;
	_lastMeditation = 0;
	_lastVirtue = 0;
	_dngX = 0;
	_dngY = 0;
	_orientation = 0;
	_dngLevel = 0xFFFF;
	_location = 0;
}

void SaveGamePlayerRecord::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hpMax);
	s.syncAsUint16LE(_xp);
	s.syncAsUint16LE(_str);
	s.syncAsUint16LE(_dex);
	s.syncAsUint16LE(_intel);
	s.syncAsUint16LE(_mp);
	s.syncAsUint16LE(_unknown);
	s.syncAsUint16LE(_weapon);
	s.syncAsUint16LE(armor);
	s.syncBytes((byte *)name, 16);
	s.syncAsByte(_sex);
	s.syncAsByte(_class);
	s.syncAsByte(_status);
}

void SaveGamePlayerRecord::init() {
	int i;

	_hp = 0;
	_hpMax = 0;
	_xp = 0;
	_str = 0;
	_dex = 0;
	_intel = 0;
	_mp = 0;
	_unknown = 0;
	_weapon = WEAP_HANDS;
	armor = ARMR_NONE;

	for (i = 0; i < 16; ++i)
		name[i] = '\0';

	_sex = SEX_MALE;
	_class = CLASS_MAGE;
	_status = STAT_GOOD;
}

void SaveGameMonsterRecord::synchronize(SaveGameMonsterRecord *monsterTable, Common::Serializer &s) {
	int i;

	if (s.isSaving() && !monsterTable) {
		int dataSize = MONSTERTABLE_SIZE * 8;
		byte b = 0;
		while (dataSize-- > 0)
			s.syncAsByte(b);

		return;
	}

	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._tile);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._x);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._y);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevTile);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevX);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevY);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._unused1);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._unused2);
}

} // End of namespace Ultima4
} // End of namespace Ultima
