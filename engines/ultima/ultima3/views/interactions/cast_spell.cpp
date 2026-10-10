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

#include "ultima/ultima3/views/interactions/cast_spell.h"
#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/logic/dungeon_logic.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int SPELLS_PER_BOOK = 16;
constexpr int SPELL_NAMES_START = 88;
constexpr int MP_PER_LEVEL = 5;
constexpr int CURE_REFUND = 35;
constexpr byte MONSTER_ORC = 0x18;
constexpr byte MONSTER_SKELETON = 0x19;
constexpr byte FIRST_CHEST_CELL = 0x24;
constexpr byte LAST_CHEST_CELL = 0x27;
constexpr byte CELL_FLOOR = 0x20;

// What each spell does, in the order they are lettered in each book
static const byte WIZARD_EFFECTS[SPELLS_PER_BOOK] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
static const byte CLERIC_EFFECTS[SPELLS_PER_BOOK] = {
	16, 17, 18, 2, 4, 3, 19, 20, 21, 8, 22, 23, 11, 24, 15, 25
};

static int rollBelow(int limit) {
	return Graphics::Views::g_events->getRandomNumber(limit - 1);
}

CastSpell::CastSpell(int slot) : _inCombat(slot >= 0), _slot(slot) {
	if (slot >= 0)
		begin();
}

void CastSpell::begin() {
	const Data::RosterEntry &e = _G(savegame).partyMember(_slot);

	switch (e._class) {
	case 'D':
	case 'R':
		_G(messages).print("Spell type W/C-");
		_stage = CHOOSE_TYPE;
		break;

	case 'C':
	case 'P':
	case 'I':
		_cleric = true;
		_G(messages).print("Cleric spell-");
		_stage = CHOOSE_SPELL;
		break;

	case 'W':
	case 'L':
	case 'A':
		_G(messages).print("Wizard spell-");
		_stage = CHOOSE_SPELL;
		break;

	default:
		_G(messages).print("Not a mage!\n");
		g_engine->playSoundEffect(0xFF);
		_finished = true;
		break;
	}
}

bool CastSpell::fail() {
	_G(messages).print("Failed!\n");
	g_engine->playSoundEffect(0xFA);
	return true;
}

void CastSpell::fanfare() {
	g_engine->playSoundEffect(0xF5);
	_G(effects).flashViewport();
	g_engine->playSoundEffect(0xFD);
}

bool CastSpell::perform(Effect effect) {
	Data::Savegame &save = _G(savegame);
	CombatLogic &combat = _G(combat);
	const Data::RosterEntry &caster = save.partyMember(_slot);
	_effect = effect;

	switch (effect) {
	case SLAY_ORCS:
	case SLAY_SKELETONS:
		if (!_inCombat || !combat.castSlay(_slot, effect == SLAY_ORCS ? MONSTER_ORC : MONSTER_SKELETON))
			return fail();
		fanfare();
		return true;

	case MISSILE:
		_amount = rollBelow(0x28) | 0x10;
		break;
	case FIREBALL:
		_amount = 0x4B;
		break;
	case MIND_BLAST:
		_amount = (Data::fromBcd(caster._intelligence) * 2) & 0xFF;
		break;
	case ANNIHILATE:
		_amount = 0xFF;
		break;

	case LIGHT:
	case GREAT_LIGHT:
		save._lightTurns = (effect == LIGHT) ? 10 : 0xFA;
		fanfare();
		return true;

	case TELEPORT:
		if (save._location != Data::LOCATION_SOSARIA)
			return fail();

		fanfare();
		LocationLogic().teleportRandomly();
		return true;

	case WIZARD_TO_CLERIC:
		fanfare();
		_cleric = true;
		_G(messages).print("Cleric spell-");
		_stage = CHOOSE_SPELL;
		return false;

	case FIRE_STORM:
	case MIND_STORM:
	case DEATH_STORM:
		if (!_inCombat)
			return fail();

		fanfare();
		combat.castGroupDamage(_slot, effect == FIRE_STORM ? 0x4B : (effect == DEATH_STORM ? 0xFF :
			(Data::fromBcd(caster._intelligence) * 2) & 0xFF));
		return true;

	case HOLD_TIME:
		_G(holdTime) = 10;
		fanfare();
		return true;

	case DRAIN:
		if (!_inCombat)
			return fail();

		fanfare();
		combat.castDrain();
		return true;

	case UNLOCK: {
		fanfare();
		if (rollBelow(255) & 3) {
			if (save._location == Data::LOCATION_DUNGEON) {
				if (DungeonLogic().tile() == Data::DTILE_CHEST) {
					_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, 0);
					ChestLogic().loot(_slot);
					return true;
				}
			} else if (_G(map).cell(save._posX, save._posY) >= FIRST_CHEST_CELL &&
					_G(map).cell(save._posX, save._posY) <= LAST_CHEST_CELL) {
				byte cell = _G(map).cell(save._posX, save._posY);
				byte ground = (cell & 3) << 2;
				_G(map).setCell(save._posX, save._posY, ground ? ground : CELL_FLOOR);
				ChestLogic().loot(_slot);
				return true;
			}

			_G(messages).print("Not Here!\n");
			g_engine->playSoundEffect(0xFF);
		}

		return fail();
	}

	case HEAL_SMALL:
		_amount = rollBelow(20) + 10;
		_G(messages).print("Heal whom? ");
		_stage = TARGET;
		return false;

	case HEAL_LARGE:
		_amount = rollBelow(80) + 20;
		_G(messages).print("Heal whom? ");
		_stage = TARGET;
		return false;

	case CURE:
		_G(messages).print("Cure whom? ");
		_stage = TARGET;
		return false;

	case RESURRECT:
		if (_inCombat)
			return fail();

		_G(messages).print("Resurect whom? ");
		_stage = TARGET;
		return false;

	case RECALL:
		if (_inCombat)
			return fail();

		_G(messages).print("Recall whom? ");
		_stage = TARGET;
		return false;

	case DESCEND:
	case ASCEND:
	case RECALL_FLOOR:
	case LEAVE_DUNGEON:
		if (save._location != Data::LOCATION_DUNGEON)
			return fail();

		fanfare();
		if (effect == DESCEND) {
			if (save._dungeonLevel >= Data::DUNGEON_LEVELS - 1)
				return fail();

			++save._dungeonLevel;
			DungeonLogic().teleportRandomly();
		} else if (effect == ASCEND && save._dungeonLevel > 0) {
			--save._dungeonLevel;
			DungeonLogic().teleportRandomly();
		} else if (effect == RECALL_FLOOR) {
			DungeonLogic().teleportRandomly();
		} else {
			LocationLogic().exitToWorld();
		}

		return true;

	default:
		// The view of the world isn't available yet
		return fail();
	}

	// The spells that fly in a direction
	if (!_inCombat)
		return fail();

	_G(messages).print("Direct? ");
	_stage = DIRECTION;
	return false;
}

bool CastSpell::applyToPlayer(int target) {
	Data::RosterEntry &e = _G(savegame).partyMember(target);
	Data::RosterEntry &caster = _G(savegame).partyMember(_slot);

	switch (_effect) {
	case HEAL_SMALL:
	case HEAL_LARGE: {
		int hitPoints = MIN(Data::fromBcdWord(e._hitPoints) + _amount, Data::fromBcdWord(e._maxHitPoints));
		e._hitPoints = Data::toBcdWord(hitPoints);
		_G(effects).flashSlot(target);
		fanfare();
		return true;
	}

	case CURE:
		if (e._status != Data::STATUS_POISONED)
			return fail();

		_G(effects).flashSlot(target);
		fanfare();
		e._status = Data::STATUS_GOOD;
		return true;

	case RESURRECT:
		if (e._status != Data::STATUS_DEAD)
			return fail();

		_G(effects).flashSlot(target);
		fanfare();
		e._status = (rollBelow(255) & 3) ? Data::STATUS_GOOD : Data::STATUS_ASHES;
		return true;

	default:
		if (e._status != Data::STATUS_ASHES)
			return fail();

		_G(effects).flashSlot(target);
		fanfare();
		e._status = Data::STATUS_GOOD;
		caster._wisdom = Data::toBcd(MAX(Data::fromBcd(caster._wisdom) - 5, 0));
		return true;
	}
}

bool CastSpell::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	switch (_stage) {
	case CHOOSE_CASTER: {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		_slot = _players.slot();
		if (!save.partyMember(_slot).isAlive()) {
			_G(messages).print("Incapacitated!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		begin();
		return _finished;
	}

	case CHOOSE_TYPE: {
		char key = commandKey(msg);

		if (msg.keycode == Common::KEYCODE_ESCAPE) {
			_G(messages).print("\n");
			return true;
		}
		if (key != 'W' && key != 'C') {
			g_engine->playSoundEffect(0xFF);
			return false;
		}

		_cleric = key == 'C';
		_G(messages).putChar((char)msg.ascii);
		_G(messages).print(_cleric ? "Cleric spell-" : "Wizard spell-");
		_stage = CHOOSE_SPELL;
		return false;
	}

	case CHOOSE_SPELL: {
		char key = commandKey(msg);

		if (msg.keycode == Common::KEYCODE_ESCAPE) {
			_G(messages).print("\n");
			return true;
		}
		if (key < 'A' || key > 'P') {
			g_engine->playSoundEffect(0xFF);
			return false;
		}

		_G(messages).putChar((char)msg.ascii);
		_G(messages).print("\n");

		// Spells cost five magic points for each place down the book
		int spell = key - 'A';
		Data::RosterEntry &caster = save.partyMember(_slot);
		_cost = spell * MP_PER_LEVEL;
		if (_cost > Data::fromBcd(caster._magicPoints)) {
			_G(messages).print("M.P. too low!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		caster._magicPoints = Data::toBcd(Data::fromBcd(caster._magicPoints) - _cost);

		_G(messages).print("\n");
		_G(messages).print(Data::NAMES[SPELL_NAMES_START + spell + (_cleric ? SPELLS_PER_BOOK : 0)]);
		_G(messages).print("\n\n");

		return perform((Effect)(_cleric ? CLERIC_EFFECTS[spell] : WIZARD_EFFECTS[spell]));
	}

	case DIRECTION: {
		DirectionChooser::Result result = _directions.handleKey(msg);
		if (result == DirectionChooser::PENDING)
			return false;

		if (result == DirectionChooser::CHOSEN) {
			fanfare();
			_G(combat).castProjectile(_slot, _amount, _directions.direction());
		}

		return true;
	}

	default: {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;

		if (result == PlayerChooser::CANCELLED) {
			// Curing is refunded if no one is picked, and the rest don't mind
			if (_effect == CURE) {
				Data::RosterEntry &caster = save.partyMember(_slot);
				caster._magicPoints = Data::toBcd(MIN(Data::fromBcd(caster._magicPoints) + CURE_REFUND, 99));
				return true;
			}

			return (_effect == RESURRECT || _effect == RECALL) ? fail() : true;
		}

		return applyToPlayer(_players.slot());
	}
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
