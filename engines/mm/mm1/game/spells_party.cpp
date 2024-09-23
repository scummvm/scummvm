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

#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/game/rest.h"
#include "mm/mm1/data/locations.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/views/spells/detect_magic.h"
#include "mm/mm1/views/spells/fly.h"
#include "mm/mm1/views/spells/location.h"
#include "mm/mm1/views/spells/teleport.h"

namespace MM {
namespace MM1 {
namespace Game {

Character *SpellsParty::_destChar;

SpellsParty::SpellFn SpellsParty::SPELLS[SPELLS_COUNT] = {
	// Cleric spells
	cleric11_awaken,
	cleric12_bless,
	cleric13_blind,
	cleric14_firstAid,
	cleric15_light,
	cleric16_powerCure,
	cleric17_protectionFromFear,
	cleric18_turnUndead,
	cleric21_cureWounds,
	cleric22_heroism,
	cleric23_pain,
	cleric24_protectionFromCold,
	cleric25_protectionFromIce,
	cleric26_protectionFromPoison,
	cleric27_silence,
	cleric28_suggestion,
	cleric31_createFood,
	cleric32_cureBlindness,
	cleric33_cureParalysis,
	cleric34_lastingLight,
	cleric35_produceFlame,
	cleric36_produceFrost,
	cleric37_removeQuest,
	cleric38_walkOnWater,
	cleric41_cureDisease,
	cleric42_neutralizePoison,
	cleric43_protectionFromAcid,
	cleric44_protectionFromElectricity,
	cleric45_restoreAlignment,
	cleric46_summonLightning,
	cleric47_superHeroism,
	cleric48_surface,
	cleric51_deadlySwarm,
	cleric52_dispelMagic,
	cleric53_paralyze,
	cleric54_removeCondition,
	cleric55_restoreEnergy,
	cleric61_moonRay,
	cleric62_raiseDead,
	cleric63_rejuvinate,
	cleric64_stoneToFlesh,
	cleric65_townPortal,
	cleric71_divineIntervention,
	cleric72_holyWord,
	cleric73_protectionFromElements,
	cleric74_resurrection,
	cleric75_sunRay,

	wizard11_awaken,
	wizard12_detectMagic,
	wizard13_energyBlast,
	wizard14_flameArrow,
	wizard15_leatherSkin,
	wizard16_light,
	wizard17_location,
	wizard18_sleep,
	wizard21_electricArrow,
	wizard22_hypnotize,
	wizard23_identifyMonster,
	wizard24_jump,
	wizard25_levitate,
	wizard26_power,
	wizard27_quickness,
	wizard28_scare,
	wizard31_fireball,
	wizard32_fly,
	wizard33_invisibility,
	wizard34_lightningBolt,
	wizard35_makeRoom,
	wizard36_slow,
	wizard37_weaken,
	wizard38_web,
	wizard41_acidArrow,
	wizard42_coldBeam,
	wizard43_feebleMind,
	wizard44_freeze,
	wizard45_guardDog,
	wizard46_psychicProtection,
	wizard47_shield,
	wizard48_timeDistortion,
	wizard51_acidRain,
	wizard52_dispelMagic,
	wizard53_fingerOfDeath,
	wizard54_shelter,
	wizard55_teleport,
	wizard61_dancingSword,
	wizard62_disintegration,
	wizard63_etherialize,
	wizard64_protectionFromMagic,
	wizard65_rechargeItem,
	wizard71_astralSpell,
	wizard72_duplication,
	wizard73_meteorShower,
	wizard74_powerShield,
	wizard75_prismaticLight
};

SpellResult SpellsParty::cast(uint spell, Character *destChar) {
	assert(spell < SPELLS_COUNT);
	_destChar = destChar;

	return SPELLS[spell]();
}

bool SpellsParty::isInCombat() {
	return g_events->isPresent("Combat");
}

SpellResult SpellsParty::cleric11_awaken() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~ASLEEP;
	}

	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric12_bless() {
	g_globals->_activeSpells._s.bless++;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric13_blind() {
	SpellsState &s = g_globals->_spellsState;
	s._mmVal1++;
	s._resistanceIndex = 7;
	s._damage = BLINDED;
	s._resistanceTypeOrTargetCount = static_cast<Resistance>((int)s._resistanceTypeOrTargetCount + 1);

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric14_firstAid() {
	restoreHp(8);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric15_light() {
	addLight(1);
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric16_powerCure() {
	int totalHp = 0;
	for (uint i = 0; i < g_globals->_currCharacter->_level._current; ++i)
		totalHp += g_engine->getRandomNumber(10);

	restoreHp(MIN(totalHp, 250));
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric17_protectionFromFear() {
	g_globals->_activeSpells._s.fear =
		MIN((int)g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric18_turnUndead() {
	if (g_globals->_currCharacter->_alignment !=
		g_globals->_currCharacter->_alignmentInitial)
		// Cleric's current alignment differs from initial, so spell fails
		return SR_FAILED;

	g_globals->_combat->turnUndead();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric21_cureWounds() {
	restoreHp(16);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric22_heroism() {
	if (g_globals->_currCharacter->_alignment != _destChar->_alignment ||
		_destChar->_level._current != _destChar->_level._base)
		return SR_FAILED;

	_destChar->_level._current = MIN(
		(int)_destChar->_level._current + 2, 255);
	restoreHp(6);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric23_pain() {
	SpellsState &ss = g_globals->_spellsState;
	ss._damage = getRandomNumber(6) + getRandomNumber(6);
	ss._mmVal1++;
	ss._resistanceTypeOrTargetCount++;
	ss._resistanceIndex = 6;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric24_protectionFromCold() {
	g_globals->_activeSpells._s.cold =
		MIN((int)g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric25_protectionFromIce() {
	g_globals->_activeSpells._s.fire =
		MIN((int)g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric26_protectionFromPoison() {
	g_globals->_activeSpells._s.poison =
		MIN((int)g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric27_silence() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex = 7;
	ss._damage = SILENCED;
	ss._resistanceTypeOrTargetCount++;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric28_suggestion() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex = 6;
	ss._damage = PARALYZED;
	ss._resistanceTypeOrTargetCount++;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric31_createFood() {
	if (_destChar->_food == MAX_FOOD) {
		return SR_FAILED;
	} else {
		_destChar->_food = MIN(_destChar->_food + 6, MAX_FOOD);
		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric32_cureBlindness() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition &= ~BLINDED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric33_cureParalysis() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition &= ~PARALYZED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric34_lastingLight() {
	addLight(19);
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric35_produceFlame() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceTypeOrTargetCount++;
	ss._resistanceIndex = 4;
	ss._damage = getRandomNumber(6) +
		getRandomNumber(6) + getRandomNumber(6);
	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric36_produceFrost() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex++;
	ss._damage = getRandomNumber(6) +
		getRandomNumber(6) + getRandomNumber(6);
	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric37_removeQuest() {
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		g_globals->_party[i]._quest = 0;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric38_walkOnWater() {
	g_globals->_activeSpells._s.walk_on_water = MIN(
		(int)g_globals->_activeSpells._s.walk_on_water + 1, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric41_cureDisease() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition &= ~DISEASED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric42_neutralizePoison() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition &= ~POISONED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric43_protectionFromAcid() {
	g_globals->_activeSpells._s.acid =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric44_protectionFromElectricity() {
	g_globals->_activeSpells._s.electricity =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric45_restoreAlignment() {
	_destChar->_alignment = _destChar->_alignmentInitial;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric46_summonLightning() {
	g_globals->_combat->summonLightning();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric47_superHeroism() {
	if (_destChar->_level._current != _destChar->_level._base)
		return SR_FAILED;

	_destChar->_level._current = MIN(
		(int)_destChar->_level._current + 3, 255);
	restoreHp(10);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric48_surface() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;
	if (map[Maps::MAP_FLAGS] & 4) {
		return SR_FAILED;
	} else {
		maps._mapPos = Common::Point(map[Maps::MAP_SURFACE_X],
			map[Maps::MAP_SURFACE_Y]);
		maps.changeMap(map.dataWord(Maps::MAP_SURFACE_ID),
			map[Maps::MAP_SURFACE_SECTION]);
		return SR_SUCCESS_SILENT;
	}
}

SpellResult SpellsParty::cleric51_deadlySwarm() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();
	ss._mmVal1++;
	ss._resistanceIndex = 0;
	ss._damage = getRandomNumber(10) + getRandomNumber(10);

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric52_dispelMagic() {
	Maps::Map &map = *g_maps->_currentMap;
	if (g_engine->getRandomNumber(100) >= map[Maps::MAP_DISPEL_THRESHOLD]) {
		Common::fill(&g_globals->_activeSpells._arr[0],
			&g_globals->_activeSpells._arr[18], 0);

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c.updateAttributes();
			c.updateAC();
			if (!(c._condition & BAD_CONDITION))
				c._condition &= ~(BLINDED | SILENCED);
		}

		return SR_SUCCESS_DONE;
	} else {
		return SR_FAILED;
	}
}

SpellResult SpellsParty::cleric53_paralyze() {
	g_globals->_combat->paralyze();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric54_removeCondition() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition = FINE;

		if (!_destChar->_hpCurrent)
			_destChar->_hpCurrent = 1;
		restoreHp(1);

		return SR_SUCCESS_DONE;
	}
}

SpellResult SpellsParty::cleric55_restoreEnergy() {
	if (_destChar->_level._current < _destChar->_level._base) {
		_destChar->_level._current = MIN(
			_destChar->_level._current + g_engine->getRandomNumber(5),
			(int)_destChar->_level._base
		);
		return SR_SUCCESS_DONE;
	} else {
		return SR_FAILED;
	}
}

SpellResult SpellsParty::cleric61_moonRay() {
	SpellsState &ss = g_globals->_spellsState;

	// Heal the party
	int hp = getRandomNumber(10) + getRandomNumber(10) +
		getRandomNumber(10);

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		restoreHp(c, hp);
	}

	// Damage the monsters
	g_globals->_combat->resetDestMonster();
	ss._damage = hp;
	ss._mmVal1++;
	ss._resistanceIndex = 5;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric62_raiseDead() {
	if ((_destChar->_condition == ERADICATED) ||
		(_destChar->_condition & (BAD_CONDITION | DEAD)) ==
		(BAD_CONDITION | DEAD))
		return SR_FAILED;
	int rnd = g_engine->getRandomNumber(100);
	if (rnd == 100) {
		// This is such a minute chance, why even have it?
		Sound::sound(SOUND_3);
		return SR_FAILED;
	} else if (rnd > 90) {
		return SR_FAILED;
	}

	if (_destChar->_condition == (BAD_CONDITION | DEAD | STONE))
		_destChar->_condition = BAD_CONDITION | STONE;
	else
		_destChar->_condition = FINE;

	_destChar->_hpCurrent = 1;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric63_rejuvinate() {
	if (g_engine->getRandomNumber(100) < 75) {
		_destChar->_age = MIN(_destChar->_age - g_engine->getRandomNumber(10),
			200);
		return SR_FAILED;
	} else {
		// Failed, increase the user's age
		_destChar->_age = MIN(_destChar->_age + 10, 200);
		return SR_FAILED;
	}
}

SpellResult SpellsParty::cleric64_stoneToFlesh() {
	if ((_destChar->_condition == ERADICATED) ||
		(_destChar->_condition & (BAD_CONDITION | DEAD)) ==
		(BAD_CONDITION | DEAD))
		return SR_FAILED;

	if (_destChar->_condition == (BAD_CONDITION | DEAD | STONE))
		_destChar->_condition = BAD_CONDITION | DEAD;
	else
		_destChar->_condition = FINE;

	_destChar->_hpCurrent = 1;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric65_townPortal() {
	Sound::sound(SOUND_2);
	InfoMessage msg(
		STRING["spells.which_town"],
		[](const Common::KeyState &keyState) {
			switch (keyState.keycode) {
			case Common::KEYCODE_ESCAPE:
				g_events->focusedView()->close();
				break;
			case Common::KEYCODE_1:
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
			case Common::KEYCODE_4:
			case Common::KEYCODE_5:
			{
				Maps::Maps &maps = *g_maps;
				int townIndex = keyState.keycode - Common::KEYCODE_1;

				maps._mapPos.x = TownData::TOWN_MAP_X[townIndex];
				maps._mapPos.y = TownData::TOWN_MAP_Y[townIndex];
				maps.changeMap(
					TownData::TOWN_MAP_ID1[townIndex] |
					(TownData::TOWN_MAP_ID1[townIndex] << 8),
					1);

				g_events->close();
				return;
			}

			default:
				break;
			}
		}
	);

	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric71_divineIntervention() {
	return g_globals->_combat->divineIntervention() ?
		SR_SUCCESS_DONE : SR_FAILED;
}

SpellResult SpellsParty::cleric72_holyWord() {
	Character &c = *g_globals->_currCharacter;
	if (c._alignment != c._alignmentInitial)
		return SR_FAILED;

	g_globals->_combat->holyWord();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::cleric73_protectionFromElements() {
	int amount = g_globals->_currCharacter->_level._current + 25;

	for (int i = 0; i < 6; ++i)
		g_globals->_activeSpells._arr[i] = amount;

	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric74_resurrection() {
	if (_destChar->_condition == ERADICATED)
		return SR_FAILED;

	if (_destChar->_age < 10 || _destChar->_age > 200)
		_destChar->_age = 200;

	if (g_engine->getRandomNumber(100) > 75)
		return SR_FAILED;

	_destChar->_endurance._base = MAX((int)_destChar->_endurance._base - 1, 1);
	_destChar->_condition = FINE;
	restoreHp(1);

	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric75_sunRay() {
	SpellsState &ss = g_globals->_spellsState;
	ss._damage = getRandomNumber(51) + 49;
	ss._mmVal1++;
	ss._resistanceIndex++;
	ss._resistanceTypeOrTargetCount = RESISTANCE_FIRE;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard12_detectMagic() {
	g_events->replaceView("DetectMagic");
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard13_energyBlast() {
	SpellsState &ss = g_globals->_spellsState;
	int damage = 0;
	for (int i = 0; i < g_globals->_currCharacter->_level._current; ++i)
		damage += getRandomNumber(4);

	ss._damage = MIN(damage, 255);
	ss._resistanceIndex = 5;
	ss._resistanceTypeOrTargetCount++;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard14_flameArrow() {
	SpellsState &ss = g_globals->_spellsState;
	ss._damage = getRandomNumber(6);
	ss._mmVal1++;
	ss._resistanceIndex = 1;
	ss._resistanceTypeOrTargetCount++;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard15_leatherSkin() {
	g_globals->_activeSpells._s.leather_skin = g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard17_location() {
	g_events->replaceView("Location");
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard18_sleep() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex = 8;
	ss._resistanceTypeOrTargetCount = RESISTANCE_FEAR;
	ss._damage = 16;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard21_electricArrow() {
	SpellsState &ss = g_globals->_spellsState;
	ss._damage = getRandomNumber(6) + getRandomNumber(6);
	ss._mmVal1++;
	ss._resistanceTypeOrTargetCount++;
	ss._resistanceIndex = 2;

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard23_identifyMonster() {
	g_globals->_combat->identifyMonster();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard24_jump() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;

	if ((maps._currentWalls & maps._forwardMask) != 0)
		return SR_FAILED;
	if ((map._states[maps._mapOffset] & maps._forwardMask & 0x55) != 0)
		return SR_FAILED;
	if ((map._walls[maps._mapOffset + maps._forwardOffset] &
		maps._forwardMask) != 0)
		return SR_FAILED;
	if ((map._states[maps._mapOffset + maps._forwardOffset]
		& maps._forwardMask & 0x55) != 0)
		return SR_FAILED;

	// The delta will be two steps in the facing direction.
	// Ensure that that doesn't end up outside the map bounds
	Common::Point delta = maps.getMoveDelta(maps._forwardMask);
	delta.x *= 2;
	delta.y *= 2;

	Common::Point newPos = maps._mapPos + delta;
	if (newPos.x < 0 || newPos.y < 0 ||
		newPos.x >= MAP_W || newPos.y >= MAP_H)
		return SR_FAILED;

	// Move the two steps
	g_maps->step(delta);
	g_events->send("Game", GameMessage("UPDATE"));

	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard25_levitate() {
	g_globals->_activeSpells._s.levitate =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard26_power() {
	g_globals->_currCharacter->_might._current += getRandomNumber(4);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard27_quickness() {
	g_globals->_currCharacter->_speed._current += getRandomNumber(4);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard28_scare() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex = 7;
	ss._resistanceTypeOrTargetCount++;
	ss._damage = 1;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard31_fireball() {
	g_globals->_combat->fireball();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard32_fly() {
	g_events->addView("Fly");
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard33_invisibility() {
	g_globals->_activeSpells._s.invisbility++;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard34_lightningBolt() {
	g_globals->_combat->lightningBolt();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard35_makeRoom() {
	g_globals->_combat->makeRoom();
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard36_slow() {
	g_globals->_combat->slow();
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard37_weaken() {
	g_globals->_combat->weaken();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard38_web() {
	return g_globals->_combat->web() ? SR_SUCCESS_SILENT : SR_FAILED;
}

SpellResult SpellsParty::wizard41_acidArrow() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceTypeOrTargetCount++;
	ss._resistanceIndex = 3;
	ss._damage += getRandomNumber(10) +
		getRandomNumber(10) + getRandomNumber(10);

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard42_coldBeam() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceTypeOrTargetCount++;
	ss._resistanceIndex = 4;

	ss._damage = getRandomNumber(10) + getRandomNumber(10) +
		getRandomNumber(10) + getRandomNumber(10);

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard43_feebleMind() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1++;
	ss._resistanceIndex = 0;
	ss._resistanceTypeOrTargetCount++;
	ss._damage = 8;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard44_freeze() {
	SpellsState &ss = g_globals->_spellsState;
	ss._mmVal1 = 0;
	ss._resistanceIndex = 6;
	ss._resistanceTypeOrTargetCount++;
	ss._damage = 128;

	g_globals->_combat->iterateMonsters1();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard45_guardDog() {
	g_globals->_activeSpells._s.guard_dog =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard46_psychicProtection() {
	g_globals->_activeSpells._s.psychic_protection =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard47_shield() {
	g_globals->_activeSpells._s.shield++;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard48_timeDistortion() {
	Maps::Map &map = *g_maps->_currentMap;

	if (getRandomNumber(100) < map[Maps::MAP_DISPEL_THRESHOLD])
		return SR_FAILED;

	// End combat by closing the combat view
	g_events->focusedView()->close();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard51_acidRain() {
	return g_globals->_combat->acidRain() ? SR_SUCCESS_SILENT : SR_FAILED;
}

SpellResult SpellsParty::wizard53_fingerOfDeath() {
	g_globals->_combat->fingerOfDeath();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard54_shelter() {
	if (g_maps->_currentState & 8)
		return SR_FAILED;

	Game::Rest::execute();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard55_teleport() {
	g_events->replaceView("Teleport");
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard61_dancingSword() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();

	ss._mmVal1 = 0;
	ss._resistanceIndex = 0;
	ss._damage = getRandomNumber(30);

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard62_disintegration() {
	g_globals->_combat->disintegration();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard63_etherialize() {
	g_events->draw();
	g_maps->_currentState = 0;
	g_maps->step(Common::Point(1, 0));

	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard64_protectionFromMagic() {
	g_globals->_activeSpells._s.magic =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard65_rechargeItem() {
	g_events->replaceView("RechargeItem");
	return SR_FAILED;
}

SpellResult SpellsParty::wizard71_astralSpell() {
	Maps::Maps &maps = *g_maps;
	maps._mapPos.x = 7;
	maps._mapPos.y = 0;
	maps.changeMap(0xb1a, 3);

	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard72_duplication() {
	g_events->replaceView("Duplication");
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard73_meteorShower() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();

	ss._mmVal1++;
	ss._resistanceIndex = 5;
	ss._damage = MIN((int)getRandomNumber(120) +
		(int)g_globals->_currCharacter->_level._current, 120);

	g_globals->_combat->iterateMonsters2();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard74_powerShield() {
	g_globals->_activeSpells._s.power_shield++;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard75_prismaticLight() {
	SpellsState &ss = g_globals->_spellsState;
	g_globals->_combat->resetDestMonster();

	if (getRandomNumber(100) < 20)
		return SR_FAILED;

	if (ss._mmVal1 < 50) {
		uint count = getRandomNumber(4);
		ss._damage <<= count;
		g_globals->_combat->iterateMonsters2();

	} else {
		uint count = getRandomNumber(8);
		ss._damage <<= count;
		g_globals->_combat->iterateMonsters1();
	}

	return SR_SUCCESS_SILENT;
}

void SpellsParty::restoreHp(uint16 hp) {
	restoreHp(*_destChar, hp);
}

void SpellsParty::restoreHp(Character &c, uint16 hp) {
	c._hpCurrent = MIN((int)(c._hpCurrent + hp), (int)c._hpMax);
	if (!(c._condition & BAD_CONDITION))
		c._condition &= ~UNCONSCIOUS;
}

void SpellsParty::addLight(int amount) {
	g_globals->_activeSpells._s.light = MIN((int)g_globals->_activeSpells._s.light + amount, 255);
	g_events->send("Game", GameMessage("UPDATE"));
}

void SpellsParty::display(const InfoMessage &msg) {
	g_globals->_combat->displaySpellResult(msg);
}

} // namespace Game
} // namespace MM1
} // namespace MM
