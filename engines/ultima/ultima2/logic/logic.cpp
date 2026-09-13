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

void Logic::playerDied() {
	Data::Savegame &sg = _G(savegame);
	writeString("\n%s IS DEAD!\n", sg._name);

	sg._hp = 0;
	sg._food = 0;
	sg._foodTurnCtr = 0;
	sg._experience = 0;
	sg._gold = 0;

	// UIElement* overload used directly since the string-name one drops replaceAllViews
	g_engine->replaceView(g_engine->findView("Title"), true);
}

void Logic::alertTownGuards(int extraSlot) {
	Data::Savegame &sg = _G(savegame);
	if (sg._mapNum2 == 0)
		return;

	Data::MapMonsters &monsters = _G(map)._monsters;
	for (int slot = 0; slot <= 7; ++slot)
		monsters._offerFlag[slot] = 1;

	if (extraSlot >= 0)
		monsters._offerFlag[extraSlot] = 1;
}

int Logic::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(minNumber, maxNumber);
}

int Logic::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

byte Logic::randByte() {
	return g_engine->randByte();
}

void Logic::showAttackTile(int x, int y) {
	g_engine->focusedView()->send(AttackTileMessage(x, y, 0));
}

void Logic::playFX(int num) {
	g_engine->playFX(num);
}

void Logic::endOfTurn() {
	updateCreatures();

	Data::Savegame &sg = _G(savegame);
	if (sg._legParalysisTurns > 0)
		--sg._legParalysisTurns;
	if (sg._armParalysisTurns > 0)
		--sg._armParalysisTurns;
	if (sg._sleepTurns > 0)
		--sg._sleepTurns;
	if (sg._negateTimeTurns > 0)
		--sg._negateTimeTurns;

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

void Logic::resolveDirection(Data::Direction dir) {
	switch (_directionPurpose) {
	case DirectionPurpose::ATTACK:
		attack(dir);
		break;
	case DirectionPurpose::STEAL:
		steal(dir);
		break;
	case DirectionPurpose::UNLOCK:
		unlock(dir);
		break;
	case DirectionPurpose::OFFER:
		offer(dir);
		break;
	case DirectionPurpose::TRANSACT:
		transact(dir);
		break;
	}
}

bool Logic::trySpendGold(int amount) {
	Data::Savegame &sg = _G(savegame);
	if (sg._gold < amount) {
		writeString("YOU DONT HAVE THAT MUCH!\n");
		return false;
	}

	sg._gold -= amount;
	return true;
}

void Logic::keypress(const Shared::Messages::KeypressMessage &msg) {
	writeString("Huh?\n");
	endOfTurn();
	prompt();
}

bool Logic::attack(Data::Direction dir) {
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
	Data::Savegame &sg = _G(savegame);

	if (sg._mapNum2 >= 4) {
		// Dungeon/tower chest handling - not yet implemented
		writeString("Get?\n");
		return true;
	}

	Data::TileId tile = _G(map).tileAt(sg._mapX, sg._mapY);
	if (tile == Data::TILE_SWORD) {
		int r = randByte() & 7;
		if (r == 0)
			writeString("EMPTY!\n");
		else {
			++sg._weaponOwned[r];
			writeString("GOT %s!\n", Data::WEAPON_NAMES[r]);
		}
	} else if (tile == Data::TILE_SHIELD) {
		int r = randByte() & 3;
		if (r == 0)
			writeString("EMPTY!\n");
		else {
			++sg._armorOwned[r];
			writeString("GOT %s!\n", Data::ARMOR_NAMES[r]);
		}
	} else {
		writeString("GET WHAT?\n");
	}

	return true;
}

bool Logic::hyper() {
	writeString("Hyper?\n");
	return true;
}

bool Logic::igniteTorch() {
	writeString("IGNITE TORCH\n");
	Data::Savegame &sg = _G(savegame);

	if (sg._mapNum2 < 4)
		return true;

	if (sg._torches == 0) {
		writeString("NONE OWNED!\n");
		return true;
	}

	--sg._torches;
	sg._lightTurns = 150;
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
	g_engine->addView("ReadySpell");
	return false;
}

bool Logic::negateTime() {
	Data::Savegame &sg = _G(savegame);

	if (sg._items[Data::ITEM_STRANGE_COIN] == 0) {
		writeString(" HOW?  YOU'RE NOT EINSTEIN\n");
		return true;
	}

	--sg._items[Data::ITEM_STRANGE_COIN];
	writeString("YOU RUB A COIN...\n");
	sg._negateTimeTurns = 20;
	return true;
}

bool Logic::offer(Data::Direction dir) {
	writeString("Offer?\n");
	return true;
}

bool Logic::pass() {
	writeString("PASS\n");
	return true;
}

bool Logic::quit() {
	writeString("Quit?\n");
	return true;
}

bool Logic::ready() {
	g_engine->addView("ReadyWeapon");
	return false;
}

bool Logic::steal(Data::Direction dir) {
	writeString("Steal?\n");
	return true;
}

bool Logic::transact(Data::Direction dir) {
	writeString("Transact?\n");
	return true;
}

bool Logic::unlock(Data::Direction dir) {
	writeString("Unlock?\n");
	return true;
}

bool Logic::view() {
	Data::Savegame &sg = _G(savegame);

	if (sg._items[Data::ITEM_HELM] == 0 || sg._mapNum2 >= 4) {
		writeString("VIEW WHAT?\n");
		return true;
	}

	writeString("VIEW\nWITH MAGICAL HELM!\n");
	--sg._items[Data::ITEM_HELM];
	g_engine->addView("WorldMapOverview");
	return false;
}

bool Logic::wearArmor() {
	g_engine->addView("WearArmor");
	return false;
}

bool Logic::xit() {
	writeString("eXit?\n");
	return true;
}

bool Logic::yell() {
	g_engine->addView("Yell");
	return false;
}

bool Logic::zstats() {
	g_engine->addView("ZStats");
	return false;
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
