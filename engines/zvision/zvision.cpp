/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/mixer.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/translation.h"
#include "engines/util.h"
#include "gui/message.h"
#include "zvision/zvision.h"
#include "zvision/core/console.h"
#include "zvision/file/file_manager.h"
#include "zvision/file/save_manager.h"
#include "zvision/file/zfs_archive.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/scripting/menu.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/sound/midi.h"
#include "zvision/sound/volume_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/text/subtitle_manager.h"
#include "zvision/text/text.h"
#include "zvision/text/truetype_font.h"

namespace ZVision {

#define ZVISION_SETTINGS_KEYS_COUNT 15

struct ZvisionIniSettings {
	const char *name;
	int16 slot;
	int16 defaultValue;	// -1: use the bool value
	bool defaultBoolValue;
	bool allowEditing;
} settingsKeys[ZVISION_SETTINGS_KEYS_COUNT] = {
	// Hardcoded settings
	{"countrycode", StateKey_CountryCode, 0, false, false}, // always 0 = US, subtitles are shown for codes 0 - 4, unused
	{"lineskipvideo", StateKey_VideoLineSkip, 0, false, false}, // video line skip, 0 = default, 1 = always, 2 = pixel double when possible, unused
	{"installlevel", StateKey_InstallLevel, 0, false, false},   // 0 = full, checked by universe.scr
	{"debugcheats", StateKey_DebugCheats, -1, true, false}, // always start with the GOxxxx cheat enabled
	{"Pentium", StateKey_CPU, 1, true, false},	// !1 = 486, 1 = i586/Pentium
	{"LowMemory", StateKey_WIN958, 0, false, false},	// 0 = high system RAM, !0 = low system RAM (<8MB)
	{"DOS", StateKey_Platform, 0, false, false}, // 0 = Windows, !0 = DOS
	// Editable settings
	{"qsoundenabled", StateKey_Qsound, -1, true, true}, // 1 = enable generic directional audio and non-linear volume scaling.  Genuine Qsound is copyright & unlikely to be implemented.
	{"keyboardturnspeed", StateKey_KbdRotateSpeed, 5, false, true},
	{"panarotatespeed", StateKey_RotateSpeed, 540, false, true},    // checked by universe.scr
	{"noanimwhileturning", StateKey_NoTurnAnim, -1, false, true},   // toggle playing animations during pana rotation
	{"highquality", StateKey_HighQuality, -1, true, false}, // high panorama quality; enables bilinear filtering in RenderTable
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
	  _clock(_system),
	  _scriptManager(nullptr),
	  _renderManager(nullptr),
	  _fileManager(nullptr),
	  _saveManager(nullptr),
	  _stringManager(nullptr),
	  _cursorManager(nullptr),
	  _midiManager(nullptr),
	  _rnd(nullptr),
	  _menu(nullptr),
	  _subtitleManager(nullptr),
	  _volumeManager(nullptr),
	  _textRenderer(nullptr),
	  _doubleFPS(false),
	  _widescreen(false),
	  _audioId(0),
	  _frameRenderDelay(2),
	  _keyboardVelocity(0),
	  _mouseVelocity(0),
	  _videoIsPlaying(false),
	  _renderedFrameCount(0),
	  _fps(0) {

	debugC(1, kDebugLoop, "ZVision::ZVision");

	memset(_cheatBuffer, 0, sizeof(_cheatBuffer));
}

ZVision::~ZVision() {
	debugC(1, kDebugLoop, "ZVision::~ZVision");

	// Dispose of resources
	delete _cursorManager;
	delete _stringManager;
	delete _fileManager;
	delete _saveManager;
	delete _scriptManager;
	delete _renderManager;	// should be deleted after the script manager
	delete _subtitleManager;
	delete _rnd;
	delete _midiManager;
	delete _volumeManager;
	getTimerManager()->removeTimerProc(&fpsTimerCallback);
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
	// Graphics
	_widescreen = ConfMan.getBool("widescreen");
	_doubleFPS = ConfMan.getBool("doublefps");

	// Keymaps

	Common::Keymapper *keymapper = _system->getEventManager()->getKeymapper();
	_gameKeymap = keymapper->getKeymap(gameKeymapId);
	_gameKeymap->setEnabled(true);
	_cutscenesKeymap = keymapper->getKeymap(cutscenesKeymapId);
	_cutscenesKeymap->setEnabled(false);

	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	switch (getGameId()) {
	case GID_NEMESIS:
		_renderManager = new RenderManager(this, nemesisLayout, _resourcePixelFormat, _doubleFPS, _widescreen);
		_menu = new MenuNemesis(this, _renderManager->getMenuArea());
		_subtitleManager = new SubtitleManager(this, nemesisLayout, _resourcePixelFormat, _doubleFPS);
		_volumeManager = new VolumeManager(this, kVolumePowerLaw);
		break;
	case GID_GRANDINQUISITOR:
		_renderManager = new RenderManager(this, zgiLayout, _resourcePixelFormat, _doubleFPS, _widescreen);
		_menu = new MenuZGI(this, _renderManager->getMenuArea());
		_subtitleManager = new SubtitleManager(this, zgiLayout, _resourcePixelFormat, _doubleFPS);
		_volumeManager = new VolumeManager(this, kVolumeLogAmplitude);
		break;
	case GID_NONE:
	default:
		error("Unknown/unspecified GameId");
		break;
	}
	_scriptManager = new ScriptManager(this);
	_fileManager = new FileManager(this);
	_saveManager = new SaveManager(this);
	_stringManager = new StringManager(this);
	_cursorManager = new CursorManager(this, _resourcePixelFormat);
	_textRenderer = new TextRenderer(this);
	_midiManager = new MidiManager();


	// Initialize the managers
	_renderManager->initialize();
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

	// Initialize FPS timer callback
	getTimerManager()->installTimerProc(&fpsTimerCallback, 1000000, this, "zvisionFPS");
	// Ensure a new game is launched with correct panorama quality setting
	_scriptManager->setStateValue(StateKey_HighQuality, ConfMan.getBool("highquality"));
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

		const char *fontSuffixes[4] = { "", "bd", "i", "bi" };
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

			Common::String liberationFontName = curFont.liberationFontBase;
			liberationFontName += liberationFontSuffixes[j];
			liberationFontName += ".ttf";

			if (!Common::File::exists(Common::Path(fontName)) && !SearchMan.hasFile(Common::Path(fontName)) &&
				!Common::File::exists(Common::Path(liberationFontName)) && !SearchMan.hasFile(Common::Path(liberationFontName)) &&
				!Common::File::exists("fonts.dat") && !SearchMan.hasFile("fonts.dat")) {
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
				"Liberation Fonts package. You'll need all the fonts from the "
				"font package you choose, i.e., LiberationMono, LiberationSans "
				"and LiberationSerif."
		));
		dialog.runModal();
		quitGame();
		return Common::kUnknownError;
	}
	if (getGameId() == GID_NEMESIS && !_midiManager->isAvailable()) {
		GUI::MessageDialog MIDIdialog(_(
				"MIDI playback is not available, or else improperly configured. "
				"Zork Nemesis contains several music puzzles which require "
				"MIDI audio in order to be solved.  These puzzles may alternatively "
				"be solved using subtitles, if supported. Continue launching game?"
				),
				_("Yes"),
				_("No")
			);
		if (MIDIdialog.runModal() != GUI::kMessageOK)
			quitGame();
	}

	// Main loop
	while (!shouldQuit() && !_breakMainLoop) {
		debugC(1, kDebugLoop, "\nInitiating new game cycle");
		debugC(5, kDebugLoop, "Timers");
		// Timers
		_clock.update();
		uint32 deltaTime = _clock.getDeltaTime();
		debugC(5, kDebugLoop, "Logic");
		// Process game logic & update backbuffers as necessary
		debugC(5, kDebugLoop, "Cursor");
		_cursorManager->setItemID(_scriptManager->getStateValue(StateKey_InventoryItem));
		debugC(5, kDebugLoop, "Events");
		processEvents();  // NB rotateTo or playVideo event will pause clock & call renderSceneToScreen() directly.
		debugC(5, kDebugLoop, "Rotation");
		_renderManager->updateRotation();
		debugC(5, kDebugLoop, "Scripts");
		_scriptManager->process(deltaTime);
		debugC(5, kDebugLoop, "Menu");
		_menu->process(deltaTime);
		debugC(5, kDebugLoop, "Subtitles");
		_subtitleManager->process(deltaTime);
		debugC(5, kDebugLoop, "Render");
		// Render the backBuffer to the screen
		_renderManager->prepareBackground();
		if (_renderManager->renderSceneToScreen()) {
			_renderedFrameCount++;
		} else {
			_frameRenderDelay--;
		}
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
	debugC(2, kDebugGraphics, "Setting framerenderdelay to %d", delay);
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

void ZVision::initializePath(const Common::FSNode &gamePath) {
	// File Paths
	const Common::FSNode gameDataDir(gamePath);
	SearchMan.setIgnoreClashes(true);

	SearchMan.addDirectory(gamePath, 0, 1, true);
	switch (getGameId()) {
	case GID_GRANDINQUISITOR:
		break;
	case GID_NEMESIS:
		SearchMan.addSubDirectoriesMatching(gameDataDir, "znemscr", true); // Add directory that may contain .zix file in some versions of the game
		break;
	case GID_NONE:
	default:
		break;
	}

	SearchMan.addSubDirectoryMatching(gameDataDir, "FONTS");

	// Ensure extras take first search priority
	if (ConfMan.hasKey("extrapath")) {
		Common::Path gameExtraPath = ConfMan.getPath("extrapath");
		const Common::FSNode gameExtraDir(gameExtraPath);
		SearchMan.addSubDirectoryMatching(gameExtraDir, "auxvid");
		SearchMan.addSubDirectoryMatching(gameExtraDir, "auxscr");
	}

	// Ensure addons (game patches) take search priority over files listed in .zix files
	SearchMan.addSubDirectoryMatching(gameDataDir, "addon");
	Common::ArchiveMemberList listAddon;
	SearchMan.listMatchingMembers(listAddon,"*.zfs");
	for (auto &member : listAddon) {
		Common::Path path(member->getPathInArchive());
		ZfsArchive *archive = new ZfsArchive(path);
		SearchMan.add(path.toString(), archive);
	}

	switch (getGameId()) {
	case GID_GRANDINQUISITOR:
		if (!_fileManager->loadZix("INQUIS.ZIX", gameDataDir))
			error("Unable to load file INQUIS.ZIX");
		break;
	case GID_NEMESIS:
		if (!_fileManager->loadZix("NEMESIS.ZIX", gameDataDir))	// GOG version or used original game installer
			if (!_fileManager->loadZix("MEDIUM.ZIX", gameDataDir))	// Manual installation from CD or ZGI DVD according to wiki.scummvm.org
				error("Unable to load file NEMESIS.ZIX or MEDIUM.ZIX");
		break;
	case GID_NONE:
	default:
		error("Unknown/unspecified GameId");
		break;
	}
}

} // End of namespace ZVision
