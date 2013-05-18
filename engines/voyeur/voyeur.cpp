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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/voyeur.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Voyeur {

VoyeurEngine *g_vm;

VoyeurEngine::VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Voyeur") {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
}

VoyeurEngine::~VoyeurEngine() {
}

Common::String VoyeurEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool VoyeurEngine::canLoadGameStateCurrently() {
	return true;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool VoyeurEngine::canSaveGameStateCurrently() {
	return true;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error VoyeurEngine::loadGameState(int slot) {
	return Common::kNoError;
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error VoyeurEngine::saveGameState(int slot, const Common::String &desc) {
	//TODO
	return Common::kNoError;
}

Common::Error VoyeurEngine::run() {
	

	return Common::kNoError;
}

int VoyeurEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

} // End of namespace Voyeur
