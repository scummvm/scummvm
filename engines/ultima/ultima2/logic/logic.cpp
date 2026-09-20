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
	auto *view = g_engine->baseView()->findView("Stats");
	if (view)
		view->draw();
}

void Logic::playerDied() {
	Data::Savegame &sg = _G(savegame);
	writeString("\n%s IS DEAD!\n", sg._name);

	g_engine->replaceView("Dead", true);
}

void Logic::alertTownGuards(int extraSlot) {
	Data::Savegame &sg = _G(savegame);
	if (sg._mapType == 0)
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

	// Sleeping players lose the command they were about to give
	if (_G(savegame)._sleepTurns > 0) {
		writeString("ZZZZZZZZZZZZZZZZZZZZ\n");
		resumeTurn();
		return;
	}

	if (action != KEYBIND_UP && action != KEYBIND_DOWN && action != KEYBIND_LEFT && action != KEYBIND_RIGHT)
		_monstersSkipTurn = false;

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
		doEndOfTurn = fire(Data::DIR_UNSPECIFIED);
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
	bool doEndOfTurn = false;
	switch (_directionPurpose) {
	case DirectionPurpose::ATTACK:
		doEndOfTurn = attack(dir);
		break;
	case DirectionPurpose::FIRE:
		doEndOfTurn = fire(dir);
		break;
	case DirectionPurpose::STEAL:
		doEndOfTurn = steal(dir);
		break;
	case DirectionPurpose::UNLOCK:
		doEndOfTurn = unlock(dir);
		break;
	case DirectionPurpose::OFFER:
		doEndOfTurn = offer(dir);
		break;
	case DirectionPurpose::TRANSACT:
		doEndOfTurn = transact(dir);
		break;
	}

	if (doEndOfTurn)
		resumeTurn();
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
	Data::Savegame &sg = _G(savegame);
	writeString("BOARD");

	if (sg._mapType >= 4 || sg._mount != 0) {
		writeString("\nTHINK AGAIN %s\n", sg._name);
		return true;
	}

	Data::TileId vehicle = _G(map).tileAt(sg._mapX, sg._mapY);
	Data::TileId under = Data::TILE_GRASS;

	switch (vehicle) {
	case Data::TILE_HORSE:
		writeString(" HORSE");
		break;
	case Data::TILE_SHIP:
		if (sg._items[Data::ITEM_BLUE_TASSLE] == 0) {
			writeString(" SHIP\nTHE CREW OF THIS SHIP\nWILL NOT LET YOU BOARD!\n");
			return true;
		}
		writeString(" FRIGATE");
		under = Data::TILE_WATER;
		break;
	case Data::TILE_AIRPLANE:
		if (sg._items[Data::ITEM_SKULL_KEY] == 0) {
			writeString(" PLANE\nSTRANGE YOU CAN'T GET IN!\n");
			return true;
		}
		writeString(" PLANE");
		break;
	case Data::TILE_ROCKET:
		writeString(" ROCKET");
		if (sg._items[Data::ITEM_ANKH] == 0) {
			writeString("\nA METALIC VOICE COMMANDS:\nYOU MUST HAVE AN ANKH!\n");
			return true;
		}
		break;
	default:
		writeString(" WHAT?\n");
		return true;
	}

	_G(map)._tiles[sg._mapY][sg._mapX] = under;
	sg._mount = vehicle;
	writeString("\n");
	return true;
}

bool Logic::cast() {
	Data::Savegame &sg = _G(savegame);
	writeString("CAST-%s", Data::SPELL_NAMES[sg._readiedSpell]);

	if (sg._items[Data::ITEM_WAND] + sg._items[Data::ITEM_STAFF] == 0) {
		writeString("\nNEED WAND OR STAFF!\n");
		return true;
	}

	if (sg._readiedSpell == Data::SPELL_NONE) {
		writeString("\n");
		return true;
	}

	if (sg._spellCharges[sg._readiedSpell] == 0) {
		writeString("\nNO SPELL!\n");
		return true;
	}

	--sg._spellCharges[sg._readiedSpell];
	return castSpell(sg._readiedSpell);
}

bool Logic::castSpell(Data::SpellType spell) {
	// Spells only work in towers and dungeons
	writeString("-FAILED!\n");
	return true;
}

bool Logic::descend() {
	// Ladders only exist in towers and dungeons
	writeString("DESCEND-WHAT?\n");
	return true;
}

bool Logic::enter() {
	writeString("ENTER WHAT?\n");
	return true;
}

bool Logic::fire(Data::Direction dir) {
	writeString("FIRE WHAT?\n");
	return true;
}

bool Logic::get() {
	Data::Savegame &sg = _G(savegame);
	writeString("GET");

	Data::TileId tile = _G(map).tileAt(sg._mapX, sg._mapY);
	if (tile != Data::TILE_SWORD && tile != Data::TILE_SHIELD) {
		writeString(" WHAT?\n");
		return true;
	}

	_G(map)._tiles[sg._mapY][sg._mapX] = Data::TILE_GRASS;

	if (tile == Data::TILE_SWORD) {
		writeString(" WEAPON");
		int r = randByte() & 7;
		if (r != 0) {
			++sg._weaponOwned[r];
			writeString("\n");
			return true;
		}
	} else {
		writeString(" ARMOUR");
		int r = randByte() & 3;
		if (r != 0) {
			++sg._armorOwned[r];
			writeString("\n");
			return true;
		}
	}

	writeString(" EMPTY!\n");
	return true;
}

bool Logic::hyper() {
	writeString("HYPER WHAT?\n");
	return true;
}

bool Logic::igniteTorch() {
	writeString("IGNITE TORCH\n");
	Data::Savegame &sg = _G(savegame);

	if (sg._mapType < 4)
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
	writeString("JUMP...WHEE...\n");
	return true;
}

bool Logic::klimb() {
	// Ladders only exist in towers and dungeons
	writeString("KLIMB-WHAT?\n");
	return true;
}

bool Logic::launch() {
	Data::Savegame &sg = _G(savegame);

	if (sg._mount == Data::TILE_AIRPLANE) {
		writeString("LAUNCH--PLANE");
		if (sg._items[Data::ITEM_BRASS_BUTTON] == 0) {
			writeString("\nFUNNY THIS PLANE IS\nMISSING A BRASS BUTTON!\n");
			return true;
		}

		g_engine->addView("PlaneFlight");
		return false;
	} else if (sg._mount == Data::TILE_ROCKET) {
		writeString("LAUNCH--ROCKET");
		if (sg._items[Data::ITEM_TRI_LITHIUM] == 0)
			writeString("\nA METALLIC VOICE SAYS:\nSHIP INCAPABLE OF LAUNCH!\n");
		else
			writeString("\nNOT YET IMPLEMENTED\n");
	} else {
		writeString("LAUNCH WHAT?\n");
	}

	return true;
}

bool Logic::magic() {
	g_engine->addView("ReadySpell");
	return false;
}

bool Logic::negateTime() {
	Data::Savegame &sg = _G(savegame);
	writeString("NEGATE TIME");

	if (sg._items[Data::ITEM_STRANGE_COIN] == 0) {
		writeString(" HOW?\nYOU'RE NOT EINSTEIN\n");

	} else {
		--sg._items[Data::ITEM_STRANGE_COIN];
		writeString("\nYOU RUB A COIN...\n");
		sg._negateTimeTurns = 20;
	}

	return true;
}

bool Logic::offer(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("OFFER GOLD DIRECT-");
		_directionPurpose = DirectionPurpose::OFFER;
		g_engine->addView("Direction");
		return false;
	}

	// Outside a settlement there's never anyone to give gold to
	writeString("\nOFFER TO WHOM?\n");
	return true;
}

bool Logic::pass() {
	writeString("PASS\n");
	return true;
}

bool Logic::quit() {
	Data::Savegame &sg = _G(savegame);
	writeString("QUIT OR SAVE GAME.");

	if (sg._mapType != 0)
		writeString("\nONLY OUTDOORS!\n");
	else if (sg._saveDisabled)
		writeString("\nONLY ON EARTH!\n");
	else if (sg._mount != 0)
		writeString("\nONLY ON FOOT!\n");
	else {
		writeString("\nONE MOMENT PLEASE!\n");
		g_engine->saveGameDialog();
	}

	return true;
}

bool Logic::ready() {
	g_engine->addView("ReadyWeapon");
	return false;
}

bool Logic::steal(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("STEAL DIRECT-");
		_directionPurpose = DirectionPurpose::STEAL;
		g_engine->addView("Direction");
		return false;
	}

	// Outside a settlement there's nothing to steal
	writeString("\nNO LUCK!\n");
	if ((randByte() & 7) == 0)
		alertTownGuards();
	return true;
}

bool Logic::transact(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("TRANSACT-");
		_directionPurpose = DirectionPurpose::TRANSACT;
		g_engine->addView("Direction");
		return false;
	}

	writeString("\nFUNNY, NO RESPONSE!\n");
	return true;
}

bool Logic::unlock(Data::Direction dir) {
	if (dir == Data::DIR_UNSPECIFIED) {
		writeString("UNLOCK DIRECTION-");
		_directionPurpose = DirectionPurpose::UNLOCK;
		g_engine->addView("Direction");
		return false;
	}

	// Outside a settlement there are no doors
	writeString("\nNO DOOR THERE!\n");
	return true;
}

bool Logic::view() {
	Data::Savegame &sg = _G(savegame);

	if (sg._items[Data::ITEM_HELM] == 0 || sg._mapType >= 4) {
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
	Data::Savegame &sg = _G(savegame);
	writeString("X-IT");

	if (sg._mount == 0) {
		writeString(" WHAT?\n");
		return true;
	}

	Data::TileId under = _G(map).tileAt(sg._mapX, sg._mapY);
	if (under != Data::TILE_GRASS && !(under == Data::TILE_WATER && sg._mount == Data::TILE_SHIP)) {
		writeString("-NOT HERE!\n");
		return true;
	}

	_G(map)._tiles[sg._mapY][sg._mapX] = (Data::TileId)sg._mount;
	sg._mount = 0;
	writeString("\n");
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
