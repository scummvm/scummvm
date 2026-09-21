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
#include "ultima/ultima2/logic/space_logic.h"
#include "ultima/ultima2/views/space_map.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

namespace {

constexpr int DEEP_SPACE = 10;
constexpr int PLANET_X = 9;

// The ship's steering position, which is also where the stars radiate from
constexpr byte CENTER_X = 0x80, CENTER_Y = 0x40;

// Reflective armour or better is needed to survive the launch
constexpr int SAFE_ARMOR = Data::ARMOR_REFLECT;

} // namespace

void SpaceLogic::takeOff() {
	Data::Savegame &sg = _G(savegame);
	SpaceLogic *logic = new SpaceLogic();

	int world = MIN<int>(sg._orbitTarget, PLANET_X);
	for (int i = 0; i < 3; ++i)
		logic->_coords[i] = Data::PLANET_COORDS[world][i];

	--sg._items[Data::ITEM_TRI_LITHIUM];
	logic->_launching = true;

	_G(logic) = Common::SharedPtr<Logic>(logic);
	g_engine->replaceView("SpaceMap");
}

void SpaceLogic::hyperwarpTo(int xeno, int yako, int zabo) {
	_coords[0] = xeno;
	_coords[1] = yako;
	_coords[2] = zabo;

	static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"))->startSpinDown();
}

bool SpaceLogic::announceOrbit() {
	Data::Savegame &sg = _G(savegame);

	if (_coords[0] == 4 && _coords[1] == 4 && _coords[2] == 4) {
		writeString("\n\nYOU HIT THE SUN!\n");
		playerDied();
		return false;
	}

	for (int world = PLANET_X; world >= 0; --world) {
		if (_coords[0] == Data::PLANET_COORDS[world][0] && _coords[1] == Data::PLANET_COORDS[world][1] &&
				_coords[2] == Data::PLANET_COORDS[world][2]) {
			sg._orbitTarget = world;
			writeString("YOU ARE ORBITING %s.\n", world == PLANET_X ? "X" : Data::PLANET_NAMES[world]);
			return true;
		}
	}

	sg._orbitTarget = DEEP_SPACE;
	writeString("YOU ARE IN DEEP SPACE.\n");
	return true;
}

void SpaceLogic::warpFinished() {
	Data::Savegame &sg = _G(savegame);
	auto *view = static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"));

	// The ship strays when the fuel level's low digit is a 3 or 7
	if ((Data::toBcd(sg._items[Data::ITEM_TRI_LITHIUM] % 100) & 3) == 3) {
		writeString("SHIP OFF COURSE!\n");
		for (int i = 0; i < 3; ++i)
			_coords[i] = view->nextStarCoord() & 7;
	}

	if (!announceOrbit())
		return;

	if (_launching) {
		_launching = false;

		if (sg._readiedArmor < SAFE_ARMOR) {
			writeString("\nYOU HAVE EXPLODED!\n\n");
			playerDied();
			return;
		}
	}

	view->redraw();
	prompt();
}

void SpaceLogic::hyperwarp() {
	Data::Savegame &sg = _G(savegame);

	if (sg._items[Data::ITEM_TRI_LITHIUM] == 0) {
		writeString("NO FUEL!\n");
		prompt();
		return;
	}

	--sg._items[Data::ITEM_TRI_LITHIUM];
	static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"))->setShip(CENTER_X, CENTER_Y);

	writeString("HYPERWARP TO:\n");
	g_engine->addView("HyperwarpTarget");
}

void SpaceLogic::land() {
	Data::Savegame &sg = _G(savegame);
	auto *view = static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"));

	view->setShip(CENTER_X, CENTER_Y);
	writeString("LANDING REQUESTED!\n");
	if (!announceOrbit())
		return;

	// Earth is always reached in its present day, and the other worlds
	// come from their own files
	int world = sg._orbitTarget;
	int era = (world == 0) ? 4 : world;
	if (world == DEEP_SPACE || !Common::File::exists(Data::mapFilename(era, 0).c_str()) ||
			!Common::File::exists(Data::monsterFilename(era, 0).c_str())) {
		writeString("REQUEST DENIED!\n");
		prompt();
		return;
	}

	// Keep hold of this logic until the map load replaces it
	Common::SharedPtr<Logic> hold = _G(logic);

	sg._mapEra = era;
	sg._mapType = 0;
	sg._mapX = 0;
	sg._mapY = 0;
	_G(map).load(era, 0);
	g_engine->addView("RocketLanding");
}

void SpaceLogic::action(int action) {
	Common::SharedPtr<Logic> hold = _G(logic);
	auto *view = static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"));

	// Nothing can be commanded while a hyperwarp is under way
	if (view->busy())
		return;

	switch (action) {
	case KEYBIND_LEFT:
		writeString("LEFT\n");
		view->setShip(0x20, CENTER_Y);
		prompt();
		break;
	case KEYBIND_RIGHT:
		writeString("RIGHT\n");
		view->setShip(0xDF, CENTER_Y);
		prompt();
		break;
	case KEYBIND_UP:
		writeString("CLIMB\n");
		view->setShip(CENTER_X, 0x10);
		prompt();
		break;
	case KEYBIND_DOWN:
		writeString("DIVE\n");
		view->setShip(CENTER_X, 0x6F);
		prompt();
		break;
	case KEYBIND_HYPER:
		hyperwarp();
		break;
	case KEYBIND_LAUNCH:
		land();
		break;
	default:
		view->setShip(CENTER_X, CENTER_Y);
		break;
	}
}

void SpaceLogic::keypress(const Shared::Messages::KeypressMessage &msg) {
	static_cast<Views::SpaceMap *>(g_engine->findView("SpaceMap"))->setShip(CENTER_X, CENTER_Y);
}

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima
