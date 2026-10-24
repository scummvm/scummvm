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

#include "mm/mm1/game/spells_monsters.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Game {

const SpellMonstersSpell SpellsMonsters::SPELLS[MONSTER_SPELLS_COUNT] = {
	&SpellsMonsters::spell01_curse,
	&SpellsMonsters::spell02_energyBlast,
	&SpellsMonsters::spell03_fire,
	&SpellsMonsters::spell04_blindness,
	&SpellsMonsters::spell05_sprayPoison,
	&SpellsMonsters::spell06_sprayAcid,
	&SpellsMonsters::spell07_sleep,
	&SpellsMonsters::spell08_paralyze,
	&SpellsMonsters::spell09_dispel,
	&SpellsMonsters::spell10_lightningBolt,
	&SpellsMonsters::spell11_strangeGas,
	&SpellsMonsters::spell12_explode,
	&SpellsMonsters::spell13_fireball,
	&SpellsMonsters::spell14_fireBreath,
	&SpellsMonsters::spell15_gazes,
	&SpellsMonsters::spell16_acidArrow,
	&SpellsMonsters::spell17_elements,
	&SpellsMonsters::spell18_coldBeam,
	&SpellsMonsters::spell19_dancingSword,
	&SpellsMonsters::spell20_magicDrain,
	&SpellsMonsters::spell21_fingerOfDeath,
	&SpellsMonsters::spell22_sunRay,
	&SpellsMonsters::spell23_disintegration,
	&SpellsMonsters::spell24_commandsEnergy,
	&SpellsMonsters::spell25_poison,
	&SpellsMonsters::spell26_lightning,
	&SpellsMonsters::spell27_frost,
	&SpellsMonsters::spell28_spikes,
	&SpellsMonsters::spell29_acid,
	&SpellsMonsters::spell30_fire,
	&SpellsMonsters::spell31_energy,
	&SpellsMonsters::spell32_swarm
};

SpellsMonsters::SpellsMonsters() {
}

void SpellsMonsters::castMonsterSpell(const Common::String &monsterName, int spellNum) {
	g_globals->_spellsState._mmVal1 = 0;
	g_globals->_spellsState._resistanceIndex = 0;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_MAGIC;
	g_globals->_spellsState._damage = 0;

	// All spell messages start with the monster who casts it
	_lines.clear();
	_lines.push_back(Line(monsterName));
	add(' ');

	(this->*SPELLS[spellNum - 1])();
}

void SpellsMonsters::spell01_curse() {
	if (casts()) {
		add("a curse");
		g_globals->_activeSpells._s.cursed = MIN(255,
			(int)g_globals->_activeSpells._s.cursed + 1);
	}
}

void SpellsMonsters::spell02_energyBlast() {
	if (casts()) {		
		add("energy blast");
		++g_globals->_spellsState._mmVal1;
		g_globals->_spellsState._damage = getRandomNumber(16) + 4;
		damageRandomChar();
	}
}

void SpellsMonsters::spell03_fire() {
	add("breathes fire");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_FIRE;

	int count = g_globals->_encounters._monsterList[getMonsterIndex()]._level;
	g_globals->_spellsState._damage += count * 6;

	damageRandomChar();
}

void SpellsMonsters::spell04_blindness() {
	if (casts()) {
		add("blindness");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._damage = 2;

		handlePartyEffects();
	}
}

void SpellsMonsters::spell05_sprayPoison() {
	add("sprays poison");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_POISON;
	g_globals->_spellsState._damage = POISONED;
	handlePartyEffects();
}

void SpellsMonsters::spell06_sprayAcid() {
	add("sprays acid");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ACID;
	g_globals->_spellsState._damage = getRandomNumber((int)POISONED);
	add(':');

	handlePartyDamage();
}

void SpellsMonsters::spell07_sleep() {
	if (casts()) {
		add("sleep");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_PSYCHIC;
		g_globals->_spellsState._damage = ASLEEP;
		handlePartyEffects();
	}
}

void SpellsMonsters::spell08_paralyze() {
	if (casts()) {
		add("paralyze");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_FEAR;
		g_globals->_spellsState._damage = PARALYZED;

		if (g_globals->_encounters._monsterList[getMonsterIndex()]._level >= 5) {
			handlePartyEffects();
		} else {
			chooseCharacter();
			writeConditionEffect();
		}
	}
}

void SpellsMonsters::spell09_dispel() {
	if (casts()) {
		add("dispel");
		dispelParty();
	}
}

void SpellsMonsters::spell10_lightningBolt() {
	if (casts()) {
		add("lightning bolt");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ELECTRICITY;
		g_globals->_spellsState._damage = getRandomNumber(37) + 5;
		damageRandomChar();
	}
}

void SpellsMonsters::spell11_strangeGas() {
	add("breathes strange gas");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_POISON;
	g_globals->_spellsState._damage = BAD_CONDITION | STONE;

	handlePartyEffects();
}

void SpellsMonsters::spell12_explode() {
	add("explode");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_POISON;
	g_globals->_spellsState._damage = getRandomNumber(
		g_globals->_encounters._monsterList[getMonsterIndex()]._level);
	g_globals->_encounters._monsterList[getMonsterIndex()]._level = 0;
	g_globals->_encounters._monsterList[getMonsterIndex()]._status = MONFLAG_DEAD;
	removeMonster();

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell13_fireball() {
	if (casts()) {
		add("fireball");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_FIRE;

		// This whole condition choice makes no sense
		g_globals->_spellsState._damage += 6 *
			g_globals->_encounters._monsterList[getMonsterIndex()]._level;
		g_globals->_spellsState._damage = getRandomNumber(g_globals->_spellsState._damage) + 4;

		add(':');
		handlePartyDamage();
	}
}

void SpellsMonsters::spell14_fireBreath() {
	add("breathes fire");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_FIRE;

	// This whole condition choice makes no sense
	g_globals->_spellsState._damage += 8 *
		g_globals->_encounters._monsterList[getMonsterIndex()]._level;
	g_globals->_spellsState._damage = getRandomNumber(g_globals->_spellsState._damage);

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell15_gazes() {
	add("gazes");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._damage = BAD_CONDITION | STONE;

	chooseCharacter();
	writeConditionEffect();
}

void SpellsMonsters::spell16_acidArrow() {
	add("acid arrow");
	++g_globals->_spellsState._mmVal1;
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ACID;
	g_globals->_spellsState._damage = getRandomNumber(31) + 9;

	damageRandomChar();
}

void SpellsMonsters::spell17_elements() {
	add("calls upon the elements");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._damage = getRandomNumber(21) + 39;

	damageRandomChar();
}

void SpellsMonsters::spell18_coldBeam() {
	if (casts()) {
		add("cold beam");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_COLD;
		g_globals->_spellsState._damage = getRandomNumber(41) + 9;

		damageRandomChar();
	}
}

void SpellsMonsters::spell19_dancingSword() {
	if (casts()) {
		add("dancing sword");
		++g_globals->_spellsState._mmVal1;
		g_globals->_spellsState._damage = getRandomNumber(14) + 16;

		add(':');
		handlePartyDamage();
	}
}

void SpellsMonsters::spell20_magicDrain() {
	add("magic drain");

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;
		c._sp._current = 0;
	}

	dispelParty();
}

void SpellsMonsters::spell21_fingerOfDeath() {
	if (casts()) {
		add("finger of death");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._damage = BAD_CONDITION | DEAD;

		chooseCharacter();
		writeConditionEffect();
	}
}

void SpellsMonsters::spell22_sunRay() {
	if (casts()) {
		add("sun ray");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._damage = getRandomNumber(51) + 49;

		damageRandomChar();
	}
}

void SpellsMonsters::spell23_disintegration() {
	if (casts()) {
		add("disintegration");
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._resistanceIndex;
		g_globals->_spellsState._damage = ERADICATED;

		chooseCharacter();
		writeConditionEffect();
	}
}

void SpellsMonsters::spell24_commandsEnergy() {
	add("commands energy");
	g_globals->_spellsState._damage = SILENCED | PARALYZED | UNCONSCIOUS;
	damageRandomChar();
}

void SpellsMonsters::spell25_poison() {
	add("breathes poison");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_POISON;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell26_lightning() {
	add("breathes lightning");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ELECTRICITY;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell27_frost() {
	add("breathes frost");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_COLD;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell28_spikes() {
	add("breathes spikes");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ELECTRICITY;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell29_acid() {
	add("breathes acid");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_ACID;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell30_fire() {
	add("breathes fire");
	g_globals->_spellsState._resistanceTypeOrTargetCount = RESISTANCE_FIRE;
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell31_energy() {
	add("energy");
	g_globals->_spellsState._damage = g_globals->_encounters._monsterList[getMonsterIndex()]._hp;
	++g_globals->_spellsState._resistanceIndex;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell32_swarm() {
	add("swarm");
	++g_globals->_spellsState._resistanceIndex;
	g_globals->_spellsState._damage = getRandomNumber(12);

	add(':');
	handlePartyDamage();
}

bool SpellsMonsters::casts() {
	if (canMonsterCast()) {
		add("casts ");
		return true;
	} else {
		add("fails to cast a spell");
		return false;
	}
}

void SpellsMonsters::addCharName() {
	add(g_globals->_currCharacter->_name);
}

void SpellsMonsters::damageRandomChar() {
	chooseCharacter();
	handleDamage();
}

void SpellsMonsters::chooseCharacter() {
	add(':');

	// Choose a random character
	g_globals->_currCharacter = &g_globals->_party[
		getRandomNumber(g_globals->_party.size()) - 1
	];

	if (g_globals->_currCharacter->_condition & (BAD_CONDITION | UNCONSCIOUS)) {
		// Can't use character, so sequently scan party
		// to find a character that can be used
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_currCharacter = &g_globals->_party[i];
			if (g_globals->_currCharacter->_condition & (BAD_CONDITION | UNCONSCIOUS))
				break;
		}
	}
}

bool SpellsMonsters::isCharAffected() const {
	int val = g_globals->_currCharacter->_resistances._s._magic._current +
		g_globals->_activeSpells._s.magic;
	return randomThreshold(val);
}

void SpellsMonsters::handleDamage() {
	g_globals->_spellsState._mmVal5 = 1;
	_damage = g_globals->_spellsState._damage;

	if (charAffected()) {
		if (isEffective()) {
			if (testElementalResistance()) {
				if (g_globals->_activeSpells._s.power_shield)
					_damage = 1;

				writeDamage();
				Common::String str = subtractDamageFromChar();
				if (!str.empty())
					_lines.push_back(Line(0, _lines.back().y + 1, str));
			}
		}
	}
}

bool SpellsMonsters::charAffected() {
	_lines.push_back(Line(0, 2, ""));
	addCharName();
	add(' ');

	if (g_globals->_spellsState._mmVal1 && !isCharAffected()) {
		_lines.back()._text += "is not affected!";
		return false;
	}

	return true;
}

bool SpellsMonsters::isEffective() {
	if (g_globals->_spellsState._resistanceIndex) {
		proc9();

		if (g_globals->_spellsState._mmVal7) {
			if (g_globals->_spellsState._mmVal5) {
				_damage >>= 1;
			} else {
				_lines.back()._text += "is not affected!";
				return false;
			}
		}
	}

	return true;
}

bool SpellsMonsters::testElementalResistance() {
	bool result = false;

	switch (g_globals->_spellsState._resistanceTypeOrTargetCount) {
	case RESISTANCE_FIRE:
		result = damageType1(); break;
	case RESISTANCE_COLD:
		result = damageType2(); break;
	case RESISTANCE_ELECTRICITY:
		result = damageType3(); break;
	case RESISTANCE_ACID:
		result = damageType4(); break;
	case RESISTANCE_FEAR:
		result = damageType5(); break;
	case RESISTANCE_POISON:
		result = damageType6(); break;
	case RESISTANCE_PSYCHIC:
		result = damageType7(); break;
	default:
		break;
	}

	if (!result) {
		if (g_globals->_spellsState._mmVal5) {
			_damage >>= 2;
		} else {
			add("is not affected!");
			return false;
		}
	}

	return true;
}

void SpellsMonsters::writeDamage() {
	add("takes ");
	add(Common::String::format("%d ", _damage));
	add(_damage > 1 ? "points" : "point");
	add(' ');

	if (_lines.back()._text.size() >= 30)
		add('!');
	else
		add("of damage!");
}

void SpellsMonsters::proc9() {
	const Character &c = *g_globals->_currCharacter;
	int val = c._level._current * 4 + c._luck._current;
	if (c._class == PALADIN)
		val += 20;

	int randVal = getRandomNumber(100);
	g_globals->_spellsState._mmVal7 = randVal < 99 && randVal <= val ? 1 : 0;
}

bool SpellsMonsters::damageType1() {
	int threshold = g_globals->_currCharacter->_resistances._s._fire +
		g_globals->_activeSpells._s.fire;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType2() {
	int threshold = g_globals->_currCharacter->_resistances._s._cold +
		g_globals->_activeSpells._s.cold;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType3() {
	int threshold = g_globals->_currCharacter->_resistances._s._electricity +
		g_globals->_activeSpells._s.electricity;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType4() {
	int threshold = g_globals->_currCharacter->_resistances._s._acid +
		g_globals->_activeSpells._s.acid;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType5() {
	if (g_globals->_activeSpells._s.psychic_protection) {
		return false;
	} else {
		int threshold = g_globals->_currCharacter->_resistances._s._fear +
			g_globals->_activeSpells._s.fear;
		return randomThreshold(threshold);
	}
}

bool SpellsMonsters::damageType6() {
	int threshold = g_globals->_currCharacter->_resistances._s._poison +
		g_globals->_activeSpells._s.poison;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType7() {
	if (g_globals->_activeSpells._s.psychic_protection) {
		return false;
	} else {
		int threshold = g_globals->_currCharacter->_resistances._s._psychic;
		return randomThreshold(threshold);
	}
}

void SpellsMonsters::writeConditionEffect() {
	g_globals->_spellsState._mmVal5 = 0;
	int effectNum;

	if (!charAffected() || !isEffective() || !testElementalResistance())
		return;

	const byte condition = g_globals->_spellsState._damage;
	setCondition(condition);

	if (condition == 0) {
		effectNum = 10;
	} else if (condition == ERADICATED) {
		effectNum = 9;
	} else if (condition == (BAD_CONDITION | DEAD)) {
		effectNum = 7;
	} else if (condition == (BAD_CONDITION | STONE)) {
		effectNum = 8;
	} else {
		// Lowest set condition bit selects the message
		effectNum = 0;

		for (byte bitset = condition; !(bitset & 1);
				++effectNum, bitset >>= 1) {
		}
	}

	add(CHAR_EFFECTS[effectNum]);
	add('!');
}

void SpellsMonsters::setCondition(byte newCondition) {
	Character &c = *g_globals->_currCharacter;

	if (!(c._condition & BAD_CONDITION)) {
		c._condition |= newCondition;
	} else if (newCondition & BAD_CONDITION) {
		c._condition = newCondition;
	}
}

void SpellsMonsters::handlePartyEffects() {
	Character *priorCharacter = g_globals->_currCharacter;

	add(':');

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		writeConditionEffect();
	}

	// Party-wide condition effects temporarily select each target.
	// Restore the combat character that was active beforehand.
	g_globals->_currCharacter = priorCharacter;
}

void SpellsMonsters::handlePartyDamage() {
	Character *tmp = g_globals->_currCharacter;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		handleDamage();
	}

	g_globals->_currCharacter = tmp;
}

} // namespace Game
} // namespace MM1
} // namespace MM
