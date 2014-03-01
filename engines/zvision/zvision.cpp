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

#include "zvision/zvision.h"

#include "zvision/console.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/cursor_manager.h"
#include "zvision/save_manager.h"
#include "zvision/string_manager.h"
#include "zvision/zfs_archive.h"
#include "zvision/detection.h"
#include "zvision/menu.h"
#include "zvision/search_manager.h"
#include "zvision/text.h"
#include "zvision/truetype_font.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/system.h"
#include "common/file.h"

#include "engines/util.h"

#include "audio/mixer.h"


namespace ZVision {

ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _workingWindow_ZGI((WINDOW_WIDTH - WORKING_WINDOW_WIDTH) / 2, (WINDOW_HEIGHT - WORKING_WINDOW_HEIGHT) / 2, ((WINDOW_WIDTH - WORKING_WINDOW_WIDTH) / 2) + WORKING_WINDOW_WIDTH, ((WINDOW_HEIGHT - WORKING_WINDOW_HEIGHT) / 2) + WORKING_WINDOW_HEIGHT),
	  _workingWindow_ZNM((WINDOW_WIDTH - ZNM_WORKING_WINDOW_WIDTH) / 2, (WINDOW_HEIGHT - ZNM_WORKING_WINDOW_HEIGHT) / 2, ((WINDOW_WIDTH - ZNM_WORKING_WINDOW_WIDTH) / 2) + ZNM_WORKING_WINDOW_WIDTH, ((WINDOW_HEIGHT - ZNM_WORKING_WINDOW_HEIGHT) / 2) + ZNM_WORKING_WINDOW_HEIGHT),
	  _workingWindow(gameDesc->gameId == GID_NEMESIS ? _workingWindow_ZNM : _workingWindow_ZGI),
	  _pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), /*RGB 565*/
	  _desiredFrameTime(33), /* ~30 fps */
	  _clock(_system),
	  _scriptManager(nullptr),
	  _renderManager(nullptr),
	  _saveManager(nullptr),
	  _stringManager(nullptr),
	  _cursorManager(nullptr),
	  _aud_id(0),
	  _rendDelay(2),
	  _velocity(0) {

	debug(1, "ZVision::ZVision");
}

ZVision::~ZVision() {
	debug(1, "ZVision::~ZVision");

	// Dispose of resources
	delete _console;
	delete _cursorManager;
	delete _stringManager;
	delete _saveManager;
	delete _renderManager;
	delete _scriptManager;
	delete _rnd;

	// Remove all of our debug levels
	DebugMan.clearAllDebugChannels();
}

void ZVision::initialize() {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	_searchManager = new sManager(ConfMan.get("path"), 6);

	_searchManager->addDir("FONTS");
	_searchManager->addDir("addon");

	if (_gameDescription->gameId == GID_GRANDINQUISITOR) {
		_searchManager->loadZix("INQUIS.ZIX");
		_searchManager->addPatch("C000H01Q.RAW", "C000H01Q.SRC");
		_searchManager->addPatch("CM00H01Q.RAW", "CM00H01Q.SRC");
		_searchManager->addPatch("DM00H01Q.RAW", "DM00H01Q.SRC");
		_searchManager->addPatch("E000H01Q.RAW", "E000H01Q.SRC");
		_searchManager->addPatch("EM00H50Q.RAW", "EM00H50Q.SRC");
		_searchManager->addPatch("GJNPH65P.RAW", "GJNPH65P.SRC");
		_searchManager->addPatch("GJNPH72P.RAW", "GJNPH72P.SRC");
		_searchManager->addPatch("H000H01Q.RAW", "H000H01Q.SRC");
		_searchManager->addPatch("M000H01Q.RAW", "M000H01Q.SRC");
		_searchManager->addPatch("P000H01Q.RAW", "P000H01Q.SRC");
		_searchManager->addPatch("Q000H01Q.RAW", "Q000H01Q.SRC");
		_searchManager->addPatch("SW00H01Q.RAW", "SW00H01Q.SRC");
		_searchManager->addPatch("T000H01Q.RAW", "T000H01Q.SRC");
		_searchManager->addPatch("U000H01Q.RAW", "U000H01Q.SRC");
	} else if (_gameDescription->gameId == GID_NEMESIS)
		_searchManager->loadZix("NEMESIS.ZIX");
	// TODO: There are 10 file clashes when we flatten the directories.
	// From a quick look, the files are exactly the same, so it shouldn't matter.
	// But I'm noting it here just in-case it does become a problem.
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets1", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets2", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemmx", 0, 1, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zgi", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zgi_mx", 0, 1, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "fonts", 0, 1, true);

	// Find zfs archive files
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.zfs");

	// Register the file entries within the zfs archives with the SearchMan
	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String name = (*iter)->getName();
		Common::SeekableReadStream *stream = (*iter)->createReadStream();
		ZfsArchive *archive = new ZfsArchive(name, stream);

		delete stream;

		SearchMan.add(name, archive);
	}

	initGraphics(WINDOW_WIDTH, WINDOW_HEIGHT, true, &_pixelFormat);

	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	_scriptManager = new ScriptManager(this);
	_renderManager = new RenderManager(this, WINDOW_WIDTH, WINDOW_HEIGHT, _workingWindow, _pixelFormat);
	_saveManager = new SaveManager(this);
	_stringManager = new StringManager(this);
	_cursorManager = new CursorManager(this, &_pixelFormat);
	_textRenderer = new textRenderer(this);

	if (_gameDescription->gameId == GID_GRANDINQUISITOR)
		_menu = new menuZgi(this);
	else
		_menu = new menuNem(this);

	// Initialize the managers
	_cursorManager->initialize();
	_scriptManager->initialize();
	_stringManager->initialize(_gameDescription->gameId);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
}

Common::Error ZVision::run() {
	initialize();

	// Main loop
	while (!shouldQuit()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();
		uint32 deltaTime = _clock.getDeltaTime();

		_cursorManager->setItemID(_scriptManager->getStateValue(StateKey_InventoryItem));

		processEvents();
		updateRotation();

		// Call _renderManager->update() first so the background renders
		// before anything that puzzles/controls will render
		_scriptManager->update(deltaTime);
		_menu->process(deltaTime);

		// Render the backBuffer to the screen
		_renderManager->prepareBkg();
		_renderManager->renderMenuToScreen();
		_renderManager->processSubs(deltaTime);
		_renderManager->renderBackbufferToScreen();

		// Update the screen
		if (_rendDelay <= 0)
			_system->updateScreen();
		else
			_rendDelay--;

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}

	return Common::kNoError;
}

void ZVision::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);

	if (pause) {
		_clock.stop();
	} else {
		_clock.start();
	}
}

Common::String ZVision::generateSaveFileName(uint slot) {
	return Common::String::format("%s.%02u", _targetName.c_str(), slot);
}

Common::String ZVision::generateAutoSaveFileName() {
	return Common::String::format("%s.auto", _targetName.c_str());
}

void ZVision::setRenderDelay(uint delay) {
	_rendDelay = delay;
}

bool ZVision::canRender() {
	return _rendDelay <= 0;
}

void ZVision::updateRotation() {
	if (_velocity) {
		RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
		if (renderState == RenderTable::PANORAMA) {
			int16 st_pos = _scriptManager->getStateValue(StateKey_ViewPos);

			int16 new_pos = st_pos + _velocity * (1 - 2 * 0);

			int16 zero_point = 0;
			if (st_pos >= zero_point && new_pos < zero_point)
				_scriptManager->setStateValue(StateKey_Rounds, _scriptManager->getStateValue(StateKey_Rounds) - 1);
			if (st_pos <= zero_point && new_pos > zero_point)
				_scriptManager->setStateValue(StateKey_Rounds, _scriptManager->getStateValue(StateKey_Rounds) + 1);

			int16 scr_width = _renderManager->getBkgSize().x;
			if (scr_width)
				new_pos %= scr_width;

			if (new_pos < 0)
				new_pos += scr_width;

			_scriptManager->setStateValue(StateKey_ViewPos, new_pos);
			_renderManager->setBackgroundPosition(new_pos);
		} else if (renderState == RenderTable::TILT) {
			int16 st_pos = _scriptManager->getStateValue(StateKey_ViewPos);

			int16 new_pos = st_pos + _velocity * (1 - 2 * 0);

			int16 scr_height = _renderManager->getBkgSize().y;
			int16 tilt_gap = _workingWindow.height() / 2;

			if (new_pos >= (scr_height - tilt_gap))
				new_pos = scr_height - tilt_gap;
			if (new_pos <= tilt_gap)
				new_pos = tilt_gap;

			_scriptManager->setStateValue(StateKey_ViewPos, new_pos);
			_renderManager->setBackgroundPosition(new_pos);
		}
	}
}

} // End of namespace ZVision
