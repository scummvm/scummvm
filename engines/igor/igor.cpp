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
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"

#include "igor/igor.h"
#include "igor/console.h"
#include "igor/detection.h"
#include "igor/midi.h"


namespace Igor {

IgorEngine *g_engine;

IgorEngine::IgorEngine(OSystem *syst, const IgorGameDescription *gameDesc) : Engine(syst),
																		   _gameDescription(gameDesc), _randomSource("Igor") {
	g_engine = this;

	// Some parts draw outside the visible 320x200 area, but still within 64kb segment
	// hence allocate full 64-KiB VGA segment for the screen buffer
	_screenVGA = (uint8 *)malloc(65536);
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
	_verbsPanelSize = 0; // set when the panel is loaded

	_game.ovlFileName = gameDesc->ovlFileName;
	_game.sfxFileName = gameDesc->sfxFileName;
	_game.version = gameDesc->gameVersion;
	_game.flags = gameDesc->gameFlags;
	_game.language = gameDesc->desc.language;

	if (_game.version == kIdSpaFloppy) {
		_dialogueFont = &_floppyDialogueFont;
		_sentenceFont = &_floppySentenceFont;
	} else {
		_dialogueFont = &_font;
		_sentenceFont = &_font;
	}
	memset(_actionSentenceColors, 0, sizeof(_actionSentenceColors));

	_currentPart = kStartupPart;

	// _currentPart = 850;
	_midiPlayer = nullptr;
	_musicData = nullptr;

	_fastMode = 1;
}

IgorEngine::~IgorEngine() {
	_optionsMenuThumbnail.free();
	delete _midiPlayer;
	free(_musicData);
	free(_resourceEntries);
	free(_soundOffsets);
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

	delete _screen;
}

uint32 IgorEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String IgorEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

void IgorEngine::restart() {
	_screenVGAVOffset = 0;
	_gameStateLoaded = false;
	_creditsPass = 0;

	memset(&_gameState, 0, sizeof(_gameState));
	_mazeLocation = 0;
	_mazeSavedLocation = 0;
	_mazeRoom = 0;
	memset(&_roomWalkTargetRules, 0, sizeof(_roomWalkTargetRules));
	_roomWalkTargetBottom = 143;
	_roomGiveObjectSize = 0;
	_nextTimer = 0;
	_musicUpdateMillis = _system->getMillis();
	_musicTimerRemainder = 0;
	_musicTimerDivisor = 65536;
	_musicRestartCounter = 0xFF;
	_musicRestartDelay = 50;
	_musicSuspended = false;
	if (_midiPlayer)
		_midiPlayer->stopMusic();
	_fastMode = 1;
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
	_rightButtonSelecting = false;
	_hoverVerbState = kHoverVerbSelected;
	_keyboardCursorStep = 1;
	_rightButtonSelectCursorX = _rightButtonSelectCursorY = 0;
	memset(_inputVars, 0, sizeof(_inputVars));

	_talkDelay = _talkSpeechCounter = _talkDelayCounter = 0;
	memset(_dialogueTextsTable, 0, sizeof(_dialogueTextsTable));
	_dialogueTextsBuildCount = 0;
	_dialogueTextsStart = 0;
	_dialogueTextsCount = 0;
	_dialogueDirtyRectY = 0;
	_dialogueDirtyRectSize = 0;
	memset(_dialogueQuestions, 0, sizeof(_dialogueQuestions));
	memset(_dialogueReplies, 0, sizeof(_dialogueReplies));
	_dialogueEnded = false;
	_dialogueChoiceSelected = 0;
	memset(_dialogueInfo, 0, sizeof(_dialogueInfo));

	memset(_objectsState, 0, sizeof(_objectsState));
	_part07FirstVisitDone = false;
	_roomAmbientIndex = 0;
	_part68QuestionCounter = 1;
	_part69FireflyFrameA = 0;
	_part69FireflyFrameB = 0;
	_part69FireflyStepCount = 0;
	_part68LastDialogueCode = 0;
	_part68NpcPresent = false;
	_parkLadyIdleStep = 1;
	memcpy(_inventoryImages, INVENTORY_IMG_INIT, 36);
	memset(_inventoryInfo, 0, sizeof(_inventoryInfo));
	memset(_verbPrepositions, 0, sizeof(_verbPrepositions));
	memset(_globalObjectNames, 0, sizeof(_globalObjectNames));
	memset(_globalDialogueTexts, 0, sizeof(_globalDialogueTexts));
	memset(_verbsName, 0, sizeof(_verbsName));
	memset(_roomObjectNames, 0, sizeof(_roomObjectNames));

	_igorTempFrames = _facingIgorFrames[0] + 10500;

	memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
	memset(_roomActionsTable, 0, sizeof(_roomActionsTable));
	_executeMainAction = 0;
	_executeRoomAction = 0;
	_previousMusic = 0;
	_musicData = 0;
	_scrollInventory = false;
	_roomCursorOn = true;
	_currentCursor = 0;
	_dialogueCursorOn = true;
	_updateDialogue = 0;
	_talkColorIndex = kTalkColor;
	_talkShadowIndex = kTalkShadowColor;
	_updateRoomBackground = 0;

	_resourceEntriesCount = 0;
	_resourceEntries = 0;
	_soundOffsetsCount = 0;
	_soundOffsets = 0;

	_demoActionsCounter = 0;

	_gameTicks = 0;
}

Common::Error IgorEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	_screen = new Graphics::Screen();

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));

	// Set the engine's debugger console
	setDebugger(new Console());

	restart();
	setupDefaultPalette();

	if (!_ovlFile.open(_game.ovlFileName)) {
		error("Unable to open '%s'", _game.ovlFileName);
	}

	if (!_sndFile.open(_game.sfxFileName)) {
		error("Unable to open '%s'", _game.sfxFileName);
	}

	if (_game.version == kIdSpaFloppy) {
		unpackExecutable();
	}

	readTableFile();
	if (_game.flags & kFlagFloppy)
		_midiPlayer = new MidiPlayer(_mixer);
	loadMainTexts();
	loadIgorFrames();

	_gameState.talkMode = kTalkModeSpeechAndText;
	_gameState.talkSpeed = 3;
	syncSoundSettings();
	_talkSpeechCounter = 5;
	_eventQuitGame = false;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	partMain();
	_ovlFile.close();
	_sndFile.close();

	return Common::kNoError;
}


void IgorEngine::enterPartLoop() {
	if (!_gameState.dialogueTextRunning) {
		showCursor();
	}
	_gameState.igorMoving = false;
	if (_game.version == kIdEngDemo110 && _demoActionsCounter >= 10) {
		ADD_DIALOGUE_TEXT(104, 2);
		ADD_DIALOGUE_TEXT(106, 2);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
	}
}

bool IgorEngine::restoreRoomAfterLoad(bool drawIgor) {
	if (!_gameStateLoaded)
		return false;

	memcpy(_screenVGA, _screenLayer1, 46080);
	if (drawIgor) { // position Igor in the room at the saved position
		if (!_gameState.igorMoving)
			_walkDataCurrentIndex = _walkDataLastIndex - 1;
		WalkData *wd = &_walkData[_walkDataCurrentIndex];
		moveIgor(wd->posNum, wd->frameNum);
		if (!_gameState.igorMoving)
			_walkDataCurrentIndex = _walkDataLastIndex;
	}
	if (!_gameState.dialogueStarted) { // if loading we need to redraw the room background and UI
		memset(_screenVGA + 46080, 0, 17920);
		SET_PAL_208_96_1();
		SET_PAL_240_48_1();
		drawVerbsPanel();
		redrawVerb(_currentAction.verb, true);
		drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	}
	if (!_gameState.dialogueTextRunning)
		showCursor();
	_gameStateLoaded = false;
	fadeIn(768);
	return true;
}

void IgorEngine::leavePartLoop() {
	_roomWalkTargetRules.enabled = false;
	_roomGiveObjectSize = 0;
	hideCursor();
	partMainSetExecuteAction(1, 0);
	_updateRoomBackground = 0;

}

void IgorEngine::runPartLoop() {
	handleRoomInput();
	if (_gameStateLoaded)
		return;
	if (compareGameTick(1, 16)) {
		handleRoomIgorWalk();
	}
	if (compareGameTick(19, 32)) {
		handleRoomDialogue();
	}
	if (compareGameTick(4, 8)) {
		handleRoomInventoryScroll();
	}
	if (compareGameTick(1)) {
		handleIgorIdleAnimation();
	}
	if (_updateRoomBackground) {
		(this->*_updateRoomBackground)();
	}

	waitForTimer();
}

/**
 * igor's eyes blink when he is idle
 */
void IgorEngine::handleIgorIdleAnimation() {
	if (_gameState.dialogueTextRunning || _gameState.igorMoving) {
		_gameState.updateLight = false;
	} else if (_gameState.updateLight) {
		updateIgorIdleAnimation(0);
		_gameState.updateLight = 0;
	} else if (getRandomNumber(9) == 0) {
		updateIgorIdleAnimation(1);
		_gameState.updateLight = true;
	}
}

void IgorEngine::handleRoomInventoryScroll() {
	if (_scrollInventory) {
		scrollInventory();
	}
}

void IgorEngine::addObjectToInventory(int object, int index) {
	debugC(9, kDebugEngine, "addObjectToInventory %d %d", object, index);
	++_inventoryInfo[kInventoryCount];
	_inventoryInfo[_inventoryInfo[kInventoryCount] - 1] = object;
	_inventoryInfo[index] = _inventoryInfo[kInventoryCount];
	if (_game.version == kIdSpaFloppy) {
		if (_inventoryInfo[kInventoryCount] > 8)
			_inventoryInfo[kInventoryFirstVisible] = _inventoryInfo[kInventoryCount] - 7;
	} else {
		_inventoryInfo[kInventoryFirstVisible] = _inventoryOffsetTable[(_inventoryInfo[kInventoryCount] - 1) / 7];
	}
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	playSound(51, 1);
}

void IgorEngine::debugChangePart(int state) {
	_currentPart = state;
}

bool IgorEngine::debugAddObjectToInventory(int object) {
	if (object < 1 || object > (int)ARRAYSIZE(_inventoryImages)) {
		return false;
	}

	const int index = object + ARRAYSIZE(_inventoryImages) - 1;
	if (_inventoryInfo[index] != 0 || _inventoryInfo[kInventoryCount] >= ARRAYSIZE(_inventoryImages)) {
		return false;
	}

	addObjectToInventory(object, index);
	return true;
}

/**
 * just removes without updating ui or playing the sound
 */
void IgorEngine::removeInventoryEntry(int index) {
	_inventoryInfo[_inventoryInfo[index] - 1] = 0;
	_inventoryInfo[index] = 0;
	packInventory();
	if (_game.version != kIdSpaFloppy && _inventoryInfo[kInventoryFirstVisible] > _inventoryInfo[kInventoryCount]) {
		_inventoryInfo[kInventoryFirstVisible] = _inventoryOffsetTable[(_inventoryInfo[kInventoryCount] - 1) / 7];
	}
}

void IgorEngine::removeObjectFromInventory(int index) {
	removeInventoryEntry(index);
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	playSound(63, 1);
}

int IgorEngine::getObjectFromInventory(int x) const {
	if (_game.version == kIdSpaFloppy) {
		if (x >= 0 && x < 320) {
			const int i = x / 40 + _inventoryInfo[kInventoryFirstVisible];
			if (i <= _inventoryInfo[kInventoryCount])
				return _inventoryInfo[i - 1];
		}
		return 0;
	}
	if (x >= 20 && x <= 299) {
		int i = (x - 20) / 40 + _inventoryInfo[kInventoryFirstVisible];
		if (i <= _inventoryInfo[kInventoryCount]) {
			return _inventoryInfo[i - 1];
		}
	}
	return 0;
}

void IgorEngine::pageFloppyInventory(bool forward) {
	const int count = _inventoryInfo[kInventoryCount];
	const int start = _inventoryInfo[kInventoryFirstVisible];
	if (forward) {
		if (count <= start + 7 || start >= count - 7)
			return;
		_inventoryInfo[kInventoryFirstVisible] = (start < count - 11) ? start + 4 : count - 7;
	} else {
		if (count <= 8 || start <= 1)
			return;
		_inventoryInfo[kInventoryFirstVisible] = (start > 4) ? start - 4 : 1;
	}
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
}

void IgorEngine::packInventory() {
	for (int i = 1; i <= _inventoryInfo[kInventoryCount]; ++i) {

		if (_inventoryInfo[i - 1] != 0) {
			continue;
		}
		int count = _inventoryInfo[kInventoryCount] - 1;
		for (int index = i; index <= count; ++index) {
			_inventoryInfo[index - 1] = _inventoryInfo[index];
			_inventoryInfo[_inventoryInfo[index - 1] + ARRAYSIZE(_inventoryImages) - 1] = index;
		}
		_inventoryInfo[_inventoryInfo[kInventoryCount] - 1] = 0;
		--_inventoryInfo[kInventoryCount];
	}
}

void IgorEngine::executeAction(int action) {
	debugC(9, kDebugEngine, "executeAction %d", action);
	assert(action < 200);
	if (action <= 100) {
		(this->*_executeMainAction)(action);
	} else {
		(this->*_executeRoomAction)(action);
	}
}


void IgorEngine::handlePause() {
	// The CD version's pause routine returns without doing anything.
	if (_game.version == kIdEngCD || _game.version == kIdSpaCD)
		return;

	drawActionSentence(getString(STR_GamePaused), 0xFB);
	do {
		waitForTimer();
	} while (!_inputVars[kInputPause]);
	memset(_inputVars, 0, sizeof(_inputVars));
}

Common::Error IgorEngine::syncGame(Common::Serializer &s) {

	// Walk data (100 entries)
	for (int i = 0; i < 100; ++i) {
		s.skip(2);
		s.syncAsSint16LE(_walkData[i].x);
		s.syncAsSint16LE(_walkData[i].y);
		s.syncAsByte(_walkData[i].posNum);
		s.syncAsByte(_walkData[i].frameNum);
		s.syncAsByte(_walkData[i].clipSkipX);
		s.syncAsSint16LE(_walkData[i].clipWidth);
		s.syncAsSint16LE(_walkData[i].scaleWidth);
		s.syncAsByte(_walkData[i].xPosChanged);
		s.syncAsSint16LE(_walkData[i].dxPos);
		s.syncAsByte(_walkData[i].yPosChanged);
		s.syncAsSint16LE(_walkData[i].dyPos);
		s.syncAsByte(_walkData[i].scaleHeight);
	}

	// Walk path state
	s.skip(20);
	s.syncAsByte(_walkDataCurrentIndex);
	s.syncAsByte(_walkDataLastIndex);
	s.syncAsByte(_walkCurrentFrame);
	s.syncAsByte(_walkCurrentPos);
	s.skip(23);

	// Current action
	s.syncAsByte(_currentAction.verb);
	s.syncAsByte(_currentAction.object1Num);
	s.syncAsByte(_currentAction.object1Type);
	s.syncAsByte(_currentAction.verbType);
	s.syncAsByte(_currentAction.object2Num);
	s.syncAsByte(_currentAction.object2Type);
	s.skip(10);

	// Part/state
	s.syncAsSint16LE(_currentPart);
	// the location in the maze
	s.syncAsByte(_mazeLocation);
	// whether Igor has already read the notice board in room 7
	s.syncAsByte(_part07FirstVisitDone);
	s.skip(6);

	// Action state
	s.syncAsByte(_actionCode);
	s.syncAsByte(_actionWalkPoint);
	s.skip(2);

	// Cursor position
	s.syncAsSint16LE(_inputVars[kInputCursorXPos]);
	s.syncAsSint16LE(_inputVars[kInputCursorYPos]);

	// Game state
	s.syncAsByte(_gameState.enableLight);
	s.syncAsByte(_gameState.colorLum);
	for (int i = 0; i < 5; ++i) {
		s.syncAsSint16LE(_gameState.counter[i]);
	}
	{
		byte v = _gameState.igorMoving ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.igorMoving = v != 0;
	}
	{
		byte v = _gameState.dialogueTextRunning ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.dialogueTextRunning = v != 0;
	}
	{
		byte v = _gameState.updateLight ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.updateLight = v != 0;
	}
	{
		byte v = _gameState.unkF ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.unkF = v != 0;
	}
	s.syncAsByte(_gameState.unk10);
	s.syncAsByte(_gameState.unk11);
	{
		byte v = _gameState.dialogueStarted ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.dialogueStarted = v != 0;
	}
	s.skip(1);
	for (int i = 0; i < 500; ++i) {
		s.syncAsByte(_gameState.dialogueData[i]);
	}
	s.syncAsByte(_gameState.dialogueChoiceStart);
	s.syncAsByte(_gameState.dialogueChoiceCount);
	s.skip(2);
	s.syncAsByte(_gameState.shouldShowCutsceneCounter);
	{
		byte v = _gameState.jumpToNextMusic ? 1 : 0;
		s.syncAsByte(v);
		if (s.isLoading())
			_gameState.jumpToNextMusic = v != 0;
	}
	s.syncAsByte(_gameState.configSoundEnabled);
	s.syncAsByte(_gameState.talkSpeed);
	s.syncAsByte(_gameState.talkMode);
	s.skip(3);
	s.syncAsByte(_gameState.musicNum);
	s.syncAsByte(_gameState.musicSequenceIndex);

	// Global Object states
	for (int i = 0; i < 112; ++i) {
		s.syncAsByte(_objectsState[i]);
	}

	// Inventory
	for (int i = 0; i < 74; ++i) {
		s.syncAsByte(_inventoryInfo[i]);
	}

	if (s.isLoading()) {
		_musicSuspended = true;
		syncSoundSettings();
		// Fade the outgoing screen before any loaded room redraws its pixels.
		fadeOut(768);
		_gameStateLoaded = true;
		memcpy(_igorPalette, (_currentPart == 760) ? PAL_IGOR_2 : PAL_IGOR_1, 48);
		partMainUpdateObjectState(255);
		playMusic(_gameState.musicNum, true);
		_musicSuspended = false;
		_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
		if (_currentPart < 900) {
			showCursor();
		}
	}

	return Common::kNoError;
}

} // End of namespace Igor
