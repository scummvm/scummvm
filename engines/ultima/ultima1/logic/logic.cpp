
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

#include "ultima/ultima1/logic/logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

using namespace Shared::Messages;

void Logic::writeString(const Common::String &msg) {
	g_engine->findView("Commands")->send(GameMessage("TEXT", msg));
}

void Logic::writeString(const char *format, ...) {
	va_list alist;

	// Start reading values
	va_start(alist, format);
	Common::String msg = Common::String::vformat(format, alist);
	va_end(alist);

	writeString(msg);
}

void Logic::prompt() {
	g_engine->findView("Commands")->send(GameMessage("PROMPT"));
}

void Logic::redrawMap() {
	g_engine->focusedView()->redraw();
}

void Logic::showAttackTile(int x, int y, int tileId) {
	g_engine->focusedView()->send(AttackTileMessage(x, y, tileId));
}

void Logic::redrawStats() {
	g_engine->findView("Stats")->draw();
}

void Logic::playFX(int num) {
	g_engine->playFX(num);
}

int Logic::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(minNumber, maxNumber);
}

int Logic::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

void Logic::endOfTurn() {
	_G(moveCtr)++;
	updateCreatures();

	redrawMap();
	redrawStats();

	if (_G(savegame)._hits == 0)
		g_engine->addView("Dead");
}

void Logic::action(int action) {
	Common::SharedPtr<Logic> currLogic = _G(logic);
	bool doEndOfTurn = true;

	switch (action) {
	case KEYBIND_UP:
		doEndOfTurn = move(Data::DIR_UP);
		break;
	case KEYBIND_DOWN:
		doEndOfTurn = move(Data::DIR_DOWN);
		break;
	case KEYBIND_LEFT:
		doEndOfTurn = move(Data::DIR_LEFT);
		break;
	case KEYBIND_RIGHT:
		doEndOfTurn = move(Data::DIR_RIGHT);
		break;
	case KEYBIND_ATTACK:
		doEndOfTurn = attack(Data::DIR_UNSPECIFIED);
		break;
	case KEYBIND_ATTACK_UP:
		doEndOfTurn = attack(Data::DIR_UP);
		break;
	case KEYBIND_ATTACK_DOWN:
		doEndOfTurn = attack(Data::DIR_DOWN);
		break;
	case KEYBIND_ATTACK_LEFT:
		doEndOfTurn = attack(Data::DIR_LEFT);
		break;
	case KEYBIND_ATTACK_RIGHT:
		doEndOfTurn = attack(Data::DIR_RIGHT);
		break;
	case KEYBIND_BOARD:
		doEndOfTurn = board();
		break;
	case KEYBIND_CLIMB:
		doEndOfTurn = climb();
		break;
	case KEYBIND_DROP:
		doEndOfTurn = drop();
		break;
	case KEYBIND_ENTER:
		doEndOfTurn = enter();
		break;
	case KEYBIND_EXIT:
		doEndOfTurn = xit();
		break;
	case KEYBIND_FIRE:
		doEndOfTurn = fire();
		break;
	case KEYBIND_GET:
		doEndOfTurn = get();
		break;
	case KEYBIND_HYPERJUMP:
		doEndOfTurn = hyperjump();
		break;
	case KEYBIND_INFORM:
		doEndOfTurn = inform();
		break;
	case KEYBIND_NOISE:
		doEndOfTurn = noise();
		break;
	case KEYBIND_OPEN:
		doEndOfTurn = open();
		break;
	case KEYBIND_QUIT:
		doEndOfTurn = quit();
		break;
	case KEYBIND_READY:
		doEndOfTurn = ready();
		break;
	case KEYBIND_STEAL:
		doEndOfTurn = steal();
		break;
	case KEYBIND_TRANSACT:
		doEndOfTurn = transact();
		break;
	case KEYBIND_PASS:
		doEndOfTurn = pass();
		break;
	case KEYBIND_STATS:
		doEndOfTurn = zstats();
		break;
	default:
		writeString("Huh?\n");
		break;
	}

	// End of turn handling
	if (doEndOfTurn) {
		// Only do end of turn stuff like creature movement and food reduction if we're
		// still in the same map we started on
		if (_G(logic) == currLogic)
			endOfTurn();

		if (_G(savegame)._hits > 0)
			prompt();
	}
}

void Logic::keypress(Common::KeyCode keycode) {
	writeString("Huh?\n");
	endOfTurn();
}

bool Logic::attack(Data::Direction dir) {
	writeString("Attack with %s", Data::WEAPON_NAMES_LOWER[_G(savegame)._equippedWeapon]);

	if (Data::WEAPONS_DISTANCE[_G(savegame)._equippedWeapon]) {
		writeString(": ");

		if (dir == Data::DIR_UNSPECIFIED) {
			_G(logic) = Common::SharedPtr<Logic>(new DirectionLogic(DirectionLogic::WEAPON));
			g_engine->addView("Direction");
			return false;
		} else {
			combat(dir, 7);
			return true;
		}
	} else {
		// It's a non-attacking "weapon" like the rope
		writeString("?\n");
		playFX(1);
		return true;
	}
}

bool Logic::board() {
	writeString("Board?\n");
	playFX(1);
	return true;
}

bool Logic::cast() {
	writeString("Cast -- Hmmmm... no effect!\n");
	playFX(6);
	return true;
}

bool Logic::climb() {
	writeString("K-Limb?\n");
	playFX(1);
	return true;
}

bool Logic::drop() {
	writeString("Drop?\n");
	playFX(1);
	return true;
}

bool Logic::enter() {
	writeString("Enter?\n");
	playFX(1);
	return true;
}

bool Logic::fire() {
	writeString("Fire?\n");
	playFX(1);
	return true;
}

bool Logic::get() {
	writeString("Get?\n");
	playFX(1);
	return true;
}

bool Logic::hyperjump() {
	writeString("HyperJump?\n");
	playFX(1);
	return true;
}

bool Logic::inform() {
	writeString("Inform and search\n");
	int location = _G(map).getLocationAt(_G(savegame)._overworldPos);
	int continent = _G(map).getContinentAt(_G(savegame)._overworldPos);

	if (location) {
		if (location < 33)
			writeString("the city of ");

		writeString("%s\n", Data::LOCATION_NAMES[location - 1]);

	} else {
		switch (_G(map).getMapTile(_G(savegame)._overworldPos.x, _G(savegame)._overworldPos.y)) {
		case Data::TILE_OCEAN:
			writeString("You are at sea\n");
			break;
		case Data::TILE_WOODS:
			writeString("You are in the woods\n");
			break;
		default:
			writeString("You are in the lands\n");
			writeString("%s\n", Data::CONTINENT_NAMES[continent]);
			break;
		}
	}

	return true;
}

bool Logic::noise() {
	_G(savegame)._soundOn = !_G(savegame)._soundOn;
	writeString("Noise %s\n", _G(savegame)._soundOn ? "on" : "off");
	return true;
}

bool Logic::open() {
	writeString("Open?\n");
	playFX(1);
	return true;
}

bool Logic::quit() {
	writeString("Quit - only allowed outdoors!\n");
	playFX(1);
	return true;
}

bool Logic::ready() {
	g_engine->addView("Ready");
	return false;
}

bool Logic::steal() {
	writeString("Steal?\n");
	playFX(1);
	return true;
}

bool Logic::transact() {
	writeString("Transact?\n");
	playFX(1);
	return true;
}

bool Logic::xit() {
	writeString("Xit?\n");
	playFX(1);
	return true;
}

bool Logic::zstats() {
	writeString("Ztats\n");
	g_engine->addView("ZStats");
	return false;
}

bool Logic::pass() {
	writeString("Pass\n");
	return true;
}

void Logic::combat(Data::Direction direction, int effect) {
	if (direction == Data::DIR_UNSPECIFIED) {
		writeString("nothing\n");
	} else {
		writeString("%s\n", Data::DIRECTION_NAMES[direction]);
		combatDir(direction, effect);
	}
}

void Logic::combatDir(Data::Direction direction, int effect) {
	Data::Savegame &sg = _G(savegame);
	int maxDistance, hitChance, strike;

	if (effect == 7) {
		// Attacking with the readied weapon
		maxDistance = Data::WEAPONS_DISTANCE[sg._equippedWeapon];
		hitChance = sg._agility + 50;
		strike = getRandomNumber(2, sg._equippedWeapon * 8 + sg._strength);
	} else {
		// Firing a frigate's cannons/aircar's lasers
		maxDistance = 3;
		hitChance = 80;
		strike = getRandomNumber(1, sg._transportType * 10) + 30;
	}

	damage(direction, effect, maxDistance, strike, hitChance, Data::TILE_ATTACK);
}

/*-------------------------------------------------------------------*/

DirectionLogic::DirectionLogic(Mode mode) : _mode(mode) {
	_oldLogic = _G(logic);
}

void DirectionLogic::action(int action) {
	Data::Direction dir;

	switch (action) {
	case KEYBIND_UP:
		dir = Data::DIR_UP;
		break;
	case KEYBIND_DOWN:
		dir = Data::DIR_DOWN;
		break;
	case KEYBIND_LEFT:
		dir = Data::DIR_LEFT;
		break;
	case KEYBIND_RIGHT:
		dir = Data::DIR_RIGHT;
		break;
	default:
		dir = Data::DIR_UNSPECIFIED;
		break;
	}

	_G(logic) = _oldLogic;

	switch (_mode) {
	case SPELL:
		_oldLogic->castSpellAttack(dir);
		break;
	case WEAPON:
		_oldLogic->combat(dir, 7);
		break;
	case FIRE:
	default:
		_oldLogic->combat(dir, 8);
		break;
	}
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
