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
int SpellsParty::_destMonsterNum;

SpellsParty::SpellFn SpellsParty::SPELLS[SPELLS_COUNT] = {
	// Cleric spells
	placeholder,
	placeholder,
	placeholder,
	cleric14_firstAid,
	cleric15_light,
	cleric16_powerCure,
	cleric17_protectionFromFear,
	placeholder,
	cleric21_cureWounds,
	placeholder,
	placeholder,
	cleric24_protectionFromCold,
	cleric25_protectionFromIce,
	cleric26_protectionFromPoison,
	placeholder,
	placeholder,
	cleric31_createFood,
	cleric32_cureBlindness,
	cleric33_cureParalysis,
	cleric34_lastingLight,
	placeholder,
	placeholder,
	cleric37_removeQuest,
	cleric38_walkOnWater,
	cleric41_cureDisease,
	cleric42_neutralizePoison,
	cleric43_protectionFromAcid,
	cleric44_protectionFromElectricity,
	cleric45_restoreAlignment,
	placeholder,
	placeholder,
	cleric48_surface,
	placeholder,
	cleric52_dispelMagic,
	placeholder,
	cleric54_removeCondition,
	cleric55_restoreEnergy,
	placeholder,
	cleric62_raiseDead,
	cleric63_rejuvinate,
	cleric64_stoneToFlesh,
	cleric65_townPortal,
	placeholder,
	placeholder,
	cleric73_protectionFromElements,
	cleric74_resurrection,
	placeholder,

	placeholder,
	wizard12_detectMagic,
	placeholder,
	placeholder,
	wizard15_leatherSkin,
	wizard16_light,
	wizard17_location,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard24_jump,
	wizard25_levitate,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard32_fly,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard45_guardDog,
	wizard46_psychicProtection,
	placeholder,
	placeholder,
	placeholder,
	wizard52_dispelMagic,
	placeholder,
	wizard54_shelter,
	wizard55_teleport,
	placeholder,
	placeholder,
	wizard63_etherialize,
	wizard64_protectionFromMagic,
	wizard65_rechargeItem,
	wizard71_astralSpell,
	wizard72_duplication,
	placeholder,
	placeholder,
	placeholder,
};

byte FLY_MAP_ID1[20] = {
	1, 0, 4, 5, 0x12,
	2, 3, 0x11, 5, 6,
	2, 1, 4, 6, 0x1A,
	3, 3, 4, 1, 0x1B
};

byte FLY_MAP_ID2[20] = {
	0xF, 0xA, 3, 5, 1,
	5, 7, 0xA, 0xB, 7,
	0xB, 1, 9, 1, 0xB,
	1, 0xD, 0xF, 8, 1
};

byte FLY_MAP_X[20] = {
	15, 8, 11, 0, 9,
	15, 3, 10, 4, 11,
	15, 3, 3, 7, 12,
	14, 11, 5, 7, 15
};

byte FLY_MAP_Y[20] = {
	7, 10, 0, 8, 11,
	7, 2, 10, 0, 0,
	15, 3, 9, 0, 6,
	14, 15, 15, 7, 15
};

SpellResult SpellsParty::cast(uint spell, Character *destChar) {
	assert(spell < SPELLS_COUNT);
	_destChar = destChar;
	_destMonsterNum = -1;

	return SPELLS[spell]();
}

SpellResult SpellsParty::cast(uint spell, int destMonsterIdx) {
	assert(spell < SPELLS_COUNT);
	_destChar = nullptr;
	_destMonsterNum = destMonsterIdx;

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
	s._mmVal2 = 7;
	s._newCondition = BLINDED;
	s._resistanceType = static_cast<Resistance>((int)s._resistanceType + 1);

	iterateMonsters1();
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric14_firstAid() {
	restoreHp(_destChar, 8);
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

	restoreHp(_destChar, MIN(totalHp, 250));
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric17_protectionFromFear() {
	g_globals->_activeSpells._s.fear =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric21_cureWounds() {
	restoreHp(_destChar, 16);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric24_protectionFromCold() {
	g_globals->_activeSpells._s.cold =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric25_protectionFromIce() {
	g_globals->_activeSpells._s.fire =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric26_protectionFromPoison() {
	g_globals->_activeSpells._s.poison =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
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

SpellResult SpellsParty::cleric48_surface() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;
	if (map[Maps::MAP_FLAGS] & 4) {
		return SR_FAILED;
	} else {
		maps._mapPos = Common::Point(map[Maps::MAP_SURFACE_X],
			map[Maps::MAP_SURFACE_Y]);
		maps.changeMap(map[Maps::MAP_SURFACE_DEST_ID1] &
			(map[Maps::MAP_SURFACE_DEST_ID2] << 8),
			map[Maps::MAP_SURFACE_DEST_SECTION]);
		return SR_SUCCESS_SILENT;
	}
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

SpellResult SpellsParty::cleric54_removeCondition() {
	if (_destChar->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		_destChar->_condition = FINE;

		if (!_destChar->_hpBase)
			_destChar->_hpBase = 1;
		restoreHp(_destChar, 1);

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

	_destChar->_hpBase = 1;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric63_rejuvinate() {
	if (g_engine->getRandomNumber(100) < 75) {
		_destChar->_age._base = MIN(_destChar->_age._base - g_engine->getRandomNumber(10),
			200);
		return SR_FAILED;
	} else {
		// Failed, increase the user's age
		_destChar->_age._base = MIN(_destChar->_age._base + 10, 200);
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

	_destChar->_hpBase = 1;
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

SpellResult SpellsParty::cleric73_protectionFromElements() {
	int amount = g_globals->_currCharacter->_level._current + 25;

	for (int i = 0; i < 6; ++i)
		g_globals->_activeSpells._arr[i] = amount;

	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::cleric74_resurrection() {
	if (_destChar->_condition == ERADICATED)
		return SR_FAILED;

	if (_destChar->_age._base < 10 || _destChar->_age._base > 200)
		_destChar->_age._base = 200;

	if (g_engine->getRandomNumber(100) > 75)
		return SR_FAILED;

	_destChar->_endurance._base = MAX((int)_destChar->_endurance._base - 1, 1);
	_destChar->_condition = FINE;
	restoreHp(_destChar, 1);

	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard12_detectMagic() {
	Views::Spells::DetectMagic::show();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard15_leatherSkin() {
	g_globals->_activeSpells._s.leather_skin = g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult SpellsParty::wizard17_location() {
	Views::Spells::Location::show();
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

SpellResult SpellsParty::wizard32_fly() {
	Views::Spells::Fly::show(
		[](int mapIndex) {
			if (mapIndex != -1) {
				Maps::Maps &maps = *g_maps;
				int id = FLY_MAP_ID1[mapIndex] | ((int)FLY_MAP_ID2[mapIndex] << 8);

				maps._mapPos.x = FLY_MAP_X[mapIndex];
				maps._mapPos.y = FLY_MAP_Y[mapIndex];
				maps.changeMap(id, 2);
			}
		}
	);

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

SpellResult SpellsParty::wizard54_shelter() {
	if (g_maps->_currentState & 8)
		return SR_FAILED;

	Game::Rest::execute();
	return SR_SUCCESS_SILENT;
}

SpellResult SpellsParty::wizard55_teleport() {
	Views::Spells::Teleport::show();
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
	return SR_FAILED;
}

void SpellsParty::restoreHp(Character *destChar, uint16 hp) {
	destChar->_hpBase = MIN((int)(_destChar->_hpBase + hp), (int)_destChar->_hpMax);
}

void SpellsParty::addLight(int amount) {
	g_globals->_activeSpells._s.light = MIN((int)g_globals->_activeSpells._s.light + amount, 255);
	g_events->send("Game", GameMessage("UPDATE"));
}

void SpellsParty::iterateMonsters1() {
	
}

void SpellsParty::iterateMonsters2() {

}

} // namespace Game
} // namespace MM1
} // namespace MM
