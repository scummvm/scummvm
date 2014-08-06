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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "access/access.h"

namespace Access {

AccessEngine::AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("Access") {
	_debugger = nullptr;
	_events = nullptr;
	_files = nullptr;
	_player = nullptr;
	_room = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;

	_destIn = nullptr;
	clearCellTable();
	_pCount = 0;
	_selectCommand = 0;
	_normalMouse = true;
	_mouseMode = 0;
	_currentMan = 0;
	_currentManOld = -1;
	_man1 = nullptr;
	_inactive = nullptr;
	_manPal1 = nullptr;
	_music = nullptr;
	_playField = nullptr;
	_anim = nullptr;
	_title = nullptr;
	_converseMode = 0;
	_startInvItem = 0;
	_startAboutItem = 0;
	_startTravelItem = 0;
	_startInvBox = 0;
	_startAboutBox = 0;
	_startTravelBox = 0;
	_numImages = 0;
	_nextImage = 0;
	_numAnimTimers = 0;
	_startup = 0;
	_currentCharFlag = false;
	_boxSelect = false;

	_roomNumber = 0;
	_rawPlayerXLow = 0;
	_rawPlayerX = 0;
	_rawPlayerYLow = 0;
	_rawPlayerY = 0;
	_conversation = 0;
	_currentMan = 0;
	_newTime = 0;
	_newDate = 0;
	_intTim[3] = 0;
	_timer[3] = 0;
	_timerFlag = false;
	Common::fill(&_flags[0], &_flags[99], 0);
	Common::fill(&_useItem[0], &_useItem[23], 0);
	_guardLoc = 0;
	_guardFind = 0;
	_helpLevel = 0;
	_jasMayaFlag = 0;
	_moreHelp = 0;
	_startup = 0;
	_flashbackFlag = false;
	_manScaleOff = 0;
	_riverFlag = false;
	_antOutFlag = false;
	_badEnd = 0;
	_noHints = false;
	_antFlag = false;
	_allenFlag = false;
	_noSound = false;
	Common::fill(&inv[0], &inv[85], 0);
	Common::fill(&_help1[0], &_help1[366], 0);
	Common::fill(&_help2[0], &_help2[366], 0);
	Common::fill(&_help1[0], &_help3[366], 0);
	_travel = 0;
	_ask = 0;
	_rScrollRow = 0;
	_rScrollCol = 0;
	_rSrcollX = 0;
	_rScrollY = 0;
	_rOldRectCount = 0;
	_rNewRectCount = 0;
	_rKeyFlag = 0;
	_mapOffset = 0;
	_screenVirtX = 0;
}

AccessEngine::~AccessEngine() {
	delete _debugger;
	delete _events;
	delete _files;
	delete _player;
	delete _room;
	delete _screen;
	delete _scripts;
	delete _sound;

	_buffer1.free();
	_buffer2.free();
}

void AccessEngine::setVGA() {
	initGraphics(320, 200, false);
}

void AccessEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_files = new FileManager(this);
	_player = new Player(this);
	_screen = new Screen(this);
	_sound = new SoundManager(this, _mixer);

	_buffer1.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
	_buffer2.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
}

Common::Error AccessEngine::run() {
	setVGA();
	initialize();

	playGame();

	dummyLoop();
	return Common::kNoError;
}

void AccessEngine::dummyLoop() {
	// Dummy game loop
	while (!shouldQuit()) {
		_events->pollEvents();
		g_system->delayMillis(50);
		g_system->updateScreen();

		if (_events->_leftButton) {
			CursorType cursorId = _events->getCursor();
			_events->setCursor((cursorId == CURSOR_HELP) ? CURSOR_0 : (CursorType)(cursorId + 1));
		}
	}

}

int AccessEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void AccessEngine::clearCellTable() {
	Common::fill(&_objectsTable[0], &_objectsTable[100], (byte *)nullptr);
}

} // End of namespace Access
