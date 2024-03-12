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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "image/iff.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/texte.h"
#include "cine/sound.h"
#include "cine/various.h"

namespace Cine {

Sound *g_sound = nullptr;

CineEngine *g_cine = nullptr;

CineEngine::CineEngine(OSystem *syst, const CINEGameDescription *gameDesc)
	: Engine(syst),
	_gameDescription(gameDesc),
	_rnd("cine") {

	// Setup mixer
	syncSoundSettings();

	setDebugger(new CineConsole(this));

	g_cine = this;

	for (int i = 0; i < NUM_FONT_CHARS; i++) {
		_textHandler.fontParamTable[i].characterIdx = 0;
		_textHandler.fontParamTable[i].characterWidth = 0;
	}
	_restartRequested = false;
	_preLoad = false;
	setDefaultGameSpeed();
}

CineEngine::~CineEngine() {
	if (getGameType() == Cine::GType_OS) {
		freeErrmessDat();
	}
}

void CineEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// Use music volume for plain sound types (At least the AdLib player uses a plain sound type
	// so previously the music and sfx volume controls didn't affect it at all).
	// FIXME: Make AdLib player differentiate between playing sound effects and music and remove this.
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
									mute ? 0 : ConfMan.getInt("music_volume"));
}

Common::Error CineEngine::run() {
	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(320, 200));
	if (g_cine->getGameType() == GType_FW && (g_cine->getFeatures() & GF_CD)) {
		modes.push_back(Graphics::Mode(640, 480));
		initGraphicsModes(modes);
		showSplashScreen();
	} else {
		initGraphicsModes(modes);
	}

	// Initialize backend
	initGraphics(320, 200);

	if (g_cine->getGameType() == GType_FW && (g_cine->getFeatures() & GF_CD)) {
		if (!existExtractedCDAudioFiles(19)  // tracks <19 are not used
		    && !isDataAndCDAudioReadFromSameCD()) {
			warnMissingExtractedCDAudio();
		}
	}

	if (getPlatform() == Common::kPlatformDOS) {
		g_sound = new PCSound(_mixer, this);
	} else {
		// Paula chipset for Amiga and Atari versions
		g_sound = new PaulaSound(_mixer, this);
	}

	_restartRequested = false;

	do {
		initialize();

		_restartRequested = false;

		CursorMan.showMouse(true);
		mainLoop(BOOT_SCRIPT_INDEX);

		delete renderer;
		delete[] collisionPage;
		delete _scriptInfo;
	} while (_restartRequested);

	delete g_sound;

	return Common::kNoError;
}

uint32 CineEngine::getTimerDelay() const {
	return (10923000 * _timerDelayMultiplier) / 1193180;
}

/**
 * Modify game speed
 * @param speedChange Negative values slow game down, positive values speed it up, zero does nothing
 * @return Timer delay multiplier's value after the game speed change
 */
int CineEngine::modifyGameSpeed(int speedChange) {
	// If we want more speed we decrement the timer delay multiplier and vice versa.
	_timerDelayMultiplier = CLIP(_timerDelayMultiplier - speedChange, 1, 50);
	return _timerDelayMultiplier;
}

void CineEngine::setDefaultGameSpeed() {
	_timerDelayMultiplier = 12;
}

void CineEngine::initialize() {
	setTotalPlayTime(0); // Reset total play time
	_globalVars.reinit(NUM_MAX_VAR + 1);

	// Initialize all savegames' descriptions to empty strings
	memset(currentSaveName, 0, sizeof(currentSaveName));

	// Resize object table to its correct size and reset all its elements
	g_cine->_objectTable.resize(NUM_MAX_OBJECT);
	resetObjectTable();

	// Resize animation data table to its correct size and reset all its elements
	g_cine->_animDataTable.resize(NUM_MAX_ANIMDATA);
	freeAnimDataTable();

	// Resize zone data table to its correct size and reset all its elements
	g_cine->_zoneData.resize(NUM_MAX_ZONE);
	Common::fill(g_cine->_zoneData.begin(), g_cine->_zoneData.end(), 0);

	// Resize zone query table to its correct size and reset all its elements
	g_cine->_zoneQuery.resize(NUM_MAX_ZONE);
	Common::fill(g_cine->_zoneQuery.begin(), g_cine->_zoneQuery.end(), 0);

	setDefaultGameSpeed();
	_scriptInfo = setupOpcodes();

	initLanguage(getLanguage());

	if (getGameType() == Cine::GType_OS) {
		renderer = new OSRenderer;
	} else {
		renderer = new FWRenderer;
	}

	renderer->initialize();
	forbidBgPalReload = 0;
	reloadBgPalOnNextFlip = 0;
	gfxFadeOutCompleted = 0;
	gfxFadeInRequested = 0;
	safeControlsLastAccessedMs = 0;
	lastSafeControlObjIdx = -1;
	currentDisk = 1;

	collisionPage = new byte[320 * 200]();

	// Clear part buffer as there's nothing loaded into it yet.
	// Its size will change when loading data into it with the loadPart function.
	g_cine->_partBuffer.clear();

	if (getGameType() == Cine::GType_OS) {
		readVolCnf();
	}

	loadTextData("texte.dat");

	if (getGameType() == Cine::GType_OS && !(getFeatures() & GF_DEMO)) {
		loadPoldatDat("poldat.dat");
		loadErrmessDat("errmess.dat");
	}

	// in case ScummVM engines can be restarted in the future
	g_cine->_scriptTable.clear();
	g_cine->_relTable.clear();
	g_cine->_objectScripts.clear();
	g_cine->_globalScripts.clear();
	g_cine->_bgIncrustList.clear();
	freeAnimDataTable();
	g_cine->_overlayList.clear();
	g_cine->_messageTable.clear();
	resetObjectTable();
	g_cine->_seqList.clear();

	if (getGameType() == Cine::GType_OS) {
		disableSystemMenu = 1;
	} else {
		// WORKAROUND: We do not save this variable in FW's savegames.
		// Initializing this to 1, like we do it in the OS case, will
		// cause the menu disabled when loading from the launcher or
		// command line.
		// A proper fix here would be to save this variable in FW's saves.
		// Since it seems these are unversioned so far, there would be need
		// to properly add versioning to them first.
		//
		// Adding versioning to FW saves didn't solve this problem. Setting
		// disableSystemMenu according to the saved value still caused the
		// action menu (EXAMINE, TAKE, INVENTORY, ...) sometimes to be
		// disabled when it wasn't supposed to be disabled when
		// loading from the launcher or command line.
		disableSystemMenu = 0;
	}

	var8 = 0;
	bgVar0 = 0;
	var2 = var3 = var4 = lastType20OverlayBgIdx = 0;
	musicIsPlaying = 0;
	currentDatName[0] = 0;
	_keyInputList.clear();

	// Used for making sound effects work using Roland MT-32 and AdLib in
	// Operation Stealth after loading a savegame. The sound effects are loaded
	// in AUTO00.PRC using a combination of o2_loadAbs and o2_playSample(1, ...)
	// before o1_freePartRange(0, 200). In the original game AUTO00.PRC
	// was run when starting or restarting the game and one could not load a savegame
	// before passing the copy protection. Thus, we try to emulate that behaviour by
	// running at least part of AUTO00.PRC before loading a savegame.
	//
	// Confirmed that DOS and Atari ST versions do have these commands in their AUTO00.PRC files.
	// Confirmed that Amiga and demo versions do not have these commands in their AUTO00.PRC files.
	if (getGameType() == Cine::GType_OS && !(getFeatures() & GF_DEMO) &&
		(getPlatform() == Common::kPlatformDOS || getPlatform() == Common::kPlatformAtariST)) {
		loadPrc(BOOT_PRC_NAME);
		Common::strcpy_s(currentPrcName, BOOT_PRC_NAME);
		addScriptToGlobalScripts(BOOT_SCRIPT_INDEX);
		runOnlyUntilFreePartRangeFirst200 = true;
		executeGlobalScripts();
	}

	_preLoad = false;
	if (ConfMan.hasKey("save_slot") && !_restartRequested) {
		Common::Error loadError = loadGameState(ConfMan.getInt("save_slot"));

		if (loadError.getCode() == Common::kNoError)
			_preLoad = true;
	}

	if (!_preLoad) {
		loadPrc(BOOT_PRC_NAME);
		Common::strcpy_s(currentPrcName, BOOT_PRC_NAME);
		setMouseCursor(MOUSE_CURSOR_NORMAL);
	}
}

void CineEngine::showSplashScreen() {
	Common::File file;
	if (!file.open("sony.lbm"))
		return;

	Image::IFFDecoder decoder;
	if (!decoder.loadStream(file))
		return;

	const Graphics::Surface *surface = decoder.getSurface();
	if (surface->w == 640 && surface->h == 480) {
		initGraphics(640, 480);

		const byte *palette = decoder.getPalette();
		int paletteColorCount = decoder.getPaletteColorCount();
		g_system->getPaletteManager()->setPalette(palette, 0, paletteColorCount);

		g_system->copyRectToScreen(surface->getPixels(), 640, 0, 0, 640, 480);
		g_system->updateScreen();

		Common::EventManager *eventMan = g_system->getEventManager();

		bool done = false;
		uint32 now = g_system->getMillis();

		while (!done && g_system->getMillis() - now < 2000) {
			Common::Event event;
			while (eventMan->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					done = true;
					break;
				}
				if (shouldQuit())
					done = true;
			}
		}
	}

	decoder.destroy();
}

} // End of namespace Cine
