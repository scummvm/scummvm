/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/console.h"
#include "twine/twine.h"
#include "twine/gamestate.h"
#include "twine/scene.h"
#include "twine/text.h"

namespace TwinE {

TwinEConsole::TwinEConsole(TwinEEngine *engine) : _engine(engine), GUI::Debugger() {
	registerCmd("give_allitems", WRAP_METHOD(TwinEConsole, doGiveAllItems));
	registerCmd("change_scene", WRAP_METHOD(TwinEConsole, doChangeScene));
	registerCmd("list_menutext", WRAP_METHOD(TwinEConsole, doListMenuText));
	registerCmd("toggle_debug", WRAP_METHOD(TwinEConsole, doToggleDebug));
}

TwinEConsole::~TwinEConsole() {
}

bool TwinEConsole::doToggleDebug(int argc, const char **argv) {
	if (_engine->cfgfile.Debug) {
		debug("Disabling debug mode");
		_engine->cfgfile.Debug = false;
	} else {
		debug("Enabling debug mode");
		_engine->cfgfile.Debug = true;
	}
	return true;
}

bool TwinEConsole::doListMenuText(int argc, const char **argv) {
	for (int32 i = 0; i < 1000; ++i) {
		char buf[256];
		if (_engine->_text->getMenuText(i, buf, sizeof(buf))) {
			debug("%4i: %s", i, buf);
		}
	}
	return true;
}

bool TwinEConsole::doChangeScene(int argc, const char **argv) {
	if (argc <= 1) {
		warning("Expected to get a scene index as first parameter");
		return false;
	}
	byte newSceneIndex = atoi(argv[1]);
	if (newSceneIndex >= LBA1SceneId::SceneIdMax) {
		warning("Scene index out of bounds");
		return false;
	}
	_engine->_scene->needChangeScene = atoi(argv[1]);
	_engine->_scene->changeScene();
	return true;
}

bool TwinEConsole::doGiveAllItems(int argc, const char **argv) {
	for (int32 i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		_engine->_gameState->gameFlags[i] = 1;
		_engine->_gameState->inventoryFlags[i] = 1;
	}
	int amount = 10;
	if (argc > 1) {
		amount = atoi(argv[1]);
	}
	_engine->_gameState->inventoryNumKeys += amount;
	_engine->_gameState->inventoryNumKashes += amount;
	_engine->_gameState->inventoryNumLeafsBox += amount;
	_engine->_gameState->inventoryNumLeafs += amount;
	_engine->_gameState->inventoryMagicPoints += amount;
	_engine->_gameState->inventoryNumGas += amount;
	return true;
}

} // End of namespace TwinE
