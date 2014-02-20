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

#include "common/scummsys.h"

#include "zvision/zvision.h"

#include "zvision/core/console.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/cursors/cursor_manager.h"
#include "zvision/core/save_manager.h"
#include "zvision/strings/string_manager.h"
#include "zvision/archives/zfs_archive.h"
#include "zvision/detection.h"

#include "common/config-manager.h"
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
		  _workingWindow(gameDesc->gameId == GID_NEMESIS ? Common::Rect((WINDOW_WIDTH - ZNEM_WORKING_WINDOW_WIDTH) / 2, (WINDOW_HEIGHT - ZNEM_WORKING_WINDOW_HEIGHT) / 2, ((WINDOW_WIDTH - ZNEM_WORKING_WINDOW_WIDTH) / 2) + ZNEM_WORKING_WINDOW_WIDTH, ((WINDOW_HEIGHT - ZNEM_WORKING_WINDOW_HEIGHT) / 2) + ZNEM_WORKING_WINDOW_HEIGHT) :
		                                                   Common::Rect((WINDOW_WIDTH - ZGI_WORKING_WINDOW_WIDTH) / 2, (WINDOW_HEIGHT - ZGI_WORKING_WINDOW_HEIGHT) / 2, ((WINDOW_WIDTH - ZGI_WORKING_WINDOW_WIDTH) / 2) + ZGI_WORKING_WINDOW_WIDTH, ((WINDOW_HEIGHT - ZGI_WORKING_WINDOW_HEIGHT) / 2) + ZGI_WORKING_WINDOW_HEIGHT)),
		  _pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), /*RGB 565*/
		  _desiredFrameTime(33), /* ~30 fps */
		  _clock(_system),
		  _scriptManager(nullptr),
		  _renderManager(nullptr),
		  _saveManager(nullptr),
		  _stringManager(nullptr),
		  _cursorManager(nullptr) {

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
	_renderManager = new RenderManager(_system, WINDOW_WIDTH, WINDOW_HEIGHT, _workingWindow, _pixelFormat);
	_saveManager = new SaveManager(this);
	_stringManager = new StringManager(this);
	_cursorManager = new CursorManager(this, &_pixelFormat);

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

		processEvents();

		// Call _renderManager->update() first so the background renders
		// before anything that puzzles/controls will render
		_renderManager->update(deltaTime);
		_scriptManager->update(deltaTime);

		// Render the backBuffer to the screen
		_renderManager->renderBackbufferToScreen();

		// Update the screen
		_system->updateScreen();

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

} // End of namespace ZVision
