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

#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "gui/message.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/tilepuzzle.h"
#include "lab/utils.h"

namespace Lab {

LabEngine *g_lab;

LabEngine::LabEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc), _extraGameFeatures(0), _rnd("lab") {
	g_lab = this;

	_lastWaitTOFTicks = 0;

	_isHiRes = false;
	_roomNum = -1;
	for (int i = 0; i < MAX_CRUMBS; i++) {
		_breadCrumbs[i]._roomNum = 0;
		_breadCrumbs[i]._direction = NORTH;
	}

	_numCrumbs = 0;
	_droppingCrumbs = false;
	_followingCrumbs = false;
	_followCrumbsFast = false;
	_isCrumbTurning = false;
	_isCrumbWaiting = false;
	_noUpdateDiff = false;
	_quitLab = false;
	_mainDisplay = true;

	_numInv = 0;
	_manyRooms = 0;
	_direction = 0;
	_highestCondition = 0;
	_crumbTimestamp = 0;
	_maxRooms = 0;

	_event = nullptr;
	_resource = nullptr;
	_music = nullptr;
	_anim = nullptr;
	_graphics = nullptr;
	_rooms = nullptr;
	_tilePuzzle = nullptr;
	_utils = nullptr;

	_lastTooLong = false;
	_interfaceOff = false;
	_alternate = false;

	for (int i = 0; i < 20; i++)
		_moveImages[i] = nullptr;

	for (int i = 0; i < 10; i++) {
		_invImages[i] = nullptr;
	}

	_curFileName = nullptr;
	_nextFileName = nullptr;
	_newFileName = nullptr;

	_curFileName = " ";
	_msgFont = nullptr;
	_inventory = nullptr;

	_imgMap = nullptr;
	_imgRoom = nullptr;
	_imgUpArrowRoom = nullptr;
	_imgDownArrowRoom = nullptr;
	_imgBridge = nullptr;
	_imgHRoom = nullptr;
	_imgVRoom = nullptr;
	_imgMaze = nullptr;
	_imgHugeMaze = nullptr;
	_imgPath = nullptr;
	_imgMapNorth = nullptr;
	_imgMapEast = nullptr;
	_imgMapSouth = nullptr;
	_imgMapWest = nullptr;
	_imgXMark = nullptr;
	_maps = nullptr;

	_blankJournal = nullptr;
	_journalFont = nullptr;
	_journalText = nullptr;
	_journalTextTitle = nullptr;
	_journalPage = 0;
	_lastPage = false;
	_monitorPage = 0;
	_monitorTextFilename = "";
	_monitorButton = nullptr;
	_monitorButtonHeight = 1;
	for (int i = 0; i < 20; i++)
		_highPalette[i] = 0;

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
	delete _anim;
	delete _graphics;
	delete[] _rooms;
	delete _tilePuzzle;
	delete _utils;
	delete _imgMap;
	delete _imgRoom;
	delete _imgUpArrowRoom;
	delete _imgDownArrowRoom;
	delete _imgBridge;
	delete _imgHRoom;
	delete _imgVRoom;
	delete _imgMaze;
	delete _imgHugeMaze;
	delete _imgPath;
	delete _imgMapNorth;
	delete _imgMapEast;
	delete _imgMapSouth;
	delete _imgMapWest;
	delete _imgXMark;
	delete _maps;
}

Common::Error LabEngine::run() {
	if (getFeatures() & GF_LOWRES)
		initGraphics(320, 200, false);
	else
		initGraphics(640, 480, true);

	_event = new EventManager(this);
	_resource = new Resource(this);
	_music = new Music(this);
	_graphics = new DisplayMan(this);
	_anim = new Anim(this);
	_tilePuzzle = new TilePuzzle(this);
	_utils = new Utils(this);

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

void LabEngine::drawStaticMessage(byte index) {
	_graphics->drawMessage(_resource->getStaticText((StaticText)index).c_str());
}

void LabEngine::changeVolume(int delta) {
	warning("STUB: changeVolume()");
}

void LabEngine::waitTOF() {
	g_system->copyRectToScreen(_graphics->_displayBuffer, _graphics->_screenWidth, 0, 0, _graphics->_screenWidth, _graphics->_screenHeight);
	g_system->updateScreen();

	_event->processInput();

	uint32 now;

	for (now = g_system->getMillis(); now - _lastWaitTOFTicks <= 0xF; now = g_system->getMillis() )
		g_system->delayMillis(_lastWaitTOFTicks - now + 17);

	_lastWaitTOFTicks = now;
}
} // End of namespace Lab
