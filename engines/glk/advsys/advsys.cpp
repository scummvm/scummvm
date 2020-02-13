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

#include "glk/advsys/advsys.h"
#include "common/translation.h"
#include "common/config-manager.h"

namespace Glk {
namespace AdvSys {

void AdvSys::runGame() {
	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	if (!initialize()) {
		GUIErrorMessage(_("Could not start AdvSys game"));
		return;
	}

	// Outer play loop - this loop re-iterates if a game is restarted
	while (!shouldQuit()) {
		// Run game startup
		execute(_initCodeOffset);

		if (_saveSlot != -1) {
			Common::ErrorCode err = loadGameState(_saveSlot).getCode();
			_saveSlot = -1;
			if (err != Common::kNoError)
				print(_("Sorry, the savegame couldn't be restored"));
			else
				_pendingLine = "look";		// Do a look action after loading the savegame
		}

		// Gameplay loop
		while (!shouldQuit() && !shouldRestart()) {
			// Run update code
			execute(_updateCodeOffset);

			// Get and parse a single line
			if (getInput()) {
				if (singleAction()) {
					while (!shouldQuit() && nextCommand() && singleAction()) {}
				}
			}
		}
	}

	deinitialize();
}

bool AdvSys::initialize() {
	// Create a Glk window for the game
	if (!GlkInterface::initialize())
		return false;

	// Load the game's header
	if (!Game::init(&_gameFile))
		return false;

	return true;
}

void AdvSys::deinitialize() {
}

bool AdvSys::singleAction() {
	// Do the before code
	switch (execute(_beforeOffset)) {
	case ABORT:
		// Script aborted
		return false;
	case CHAIN:
		// Execute the action handler
		if (execute(getActionField(getVariable(V_ACTION), A_CODE)) == ABORT)
			return false;

		// fall through
	case FINISH:
		// Do the after code
		if (execute(_afterOffset) == ABORT)
			return false;
		break;

	default:
		break;
	}

	return true;
}

Common::Error AdvSys::readSaveData(Common::SeekableReadStream *rs) {
	if (rs->size() != (int)_saveSize)
		return Common::kReadingFailed;

	rs->read(_saveArea, rs->size());
	return Common::kNoError;
}

Common::Error AdvSys::writeGameData(Common::WriteStream *ws) {
	ws->write(_saveArea, _saveSize);
	return Common::kNoError;
}

} // End of namespace AdvSys
} // End of namespace Glk
