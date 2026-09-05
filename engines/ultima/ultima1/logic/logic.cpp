
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

	redrawMap();
	redrawStats();
	prompt();

	// If this logic is no longer the active one, such as from switching maps,
	// then delete it
	if (this != _G(logic))
		delete this;
}

void Logic::action(int action) {
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
	case KEYBIND_CLIMB:
		doEndOfTurn = climb();
		break;
	case KEYBIND_DROP:
		doEndOfTurn = drop();
		break;
	case KEYBIND_ENTER:
		doEndOfTurn = enter();
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

	if (doEndOfTurn)
		endOfTurn();
}

void Logic::keypress(Common::KeyCode keycode) {
	writeString("Huh?\n");
	endOfTurn();
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

bool Logic::zstats() {
	writeString("Ztats\n");
	g_engine->addView("ZStats");
	return false;
}

bool Logic::pass() {
	writeString("Pass\n");
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
