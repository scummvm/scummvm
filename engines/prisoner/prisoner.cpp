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

#include "common/algorithm.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "prisoner/prisoner.h"
#include "prisoner/kroarchive.h"
#include "prisoner/midi.h"
#include "prisoner/muxplayer.h"
#include "prisoner/path.h"
#include "prisoner/resourcemgr.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"
#include "prisoner/scriptops.h"

#include "prisoner/menumgr.h"

namespace Prisoner {

PrisonerEngine::PrisonerEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_midi = nullptr;

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("prisoner");

	_digitalMusic = !(ConfMan.hasKey("midi_music") && ConfMan.getBool("midi_music"));

	_menuFunctionArray[kMainPanel] = &PrisonerEngine::updateMainMenu;
	_menuFunctionArray[kOptionsPanel] = &PrisonerEngine::updateOptionsMenu;
	_menuFunctionArray[kOptGfxPanel] = &PrisonerEngine::updateDisplaySettingsMenu;
	_menuFunctionArray[kDivOptPanel] = &PrisonerEngine::updateTextMusicMenu;
	_menuFunctionArray[kVolOptPanel] = &PrisonerEngine::updateVolumeMenu;
}

PrisonerEngine::~PrisonerEngine() {
	delete _rnd;
}

Common::Error PrisonerEngine::run() {
	initGraphics(640, 480);

	// As per metaengine, the language files start with the language letter (A_KLANG.BIN, E_KLANG.BIN, D_KLANG.BIN...).
	// Every detection entry lists its language's KLANG.BIN first, so take the letter from there.
	_languageChar = toupper(_gameDescription->filesDescriptions[0].fileName[0]);

	_currModuleIndex = 2;

	_isPlayingMux = false;
	_inventoryInputRunning = false;

	_cameraX = 0;
	_cameraY = 0;

	_screen = new Screen(this);
	_resLoader = new PrisonerResourceLoader(_languageChar);

	// Videos with speech are in a language-specific folder, e.g. E_VIDEO (like the original)
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, Common::String::format("%c_video", tolower(_languageChar)));

	_res = new ResourceManager(_resLoader);
	_scriptOpcodes = new ScriptOpcodes(this);
	_scriptOpcodes->setupOpcodes();
	_pathSystem = new PathSystem(this);

	_screenBackupSurface = new Graphics::Surface();
	_screenBackupSurface->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_screenBackedup = false;

	_menuBackgroundSurface = new Graphics::Surface();
	_menuBackgroundSurface->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_menuBackgroundSaved = false;

	_screen->initPaletteTransTable(65);
	memcpy(_effectPalette, constPalette1, 768);
	memcpy(_scenePalette, constPalette1, 768);
	_effectPaletteOk = true;
	_scenePaletteOk = true;
	_screen->buildPaletteTransTable(constPalette1, 0);
	_needToUpdatePalette = true;

	initializeMidi();

#if 0
	initInput();
	MuxPlayer mux(this);
	mux.open("Sin1.mux");
	//mux.open("Vintro.mux");
	mux.play();
	mux.close();
	return Common::kNoError;
#endif

#if 0
	//Common::String pakName = "E_TCETXT";
	//_res->dump(pakName, 1, 3);
	Common::String pakName = "E_M02R06";
	_res->dump(pakName, 25, 16);
#endif

#if 0
	Common::String pakName = "MUS02";
	int16 slot = _res->load<MidiResource>(pakName, 0, 13);
	_midi->playMusic(_res->get<MidiResource>(slot), 200, false);
#endif

#if 1
	Common::String n;

	// TODO: Later read font def from config data
	{
		Common::String pakName = "S_FONT";
		_menuFont = addFont(pakName, 0, 2, 2);
		_textFont = addFont(pakName, 1, 96, 255);
	}

	setDefaultTextDisplayColors();

	initInput();

	CursorMan.showMouse(true);
	loadMouseCursors();

	// Sets _talkieEnabled and _speechMuted from ScummVM's settings
	syncSoundSettings();

	_menuMouseCursorActive = false;
	_menuMouseCursor = -1;
	_updateDirtyRectsFlag = true;
	_autoSaveRequested = false;
	_mainMenuRequested = false;
	_currOptionsMenuIndex = 0;
	_optionsOpenedFromGame = false;
	_isDialogMenuShowing = false;
	_inGame = false;
	_mainLoopDone = false;
	_dialogRunning = false;
	_screenTextShowing = false;
	_lockUserInputRefCounter = 0;
	_zoneMouseCursorActive = false;
	_inventoryItemCursor = -1;
	_currMouseCursor = -1;
	_currAnimatedMouseCursor = -1;

	_screensaverRunning = false;
	_screensaverAborted = false;
	_screensaverTime = kTicksPerMinute;

	_cameraFollowsActorIndex = -1;
	_backgroundResourceCacheSlot = -1;
	_backgroundResource = NULL;
	_backgroundWidth = 0;
	_backgroundHeight = 0;
	_backgroundNoScrollFlag1 = false;
	_backgroundNoScrollFlag2 = false;
	_needToPlayMux = false;
	_inScene = false;
	_alarmPaletteSubDelta = 15;
	_alarmPaletteSub = 10;

	_dialogPanelResourceCacheSlot = -1;
	_inventoryItemsResourceCacheIndex = -1;
	_inventoryBoxResourceCacheSlot = -1;

	clearScriptPrograms();

	resetGlobalScriptVars();

	_moduleScriptVars[0] = -1;
	for (uint i = 1; i < 300; i++)
		_moduleScriptVars[i] = 0;

	_animationFrameTimeFlag = false;
	_animationSpeed = 100;

	_loadingSavegame = false;

	_backgroundObjectsResourceCacheSlot = -1;
	_clearBackgroundFlag = false;
	_sceneFlag = false;
	_currInventoryItemCursor = -1;

	/* Screen text init */
	_autoAdvanceScreenTexts = false;

	/* Talkie init */
	_talkieSpeechActive = false;
	_talkieDataResourceCacheSlot = -1;
	_talkieSpeechPlayNow = false;

	/* Scene init */
	_enterSceneScriptIndex = -1;
	_enterSceneScriptProgramIndex = -1;
	_leaveSceneScriptIndex = -1;
	_leaveSceneScriptProgramIndex = -1;

	/* LipSync init */
	_lipSyncScriptNumber = -1;
	_lipSyncActorIndex = -1;
	_lipSyncResourceCacheSlot = -1;
	_lipSyncChannelStatusRestored = false;

	/* Inventory init */
	_inventoryBarEnabled = false;
	_inventoryBarFlag = false;
	_inventoryWarpMouse = false;

	_cameraFocusActor = false;
	_cameraFollowsActorIndex = -1;

	_halfScreenScrolling = true;
	loadDisplaySettings();
	loadTextMusicSettings();

	_currModuleIndex = -1;
	_currSceneIndex = -1;
	_newModuleIndex = -1;
	_newSceneIndex = -1;

	// Note: These are now set via the main menu
	// TODO: Remove after debugging
	//_newModuleIndex = 2;
	//_newSceneIndex = 39;

#if 0
	_newModuleIndex = 3;
	_newSceneIndex = 1;
#endif

#if 0
	_newModuleIndex = 9;
	//_newSceneIndex = 2;//lava room
	//_newSceneIndex = 4;//mine cart room
	//_newSceneIndex = 9;//lab room (present)
	//_newSceneIndex = 15;//lab room (future)
	_newSceneIndex = 14;
#endif

#if 0
	_newModuleIndex = 10;
	_newSceneIndex = 1;
#endif

#if 0
	_newModuleIndex = 2;
	_newSceneIndex = 10;
#endif

#if 0
	_newModuleIndex = 3;
	//_newSceneIndex = 27;//safe
	_newSceneIndex = 37;
#endif

#if 0
	_newModuleIndex = 6;
	_newSceneIndex = 7;
#endif

#if 0
	_newModuleIndex = 2;
	_newSceneIndex = 33;//radar
#endif

	{
		// TODO: Move to init function
		// TODO: Load pakName/slot from exe etc.
		Common::String pakName;
		pakName = "F_ICETXT";
		_globalTextResourceCacheSlot = loadTextResource(pakName, 0);
		pakName = "S_PANEL";
		_inventoryBoxResourceCacheSlot = _res->load<AnimationResource>(pakName, getMenuPanelData(kForLookInvPanel).pakSlot, 11);
	}

	loadMenuPanels();


	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= getMetaEngine()->getMaximumSaveSlot()) {
			loadGameState(saveSlot);
		}
		_mainMenuRequested = false;
		_inGame = true;
	} else {
		playIntroVideos();
		// TODO: Later: _mainMenuRequested = true;
	}

	mainLoop();

	{
		// TODO: Move to shutdown function
		_res->unload(_globalTextResourceCacheSlot);
		_globalTextResourceCacheSlot = -1;
		_res->unload(_inventoryBoxResourceCacheSlot);
		_inventoryBoxResourceCacheSlot = -1;
	}

#endif

	shutdownMidi();
	_menuBackgroundSurface->free();
	delete _menuBackgroundSurface;
	delete _screenBackupSurface;
	delete _pathSystem;
	delete _scriptOpcodes;
	delete _res;
	delete _resLoader;

	debug("Exit ok");
	return Common::kNoError;
}

const Common::String PrisonerEngine::getGlobalText(Common::String &identifier) {
	TextResource *textResource = _res->get<TextResource>(_globalTextResourceCacheSlot);
	return textResource->getText(identifier)->getChunkLineString(0, 0);
}

void PrisonerEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	// The original's sound effects and music on/off switches (volume settings screen)
	bool muteAll = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, muteAll || (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute")));
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, muteAll || (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute")));

	bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	_speechMuted = mute || ConfMan.getBool("speech_mute");
	// In the original's talkie mode spoken lines are shown without text
	_talkieEnabled = !_speechMuted && !ConfMan.getBool("subtitles");

	if (_midi)
		_midi->syncVolume();
}

void PrisonerEngine::checkScreensaver() {
	// A stand-by time of 0 disables the screensaver
	if (_screensaverTime > 0 && _screensaverTime < (int32)getTicks() - _lastInputTicks) {
		_mainMenuRequested = false;
		runScreensaver();
	}
}

void PrisonerEngine::mainLoop() {

	// shouldQuit() is set when the player quits or returns to the launcher from ScummVM's menu
	while (!_mainLoopDone && !shouldQuit()) {

		updateEvents();

		if (_mainMenuRequested) {
			// The original's menus show the menu cursor while they run (runMainMenu)
			if (!_menuMouseCursorActive) {
				_menuMouseCursorActive = true;
				_menuMouseCursor = kCursorMenu;
				updateMouseCursor();
			}

			updateFrameTime();
			updateAnimationFrameTicks();
			checkScreensaver();

			// Start each menu frame from what was behind the menu, so a panel never
			// shows through the one drawn after it
			if (_menuBackgroundSaved)
				memcpy(_screen->getScreen()->getPixels(), _menuBackgroundSurface->getPixels(), 640 * 480);
			else
				_screen->clear();

			if (_menuFunctionArray[_selectedMenuIndex]) {
				(this->*_menuFunctionArray[_selectedMenuIndex])(67, 77);
				// updateMainMenu(67, 77);
			}
			handleInput(_cameraX + _mouseX, _cameraY + _mouseY);

			updateMouseCursor();
			updateMouseCursorAnimation();

			if (_needToUpdatePalette) {
				_screen->setFullPalette(_effectPalette);
				_needToUpdatePalette = false;
			}


			_screen->update();
			_system->delayMillis(10);

			continue;
		}

		// Only the game screen captured by openMainMenu() in this frame is valid
		_menuBackgroundSaved = false;

		// Leaving the menus brings back the game's cursor
		if (_menuMouseCursorActive) {
			_menuMouseCursorActive = false;
			_menuMouseCursor = -1;
			updateMouseCursor();
		}

		if (isPaused()) {
			handleInput(_cameraX + _mouseX, _cameraY + _mouseY);
			continue;
		}

		updateFrameTime();
		updateAnimationFrameTicks();
		updateActors();
		runScripts(kSceneScriptProgram);
		updateModuleScript();
		if (_needToPlayMux) {
			playMux(_muxFilename);
			_needToPlayMux = false;
			updateScreen(true, _cameraX + _mouseX, _cameraY + _mouseY);
		}
		updateScreen(true, _cameraX + _mouseX, _cameraY + _mouseY);

		if (_paletteTasks[3].active || _currSceneIndex == -1 || !_sceneFlag ||
			(_moduleScriptCalled && !_screenTextShowing)) {
			_sceneFlag = true;
		} else {
			handleInput(_cameraX + _mouseX, _cameraY + _mouseY);
		}

		if (_autoSaveRequested) {
			_autoSaveRequested = false;
			performAutoSave();
		}

		// TODO: updateDirtyRects();
		checkForSceneChange();
		updateMouseCursor();
		updateMouseCursorAnimation();

		if (_needToUpdatePalette) {
			_screen->setFullPalette(_effectPalette);
			_needToUpdatePalette = false;
		}

		_loadingSavegame = false;

#if 0
		// Test: Draw the main actor's path
		if (_mainActorIndex != -1 && _actors[_mainActorIndex].pathResultCount > 0) {
			Actor *actor = &_actors[_mainActorIndex];
			for (int16 i = 0; i < actor->pathResultCount - 1; i++) {
				_screen->drawLine(
					actor->pathWalker2[i].x - _cameraX, actor->pathWalker2[i].y - _cameraY,
					actor->pathWalker2[i+1].x - _cameraX, actor->pathWalker2[i+1].y - _cameraY,
					250);
			}
		}
#endif

		_screen->update();
		_system->delayMillis(10);
	}

}

int16 PrisonerEngine::loadTextResource(Common::String &pakName, int16 pakSlot) {
	makeLanguageString(pakName);
	pakName.setChar('T', 2);
	return _res->load<TextResource>(pakName, pakSlot, 3);
}

void PrisonerEngine::resetGlobalScriptVars() {
	_globalScriptVars[0] = -1;
	for (uint i = 1; i < 250; i++)
		_globalScriptVars[i] = 0;
}

void PrisonerEngine::makeLanguageString(Common::String &value) {
	value.setChar(_languageChar, 0);
}

int16 PrisonerEngine::getGlobalScriptVar(int16 varIndex) {
	return _globalScriptVars[varIndex];
}

void PrisonerEngine::setGlobalScriptVar(int16 varIndex, int16 value) {
	_globalScriptVars[varIndex] = value;
}

int16 PrisonerEngine::getModuleScriptVar(int16 varIndex) {
	return _moduleScriptVars[varIndex];
}

void PrisonerEngine::setModuleScriptVar(int16 varIndex, int16 value) {
	_moduleScriptVars[varIndex] = value;
}

int16 PrisonerEngine::getSysVar(int16 varIndex) {
	switch (varIndex) {
	case 0:
	case 1:
	case 2:
	case 3:
		// Seem to be unused in POI
		return 0;
	case 4:
		return _prevSceneIndex;
	case 5:
		return _prevModuleIndex;
	case 6:
		return _selectedDialogKeywordIndex;
	case 7:
		return _currSceneIndex;
	case 8:
		return _currModuleIndex;
	case 9:
		return _inventoryItemIndex2;
	default:
		return 0;
	}
}

// Polls the events once per frame. A key or button that is pressed and released within
// the same poll stays down until the next poll, so quick taps and clicks aren't lost.
void PrisonerEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	bool keyPressed = false;
	uint16 buttonsPressed = 0;

	if (_keyReleasePending) {
		_keyState = Common::KEYCODE_INVALID;
		_keyReleasePending = false;
	}
	_buttonState &= ~_buttonReleasePending;
	_buttonReleasePending = 0;

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			this->_lastInputTicks = getTicks();
			_keyState = event.kbd.keycode;
			_keyAscii = event.kbd.ascii;
			_keyReleasePending = false;
			keyPressed = true;
			break;
		case Common::EVENT_KEYUP:
			if (keyPressed) {
				_keyReleasePending = true;
			} else {
				_keyState = Common::KEYCODE_INVALID;
				// The key is up, so a pending wait for its release is over
				_inpKeybWaitRelease = false;
				_inpKeybPulseRelease = false;
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			this->_lastInputTicks = getTicks();
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN: {
			uint16 button = event.type == Common::EVENT_LBUTTONDOWN ? kLeftButton : kRightButton;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_buttonState |= button;
			_buttonReleasePending &= ~button;
			buttonsPressed |= button;
			break;
		}
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP: {
			uint16 button = event.type == Common::EVENT_LBUTTONUP ? kLeftButton : kRightButton;
			if (buttonsPressed & button) {
				_buttonReleasePending |= button;
			} else {
				_buttonState &= ~button;
				if (_buttonState == 0) {
					_inpMouseWaitRelease = false;
					_inpMousePulseRelease = false;
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

uint32 PrisonerEngine::getTicks() {
	return _system->getMillis() / 10;
}

void PrisonerEngine::addDirtyRect(int16 x1, int16 y1, int16 x2, int16 y2, int16 flag) {
	// TODO
	// Just a dummy until the real code is done
}

void PrisonerEngine::playIntroVideos() {
	const EasterEggVideo introSequence[] = {
		{"vintro.mux", Common::KEYCODE_i},
		{"vcredits.mux", Common::KEYCODE_c},
		{"vtitre.mux", Common::KEYCODE_e}};

	_muxEasterEggCount = 0;
	_muxClearScreenBefore = true;
	_muxClearScreenAfter = true;

	for (const auto &entry : introSequence) {
		playMux(entry.filename);
		if (_muxEasterEggKey == entry.key)
			_muxEasterEggCount++;
	}
	if (_muxEasterEggCount == 3)
		playMux("vice.mux");
	_muxEasterEggCount = 0;
}

// The intro, credits and title videos play in a loop until a key or a mouse button is pressed.
// During a game the "best of" video plays too, and the game's music, sounds and voices are
// stopped meanwhile.
void PrisonerEngine::runScreensaver() {
	const char *const videos[] = {"vintro.mux", "vcredits.mux", "vtitre.mux", "vbestoff.mux"};
	const bool inGame = _inGame;
	const uint videoCount = inGame ? ARRAYSIZE(videos) : ARRAYSIZE(videos) - 1;

	if (inGame) {
		suspendMusics();
		suspendSounds();
		setVoicesMuted(true);
	}

	_screensaverRunning = true;
	_muxClearScreenBefore = true;
	_muxClearScreenAfter = true;
	bool aborted = false;
	while (!aborted && !shouldQuit()) {
		const uint32 roundStart = getTicks();
		for (uint i = 0; i < videoCount && !aborted && !shouldQuit(); i++) {
			playMux(videos[i]);
			aborted = _screensaverAborted;
		}
		// Missing video files would make this loop spin without ever reading input
		if (getTicks() - roundStart < 100)
			break;
	}
	_screensaverRunning = false;

	if (inGame) {
		resumeSounds();
		resumeMusics();
		setVoicesMuted(false);
	}
	resetFrameValues();
}

void PrisonerEngine::death() {
	_mainMenuRequested = true;
	_inGame = false;
	_newModuleIndex = -1;
	leaveScene();
	//resetDirtyRects();
	_screen->clear();
}

// Insert or Delete (scan codes 52h and 53h) keys act as the mouse buttons
// when playing with the keyboard (original: op_waitForInput)
bool PrisonerEngine::waitForInput() {
	return !_paletteTasks[3].active && (_buttonState != 0 || isLeftButtonKey(_keyState) || isRightButtonKey(_keyState));
}

bool PrisonerEngine::isLeftButtonKey(Common::KeyCode keyCode) const {
	return keyCode == Common::KEYCODE_INSERT || keyCode == Common::KEYCODE_KP0;
}

bool PrisonerEngine::isRightButtonKey(Common::KeyCode keyCode) const {
	return keyCode == Common::KEYCODE_DELETE || keyCode == Common::KEYCODE_KP_PERIOD;
}

// Called after scene changes, videos, loading and the menus. (resetScreensaverValues) restarts
// the stand-by time, so the screensaver doesn't start right after a long video or scene change.
void PrisonerEngine::resetFrameValues() {
	initFrameTime();
	_lastInputTicks = getTicks();
}

void PrisonerEngine::initFrameTime() {
	_lastFrameTime = getTicks();
	_frameTicks = 0;
}

void PrisonerEngine::updateFrameTime() {
	_frameTicks = getTicks() - _lastFrameTime;
	_lastFrameTime = getTicks();
}

void PrisonerEngine::resetAnimationFrameTicks() {
	_animationFrameTicks = 0;
}

void PrisonerEngine::updateAnimationFrameTicks() {
	if (_animationFrameTimeFlag) {
		_animationFrameTicks = 100;
	} else {
		_animationFrameTicks = _frameTicks;
	}
}

/* Input low-level */

void PrisonerEngine::initInput() {

	_buttonState = 0;
	_keyState = Common::KEYCODE_INVALID;
	_keyReleasePending = false;
	_buttonReleasePending = 0;
	_walkKey = Common::KEYCODE_INVALID;
	_walkKeyFirstPress = false;

	_inpKeybWaitRelease = false;
	_inpMouseWaitRelease = false;
	_inpKeybPulseRelease = false;
	_inpMousePulseRelease = false;
	_inpMouseDblClickLastClickTime = 0;
	_inpMouseDblClickTime = 0;

}

void PrisonerEngine::getInputStatus(Common::KeyCode &keyState, uint16 &buttonState) {

	buttonState = _buttonState;
	if (_inpMouseWaitRelease) {
		if (buttonState == 0) {
			_inpMouseWaitRelease = false;
			_inpMousePulseRelease = false;
		} else if (_inpMousePulseRelease) {
			// TODO: Only used in the menu
		} else {
			buttonState = 0;
		}
	} else if (buttonState != 0) {
		// TODO: Only used in the menu
		/* TODO...
		_inpMouseDblClickTime = getTicks();
		if (_inpMouseDblClickLastClickTime - _inpMouseDblClickTime < 50)
			buttonState |= buttonState << 2;
		_inpMouseDblClickLastClickTime = getTicks();
		*/
	}

	keyState = _keyState;
	// Insert and Delete act as the left and right mouse buttons. A key press counts as
	// one click, the key has to be released before it clicks again.
	if (!_inpMouseWaitRelease && !_inpKeybWaitRelease && (isLeftButtonKey(keyState) || isRightButtonKey(keyState))) {
		buttonState |= isLeftButtonKey(keyState) ? kLeftButton : kRightButton;
		inpKeybSetWaitRelease(true);
	}
	if (_inpKeybWaitRelease) {
		if (keyState == Common::KEYCODE_INVALID) {
			_inpKeybWaitRelease = false;
			_inpKeybPulseRelease = false;
		} else if (_inpKeybPulseRelease && getTicks() - _inpKeybPulseTime > _inpKeybPulseTicks) {
			_inpKeybPulseTime = getTicks();
			_inpKeybPulseTicks = 10;
		} else
			keyState = Common::KEYCODE_INVALID;
	}

}

void PrisonerEngine::inpSetWaitRelease(bool value) {
	inpMouseSetWaitRelease(value);
	inpKeybSetWaitRelease(value);
}

void PrisonerEngine::inpMouseSetWaitRelease(bool value) {
	_inpMouseWaitRelease = value;
	_inpMousePulseRelease = false;
}

void PrisonerEngine::inpKeybSetWaitRelease(bool value) {
	_inpKeybWaitRelease = value;
	_inpKeybPulseRelease = false;
}

/* Input */

int16 PrisonerEngine::handleInput(int16 x, int16 y) {

	Common::KeyCode keyState;
	uint16 buttonState;

	if (_optionsTextTicksCounter > 0 && getTicks() >= _optionsTextTicks + _optionsTextTicksCounter) {
		_optionsTextTicksCounter = -1;
		_screen->fillRect(0, 0, 639, 81, 0);
		addDirtyRect(0, 0, 640, 82, 1);
	}

	getInputStatus(keyState, buttonState);

	if (_queuedZoneAction.used == 1)
		checkQueuedZoneAction();

	if (!_mainMenuRequested && x - _cameraX > 610 && y - _cameraY <= 82) {
		// Clicking the top right corner opens the main menu
		if (_inGame && buttonState != 0) {
			inpMouseSetWaitRelease(true);
			openMainMenu();
		}
		return 0;
	}

	if (_inventoryBarEnabled && _updateDirtyRectsFlag) {
		if (_inventoryBarFlag &&
			(_currMouseCursor == kCursorDefault || _currMouseCursor == kCursorItem) &&
			(y - _cameraY < 82 || _inventoryWarpMouse) &&
			_inventoryItemsCount > 0) {
			debug("inv!");
			handleInventoryInput();
			return 0;
		} else if (y - _cameraY >= 82)
			_inventoryBarFlag = true;
	}

	// The original save/load screens handle all keys themselves (Escape cancels), but not those
	// of their delete confirmation box
	if (_mainMenuRequested && !_isDialogMenuShowing && (_selectedMenuIndex == kSavePanel || _selectedMenuIndex == kLoadPanel)) {
		handleSaveLoadInput();
		return 0;
	}

	if (_isDialogMenuShowing) {
		handleDialogMenuInput();
		return 0;
	}

	if (_inGame) {
		// The display settings screen handles Escape itself (cancel)
		if (keyState == Common::KEYCODE_ESCAPE && !(_mainMenuRequested && (_selectedMenuIndex == kOptGfxPanel || _selectedMenuIndex == kDivOptPanel ||
			_selectedMenuIndex == kVolOptPanel))) {
			inpKeybSetWaitRelease(true);
			if (!_mainMenuRequested) {
				openMainMenu();
			} else {
				// TODO CLEAR MENU (MAYBE VIA DIRTY RECTS?)
				this->resumeGame();
			}
		} else if (keyState == Common::KEYCODE_F2) {
			inpKeybSetWaitRelease(true);
			openSaveLoadScreen(true);
		} else if (keyState == Common::KEYCODE_F3) {
			inpKeybSetWaitRelease(true);
			openSaveLoadScreen(false);
		} else if (keyState == Common::KEYCODE_F4) {
			inpKeybSetWaitRelease(true);
			openOptionsMenuFromGame();
		} else if (keyState == Common::KEYCODE_p && !_mainMenuRequested) {
			inpKeybSetWaitRelease(true);
			runPauseScreen();
		}
	}

	// The arrow keys (original handleInput). With Shift held and the default, door or item cursor
	// they walk the main character (updateActors does the walking); otherwise they move the
	// pointer by one pixel each frame.
	if (!_mainMenuRequested && (keyState == Common::KEYCODE_LEFT || keyState == Common::KEYCODE_RIGHT ||
		keyState == Common::KEYCODE_UP || keyState == Common::KEYCODE_DOWN)) {
		const bool shiftHeld = (_eventMan->getModifierState() & Common::KBD_SHIFT) != 0;
		if (shiftHeld && (_currMouseCursor == kCursorDefault || _currMouseCursor == kCursorDoor ||
			_currMouseCursor == kCursorItem)) {
			_walkKey = keyState;
			_walkKeyFirstPress = !_inputFlag;
			_inputFlag = true;
			_queuedZoneAction.used = 0;
		} else {
			_mouseX += (keyState == Common::KEYCODE_RIGHT && _mouseX < 639) - (keyState == Common::KEYCODE_LEFT && _mouseX > 0);
			_mouseY += (keyState == Common::KEYCODE_DOWN && _mouseY < 479) - (keyState == Common::KEYCODE_UP && _mouseY > 0);
			_system->warpMouse(_mouseX, _mouseY);
			x = _cameraX + _mouseX;
			y = _cameraY + _mouseY;
		}
		keyState = Common::KEYCODE_INVALID;
	}

	if (_mainMenuRequested) {
		handleMainMenuInput();
		return 0;
	}

	//debug("_currMouseCursor = %d", _currMouseCursor);

	if (keyState == Common::KEYCODE_KP5 && _mainActorIndex != -1 && _mainActorValid &&
		(_currMouseCursor == kCursorDefault || _currMouseCursor == kCursorDoor || _currMouseCursor == kCursorItem)) {
		inpKeybSetWaitRelease(true);
		_queuedZoneAction.used = 0;
		setActorLookInDirection(_mainActorIndex);
	}

	switch (_currMouseCursor) {

	case kCursorDefault:
		if (buttonState != 0) {
			bool doWalk = false, checkMore = true;

			// Walk if clicked inside the mainActor's zone
			if (_mainActorIndex != -1 && _zoneIndexAtMouse != -1 &&
				_zones[_zoneIndexAtMouse].actorIndex == _mainActorIndex) {
				doWalk = true;
			}

			if (checkMore && !doWalk) {
				if ((_zoneIndexAtMouse == -1 || !_zones[_zoneIndexAtMouse].hasText) && _mainActorIndex != -1) {
					if (checkZoneAction(2) != -1) {
						checkMore = false;
					} else if (_zoneIndexAtMouse == -1 || !_zones[_zoneIndexAtMouse].hasText || _mainActorIndex == -1) {
						doWalk = true;
					}
				}
			}

			if (checkMore && !doWalk && (buttonState & kLeftButton)) {
				if (checkZoneAction(1) != -1 || _currZoneActionIndex != -1 || _mainActorIndex == -1) {
					checkMore = false;
				} else {
					doWalk = true;
				}
			}

			if (checkMore && !doWalk && (buttonState & kRightButton)) {
				if (checkZoneAction(6) != -1 || _currZoneActionIndex != -1 || _mainActorIndex == -1) {
					checkMore = false;
				} else {
					doWalk = true;
				}
			}

			if (doWalk) {
				_queuedZoneAction.used = 0;
				_inputFlag = false;
				actorWalkToPoint(_mainActorIndex, _cameraX + _mouseX, _cameraY + _mouseY);
			}

			inpMouseSetWaitRelease(true);
		}
		break;

	case kCursorDoor:
		if (buttonState != 0) {
			checkZoneAction(3);
			inpMouseSetWaitRelease(true);
		}
		break;

	case kCursorRead:
	case kCursorTalk:
		if (buttonState != 0 || _autoAdvanceScreenTexts) {
			advanceScreenTexts();
			inpMouseSetWaitRelease(true);
		}
		break;

	case kCursorDialog:
		if (buttonState != 0) {
			if (!((buttonState & kLeftButton) && isPointInDialogRect(_mouseX, _mouseY))) {
				_selectedDialogKeywordIndex = -1;
			}
			_dialogRunning = false;
			inpMouseSetWaitRelease(true);
			_screen->fillRect(0, 0, 639, 81, 0);
			_screen->fillRect(0, 398, 639, 479, 0);
			addDirtyRect(0, 0, 640, 82, 1);
			addDirtyRect(0, 398, 640, 82, 1);
		}
		break;

	case kCursorItem:
		if (buttonState != 0) {
			if (buttonState & kLeftButton) {
				if (_zoneIndexAtMouse == -1 && _mainActorIndex != -1) {
					if (checkZoneAction(2) == -1) {
						_queuedZoneAction.used = false;
						_inputFlag = false;
						actorWalkToPoint(_mainActorIndex, _cameraX + _mouseX, _cameraY + _mouseY);
					}
				} else {
					checkZoneAction(9);
				}
			} else if (buttonState & kRightButton) {
				_inventoryItemCursor = -1;
			}
			inpMouseSetWaitRelease(true);
		}
		break;

	default:
		//debug(0, "Unhandled _currMouseCursor: %d", _currMouseCursor);
		break;

	}

	return 0;
}

void PrisonerEngine::setUserInput(bool enabled) {
	if (enabled) {
		if (_lockUserInputRefCounter > 0)
			_lockUserInputRefCounter--;
	} else {
		_lockUserInputRefCounter++;
	}
	_inputFlag = false;
}

void PrisonerEngine::playMux(Common::String filename) {
	MuxPlayer *muxPlayer;

	debug("playMux('%s')", filename.c_str());
	// Set char for the SVGA videos
	filename.setChar('S', 0);
	if (!Common::File::exists(Common::Path(filename))) {
		// The GOG release keeps the videos with speech (the endings) in the VIDEO folder, with the
		// language letter after the S: SAFINAL1.MUX instead of A_VIDEO/SFINAL1.MUX
		Common::String languageFilename = filename;
		languageFilename.insertChar(tolower(_languageChar), 1);
		if (Common::File::exists(Common::Path(languageFilename)))
			filename = languageFilename;
		else
			warning("playMux: Video '%s' not found", filename.c_str());
	}
	_screensaverAborted = false;
	if (_moduleScriptCalled) {
		_newModuleIndex = -1;
		leaveScene();
	}
	if (!_screensaverRunning) {
		suspendMusics();
		suspendSounds();
		setVoicesMuted(true);
	}
	CursorMan.showMouse(false);
	// TODO: resetDirtyRects();
	if (_muxClearScreenBefore) {
		_screen->clear();
		addDirtyRect(0, 0, 640, 480, 1);
		// TODO: drawDirtyRects(0);
	}
	muxPlayer = new MuxPlayer(this);
	muxPlayer->open(filename.c_str());
	_isPlayingMux = true;
	muxPlayer->play();
	_isPlayingMux = false;
	muxPlayer->close();
	delete muxPlayer;
	// TODO: resetDirtyRects();
	if (_muxClearScreenAfter) {
		_backgroundFlag = 0;
		_screen->clear();
		addDirtyRect(0, 0, 640, 480, 1);
		// TODO: drawDirtyRects(0);
	}
	if (!_screensaverRunning) {
		resumeSounds();
		resumeMusics();
		setVoicesMuted(false);
	}
	if (_moduleScriptCalled) {
		_mainMenuRequested = true;
		_inGame = false;
		// TODO: resetDirtyRects();
		_screen->clear();
	}
	_screen->setFullPalette(_scenePalette);
	CursorMan.showMouse(true);
	resetFrameValues();
}

bool PrisonerEngine::handleMuxInput() {
	bool aborted = false;
	Common::KeyCode keyState;
	uint16 buttonState;
	updateEvents();
	getInputStatus(keyState, buttonState);
	//debug("keyState = %d; buttonState = %d", keyState, buttonState);
	if (_loadingSavegame || shouldQuit())
		aborted = true;
	else if (_muxEasterEggCount != 3 && (keyState != Common::KEYCODE_INVALID || buttonState != 0)) {
		_muxEasterEggKey = keyState;
		aborted = true;
		// Input also ends the screensaver
		_screensaverAborted = true;
		inpSetWaitRelease(true);
	}
	return aborted;
}

void PrisonerEngine::playMuxSoon(Common::String &filename, bool clearScreenAfter, bool clearScreenBefore) {
	_muxFilename = filename;
	_muxClearScreenAfter = clearScreenAfter;
	_muxClearScreenBefore = clearScreenBefore;
	_needToPlayMux = true;
}

void PrisonerEngine::pauseGame() {
	updateActiveMenuItems();

	debug("pause game");
	_pauseToken = pauseEngine();
}

void PrisonerEngine::resumeGame() {
	_mainMenuRequested = false;
	_selectedMenuIndex = kMainPanel;
	this->_screen->clear();
	_pauseToken.clear();
	resetFrameValues();
}

void PrisonerEngine::runPauseScreen() {
	const int16 kPictureWidth = 320;
	const int16 kPictureHeight = 158;
	const int16 kPictureX = 162;
	const int16 kPictureY = 170;
	const int16 kGameAreaY = 82;

	AnimationResource *pausePanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[kPausePanel]);
	Graphics::Surface *screen = _screen->getScreen();

	CursorMan.showMouse(false);

	// Every second pixel of every second line of the game area
	Common::Array<byte> picture;
	picture.resize(kPictureWidth * kPictureHeight);
	for (int16 y = 0; y < kPictureHeight; y++) {
		const byte *src = (const byte *)screen->getBasePtr(0, kGameAreaY + y * 2);
		for (int16 x = 0; x < kPictureWidth; x++)
			picture[y * kPictureWidth + x] = src[x * 2];
	}

	suspendMusics();
	PauseToken pauseToken = pauseEngine();

	byte savedPalette[768];
	memcpy(savedPalette, _effectPalette, 768);
	byte greyPalette[768];
	for (int i = 0; i < 256; i++) {
		const byte grey = (_effectPalette[i * 3] + _effectPalette[i * 3 + 1] + _effectPalette[i * 3 + 2]) / 3;
		greyPalette[i * 3] = greyPalette[i * 3 + 1] = greyPalette[i * 3 + 2] = grey;
	}

	fadeInOutColor(greyPalette, 1, 255, 255, 255, 0);
	_screen->setFullPalette(_effectPalette);
	_screen->clear();
	_screen->drawAnimationElement(pausePanel, 0, 67, 77, 0);
	for (int16 y = 0; y < kPictureHeight; y++)
		memcpy(screen->getBasePtr(kPictureX, kPictureY + y), &picture[y * kPictureWidth], kPictureWidth);
	_screen->update();

	// Fade from white to grey, one step per vertical retrace in the original
	for (int16 position = 0; position < 255 && !shouldQuit(); position += 4) {
		fadeInOutColor(greyPalette, 1, 255, 255, 255, position);
		_screen->setFullPalette(_effectPalette);
		_screen->update();
		updateEvents();
		_system->delayMillis(14);
	}
	memcpy(_effectPalette, greyPalette, 768);
	_screen->setFullPalette(_effectPalette);

	inpSetWaitRelease(true);
	while (!shouldQuit()) {
		Common::KeyCode keyState;
		uint16 buttonState;
		updateEvents();
		getInputStatus(keyState, buttonState);
		if (keyState != Common::KEYCODE_INVALID || buttonState != 0)
			break;
		_screen->update();
		_system->delayMillis(10);
	}

	_screen->clear();
	pauseToken.clear();
	resumeMusics();
	memcpy(_effectPalette, savedPalette, 768);
	_screen->setFullPalette(_effectPalette);
	inpSetWaitRelease(true);
	CursorMan.showMouse(true);
	updateScreen(true, -1, -1);
	resetFrameValues();
}

void PrisonerEngine::requestAutoSave(Common::String &pakName, int16 pakSlot, Common::String &identifier) {
	_autoSaveRequested = true;
	_autoSavePakName = pakName;
	_autoSavePakSlot = pakSlot;
	_autoSaveIdentifier = identifier;
}

void PrisonerEngine::performAutoSave() {

	int16 textResourceCacheSlot = loadTextResource(_autoSavePakName, _autoSavePakSlot);
	TextResource *textResource = _res->get<TextResource>(textResourceCacheSlot);
	const Common::String savegameDescription = textResource->getText(_autoSaveIdentifier)->getChunkLineString(0, 0);
	Common::String waitMessageIdentifier = "S_SCE";
	const Common::String waitMessage = getGlobalText(waitMessageIdentifier);

	debug("AUTOSAVE: [%s] (%s)", savegameDescription.c_str(), waitMessage.c_str());

	setFontColors(_textFont, _zoneFontColor.outlineColor, _zoneFontColor.inkColor);
	_screen->fillRect(0, 398, 639, 479, 0);
	drawTextEx(0, 639, 398, 479, waitMessage);
	addDirtyRect(0, 398, 540, 82, 1);
	_screen->update();

	saveGameState(getAutosaveSlot(), savegameDescription, true);

}

void PrisonerEngine::getInteractMessage(Common::String &pakName, int16 pakSlot, Common::String &identifier,
	int16 &outResourceCacheSlot, Common::String &outIdentifier) {

	outResourceCacheSlot = loadTextResource(pakName, pakSlot);
	TextResource *textResource = _res->get<TextResource>(outResourceCacheSlot);

	if (_inventoryItemIndex2 == -1) {
		outIdentifier = identifier + "LOOK";
	} else if (_inventoryItemIndex2 == -6) {
		outIdentifier = identifier + "ACT";
	} else {
		outIdentifier = Common::String::format("%sO%02d", identifier.c_str(), _inventoryItemCursor);
		if (textResource->getIndex(outIdentifier) < 0)
			outIdentifier = identifier + "OBJ";
	}

	if (textResource->getIndex(outIdentifier) < 0) {
		Common::String iceTxtPakName = "F_ICETXT";
		Common::String maxIdentifier, identifierPrefix;
		_res->unload(outResourceCacheSlot);
		outResourceCacheSlot = loadTextResource(iceTxtPakName, 1);
		textResource = _res->get<TextResource>(outResourceCacheSlot);
		if (_inventoryItemIndex2 == -1) {
			identifierPrefix = "DLOOK";
			maxIdentifier = "MAXLOOK";
		} else if (_inventoryItemIndex2 == -6) {
			identifierPrefix = "DACT";
			maxIdentifier = "MAXACT";
		} else {
			identifierPrefix = "DOBJ";
			maxIdentifier = "MAXOBJ";
		}

		int defaultIndex = 0;

		if (textResource->getIndex(maxIdentifier) >= 0) {
			const Common::String maxCountStr = textResource->getText(maxIdentifier)->getChunkLineString(0, 0);
			int maxCount = atoi(maxCountStr.c_str()) - 1;
			defaultIndex = _rnd->getRandomNumber(maxCount);
		}

		outIdentifier = Common::String::format("%s%02d", identifierPrefix.c_str(), defaultIndex);

	}

}

// TODO: Move MIDI code to own class

void PrisonerEngine::initializeMidi() {

	MidiDriver::DeviceHandle midiDriver = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	bool native_mt32 = ((midiDriver & MDT_PREFER_MT32) || ConfMan.getBool("native_mt32"));
	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
	_midi = new MidiPlayer(this, driver);
	_midi->setGM(true);
	_midi->setNativeMT32(native_mt32);

}

void PrisonerEngine::shutdownMidi() {
	delete _midi;
}

void PrisonerEngine::setBackgroundObjects(Common::String &pakName, int16 pakSlot) {

	_backgroundObjectsResourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 2);
	AnimationResource *animationResource = _res->get<AnimationResource>(_backgroundObjectsResourceCacheSlot);

	if (animationResource->_anims.size() != 1 || animationResource->_anims[0]->frames.size() == 0)
		return;

	AnimationElement *element = animationResource->_elements[animationResource->_anims[0]->frames[0]->elementIndex];

	for (uint i = 0; i < element->commands.size() && i < (uint)kMaxBackgroundObjects; i++) {
		AnimationCommand *command = element->commands[i];
		if (command->cmd == kActElement || command->cmd == kActCelSprite) {
			int16 backgroundObjectIndex = _backgroundObjects.getFreeSlot();
			BackgroundObject *backgroundObject = &_backgroundObjects[backgroundObjectIndex];
			backgroundObject->used = 1;
			backgroundObject->va2Command = command->cmd;
			backgroundObject->celIndex = command->argAsInt16();
			backgroundObject->x = command->points[0].x;
			backgroundObject->y = command->points[0].y;
			backgroundObject->animationResource = animationResource;
			// TODO: Init bounds: Not used in POI
		}
	}

	_backgroundObjectsDrawQueue.clear();

	for (int16 backgroundObjectIndex = 0; backgroundObjectIndex < kMaxBackgroundObjects; backgroundObjectIndex++) {
		BackgroundObject *backgroundObject = &_backgroundObjects[backgroundObjectIndex];
		if (backgroundObject->used == 1) {
			_backgroundObjectsDrawQueue.push_back(backgroundObject);
		}
	}

	Common::sort(_backgroundObjectsDrawQueue.begin(), _backgroundObjectsDrawQueue.end(), CompareBackgroundObjectByY());

}

void PrisonerEngine::unloadBackgroundObjects() {
	if (_backgroundObjectsResourceCacheSlot != -1) {
		_res->unload(_backgroundObjectsResourceCacheSlot);
		_backgroundObjectsResourceCacheSlot = -1;
	}
	_backgroundObjects.clear();
	_backgroundObjectsDrawQueue.clear();
}

void PrisonerEngine::drawSprites(int16 xOffs, int16 yOffs) {

	uint backgroundObjectIndex = 0;
	uint actorSpriteIndex = 0;
	BackgroundObject *backgroundObject;
	ActorSprite *actorSprite;

	Common::sort(_actorSpriteDrawQueue.begin(), _actorSpriteDrawQueue.end(), CompareActorSpriteByY());

	do {

		backgroundObject = backgroundObjectIndex < _backgroundObjectsDrawQueue.size() ?
			_backgroundObjectsDrawQueue[backgroundObjectIndex] : NULL;
		actorSprite = actorSpriteIndex < _actorSpriteDrawQueue.size() ?
			_actorSpriteDrawQueue[actorSpriteIndex] : NULL;

		if (actorSprite && (!backgroundObject || actorSprite->y <= backgroundObject->y)) {
			if (actorSprite->actorIndex != _lipSyncActorIndex)
				drawActorSprite(xOffs, yOffs, actorSprite);
			actorSpriteIndex++;
		} else if (backgroundObject && (!actorSprite || backgroundObject->y <= actorSprite->y)) {
			drawBackgroundObject(xOffs, yOffs, backgroundObject);
			backgroundObjectIndex++;
		}

	} while (backgroundObject || actorSprite);

}

void PrisonerEngine::buildActorSpriteDrawQueue() {
	_actorSpriteDrawQueue.clear();
	for (int16 i = 0; i < kMaxActors; i++) {
		if (_actorSprites[i].used == 1) {
			_actorSpriteDrawQueue.push_back(&_actorSprites[i]);
		}
	}
}

void PrisonerEngine::drawBackgroundObject(int16 xOffs, int16 yOffs, BackgroundObject *backgroundObject) {
	if (backgroundObject->used == 1 && backgroundObject->va2Command == kActElement) {
		_screen->drawAnimationElement(backgroundObject->animationResource,
			backgroundObject->celIndex, backgroundObject->x - xOffs, backgroundObject->y - yOffs, 0);
	}
}

void PrisonerEngine::drawActorSprite(int16 xOffs, int16 yOffs, ActorSprite *actorSprite) {
	if (actorSprite->scale < 100) {
		AnimationCommand *cmd = actorSprite->animationResource->_elements[actorSprite->elementIndex]->commands[0];
		if (actorSprite->scale > 0 && cmd->cmd == kActCelSprite) {
			int16 celIndex = cmd->argAsInt16() & 0x0FFF;
			AnimationCel *cel = actorSprite->animationResource->_cels[celIndex];
			cel->scale = actorSprite->scale;
			_screen->drawAnimationElement(actorSprite->animationResource,
				actorSprite->elementIndex, actorSprite->x - xOffs, actorSprite->y - yOffs, 0);
			cel->scale = 100;
		}
	} else {
		_screen->drawAnimationElement(actorSprite->animationResource,
			actorSprite->elementIndex, actorSprite->x - xOffs, actorSprite->y - yOffs, 0);
	}
}

bool PrisonerEngine::backupScreen() {
	if (!_screenBackedup) {
		_screenBackedup = true;
		_screen->setFullPalette(_scenePalette);
		memcpy(_screenBackupSurface->getPixels(), _screen->getScreen()->getPixels(), 640 * 480);
		return false;
	} else
		return true;
}

// TODO: Change ignorePalette name
void PrisonerEngine::restoreScreen(bool ignorePalette) {
	if (_screenBackedup) {
		_screenBackedup = ignorePalette;
		memcpy(_screen->getScreen()->getPixels(), _screenBackupSurface->getPixels(), 640 * 480);
		addDirtyRect(0, 0, 640, 480, 1);
		if (!ignorePalette) {
			_screen->setFullPalette(_effectPalette);
		}
	}
}

int16 PrisonerEngine::calcDirection(int16 x1, int16 y1, int16 x2, int16 y2) {
	int16 direction = -1;
	if (x1 != x2) {
		int16 deltaX = ABS(x2 - x1);
		int16 deltaY = ABS(y2 - y1);
		if (deltaX >= deltaY * 6) {
			if (x2 >= x1)
				direction = 2;
			else
				direction = 6;
		} else if (deltaX < deltaY) {
			if (y2 >= y1)
				direction = 4;
			else
				direction = 0;
		} else if (y2 >= y1) {
			if (x2 >= x1)
				direction = 3;
			else
				direction = 5;
		} else {
			if (x2 >= x1)
				direction = 1;
			else
				direction = 7;
		}
	} else if (y1 != y2) {
		if (y2 > y1)
			direction = 4;
		else
			direction = 0;
	}
	return direction;
}

void PrisonerEngine::setDefaultTextDisplayColors() {
	_screenTextFontColor.outlineColor = 131;
	_screenTextFontColor.inkColor = 124;
	_zoneFontColor.outlineColor = 32;
	_zoneFontColor.inkColor = 43;
	_inventoryScreenTextFontColor.outlineColor = 32;
	_inventoryScreenTextFontColor.inkColor = 110;
	_inventoryFontColor.outlineColor = 32;
	_inventoryFontColor.inkColor = 47;
	_dialogFontColor.outlineColor = 56;
	_dialogFontColor.inkColor = 63;
	_dialogHoverFontColor.outlineColor = 80;
	_dialogHoverFontColor.inkColor = 79;
}

void PrisonerEngine::setTextDisplayColor(int16 textDisplayNum, int16 outlineColor, int16 inkColor) {
	#define CASE(NUM, S) case NUM: S.outlineColor = outlineColor; S.inkColor = inkColor; break;
	switch (textDisplayNum) {
		CASE(0, _dialogFontColor);
		CASE(1, _zoneFontColor);
		CASE(2, _screenTextFontColor);
		CASE(3, _dialogHoverFontColor);
		CASE(4, _inventoryScreenTextFontColor);
		CASE(5, _inventoryFontColor);
	}
	#undef CASE
}

} // End of namespace Prisoner
