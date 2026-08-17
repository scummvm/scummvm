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

#include "daad/daad.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "gui/message.h"

// Renaming layer first, then the ADP public API. Order matters: see the
// comment at the top of adp_prefix.h.
#include "daad/adp_prefix.h"
#include <ddb.h>
#include <ddb_vid.h>
#include <os_lib.h>

namespace DAAD {

DAADEngine *g_daad;

DAADEngine::DAADEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {
	g_daad = this;
}

DAADEngine::~DAADEngine() {
	g_daad = nullptr;
}

bool DAADEngine::hasFeature(EngineFeature f) const {
	// Saving and loading is done by the game's own SAVE/LOAD condacts, which
	// go through File_Create()/File_Open() and end up in the save file
	// manager (see daad_file.cpp), so the launcher's save UI stays out of it.
	return f == kSupportsReturnToLauncher;
}

const char *DAADEngine::getGameId() const {
	return _gameDescription != nullptr ? _gameDescription->gameId : "daad";
}

Common::Error DAADEngine::run() {
	OSInit();

	// Smallest common denominator; VID_Initialize() calls initGraphics()
	// again with the real dimensions once the database has said which machine
	// we are emulating (256x192, 320x200, 640x200, 640x400 or 720x256). Note
	// that it must go through initGraphics() rather than OSystem::initSize(),
	// which asserts unless it is called inside a graphics transaction.
	initGraphics(320, 200);

	syncSoundSettings();

	// Match the desktop player: honour an explicit mode override if one was
	// requested, otherwise pick the highest fidelity mode the data supports.
	DDB_SetStartupVideoModePolicy(DDB_StartupVideoModePolicy_OverrideOrHighest);
	DDB_ClearStartupScreenModeOverride();

	// DDB_RunPlayer() owns the whole session: it enumerates the game folder,
	// picks the database (and the part to start with), shows the loading
	// screen, and then drives DDB_Run() -> SCR_MainLoop() -> VID_MainLoop(),
	// which is where daad_vid.cpp pumps ScummVM events and presents frames.
	// It only returns when the game quits or fails to start.
	if (!DDB_RunPlayer()) {
		const char *msg = DDB_GetErrorString();
		if (!shouldQuit()) {
			GUI::MessageDialog dialog(Common::U32String::format(
				_("Could not start the DAAD game: %s"), msg ? msg : "unknown error"));
			dialog.runModal();
		}
		warning("DAAD: %s", msg ? msg : "unknown error");

		VID_Finish();
		return Common::kUnknownError;
	}

	VID_Finish();
	return Common::kNoError;
}

} // End of namespace DAAD
