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

#include "ultima/ultima2/logic/logic.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

using namespace Shared::Messages;

void Logic::writeString(const Common::String &msg) {
	g_engine->baseView()->findView("Commands")->send(GameMessage("TEXT", msg));
}

void Logic::writeString(const char *format, ...) {
	va_list alist;
	va_start(alist, format);
	Common::String msg = Common::String::vformat(format, alist);
	va_end(alist);

	writeString(msg);
}

void Logic::prompt() {
	g_engine->baseView()->findView("Commands")->send(GameMessage("PROMPT"));
}

void Logic::redrawMap() {
	g_engine->focusedView()->redraw();
}

void Logic::redrawStats() {
	g_engine->baseView()->findView("Stats")->draw();
}

int Logic::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(minNumber, maxNumber);
}

int Logic::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

void Logic::endOfTurn() {
	updateCreatures();
	redrawMap();
	redrawStats();
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
		doEndOfTurn = attack();
		break;
	case KEYBIND_BOARD:
		doEndOfTurn = board();
		break;
	case KEYBIND_CAST:
		doEndOfTurn = cast();
		break;
	case KEYBIND_DESCEND:
		doEndOfTurn = descend();
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
	case KEYBIND_HYPER:
		doEndOfTurn = hyper();
		break;
	case KEYBIND_IGNITE_TORCH:
		doEndOfTurn = igniteTorch();
		break;
	case KEYBIND_JUMP:
		doEndOfTurn = jump();
		break;
	case KEYBIND_KLIMB:
		doEndOfTurn = klimb();
		break;
	case KEYBIND_LAUNCH:
		doEndOfTurn = launch();
		break;
	case KEYBIND_MAGIC:
		doEndOfTurn = magic();
		break;
	case KEYBIND_NEGATE_TIME:
		doEndOfTurn = negateTime();
		break;
	case KEYBIND_OFFER:
		doEndOfTurn = offer();
		break;
	case KEYBIND_PASS:
		doEndOfTurn = pass();
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
	case KEYBIND_UNLOCK:
		doEndOfTurn = unlock();
		break;
	case KEYBIND_VIEW:
		doEndOfTurn = view();
		break;
	case KEYBIND_WEAR_ARMOR:
		doEndOfTurn = wearArmor();
		break;
	case KEYBIND_X_IT:
		doEndOfTurn = xit();
		break;
	case KEYBIND_YELL:
		doEndOfTurn = yell();
		break;
	case KEYBIND_ZSTATS:
		doEndOfTurn = zstats();
		break;
	default:
		writeString("Huh?\n");
		break;
	}

	if (doEndOfTurn) {
		// Only do end of turn stuff like creature movement and food
		// reduction if we're still in the same mode we started on
		if (_G(logic) == currLogic)
			endOfTurn();

		if (_G(savegame)._hp > 0)
			prompt();
	}
}

void Logic::keypress(Common::KeyCode keycode) {
	writeString("Huh?\n");
	endOfTurn();
	prompt();
}

bool Logic::attack() {
	writeString("Attack?\n");
	return true;
}

bool Logic::board() {
	writeString("Board?\n");
	return true;
}

bool Logic::cast() {
	writeString("Cast?\n");
	return true;
}

bool Logic::descend() {
	writeString("Descend?\n");
	return true;
}

bool Logic::enter() {
	writeString("Enter?\n");
	return true;
}

bool Logic::fire() {
	writeString("Fire?\n");
	return true;
}

bool Logic::get() {
	writeString("Get?\n");
	return true;
}

bool Logic::hyper() {
	writeString("Hyper?\n");
	return true;
}

bool Logic::igniteTorch() {
	writeString("Ignite Torch?\n");
	return true;
}

bool Logic::jump() {
	writeString("Jump?\n");
	return true;
}

bool Logic::klimb() {
	writeString("Klimb?\n");
	return true;
}

bool Logic::launch() {
	writeString("Launch?\n");
	return true;
}

bool Logic::magic() {
	writeString("Magic?\n");
	return true;
}

bool Logic::negateTime() {
	writeString("Negate Time?\n");
	return true;
}

bool Logic::offer() {
	writeString("Offer?\n");
	return true;
}

bool Logic::pass() {
	writeString("Pass\n");
	return true;
}

bool Logic::quit() {
	writeString("Quit?\n");
	return true;
}

bool Logic::ready() {
	writeString("Ready?\n");
	return true;
}

bool Logic::steal() {
	writeString("Steal?\n");
	return true;
}

bool Logic::transact() {
	writeString("Transact?\n");
	return true;
}

bool Logic::unlock() {
	writeString("Unlock?\n");
	return true;
}

bool Logic::view() {
	writeString("View?\n");
	return true;
}

bool Logic::wearArmor() {
	writeString("Wear Armor?\n");
	return true;
}

bool Logic::xit() {
	writeString("eXit?\n");
	return true;
}

bool Logic::yell() {
	writeString("Yell?\n");
	return true;
}

bool Logic::zstats() {
	writeString("Zstats\n");
	return true;
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
