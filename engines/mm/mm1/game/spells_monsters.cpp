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
	Common::fill(&_arr1[0], &_arr1[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_monsterStatus[0], &_monsterStatus[MAX_COMBAT_MONSTERS], 0);
}

void SpellsMonsters::castMonsterSpell(const Common::String &monsterName, int spellNum) {
	g_globals->_spellsState._mmVal1 = 0;
	g_globals->_spellsState._mmVal2 = 0;
	g_globals->_spellsState._resistanceType = RESISTANCE_MAGIC;
	g_globals->_spellsState._newCondition = 0;

	// All spell messages start with the monster who casts it
	_lines.clear();
	_lines.push_back(Line(monsterName));

	(this->*SPELLS[spellNum - 1])();
}

void SpellsMonsters::spell01_curse() {
	if (casts()) {
		add(STRING["monster_spellsState.a_curse"]);
		g_globals->_activeSpells._s.cursed = MIN(255,
			(int)g_globals->_activeSpells._s.cursed + 1);
	}
}

void SpellsMonsters::spell02_energyBlast() {
	if (casts()) {		
		add(STRING["monster_spellsState.energy_blast"]);
		++g_globals->_spellsState._mmVal1;
		g_globals->_spellsState._newCondition = getRandomNumber(16) + 4;
		damageRandomChar();
	}
}

void SpellsMonsters::spell03_fire() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.fire"].c_str()));
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_FIRE;

	int count = g_globals->_encounters._arr1[getMonsterIndex()];
	g_globals->_spellsState._newCondition += count * 6;

	damageRandomChar();
}

void SpellsMonsters::spell04_blindness() {
	if (casts()) {
		add(STRING["monster_spellsState.blindness"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._newCondition = 2;

		handlePartyEffects();
	}
}

void SpellsMonsters::spell05_sprayPoison() {
	add(STRING["monster_spellsState.sprays_poison"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_POISON;
	g_globals->_spellsState._newCondition = POISONED;
	handlePartyEffects();
}

void SpellsMonsters::spell06_sprayAcid() {
	add(STRING["monster_spellsState.sprays_acid"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_ACID;
	g_globals->_spellsState._newCondition = getRandomNumber((int)POISONED);
	add(':');

	handlePartyDamage();
}

void SpellsMonsters::spell07_sleep() {
	if (casts()) {
		add(STRING["monster_spellsState.sleep"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._resistanceType = RESISTANCE_PSYCHIC;
		g_globals->_spellsState._newCondition = ASLEEP;
		handlePartyEffects();
	}
}

void SpellsMonsters::spell08_paralyze() {
	if (casts()) {
		add(STRING["monster_spellsState.paralyze"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._resistanceType = RESISTANCE_FEAR;
		g_globals->_spellsState._newCondition = PARALYZED;

		if (g_globals->_encounters._arr1[getMonsterIndex()] >= 5) {
			handlePartyEffects();
		} else {
			chooseCharacter();
			writeConditionEffect();
		}
	}
}

void SpellsMonsters::spell09_dispel() {
	if (casts()) {
		add(STRING["monster_spellsState.dispel"]);
		dispelParty();
	}
}

void SpellsMonsters::spell10_lightningBolt() {
	if (casts()) {
		add(STRING["monster_spellsState.lightning_bolt"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._resistanceType = RESISTANCE_ELECTRICITY;
		g_globals->_spellsState._newCondition = getRandomNumber(37) + 5;
		damageRandomChar();
	}
}

void SpellsMonsters::spell11_strangeGas() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.strange_gas"].c_str()));
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_POISON;
	g_globals->_spellsState._newCondition = BAD_CONDITION | STONE;

	handlePartyEffects();
}

void SpellsMonsters::spell12_explode() {
	add(STRING["monster_spellsState.explode"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_POISON;
	g_globals->_spellsState._newCondition = getRandomNumber(_arr1[getMonsterIndex()]);
	_arr1[getMonsterIndex()] = 0;
	_monsterStatus[getMonsterIndex()] = MONFLAG_DEAD;
	removeMonster();

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell13_fireball() {
	if (casts()) {
		add(STRING["monster_spellsState.fireball"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._resistanceType = RESISTANCE_FIRE;

		// This whole condition choice makes no sense
		g_globals->_spellsState._newCondition += 6 *
			g_globals->_encounters._arr1[getMonsterIndex()];
		g_globals->_spellsState._newCondition = getRandomNumber(g_globals->_spellsState._newCondition) + 4;

		add(':');
		handlePartyDamage();
	}
}

void SpellsMonsters::spell14_fireBreath() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.fire"].c_str()));
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_FIRE;

	// This whole condition choice makes no sense
	g_globals->_spellsState._newCondition += 8 *
		g_globals->_encounters._arr1[getMonsterIndex()];
	g_globals->_spellsState._newCondition = getRandomNumber(g_globals->_spellsState._newCondition);

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell15_gazes() {
	add(STRING["monster_spellsState.gazes"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._newCondition = BAD_CONDITION | STONE;

	chooseCharacter();
	writeConditionEffect();
}

void SpellsMonsters::spell16_acidArrow() {
	add(STRING["monster_spellsState.acid_arrow"]);
	++g_globals->_spellsState._mmVal1;
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._resistanceType = RESISTANCE_ACID;
	g_globals->_spellsState._newCondition = getRandomNumber(31) + 9;

	damageRandomChar();
}

void SpellsMonsters::spell17_elements() {
	add(STRING["monster_spellsState.call_elements"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._newCondition = getRandomNumber(21) + 39;

	damageRandomChar();
}

void SpellsMonsters::spell18_coldBeam() {
	if (casts()) {
		add(STRING["monster_spellsState.cold_beam"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._resistanceType = RESISTANCE_COLD;
		g_globals->_spellsState._newCondition = getRandomNumber(41) + 9;

		damageRandomChar();
	}
}

void SpellsMonsters::spell19_dancingSword() {
	if (casts()) {
		add(STRING["monster_spellsState.dancing_sword"]);
		++g_globals->_spellsState._mmVal1;
		g_globals->_spellsState._newCondition = getRandomNumber(14) + 16;

		add(':');
		handlePartyDamage();
	}
}

void SpellsMonsters::spell20_magicDrain() {
	add(STRING["monster_spellsState.magic_drain"]);

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;
		c._sp._current = 0;
	}

	dispelParty();
}

void SpellsMonsters::spell21_fingerOfDeath() {
	if (casts()) {
		add(STRING["monster_spellsState.finger_of_death"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._newCondition = BAD_CONDITION | DEAD;

		chooseCharacter();
		writeConditionEffect();
	}
}

void SpellsMonsters::spell22_sunRay() {
	if (casts()) {
		add(STRING["monster_spellsState.sun_ray"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._newCondition = getRandomNumber(51) + 49;

		damageRandomChar();
	}
}

void SpellsMonsters::spell23_disintegration() {
	if (casts()) {
		add(STRING["monster_spellsState.disintegration"]);
		++g_globals->_spellsState._mmVal1;
		++g_globals->_spellsState._mmVal2;
		g_globals->_spellsState._newCondition = ERADICATED;

		chooseCharacter();
		writeConditionEffect();
	}
}

void SpellsMonsters::spell24_commandsEnergy() {
	add(STRING["monster_spellsState.commands_energy"]);
	g_globals->_spellsState._newCondition = SILENCED | PARALYZED | UNCONSCIOUS;
	damageRandomChar();
}

void SpellsMonsters::spell25_poison() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.poison"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_POISON;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell26_lightning() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.lightning"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_ELECTRICITY;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell27_frost() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.frost"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_COLD;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell28_spikes() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.spikes"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_ELECTRICITY;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell29_acid() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.acid"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_ACID;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell30_fire() {
	add(Common::String::format("%s %s",
		STRING["monster_spellsState.breathes"].c_str(),
		STRING["monster_spellsState.fire"].c_str()));
	g_globals->_spellsState._resistanceType = RESISTANCE_FIRE;
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell31_energy() {
	add(STRING["monster_spellsState.energy"]);
	g_globals->_spellsState._newCondition = _arr1[getMonsterIndex()];
	++g_globals->_spellsState._mmVal2;

	add(':');
	handlePartyDamage();
}

void SpellsMonsters::spell32_swarm() {
	add(STRING["monster_spellsState.swarm"]);
	++g_globals->_spellsState._mmVal2;
	g_globals->_spellsState._newCondition = getRandomNumber(12);

	add(':');
	handlePartyDamage();
}

bool SpellsMonsters::casts() {
	if (canMonsterCast()) {
		add(STRING["monster_spellsState.casts"]);
		return true;
	} else {
		add(STRING["monster_spellsState.fails_to_cast"]);
		return false;
	}
}

void SpellsMonsters::addCharName() {
	// Wrap name in special characters so that characters
	// with multiple word names will capitalize each word
	// of the name
	add(Common::String("|%s|",
		g_globals->_currCharacter->_name));
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
	_damage = g_globals->_spellsState._newCondition;

	if (charAffected()) {
		if (isEffective()) {
			if (testElementalResistance()) {
				if (g_globals->_activeSpells._s.power_shield)
					_damage = 1;

				writeDamage();
				subtractDamage();
			}
		}
	}
}

bool SpellsMonsters::charAffected() {
	_lines.push_back(Line(0, 2, ""));
	addCharName();
	add(' ');

	if (g_globals->_spellsState._mmVal1 && !isCharAffected()) {
		_lines.back()._text += STRING["monster_spellsState.not_affected"];
		return false;
	}

	return true;
}

bool SpellsMonsters::isEffective() {
	if (g_globals->_spellsState._mmVal2) {
		proc9();

		if (g_globals->_spellsState._mmVal7) {
			if (g_globals->_spellsState._mmVal5) {
				_damage >>= 1;
			} else {
				_lines.back()._text += STRING["monster_spellsState.not_affected"];
				return false;
			}
		}
	}

	return true;
}

bool SpellsMonsters::testElementalResistance() {
	bool result = false;

	switch (g_globals->_spellsState._resistanceType) {
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
			add(STRING["monster_spellsState.not_affected"]);
			return false;
		}
	}

	return true;
}

void SpellsMonsters::writeDamage() {
	add(STRING["monster_spellsState.takes"]);
	add(Common::String::format("%d ", _damage));
	add(STRING[_damage > 1 ? "monster_spellsState.points" : "monster_spellsState.point"]);
	add(' ');

	if (_lines.back()._text.size() >= 30)
		add('!');
	else
		add(STRING["monster_spellsState.of_damage"]);
}

void SpellsMonsters::subtractDamage() {
	Character &c = *g_globals->_currCharacter;
	int newHp = c._hpBase - _damage;

	if (newHp > 0) {
		c._hpBase = newHp;

	} else {
		c._hpBase = 0;

		if (!(c._condition & (BAD_CONDITION | UNCONSCIOUS))) {
			c._condition |= UNCONSCIOUS;
			addCharName();
			add(' ');
			add(STRING["monster_spellsState.goes_down"]);
			Sound::sound2(SOUND_8);

		} else {
			if (c._condition & BAD_CONDITION)
				c._condition = BAD_CONDITION | DEAD;

			_lines.push_back(Line(0, 3, ""));
			addCharName();
			add(' ');
			add(STRING["monster_spellsState.dies"]);
			Sound::sound2(SOUND_8);
		}
	}
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

	if (g_globals->_spellsState._newCondition == 0) {
		effectNum = 10;
	} else if (g_globals->_spellsState._newCondition & BAD_CONDITION) {
		if (!(g_globals->_spellsState._newCondition & (BAD_CONDITION | DEAD)))
			effectNum = 7;
		else if (!(g_globals->_spellsState._newCondition & (BAD_CONDITION | STONE)))
			effectNum = 8;
		else if (g_globals->_spellsState._newCondition == ERADICATED)
			effectNum = 9;
		else
			effectNum = 10;
	} else {
		effectNum = 0;

		for (byte bitset = g_globals->_spellsState._newCondition; bitset & 1;
			++effectNum, bitset >>= 1) {
		}
	}

	add(STRING[Common::String::format("spells.char_effects.%d",
		effectNum)]);
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
	add(':');

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		writeConditionEffect();
	}
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
