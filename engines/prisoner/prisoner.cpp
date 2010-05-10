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
 * $URL$
 * $Id$
 *
 */

#include "common/algorithm.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "engines/util.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "prisoner/prisoner.h"
#include "prisoner/kroarchive.h"
#include "prisoner/midi.h"
#include "prisoner/muxplayer.h"
#include "prisoner/path.h"
#include "prisoner/resourcemgr.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"
#include "prisoner/scriptops.h"

namespace Prisoner {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings prisonerSettings[] = {
	{"prisoner", "Prisoner of Ice game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

PrisonerEngine::PrisonerEngine(OSystem *syst, const PrisonerGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = prisonerSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "prisoner");

	/*
	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);
	*/

}

PrisonerEngine::~PrisonerEngine() {
	delete _rnd;
}

Common::Error PrisonerEngine::run() {

	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(false);
	_system->initSize(640, 480);
	_system->endGFXTransaction();

	_languageChar = 'E';
	_currModuleIndex = 2;

	_cameraX = 0;
	_cameraY = 0;

	_screen = new Screen(this);
	_resLoader = new PrisonerResourceLoader();
	_res = new ResourceManager(_resLoader);
	_scriptOpcodes = new ScriptOpcodes(this);
	_scriptOpcodes->setupOpcodes();
	_pathSystem = new PathSystem(this);

	_screenBackupSurface = new Graphics::Surface();
	_screenBackupSurface->create(640, 480, 1);
	_screenBackedup = false;

	_screen->initPaletteTransTable(65);

//	initializeMidi();

#if 0
	MuxPlayer mux(_system, _mixer);
	mux.open("Vin1.mux");
	mux.play();
	mux.close();
#endif

#if 0
	Common::String n = "E_X02R24";
	int16 r = _res->load<LipSyncSoundResource>(n, 2, 16);
#endif

#if 0
	Common::String pakName = "E_TCETXT";
	_res->dump(pakName, 1, 3);
#endif

#if 0
	Common::String pakName = "MUS02";
	int16 slot = _res->load<MidiResource>(pakName, 0, 13);
	_midi->playMusic(_res->get<MidiResource>(slot), 200, false);
#endif

#if 0
	Common::String pakName = "SA02";
	int16 slot = _res->load<AnimationResource>(pakName, 0, 11);
	AnimationResource *a = _res->get<AnimationResource>(slot);
	AnimationCel *cel = a->_cels[0];
	FILE *x = fopen("dump.0", "wb");
	fwrite(cel->data, cel->dataSize, 1, x);
	fclose(x);
	AnimationCel outCel;
	buildScaledSprite(*cel, outCel, 25);
	x = fopen("dump.1", "wb");
	fwrite(outCel.data, outCel.dataSize, 1, x);
	fclose(x);
#endif

#if 1
	Common::String n;

	// TODO: Later read font def from config data
	{
		Common::String pakName = "S_FONT";
		_menuFont = addFont(pakName, 0, 2, 2);
		_textFont = addFont(pakName, 1, 96, 255);
	}

	setFontDefaultColors();

	initInput();

	_system->showMouse(true);
	loadMouseCursors();

	_talkieEnabled = false;

	_menuMouseCursorActive = false;
	_menuMouseCursor = -1;
	_updateDirtyRectsFlag = true;
	_autoSaveRequested = false;
	_dialogRunning = false;
	_screenTextShowing = false;
	_userInputCounter = 0;
	_zoneMouseCursorActive = false;
	_inventoryItemCursor = -1;
	_currMouseCursor = -1;
	_currAnimatedMouseCursor = -1;

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

	for (uint i = 0; i < 250; i++)
		_globalScriptVars[i] = 0;
	_globalScriptVars[0] = -1;

	for (uint i = 0; i < 300; i++)
		_moduleScriptVars[i] = 0;
	_moduleScriptVars[0] = -1;

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
	_talkieSpeechDataPlayNow = false;

	/* Scene init */
	_enterSceneScriptIndex = -1;
	_enterSceneScriptProgramIndex = -1;
	_leaveSceneScriptIndex = -1;
	_leaveSceneScriptProgramIndex = -1;

	/* LipSync init */
	_lipSyncScriptNumber = -1;
	_lipSyncActorIndex = -1;
	_lipSyncResourceCacheSlot = -1;

	/* Inventory init */
	_inventoryBarEnabled = false;
	_inventoryBarFlag = false;

	_cameraFocusActor = false;
	_cameraFollowsActorIndex = -1;

	_updateScreenValue = true;

	_currModuleIndex = -1;
	_currSceneIndex = -1;

	_newModuleIndex = 2;
	_newSceneIndex = 39;

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

#if 1
	_newModuleIndex = 12;
	_newSceneIndex = 1;//script opcode 21d9b etc. -> OK; skull room
#endif

#if 0
	_newModuleIndex = 2;
	_newSceneIndex = 33;//radar
#endif

	{
		// TODO: Move to init function
		// TODO: Load pakName/slot from exe etc.
		Common::String pakName = "S_PANEL";
		_inventoryBoxResourceCacheSlot = _res->load<AnimationResource>(pakName, 12, 11);
	}

	// Test-Main-Loop
	bool done = false;
	while (!done) {
	//_moduleScriptVars[49] = 2;

		updateEvents();

		updateFrameTime();
		updateAnimationFrameTicks();
		updateActors();
		runScripts(kSceneScriptProgram);
		updateModuleScript();
		updateScreen(true, _cameraX + _mouseX, _cameraY + _mouseY);
		handleInput(_cameraX + _mouseX, _cameraY + _mouseY);
		checkForSceneChange();
		updateMouseCursor();
		updateMouseCursorAnimation();

		if (_needToUpdatePalette) {
			_screen->setFullPalette(_effectPalette);
			_needToUpdatePalette = false;
		}

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

	{
		// TODO: Move to shutdown function
		_res->unload(_inventoryBoxResourceCacheSlot);
		_inventoryBoxResourceCacheSlot = -1;
	}

#endif

//	shutdownMidi();
	delete _screenBackupSurface;
	delete _pathSystem;
	delete _scriptOpcodes;
	delete _res;
	delete _resLoader;

	return Common::kNoError;
}

int16 PrisonerEngine::loadTextResource(Common::String &pakName, int16 pakSlot) {
	makeLanguageString(pakName);
	pakName.setChar('T', 2);
	return _res->load<TextResource>(pakName, pakSlot, 3);
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

void PrisonerEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_buttonState |= kLeftButton;
			break;
		case Common::EVENT_LBUTTONUP:
			_buttonState &= ~kLeftButton;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_buttonState |= kRightButton;
			break;
		case Common::EVENT_RBUTTONUP:
			_buttonState &= ~kRightButton;
			break;
		case Common::EVENT_QUIT:
			_system->quit();
			break;
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

void PrisonerEngine::resetFrameValues() {
	initFrameTime();
	// TODO: Screensaver
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
		/* TODO...
		_inpMouseDblClickTime = getTicks();
		if (_inpMouseDblClickLastClickTime - _inpMouseDblClickTime < 50)
			buttonState |= buttonState << 2;
		_inpMouseDblClickLastClickTime = getTicks();
		*/
	}

	// TODO: The same for keyboard input...
	keyState = Common::KEYCODE_INVALID;

}

void PrisonerEngine::inpSetWaitRelease(bool value) {
	inpMouseSetWaitRelease(value);
	// TODO: inpKeybSetWaitRelease(value);
}

void PrisonerEngine::inpMouseSetWaitRelease(bool value) {
	_inpMouseWaitRelease = value;
	_inpMousePulseRelease = false;
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

	if (x - _cameraX > 610 && y - _cameraY <= 82) {
		// TODO: Open main menu and return the menu result
		//debug("TODO: Open main menu and return the menu result");
		return 0;
	}

	if (_inventoryBarEnabled && _updateDirtyRectsFlag) {
		if (_inventoryBarFlag &&
			(_currMouseCursor == kCursorDefault || _currMouseCursor == kCursorItem) &&
			(y - _cameraY < 82 /*TODO: Flag?*/) &&
			_inventoryItemsCount > 0) {
			handleInventoryInput();
			return 0;
		} else if (y - _cameraY >= 82)
			_inventoryBarFlag = true;
	}

	//debug("_currMouseCursor = %d", _currMouseCursor);

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
			checkZoneAction(kCursorDoor);
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
			bool cancelDialog = false, dialogDone = false;
			if (buttonState & kLeftButton) {
				if (isPointInDialogRect(_mouseX, _mouseY)) {
					_dialogRunning = false;
					dialogDone = true;
					inpMouseSetWaitRelease(true);
				} else
					cancelDialog = true;
			} else if (buttonState & kRightButton)
				cancelDialog = true;
			if (cancelDialog) {
				_dialogRunning = false;
				dialogDone = true;
				inpMouseSetWaitRelease(true);
				_selectedDialogKeywordIndex = -1;
			}
			if (dialogDone) {
				_screen->fillRect(0, 0, 639, 81, 0);
				_screen->fillRect(0, 398, 639, 479, 0);
				addDirtyRect(0, 0, 640, 82, 1);
				addDirtyRect(0, 398, 640, 82, 1);
			}
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
		if (_userInputCounter > 0)
			_userInputCounter--;
	} else {
		_userInputCounter++;
	}
	_inputFlag = false;
}

void PrisonerEngine::playMux(Common::String &filename) {
	debug("playMux('%s')", filename.c_str());
#if 0
	MuxPlayer mux(_system, _mixer);
	mux.open(filename.c_str());
	mux.play();
	mux.close();
#endif
}

void PrisonerEngine::playMuxSoon(Common::String &filename, bool clearScreenAfter, bool clearScreenBefore) {
	_muxFilename = filename;
	_muxClearScreenAfter = clearScreenAfter;
	_muxClearScreenBefore = clearScreenBefore;
	_needToPlayMux = true;
}

void PrisonerEngine::requestAutoSave(Common::String &pakName, int16 pakSlot, Common::String &identifier) {
	_autoSaveRequested = true;
	_autoSavePakName = pakName;
	_autoSavePakSlot = pakSlot;
	_autoSaveIdentifier = identifier;
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
		outIdentifier = Common::String::printf("%sO%02d", identifier.c_str(), _inventoryItemCursor);
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

		outIdentifier = Common::String::printf("%s%02d", identifierPrefix.c_str(), defaultIndex);

	}

}

// TODO: Move MIDI code to own class

void PrisonerEngine::initializeMidi() {
	MidiDriverType midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
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
		// This scaled drawing code is kinda ugly...
		if (actorSprite->scale == 0)
			return;
		AnimationCommand *cmd = actorSprite->animationResource->_elements[actorSprite->elementIndex]->commands[0];
		if (cmd->cmd != kActCelSprite)
			return;
		int16 celIndex = cmd->argAsInt16() & 0x0FFF;
		AnimationCel *cel = actorSprite->animationResource->_cels[celIndex];
		AnimationCel scaledCel;
		buildScaledSprite(*cel, scaledCel, actorSprite->scale);
		actorSprite->animationResource->_cels[celIndex] = &scaledCel;
		_screen->drawAnimationElement(actorSprite->animationResource,
			actorSprite->elementIndex, actorSprite->x - xOffs, actorSprite->y - yOffs, 0);
		delete[] scaledCel.data;
		actorSprite->animationResource->_cels[celIndex] = cel;
	} else {
		_screen->drawAnimationElement(actorSprite->animationResource,
			actorSprite->elementIndex, actorSprite->x - xOffs, actorSprite->y - yOffs, 0);
	}
}

void PrisonerEngine::buildScaledSprite(AnimationCel &inCel, AnimationCel &outCel, int16 scale) {

	uint16 scaledWidth, widthIncr, widthMod;
	uint16 scaledHeight, heightIncr, heightMod;
	int widthErr, heightErr;

	scaledWidth = inCel.width * scale / 100;
	widthIncr = inCel.width / scaledWidth;
	widthMod = inCel.width - widthIncr * scaledWidth;

	scaledHeight = inCel.height * scale / 100;
	heightIncr = inCel.height / scaledHeight;
	heightMod = inCel.height - heightIncr * scaledHeight;

	outCel.flags |= 0x4000;
	outCel.width = scaledWidth;
	outCel.height = scaledHeight;
	outCel.dataSize = outCel.width * outCel.height;
	outCel.data = new byte[outCel.dataSize];

	byte *src = inCel.data;
	byte *dst = outCel.data;

	byte lineBuffer[640];
	int lineSkip = 1;

	heightErr = scaledHeight;

	while (scaledHeight > 0) {

		while (lineSkip--) {
			// Decompress the current pixel row
			memset(lineBuffer, 0, inCel.width);
			byte chunks = *src++;
			byte *lineBufferPtr = lineBuffer;
			while (chunks--) {
				byte skip = src[0];
				uint count = src[1] * 4 + src[2];
				src += 3;
				lineBufferPtr += skip;
				memcpy(lineBufferPtr, src, count);
				lineBufferPtr += count;
				src += count;
			}
			memset(lineBufferPtr, 0, *src++);
		}

		byte *lineBufferPtr = lineBuffer;
		widthErr = scaledWidth;
		for (int i = 0; i < scaledWidth; i++) {
			*dst++ = *lineBufferPtr;
			lineBufferPtr += widthIncr;
			if (widthErr <= 0) {
				lineBufferPtr++;
				widthErr += outCel.width;
			}
			widthErr -= widthMod;
		}

		lineSkip = heightIncr;
		if (heightErr <= 0) {
			lineSkip++;
			heightErr += outCel.height;
		}
		heightErr -= heightMod;

		scaledHeight--;

	}

}

bool PrisonerEngine::backupScreen() {
	if (!_screenBackedup) {
		_screenBackedup = true;
		_screen->setFullPalette(_scenePalette);
		memcpy(_screenBackupSurface->pixels, _screen->getScreen()->pixels, 640 * 480);
		return false;
	} else
		return true;
}

// TODO: Change ignorePalette name
void PrisonerEngine::restoreScreen(bool ignorePalette) {
	if (_screenBackedup) {
		_screenBackedup = ignorePalette;
		memcpy(_screen->getScreen()->pixels, _screenBackupSurface->pixels, 640 * 480);
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

void PrisonerEngine::setFontDefaultColors() {
	// TODO: More colors
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

} // End of namespace Prisoner
