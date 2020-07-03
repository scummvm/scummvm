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
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/file/save_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/scripting/menu.h"
#include "zvision/file/search_manager.h"
#include "zvision/text/text.h"
#include "zvision/text/truetype_font.h"
#include "zvision/sound/midi.h"

#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/system.h"
#include "common/file.h"

#include "gui/message.h"
#include "engines/util.h"
#include "audio/mixer.h"

namespace ZVision {

#define ZVISION_SETTINGS_KEYS_COUNT 12

struct zvisionIniSettings {
	const char *name;
	int16 slot;
	int16 defaultValue;	// -1: use the bool value
	bool defaultBoolValue;
	bool allowEditing;
} settingsKeys[ZVISION_SETTINGS_KEYS_COUNT] = {
	// Hardcoded settings
	{"countrycode", StateKey_CountryCode, 0, false, false},	// always 0 = US, subtitles are shown for codes 0 - 4, unused
	{"lineskipvideo", StateKey_VideoLineSkip, 0, false, false},	// video line skip, 0 = default, 1 = always, 2 = pixel double when possible, unused
	{"installlevel", StateKey_InstallLevel, 0, false, false},	// 0 = full, checked by universe.scr
	{"highquality", StateKey_HighQuality, -1, true, false},	// high panorama quality, unused
	{"qsoundenabled", StateKey_Qsound, -1, true, false},	// 1 = enable QSound - TODO: not supported yet
	{"debugcheats", StateKey_DebugCheats, -1, true, false},	// always start with the GOxxxx cheat enabled
	// Editable settings
	{"keyboardturnspeed", StateKey_KbdRotateSpeed, 5, false, true},
	{"panarotatespeed", StateKey_RotateSpeed, 540, false, true},	// checked by universe.scr
	{"noanimwhileturning", StateKey_NoTurnAnim, -1, false, true},	// toggle playing animations during pana rotation
	{"venusenabled", StateKey_VenusEnable, -1, true, true},
	{"subtitles", StateKey_Subtitles, -1, true, true},
	{"mpegmovies", StateKey_MPEGMovies, -1, true, true}		// Zork: Grand Inquisitor DVD hi-res MPEG movies (0 = normal, 1 = hires, 2 = disable option)
};

const char *mainKeymapId = "zvision";
const char *gameKeymapId = "zvision-game";
const char *cutscenesKeymapId = "zvision-cutscenes";

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

	memset(_cheatBuffer, 0, sizeof(_cheatBuffer));
}

ZVision::~ZVision() {
	debug(1, "ZVision::~ZVision");

	// Dispose of resources
	delete _cursorManager;
	delete _stringManager;
	delete _saveManager;
	delete _scriptManager;
	delete _renderManager;	// should be deleted after the script manager
	delete _rnd;
	delete _midiManager;

	getTimerManager()->removeTimerProc(&fpsTimerCallback);

	// Remove all of our debug levels
	DebugMan.clearAllDebugChannels();
}

void ZVision::registerDefaultSettings() {
	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++) {
		if (settingsKeys[i].allowEditing) {
			if (settingsKeys[i].defaultValue >= 0)
				ConfMan.registerDefault(settingsKeys[i].name, settingsKeys[i].defaultValue);
			else
				ConfMan.registerDefault(settingsKeys[i].name, settingsKeys[i].defaultBoolValue);
		}
	}
}

void ZVision::loadSettings() {
	int16 value = 0;
	bool boolValue = false;

	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++) {
		if (settingsKeys[i].defaultValue >= 0) {
			value = (settingsKeys[i].allowEditing) ? ConfMan.getInt(settingsKeys[i].name) : settingsKeys[i].defaultValue;
		} else {
			boolValue = (settingsKeys[i].allowEditing) ? ConfMan.getBool(settingsKeys[i].name) : settingsKeys[i].defaultBoolValue;
			value = (boolValue) ? 1 : 0;
		}

		_scriptManager->setStateValue(settingsKeys[i].slot, value);
	}

	if (getGameId() == GID_NEMESIS)
		_scriptManager->setStateValue(StateKey_ExecScopeStyle, 1);
	else
		_scriptManager->setStateValue(StateKey_ExecScopeStyle, 0);
}

void ZVision::saveSettings() {
	for (int i = 0; i < ZVISION_SETTINGS_KEYS_COUNT; i++) {
		if (settingsKeys[i].allowEditing) {
			if (settingsKeys[i].defaultValue >= 0)
				ConfMan.setInt(settingsKeys[i].name, _scriptManager->getStateValue(settingsKeys[i].slot));
			else
				ConfMan.setBool(settingsKeys[i].name, (_scriptManager->getStateValue(settingsKeys[i].slot) == 1));
		}
	}

	ConfMan.flushToDisk();
}

void ZVision::initialize() {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	_searchManager = new SearchManager(ConfMan.get("path"), 6);

	_searchManager->addDir("FONTS");
	_searchManager->addDir("addon");

	if (getGameId() == GID_GRANDINQUISITOR) {
		if (!_searchManager->loadZix("INQUIS.ZIX"))
			error("Unable to load file INQUIS.ZIX");
	} else if (getGameId() == GID_NEMESIS) {
		if (!_searchManager->loadZix("NEMESIS.ZIX")) {
			// The game might not be installed, try MEDIUM.ZIX instead
			if (!_searchManager->loadZix("ZNEMSCR/MEDIUM.ZIX"))
				error("Unable to load the file ZNEMSCR/MEDIUM.ZIX");
		}
	}

	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(WINDOW_WIDTH, WINDOW_HEIGHT));
#if defined(USE_MPEG2) && defined(USE_A52)
	// For the DVD version of ZGI we can play high resolution videos
	if (getGameId() == GID_GRANDINQUISITOR && (getFeatures() & GF_DVD))
		modes.push_back(Graphics::Mode(HIRES_WINDOW_WIDTH, HIRES_WINDOW_HEIGHT));
#endif
	initGraphicsModes(modes);

	initScreen();

	Common::Keymapper *keymapper = _system->getEventManager()->getKeymapper();
	_gameKeymap = keymapper->getKeymap(gameKeymapId);
	_gameKeymap->setEnabled(true);
	_cutscenesKeymap = keymapper->getKeymap(cutscenesKeymapId);
	_cutscenesKeymap->setEnabled(false);

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

	if (getGameId() == GID_GRANDINQUISITOR)
		_menu = new MenuZGI(this);
	else
		_menu = new MenuNemesis(this);

	// Initialize the managers
	_cursorManager->initialize();
	_scriptManager->initialize();
	_stringManager->initialize(getGameId());

	registerDefaultSettings();

	loadSettings();

#if !defined(USE_MPEG2) || !defined(USE_A52)
	// libmpeg2 or liba52 not loaded, disable the MPEG2 movies option
	_scriptManager->setStateValue(StateKey_MPEGMovies, 2);
#endif

	// Create debugger console. It requires GFX to be initialized
	setDebugger(new Console(this));
	_doubleFPS = ConfMan.getBool("doublefps");

	// Initialize FPS timer callback
	getTimerManager()->installTimerProc(&fpsTimerCallback, 1000000, this, "zvisionFPS");
}

extern const FontStyle getSystemFont(int fontIndex);

Common::Error ZVision::run() {
	initialize();

	// Check if a saved game is to be loaded from the launcher
	if (ConfMan.hasKey("save_slot"))
		_saveManager->loadGame(ConfMan.getInt("save_slot"));

	bool foundAllFonts = true;

	// Before starting, make absolutely sure that the user has copied the needed fonts
	for (int i = 0; i < FONT_COUNT; i++) {
		FontStyle curFont = getSystemFont(i);
		Common::String freeFontBoldItalic = Common::String("Bold") + curFont.freeFontItalicName;

		const char *fontSuffixes[4] = { "", "bd", "i", "bi" };
		const char *freeFontSuffixes[4] = { "", "Bold", curFont.freeFontItalicName, freeFontBoldItalic.c_str() };
		const char *liberationFontSuffixes[4] = { "-Regular", "-Bold", "-Italic", "-BoldItalic" };

		for (int j = 0; j < 4; j++) {
			Common::String fontName = curFont.fontBase;
			if (fontName == "censcbk" && j > 0)
				fontName = "schlbk";
			fontName += fontSuffixes[j];
			fontName += ".ttf";

			if (fontName == "schlbkbd.ttf")
				fontName = "schlbkb.ttf";
			if (fontName == "garabi.ttf")
				continue;
			if (fontName == "garai.ttf")
				fontName = "garait.ttf";

			Common::String freeFontName = curFont.freeFontBase;
			freeFontName += freeFontSuffixes[j];
			freeFontName += ".ttf";

			Common::String liberationFontName = curFont.liberationFontBase;
			liberationFontName += liberationFontSuffixes[j];
			liberationFontName += ".ttf";

			if (!Common::File::exists(fontName) && !_searchManager->hasFile(fontName) &&
				!Common::File::exists(liberationFontName) && !_searchManager->hasFile(liberationFontName) &&
				!Common::File::exists(freeFontName) && !_searchManager->hasFile(freeFontName) &&
				!Common::File::exists("fonts.dat") && !_searchManager->hasFile("fonts.dat")) {
				foundAllFonts = false;
				break;
			}
		}

		if (!foundAllFonts)
			break;
	}

	if (!foundAllFonts) {
		GUI::MessageDialog dialog(_(
				"Before playing this game, you'll need to copy the required "
				"fonts into ScummVM's extras directory, or into the game directory. "
				"On Windows, you'll need the following font files from the Windows "
				"font directory: Times New Roman, Century Schoolbook, Garamond, "
				"Courier New and Arial. Alternatively, you can download the "
				"Liberation Fonts or the GNU FreeFont package. You'll need all the "
				"fonts from the font package you choose, i.e., LiberationMono, "
				"LiberationSans and LiberationSerif, or FreeMono, FreeSans and "
				"FreeSerif respectively."
		));
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

void ZVision::setRenderDelay(uint delay) {
	_frameRenderDelay = delay;
}

bool ZVision::canRender() {
	return _frameRenderDelay <= 0;
}

void ZVision::syncSoundSettings() {
	Engine::syncSoundSettings();

	_scriptManager->setStateValue(StateKey_Subtitles, ConfMan.getBool("subtitles") ? 1 : 0);
}

void ZVision::fpsTimerCallback(void *refCon) {
	((ZVision *)refCon)->fpsTimer();
}

void ZVision::fpsTimer() {
	_fps = _renderedFrameCount;
	_renderedFrameCount = 0;
}

void ZVision::initScreen() {
	uint16 workingWindowWidth = (getGameId() == GID_NEMESIS) ? ZNM_WORKING_WINDOW_WIDTH : ZGI_WORKING_WINDOW_WIDTH;
	uint16 workingWindowHeight = (getGameId() == GID_NEMESIS) ? ZNM_WORKING_WINDOW_HEIGHT : ZGI_WORKING_WINDOW_HEIGHT;
	_workingWindow = Common::Rect(
						 (WINDOW_WIDTH  -  workingWindowWidth) / 2,
						 (WINDOW_HEIGHT - workingWindowHeight) / 2,
						((WINDOW_WIDTH  -  workingWindowWidth) / 2) + workingWindowWidth,
						((WINDOW_HEIGHT - workingWindowHeight) / 2) + workingWindowHeight
					 );

	initGraphics(WINDOW_WIDTH, WINDOW_HEIGHT, &_screenPixelFormat);
}

void ZVision::initHiresScreen() {
	_renderManager->upscaleRect(_workingWindow);

	initGraphics(HIRES_WINDOW_WIDTH, HIRES_WINDOW_HEIGHT, &_screenPixelFormat);
}

} // End of namespace ZVision
