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

#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/io.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

int SaveGame::write(Common::WriteStream *f) const {
	int i;

	if (!writeInt(_unknown1, f) ||
	        !writeInt(_moves, f))
		return 0;

	for (i = 0; i < 8; i++) {
		if (!_players[i].write(f))
			return 0;
	}

	if (!writeInt(_food, f) ||
	        !writeShort(_gold, f))
		return 0;

	for (i = 0; i < 8; i++) {
		if (!writeShort(_karma[i], f))
			return 0;
	}

	if (!writeShort(_torches, f) ||
	        !writeShort(_gems, f) ||
	        !writeShort(_keys, f) ||
	        !writeShort(_sextants, f))
		return 0;

	for (i = 0; i < ARMR_MAX; i++) {
		if (!writeShort(_armor[i], f))
			return 0;
	}

	for (i = 0; i < WEAP_MAX; i++) {
		if (!writeShort(_weapons[i], f))
			return 0;
	}

	for (i = 0; i < REAG_MAX; i++) {
		if (!writeShort(_reagents[i], f))
			return 0;
	}

	for (i = 0; i < SPELL_MAX; i++) {
		if (!writeShort(_mixtures[i], f))
			return 0;
	}

	if (!writeShort(_items, f) ||
	        !writeChar(_x, f) ||
	        !writeChar(_y, f) ||
	        !writeChar(_stones, f) ||
	        !writeChar(_runes, f) ||
	        !writeShort(_members, f) ||
	        !writeShort(_transport, f) ||
	        !writeShort(_balloonstate, f) ||
	        !writeShort(_trammelPhase, f) ||
	        !writeShort(_feluccaPhase, f) ||
	        !writeShort(_shipHull, f) ||
	        !writeShort(_lbIntro, f) ||
	        !writeShort(_lastCamp, f) ||
	        !writeShort(_lastReagent, f) ||
	        !writeShort(_lastMeditation, f) ||
	        !writeShort(_lastVirtue, f) ||
	        !writeChar(_dngX, f) ||
	        !writeChar(_dngY, f) ||
	        !writeShort(_orientation, f) ||
	        !writeShort(_dngLevel, f) ||
	        !writeShort(_location, f))
		return 0;

	return 1;
}

int SaveGame::read(Common::ReadStream *f) {
	int i;

	if (!readInt(&_unknown1, f) ||
	        !readInt(&_moves, f))
		return 0;

	for (i = 0; i < 8; i++) {
		if (!_players[i].read(f))
			return 0;
	}

	if (!readInt((unsigned int *)&_food, f) ||
	        !readShort((unsigned short *)&_gold, f))
		return 0;

	for (i = 0; i < 8; i++) {
		if (!readShort((unsigned short *) & (_karma[i]), f))
			return 0;
	}

	if (!readShort((unsigned short *)&_torches, f) ||
	        !readShort((unsigned short *)&_gems, f) ||
	        !readShort((unsigned short *)&_keys, f) ||
	        !readShort((unsigned short *)&_sextants, f))
		return 0;

	for (i = 0; i < ARMR_MAX; i++) {
		if (!readShort((unsigned short *) & (_armor[i]), f))
			return 0;
	}

	for (i = 0; i < WEAP_MAX; i++) {
		if (!readShort((unsigned short *) & (_weapons[i]), f))
			return 0;
	}

	for (i = 0; i < REAG_MAX; i++) {
		if (!readShort((unsigned short *) & (_reagents[i]), f))
			return 0;
	}

	for (i = 0; i < SPELL_MAX; i++) {
		if (!readShort((unsigned short *) & (_mixtures[i]), f))
			return 0;
	}

	if (!readShort(&_items, f) ||
	        !readChar(&_x, f) ||
	        !readChar(&_y, f) ||
	        !readChar(&_stones, f) ||
	        !readChar(&_runes, f) ||
	        !readShort(&_members, f) ||
	        !readShort(&_transport, f) ||
	        !readShort(&_balloonstate, f) ||
	        !readShort(&_trammelPhase, f) ||
	        !readShort(&_feluccaPhase, f) ||
	        !readShort(&_shipHull, f) ||
	        !readShort(&_lbIntro, f) ||
	        !readShort(&_lastCamp, f) ||
	        !readShort(&_lastReagent, f) ||
	        !readShort(&_lastMeditation, f) ||
	        !readShort(&_lastVirtue, f) ||
	        !readChar(&_dngX, f) ||
	        !readChar(&_dngY, f) ||
	        !readShort(&_orientation, f) ||
	        !readShort(&_dngLevel, f) ||
	        !readShort(&_location, f))
		return 0;

	/* workaround of U4DOS bug to retain savegame compatibility */
	if (_location == 0 && _dngLevel == 0)
		_dngLevel = 0xFFFF;

	return 1;
}

void SaveGame::init(const SaveGamePlayerRecord *avatarInfo) {
	int i;

	_unknown1 = 0;
	_moves = 0;

	_players[0] = *avatarInfo;
	for (i = 1; i < 8; i++)
		_players[i].init();

	_food = 0;
	_gold = 0;

	for (i = 0; i < 8; i++)
		_karma[i] = 20;

	_torches = 0;
	_gems = 0;
	_keys = 0;
	_sextants = 0;

	for (i = 0; i < ARMR_MAX; i++)
		_armor[i] = 0;

	for (i = 0; i < WEAP_MAX; i++)
		_weapons[i] = 0;

	for (i = 0; i < REAG_MAX; i++)
		_reagents[i] = 0;

	for (i = 0; i < SPELL_MAX; i++)
		_mixtures[i] = 0;

	_items = 0;
	_x = 0;
	_y = 0;
	_stones = 0;
	_runes = 0;
	_members = 1;
	_transport = 0x1f;
	_balloonstate = 0;
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

int SaveGamePlayerRecord::write(Common::WriteStream *f) const {
	int i;

	if (!writeShort(_hp, f) ||
	        !writeShort(_hpMax, f) ||
	        !writeShort(_xp, f) ||
	        !writeShort(_str, f) ||
	        !writeShort(_dex, f) ||
	        !writeShort(_intel, f) ||
	        !writeShort(_mp, f) ||
	        !writeShort(_unknown, f) ||
	        !writeShort((unsigned short)_weapon, f) ||
	        !writeShort((unsigned short)armor, f))
		return 0;

	for (i = 0; i < 16; i++) {
		if (!writeChar(name[i], f))
			return 0;
	}

	if (!writeChar((unsigned char)_sex, f) ||
	        !writeChar((unsigned char)_class, f) ||
	        !writeChar((unsigned char)_status, f))
		return 0;

	return 1;
}

int SaveGamePlayerRecord::read(Common::ReadStream *f) {
	int i;
	unsigned char ch;
	unsigned short s;

	if (!readShort(&_hp, f) ||
	        !readShort(&_hpMax, f) ||
	        !readShort(&_xp, f) ||
	        !readShort(&_str, f) ||
	        !readShort(&_dex, f) ||
	        !readShort(&_intel, f) ||
	        !readShort(&_mp, f) ||
	        !readShort(&_unknown, f))
		return 0;

	if (!readShort(&s, f))
		return 0;
	_weapon = (WeaponType) s;
	if (!readShort(&s, f))
		return 0;
	armor = (ArmorType) s;

	for (i = 0; i < 16; i++) {
		if (!readChar((unsigned char *) & (name[i]), f))
			return 0;
	}

	if (!readChar(&ch, f))
		return 0;
	_sex = (SexType) ch;
	if (!readChar(&ch, f))
		return 0;
	_class = (ClassType) ch;
	if (!readChar(&ch, f))
		return 0;
	_status = (StatusType) ch;

	return 1;
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

	for (i = 0; i < 16; i++)
		name[i] = '\0';

	_sex = SEX_MALE;
	_class = CLASS_MAGE;
	_status = STAT_GOOD;
}

int saveGameMonstersWrite(SaveGameMonsterRecord *monsterTable, Common::WriteStream *f) {
	int i, max;

	if (monsterTable) {
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._tile, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._x, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._y, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._prevTile, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._prevX, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._prevY, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._unused1, f)) return 0;
		for (i = 0; i < MONSTERTABLE_SIZE; i++)
			if (!writeChar(monsterTable[i]._unused2, f)) return 0;
	} else {
		max = MONSTERTABLE_SIZE * 8;
		for (i = 0; i < max; i++)
			if (!writeChar((unsigned char)0, f)) return 0;
	}
	return 1;
}

int saveGameMonstersRead(SaveGameMonsterRecord *monsterTable, Common::ReadStream *f) {
	int i;

	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._tile, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._x, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._y, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._prevTile, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._prevX, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._prevY, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._unused1, f)) return 0;
	for (i = 0; i < MONSTERTABLE_SIZE; i++)
		if (!readChar(&monsterTable[i]._unused2, f)) return 0;

	return 1;
}

} // End of namespace Ultima4
} // End of namespace Ultima
