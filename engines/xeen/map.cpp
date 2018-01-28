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

#include "common/serializer.h"
#include "xeen/map.h"
#include "xeen/interface.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/screen.h"
#include "xeen/xeen.h"

namespace Xeen {

const int MAP_GRID_PRIOR_INDEX[] = { 0, 0, 0, 0, 1, 2, 3, 4, 0 };

const int MAP_GRID_PRIOR_DIRECTION[] = { 0, 1, 2, 3, 1, 2, 3, 0, 0 };

const int MAP_GRID_PRIOR_INDEX2[] = { 0, 0, 0, 0, 2, 3, 4, 1, 0 };

const int MAP_GRID_PRIOR_DIRECTION2[] = { 0, 1, 2, 3, 0, 1, 2, 3, 0 };

const char *const MUSIC_FILES1[5] = {
	"outdoors.m", "town.m", "cavern.m", "dungeon.m", "castle.m"
};

const char *const MUSIC_FILES2[6][7] = {
	{ "outday1.m", "outday2.m", "outday4.m", "outnght1.m",
	"outnght2.m", "outnght4.m", "daydesrt.m" },
	{ "townday1.m", "twnwlk.m", "newbrigh.m", "twnnitea.m",
	"twnniteb.m", "twnwlk.m", "townday1.m" },
	{ "cavern1.m", "cavern2.m", "cavern3a.m", "cavern1.m",
	"cavern2.m", "cavern3a.m", "cavern1.m" },
	{ "dngon1.m", "dngon2.m", "dngon3.m", "dngon1.m",
	"dngon2.m", "dngon3.m", "dngon1.m" },
	{ "cstl1rev.m", "cstl2rev.m", "cstl3rev.m", "cstl1rev.m",
	"cstl2rev.m", "cstl3rev.m", "cstl1rev.m" },
	{ "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m" }
};

MonsterStruct::MonsterStruct() {
	_experience = 0;
	_hp = 0;
	_accuracy = 0;
	_speed = 0;
	_numberOfAttacks = 0;
	_hatesClass = CLASS_KNIGHT;
	_strikes = 0;
	_dmgPerStrike = 0;
	_attackType = DT_PHYSICAL;
	_specialAttack = SA_NONE;
	_hitChance = 0;
	_rangeAttack = 0;
	_monsterType = MONSTER_0;
	_fireResistence = 0;
	_electricityResistence = 0;
	_coldResistence = 0;
	_poisonResistence = 0;
	_energyResistence = 0;
	_magicResistence = 0;
	_phsyicalResistence = 0;
	_field29 = 0;
	_gold = 0;
	_gems = 0;
	_itemDrop = 0;
	_flying = 0;
	_imageNumber = 0;
	_loopAnimation = 0;
	_animationEffect = 0;
	_fx = 0;
}

MonsterStruct::MonsterStruct(Common::String name, int experience, int hp, int accuracy,
		int speed, int numberOfAttacks, CharacterClass hatesClass, int strikes,
		int dmgPerStrike, DamageType attackType, SpecialAttack specialAttack,
		int hitChance, int rangeAttack, MonsterType monsterType,
		int fireResistence, int electricityResistence, int coldResistence,
		int poisonResistence, int energyResistence, int magicResistence,
		int phsyicalResistence, int field29, int gold, int gems, int itemDrop,
		bool flying, int imageNumber, int loopAnimation, int animationEffect,
		int fx, Common::String attackVoc):
		_name(name), _experience(experience), _hp(hp), _accuracy(accuracy),
		_speed(speed), _numberOfAttacks(numberOfAttacks), _hatesClass(hatesClass),
		_strikes(strikes), _dmgPerStrike(dmgPerStrike), _attackType(attackType),
		_specialAttack(specialAttack), _hitChance(hitChance), _rangeAttack(rangeAttack),
		_monsterType(monsterType), _fireResistence(fireResistence),
		_electricityResistence(electricityResistence), _coldResistence(coldResistence),
		_poisonResistence(poisonResistence), _energyResistence(energyResistence),
		_magicResistence(magicResistence), _phsyicalResistence(phsyicalResistence),
		_field29(field29), _gold(gold), _gems(gems), _itemDrop(itemDrop),
		_flying(flying), _imageNumber(imageNumber), _loopAnimation(loopAnimation),
		_animationEffect(animationEffect), _fx(fx), _attackVoc(attackVoc) {
}

void MonsterStruct::synchronize(Common::SeekableReadStream &s) {
	char name[16];
	s.read(name, 16);
	name[15] = '\0';
	_name = Common::String(name);

	_experience = s.readUint32LE();
	_hp = s.readUint16LE();
	_accuracy = s.readByte();
	_speed = s.readByte();
	_numberOfAttacks = s.readByte();
	_hatesClass = (CharacterClass)s.readByte();
	_strikes = s.readUint16LE();
	_dmgPerStrike = s.readByte();
	_attackType = (DamageType)s.readByte();
	_specialAttack = (SpecialAttack)s.readByte();
	_hitChance = s.readByte();
	_rangeAttack = s.readByte();
	_monsterType = (MonsterType)s.readByte();
	_fireResistence = s.readByte();
	_electricityResistence = s.readByte();
	_coldResistence = s.readByte();
	_poisonResistence = s.readByte();
	_energyResistence = s.readByte();
	_magicResistence = s.readByte();
	_phsyicalResistence = s.readByte();
	_field29 = s.readByte();
	_gold = s.readUint16LE();
	_gems = s.readByte();
	_itemDrop = s.readByte();
	_flying = s.readByte() != 0;
	_imageNumber = s.readByte();
	_loopAnimation = s.readByte();
	_animationEffect = s.readByte();
	_fx = s.readByte();

	char attackVoc[10];
	s.read(attackVoc, 9);
	attackVoc[9] = '\0';
	_attackVoc = Common::String(attackVoc);
}

MonsterData::MonsterData() {
	push_back(MonsterStruct("", 0, 0, 0, 0, 0, CLASS_KNIGHT, 1, 1, DT_PHYSICAL,
				   SA_NONE, 1, 0, MONSTER_0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				   0, false, 0, 0, 0, 100, "Slime"));
	push_back(MonsterStruct("Whirlwind", 250000, 1000, 10, 250, 1, CLASS_15, 5,
				   100, DT_PHYSICAL, SA_CONFUSE, 250, 0, MONSTER_0, 100,
				   100, 100, 100, 0, 0, 100, 0, 0, 0, 0, false, 1, 0, 0, 176,
				   "airmon"));
	push_back(MonsterStruct("Annihilator", 1000000, 1500, 40, 200, 12, CLASS_16, 5,
				   50, DT_ENERGY, SA_NONE, 1, 1, MONSTER_0, 80, 80, 100,
				   100, 0, 0, 80, 0, 0, 0, 0, false, 2, 0, 0, 102, "alien1"));
	push_back(MonsterStruct("Autobot", 1000000, 2500, 100, 200, 2, CLASS_16, 5,
				   100, DT_ENERGY, SA_NONE, 1, 0, MONSTER_0, 50, 50, 100,
				   100, 0, 0, 50, 0, 0, 0, 0, true, 3, 0, 0, 101, "alien2"));
	push_back(MonsterStruct("Sewer Stalker", 50000, 250, 30, 25, 1, CLASS_16, 3,
				   100, DT_PHYSICAL, SA_NONE, 50, 0, MONSTER_ANIMAL, 0,
				   0, 50, 50, 0, 0, 0, 0, 0, 0, 0, false, 4, 0, 0, 113,
				   "iguana"));
	push_back(MonsterStruct("Armadillo", 60000, 800, 50, 15, 1, CLASS_16, 100, 6,
				   DT_PHYSICAL, SA_BREAKWEAPON, 60, 0, MONSTER_ANIMAL,
				   50, 0, 80, 80, 50, 0, 50, 0, 0, 0, 0, false, 5, 1, 0, 113,
				   "unnh"));
	push_back(MonsterStruct("Barbarian", 5000, 50, 5, 40, 3, CLASS_SORCERER, 1, 20,
				   DT_PHYSICAL, SA_NONE, 20, 1, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 100, 0, 3, false, 6, 0, 0, 100,
				   "barbarch"));
	push_back(MonsterStruct("Electrapede", 10000, 200, 10, 50, 1, CLASS_PALADIN,
				   50, 1, DT_ELECTRICAL, SA_PARALYZE, 1, 0,
				   MONSTER_INSECT, 50, 100, 50, 50, 50, 0, 0, 0, 0, 0, 0,
				   false, 7, 1, 0, 107, "centi"));
	push_back(MonsterStruct("Cleric of Mok", 30000, 125, 10, 40, 1, CLASS_CLERIC,
				   250, 1, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID,
				   10, 100, 10, 10, 10, 10, 0, 0, 0, 10, 0, false, 8, 0, 0,
				   117, "cleric"));
	push_back(MonsterStruct("Mok Heretic", 50000, 150, 12, 50, 1, CLASS_CLERIC,
				   500, 1, DT_MAGICAL, SA_NONE, 1, 1, MONSTER_HUMANOID, 20, 50,
				   20, 20, 20, 30, 0, 0, 0, 25, 4, false, 8, 0, 0, 117,
				   "cleric"));
	push_back(MonsterStruct("Mantis Ant", 40000, 300, 30, 40, 2, CLASS_16, 2, 100,
				   DT_PHYSICAL, SA_POISON, 30, 0, MONSTER_INSECT, 0, 0,
				   0, 100, 0, 0, 30, 0, 0, 0, 0, false, 10, 0, 0, 104,
				   "spell001"));
	push_back(MonsterStruct("Cloud Dragon", 500000, 2000, 40, 150, 1, CLASS_15,
				   600, 1, DT_COLD, SA_NONE, 1, 1, MONSTER_DRAGON, 0, 50,
				   100, 100, 50, 25, 50, 0, 0, 10, 0, false, 11, 0, 0, 140,
				   "tiger1"));
	push_back(MonsterStruct("Phase Dragon", 2000000, 4000, 80, 200, 1, CLASS_15,
				   750, 1, DT_COLD, SA_NONE, 1, 1, MONSTER_DRAGON, 0, 50,
				   100, 100, 80, 50, 50, 0, 0, 20, 0, false, 11, 0, 10, 140,
				   "Begger"));
	push_back(MonsterStruct("Green Dragon", 500000, 2500, 50, 150, 1, CLASS_15,
				   500, 1, DT_FIRE, SA_NONE, 1, 1, MONSTER_DRAGON, 100,
				   50, 0, 100, 50, 25, 50, 0, 0, 10, 0, false, 13, 0, 0, 140,
				   "tiger1"));
	push_back(MonsterStruct("Energy Dragon", 2000000, 5000, 100, 250, 1, CLASS_15,
				   1000, 1, DT_ENERGY, SA_NONE, 1, 1, MONSTER_DRAGON, 80,
				   80, 60, 100, 100, 30, 50, 0, 0, 20, 0, false, 13, 0, 7,
				   140, "begger"));
	push_back(MonsterStruct("Dragon Mummy", 2000000, 3000, 30, 100, 1,
				   CLASS_CLERIC, 2000, 2, DT_PHYSICAL, SA_DISEASE, 200,
				   0, MONSTER_DRAGON, 0, 80, 100, 100, 0, 10, 90, 0, 0, 0,
				   0, false, 15, 0, 0, 140, "dragmum"));
	push_back(MonsterStruct("Scraps", 2000000, 3000, 30, 100, 1, CLASS_16, 2000, 2,
				   DT_PHYSICAL, SA_NONE, 200, 0, MONSTER_DRAGON, 0, 80,
				   100, 100, 0, 10, 90, 0, 0, 0, 0, false, 15, 0, 0, 140,
				   "dragmum"));
	push_back(MonsterStruct("Earth Blaster", 250000, 1000, 10, 100, 1, CLASS_15, 5,
				   100, DT_PHYSICAL, SA_NONE, 200, 0, MONSTER_0, 100, 90,
				   90, 100, 0, 0, 90, 0, 0, 0, 0, false, 17, 0, 0, 100,
				   "earthmon"));
	push_back(MonsterStruct("Beholder Bat", 10000, 75, 15, 80, 1, CLASS_15, 5, 5,
				   DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, true, 18, 0, 0, 120, "eyeball"));
	push_back(MonsterStruct("Fire Blower", 250000, 1000, 20, 60, 1, CLASS_15, 5,
				   100, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0,
				   100, 50, 0, 50, 0, 0, 0, 0, false, 19, 0, 0, 110, "fire"));
	push_back(MonsterStruct("Hell Hornet", 50000, 250, 30, 50, 2, CLASS_DRUID, 2,
				   250, DT_POISON, SA_WEAKEN, 1, 0, MONSTER_INSECT, 50,
				   50, 50, 100, 50, 0, 50, 0, 0, 0, 0, true, 20, 0, 0, 123,
				   "insect"));
	push_back(MonsterStruct("Gargoyle", 30000, 150, 35, 30, 2, CLASS_16, 5, 50,
				   DT_PHYSICAL, SA_NONE, 60, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   20, 0, 0, 0, 0, 0, false, 21, 0, 10, 100, "gargrwl"));
	push_back(MonsterStruct("Giant", 100000, 500, 25, 45, 2, CLASS_16, 100, 5,
				   DT_PHYSICAL, SA_UNCONSCIOUS, 100, 0, MONSTER_0, 0, 0,
				   0, 0, 0, 0, 0, 0, 1000, 0, 5, false, 22, 0, 0, 100,
				   "giant"));
	push_back(MonsterStruct("Goblin", 1000, 10, 5, 30, 2, CLASS_16, 2, 6,
				   DT_PHYSICAL, SA_NONE, 1, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, false, 25, 0, 0, 131, "gremlin"));
	push_back(MonsterStruct("Onyx Golem", 1000000, 10000, 50, 100, 1, CLASS_15, 2,
				   250, DT_MAGICAL, SA_DRAINSP, 1, 0, MONSTER_GOLEM, 100, 100,
				   100, 100, 100, 100, 50, 0, 0, 100, 0, true, 24, 0, 10,
				   100, "golem"));
	push_back(MonsterStruct("Gremlin", 2000, 20, 7, 35, 2, CLASS_16, 2, 10,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, false, 26, 0, 0, 101, "gremlink"));
	push_back(MonsterStruct("Gremlin Guard", 3000, 50, 10, 35, 2, CLASS_16, 6, 5,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, false, 26, 0, 0, 101, "gremlink"));
	push_back(MonsterStruct("Griffin", 60000, 800, 35, 150, 2, CLASS_KNIGHT, 50, 6,
				   DT_PHYSICAL, SA_NONE, 150, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 80, 0, 0, 0, 0, 0, false, 27, 0, 0, 120, "screech"));
	push_back(MonsterStruct("Gamma Gazer", 1000000, 5000, 60, 200, 7, CLASS_16, 10,
				   20, DT_ENERGY, SA_NONE, 1, 0, MONSTER_0, 100, 100, 0,
				   100, 100, 0, 60, 0, 0, 0, 0, false, 28, 0, 0, 140, "hydra"));
	push_back(MonsterStruct("Iguanasaurus", 100000, 2500, 20, 30, 1, CLASS_16, 10,
				   50, DT_PHYSICAL, SA_INSANE, 150, 0, MONSTER_ANIMAL, 50,
				   50, 50, 50, 50, 0, 20, 0, 0, 0, 0, false, 29, 0, 0, 113,
				   "iguana"));
	push_back(MonsterStruct("Slayer Knight", 50000, 500, 30, 50, 1, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_NONE, 100, 0, MONSTER_HUMANOID,
				   50, 50, 50, 50, 50, 0, 0, 0, 50, 0, 5, false, 30, 0, 0,
				   141, "knight"));
	push_back(MonsterStruct("Death Knight", 100000, 750, 50, 80, 2, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_NONE, 150, 0, MONSTER_HUMANOID,
				   50, 50, 50, 50, 50, 10, 0, 0, 100, 0, 6, false, 30, 0, 0,
				   141, "knight"));
	push_back(MonsterStruct("Lava Dweller", 500000, 1500, 30, 40, 1, CLASS_15, 5,
				   100, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 100, 0,
				   100, 50, 0, 50, 0, 0, 0, 0, false, 19, 0, 0, 110, "fire"));
	push_back(MonsterStruct("Lava Roach", 50000, 500, 20, 70, 1, CLASS_16, 5, 50,
				   DT_FIRE, SA_NONE, 1, 0, MONSTER_INSECT, 100, 100, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, false, 33, 0, 0, 131, "Phantom"));
	push_back(MonsterStruct("Power Lich", 200000, 500, 20, 60, 1, CLASS_15, 10, 10,
				   DT_MAGICAL, SA_UNCONSCIOUS, 1, 1, MONSTER_UNDEAD, 0, 0, 0, 0,
				   0, 80, 70, 0, 0, 0, 0, true, 34, 0, 0, 141, "lich"));
	push_back(MonsterStruct("Mystic Mage", 100000, 200, 20, 70, 1, CLASS_15, 10,
				   20, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 50, 100,
				   50, 50, 50, 30, 0, 0, 0, 50, 0, true, 35, 0, 0, 163,
				   "monsterb"));
	push_back(MonsterStruct("Magic Mage", 200000, 300, 25, 80, 1, CLASS_15, 10, 30,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 50, 100, 50,
				   50, 50, 50, 0, 0, 0, 75, 0, true, 35, 0, 0, 163,
				   "monsterb"));
	push_back(MonsterStruct("Minotaur", 250000, 3000, 80, 120, 1, CLASS_16, 100, 4,
				   DT_PHYSICAL, SA_AGING, 150, 0, MONSTER_0, 0, 0, 10, 0,
				   0, 50, 60, 0, 0, 0, 0, false, 37, 0, 0, 141, "stonegol"));
	push_back(MonsterStruct("Gorgon", 250000, 4000, 90, 100, 1, CLASS_16, 100, 3,
				   DT_PHYSICAL, SA_STONE, 100, 0, MONSTER_0, 0, 0, 0, 0,
				   0, 60, 70, 0, 0, 0, 0, false, 37, 0, 0, 141, "stonegol"));
	push_back(MonsterStruct("Higher Mummy", 100000, 400, 20, 60, 1, CLASS_CLERIC,
				   10, 40, DT_PHYSICAL, SA_CURSEITEM, 100, 0,
				   MONSTER_UNDEAD, 0, 50, 50, 100, 50, 20, 75, 0, 0, 0, 0,
				   false, 39, 0, 0, 141, "mummy"));
	push_back(MonsterStruct("Orc Guard", 5000, 60, 10, 20, 1, CLASS_12, 3, 10,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 50, 0, 2, false, 40, 0, 0, 125, "orc"));
	push_back(MonsterStruct("Octopod", 250000, 2500, 40, 80, 1, CLASS_15, 2, 100,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_ANIMAL, 0, 0, 50,
				   100, 0, 0, 0, 0, 0, 0, 0, true, 41, 0, 0, 101, "photon"));
	push_back(MonsterStruct("Ogre", 10000, 100, 15, 30, 1, CLASS_16, 4, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 100, 0, 0, false, 42, 0, 0, 136, "ogre"));
	push_back(MonsterStruct("Orc Shaman", 10000, 50, 15, 30, 1, CLASS_15, 5, 5,
				   DT_COLD, SA_SLEEP, 1, 1, MONSTER_HUMANOID, 0, 0, 0, 0,
				   0, 10, 0, 0, 75, 10, 2, false, 43, 0, 0, 125, "fx7"));
	push_back(MonsterStruct("Sabertooth", 10000, 100, 20, 60, 3, CLASS_16, 5, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, false, 44, 1, 0, 101, "saber"));
	push_back(MonsterStruct("Sand Flower", 10000, 100, 10, 50, 5, CLASS_16, 5, 5,
				   DT_PHYSICAL, SA_INLOVE, 50, 0, MONSTER_0, 0, 0, 0, 0,
				   0, 50, 50, 0, 0, 0, 0, false, 45, 0, 0, 106, "sand"));
	push_back(MonsterStruct("Killer Cobra", 25000, 1000, 25, 100, 1, CLASS_16, 2,
				   100, DT_PHYSICAL, SA_AGING, 30, 0, MONSTER_ANIMAL, 0,
				   0, 0, 100, 0, 50, 0, 0, 0, 0, 0, false, 46, 0, 0, 100,
				   "hiss"));
	push_back(MonsterStruct("Sewer Rat", 2000, 40, 5, 35, 1, CLASS_16, 3, 10,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, false, 47, 0, 0, 136, "rat"));
	push_back(MonsterStruct("Sewer Slug", 1000, 25, 2, 25, 1, CLASS_16, 2, 10,
				   DT_PHYSICAL, SA_NONE, 5, 0, MONSTER_INSECT, 0, 0, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, false, 48, 0, 0, 111, "zombie"));
	push_back(MonsterStruct("Skeletal Lich", 500000, 2000, 30, 200, 1,
				   CLASS_SORCERER, 1000, 1, DT_ENERGY, SA_ERADICATE, 1, 1,
				   MONSTER_UNDEAD, 80, 70, 80, 100, 100, 50, 50, 0, 0, 0,
				   0, false, 49, 0, 0, 140, "elecbolt"));
	push_back(MonsterStruct("Enchantress", 40000, 100, 25, 60, 1, CLASS_CLERIC, 3,
				   150, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID,
				   10, 100, 10, 10, 10, 20, 0, 0, 0, 20, 0, false, 50, 0, 0,
				   163, "disint"));
	push_back(MonsterStruct("Sorceress", 80000, 200, 30, 80, 1, CLASS_15, 2, 50,
				   DT_MAGICAL, SA_NONE, 1, 1, MONSTER_HUMANOID, 10, 20, 10, 10,
				   10, 80, 0, 0, 0, 50, 5, false, 50, 0, 0, 163, "disint"));
	push_back(MonsterStruct("Arachnoid", 4000, 50, 10, 40, 1, CLASS_16, 3, 5,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_INSECT, 0, 0, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, false, 52, 0, 0, 104, "web"));
	push_back(MonsterStruct("Medusa Sprite", 5000, 30, 5, 30, 1, CLASS_RANGER, 3,
				   3, DT_PHYSICAL, SA_STONE, 10, 0, MONSTER_0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, true, 53, 0, 0, 42, "hiss"));
	push_back(MonsterStruct("Rogue", 5000, 50, 10, 30, 1, CLASS_ROBBER, 1, 60,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 70, 0, 0, false, 54, 0, 0, 100, "thief"));
	push_back(MonsterStruct("Thief", 10000, 100, 15, 40, 1, CLASS_ROBBER, 1, 100,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 200, 0, 0, false, 54, 0, 0, 100,
				   "thief"));
	push_back(MonsterStruct("Troll Grunt", 10000, 100, 5, 50, 1, CLASS_16, 2, 25,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, false, 56, 0, 0, 136, "troll"));
	push_back(MonsterStruct("Vampire", 200000, 400, 30, 80, 1, CLASS_CLERIC, 10,
				   10, DT_PHYSICAL, SA_WEAKEN, 100, 0, MONSTER_UNDEAD, 50,
				   50, 50, 50, 50, 50, 50, 0, 0, 0, 0, false, 57, 0, 0, 42,
				   "vamp"));
	push_back(MonsterStruct("Vampire Lord", 300000, 500, 35, 100, 1, CLASS_CLERIC,
				   10, 30, DT_PHYSICAL, SA_SLEEP, 120, 0, MONSTER_UNDEAD,
				   50, 50, 50, 50, 50, 50, 70, 0, 0, 0, 0, false, 58, 0, 0,
				   42, "vamp"));
	push_back(MonsterStruct("Vulture Roc", 200000, 2500, 50, 150, 1, CLASS_16, 5,
				   60, DT_PHYSICAL, SA_NONE, 100, 0, MONSTER_ANIMAL, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, true, 59, 0, 0, 120, "vulture"));
	push_back(MonsterStruct("Sewer Hag", 50000, 75, 10, 40, 1, CLASS_PALADIN, 10,
				   25, DT_ELECTRICAL, SA_INSANE, 1, 1, MONSTER_HUMANOID,
				   0, 100, 0, 100, 0, 20, 0, 0, 0, 10, 0, false, 62, 0, 0,
				   108, "elecspel"));
	push_back(MonsterStruct("Tidal Terror", 500000, 1000, 10, 200, 1, CLASS_15, 5,
				   100, DT_COLD, SA_NONE, 1, 0, MONSTER_0, 100, 50, 50,
				   100, 50, 0, 100, 0, 0, 0, 0, true, 61, 0, 0, 101,
				   "splash3"));
	push_back(MonsterStruct("Witch", 80000, 150, 15, 70, 1, CLASS_15, 10, 10,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID, 0, 100,
				   0, 20, 0, 20, 0, 0, 0, 10, 0, false, 63, 0, 0, 114,
				   "elecspel"));
	push_back(MonsterStruct("Coven Leader", 120000, 250, 20, 100, 1, CLASS_15, 10,
				   15, DT_ENERGY, SA_DRAINSP, 1, 1, MONSTER_HUMANOID, 10,
				   100, 0, 50, 100, 50, 0, 0, 0, 20, 6, false, 63, 0, 10, 114,
				   "elecspel"));
	push_back(MonsterStruct("Master Wizard", 120000, 500, 25, 150, 2, CLASS_KNIGHT,
				   10, 40, DT_FIRE, SA_NONE, 1, 1, MONSTER_HUMANOID, 100,
				   50, 50, 50, 50, 50, 0, 0, 0, 50, 0, false, 64, 0, 0, 163,
				   "boltelec"));
	push_back(MonsterStruct("Wizard", 60000, 250, 20, 125, 1, CLASS_PALADIN, 10,
				   25, DT_MAGICAL, SA_NONE, 1, 1, MONSTER_HUMANOID, 50, 30, 30,
				   30, 30, 30, 0, 0, 0, 20, 0, false, 65, 0, 0, 163, "wizard"));
	push_back(MonsterStruct("Dark Wolf", 10000, 70, 10, 70, 3, CLASS_16, 3, 8,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, false, 66, 1, 0, 100, "wolf"));
	push_back(MonsterStruct("Screamer", 500000, 3000, 50, 200, 1, CLASS_15, 10, 20,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_0, 0, 0, 0, 100, 0,
				   0, 60, 0, 0, 0, 0, false, 67, 0, 0, 110, "dragon"));
	push_back(MonsterStruct("Cult Leader", 100000, 100, 20, 60, 1, CLASS_15, 10,
				   10, DT_ENERGY, SA_NONE, 1, 1, MONSTER_HUMANOID, 50, 50,
				   50, 50, 100, 50, 0, 0, 0, 100, 6, false, 8, 0, 0, 100,
				   "cleric"));
	push_back(MonsterStruct("Mega Dragon", 100000000, 64000, 100, 200, 1, CLASS_15,
				   10, 200, DT_ENERGY, SA_ERADICATE, 1, 1, MONSTER_DRAGON,
				   100, 100, 100, 100, 100, 100, 90, 0, 0, 232, 0, false, 11,
				   0, 7, 100, "tiger1"));
	push_back(MonsterStruct("Gettlewaithe", 5000, 100, 15, 35, 2, CLASS_16, 5, 5,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 2000, 0, 5, false, 25, 0, 0, 100, "gremlin"));
	push_back(MonsterStruct("Doom Knight", 500000, 1000, 50, 100, 4, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_DEATH, 150, 0,
				   MONSTER_HUMANOID, 80, 80, 80, 80, 80, 20, 0, 0, 200,
				   0, 7, false, 30, 0, 10, 100, "knight"));
	push_back(MonsterStruct("Sandro", 200000, 1000, 20, 75, 1, CLASS_15, 10, 10,
				   DT_MAGICAL, SA_DEATH, 1, 1, MONSTER_UNDEAD, 0, 0, 0, 0, 0,
				   90, 80, 0, 0, 100, 7, true, 34, 0, 10, 100, "lich"));
	push_back(MonsterStruct("Mega Mage", 500000, 500, 35, 100, 1, CLASS_15, 10, 40,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 80, 100, 80,
				   80, 80, 80, 0, 0, 0, 100, 6, true, 35, 0, 11, 100,
				   "monsterb"));
	push_back(MonsterStruct("Orc Elite", 15000, 200, 15, 40, 2, CLASS_12, 5, 10,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 100, 0, 3, false, 40, 0, 0, 100, "orc"));
	push_back(MonsterStruct("Shaalth", 20000, 300, 15, 50, 1, CLASS_15, 5, 10,
				   DT_COLD, SA_SLEEP, 1, 0, MONSTER_HUMANOID, 0, 0, 0, 0,
				   0, 20, 0, 0, 1000, 50, 5, false, 43, 0, 10, 100, "fx7"));
	push_back(MonsterStruct("Rooka", 5000, 60, 5, 40, 1, CLASS_16, 3, 10,
				   DT_PHYSICAL, SA_DISEASE, 15, 0, MONSTER_ANIMAL, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 10, 4, false, 47, 0, 0, 100, "rat"));
	push_back(MonsterStruct("Morgana", 200000, 300, 35, 100, 1, CLASS_15, 2, 60,
				   DT_ENERGY, SA_PARALYZE, 1, 1, MONSTER_HUMANOID, 50, 50,
				   50, 50, 100, 80, 0, 0, 0, 100, 6, false, 50, 0, 10, 100,
				   "disint"));
	push_back(MonsterStruct("Master Thief", 20000, 100, 20, 50, 1, CLASS_ROBBER, 1,
				   250, DT_PHYSICAL, SA_NONE, 40, 0, MONSTER_HUMANOID, 0,
				   0, 0, 0, 0, 0, 0, 0, 250, 20, 4, false, 54, 0, 14, 100,
				   "thief"));
	push_back(MonsterStruct("Royal Vampire", 400000, 750, 40, 125, 1, CLASS_CLERIC,
				   10, 50, DT_PHYSICAL, SA_CURSEITEM, 120, 0,
				   MONSTER_UNDEAD, 50, 50, 50, 50, 50, 50, 65, 0, 0, 0, 0,
				   false, 57, 0, 0, 100, "vamp"));
	push_back(MonsterStruct("Ct. Blackfang", 2000000, 1500, 50, 150, 1,
				   CLASS_CLERIC, 10, 100, DT_PHYSICAL, SA_DEATH, 120, 0,
				   MONSTER_UNDEAD, 75, 75, 75, 75, 75, 75, 75, 0, 0, 0, 0,
				   false, 58, 0, 10, 100, "vamp"));
	push_back(MonsterStruct("Troll Guard", 15000, 200, 10, 60, 1, CLASS_16, 2, 35,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, false, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Troll Chief", 20000, 300, 15, 65, 1, CLASS_16, 2, 50,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, false, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Hobstadt", 25000, 400, 20, 70, 1, CLASS_16, 2, 50,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 1000, 0, 4, false, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Graalg", 20000, 200, 15, 50, 1, CLASS_16, 5, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 1000, 0, 5, false, 42, 0, 0, 100, "ogre"));
	push_back(MonsterStruct("Vampire King", 3000000, 10000, 60, 200, 1,
				   CLASS_CLERIC, 10, 250, DT_PHYSICAL, SA_ERADICATE, 150,
				   0, MONSTER_UNDEAD, 80, 80, 80, 80, 80, 80, 90, 0, 0, 0,
				   0, false, 58, 0, 0, 100, "vamp"));
	push_back(MonsterStruct("Valio", 60000, 150, 15, 60, 1, CLASS_PALADIN, 10, 25,
				   DT_MAGICAL, SA_NONE, 1, 0, MONSTER_HUMANOID, 50, 30, 30, 30,
				   40, 30, 0, 0, 0, 0, 0, false, 65, 0, 0, 100, "wizard"));
	push_back(MonsterStruct("Sky Golem", 200000, 1000, 50, 100, 1, CLASS_15, 2,
				   100, DT_COLD, SA_NONE, 1, 1, MONSTER_GOLEM, 50, 50,
				   100, 50, 50, 50, 50, 0, 0, 0, 0, true, 24, 0, 0, 100,
				   "golem"));
	push_back(MonsterStruct("Gurodel", 100000, 750, 30, 60, 2, CLASS_16, 100, 6,
				   DT_PHYSICAL, SA_UNCONSCIOUS, 110, 0, MONSTER_0, 0, 0,
				   0, 0, 0, 0, 0, 0, 5000, 0, 6, false, 22, 0, 0, 100,
				   "giant"));
	push_back(MonsterStruct("Yog", 25000, 100, 5, 60, 1, CLASS_SORCERER, 1, 30,
				   DT_PHYSICAL, SA_NONE, 25, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 200, 0, 4, false, 6, 0, 10, 100,
				   "barbarch"));
	push_back(MonsterStruct("Sharla", 10000, 50, 5, 50, 1, CLASS_RANGER, 3, 4,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, true, 53, 0, 0, 100, "hiss"));
	push_back(MonsterStruct("Ghost Mummy", 500000, 500, 35, 175, 1, CLASS_CLERIC,
				   200, 5, DT_PHYSICAL, SA_AGING, 150, 0, MONSTER_UNDEAD,
				   0, 60, 80, 80, 80, 50, 80, 0, 0, 0, 0, false, 40, 0, 6,
				   100, "orc"));
	push_back(MonsterStruct("Phase Mummy", 500000, 500, 35, 175, 1, CLASS_CLERIC,
				   200, 6, DT_PHYSICAL, SA_DRAINSP, 150, 0,
				   MONSTER_UNDEAD, 0, 70, 80, 80, 80, 60, 85, 0, 0, 0, 0,
				   false, 39, 0, 7, 100, "mummy"));
	push_back(MonsterStruct("Xenoc", 250000, 700, 35, 175, 1, CLASS_15, 10, 50,
				   DT_ENERGY, SA_NONE, 1, 0, MONSTER_HUMANOID, 50, 50, 50,
				   50, 100, 50, 0, 0, 0, 100, 6, false, 64, 0, 0, 100,
				   "boltelec"));
	push_back(MonsterStruct("Barkman", 4000000, 40000, 25, 100, 3, CLASS_16, 250,
				   1, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0, 100,
				   0, 0, 0, 0, 0, 0, 6, false, 19, 0, 11, 100, "fire"));
}

void MonsterData::load(const Common::String &name) {
	File f(name);
	synchronize(f);
}

void MonsterData::synchronize(Common::SeekableReadStream &s) {
	clear();

	MonsterStruct spr;
	while (!s.eos()) {
		spr.synchronize(s);
		push_back(spr);
	}
}

/*------------------------------------------------------------------------*/

SurroundingMazes::SurroundingMazes() {
	clear();
}

void SurroundingMazes::clear() {
	_north = 0;
	_east = 0;
	_south = 0;
	_west = 0;
}

void SurroundingMazes::synchronize(XeenSerializer &s) {
	s.syncAsUint16LE(_north);
	s.syncAsUint16LE(_east);
	s.syncAsUint16LE(_south);
	s.syncAsUint16LE(_west);
}

int &SurroundingMazes::operator[](int idx) {
	switch (idx) {
	case DIR_NORTH:
		return _north;
	case DIR_EAST:
		return _east;
	case DIR_SOUTH:
		return _south;
	default:
		return _west;
	}
}

/*------------------------------------------------------------------------*/

MazeDifficulties::MazeDifficulties() {
	_unlockDoor = 0;
	_unlockBox = 0;
	_bashDoor = 0;
	_bashGrate = 0;
	_bashWall = 0;
	_wallNoPass = -1;
	_surfaceNoPass = -1;
	_chance2Run = -1;
}

void MazeDifficulties::synchronize(XeenSerializer &s) {
	s.syncAsByte(_wallNoPass);
	s.syncAsByte(_surfaceNoPass);
	s.syncAsByte(_unlockDoor);
	s.syncAsByte(_unlockBox);
	s.syncAsByte(_bashDoor);
	s.syncAsSint8(_bashGrate);
	s.syncAsSint8(_bashWall);
	s.syncAsSint8(_chance2Run);
}

/*------------------------------------------------------------------------*/

MazeData::MazeData() {
	clear();
}

void MazeData::clear() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_wallData[y][x]._data = 0;
		Common::fill(&_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH], false);
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], false);
		_wallTypes[y] = 0;
		_surfaceTypes[y] = 0;
	}
	_mazeNumber = 0;
	_surroundingMazes.clear();
	_mazeFlags = _mazeFlags2  = 0;
	_floorType = 0;
	_trapDamage = 0;
	_wallKind = 0;
	_tavernTips = 0;
	_mazeId = 0;
}

void MazeData::synchronize(XeenSerializer &s) {
	byte b;

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			s.syncAsUint16LE(_wallData[y][x]._data);
	}
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			if (s.isLoading()) {
				s.syncAsByte(b);
				_cells[y][x]._surfaceId = b & 7;
				_cells[y][x]._flags = b & 0xF8;
			} else {
				b = (_cells[y][x]._surfaceId & 7) | (_cells[y][x]._flags & 0xf8);
				s.syncAsByte(b);
			}
		}
	}

	s.syncAsUint16LE(_mazeNumber);
	_surroundingMazes.synchronize(s);
	s.syncAsUint16LE(_mazeFlags);
	s.syncAsUint16LE(_mazeFlags2);

	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_wallTypes[i]);
	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_surfaceTypes[i]);

	s.syncAsByte(_floorType);
	s.syncAsByte(_runPosition.x);
	_difficulties.synchronize(s);
	s.syncAsByte(_runPosition.y);
	s.syncAsByte(_trapDamage);
	s.syncAsByte(_wallKind);
	s.syncAsByte(_tavernTips);

	for (int y = 0; y < MAP_HEIGHT; ++y)
		File::syncBitFlags(s, &_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH]);
	for (int y = 0; y < MAP_HEIGHT; ++y)
		File::syncBitFlags(s, &_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH]);
}

void MazeData::setAllTilesStepped() {
	for (int y = 0; y < MAP_HEIGHT; ++y)
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], true);
}

void MazeData::clearCellSurfaces() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_cells[y][x]._surfaceId = 0;
	}
}

/*------------------------------------------------------------------------*/

MobStruct::MobStruct() {
	_id = 0;
	_direction = DIR_NORTH;
}

bool MobStruct::synchronize(XeenSerializer &s) {
	s.syncAsSint8(_pos.x);
	s.syncAsSint8(_pos.y);
	s.syncAsByte(_id);
	s.syncAsByte(_direction);

	return _id != 0xff || _pos.x != -1 || _pos.y != -1;
}

void MobStruct::endOfList() {
	_pos.x = _pos.y = -1;
	_id = 0xff;
	_direction = (Direction)-1;
}

/*------------------------------------------------------------------------*/

MazeObject::MazeObject() {
	_id = 0;
	_frame = 0;
	_spriteId = 0;
	_direction = DIR_NORTH;
	_flipped = false;
	_sprites = nullptr;
}

/*------------------------------------------------------------------------*/

MazeMonster::MazeMonster() {
	_frame = 0;
	_id = 0;
	_spriteId = 0;
	_isAttacking = false;
	_damageType = DT_PHYSICAL;
	_field9 = 0;
	_postAttackDelay = 0;
	_hp = 0;
	_effect1 = _effect2 = 0;
	_effect3 = 0;
	_sprites = nullptr;
	_attackSprites = nullptr;
	_monsterData = nullptr;
}

int MazeMonster::getTextColor() const {
	if (_hp == _monsterData->_hp)
		return 15;
	else if (_hp >= (_monsterData->_hp / 2))
		return 9;
	else
		return 32;
}

/*------------------------------------------------------------------------*/

MazeWallItem::MazeWallItem() {
	_id = 0;
	_frame = 0;
	_spriteId = 0;
	_direction = DIR_NORTH;
	_sprites = nullptr;
}

/*------------------------------------------------------------------------*/

MonsterObjectData::MonsterObjectData(XeenEngine *vm): _vm(vm) {
}

void MonsterObjectData::synchronize(XeenSerializer &s, MonsterData &monsterData) {
	Common::Array<MobStruct> mobStructs;
	MobStruct mobStruct;
	byte b;

	if (s.isLoading()) {
		_objectSprites.clear();
		_monsterSprites.clear();
		_monsterAttackSprites.clear();
		_wallItemSprites.clear();
		_objects.clear();
		_monsters.clear();
		_wallItems.clear();
	}

	for (uint i = 0; i < 16; ++i) {
		b = (i >= _objectSprites.size()) ? 0xff : _objectSprites[i]._spriteId;
		s.syncAsByte(b);
		if (s.isLoading() && b != 0xff)
			_objectSprites.push_back(SpriteResourceEntry(b));
	}
	for (uint i = 0; i < 16; ++i) {
		b = (i >= _monsterSprites.size()) ? 0xff : _monsterSprites[i]._spriteId;
		s.syncAsByte(b);
		if (s.isLoading() && b != 0xff)
			_monsterSprites.push_back(SpriteResourceEntry(b));
	}
	for (uint i = 0; i < 16; ++i) {
		b = (i >= _wallItemSprites.size()) ? 0xff : _wallItemSprites[i]._spriteId;
		s.syncAsByte(b);
		if (s.isLoading() && b != 0xff)
			_wallItemSprites.push_back(SpriteResourceEntry(b));
	}

	if (s.isSaving()) {
		// Save objects
		for (uint i = 0; i < _objects.size(); ++i) {
			mobStruct._pos = _objects[i]._position;
			mobStruct._id = _objects[i]._id;
			mobStruct._direction = _objects[i]._direction;
			mobStruct.synchronize(s);
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

		// Save monsters
		for (uint i = 0; i < _monsters.size(); ++i) {
			mobStruct._pos = _monsters[i]._position;
			mobStruct._id = _monsters[i]._id;
			mobStruct._direction = DIR_NORTH;
			mobStruct.synchronize(s);
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

		// Save wall items
		if (_wallItems.size() == 0) {
			MobStruct nullStruct;
			nullStruct._pos.x = nullStruct._pos.y = 0x80;
			nullStruct.synchronize(s);
		} else {
			for (uint i = 0; i < _wallItems.size(); ++i) {
				mobStruct._pos = _wallItems[i]._position;
				mobStruct._id = _wallItems[i]._id;
				mobStruct._direction = _wallItems[i]._direction;
				mobStruct.synchronize(s);
			}
		}
		mobStruct.endOfList();
		mobStruct.synchronize(s);

	} else {
		// Load monster/obbject data and merge together with sprite Ids
		// Load objects
		mobStruct.synchronize(s);
		do {
			MazeObject obj;
			obj._position = mobStruct._pos;
			obj._id = mobStruct._id;
			obj._direction = mobStruct._direction;
			obj._frame = 100;

			if (obj._id < (int)_objectSprites.size()) {
				obj._spriteId = _objectSprites[obj._id]._spriteId;
				obj._sprites = &_objectSprites[obj._id]._sprites;
			} else {
				assert(!obj._id);
			}

			_objects.push_back(obj);
			mobStruct.synchronize(s);
		} while (mobStruct._id != 255 || mobStruct._pos.x != -1);

		// Load monsters
		mobStruct.synchronize(s);
		do {
			MazeMonster mon;
			mon._position = mobStruct._pos;
			mon._id = mobStruct._id;
			mon._frame = _vm->getRandomNumber(7);

			if (mon._id < (int)_monsterSprites.size()) {
				mon._spriteId = _monsterSprites[mon._id]._spriteId;
				mon._sprites = &_monsterSprites[mon._id]._sprites;
				mon._attackSprites = &_monsterSprites[mon._id]._attackSprites;
				mon._monsterData = &monsterData[mon._spriteId];
			} else {
				assert(!mon._id);
			}

			MonsterStruct &md = *mon._monsterData;
			mon._hp = md._hp;
			mon._effect1 = mon._effect2 = md._animationEffect;
			if (md._animationEffect)
				mon._effect3 = _vm->getRandomNumber(7);

			_monsters.push_back(mon);
			mobStruct.synchronize(s);
		} while (mobStruct._id != 255 || mobStruct._pos.x != -1);

		// Load wall items
		mobStruct.synchronize(s);
		do {
			if (mobStruct._id < (int)_wallItemSprites.size()) {
				MazeWallItem wi;
				wi._position = mobStruct._pos;
				wi._id = mobStruct._id;
				wi._direction = mobStruct._direction;
				wi._spriteId = _wallItemSprites[wi._id]._spriteId;
				wi._sprites = &_wallItemSprites[wi._id]._sprites;

				_wallItems.push_back(wi);
			}

			mobStruct.synchronize(s);
		} while (mobStruct._id != 255 || mobStruct._pos.x != -1);
	}
}

void MonsterObjectData::clearMonsterSprites() {
	_monsterSprites.clear();
	_monsterAttackSprites.clear();
}

void MonsterObjectData::addMonsterSprites(MazeMonster &monster) {
	Map &map = *g_vm->_map;
	int imgNumber = map._monsterData[monster._spriteId]._imageNumber;
	uint idx;

	// Find the sprites for the monster, loading them in if necessary
	for (idx = 0; idx < _monsterSprites.size(); ++idx) {
		if (_monsterSprites[idx]._spriteId == monster._spriteId) {
			monster._sprites = &_monsterSprites[idx]._sprites;
			break;
		}
	}
	if (idx == _monsterSprites.size()) {
		_monsterSprites.push_back(SpriteResourceEntry(monster._spriteId));
		_monsterSprites.back()._sprites.load(Common::String::format("%03u.mon", imgNumber));
		monster._sprites = &_monsterSprites.back()._sprites;
	}

	// Find the attack sprites for the monster, loading them in if necessary
	for (idx = 0; idx < _monsterAttackSprites.size(); ++idx) {
		if (_monsterAttackSprites[idx]._spriteId == monster._spriteId) {
			monster._attackSprites = &_monsterAttackSprites[idx]._sprites;
			break;
		}
	}
	if (idx == _monsterAttackSprites.size()) {
		_monsterAttackSprites.push_back(SpriteResourceEntry(monster._spriteId));
		_monsterAttackSprites.back()._sprites.load(Common::String::format("%03u.att", imgNumber));
		monster._attackSprites = &_monsterAttackSprites.back()._sprites;
	}
}

/*------------------------------------------------------------------------*/

HeadData::HeadData() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			_data[y][x]._left = _data[y][x]._right = 0;
		}
	}
}

void HeadData::synchronize(Common::SeekableReadStream &s) {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			_data[y][x]._left = s.readByte();
			_data[y][x]._right = s.readByte();
		}
	}
}

/*------------------------------------------------------------------------*/

void AnimationEntry::synchronize(Common::SeekableReadStream &s) {
	for (int i = 0; i < 4; ++i)
		_frame1._frames[i] = s.readByte();
	for (int i = 0; i < 4; ++i)
		_flipped._flags[i] = s.readByte() != 0;
	for (int i = 0; i < 4; ++i)
		_frame2._frames[i] = s.readByte();
}

void AnimationInfo::synchronize(Common::SeekableReadStream &s) {
	AnimationEntry entry;

	clear();
	while (s.pos() < s.size()) {
		entry.synchronize(s);
		push_back(entry);
	}
}

void AnimationInfo::load(const Common::String &name) {
	File f(name);
	synchronize(f);
	f.close();
}

/*------------------------------------------------------------------------*/

Map::Map(XeenEngine *vm) : _vm(vm), _mobData(vm) {
	_loadDarkSide = false;
	_sideTownPortal = 0;
	_sideObjects = 0;
	_sideMonsters = 0;
	_sidePictures = 0;
	_sideMusic = 0;
	_isOutdoors = false;
	_mazeDataIndex = 0;
	_currentSteppedOn = false;
	_currentSurfaceId = 0;
	_currentWall = 0;
	_currentTile = 0;
	_currentGrateUnlocked = false;
	_currentCantRest = false;
	_currentIsDrain = false;
	_currentIsEvent = false;
	_currentSky = 0;
	_currentMonsterFlags = 0;
}

void Map::load(int mapId) {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	IndoorDrawList &indoorList = intf._indoorList;
	OutdoorDrawList &outdoorList = intf._outdoorList;

	PleaseWait waitMsg(intf._falling);
	waitMsg.show();

	intf._objNumber = 0;
	party._stepped = true;
	party._mazeId = mapId;
	saveMaze();
	events.clearEvents();

	_sideObjects = 1;
	_sideMonsters = 1;
	_sidePictures = 1;
	if (mapId >= 113 && mapId <= 127) {
		_sideTownPortal = 0;
	} else {
		_sideTownPortal = _loadDarkSide ? 1 : 0;
	}

	if (_vm->getGameID() == GType_WorldOfXeen) {
		files.setGameCc(1);

		if (!_loadDarkSide) {
			_animationInfo.load("clouds.dat");
			_monsterData.load("xeen.mon");
			_wallPicSprites.load("xeenpic.dat");
			_sidePictures = 0;
			_sideMonsters = 0;
			_sideObjects = 0;
		} else {
			switch (mapId) {
			case 113:
			case 114:
			case 115:
			case 116:
			case 128:
				_animationInfo.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObjects = 0;
				break;
			case 117:
			case 118:
			case 119:
			case 120:
			case 124:
				_animationInfo.load("clouds.dat");
				_monsterData.load("xeen.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObjects = 0;
				_sideMonsters = 0;
				break;
			case 125:
			case 126:
			case 127:
				_animationInfo.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("xeenpic.dat");
				_sideObjects = 0;
				_sidePictures = 0;
				break;
			default:
				_animationInfo.load("dark.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("darkpic.dat");
				break;
			}
		}

		files.setGameCc(_loadDarkSide);
	}

	// Load any events for the new map
	loadEvents(mapId);

	// Iterate through loading the given maze as well as the two successive
	// mazes in each of the four cardinal directions
	bool isDarkCc = files._isDarkCc;
	MazeData *mazeDataP = &_mazeData[0];
	bool textLoaded = false;

	for (int idx = 0; idx < 9; ++idx, ++mazeDataP) {
		mazeDataP->_mazeId = mapId;

		if (mapId == 0) {
			mazeDataP->clear();
		} else {
			// Load in the maze's data file
			Common::String datName = Common::String::format("maze%c%03d.dat",
				(mapId >= 100) ? 'x' : '0', mapId);
			File datFile(datName);
			XeenSerializer datSer(&datFile, nullptr);
			mazeDataP->synchronize(datSer);
			datFile.close();

			if (isDarkCc && mapId == 50)
				mazeDataP->setAllTilesStepped();
			if (!isDarkCc && party._gameFlags[0][25] &&
					(mapId == 42 || mapId == 43 || mapId == 4)) {
				mazeDataP->clearCellSurfaces();
			}

			_isOutdoors = (mazeDataP->_mazeFlags2 & FLAG_IS_OUTDOORS) != 0;

			// Handle loading text data
			if (!textLoaded) {
				textLoaded = true;

				Common::String txtName = Common::String::format("%s%c%03d.txt",
					isDarkCc ? "dark" : "xeen", mapId >= 100 ? 'x' : '0', mapId);
				File fText(txtName, 1);
				char mazeName[33];
				fText.read(mazeName, 33);
				mazeName[32] = '\0';

				_mazeName = Common::String(mazeName);
				fText.close();

				// Load the monster/object data
				Common::String mobName = Common::String::format("maze%c%03d.mob",
					(mapId >= 100) ? 'x' : '0', mapId);
				File mobFile(mobName);
				XeenSerializer sMob(&mobFile, nullptr);
				_mobData.synchronize(sMob, _monsterData);
				mobFile.close();

				Common::String headName = Common::String::format("aaze%c%03d.hed",
					(mapId >= 100) ? 'x' : '0', mapId);
				File headFile(headName);
				_headData.synchronize(headFile);
				headFile.close();

				if (!isDarkCc && mapId == 15) {
					if ((_mobData._monsters[0]._position.x > 31 || _mobData._monsters[0]._position.y > 31) &&
						(_mobData._monsters[1]._position.x > 31 || _mobData._monsters[1]._position.y > 31) &&
						(_mobData._monsters[2]._position.x > 31 || _mobData._monsters[2]._position.y > 31)) {
						party._gameFlags[0][56] = true;
					}
				}
			}
		}

		// Move to next surrounding maze
		MazeData *baseMaze = &_mazeData[MAP_GRID_PRIOR_INDEX[idx]];
		mapId = baseMaze->_surroundingMazes[MAP_GRID_PRIOR_DIRECTION[idx]];
		if (!mapId) {
			baseMaze = &_mazeData[MAP_GRID_PRIOR_INDEX2[idx]];
			mapId = baseMaze->_surroundingMazes[MAP_GRID_PRIOR_DIRECTION2[idx]];
		}
	}

	// Reload the monster data for the main maze that we're loading
	mapId = party._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.mob",
		(mapId >= 100) ? 'x' : '0', mapId);
	File mobFile(filename);
	XeenSerializer sMob(&mobFile, nullptr);
	_mobData.synchronize(sMob, _monsterData);
	mobFile.close();

	// Load sprites for the objects
	for (uint i = 0; i < _mobData._objectSprites.size(); ++i) {
		files.setGameCc(_sideObjects);

		if (party._cloudsEnd && _mobData._objectSprites[i]._spriteId == 85 &&
				mapId == 27 && isDarkCc) {
			_mobData._objects[29]._spriteId = 0;
			_mobData._objects[29]._id = 8;
			_mobData._objectSprites[i]._sprites.clear();
		} else if (mapId == 12 && party._gameFlags[0][43] &&
			_mobData._objectSprites[i]._spriteId == 118 && !isDarkCc) {
			filename = "085.obj";
			_mobData._objectSprites[0]._spriteId = 85;
		} else {
			filename = Common::String::format("%03d.%cbj",
				_mobData._objectSprites[i]._spriteId,
				_mobData._objectSprites[i]._spriteId >= 100 ? '0' : 'o');
		}

		// Read in the object sprites
		_mobData._objectSprites[i]._sprites.load(filename);
	}

	// Load sprites for the monsters
	for (uint i = 0; i < _mobData._monsterSprites.size(); ++i) {
		MonsterObjectData::SpriteResourceEntry &spr = _mobData._monsterSprites[i];
		uint imgNumber = _monsterData[spr._spriteId]._imageNumber;

		files.setGameCc((spr._spriteId == 91 && _vm->getGameID() == GType_WorldOfXeen) ?
			0 : _sideMonsters);
		filename = Common::String::format("%03u.mon", imgNumber);
		_mobData._monsterSprites[i]._sprites.load(filename);

		filename = Common::String::format("%03u.att", imgNumber);
		_mobData._monsterSprites[i]._attackSprites.load(filename);
	}

	// Load wall picture sprite resources
	for (uint i = 0; i < _mobData._wallItemSprites.size(); ++i) {
		filename = Common::String::format("%03d.pic", _mobData._wallItemSprites[i]._spriteId);
		_mobData._wallItemSprites[i]._sprites.load(filename, _sidePictures);
	}

	files.setGameCc(isDarkCc);

	// Handle loading miscellaneous sprites for the map
	if (_isOutdoors) {
		// Start playing relevant music
		sound._musicSide = isDarkCc;
		Common::String musName;

		if (_vm->_files->_isDarkCc) {
			int randIndex = _vm->getRandomNumber(6);
			musName = MUSIC_FILES2[_mazeData->_wallKind][randIndex];
		} else {
			musName = "outdoors.m";
		}
		if (musName != sound._currentMusic)
			sound.playSong(musName, 207);

		// Load sprite sets needed for scene rendering
		_groundSprites.load("water.out");
		_tileSprites.load("outdoor.til");
		outdoorList._sky1._sprites = &_skySprites[0];
		outdoorList._sky2._sprites = &_skySprites[0];
		outdoorList._groundSprite._sprites = &_groundSprites;

		for (int i = 0; i < TOTAL_SURFACES; ++i) {
			_wallSprites._surfaces[i].clear();

			if (_mazeData[0]._wallTypes[i] != 0) {
				_wallSprites._surfaces[i].load(Common::String::format("%s.wal",
					Res.OUTDOORS_WALL_TYPES[_mazeData[0]._wallTypes[i]]));
			}

			_surfaceSprites[i].clear();
			if (i != 0 && _mazeData[0]._surfaceTypes[i] != 0)
				_surfaceSprites[i].load(Res.SURFACE_NAMES[_mazeData[0]._surfaceTypes[i]]);
		}
	} else {
		if (files._isDarkCc && (mapId == 125 || mapId == 126 || mapId == 127))
			files.setGameCc(0);
		sound._musicSide = files._isDarkCc;

		// Start playing relevant music
		const int MUS_INDEXES[] = { 1, 2, 3, 4, 3, 5 };
		Common::String musName;

		_sideMusic = isDarkCc;
		if (isDarkCc) {
			int randIndex = _vm->getRandomNumber(6);
			musName = MUSIC_FILES2[MUS_INDEXES[_mazeData->_wallKind]][randIndex];
		} else {
			musName = MUSIC_FILES1[MUS_INDEXES[_mazeData->_wallKind]];
		}
		if (musName != sound._currentMusic)
			sound.playSong(musName, 207);

		// Load sprite sets needed for scene rendering
		_skySprites[1].load(Common::String::format("%s.sky",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_groundSprites.load(Common::String::format("%s.gnd",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_tileSprites.load(Common::String::format("%s.til",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));

		for (int i = 0; i < TOTAL_SURFACES; ++i) {
			_surfaceSprites[i].clear();

			if (_mazeData[0]._surfaceTypes[i] != 0 || i == 4)
				_surfaceSprites[i].load(Res.SURFACE_NAMES[_mazeData[0]._surfaceTypes[i]]);
		}

		for (int i = 0; i < TOTAL_SURFACES; ++i)
			_wallSprites._surfaces[i].clear();

		_wallSprites._fwl1.load(Common::String::format("f%s1.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_wallSprites._fwl2.load(Common::String::format("f%s2.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_wallSprites._fwl3.load(Common::String::format("f%s3.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_wallSprites._fwl4.load(Common::String::format("f%s4.fwl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));
		_wallSprites._swl.load(Common::String::format("s%s.swl",
			Res.TERRAIN_TYPES[_mazeData[0]._wallKind]));

		// Set entries in the indoor draw list to the correct sprites
		// for drawing various parts of the background
		indoorList._swl_0F1R._sprites = &_wallSprites._swl;
		indoorList._swl_0F1L._sprites = &_wallSprites._swl;
		indoorList._swl_1F1R._sprites = &_wallSprites._swl;
		indoorList._swl_1F1L._sprites = &_wallSprites._swl;
		indoorList._swl_2F2R._sprites = &_wallSprites._swl;
		indoorList._swl_2F1R._sprites = &_wallSprites._swl;
		indoorList._swl_2F1L._sprites = &_wallSprites._swl;
		indoorList._swl_2F2L._sprites = &_wallSprites._swl;

		indoorList._swl_3F1R._sprites = &_wallSprites._swl;
		indoorList._swl_3F2R._sprites = &_wallSprites._swl;
		indoorList._swl_3F3R._sprites = &_wallSprites._swl;
		indoorList._swl_3F4R._sprites = &_wallSprites._swl;
		indoorList._swl_3F1L._sprites = &_wallSprites._swl;
		indoorList._swl_3F2L._sprites = &_wallSprites._swl;
		indoorList._swl_3F3L._sprites = &_wallSprites._swl;
		indoorList._swl_3F4L._sprites = &_wallSprites._swl;

		indoorList._swl_4F4R._sprites = &_wallSprites._swl;
		indoorList._swl_4F3R._sprites = &_wallSprites._swl;
		indoorList._swl_4F2R._sprites = &_wallSprites._swl;
		indoorList._swl_4F1R._sprites = &_wallSprites._swl;
		indoorList._swl_4F1L._sprites = &_wallSprites._swl;
		indoorList._swl_4F2L._sprites = &_wallSprites._swl;
		indoorList._swl_4F3L._sprites = &_wallSprites._swl;
		indoorList._swl_4F4L._sprites = &_wallSprites._swl;

		indoorList._fwl_4F4R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F3R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F2R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F1R._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F1L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F2L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F3L._sprites = &_wallSprites._fwl4;
		indoorList._fwl_4F4L._sprites = &_wallSprites._fwl4;

		indoorList._fwl_2F1R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_2F._sprites = &_wallSprites._fwl3;
		indoorList._fwl_2F1L._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F2R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F1R._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F1L._sprites = &_wallSprites._fwl3;
		indoorList._fwl_3F2L._sprites = &_wallSprites._fwl3;

		indoorList._fwl_1F._sprites = &_wallSprites._fwl1;
		indoorList._fwl_1F1R._sprites = &_wallSprites._fwl1;
		indoorList._fwl_1F1L._sprites = &_wallSprites._fwl1;
		indoorList._horizon._sprites = &_wallSprites._fwl1;

		indoorList._ground._sprites = &_groundSprites;

		// Don't show horizon for certain maps
		if (_vm->_files->_isDarkCc) {
			if ((mapId >= 89 && mapId <= 112) || mapId == 128 || mapId == 129)
				indoorList._horizon._sprites = nullptr;
		} else {
			if (mapId >= 25 && mapId <= 27)
				indoorList._horizon._sprites = nullptr;
		}
	}

	loadSky();

	files.setGameCc(isDarkCc);
}

void Map::findMap(int mapId) {
	if (mapId == -1)
		mapId = _vm->_party->_mazeId;

	_mazeDataIndex = 0;
	while (_mazeDataIndex < 9 && _mazeData[_mazeDataIndex]._mazeId != mapId)
		++_mazeDataIndex;
	if (_mazeDataIndex == 9)
		error("Could not find map %d", mapId);
}

int Map::mazeLookup(const Common::Point &pt, int layerShift, int wallMask) {
	Common::Point pos = pt;
	int mapId = _vm->_party->_mazeId;

	if (pt.x < -16 || pt.y < -16 || pt.x >= 32 || pt.y >= 32) {
		_currentWall = INVALID_CELL;
		return INVALID_CELL;
	}

	// Find the correct maze data out of the set to use
	findMap();

	// Handle map changing to the north or south as necessary
	if (pos.y & 16) {
		if (pos.y >= 0) {
			pos.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pos.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (mapId) {
			// Move to the correct map to north/south
			findMap(mapId);
		} else {
			// No map, so reached outside indoor area or outer space outdoors
			_currentSteppedOn = true;
			return _isOutdoors ? SURFTYPE_SPACE : INVALID_CELL;
		}
	}

	// Handle map changing to the east or west as necessary
	if (pos.x & 16) {
		if (pos.x >= 0) {
			pos.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pos.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		if (mapId)
			// Move to the correct map to east/west
			findMap(mapId);
	}

	if (mapId) {
		if (_isOutdoors) {
			_currentSurfaceId = _mazeData[_mazeDataIndex]._wallData[pos.y][pos.x]._outdoors._surfaceId;
		} else {
			_currentSurfaceId = _mazeData[_mazeDataIndex]._cells[pos.y][pos.x]._surfaceId;
		}

		if (_currentSurfaceId == SURFTYPE_SPACE || _currentSurfaceId == SURFTYPE_SKY) {
			_currentSteppedOn = true;
		} else {
			_currentSteppedOn = _mazeData[_mazeDataIndex]._steppedOnTiles[pos.y][pos.x];
		}

		return (_mazeData[_mazeDataIndex]._wallData[pos.y][pos.x]._data >> layerShift) & wallMask;

	} else {
		_currentSteppedOn = _isOutdoors;
		return _isOutdoors ? SURFTYPE_SPACE : INVALID_CELL;
	}
}

void Map::loadEvents(int mapId) {
	// Load events
	Common::String filename = Common::String::format("maze%c%03d.evt",
		(mapId >= 100) ? 'x' : '0', mapId);
	File fEvents(filename);
	XeenSerializer sEvents(&fEvents, nullptr);
	_events.synchronize(sEvents);
	fEvents.close();

	// Load text data
	filename = Common::String::format("aaze%c%03d.txt",
		(mapId >= 100) ? 'x' : '0', mapId);
	File fText(filename);
	_events._text.clear();
	while (fText.pos() < fText.size())
		_events._text.push_back(fText.readString());
	fText.close();
}

void Map::saveEvents() {
	// Save eents
	int mapId = _mazeData[0]._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.evt",
		(mapId >= 100) ? 'x' : '0', mapId);
	OutFile fEvents(filename);
	XeenSerializer sEvents(nullptr, &fEvents);
	_events.synchronize(sEvents);
	fEvents.finalize();
}

void Map::saveMap() {
	FileManager &files = *g_vm->_files;
	Party &party = *g_vm->_party;
	int mapId = _mazeData[0]._mazeId;
	if (!files._isDarkCc && mapId == 85)
		return;

	// Save the primary maze
	Common::String datName = Common::String::format("maze%c%03d.dat", (mapId >= 100) ? 'x' : '0', mapId);
	OutFile datFile(datName);
	XeenSerializer datSer(nullptr, &datFile);
	_mazeData[0].synchronize(datSer);
	datFile.finalize();

	if (!files._isDarkCc && mapId == 15) {
		for (uint idx = 0; idx < MIN(_mobData._monsters.size(), (uint)3); ++idx) {
			MazeMonster &mon = _mobData._monsters[idx];
			if (mon._position.x > 31 || mon._position.y > 31) {
				party._gameFlags[0][56] = true;
				break;
			}
		}
	}

	if (!_isOutdoors) {
		// Iterate through the surrounding mazes
		for (int mazeIndex = 1; mazeIndex < 9; ++mazeIndex) {
			mapId = _mazeData[mazeIndex]._mazeId;
			if (mapId == 0)
				continue;

			datName = Common::String::format("maze%c%03d.dat", (mapId >= 100) ? 'x' : '0', mapId);
			OutFile datFile2(datName);
			XeenSerializer datSer2(nullptr, &datFile2);
			_mazeData[mazeIndex].synchronize(datSer2);
			datFile2.finalize();
		}
	}
}

void Map::saveMonsters() {
	int mapId = _mazeData[0]._mazeId;
	Common::String filename = Common::String::format("maze%c%03d.mob",
		(mapId >= 100) ? 'x' : '0', mapId);
	OutFile fMob(filename);
	XeenSerializer sMob(nullptr, &fMob);
	_mobData.synchronize(sMob, _monsterData);
	fMob.finalize();
}

void Map::saveMaze() {
	int mazeNum = _mazeData[0]._mazeNumber;
	if (!mazeNum || (mazeNum == 85 && !_vm->_files->_isDarkCc))
		return;

	saveEvents();
	saveMap();
	saveMonsters();
}

void Map::cellFlagLookup(const Common::Point &pt) {
	Common::Point pos = pt;
	findMap();

	int mapId = _vm->_party->_mazeId;
	findMap(mapId);

	// Handle map changing to the north or south as necessary
	if (pos.y & 16) {
		if (pos.y >= 0) {
			pos.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pos.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		findMap(mapId);
	}

	// Handle map changing to the east or west as necessary
	if (pos.x & 16) {
		if (pos.x >= 0) {
			pos.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pos.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		findMap(mapId);
	}

	// Get the cell flags
	const MazeCell &cell = _mazeData[_mazeDataIndex]._cells[pos.y][pos.x];
	_currentGrateUnlocked = cell._flags & OUTFLAG_GRATE;
	_currentCantRest = cell._flags & RESTRICTION_REST;
	_currentIsDrain = cell._flags & OUTFLAG_DRAIN;
	_currentIsEvent = cell._flags & FLAG_AUTOEXECUTE_EVENT;
	_currentSky = (cell._flags & OUTFLAG_OBJECT_EXISTS) ? 1 : 0;
	_currentMonsterFlags = cell._flags & 7;
}

void Map::setCellSurfaceFlags(const Common::Point &pt, int bits) {
	mazeLookup(pt, 0);

	Common::Point mapPos(pt.x & 15, pt.y & 15);
	MazeCell &cell = _mazeData[_mazeDataIndex]._cells[mapPos.y][mapPos.x];
	cell._flags |= bits & 0xF8;
}

void Map::setWall(const Common::Point &pt, Direction dir, int v) {
	const int XOR_MASKS[4] = { 0xFFF, 0xF0FF, 0xFF0F, 0xFFF0 };
	mazeLookup(pt, 0, 0);

	Common::Point mapPos(pt.x & 15, pt.y & 15);
	MazeWallLayers &wallLayer = _mazeData[_mazeDataIndex]._wallData[mapPos.y][mapPos.x];
	wallLayer._data &= XOR_MASKS[dir];
	wallLayer._data |= v << Res.WALL_SHIFTS[dir][2];
}

int Map::getCell(int idx) {
	int mapId = _vm->_party->_mazeId;
	Direction dir = _vm->_party->_mazeDirection;
	Common::Point pt(
		_vm->_party->_mazePosition.x + Res.SCREEN_POSITIONING_X[_vm->_party->_mazeDirection][idx],
		_vm->_party->_mazePosition.y + Res.SCREEN_POSITIONING_Y[_vm->_party->_mazeDirection][idx]
	);

	if (pt.x > 31 || pt.y > 31) {
		if (_vm->_files->_isDarkCc) {
			if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
					mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
				_currentSurfaceId = SURFTYPE_DESERT;
			} else {
				_currentSurfaceId = 0;
			}
		} else {
			_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? 7 : 0;
		}
		_currentWall = INVALID_CELL;
		return INVALID_CELL;
	}

	findMap(mapId);

	if (pt.y & 16) {
		if (pt.y >= 0) {
			pt.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pt.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (!mapId) {
			if (_isOutdoors) {
				_currentSurfaceId = SURFTYPE_SPACE;
				_currentWall = 0;
				return 0;
			} else {
				if (_vm->_files->_isDarkCc) {
					if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
						mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
						_currentSurfaceId = 6;
					} else {
						_currentSurfaceId = 0;
					}
				} else {
					_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? SURFTYPE_ROAD : SURFTYPE_DEFAULT;
				}

				_currentWall = INVALID_CELL;
				return INVALID_CELL;
			}
		}

		findMap(mapId);
	}

	if (pt.x & 16) {
		if (pt.x >= 0) {
			pt.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pt.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		}

		if (!mapId) {
			if (_isOutdoors) {
				_currentSurfaceId = SURFTYPE_SPACE;
				_currentWall = 0;
				return 0;
			} else {
				if (_vm->_files->_isDarkCc) {
					if ((mapId >= 53 && mapId <= 88 && mapId != 73) || (mapId >= 74 && mapId <= 120) ||
						mapId == 125 || mapId == 126 || mapId == 128 || mapId == 129) {
						_currentSurfaceId = 6;
					} else {
						_currentSurfaceId = 0;
					}
				} else {
					_currentSurfaceId = (mapId >= 25 && mapId <= 27) ? SURFTYPE_ROAD : SURFTYPE_DEFAULT;
				}

				_currentWall = INVALID_CELL;
				return INVALID_CELL;
			}
		}

		findMap(mapId);
	}

	assert(pt.x >= 0 && pt.x < 16 && pt.y >= 0 && pt.y < 16);
	int wallData = _mazeData[_mazeDataIndex]._wallData[pt.y][pt.x]._data;
	if (_isOutdoors) {
		if (mapId) {
			_currentTile = (wallData >> 8) & 0xFF;
			_currentWall = (wallData >> 4) & 0xF;
			_currentSurfaceId = wallData & 0xF;
		} else {
			_currentSurfaceId = SURFTYPE_DEFAULT;
			_currentWall = 0;
			_currentTile = 0;
		}
	} else {
		if (!mapId)
			return 0;

		_currentSurfaceId = _mazeData[_mazeDataIndex]._cells[pt.y][pt.x]._surfaceId;
		_currentWall = wallData;
		return (_currentWall >> Res.WALL_SHIFTS[dir][idx]) & 0xF;
	}

	return _currentWall;
}

void Map::loadSky() {
	Party &party = *_vm->_party;

	party._isNight = party._minutes < (5 * 60) || party._minutes >= (21 * 60);
	_skySprites[0].load(((party._mazeId >= 89 && party._mazeId <= 112) ||
		party._mazeId == 128 || party._mazeId == 129) || !party._isNight
		? "sky.sky" : "night.sky");
}

void Map::getNewMaze() {
	Party &party = *_vm->_party;
	Common::Point pt = party._mazePosition;
	int mapId = party._mazeId;

	// Get the correct map to use from the cached list
	findMap(mapId);

	// Adjust Y and X to be in the 0-15 range, and on the correct surrounding
	// map if either value is < 0 or >= 16
	if (pt.y & 16) {
		if (pt.y >= 0) {
			pt.y -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._north;
		} else {
			pt.y += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._south;
		}

		if (mapId)
			findMap(mapId);
	}

	if (pt.x & 16) {
		if (pt.x >= 0) {
			pt.x -= 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._east;
		} else {
			pt.x += 16;
			mapId = _mazeData[_mazeDataIndex]._surroundingMazes._west;
		}

		if (mapId)
			findMap(mapId);
	}

	// Save the adjusted (0,0)-(15,15) position and load the given map.
	// This will make it the new center, with it's own surrounding mazees loaded
	party._mazePosition = pt;
	if (mapId)
		load(mapId);
}

} // End of namespace Xeen
