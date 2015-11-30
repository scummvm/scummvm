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

 /*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/rect.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/util.h"

#include "gui/message.h"

#include "lab/lab.h"
#include "lab/labfun.h"
#include "lab/resource.h"

#include "engines/advancedDetector.h"

namespace Lab {

LabEngine *g_lab;

LabEngine::LabEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc), _extraGameFeatures(0) {
	g_lab = this;

    _screenWidth = 320;
	_screenHeight = 200;
	_screenBytesPerPage = 65536;

    _curapen = 0;

    _currentDsplayBuffer = 0;
    _displayBuffer = 0;

    _lastWaitTOFTicks = 0;

	_mousePos = Common::Point(0, 0);
	_mouseAtEdge = false;

    _nextKeyIn = 0;
    _nextKeyOut = 0;

	_isHiRes = false;

	_event = nullptr;
	_resource = nullptr;
	_music = nullptr;

	//const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game/pict");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "game/spict");
	//SearchMan.addSubDirectoryMatching(gameDataDir, "music");
}

LabEngine::~LabEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _event;
	delete _resource;
	delete _music;
}

Common::Error LabEngine::run() {
	if (getFeatures() & GF_LOWRES)
		initGraphics(320, 200, false);
	else
		initGraphics(640, 480, true);

	_event = new EventManager(this);
	_resource = new Resource(this);
	_music = new Music(this);

	if (getPlatform() == Common::kPlatformWindows) {
		// Check if this is the Wyrmkeep trial
		Common::File roomFile;
		bool knownVersion = true;
		bool roomFileOpened = roomFile.open("game/rooms/48");

		if (!roomFileOpened)
			knownVersion = false;
		else if (roomFile.size() != 892)
			knownVersion = false;
		else {
			roomFile.seek(352);
			byte checkByte = roomFile.readByte();
			if (checkByte == 0x00) {
				// Full Windows version
			} else if (checkByte == 0x80) {
				// Wyrmkeep trial version
				_extraGameFeatures = GF_WINDOWS_TRIAL;

				GUI::MessageDialog trialMessage("This is a trial Windows version of the game. To play the full version, you will need to use the original interpreter and purchase a key from Wyrmkeep");
				trialMessage.runModal();
			} else {
				knownVersion = false;
			}

			roomFile.close();

			if (!knownVersion) {
				warning("Unknown Windows version found, please report this version to the ScummVM team");
				return Common::kNoGameDataFoundError;
			}
		}
	}

	go();

	return Common::kNoError;
}

Common::String LabEngine::generateSaveFileName(uint slot) {
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

/*void LabEngine::showMainMenu() {

}*/

} // End of namespace Lab
