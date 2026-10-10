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

#include "common/file.h"
#include "ultima/ultima3/views/interactions/dungeon_special.h"
#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int FOUNTAIN_DAMAGE = 25;
constexpr int ROD_DAMAGE = 50;
constexpr int GREMLIN_DAMAGE = 5;
constexpr int GREMLIN_FOOD = 100;

DungeonSpecial::DungeonSpecial(byte kind) : _kind(kind) {
	Data::Savegame &save = _G(savegame);

	switch (kind) {
	case Data::DTILE_TIME_LORD:
		_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, 0);
		showScene("TIME.IMG");
		_G(messages).print("You see a vision\nof the Time Lord\n  He tells you\n The one way is\n"
			"   Love, Sol,\n Moons & Death,\nAll else fails.");
		_waiting = true;
		break;

	case Data::DTILE_FOUNTAIN:
		showScene("FOUNTAIN.IMG");
		askWhoDrinks();
		break;

	case Data::DTILE_WIND:
		_G(messages).print("Strange wind!\n");
		save._lightTurns = 0;
		_finished = true;
		break;

	case Data::DTILE_TRAP:
		_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, 0);
		trap();
		_finished = true;
		break;

	case Data::DTILE_MARK:
		showScene("BRAND.IMG");
		_G(messages).print("A red hot rod\nin the wall. Who\nwill touch? ");
		break;

	default:
		_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, 0);
		gremlins();
		_finished = true;
		break;
	}
}

DungeonSpecial::~DungeonSpecial() {
	hideScene();
}

void DungeonSpecial::showScene(const char *filename) {
	Common::File f;
	if (!f.open(filename) || f.read(_G(scene), sizeof(_G(scene))) != sizeof(_G(scene)))
		error("Could not load %s", filename);

	_G(sceneShown) = true;
}

void DungeonSpecial::hideScene() {
	_G(sceneShown) = false;
}

void DungeonSpecial::askWhoDrinks() {
	_G(messages).print("\nA fountain.  Who\nwill drink? ");
}

void DungeonSpecial::drink(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	if (!e.isAlive()) {
		_G(messages).print("Can't!\n");
		g_engine->playSoundEffect(0xFF);
		return;
	}

	// Which fountain it is depends on where it is
	switch (_G(savegame)._posY & 3) {
	case 0:
		_G(messages).print("Yuck! Horrible!\n");
		e._status = Data::STATUS_POISONED;
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);
		break;

	case 1:
		e._hitPoints = e._maxHitPoints;
		_G(messages).print("How wonderful!\n");
		break;

	case 2:
		_G(messages).print("Argh! Blah! Yuk!\n");
		ChestLogic().damageCharacter(slot, FOUNTAIN_DAMAGE);
		_G(effects).flashViewport();
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);
		break;

	default:
		_G(messages).print("Ah! That's nice!\n");
		e._status = Data::STATUS_GOOD;
		break;
	}
}

void DungeonSpecial::touchRod(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	// Each rod leaves a different mark, depending on where it is
	e._marksAndCards |= 1 << ((_G(savegame)._posY & 3) + 4);

	_G(effects).flashSlot(slot);
	g_engine->playSoundEffect(0xF7);
	ChestLogic().damageCharacter(slot, ROD_DAMAGE);
	_G(messages).print("It left a mark!\n");
}

void DungeonSpecial::trap() {
	ChestLogic chest;

	_G(messages).print("Argh!! A trap!!\n");
	g_engine->playSoundEffect(0xF6);

	if (chest.evadesTrap(0))
		_G(messages).print("Evaded!!\n");
	else
		chest.damageAll(_G(savegame)._dungeonLevel);
}

void DungeonSpecial::gremlins() {
	Data::Savegame &save = _G(savegame);
	int slot = Graphics::Views::g_events->getRandomNumber(save._partySize - 1);
	Data::RosterEntry &e = save.partyMember(slot);

	if (e.isAlive()) {
		_G(messages).print("Gremlins!\n");

		int food = Data::fromBcdWord(e._food) - GREMLIN_FOOD;
		if (food >= 0) {
			e._food = Data::toBcdWord(food);
		} else {
			e._food = 0;
			_G(messages).print("Starving!\n");
			_G(effects).flashSlot(slot);
			g_engine->playSoundEffect(0xF7);
			ChestLogic().damageCharacter(slot, GREMLIN_DAMAGE);
		}
	}

	g_engine->playSoundEffect(0xFA);
}

bool DungeonSpecial::keypress(const KeypressMessage &msg) {
	if (_waiting) {
		if (isModifierKey(msg.keycode))
			return false;

		_G(messages).print("\n");
		return true;
	}

	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	if (_kind == Data::DTILE_FOUNTAIN) {
		drink(_chooser.slot());
		askWhoDrinks();
		return false;
	}

	touchRod(_chooser.slot());
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
