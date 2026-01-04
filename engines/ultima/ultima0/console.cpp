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

#include "ultima/ultima0/console.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

Console::Console() : GUI::Debugger() {
	registerCmd("view", WRAP_METHOD(Console, cmdView));
	registerCmd("food", WRAP_METHOD(Console, cmdFood));
	registerCmd("hp", WRAP_METHOD(Console, cmdHP));
	registerCmd("gold", WRAP_METHOD(Console, cmdGold));
	registerCmd("demo", WRAP_METHOD(Console, cmdDemo));
	registerCmd("debug", WRAP_METHOD(Console, cmdDebug));
}

Console::~Console() {
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	} else {
		g_events->replaceView(argv[1]);
		return false;
	}
}

bool Console::cmdFood(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("food <amount>\n");
		return true;
	} else {
		g_engine->_player._object[OB_FOOD] = atoi(argv[1]);
		g_engine->focusedView()->redraw();
		return false;
	}
}

bool Console::cmdGold(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("gold <amount>\n");
		return true;
	} else {
		g_engine->_player._attr[AT_GOLD] = atoi(argv[1]);
		g_engine->focusedView()->redraw();
		return false;
	}
}

bool Console::cmdHP(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("hp <amount>\n");
		return true;
	} else {
		g_engine->_player._attr[AT_HP] = atoi(argv[1]);
		g_engine->focusedView()->redraw();
		return false;
	}
}

bool Console::cmdDemo(int argc, const char **argv) {
	auto &p = g_engine->_player;
	auto &map = g_engine->_worldMap;

	p.init();
	Common::strcpy_s(p._name, "Demo");	// Characters Name
	p._class = 'F';						// Fighter
	p._luckyNumber = 42;					// Always the same.....
	p._skill = 1;						// Skill level 1
	p._task = 1;						// Starting first task

	// Nice high attributes
	Common::fill(p._attr, p._attr + MAX_ATTR, 15);
	p._attr[AT_HP] = 18;
	p._attr[AT_GOLD] = 99;

	for (int i = 0; i < MAX_OBJ; i++)			// Lots of nice objects
		p._object[i] = (i == OB_FOOD || i == OB_BOW) ? 999 : 4.0;

	p._level = 0;
	map.init(p);
	g_engine->replaceView("WorldMap");

	return false;
}


bool Console::cmdDebug(int argc, const char **argv) {
	auto &p = g_engine->_player;
	auto &map = g_engine->_worldMap;
	int i;

	p.init();
	Common::strcpy_s(p._name, "Debuggo");	// Characters Name
	p._class = 'F';							// Fighter
	p._luckyNumber = 42;					// Always the same.....
	p._skill = 1;							// Skill level 1
	p._task = 1;							// Starting first task

	for (i = 0; i < MAX_ATTR; i++)			// Nice high attributes
		p._attr[i] = 99;
	p._attr[AT_HP] = 999;
	p._attr[AT_GOLD] = 9999;
	for (i = 0; i < MAX_OBJ; i++)			// Lots of nice objects
		p._object[i] = (i == OB_FOOD || i == OB_BOW) ? 9999.9 : 99.0;

	p._level = 0;
	map.init(p);
	g_engine->replaceView("WorldMap");

	return false;
}

} // namespace Ultima0
} // namespace Ultima
