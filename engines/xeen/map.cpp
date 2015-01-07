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
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/screen.h"
#include "xeen/xeen.h"

namespace Xeen {

const int MAP_GRID_PRIOR_INDEX[] = { 0, 0, 0, 0, 1, 2, 3, 4, 0 };

const int MAP_GRID_PRIOR_DIRECTION[] = { 0, 1, 2, 3, 1, 2, 3, 0, 0 };

const int MAP_GRID_PRIOR_INDEX2[] = { 0, 0, 0, 0, 2, 3, 4, 1, 0 };

const int MAP_GRID_PRIOR_DIRECTION2[] = { 0, 1, 2, 3, 0, 1, 2, 3, 0 };

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
	_field32 = 0;
}

MonsterStruct::MonsterStruct(Common::String name, int experience, int hp, int accuracy,
		int speed, int numberOfAttacks, CharacterClass hatesClass, int strikes,
		int dmgPerStrike, DamageType attackType, SpecialAttack specialAttack,
		int hitChance, int rangeAttack, MonsterType monsterType,
		int fireResistence, int electricityResistence, int coldResistence,
		int poisonResistence, int energyResistence, int magicResistence,
		int phsyicalResistence, int field29, int gold, int gems, int itemDrop,
		int flying, int imageNumber, int loopAnimation, int animationEffect,
		int field32, Common::String attackVoc):
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
		_animationEffect(animationEffect), _field32(field32), _attackVoc(attackVoc) {
}


void MonsterStruct::synchronize(Common::SeekableReadStream &s) {
	_experience = s.readByte();
	_hp = s.readByte();
	_accuracy = s.readByte();
	_speed = s.readByte();
	_numberOfAttacks = s.readByte();
	_hatesClass = CLASS_KNIGHT;
	_strikes = s.readByte();
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
	_gold = s.readByte();
	_gems = s.readByte();
	_itemDrop = s.readByte();
	_flying = s.readByte();
	_imageNumber = s.readByte();
	_loopAnimation = s.readByte();
	_animationEffect = s.readByte();
	_field32 = s.readByte();
	
	char attackVoc[9];
	s.read(attackVoc, 9);
	attackVoc[8] = '\0';
	_attackVoc = Common::String(attackVoc);
}

MonsterData::MonsterData() {
	push_back(MonsterStruct("", 0, 0, 0, 0, 0, CLASS_KNIGHT, 1, 1, DT_PHYSICAL,
				   SA_NONE, 1, 0, MONSTER_0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 100, "Slime"));
	push_back(MonsterStruct("Whirlwind", 250000, 1000, 10, 250, 1, CLASS_15, 5,
				   100, DT_PHYSICAL, SA_CONFUSE, 250, 0, MONSTER_0, 100,
				   100, 100, 100, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 176,
				   "airmon"));
	push_back(MonsterStruct("Annihilator", 1000000, 1500, 40, 200, 12, CLASS_16, 5,
				   50, DT_ENERGY, SA_NONE, 1, 1, MONSTER_0, 80, 80, 100,
				   100, 0, 0, 80, 0, 0, 0, 0, 0, 2, 0, 0, 102, "alien1"));
	push_back(MonsterStruct("Autobot", 1000000, 2500, 100, 200, 2, CLASS_16, 5,
				   100, DT_ENERGY, SA_NONE, 1, 0, MONSTER_0, 50, 50, 100,
				   100, 0, 0, 50, 0, 0, 0, 0, 1, 3, 0, 0, 101, "alien2"));
	push_back(MonsterStruct("Sewer Stalker", 50000, 250, 30, 25, 1, CLASS_16, 3,
				   100, DT_PHYSICAL, SA_NONE, 50, 0, MONSTER_ANIMAL, 0,
				   0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 113,
				   "iguana"));
	push_back(MonsterStruct("Armadillo", 60000, 800, 50, 15, 1, CLASS_16, 100, 6,
				   DT_PHYSICAL, SA_BREAKWEAPON, 60, 0, MONSTER_ANIMAL,
				   50, 0, 80, 80, 50, 0, 50, 0, 0, 0, 0, 0, 5, 1, 0, 113,
				   "unnh"));
	push_back(MonsterStruct("Barbarian", 5000, 50, 5, 40, 3, CLASS_SORCERER, 1, 20,
				   DT_PHYSICAL, SA_NONE, 20, 1, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 100, 0, 3, 0, 6, 0, 0, 100,
				   "barbarch"));
	push_back(MonsterStruct("Electrapede", 10000, 200, 10, 50, 1, CLASS_PALADIN,
				   50, 1, DT_ELECTRICAL, SA_PARALYZE, 1, 0,
				   MONSTER_INSECT, 50, 100, 50, 50, 50, 0, 0, 0, 0, 0, 0,
				   0, 7, 1, 0, 107, "centi"));
	push_back(MonsterStruct("Cleric of Mok", 30000, 125, 10, 40, 1, CLASS_CLERIC,
				   250, 1, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID,
				   10, 100, 10, 10, 10, 10, 0, 0, 0, 10, 0, 0, 8, 0, 0,
				   117, "cleric"));
	push_back(MonsterStruct("Mok Heretic", 50000, 150, 12, 50, 1, CLASS_CLERIC,
				   500, 1, DT_1, SA_NONE, 1, 1, MONSTER_HUMANOID, 20, 50,
				   20, 20, 20, 30, 0, 0, 0, 25, 4, 0, 8, 0, 0, 117,
				   "cleric"));
	push_back(MonsterStruct("Mantis Ant", 40000, 300, 30, 40, 2, CLASS_16, 2, 100,
				   DT_PHYSICAL, SA_POISON, 30, 0, MONSTER_INSECT, 0, 0,
				   0, 100, 0, 0, 30, 0, 0, 0, 0, 0, 10, 0, 0, 104,
				   "spell001"));
	push_back(MonsterStruct("Cloud Dragon", 500000, 2000, 40, 150, 1, CLASS_15,
				   600, 1, DT_COLD, SA_NONE, 1, 1, MONSTER_DRAGON, 0, 50,
				   100, 100, 50, 25, 50, 0, 0, 10, 0, 0, 11, 0, 0, 140,
				   "tiger1"));
	push_back(MonsterStruct("Phase Dragon", 2000000, 4000, 80, 200, 1, CLASS_15,
				   750, 1, DT_COLD, SA_NONE, 1, 1, MONSTER_DRAGON, 0, 50,
				   100, 100, 80, 50, 50, 0, 0, 20, 0, 0, 11, 0, 10, 140,
				   "Begger"));
	push_back(MonsterStruct("Green Dragon", 500000, 2500, 50, 150, 1, CLASS_15,
				   500, 1, DT_FIRE, SA_NONE, 1, 1, MONSTER_DRAGON, 100,
				   50, 0, 100, 50, 25, 50, 0, 0, 10, 0, 0, 13, 0, 0, 140,
				   "tiger1"));
	push_back(MonsterStruct("Energy Dragon", 2000000, 5000, 100, 250, 1, CLASS_15,
				   1000, 1, DT_ENERGY, SA_NONE, 1, 1, MONSTER_DRAGON, 80,
				   80, 60, 100, 100, 30, 50, 0, 0, 20, 0, 0, 13, 0, 7,
				   140, "begger"));
	push_back(MonsterStruct("Dragon Mummy", 2000000, 3000, 30, 100, 1,
				   CLASS_CLERIC, 2000, 2, DT_PHYSICAL, SA_DISEASE, 200,
				   0, MONSTER_DRAGON, 0, 80, 100, 100, 0, 10, 90, 0, 0, 0,
				   0, 0, 15, 0, 0, 140, "dragmum"));
	push_back(MonsterStruct("Scraps", 2000000, 3000, 30, 100, 1, CLASS_16, 2000, 2,
				   DT_PHYSICAL, SA_NONE, 200, 0, MONSTER_DRAGON, 0, 80,
				   100, 100, 0, 10, 90, 0, 0, 0, 0, 0, 15, 0, 0, 140,
				   "dragmum"));
	push_back(MonsterStruct("Earth Blaster", 250000, 1000, 10, 100, 1, CLASS_15, 5,
				   100, DT_PHYSICAL, SA_NONE, 200, 0, MONSTER_0, 100, 90,
				   90, 100, 0, 0, 90, 0, 0, 0, 0, 0, 17, 0, 0, 100,
				   "earthmon"));
	push_back(MonsterStruct("Beholder Bat", 10000, 75, 15, 80, 1, CLASS_15, 5, 5,
				   DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 1, 18, 0, 0, 120, "eyeball"));
	push_back(MonsterStruct("Fire Blower", 250000, 1000, 20, 60, 1, CLASS_15, 5,
				   100, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0,
				   100, 50, 0, 50, 0, 0, 0, 0, 0, 19, 0, 0, 110, "fire"));
	push_back(MonsterStruct("Hell Hornet", 50000, 250, 30, 50, 2, CLASS_DRUID, 2,
				   250, DT_POISON, SA_WEAKEN, 1, 0, MONSTER_INSECT, 50,
				   50, 50, 100, 50, 0, 50, 0, 0, 0, 0, 1, 20, 0, 0, 123,
				   "insect"));
	push_back(MonsterStruct("Gargoyle", 30000, 150, 35, 30, 2, CLASS_16, 5, 50,
				   DT_PHYSICAL, SA_NONE, 60, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   20, 0, 0, 0, 0, 0, 0, 21, 0, 10, 100, "gargrwl"));
	push_back(MonsterStruct("Giant", 100000, 500, 25, 45, 2, CLASS_16, 100, 5,
				   DT_PHYSICAL, SA_UNCONSCIOUS, 100, 0, MONSTER_0, 0, 0,
				   0, 0, 0, 0, 0, 0, 1000, 0, 5, 0, 22, 0, 0, 100,
				   "giant"));
	push_back(MonsterStruct("Goblin", 1000, 10, 5, 30, 2, CLASS_16, 2, 6,
				   DT_PHYSICAL, SA_NONE, 1, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 131, "gremlin"));
	push_back(MonsterStruct("Onyx Golem", 1000000, 10000, 50, 100, 1, CLASS_15, 2,
				   250, DT_1, SA_DRAINSP, 1, 0, MONSTER_GOLEM, 100, 100,
				   100, 100, 100, 100, 50, 0, 0, 100, 0, 1, 24, 0, 10,
				   100, "golem"));
	push_back(MonsterStruct("Gremlin", 2000, 20, 7, 35, 2, CLASS_16, 2, 10,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 101, "gremlink"));
	push_back(MonsterStruct("Gremlin Guard", 3000, 50, 10, 35, 2, CLASS_16, 6, 5,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 101, "gremlink"));
	push_back(MonsterStruct("Griffin", 60000, 800, 35, 150, 2, CLASS_KNIGHT, 50, 6,
				   DT_PHYSICAL, SA_NONE, 150, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 80, 0, 0, 0, 0, 0, 0, 27, 0, 0, 120, "screech"));
	push_back(MonsterStruct("Gamma Gazer", 1000000, 5000, 60, 200, 7, CLASS_16, 10,
				   20, DT_ENERGY, SA_NONE, 1, 0, MONSTER_0, 100, 100, 0,
				   100, 100, 0, 60, 0, 0, 0, 0, 0, 28, 0, 0, 140, "hydra"));
	push_back(MonsterStruct("Iguanasaurus", 100000, 2500, 20, 30, 1, CLASS_16, 10,
				   50, DT_PHYSICAL, SA_INSANE, 150, 0, MONSTER_ANIMAL, 50,
				   50, 50, 50, 50, 0, 20, 0, 0, 0, 0, 0, 29, 0, 0, 113,
				   "iguana"));
	push_back(MonsterStruct("Slayer Knight", 50000, 500, 30, 50, 1, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_NONE, 100, 0, MONSTER_HUMANOID,
				   50, 50, 50, 50, 50, 0, 0, 0, 50, 0, 5, 0, 30, 0, 0,
				   141, "knight"));
	push_back(MonsterStruct("Death Knight", 100000, 750, 50, 80, 2, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_NONE, 150, 0, MONSTER_HUMANOID,
				   50, 50, 50, 50, 50, 10, 0, 0, 100, 0, 6, 0, 30, 0, 0,
				   141, "knight"));
	push_back(MonsterStruct("Lava Dweller", 500000, 1500, 30, 40, 1, CLASS_15, 5,
				   100, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 100, 0,
				   100, 50, 0, 50, 0, 0, 0, 0, 0, 19, 0, 0, 110, "fire"));
	push_back(MonsterStruct("Lava Roach", 50000, 500, 20, 70, 1, CLASS_16, 5, 50,
				   DT_FIRE, SA_NONE, 1, 0, MONSTER_INSECT, 100, 100, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, 0, 33, 0, 0, 131, "Phantom"));
	push_back(MonsterStruct("Power Lich", 200000, 500, 20, 60, 1, CLASS_15, 10, 10,
				   DT_1, SA_UNCONSCIOUS, 1, 1, MONSTER_UNDEAD, 0, 0, 0, 0,
				   0, 80, 70, 0, 0, 0, 0, 1, 34, 0, 0, 141, "lich"));
	push_back(MonsterStruct("Mystic Mage", 100000, 200, 20, 70, 1, CLASS_15, 10,
				   20, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 50, 100,
				   50, 50, 50, 30, 0, 0, 0, 50, 0, 1, 35, 0, 0, 163,
				   "monsterb"));
	push_back(MonsterStruct("Magic Mage", 200000, 300, 25, 80, 1, CLASS_15, 10, 30,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 50, 100, 50,
				   50, 50, 50, 0, 0, 0, 75, 0, 1, 35, 0, 0, 163,
				   "monsterb"));
	push_back(MonsterStruct("Minotaur", 250000, 3000, 80, 120, 1, CLASS_16, 100, 4,
				   DT_PHYSICAL, SA_AGING, 150, 0, MONSTER_0, 0, 0, 10, 0,
				   0, 50, 60, 0, 0, 0, 0, 0, 37, 0, 0, 141, "stonegol"));
	push_back(MonsterStruct("Gorgon", 250000, 4000, 90, 100, 1, CLASS_16, 100, 3,
				   DT_PHYSICAL, SA_STONE, 100, 0, MONSTER_0, 0, 0, 0, 0,
				   0, 60, 70, 0, 0, 0, 0, 0, 37, 0, 0, 141, "stonegol"));
	push_back(MonsterStruct("Higher Mummy", 100000, 400, 20, 60, 1, CLASS_CLERIC,
				   10, 40, DT_PHYSICAL, SA_CURSEITEM, 100, 0,
				   MONSTER_UNDEAD, 0, 50, 50, 100, 50, 20, 75, 0, 0, 0, 0,
				   0, 39, 0, 0, 141, "mummy"));
	push_back(MonsterStruct("Orc Guard", 5000, 60, 10, 20, 1, CLASS_12, 3, 10,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 50, 0, 2, 0, 40, 0, 0, 125, "orc"));
	push_back(MonsterStruct("Octopod", 250000, 2500, 40, 80, 1, CLASS_15, 2, 100,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_ANIMAL, 0, 0, 50,
				   100, 0, 0, 0, 0, 0, 0, 0, 1, 41, 0, 0, 101, "photon"));
	push_back(MonsterStruct("Ogre", 10000, 100, 15, 30, 1, CLASS_16, 4, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 100, 0, 0, 0, 42, 0, 0, 136, "ogre"));
	push_back(MonsterStruct("Orc Shaman", 10000, 50, 15, 30, 1, CLASS_15, 5, 5,
				   DT_COLD, SA_SLEEP, 1, 1, MONSTER_HUMANOID, 0, 0, 0, 0,
				   0, 10, 0, 0, 75, 10, 2, 0, 43, 0, 0, 125, "fx7"));
	push_back(MonsterStruct("Sabertooth", 10000, 100, 20, 60, 3, CLASS_16, 5, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 44, 1, 0, 101, "saber"));
	push_back(MonsterStruct("Sand Flower", 10000, 100, 10, 50, 5, CLASS_16, 5, 5,
				   DT_PHYSICAL, SA_INLOVE, 50, 0, MONSTER_0, 0, 0, 0, 0,
				   0, 50, 50, 0, 0, 0, 0, 0, 45, 0, 0, 106, "sand"));
	push_back(MonsterStruct("Killer Cobra", 25000, 1000, 25, 100, 1, CLASS_16, 2,
				   100, DT_PHYSICAL, SA_AGING, 30, 0, MONSTER_ANIMAL, 0,
				   0, 0, 100, 0, 50, 0, 0, 0, 0, 0, 0, 46, 0, 0, 100,
				   "hiss"));
	push_back(MonsterStruct("Sewer Rat", 2000, 40, 5, 35, 1, CLASS_16, 3, 10,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 47, 0, 0, 136, "rat"));
	push_back(MonsterStruct("Sewer Slug", 1000, 25, 2, 25, 1, CLASS_16, 2, 10,
				   DT_PHYSICAL, SA_NONE, 5, 0, MONSTER_INSECT, 0, 0, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 111, "zombie"));
	push_back(MonsterStruct("Skeletal Lich", 500000, 2000, 30, 200, 1,
				   CLASS_SORCERER, 1000, 1, DT_ENERGY, SA_ERADICATE, 1, 1,
				   MONSTER_UNDEAD, 80, 70, 80, 100, 100, 50, 50, 0, 0, 0,
				   0, 0, 49, 0, 0, 140, "elecbolt"));
	push_back(MonsterStruct("Enchantress", 40000, 100, 25, 60, 1, CLASS_CLERIC, 3,
				   150, DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID,
				   10, 100, 10, 10, 10, 20, 0, 0, 0, 20, 0, 0, 50, 0, 0,
				   163, "disint"));
	push_back(MonsterStruct("Sorceress", 80000, 200, 30, 80, 1, CLASS_15, 2, 50,
				   DT_1, SA_NONE, 1, 1, MONSTER_HUMANOID, 10, 20, 10, 10,
				   10, 80, 0, 0, 0, 50, 5, 0, 50, 0, 0, 163, "disint"));
	push_back(MonsterStruct("Arachnoid", 4000, 50, 10, 40, 1, CLASS_16, 3, 5,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_INSECT, 0, 0, 0,
				   100, 0, 0, 0, 0, 0, 0, 0, 0, 52, 0, 0, 104, "web"));
	push_back(MonsterStruct("Medusa Sprite", 5000, 30, 5, 30, 1, CLASS_RANGER, 3,
				   3, DT_PHYSICAL, SA_STONE, 10, 0, MONSTER_0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 1, 53, 0, 0, 42, "hiss"));
	push_back(MonsterStruct("Rogue", 5000, 50, 10, 30, 1, CLASS_ROBBER, 1, 60,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 70, 0, 0, 0, 54, 0, 0, 100, "thief"));
	push_back(MonsterStruct("Thief", 10000, 100, 15, 40, 1, CLASS_ROBBER, 1, 100,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 200, 0, 0, 0, 54, 0, 0, 100,
				   "thief"));
	push_back(MonsterStruct("Troll Grunt", 10000, 100, 5, 50, 1, CLASS_16, 2, 25,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, 0, 56, 0, 0, 136, "troll"));
	push_back(MonsterStruct("Vampire", 200000, 400, 30, 80, 1, CLASS_CLERIC, 10,
				   10, DT_PHYSICAL, SA_WEAKEN, 100, 0, MONSTER_UNDEAD, 50,
				   50, 50, 50, 50, 50, 50, 0, 0, 0, 0, 0, 57, 0, 0, 42,
				   "vamp"));
	push_back(MonsterStruct("Vampire Lord", 300000, 500, 35, 100, 1, CLASS_CLERIC,
				   10, 30, DT_PHYSICAL, SA_SLEEP, 120, 0, MONSTER_UNDEAD,
				   50, 50, 50, 50, 50, 50, 70, 0, 0, 0, 0, 0, 58, 0, 0,
				   42, "vamp"));
	push_back(MonsterStruct("Vulture Roc", 200000, 2500, 50, 150, 1, CLASS_16, 5,
				   60, DT_PHYSICAL, SA_NONE, 100, 0, MONSTER_ANIMAL, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 59, 0, 0, 120, "vulture"));
	push_back(MonsterStruct("Sewer Hag", 50000, 75, 10, 40, 1, CLASS_PALADIN, 10,
				   25, DT_ELECTRICAL, SA_INSANE, 1, 1, MONSTER_HUMANOID,
				   0, 100, 0, 100, 0, 20, 0, 0, 0, 10, 0, 0, 62, 0, 0,
				   108, "elecspel"));
	push_back(MonsterStruct("Tidal Terror", 500000, 1000, 10, 200, 1, CLASS_15, 5,
				   100, DT_COLD, SA_NONE, 1, 0, MONSTER_0, 100, 50, 50,
				   100, 50, 0, 100, 0, 0, 0, 0, 1, 61, 0, 0, 101,
				   "splash3"));
	push_back(MonsterStruct("Witch", 80000, 150, 15, 70, 1, CLASS_15, 10, 10,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_HUMANOID, 0, 100,
				   0, 20, 0, 20, 0, 0, 0, 10, 0, 0, 63, 0, 0, 114,
				   "elecspel"));
	push_back(MonsterStruct("Coven Leader", 120000, 250, 20, 100, 1, CLASS_15, 10,
				   15, DT_ENERGY, SA_DRAINSP, 1, 1, MONSTER_HUMANOID, 10,
				   100, 0, 50, 100, 50, 0, 0, 0, 20, 6, 0, 63, 0, 10, 114,
				   "elecspel"));
	push_back(MonsterStruct("Master Wizard", 120000, 500, 25, 150, 2, CLASS_KNIGHT,
				   10, 40, DT_FIRE, SA_NONE, 1, 1, MONSTER_HUMANOID, 100,
				   50, 50, 50, 50, 50, 0, 0, 0, 50, 0, 0, 64, 0, 0, 163,
				   "boltelec"));
	push_back(MonsterStruct("Wizard", 60000, 250, 20, 125, 1, CLASS_PALADIN, 10,
				   25, DT_1, SA_NONE, 1, 1, MONSTER_HUMANOID, 50, 30, 30,
				   30, 30, 30, 0, 0, 0, 20, 0, 0, 65, 0, 0, 163, "wizard"));
	push_back(MonsterStruct("Dark Wolf", 10000, 70, 10, 70, 3, CLASS_16, 3, 8,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_ANIMAL, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 66, 1, 0, 100, "wolf"));
	push_back(MonsterStruct("Screamer", 500000, 3000, 50, 200, 1, CLASS_15, 10, 20,
				   DT_POISON, SA_POISON, 1, 0, MONSTER_0, 0, 0, 0, 100, 0,
				   0, 60, 0, 0, 0, 0, 0, 67, 0, 0, 110, "dragon"));
	push_back(MonsterStruct("Cult Leader", 100000, 100, 20, 60, 1, CLASS_15, 10,
				   10, DT_ENERGY, SA_NONE, 1, 1, MONSTER_HUMANOID, 50, 50,
				   50, 50, 100, 50, 0, 0, 0, 100, 6, 0, 8, 0, 0, 100,
				   "cleric"));
	push_back(MonsterStruct("Mega Dragon", 100000000, 64000, 100, 200, 1, CLASS_15,
				   10, 200, DT_ENERGY, SA_ERADICATE, 1, 1, MONSTER_DRAGON,
				   100, 100, 100, 100, 100, 100, 90, 0, 0, 232, 0, 0, 11,
				   0, 7, 100, "tiger1"));
	push_back(MonsterStruct("Gettlewaithe", 5000, 100, 15, 35, 2, CLASS_16, 5, 5,
				   DT_PHYSICAL, SA_NONE, 10, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 2000, 0, 5, 0, 25, 0, 0, 100, "gremlin"));
	push_back(MonsterStruct("Doom Knight", 500000, 1000, 50, 100, 4, CLASS_PALADIN,
				   2, 250, DT_PHYSICAL, SA_DEATH, 150, 0,
				   MONSTER_HUMANOID, 80, 80, 80, 80, 80, 20, 0, 0, 200,
				   0, 7, 0, 30, 0, 10, 100, "knight"));
	push_back(MonsterStruct("Sandro", 200000, 1000, 20, 75, 1, CLASS_15, 10, 10,
				   DT_1, SA_DEATH, 1, 1, MONSTER_UNDEAD, 0, 0, 0, 0, 0,
				   90, 80, 0, 0, 100, 7, 1, 34, 0, 10, 100, "lich"));
	push_back(MonsterStruct("Mega Mage", 500000, 500, 35, 100, 1, CLASS_15, 10, 40,
				   DT_ELECTRICAL, SA_NONE, 1, 1, MONSTER_0, 80, 100, 80,
				   80, 80, 80, 0, 0, 0, 100, 6, 1, 35, 0, 11, 100,
				   "monsterb"));
	push_back(MonsterStruct("Orc Elite", 15000, 200, 15, 40, 2, CLASS_12, 5, 10,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 100, 0, 3, 0, 40, 0, 0, 100, "orc"));
	push_back(MonsterStruct("Shaalth", 20000, 300, 15, 50, 1, CLASS_15, 5, 10,
				   DT_COLD, SA_SLEEP, 1, 0, MONSTER_HUMANOID, 0, 0, 0, 0,
				   0, 20, 0, 0, 1000, 50, 5, 0, 43, 0, 10, 100, "fx7"));
	push_back(MonsterStruct("Rooka", 5000, 60, 5, 40, 1, CLASS_16, 3, 10,
				   DT_PHYSICAL, SA_DISEASE, 15, 0, MONSTER_ANIMAL, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 10, 4, 0, 47, 0, 0, 100, "rat"));
	push_back(MonsterStruct("Morgana", 200000, 300, 35, 100, 1, CLASS_15, 2, 60,
				   DT_ENERGY, SA_PARALYZE, 1, 1, MONSTER_HUMANOID, 50, 50,
				   50, 50, 100, 80, 0, 0, 0, 100, 6, 0, 50, 0, 10, 100,
				   "disint"));
	push_back(MonsterStruct("Master Thief", 20000, 100, 20, 50, 1, CLASS_ROBBER, 1,
				   250, DT_PHYSICAL, SA_NONE, 40, 0, MONSTER_HUMANOID, 0,
				   0, 0, 0, 0, 0, 0, 0, 250, 20, 4, 0, 54, 0, 14, 100,
				   "thief"));
	push_back(MonsterStruct("Royal Vampire", 400000, 750, 40, 125, 1, CLASS_CLERIC,
				   10, 50, DT_PHYSICAL, SA_CURSEITEM, 120, 0,
				   MONSTER_UNDEAD, 50, 50, 50, 50, 50, 50, 65, 0, 0, 0, 0,
				   0, 57, 0, 0, 100, "vamp"));
	push_back(MonsterStruct("Ct. Blackfang", 2000000, 1500, 50, 150, 1,
				   CLASS_CLERIC, 10, 100, DT_PHYSICAL, SA_DEATH, 120, 0,
				   MONSTER_UNDEAD, 75, 75, 75, 75, 75, 75, 75, 0, 0, 0, 0,
				   0, 58, 0, 10, 100, "vamp"));
	push_back(MonsterStruct("Troll Guard", 15000, 200, 10, 60, 1, CLASS_16, 2, 35,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, 0, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Troll Chief", 20000, 300, 15, 65, 1, CLASS_16, 2, 50,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 0, 0, 0, 0, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Hobstadt", 25000, 400, 20, 70, 1, CLASS_16, 2, 50,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 50, 50, 50,
				   50, 0, 0, 0, 0, 1000, 0, 4, 0, 56, 0, 0, 100, "troll"));
	push_back(MonsterStruct("Graalg", 20000, 200, 15, 50, 1, CLASS_16, 5, 10,
				   DT_PHYSICAL, SA_NONE, 30, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 1000, 0, 5, 0, 42, 0, 0, 100, "ogre"));
	push_back(MonsterStruct("Vampire King", 3000000, 10000, 60, 200, 1,
				   CLASS_CLERIC, 10, 250, DT_PHYSICAL, SA_ERADICATE, 150,
				   0, MONSTER_UNDEAD, 80, 80, 80, 80, 80, 80, 90, 0, 0, 0,
				   0, 0, 58, 0, 0, 100, "vamp"));
	push_back(MonsterStruct("Valio", 60000, 150, 15, 60, 1, CLASS_PALADIN, 10, 25,
				   DT_1, SA_NONE, 1, 0, MONSTER_HUMANOID, 50, 30, 30, 30,
				   40, 30, 0, 0, 0, 0, 0, 0, 65, 0, 0, 100, "wizard"));
	push_back(MonsterStruct("Sky Golem", 200000, 1000, 50, 100, 1, CLASS_15, 2,
				   100, DT_COLD, SA_NONE, 1, 1, MONSTER_GOLEM, 50, 50,
				   100, 50, 50, 50, 50, 0, 0, 0, 0, 1, 24, 0, 0, 100,
				   "golem"));
	push_back(MonsterStruct("Gurodel", 100000, 750, 30, 60, 2, CLASS_16, 100, 6,
				   DT_PHYSICAL, SA_UNCONSCIOUS, 110, 0, MONSTER_0, 0, 0,
				   0, 0, 0, 0, 0, 0, 5000, 0, 6, 0, 22, 0, 0, 100,
				   "giant"));
	push_back(MonsterStruct("Yog", 25000, 100, 5, 60, 1, CLASS_SORCERER, 1, 30,
				   DT_PHYSICAL, SA_NONE, 25, 0, MONSTER_HUMANOID, 0, 0,
				   0, 0, 0, 0, 0, 0, 200, 0, 4, 0, 6, 0, 10, 100,
				   "barbarch"));
	push_back(MonsterStruct("Sharla", 10000, 50, 5, 50, 1, CLASS_RANGER, 3, 4,
				   DT_PHYSICAL, SA_NONE, 20, 0, MONSTER_0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 1, 53, 0, 0, 100, "hiss"));
	push_back(MonsterStruct("Ghost Mummy", 500000, 500, 35, 175, 1, CLASS_CLERIC,
				   200, 5, DT_PHYSICAL, SA_AGING, 150, 0, MONSTER_UNDEAD,
				   0, 60, 80, 80, 80, 50, 80, 0, 0, 0, 0, 0, 40, 0, 6,
				   100, "orc"));
	push_back(MonsterStruct("Phase Mummy", 500000, 500, 35, 175, 1, CLASS_CLERIC,
				   200, 6, DT_PHYSICAL, SA_DRAINSP, 150, 0,
				   MONSTER_UNDEAD, 0, 70, 80, 80, 80, 60, 85, 0, 0, 0, 0,
				   0, 39, 0, 7, 100, "mummy"));
	push_back(MonsterStruct("Xenoc", 250000, 700, 35, 175, 1, CLASS_15, 10, 50,
				   DT_ENERGY, SA_NONE, 1, 0, MONSTER_HUMANOID, 50, 50, 50,
				   50, 100, 50, 0, 0, 0, 100, 6, 0, 64, 0, 0, 100,
				   "boltelec"));
	push_back(MonsterStruct("Barkman", 4000000, 40000, 25, 100, 3, CLASS_16, 250,
				   1, DT_FIRE, SA_NONE, 1, 0, MONSTER_0, 100, 50, 0, 100,
				   0, 0, 0, 0, 0, 0, 6, 0, 19, 0, 11, 100, "fire"));
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
	_north = 0;
	_east = 0;
	_south = 0;
	_west = 0;
}

void SurroundingMazes::synchronize(Common::SeekableReadStream &s) {
	_north = s.readUint16LE();
	_east = s.readUint16LE();
	_south = s.readUint16LE();
	_west = s.readUint16LE();
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
}

void MazeDifficulties::synchronize(Common::SeekableReadStream &s) {
	_wallNoPass = s.readByte();
	_surfaceNoPass = s.readByte();
	_unlockDoor = s.readByte();
	_unlockBox = s.readByte();
	_bashDoor = s.readByte();
	_bashGrate = s.readByte();
	_bashWall = s.readByte();
	_chance2Run = s.readByte();
}

/*------------------------------------------------------------------------*/

MazeData::MazeData() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		Common::fill(&_wallData[y][0], &_wallData[y][MAP_WIDTH], 0);
		Common::fill(&_cellFlag[y][0], &_cellFlag[y][MAP_WIDTH], 0);
		Common::fill(&_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH], 0);
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], 0);
		_wallTypes[y] = 0;
		_surfaceTypes[y] = 0;
	}
	_mazeNumber = 0;
	_mazeFlags = _mazeFlags2  = 0;
	_floorType = 0;
	_trapDamage = 0;
	_wallKind = 0;
	_tavernTips = 0;
}

void MazeData::synchronize(Common::SeekableReadStream &s) {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_wallData[y][x] = s.readUint16LE();
	}
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_cellFlag[y][x] = s.readByte();
	}

	_mazeNumber = s.readUint16LE();
	_surroundingMazes.synchronize(s);
	_mazeFlags = s.readUint16LE();
	_mazeFlags2 = s.readUint16LE();

	for (int i = 0; i < 16; ++i)
		_wallTypes[i] = s.readByte();
	for (int i = 0; i < 16; ++i)
		_surfaceTypes[i] = s.readByte();

	_floorType = s.readByte();
	_runPosition.x = s.readByte();
	_difficulties.synchronize(s);
	_runPosition.y = s.readByte();
	_trapDamage = s.readByte();
	_wallKind = s.readByte();
	_tavernTips = s.readByte();

	Common::Serializer ser(&s, nullptr);
	for (int y = 0; y < MAP_HEIGHT; ++y)
		SavesManager::syncBitFlags(ser, &_seenTiles[y][0], &_seenTiles[y][MAP_WIDTH]);
	for (int y = 0; y < MAP_HEIGHT; ++y)
		SavesManager::syncBitFlags(ser, &_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH]);
}

/**
 * Flags all tiles for the map as having been stepped on
 */
void MazeData::setAllTilesStepped() {
	for (int y = 0; y < MAP_HEIGHT; ++y)
		Common::fill(&_steppedOnTiles[y][0], &_steppedOnTiles[y][MAP_WIDTH], true);
}

void MazeData::clearCellBits() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_cellFlag[y][x] &= 0xF8;
	}
}

/*------------------------------------------------------------------------*/

MobStruct::MobStruct() {
	_id = 0;
	_direction = DIR_NORTH;
}

bool MobStruct::synchronize(Common::SeekableReadStream &s) {
	_pos.x = (int8)s.readByte();
	_pos.y = (int8)s.readByte();
	_id = s.readByte();
	_direction = (Direction)s.readByte();

	return _id != 0xff || _pos.x != -1 || _pos.y != -1;
}

/*------------------------------------------------------------------------*/

MazeObject::MazeObject() {
	_number = 0;
	_frame = 0;
	_id = 0;
	_direction = DIR_NORTH;
	_flipped = false;
}	

/*------------------------------------------------------------------------*/

MazeMonster::MazeMonster() {
	_frame = 0;
	_id = 0;
	_refId = 0;
	_hp = 0;
	_effect1 = _effect2 = 0;
	_effect3 = 0;
}

/*------------------------------------------------------------------------*/

MazeWallItem::MazeWallItem() {
	_id = 0;
	_direction = DIR_NORTH;
}

/*------------------------------------------------------------------------*/

MonsterObjectData::MonsterObjectData(XeenEngine *vm): _vm(vm) {
}

void MonsterObjectData::synchronize(Common::SeekableReadStream &s, 
		bool isOutdoors, MonsterData monsterData) {
	_objects.clear();
	_monsters.clear();
	_wallItems.clear();
	Common::Array<int> objectSprites;
	Common::Array<int> monsterIds;
	Common::Array<int> wallPicIds;
	Common::Array<MobStruct> objData;
	Common::Array<MobStruct> monData;
	Common::Array<MobStruct> wallItemData;
	byte b;

	for (int i = 0; i < 16; ++i) {
		if ((b = s.readByte()) != 0xff)
			objectSprites.push_back(b);
	}
	for (int i = 0; i < 16; ++i) {
		if ((b = s.readByte()) != 0xff)
			monsterIds.push_back(b);
	}
	for (int i = 0; i < 16; ++i) {
		if ((b = s.readByte()) != 0xff)
			wallPicIds.push_back(b);
	}

	MobStruct mobStruct;
	while (mobStruct.synchronize(s))
		objData.push_back(mobStruct);
	while (mobStruct.synchronize(s))
		monData.push_back(mobStruct);
	if (!isOutdoors) {
		while (mobStruct.synchronize(s))
			wallItemData.push_back(mobStruct);
	}

	// Merge up objects
	_objects.resize(objData.size());
	for (uint i = 0; i < objData.size(); ++i) {
		MazeObject &dest = _objects[i];
		dest._position = objData[i]._pos;
		dest._number = objData[i]._id;
		dest._id = objectSprites[dest._number];
		dest._direction = objData[i]._direction;
		dest._frame = 100;
	}

	// merge up monsters
	_monsters.resize(monData.size());
	for (uint i = 0; i < monData.size(); ++i) {
		MazeMonster &dest = _monsters[i];
		dest._position = monData[i]._pos;
		dest._id = monData[i]._id;
		dest._refId = monsterIds[dest._id];
		
		MonsterStruct &mon = monsterData[dest._refId];
		dest._hp = mon._hp;
		dest._frame = _vm->getRandomNumber(7);
		dest._effect1 = dest._effect2 = mon._animationEffect;
		if (mon._animationEffect)
			dest._effect3 = _vm->getRandomNumber(7);
	}

	// Merge up wall items
	_wallItems.clear();
	for (uint i = 0; i < wallItemData.size(); ++i) {
		MazeWallItem &dest = _wallItems[i];
		dest._position = wallItemData[i]._pos;
		dest._id = wallItemData[i]._id;
		dest._refId = wallPicIds[dest._id];
		dest._direction = wallItemData[i]._direction;
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

Map::Map(XeenEngine *vm) : _vm(vm), _mobData(vm) {
	_townPortalSide = 0;
	_sideObj = 0;
	_sideMon = 0;
	_isOutdoors = false;
	_stepped = false;
}

void Map::load(int mapId) {
	Screen &screen = *_vm->_screen;
	if (_vm->_falling) {
		Window &w = screen._windows[9];
		w.open();
		w.writeString(OOPS);
	} else {
		PleaseWait::show(_vm);
	}

	_stepped = true;
	_vm->_party._mazeId = mapId;
	_vm->_events->clearEvents();


	if (mapId >= 113 && mapId <= 127) {
		_townPortalSide = 0;
	} else {
		_townPortalSide = _vm->_loadDarkSide;
	}

	if (_vm->getGameID() == GType_WorldOfXeen) {
		if (_vm->_loadDarkSide) {
			_objPicSprites.load("clouds.dat");
			_monsterData.load("xeen.mon");
			_wallPicSprites.load("xeenpic.dat");
		} else {
			switch (mapId) {
			case 113:
			case 114:
			case 115:
			case 116:
			case 128:
				_objPicSprites.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObj = 0;
				break;
			case 117:
			case 118:
			case 119:
			case 120:
			case 124:
				_objPicSprites.load("clouds.dat");
				_monsterData.load("xeen.mon");
				_wallPicSprites.load("darkpic.dat");
				_sideObj = 0;
				_sideMon = 0;
				break;
			case 125:
			case 126:
			case 127:
				_objPicSprites.load("clouds.dat");
				_monsterData.load("dark.mon");
				_wallPicSprites.load("xeenpic.dat");
				break;
			default:
				_objPicSprites.load("dark.dat");
				_monsterData.load("ddark.mon");
				_wallPicSprites.load("darkpic.dat");
				break;
			}
		}
	}

	bool isDarkCc = _vm->getGameID() == GType_DarkSide;
	MazeData *mazeData = &_mazeData[0];
	bool textLoaded = false;

	// Iterate through loading the given maze as well as the two successive
	// mazes in each of the four cardinal directions
	for (int idx = 0; idx < 9; ++idx, ++mazeData) {
		mazeData->_mazeId = mapId;

		if (mapId != 0) {
			// Load in the maze's data file
			Common::String datName = Common::String::format("maze%c%03u.dat",
				(_vm->_party._mazeId >= 100) ? 'x' : '0', _vm->_party._mazeId);
			File datFile(datName);
			mazeData->synchronize(datFile);
			datFile.close();

			if (isDarkCc && mapId == 50)
				mazeData->setAllTilesStepped();
			if (!isDarkCc && _vm->_party._gameFlags[25] &&
					(mapId == 42 || mapId == 43 || mapId == 4)) {
				mazeData->clearCellBits();
			}

			_isOutdoors = (mazeData->_mazeFlags2 & FLAG_IS_OUTDOORS) != 0;

			// Handle loading text data
			if (!textLoaded) {
				textLoaded = true;
				Common::String txtName = Common::String::format("%s%c%03u.txt",
					isDarkCc ? "dark" : "xeen", mapId >= 100 ? 'x' : '0', mapId);
				File fText(txtName);
				char mazeName[33];
				fText.read(mazeName, 33);
				mazeName[32] = '\0';

				_mazeName = Common::String(mazeName);
				fText.close();

				// Load the monster data
				Common::String mobName = Common::String::format("maze%c%03u.mob",
					(mapId >= 100) ? 'x' : '0', mapId);
				File mobFile(mobName);
				_mobData.synchronize(mobFile, _isOutdoors, _monsterData);
				mobFile.close();

				Common::String headName = Common::String::format("aaze%c%03u.hed",
					(mapId >= 100) ? 'x' : '0', mapId);
				File headFile(headName);
				_headData.synchronize(headFile);
				headFile.close();

				if (!isDarkCc && _vm->_party._mazeId)
					_mobData._monsters.clear();

				if (!isDarkCc && mapId == 15) {
					if ((_mobData._monsters[0]._position.x > 31 || _mobData._monsters[0]._position.y > 31) &&
						(_mobData._monsters[1]._position.x > 31 || _mobData._monsters[1]._position.y > 31) &&
						(_mobData._monsters[2]._position.x > 31 || _mobData._monsters[2]._position.y > 31)) {
						_vm->_party._gameFlags[56] = true;
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

	// TODO: Switch setting flags that don't seem to ever be used


	// Load secondary bj file for the objects
	Common::String filename;
	for (uint i = 0; i < _mobData._objects.size(); ++i) {
		if (_vm->_party._cloudsEnd && _mobData._objects[i]._id == 85 &&
				_vm->_party._mazeId == 27 && isDarkCc) {
			// TODO: Flags set that don't seem to be used
		} else if (_vm->_party._mazeId == 12 && _vm->_party._gameFlags[43] &&
				_mobData._objects[i]._id == 118 && !isDarkCc) {
			filename = "085.obj";
			_mobData._objects[0]._id = 85;
		} else {
			filename = Common::String::format("%03u.%cbj", _mobData._objects[i]._id,
				_mobData._objects[i]._id >= 100 ? 'o' : '0');
		}

		// Read in the secondary object data
		File f(filename);
		Common::Array<byte> &b = _mobData._objects[i]._objBj;
		b.resize(f.size());
		f.read(&b[0], f.size());
	}

	// Load sprite resources for monster standard and attack animations
	int monsterNum = 1;
	int monsterImgNums[95];
	Common::fill(&monsterImgNums[0], &monsterImgNums[95], 0);
	for (uint i = 0; i < _mobData._monsters.size(); ++i, ++monsterNum) {
		MonsterStruct &monsterStruct = _monsterData[i];
		int monsterImgNum = monsterStruct._imageNumber;
		filename = Common::String::format("%03u.mon", monsterImgNum);

		if (!monsterImgNums[monsterImgNum]) {
			_mobData._monsters[i]._sprites.load(filename);
		} else {
			_mobData._monsters[i]._sprites = _mobData._monsters[
				monsterImgNums[monsterImgNum] - 1]._sprites;
		}

		filename = Common::String::format("%03u.att", monsterImgNum);
		if (!monsterImgNums[monsterImgNum]) {
			_mobData._monsters[i]._attackSprites.load(filename);
			monsterImgNums[monsterImgNum] = monsterNum;
		} else {
			_mobData._monsters[i]._attackSprites = _mobData._monsters[
				monsterImgNums[monsterImgNum] - 1]._attackSprites;;
		}
	}

	// Load wall picture sprite resources
	for (uint i = 0; i < _mobData._wallItems.size(); ++i) {
		filename = Common::String::format("%03u.pic", _mobData._wallItems[i]._refId);
		_mobData._wallItems[i]._sprites.load(filename);
	}
}

} // End of namespace Xeen
