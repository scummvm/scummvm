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

#include "igor/igor.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "igor/console.h"
#include "igor/detection.h"

#include "audio/mixer.h"

namespace Igor {

IgorEngine *g_engine;

IgorEngine::IgorEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																		   _gameDescription(gameDesc), _randomSource("Igor") {
	g_engine = this;

	_screenVGA = (uint8 *)malloc(320 * 200);
	for (int i = 0; i < 4; i++) {
		_facingIgorFrames[i] = (uint8 *)malloc(13500);
	}
	_screenLayer1 = (uint8 *)malloc(320 * 144);
	_screenLayer2 = (uint8 *)malloc(320 * 144);
	_screenTextLayer = (uint8 *)malloc(320 * 144);
	_screenTempLayer = (uint8 *)malloc(9996);
	_igorHeadFrames = (uint8 *)malloc(3696);
	_animFramesBuffer = (uint8 *)malloc(65535);
	_inventoryPanelBuffer = (uint8 *)malloc(9600 * 2);
	_inventoryImagesBuffer = (uint8 *)malloc(48000);
	_verbsPanelBuffer = (uint8 *)malloc(3840);

	DebugMan.addDebugChannel(kDebugEngine,   "Engine",   "Engine debug level");
	DebugMan.addDebugChannel(kDebugResource, "Resource", "Resource debug level");
	DebugMan.addDebugChannel(kDebugScreen,   "Screen",   "Screen debug level");
	DebugMan.addDebugChannel(kDebugWalk,     "Walk",     "Walk debug level");
	DebugMan.addDebugChannel(kDebugGame,     "Game",     "Game debug level");


	{ // hardcoded now
		_game.ovlFileName = "igor.exe";
		_game.sfxFileName = "igor.dat";
		_game.version = kIdSpaCD;
		_game.language = Common::ES_ESP; // Assuming 0 represents the default language
		_currentPart = 900;
	}

	// if (_game.flags & kFlagFloppy) {
	// 	_midiPlayer = new Midi(this);
	// } else {
	// 	_midiPlayer = 0;
	// }
}

IgorEngine::~IgorEngine() {
	free(_resourceEntries);
	// free(_soundOffsets);
	free(_screenVGA);
	for (int i = 0; i < 4; ++i) {
		free(_facingIgorFrames[i]);
	}
	free(_screenLayer1);
	free(_screenLayer2);
	free(_screenTextLayer);
	free(_screenTempLayer);
	free(_igorHeadFrames);
	free(_animFramesBuffer);
	free(_inventoryPanelBuffer);
	free(_inventoryImagesBuffer);
	free(_verbsPanelBuffer);

	// DebugMan.clearAllDebugChannels();

	// delete _midiPlayer;
	delete _screen;
}

uint32 IgorEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String IgorEngine::getGameId() const {
	return _gameDescription->gameId;
}

void IgorEngine::restart() {
	_screenVGAVOffset = 0;

	memset(&_gameState, 0, sizeof(_gameState));
	_nextTimer = 0;
	// _fastMode = false;
	_language = 0;

	memset(_walkData, 0, sizeof(_walkData));
	_walkCurrentPos = 0;
	_walkDataLastIndex = _walkDataCurrentIndex = 0;
	_walkCurrentFrame = 0;
	_walkDataCurrentPosX = _walkDataCurrentPosY = 0;
	_walkToObjectPosX = _walkToObjectPosY = 0;

	memset(&_currentAction, 0, sizeof(_currentAction));
	_currentAction.verb = kVerbWalk;
	_actionCode = 0;
	_actionWalkPoint = 0;
	memset(_inputVars, 0, sizeof(_inputVars));

	// _talkDelay = _talkSpeechCounter = _talkDelayCounter = 0;
	// memset(_dialogueTextsTable, 0, sizeof(_dialogueTextsTable));
	// _dialogueTextsStart = 0;
	// _dialogueTextsCount = 0;
	// _dialogueDirtyRectY = 0;
	// _dialogueDirtyRectSize = 0;
	// memset(_dialogueQuestions, 0, sizeof(_dialogueQuestions));
	// memset(_dialogueReplies, 0, sizeof(_dialogueReplies));
	// _dialogueEnded = false;
	// _dialogueChoiceSelected = 0;
	// memset(_dialogueInfo, 0, sizeof(_dialogueInfo));

	// memset(_objectsState, 0, sizeof(_objectsState));
	// memcpy(_inventoryImages, INVENTORY_IMG_INIT, 36);
	// memset(_inventoryInfo, 0, sizeof(_inventoryInfo));
	memset(_verbPrepositions, 0, sizeof(_verbPrepositions));
	memset(_globalObjectNames, 0, sizeof(_globalObjectNames));
	memset(_globalDialogueTexts, 0, sizeof(_globalDialogueTexts));
	memset(_verbsName, 0, sizeof(_verbsName));
	memset(_roomObjectNames, 0, sizeof(_roomObjectNames));

	_igorTempFrames = _facingIgorFrames[0] + 10500;

	memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
	memset(_roomActionsTable, 0, sizeof(_roomActionsTable));
	// _executeMainAction = 0;
	// _executeRoomAction = 0;
	// _previousMusic = 0;
	// _musicData = 0;
	// _scrollInventory = false;
	_roomCursorOn = true;
	_currentCursor = 0;
	// _dialogueCursorOn = true;
	// _updateDialogue = 0;
	_updateRoomBackground = 0;

	_resourceEntriesCount = 0;
	_resourceEntries = 0;
	_soundOffsetsCount = 0;
	_soundOffsets = 0;

	// _demoActionsCounter = 0;

	_gameTicks = 0;
}

Common::Error IgorEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	restart();
	setupDefaultPalette();

	if (!_ovlFile.open(_game.ovlFileName)) {
		error("Unable to open '%s'", _game.ovlFileName);
	}

	readTableFile();
	loadMainTexts();
	loadIgorFrames();

	drawVerbsPanel();
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();

	_gameState.talkMode = kTalkModeTextOnly;
	_gameState.talkSpeed = 3;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_eventQuitGame = false;
	PART_MAIN();
	_ovlFile.close();
	// _sndFile.close();

	return Common::kNoError;
}

void IgorEngine::PART_05() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);


	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	_updateRoomBackground = &IgorEngine::PART_05_UPDATE_ROOM_BACKGROUND;

	//Enforce display of room
	{
		memcpy(_screenVGA, _screenLayer1, 46080);
		drawVerbsPanel();
		fadeInPalette(720);
	}


	setRoomWalkBounds(0, 0, 319, 143);
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;

	{
		_walkData[0].setPos(0, 141, 2, 0);
		_walkData[0].setDefaultScale();
		_walkDataLastIndex = 0;
		_walkDataCurrentIndex = 1;
		buildWalkPath(0, 141, 51, 123);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}

	enterPartLoop();
	while (_currentPart >= 50 && _currentPart <= 52) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart == 255) {
		fadeOutPalette(768);
	} else if (_currentPart != 60) {
		// if (_objectsState[63] == 0) {
		// 	_objectsState[61] = _objectsState[62] = _objectsState[63] = 1;
		// }
		memcpy(_currentPalette, _paletteBuffer, 624);
		fadeOutPalette(624);
	}
}

void IgorEngine::PART_05_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61)) {
		scrollPalette(160, 167);
		setPaletteRange(160, 167);
	}
	if (compareGameTick(2, 8)) {
		scrollPalette(168, 178);
		setPaletteRange(168, 178);
	}
	if (compareGameTick(13, 16)) {
		scrollPalette(179, 184);
		setPaletteRange(179, 184);
	}
	if (compareGameTick(5, 32)) {
		scrollPalette(185, 191);
		setPaletteRange(185, 191);
	}
	if (compareGameTick(1)) {
		// switch (getRandomNumber(200)) {
		// case 0:
		// 	playSound(18, 1);
		// 	break;
		// case 3:
		// 	playSound(21, 1);
		// 	break;
		// case 9:
		// 	playSound(22, 1);
		// 	break;
		// case 17:
		// 	playSound(23, 1);
		// 	break;
		// }
	}
	// if (_gameState.talkMode == kTalkModeTextOnly || !_gameState.dialogueTextRunning) {
	// 	playSound(17, 1);
	// }
}

void IgorEngine::enterPartLoop() {
	// CursorMan.showMouse(true);
	if (!_gameState.dialogueTextRunning) {
		showCursor();
	}
	_gameState.igorMoving = false;
	// if (_game.version == kIdEngDemo110) {
	// 	CHECK_FOR_END_OF_DEMO();
	// }
}

void IgorEngine::leavePartLoop() {
	// hideCursor();
	// SET_EXEC_ACTION_FUNC(1, 0);
	// _updateRoomBackground = 0;

}

void IgorEngine::runPartLoop() {
	handleRoomInput();
	if (compareGameTick(1, 16)) {
		handleRoomIgorWalk();
	}
	// if (compareGameTick(19, 32)) {
	// 	handleRoomDialogue();
	// }
	// if (compareGameTick(4, 8)) {
	// 	handleRoomInventoryScroll();
	// }
	// if (compareGameTick(1)) {
	// 	handleRoomLight();
	// }
	if (_updateRoomBackground) {
		(this->*_updateRoomBackground)();
	}

	waitForTimer();
}

void IgorEngine::PART_MAIN() {
	while (!g_engine->shouldQuit() && !_eventQuitGame) {
		switch (_currentPart) {

		case 61:
			PART_05(); // SpringRock
			break;
		case 850:
			PART_85();
			break;
		case 900:
		case 901:
		case 902:
		case 903:
		case 904:
			PART_90();
			break;

		default:
			error("Unknown or unhandled room part %d", _currentPart);
			break;
		}
	}
}

Common::Error IgorEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Igor
