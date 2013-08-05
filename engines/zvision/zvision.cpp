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

#include "audio/mixer.h"
 
#include "zvision/zvision.h"
#include "zvision/console.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/clock.h"
#include "zvision/zfs_archive.h"
#include "zvision/detection.h"

#include "zvision/utility.h"

namespace ZVision {
 
ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc)
		: Engine(syst),
		  _gameDescription(gameDesc),
		  _width(640),
		  _height(480),
		  _pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0), /*RGB 555*/
		  _desiredFrameTime(33), /* ~30 fps */
		  _clock(_system) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	// TODO: There are 10 file clashes when we flatten the directories. From a quick look, the files are exactly the same, so it shouldn't matter. But I'm noting it here just in-case it does become a problem.
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets1", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets2", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemmx", 0, 1, true);
 
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
	delete _renderManager;
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
	while (!shouldQuit()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();
		
		processEvents();

		_scriptManager->updateNodes(_clock.getDeltaTime());
		_scriptManager->checkPuzzleCriteria();

		// Render a frame
		_renderManager->updateScreen(_console->isActive());
		
		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - (currentTime - _system->getMillis());
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
