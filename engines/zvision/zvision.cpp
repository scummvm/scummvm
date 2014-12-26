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
#include "zvision/core/console.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/file/save_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/detection.h"
#include "zvision/scripting/menu.h"
#include "zvision/file/search_manager.h"
#include "zvision/text/text.h"
#include "zvision/text/truetype_font.h"
#include "zvision/sound/midi.h"
#include "zvision/file/zfs_archive.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/system.h"
#include "common/file.h"

#include "gui/message.h"
#include "engines/util.h"
#include "audio/mixer.h"

namespace ZVision {

#define ZVISION_SETTINGS_KEYS_COUNT 17

struct zvisionIniSettings {
	const char *name;
	int16 slot;
	int16 deflt;
} settingsKeys[ZVISION_SETTINGS_KEYS_COUNT] = {
	{"ZVision_KeyboardTurnSpeed", StateKey_KbdRotateSpeed, 5},
	{"ZVision_PanaRotateSpeed", StateKey_RotateSpeed, 540},
	{"ZVision_QSoundEnabled", StateKey_Qsound, 1},
	{"ZVision_VenusEnabled", StateKey_VenusEnable, 1},
	{"ZVision_HighQuality", StateKey_HighQuality, 1},
	{"ZVision_Platform", StateKey_Platform, 0},
	{"ZVision_InstallLevel", StateKey_InstallLevel, 0},
	{"ZVision_CountryCode", StateKey_CountryCode, 0},
	{"ZVision_CPU", StateKey_CPU, 1},
	{"ZVision_MovieCursor", StateKey_MovieCursor, 1},
	{"ZVision_NoAnimWhileTurning", StateKey_NoTurnAnim, 0},
	{"ZVision_Win958", StateKey_WIN958, 0},
	{"ZVision_ShowErrorDialogs", StateKey_ShowErrorDlg, 0},
	{"ZVision_ShowSubtitles", StateKey_Subtitles, 1},
	{"ZVision_DebugCheats", StateKey_DebugCheats, 0},
	{"ZVision_JapaneseFonts", StateKey_JapanFonts, 0},
	{"ZVision_Brightness", StateKey_Brightness, 0}
};

ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _resourcePixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0), /* RGB 555 */
	  _screenPixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), /* RGB 565 */
	  _desiredFrameTime(33), /* ~30 fps */
	  _clock(_system),
	  _scriptManager(nullptr),
	  _renderManager(nullptr),
	  _saveManager(nullptr),
	  _stringManager(nullptr),
	  _cursorManager(nullptr),
	  _midiManager(nullptr),
	  _rnd(nullptr),
	  _console(nullptr),
	  _menu(nullptr),
	  _searchManager(nullptr),
	  _textRenderer(nullptr),
	  _doubleFPS(false),
	  _audioId(0),
	  _frameRenderDelay(2),
	  _keyboardVelocity(0),
	  _mouseVelocity(0),
	  _videoIsPlaying(false),
	  _renderedFrameCount(0),
	  _fps(0) {

	debug(1, "ZVision::ZVision");

	uint16 workingWindowWidth  = (gameDesc->gameId == GID_NEMESIS) ? ZNM_WORKING_WINDOW_WIDTH  : ZGI_WORKING_WINDOW_WIDTH;
	uint16 workingWindowHeight = (gameDesc->gameId == GID_NEMESIS) ? ZNM_WORKING_WINDOW_HEIGHT : ZGI_WORKING_WINDOW_HEIGHT;
	_workingWindow = Common::Rect(
						 (WINDOW_WIDTH  -  workingWindowWidth) / 2,
						 (WINDOW_HEIGHT - workingWindowHeight) / 2,
						((WINDOW_WIDTH  -  workingWindowWidth) / 2) + workingWindowWidth,
						((WINDOW_HEIGHT - workingWindowHeight) / 2) + workingWindowHeight
					 );

	memset(_cheatBuffer, 0, sizeof(_cheatBuffer));
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
	delete _midiManager;

	getTimerManager()->removeTimerProc(&fpsTimerCallback);

	// Remove all of our debug levels
	DebugMan.clearAllDebugChannels();
}

void ZVision::registerDefaultSettings() {
	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++)
		ConfMan.registerDefault(settingsKeys[i].name, settingsKeys[i].deflt);
	ConfMan.registerDefault("doublefps", false);
}

void ZVision::loadSettings() {
	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++)
		_scriptManager->setStateValue(settingsKeys[i].slot, ConfMan.getInt(settingsKeys[i].name));

	if (getGameId() == GID_NEMESIS)
		_scriptManager->setStateValue(StateKey_ExecScopeStyle, 1);
	else
		_scriptManager->setStateValue(StateKey_ExecScopeStyle, 0);
}

void ZVision::saveSettings() {
	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++)
		ConfMan.setInt(settingsKeys[i].name, _scriptManager->getStateValue(settingsKeys[i].slot));
	ConfMan.flushToDisk();
}

void ZVision::initialize() {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	_searchManager = new SearchManager(ConfMan.get("path"), 6);

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

	initGraphics(WINDOW_WIDTH, WINDOW_HEIGHT, true, &_screenPixelFormat);

	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	_scriptManager = new ScriptManager(this);
	_renderManager = new RenderManager(this, WINDOW_WIDTH, WINDOW_HEIGHT, _workingWindow, _resourcePixelFormat, _doubleFPS);
	_saveManager = new SaveManager(this);
	_stringManager = new StringManager(this);
	_cursorManager = new CursorManager(this, _resourcePixelFormat);
	_textRenderer = new TextRenderer(this);
	_midiManager = new MidiManager();

	if (_gameDescription->gameId == GID_GRANDINQUISITOR)
		_menu = new MenuZGI(this);
	else
		_menu = new MenuNemesis(this);

	// Initialize the managers
	_cursorManager->initialize();
	_scriptManager->initialize();
	_stringManager->initialize(_gameDescription->gameId);

	registerDefaultSettings();

	loadSettings();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
	_doubleFPS = ConfMan.getBool("doublefps");

	// Initialize FPS timer callback
	getTimerManager()->installTimerProc(&fpsTimerCallback, 1000000, this, "zvisionFPS");
}

Common::Error ZVision::run() {
	initialize();

	// Check if a saved game is to be loaded from the launcher
	if (ConfMan.hasKey("save_slot"))
		_saveManager->loadGame(ConfMan.getInt("save_slot"));

	// Before starting, make absolutely sure that the user has copied the needed fonts
	if (!Common::File::exists("arial.ttf") && !Common::File::exists("FreeSans.ttf")) {
		GUI::MessageDialog dialog(
				"Before playing this game, you'll need to copy the required "
				"fonts in ScummVM's extras directory, or the game directory. "
				"On Windows, you'll need the following font files from the Windows "
				"font directory: Times New Roman, Century Schoolbook, Garamond, "
				"Courier New and Arial. Alternatively, you can download the GNU "
				"FreeFont package. You'll need all the fonts from that package, "
				"i.e. FreeMono, FreeSans and FreeSerif."
		);
		dialog.runModal();
		quitGame();
		return Common::kUnknownError;
	}

	// Main loop
	while (!shouldQuit()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();
		uint32 deltaTime = _clock.getDeltaTime();

		_cursorManager->setItemID(_scriptManager->getStateValue(StateKey_InventoryItem));

		processEvents();
		_renderManager->updateRotation();

		_scriptManager->update(deltaTime);
		_menu->process(deltaTime);

		// Render the backBuffer to the screen
		_renderManager->prepareBackground();
		_renderManager->renderMenuToScreen();
		_renderManager->processSubs(deltaTime);
		_renderManager->renderSceneToScreen();

		// Update the screen
		if (canRender()) {
			_system->updateScreen();
			_renderedFrameCount++;
		} else {
			_frameRenderDelay--;
		}

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;

		if (_doubleFPS) {
			delay >>= 1;
		}

		if (canSaveGameStateCurrently() && shouldPerformAutoSave(_saveManager->getLastSaveTime())) {
			_saveManager->autoSave();
		}

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
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

void ZVision::setRenderDelay(uint delay) {
	_frameRenderDelay = delay;
}

bool ZVision::canRender() {
	return _frameRenderDelay <= 0;
}

void ZVision::fpsTimerCallback(void *refCon) {
	((ZVision *)refCon)->fpsTimer();
}

void ZVision::fpsTimer() {
	_fps = _renderedFrameCount;
	_renderedFrameCount = 0;
}

} // End of namespace ZVision
