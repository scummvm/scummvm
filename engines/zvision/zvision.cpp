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
#include "zvision/core/save_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/detection.h"
#include "zvision/core/menu.h"
#include "zvision/core/search_manager.h"
#include "zvision/text/text.h"
#include "zvision/graphics/truetype_font.h"
#include "zvision/core/midi.h"
#include "zvision/utility/zfs_archive.h"

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
	  _pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), /*RGB 565*/
	  _desiredFrameTime(33), /* ~30 fps */
	  _clock(_system),
	  _scriptManager(nullptr),
	  _renderManager(nullptr),
	  _saveManager(nullptr),
	  _stringManager(nullptr),
	  _cursorManager(nullptr),
	  _midiManager(nullptr),
	  _audioId(0),
	  _rendDelay(2),
	  _kbdVelocity(0),
	  _mouseVelocity(0),
	  _videoIsPlaying(false) {

	debug(1, "ZVision::ZVision");

	uint16 workingWindowWidth  = (gameDesc->gameId == GID_NEMESIS) ? ZNM_WORKING_WINDOW_WIDTH  : ZGI_WORKING_WINDOW_WIDTH;
	uint16 workingWindowHeight = (gameDesc->gameId == GID_NEMESIS) ? ZNM_WORKING_WINDOW_HEIGHT : ZGI_WORKING_WINDOW_HEIGHT;
	_workingWindow = Common::Rect(
						 (WINDOW_WIDTH  -  workingWindowWidth) / 2,
						 (WINDOW_HEIGHT - workingWindowHeight) / 2,
						((WINDOW_WIDTH  -  workingWindowWidth) / 2) + workingWindowWidth,
						((WINDOW_HEIGHT - workingWindowHeight) / 2) + workingWindowHeight
					 );

	memset(_cheatBuff, 0, sizeof(_cheatBuff));
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

	initGraphics(WINDOW_WIDTH, WINDOW_HEIGHT, true, &_pixelFormat);

	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	_scriptManager = new ScriptManager(this);
	_renderManager = new RenderManager(this, WINDOW_WIDTH, WINDOW_HEIGHT, _workingWindow, _pixelFormat);
	_saveManager = new SaveManager(this);
	_stringManager = new StringManager(this);
	_cursorManager = new CursorManager(this, &_pixelFormat);
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
	_halveDelay = ConfMan.getBool("doublefps");
}

Common::Error ZVision::run() {
	initialize();

	// Check if a saved game is to be loaded from the launcher
	if (ConfMan.hasKey("save_slot"))
		_saveManager->loadGame(ConfMan.getInt("save_slot"));

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
		if (_halveDelay)
			delay >>= 1;
		_system->delayMillis(delay);
	}

	return Common::kNoError;
}

bool ZVision::askQuestion(const Common::String &str) {
	uint16 msgid = _renderManager->createSubArea();
	_renderManager->updateSubArea(msgid, str);
	_renderManager->processSubs(0);
	_renderManager->renderBackbufferToScreen();
	_clock.stop();

	int result = 0;

	while (result == 0) {
		Common::Event evnt;
		while (_eventMan->pollEvent(evnt)) {
			if (evnt.type == Common::EVENT_KEYDOWN) {
				switch (evnt.kbd.keycode) {
				case Common::KEYCODE_y:
					result = 2;
					break;
				case Common::KEYCODE_n:
					result = 1;
					break;
				default:
					break;
				}
			}
		}
		_system->updateScreen();
		if (_halveDelay)
			_system->delayMillis(33);
		else
			_system->delayMillis(66);
	}
	_renderManager->deleteSubArea(msgid);
	_clock.start();
	return result == 2;
}

void ZVision::delayedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = _renderManager->createSubArea();
	_renderManager->updateSubArea(msgid, str);
	_renderManager->processSubs(0);
	_renderManager->renderBackbufferToScreen();
	_clock.stop();

	uint32 stopTime = _system->getMillis() + milsecs;
	while (_system->getMillis() < stopTime) {
		Common::Event evnt;
		while (_eventMan->pollEvent(evnt)) {
			if (evnt.type == Common::EVENT_KEYDOWN &&
			        (evnt.kbd.keycode == Common::KEYCODE_SPACE ||
			         evnt.kbd.keycode == Common::KEYCODE_RETURN ||
			         evnt.kbd.keycode == Common::KEYCODE_ESCAPE))
				break;
		}
		_system->updateScreen();
		if (_halveDelay)
			_system->delayMillis(33);
		else
			_system->delayMillis(66);
	}
	_renderManager->deleteSubArea(msgid);
	_clock.start();
}

void ZVision::timedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = _renderManager->createSubArea();
	_renderManager->updateSubArea(msgid, str);
	_renderManager->processSubs(0);
	_renderManager->renderBackbufferToScreen();
	_renderManager->deleteSubArea(msgid, milsecs);
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
	int16 _velocity = _mouseVelocity + _kbdVelocity;

	if (_halveDelay)
		_velocity /= 2;

	if (_velocity) {
		RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
		if (renderState == RenderTable::PANORAMA) {
			int16 startPosition = _scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + (_renderManager->getRenderTable()->getPanoramaReverse() ? -_velocity : _velocity);

			int16 zeroPoint = _renderManager->getRenderTable()->getPanoramaZeroPoint();
			if (startPosition >= zeroPoint && newPosition < zeroPoint)
				_scriptManager->setStateValue(StateKey_Rounds, _scriptManager->getStateValue(StateKey_Rounds) - 1);
			if (startPosition <= zeroPoint && newPosition > zeroPoint)
				_scriptManager->setStateValue(StateKey_Rounds, _scriptManager->getStateValue(StateKey_Rounds) + 1);

			int16 screenWidth = _renderManager->getBkgSize().x;
			if (screenWidth)
				newPosition %= screenWidth;

			if (newPosition < 0)
				newPosition += screenWidth;

			_renderManager->setBackgroundPosition(newPosition);
		} else if (renderState == RenderTable::TILT) {
			int16 startPosition = _scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + _velocity;

			int16 screenHeight = _renderManager->getBkgSize().y;
			int16 tiltGap = _renderManager->getRenderTable()->getTiltGap();

			if (newPosition >= (screenHeight - tiltGap))
				newPosition = screenHeight - tiltGap;
			if (newPosition <= tiltGap)
				newPosition = tiltGap;

			_renderManager->setBackgroundPosition(newPosition);
		}
	}
}

void ZVision::checkBorders() {
	RenderTable::RenderState renderState = _renderManager->getRenderTable()->getRenderState();
	if (renderState == RenderTable::PANORAMA) {
		int16 startPosition = _scriptManager->getStateValue(StateKey_ViewPos);

		int16 newPosition = startPosition;

		int16 screenWidth = _renderManager->getBkgSize().x;

		if (screenWidth)
			newPosition %= screenWidth;

		if (newPosition < 0)
			newPosition += screenWidth;

		if (startPosition != newPosition)
			_renderManager->setBackgroundPosition(newPosition);
	} else if (renderState == RenderTable::TILT) {
		int16 startPosition = _scriptManager->getStateValue(StateKey_ViewPos);

		int16 newPosition = startPosition;

		int16 screenHeight = _renderManager->getBkgSize().y;
		int16 tiltGap = _renderManager->getRenderTable()->getTiltGap();

		if (newPosition >= (screenHeight - tiltGap))
			newPosition = screenHeight - tiltGap;
		if (newPosition <= tiltGap)
			newPosition = tiltGap;

		if (startPosition != newPosition)
			_renderManager->setBackgroundPosition(newPosition);
	}
}

void ZVision::rotateTo(int16 _toPos, int16 _time) {
	if (_renderManager->getRenderTable()->getRenderState() != RenderTable::PANORAMA)
		return;

	if (_time == 0)
		_time = 1;

	int32 maxX = _renderManager->getBkgSize().x;
	int32 curX = _renderManager->getCurrentBackgroundOffset();
	int32 dx = 0;

	if (curX == _toPos)
		return;

	if (curX > _toPos) {
		if (curX - _toPos > maxX / 2)
			dx = (_toPos + (maxX - curX)) / _time;
		else
			dx = -(curX - _toPos) / _time;
	} else {
		if (_toPos - curX > maxX / 2)
			dx = -((maxX - _toPos) + curX) / _time;
		else
			dx = (_toPos - curX) / _time;
	}

	_clock.stop();

	for (int16 i = 0; i <= _time; i++) {
		if (i == _time)
			curX = _toPos;
		else
			curX += dx;

		if (curX < 0)
			curX = maxX - curX;
		else if (curX >= maxX)
			curX %= maxX;

		_renderManager->setBackgroundPosition(curX);

		_renderManager->prepareBkg();
		_renderManager->renderBackbufferToScreen();

		_system->updateScreen();

		_system->delayMillis(500 / _time);
	}

	_clock.start();
}

void ZVision::menuBarEnable(uint16 menus) {
	if (_menu)
		_menu->setEnable(menus);
}

uint16 ZVision::getMenuBarEnable() {
	if (_menu)
		return _menu->getEnable();
	return 0;
}

bool ZVision::ifQuit() {
	if (askQuestion(_stringManager->getTextLine(StringManager::ZVISION_STR_EXITPROMT))) {
		quitGame();
		return true;
	}
	return false;
}

void ZVision::pushKeyToCheatBuf(uint8 key) {
	for (int i = 0; i < KEYBUF_SIZE - 1; i++)
		_cheatBuff[i] = _cheatBuff[i + 1];

	_cheatBuff[KEYBUF_SIZE - 1] = key;
}

bool ZVision::checkCode(const char *code) {
	int codeLen = strlen(code);

	if (codeLen > KEYBUF_SIZE)
		return false;

	for (int i = 0; i < codeLen; i++)
		if (code[i] != _cheatBuff[KEYBUF_SIZE - codeLen + i] && code[i] != '?')
			return false;

	return true;
}

uint8 ZVision::getBufferedKey(uint8 pos) {
	if (pos >= KEYBUF_SIZE)
		return 0;
	else
		return _cheatBuff[KEYBUF_SIZE - pos - 1];
}

void ZVision::showDebugMsg(const Common::String &msg, int16 delay) {
	uint16 msgid = _renderManager->createSubArea();
	_renderManager->updateSubArea(msgid, msg);
	_renderManager->deleteSubArea(msgid, delay);
}

} // End of namespace ZVision
