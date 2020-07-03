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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/stage.h"
#include "director/lingo/lingo.h"

namespace Director {

const uint32 wmModeDesktop = Graphics::kWMModalMenuMode | Graphics::kWMModeManualDrawWidgets;
const uint32 wmModeFullscreen = Graphics::kWMModalMenuMode | Graphics::kWMModeNoDesktop
	| Graphics::kWMModeManualDrawWidgets | Graphics::kWMModeFullscreen;
uint32 wmMode = 0;

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugCompile, "compile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugParse, "parse", "Lingo code parsing");
	DebugMan.addDebugChannel(kDebugCompileOnly, "compileonly", "Skip Lingo code execution");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event processing");
	DebugMan.addDebugChannel(kDebugSlow, "slow", "Slow playback");
	DebugMan.addDebugChannel(kDebugFast, "fast", "Fast (no delay) playback");
	DebugMan.addDebugChannel(kDebugNoLoop, "noloop", "Do not loop the playback");
	DebugMan.addDebugChannel(kDebugBytecode, "bytecode", "Execute Lscr bytecode");
	DebugMan.addDebugChannel(kDebugFewFramesOnly, "fewframesonly", "Only run the first 10 frames");
	DebugMan.addDebugChannel(kDebugPreprocess, "preprocess", "Lingo preprocessing");
	DebugMan.addDebugChannel(kDebugScreenshot, "screenshot", "screenshot each frame");
	DebugMan.addDebugChannel(kDebugDesktop, "desktop", "Show the Classic Mac desktop");

	g_director = this;

	// Setup mixer
	syncSoundSettings();

	// Load Palettes
	loadPalettes();

	// Load Patterns
	loadPatterns();

	// Load key codes
	loadKeyCodes();

	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_lingo = nullptr;

	_wm = nullptr;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Meet Mediaband could have up to 5 levels of directories
	SearchMan.addDirectory(gameDataDir.getPath(), gameDataDir, 0, 5);

	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");
	SearchMan.addSubDirectoryMatching(gameDataDir, "main");		// Meet Mediaband

	_colorDepth = 8;	// 256-color
	_key = 0;
	_keyCode = 0;
	_machineType = 9; // Macintosh IIci
	_playbackPaused = false;
	_skipFrameAdvance = false;

	_draggingSprite = false;
	_draggingSpriteId = 0;
}

DirectorEngine::~DirectorEngine() {
	delete _soundManager;
	delete _lingo;
	delete _wm;
	delete _surface;
}

Archive *DirectorEngine::getMainArchive() const { return _currentStage->getMainArchive(); }
Movie *DirectorEngine::getCurrentMovie() const { return _currentStage->getCurrentMovie(); }
Common::String DirectorEngine::getCurrentPath() const { return _currentStage->getCurrentPath(); }

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	_currentPalette = nullptr;

	_soundManager = nullptr;

	wmMode = debugChannelSet(-1, kDebugDesktop) ? wmModeDesktop : wmModeFullscreen;
	_wm = new Graphics::MacWindowManager(wmMode, &_director3QuickDrawPatterns);
	_wm->setEngine(this);

	_currentStage = new Stage(_wm->getNextId(), false, false, false, _wm, this);

	if (!debugChannelSet(-1, kDebugDesktop))
		_currentStage->disableBorder();

	_surface = new Graphics::ManagedSurface;
	_wm->setScreen(_surface);
	_wm->addWindowInitialized(_currentStage);
	_wm->setActiveWindow(_currentStage->getId());

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound(this);

	if (getGameGID() == GID_TEST) {
		_currentStage->runTests();
		return Common::kNoError;
	} else if (getGameGID() == GID_TESTALL) {
		_currentStage->enqueueAllMovies();
	}

	if (getPlatform() == Common::kPlatformWindows)
		_machineType = 256; // IBM PC-type machine

	if (getVersion() < 4) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARDCST.MMM";
		} else {
			_sharedCastFile = "Shared Cast";
		}
	} else if (getVersion() == 5) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARED.Cxt";
		}
	} else {
		_sharedCastFile = "Shared.dir";
	}

	Common::Error err = _currentStage->loadInitialMovie();
	if (err.getCode() != Common::kNoError)
		return err;

	bool loop = true;

	while (loop) {
		loop = _currentStage->step();
	}

	return Common::kNoError;
}

} // End of namespace Director
