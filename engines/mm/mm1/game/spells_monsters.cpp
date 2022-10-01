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
	&SpellsMonsters::spell09_dispell,
	&SpellsMonsters::spell10_lightningBolt,
	&SpellsMonsters::spell11_strangeGas,
	&SpellsMonsters::spell12_explode,
	&SpellsMonsters::spell13_fireball,
	&SpellsMonsters::spell14_fire2,
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

void SpellsMonsters::castMonsterSpell(const Common::String &monsterName, int spellNum) {
	_mmVal1 = _mmVal2 = _newCondition = 0;
	_resistanceType = RESISTANCE_MAGIC;

	// All spell messages start with the monster who casts it
	_lines.clear();
	_lines.push_back(Line(monsterName));

	(this->*SPELLS[spellNum - 1])();
}

void SpellsMonsters::spell01_curse() {
	if (casts()) {
		add(STRING["monster_spells.a_curse"]);
		g_globals->_spells._s.cursed = MIN(255,
			(int)g_globals->_spells._s.cursed + 1);
	}
}

void SpellsMonsters::spell02_energyBlast() {
	if (casts()) {		
		add(STRING["monster_spells.energy_blast"]);
		++_mmVal1;
		_newCondition = getRandomNumber(16) + 4;
		damageRandomChar();
	}
}

void SpellsMonsters::spell03_fire() {
	add(Common::String::format("%s %s",
		STRING["monster_spells.breathes"].c_str(),
		STRING["monster_spells.fire"].c_str()));
	++_mmVal2;
	_resistanceType = RESISTANCE_FIRE;

	int count = g_globals->_encounters._arr1[getMonsterIndex()];
	_newCondition += count * 6;

	damageRandomChar();
}

void SpellsMonsters::spell04_blindness() {
	if (casts()) {
		add(STRING["monster_spells.blindness"]);
		++_mmVal1;
		++_mmVal2;
		_newCondition = 2;

		add(':');

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_currCharacter = &g_globals->_party[i];
			writeConditionEffect();
		}
	}
}

void SpellsMonsters::spell05_sprayPoison() {}

void SpellsMonsters::spell06_sprayAcid() {}

void SpellsMonsters::spell07_sleep() {}

void SpellsMonsters::spell08_paralyze() {}

void SpellsMonsters::spell09_dispell() {}

void SpellsMonsters::spell10_lightningBolt() {}

void SpellsMonsters::spell11_strangeGas() {}

void SpellsMonsters::spell12_explode() {}

void SpellsMonsters::spell13_fireball() {}

void SpellsMonsters::spell14_fire2() {}

void SpellsMonsters::spell15_gazes() {}

void SpellsMonsters::spell16_acidArrow() {}

void SpellsMonsters::spell17_elements() {}

void SpellsMonsters::spell18_coldBeam() {}

void SpellsMonsters::spell19_dancingSword() {}

void SpellsMonsters::spell20_magicDrain() {}

void SpellsMonsters::spell21_fingerOfDeath() {}

void SpellsMonsters::spell22_sunRay() {}

void SpellsMonsters::spell23_disintegration() {}

void SpellsMonsters::spell24_commandsEnergy() {}

void SpellsMonsters::spell25_poison() {}

void SpellsMonsters::spell26_lightning() {}

void SpellsMonsters::spell27_frost() {}

void SpellsMonsters::spell28_spikes() {}

void SpellsMonsters::spell29_acid() {}

void SpellsMonsters::spell30_fire() {}

void SpellsMonsters::spell31_energy() {}

void SpellsMonsters::spell32_swarm() {}

bool SpellsMonsters::casts() {
	if (canMonsterCast()) {
		add(STRING["monster_spells.casts"]);
		return true;
	} else {
		add(STRING["monster_spells.fails_to_cast"]);
		return false;
	}
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
		g_globals->_spells._s.magic;
	return randomThreshold(val);
}

void SpellsMonsters::handleDamage() {
	_mmVal5 = 1;
	_damage = _newCondition;

	if (charAffected()) {
		if (isEffective()) {
			if (testElementalResistance()) {
				if (g_globals->_spells._s.power_shield)
					_damage = 1;

				writeDamage();
				subtractDamage();
			}
		}
	}
}

bool SpellsMonsters::charAffected() {
	_lines.push_back(Line(0, 2, Common::String::format("%s ",
		g_globals->_currCharacter->_name)));

	if (_mmVal1 && !isCharAffected()) {
		_lines.back()._text += STRING["monster_spells.not_affected"];
		return false;
	}

	return true;
}

bool SpellsMonsters::isEffective() {
	if (_mmVal2) {
		proc9();

		if (_mmVal7) {
			if (_mmVal5) {
				_damage >>= 1;
			} else {
				_lines.back()._text += STRING["monster_spells.not_affected"];
				return false;
			}
		}
	}

	return true;
}

bool SpellsMonsters::testElementalResistance() {
	bool result = false;

	switch (_resistanceType) {
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
		if (_mmVal5) {
			_damage >>= 2;
		} else {
			add(STRING["monster_spells.not_affected"]);
			return false;
		}
	}

	return true;
}

void SpellsMonsters::writeDamage() {
	add(STRING["monster_spells.takes"]);
	add(Common::String::format("%d ", _damage));
	add(STRING[_damage > 1 ? "monster_spells.points" : "monster_spells.point"]);
	add(' ');

	if (_lines.back()._text.size() >= 30)
		add('!');
	else
		add(STRING["monster_spells.of_damage"]);
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
			add(c._name);
			add(' ');
			add(STRING["monster_spells.goes_down"]);
			Sound::sound2(SOUND_8);

		} else {
			if (c._condition & BAD_CONDITION)
				c._condition = BAD_CONDITION | DEAD;

			_lines.push_back(Line(0, _lines.back().y + 1,
				Common::String::format("%s %s",
					c._name, STRING["monster_spells.dies"].c_str())));
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
	_mmVal7 = randVal < 99 && randVal <= val ? 1 : 0;
}

bool SpellsMonsters::damageType1() {
	int threshold = g_globals->_currCharacter->_resistances._s._fire +
		g_globals->_spells._s.fire;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType2() {
	int threshold = g_globals->_currCharacter->_resistances._s._cold +
		g_globals->_spells._s.cold;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType3() {
	int threshold = g_globals->_currCharacter->_resistances._s._electricity +
		g_globals->_spells._s.electricity;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType4() {
	int threshold = g_globals->_currCharacter->_resistances._s._acid +
		g_globals->_spells._s.acid;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType5() {
	if (g_globals->_spells._s.psychic_protection) {
		return false;
	} else {
		int threshold = g_globals->_currCharacter->_resistances._s._fear +
			g_globals->_spells._s.fear;
		return randomThreshold(threshold);
	}
}

bool SpellsMonsters::damageType6() {
	int threshold = g_globals->_currCharacter->_resistances._s._poison +
		g_globals->_spells._s.poison;
	return randomThreshold(threshold);
}

bool SpellsMonsters::damageType7() {
	if (g_globals->_spells._s.psychic_protection) {
		return false;
	} else {
		int threshold = g_globals->_currCharacter->_resistances._s._psychic;
		return randomThreshold(threshold);
	}
}

void SpellsMonsters::writeConditionEffect() {
	_mmVal5 = 0;
	int effectNum;

	if (!charAffected() || !isEffective() || !testElementalResistance())
		return;

	if (_newCondition == 0) {
		effectNum = 10;
	} else if (_newCondition & BAD_CONDITION) {
		if (!(_newCondition & (BAD_CONDITION | DEAD)))
			effectNum = 7;
		else if (!(_newCondition & (BAD_CONDITION | STONE)))
			effectNum = 8;
		else if (_newCondition == ERADICATED)
			effectNum = 9;
		else
			effectNum = 10;
	} else {
		effectNum = 0;

		for (byte bitset = _newCondition; bitset & 1;
			++effectNum, bitset >>= 1) {
		}
	}

	add(STRING[Common::String::format("monster_spells.effects.%d",
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

} // namespace Game
} // namespace MM1
} // namespace MM
