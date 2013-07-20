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
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/system.h"
#include "common/file.h"

#include "engines/util.h"
 
#include "zvision/zvision.h"
#include "zvision/console.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/zfs_archive.h"
#include "zvision/detection.h"

#include "zvision/utility.h"

namespace ZVision {
 
ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc)
		: Engine(syst),
		  _gameDescription(gameDesc),
		  _pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),	// RGB555
		  _currentVideo(0),
		  _scaledVideoFrameBuffer(0),
		  _width(640),
		  _height(480) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1", 0, 4);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2", 0, 4);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3", 0, 4);
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemmx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemscr");
 
	// Here is the right place to set up the engine specific debug channels
	//DebugMan.addDebugChannel(kZVisionDebugExample, "example", "this is just an example for a engine specific debug channel");
	//DebugMan.addDebugChannel(kZVisionDebugExample2, "example2", "also an example");
 
	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	_scriptManager = new ScriptManager(this);
	_renderManager = new RenderManager(_system, _width, _height);

	debug("ZVision::ZVision");
}
 
ZVision::~ZVision() {
	debug("ZVision::~ZVision");
 
	// Dispose of resources
	delete _console;
	delete _scriptManager;
	delete _rnd;
 
	// Remove all of our debug levels
	DebugMan.clearAllDebugChannels();
}

void ZVision::initialize() {
	// Find zfs archive files
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.zfs");

	// Register the file entries within the zfs archives with the SearchMan
	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String name = (*iter)->getName();
		ZfsArchive *archive = new ZfsArchive(name, (*iter)->createReadStream());

		SearchMan.add(name, archive);
	}

	initGraphics(_width, _height, true, &_pixelFormat);

	_scriptManager->initialize();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
}

Common::Error ZVision::run() {
	initialize();

	// Main loop
	uint32 currentTime = _system->getMillis();
	uint32 lastTime = currentTime;
	const uint32 desiredFrameTime = 33; // ~30 fps

	while (!shouldQuit()) {
		processEvents();
		
		currentTime = _system->getMillis();
		uint32 deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		if (_currentVideo != 0)
			continueVideo();
		else {
			_scriptManager->updateNodes(deltaTime);
			_scriptManager->checkPuzzleCriteria();
		}

		if (_needsScreenUpdate || _console->isActive()) {
			_system->updateScreen();
			_needsScreenUpdate = false;
		}
		
		// Calculate the frame delay based off a desired frame rate
		int delay = desiredFrameTime - (currentTime - _system->getMillis());
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}

	return Common::kNoError;
}

ScriptManager *ZVision::getScriptManager() const {
	return _scriptManager;
}

RenderManager *ZVision::getRenderManager() const {
	return _renderManager;
}

Common::RandomSource *ZVision::getRandomSource() const {
	return _rnd;
}

ZVisionGameId ZVision::getGameId() const {
	return _gameDescription->gameId;
}

} // End of namespace ZVision
