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

#include "mm/mm1/data/spells.h"
#include "mm/mm1/data/locations.h"
#include "mm/mm1/game/rest.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/views/spells/fly.h"
#include "mm/mm1/views/spells/location.h"
#include "mm/mm1/views/spells/teleport.h"

namespace MM {
namespace MM1 {

Spells::SpellFn Spells::SPELLS[SPELLS_COUNT] = {
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

SpellResult Spells::cast(int spell, Character *chr) {
	assert(spell < SPELLS_COUNT);
	return SPELLS[spell](chr);
}

SpellResult Spells::cleric14_firstAid(Character *chr) {
	restoreHp(chr, 8);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric15_light(Character *chr) {
	addLight(1);
	return SR_SUCCESS_SILENT;
}

SpellResult Spells::cleric16_powerCure(Character *chr) {
	int totalHp = 0;
	for (uint i = 0; i < g_globals->_currCharacter->_level._current; ++i)
		totalHp += g_engine->getRandomNumber(10);

	restoreHp(chr, MIN(totalHp, 250));
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric17_protectionFromFear(Character *chr) {
	g_globals->_spells._s.fear =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric21_cureWounds(Character *chr) {
	restoreHp(chr, 16);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric24_protectionFromCold(Character *chr) {
	g_globals->_spells._s.cold =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric25_protectionFromIce(Character *chr) {
	g_globals->_spells._s.fire =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric26_protectionFromPoison(Character *chr) {
	g_globals->_spells._s.poison =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric31_createFood(Character *chr) {
	if (chr->_food == MAX_FOOD) {
		return SR_FAILED;
	} else {
		chr->_food = MIN(chr->_food + 6, MAX_FOOD);
		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric32_cureBlindness(Character *chr) {
	if (chr->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		chr->_condition &= ~BLINDED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric33_cureParalysis(Character *chr) {
	if (chr->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		chr->_condition &= ~PARALYZED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric34_lastingLight(Character *chr) {
	addLight(19);
	return SR_SUCCESS_SILENT;
}

SpellResult Spells::cleric37_removeQuest(Character *chr) {
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		g_globals->_party[i]._quest = 0;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric38_walkOnWater(Character *chr) {
	g_globals->_spells._s.walk_on_water = MIN(
		(int)g_globals->_spells._s.walk_on_water + 1, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric41_cureDisease(Character *chr) {
	if (chr->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		chr->_condition &= ~DISEASED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric42_neutralizePoison(Character *chr) {
	if (chr->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		chr->_condition &= ~POISONED;
		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric43_protectionFromAcid(Character *chr) {
	g_globals->_spells._s.acid =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric44_protectionFromElectricity(Character *chr) {
	g_globals->_spells._s.electricity =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric45_restoreAlignment(Character *chr) {
	chr->_alignment = chr->_alignmentInitial;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric48_surface(Character *chr) {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;
	if (map[46] & 4) {
		return SR_FAILED;
	} else {
		maps._mapPos = Common::Point(map[42], map[43]);
		maps.changeMap(map[11] & (map[12] << 8), map[13]);
		return SR_SUCCESS_SILENT;
	}
}

SpellResult Spells::cleric52_dispelMagic(Character *chr) {
	Maps::Map &map = *g_maps->_currentMap;
	if (g_engine->getRandomNumber(100) >= map[38]) {
		Common::fill(&g_globals->_spells._arr[0],
			&g_globals->_spells._arr[18], 0);

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

SpellResult Spells::cleric54_removeCondition(Character *chr) {
	if (chr->_condition & BAD_CONDITION) {
		return SR_FAILED;
	} else {
		chr->_condition = FINE;

		if (!chr->_hpBase)
			chr->_hpBase = 1;
		restoreHp(chr, 1);

		return SR_SUCCESS_DONE;
	}
}

SpellResult Spells::cleric55_restoreEnergy(Character *chr) {
	if (chr->_level._current < chr->_level._base) {
		chr->_level._current = MIN(
			chr->_level._current + g_engine->getRandomNumber(1, 5),
			(int)chr->_level._base
		);
		return SR_SUCCESS_DONE;
	} else {
		return SR_FAILED;
	}
}

SpellResult Spells::cleric62_raiseDead(Character *chr) {
	if ((chr->_condition == ERADICATED) ||
		(chr->_condition & (BAD_CONDITION | DEAD)) ==
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

	if (chr->_condition == (BAD_CONDITION | DEAD | STONE))
		chr->_condition = BAD_CONDITION | STONE;
	else
		chr->_condition = FINE;

	chr->_hpBase = 1;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric63_rejuvinate(Character *chr) {
	if (g_engine->getRandomNumber(100) < 75) {
		chr->_age._base = MIN(chr->_age._base - g_engine->getRandomNumber(1, 10),
			200);
		return SR_FAILED;
	} else {
		// Failed, increase the user's age
		chr->_age._base = MIN(chr->_age._base + 10, 200);
		return SR_FAILED;
	}
}

SpellResult Spells::cleric64_stoneToFlesh(Character *chr) {
	if ((chr->_condition == ERADICATED) ||
		(chr->_condition & (BAD_CONDITION | DEAD)) ==
		(BAD_CONDITION | DEAD))
		return SR_FAILED;

	if (chr->_condition == (BAD_CONDITION | DEAD | STONE))
		chr->_condition = BAD_CONDITION | DEAD;
	else
		chr->_condition = FINE;

	chr->_hpBase = 1;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric65_townPortal(Character *chr) {
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
			case Common::KEYCODE_5: {
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

SpellResult Spells::cleric73_protectionFromElements(Character *chr) {
	int amount = g_globals->_currCharacter->_level._current + 25;

	for (int i = 0; i < 6; ++i)
		g_globals->_spells._arr[i] = amount;

	return SR_SUCCESS_DONE;
}

SpellResult Spells::cleric74_resurrection(Character *chr) {
	if (chr->_condition == ERADICATED)
		return SR_FAILED;

	if (chr->_age._base < 10 || chr->_age._base > 200)
		chr->_age._base = 200;

	if (g_engine->getRandomNumber(100) > 75)
		return SR_FAILED;

	chr->_endurance._base = MAX((int)chr->_endurance._base - 1, 1);
	chr->_condition = FINE;
	restoreHp(chr, 1);

	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard12_detectMagic(Character *chr) {
	error("TODO: detectMagic");
	return SR_FAILED;
}

SpellResult Spells::wizard15_leatherSkin(Character *chr) {
	g_globals->_spells._s.leather_skin = g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard17_location(Character *chr) {
	Views::Spells::Location::show();
	return SR_SUCCESS_SILENT;
}

SpellResult Spells::wizard24_jump(Character *chr) {
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

SpellResult Spells::wizard25_levitate(Character *chr) {
	g_globals->_spells._s.levitate =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard32_fly(Character *chr) {
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

SpellResult Spells::wizard45_guardDog(Character *chr) {
	g_globals->_spells._s.guard_dog =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard46_psychicProtection(Character *chr) {
	g_globals->_spells._s.psychic_protection =
		g_globals->_currCharacter->_level._current;
	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard54_shelter(Character *chr) {
	if (g_maps->_currentState & 8)
		return SR_FAILED;

	Game::Rest::execute();
	return SR_SUCCESS_SILENT;
}

SpellResult Spells::wizard55_teleport(Character *chr) {
	Views::Spells::Teleport::show();
	return SR_SUCCESS_SILENT;
}

SpellResult Spells::wizard63_etherialize(Character *chr) {
	g_events->draw();
	g_maps->_currentState = 0;
	g_maps->step(Common::Point(1, 0));

	return SR_SUCCESS_SILENT;
}

SpellResult Spells::wizard64_protectionFromMagic(Character *chr) {
	g_globals->_spells._s.magic =
		MIN(g_globals->_currCharacter->_level._current + 20, 255);
	return SR_SUCCESS_DONE;
}

SpellResult Spells::wizard65_rechargeItem(Character *chr) {
	return SR_FAILED;
}

SpellResult Spells::wizard71_astralSpell(Character *chr) {
	Maps::Maps &maps = *g_maps;
	maps._mapPos.x = 7;
	maps._mapPos.y = 0;
	maps.changeMap(0xb1a, 3);

	return SR_SUCCESS_SILENT;
}

SpellResult Spells::wizard72_duplication(Character *chr) {
	return SR_FAILED;
}

void Spells::restoreHp(Character *chr, uint16 hp) {
	chr->_hpBase = MIN((int)(chr->_hpBase + hp), (int)chr->_hpMax);
}

void Spells::addLight(int amount) {
	g_globals->_spells._s.light = MIN((int)g_globals->_spells._s.light + amount, 255);
	g_events->send("Game", GameMessage("UPDATE"));
}

} // namespace MM1
} // namespace MM
