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

#include "ultima/ultima2/data/savegame.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

void Savegame::setupDummyCharacter() {
	*this = Savegame();

	Common::strlcpy(_name, "DUMMY", sizeof(_name));
	_sex = SEX_MALE;
	_race = RACE_HUMAN;
	_class = CLASS_FIGHTER;

	// Even spread of the starting points, plus the male, human and fighter bonuses
	_strength = 15 + 5 + 21;
	_agility = 15;
	_stamina = 15;
	_charisma = 15;
	_wisdom = 15;
	_intelligence = 15 + 5;

	_hp = 400;
	_food = 400;
	_gold = 400;

	_mapEra = 2;
	_mapType = 0;
	_mapX = 20;
	_mapY = 20;
}

void Savegame::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, MAX_NAME_LENGTH + 1);
	s.syncAsByte(_sex);
	s.syncAsByte(_race);
	s.syncAsByte(_class);

	s.syncAsSint16LE(_strength);
	s.syncAsSint16LE(_agility);
	s.syncAsSint16LE(_stamina);
	s.syncAsSint16LE(_charisma);
	s.syncAsSint16LE(_wisdom);
	s.syncAsSint16LE(_intelligence);

	s.syncAsSint16LE(_hp);
	s.syncAsSint16LE(_food);
	s.syncAsSint16LE(_foodTurnCtr);
	s.syncAsSint16LE(_experience);
	s.syncAsSint16LE(_gold);

	s.syncAsByte(_mapEra);
	s.syncAsByte(_mapType);
	s.syncAsByte(_mapX);
	s.syncAsByte(_mapY);
	s.syncAsByte(_overworldReturnX);
	s.syncAsByte(_overworldReturnY);

	s.syncAsByte(_readiedWeapon);
	s.syncAsByte(_readiedArmor);
	s.syncAsByte(_readiedSpell);

	s.syncAsSint16LE(_torches);
	s.syncAsSint16LE(_keys);
	s.syncAsSint16LE(_thievesTools);

	s.syncAsByte(_inSpace);
	s.syncAsByte(_launchMapX);
	s.syncAsByte(_launchMapY);

	s.syncAsSint16LE(_orbitTarget);
	s.syncBytes(_hyperwarpCoords, 3);
	s.syncAsByte(_inFlight);

	s.syncAsByte(_ringQuestFlag);
	s.syncAsByte(_enilnoOwned);
	s.syncAsByte(_patrolWaypoint);
	s.syncAsByte(_patrolTimer);
	s.syncAsByte(_patrolTerrain);

	s.syncAsByte(_dungeonLevel);
	s.syncAsSByte(_facingX);
	s.syncAsSByte(_facingY);

	for (int i = 0; i < 9; ++i)
		s.syncAsByte(_offerRewardItems[i]);

	for (int i = 0; i < ARMOR_COUNT; ++i)
		s.syncAsSint16LE(_armorOwned[i]);
	for (int i = 0; i < WEAPON_COUNT; ++i)
		s.syncAsSint16LE(_weaponOwned[i]);
	for (int i = 0; i < SPELL_COUNT; ++i)
		s.syncAsSint16LE(_spellCharges[i]);
	for (int i = 0; i < ITEM_COUNT; ++i)
		s.syncAsSint16LE(_items[i]);

	s.syncAsSint16LE(_legParalysisTurns);
	s.syncAsSint16LE(_armParalysisTurns);
	s.syncAsSint16LE(_sleepTurns);
	s.syncAsSint16LE(_negateTimeTurns);
	s.syncAsSint16LE(_lightTurns);
	s.syncAsByte(_monsterSpawnCounter);
	s.syncAsByte(_mount);
}

namespace {

// Byte offsets into the original 256-byte PLAYER struct, verified against a
// real ScummVM -> PLAYER -> DOSBox round-trip
enum {
	OFS_NAME = 0x00,
	OFS_SEX = 0x10,
	OFS_CLASS = 0x11,
	OFS_RACE = 0x12,
	OFS_MAP_ERA = 0x13,
	OFS_MAP_TYPE = 0x14,
	OFS_STRENGTH = 0x15,
	OFS_AGILITY = 0x16,
	OFS_STAMINA = 0x17,
	OFS_CHARISMA = 0x18,
	OFS_WISDOM = 0x19,
	OFS_INTELLIGENCE = 0x1A,
	OFS_HP = 0x1B,          // 2 bytes, hi/lo BCD digit pairs
	OFS_FOOD = 0x1D,        // 2 bytes
	OFS_FOOD_TURN_CTR = 0x1F,
	OFS_EXPERIENCE = 0x20,  // 2 bytes
	OFS_GOLD = 0x22,        // 2 bytes
	OFS_MAP_X = 0x24,
	OFS_MAP_Y = 0x25,
	// 0x26-0x2A unused
	OFS_READIED_WEAPON = 0x2B,
	OFS_READIED_ARMOR = 0x2C,
	OFS_READIED_SPELL = 0x2D,
	OFS_TORCHES = 0x2E,
	OFS_KEYS = 0x2F,
	OFS_THIEVES_TOOLS = 0x30,
	// 0x31-0x32 unused
	OFS_IN_SPACE = 0x33,
	OFS_LAUNCH_MAP_X = 0x34,
	OFS_LAUNCH_MAP_Y = 0x35,
	OFS_RING_QUEST_FLAG = 0x36,
	OFS_ORBIT_TARGET = 0x37,
	OFS_PATROL_WAYPOINT = 0x38,
	OFS_OFFER_REWARD_ITEMS = 0x40, // 9 bytes
	OFS_ENILNO_OWNED = 0x49,
	OFS_ARMOR_OWNED = 0x60,  // ARMOR_COUNT bytes, BCD
	OFS_WEAPON_OWNED = 0x76, // WEAPON_COUNT bytes, BCD
	OFS_SPELL_CHARGES = 0x80, // SPELL_COUNT bytes, BCD
	OFS_ITEMS = 0xA0,        // ITEM_COUNT bytes, BCD

	PLAYER_SIZE = 0x100
};

} // namespace

bool Savegame::importOriginal(Common::SeekableReadStream &stream) {
	byte buf[PLAYER_SIZE];
	if (stream.read(buf, sizeof(buf)) != sizeof(buf))
		return false;

	*this = Savegame();

	Common::strlcpy(_name, (const char *)&buf[OFS_NAME], sizeof(_name));

	_sex = (buf[OFS_SEX] == 'M') ? SEX_MALE : SEX_FEMALE;
	_class = (CharClass)CLIP<int>(buf[OFS_CLASS], 0, CLASS_COUNT - 1);
	_race = (Race)CLIP<int>(buf[OFS_RACE], 0, RACE_COUNT - 1);
	_mapEra = buf[OFS_MAP_ERA];
	_mapType = buf[OFS_MAP_TYPE];

	_strength = bcdValue(buf[OFS_STRENGTH]);
	_agility = bcdValue(buf[OFS_AGILITY]);
	_stamina = bcdValue(buf[OFS_STAMINA]);
	_charisma = bcdValue(buf[OFS_CHARISMA]);
	_wisdom = bcdValue(buf[OFS_WISDOM]);
	_intelligence = bcdValue(buf[OFS_INTELLIGENCE]);

	_hp = bcdValue(buf[OFS_HP]) * 100 + bcdValue(buf[OFS_HP + 1]);
	_food = bcdValue(buf[OFS_FOOD]) * 100 + bcdValue(buf[OFS_FOOD + 1]);
	_foodTurnCtr = bcdValue(buf[OFS_FOOD_TURN_CTR]);
	_experience = bcdValue(buf[OFS_EXPERIENCE]) * 100 + bcdValue(buf[OFS_EXPERIENCE + 1]);
	_gold = bcdValue(buf[OFS_GOLD]) * 100 + bcdValue(buf[OFS_GOLD + 1]);

	_mapX = buf[OFS_MAP_X];
	_mapY = buf[OFS_MAP_Y];

	_readiedWeapon = (WeaponType)CLIP<int>(buf[OFS_READIED_WEAPON], 0, WEAPON_COUNT - 1);
	_readiedArmor = (ArmorType)CLIP<int>(buf[OFS_READIED_ARMOR], 0, ARMOR_COUNT - 1);
	_readiedSpell = (SpellType)CLIP<int>(buf[OFS_READIED_SPELL], 0, SPELL_COUNT - 1);
	_torches = bcdValue(buf[OFS_TORCHES]);
	_keys = bcdValue(buf[OFS_KEYS]);
	_thievesTools = bcdValue(buf[OFS_THIEVES_TOOLS]);

	_inSpace = buf[OFS_IN_SPACE] != 0;
	_launchMapX = buf[OFS_LAUNCH_MAP_X];
	_launchMapY = buf[OFS_LAUNCH_MAP_Y];
	_ringQuestFlag = buf[OFS_RING_QUEST_FLAG] != 0;
	_orbitTarget = buf[OFS_ORBIT_TARGET];
	_patrolWaypoint = buf[OFS_PATROL_WAYPOINT];

	for (int i = 0; i < 9; ++i)
		_offerRewardItems[i] = buf[OFS_OFFER_REWARD_ITEMS + i];
	_enilnoOwned = buf[OFS_ENILNO_OWNED] != 0;

	for (int i = 0; i < ARMOR_COUNT; ++i)
		_armorOwned[i] = bcdValue(buf[OFS_ARMOR_OWNED + i]);
	for (int i = 0; i < WEAPON_COUNT; ++i)
		_weaponOwned[i] = bcdValue(buf[OFS_WEAPON_OWNED + i]);
	for (int i = 0; i < SPELL_COUNT; ++i)
		_spellCharges[i] = bcdValue(buf[OFS_SPELL_CHARGES + i]);
	for (int i = 0; i < ITEM_COUNT; ++i)
		_items[i] = bcdValue(buf[OFS_ITEMS + i]);

	// The rest (overworldReturn, dungeon position, moongate timer,
	// hyperwarp coordinates, in-flight flag, ...) are all ScummVM-only
	// additions with nothing to import - left at Savegame()'s defaults
	return true;
}

void Savegame::exportOriginal(Common::WriteStream &stream) const {
	byte buf[PLAYER_SIZE] = {};

	Common::strlcpy((char *)&buf[OFS_NAME], _name, MAX_NAME_LENGTH + 1);

	buf[OFS_SEX] = (_sex == SEX_MALE) ? 'M' : 'F';
	buf[OFS_CLASS] = _class;
	buf[OFS_RACE] = _race;
	buf[OFS_MAP_ERA] = _mapEra;
	buf[OFS_MAP_TYPE] = _mapType;

	buf[OFS_STRENGTH] = toBcd(_strength);
	buf[OFS_AGILITY] = toBcd(_agility);
	buf[OFS_STAMINA] = toBcd(_stamina);
	buf[OFS_CHARISMA] = toBcd(_charisma);
	buf[OFS_WISDOM] = toBcd(_wisdom);
	buf[OFS_INTELLIGENCE] = toBcd(_intelligence);

	buf[OFS_HP] = toBcd(_hp / 100);
	buf[OFS_HP + 1] = toBcd(_hp % 100);
	buf[OFS_FOOD] = toBcd(_food / 100);
	buf[OFS_FOOD + 1] = toBcd(_food % 100);
	buf[OFS_FOOD_TURN_CTR] = toBcd(_foodTurnCtr);
	buf[OFS_EXPERIENCE] = toBcd(_experience / 100);
	buf[OFS_EXPERIENCE + 1] = toBcd(_experience % 100);
	buf[OFS_GOLD] = toBcd(_gold / 100);
	buf[OFS_GOLD + 1] = toBcd(_gold % 100);

	buf[OFS_MAP_X] = _mapX;
	buf[OFS_MAP_Y] = _mapY;

	buf[OFS_READIED_WEAPON] = _readiedWeapon;
	buf[OFS_READIED_ARMOR] = _readiedArmor;
	buf[OFS_READIED_SPELL] = _readiedSpell;
	buf[OFS_TORCHES] = toBcd(_torches);
	buf[OFS_KEYS] = toBcd(_keys);
	buf[OFS_THIEVES_TOOLS] = toBcd(_thievesTools);

	buf[OFS_IN_SPACE] = _inSpace ? 1 : 0;
	buf[OFS_LAUNCH_MAP_X] = _launchMapX;
	buf[OFS_LAUNCH_MAP_Y] = _launchMapY;
	buf[OFS_RING_QUEST_FLAG] = _ringQuestFlag ? 1 : 0;
	buf[OFS_ORBIT_TARGET] = (byte)_orbitTarget;
	buf[OFS_PATROL_WAYPOINT] = _patrolWaypoint;

	for (int i = 0; i < 9; ++i)
		buf[OFS_OFFER_REWARD_ITEMS + i] = _offerRewardItems[i];
	buf[OFS_ENILNO_OWNED] = _enilnoOwned ? 1 : 0;

	for (int i = 0; i < ARMOR_COUNT; ++i)
		buf[OFS_ARMOR_OWNED + i] = toBcd(_armorOwned[i]);
	for (int i = 0; i < WEAPON_COUNT; ++i)
		buf[OFS_WEAPON_OWNED + i] = toBcd(_weaponOwned[i]);
	for (int i = 0; i < SPELL_COUNT; ++i)
		buf[OFS_SPELL_CHARGES + i] = toBcd(_spellCharges[i]);
	for (int i = 0; i < ITEM_COUNT; ++i)
		buf[OFS_ITEMS + i] = toBcd(_items[i]);

	// The original leaves this at 0xFF once the game's speed calibration
	// has run once; harmless either way since nothing reads it back in
	buf[0x3E] = 0xFF;

	stream.write(buf, sizeof(buf));
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
