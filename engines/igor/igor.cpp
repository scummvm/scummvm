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


namespace Igor {

IgorEngine *g_engine;

IgorEngine::IgorEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																		   _gameDescription(gameDesc), _randomSource("Igor") {
	g_engine = this;

	// The original draws the fifth Dean dialogue choice at y=190 into the
	// 64-KiB mode-13h VGA segment; its 11th font row is outside the visible
	// 320x200 area but still inside that segment.
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
	_debugOverlayBuffer = (uint8 *)malloc(320 * 144);

	{ // hardcoded now
		_game.ovlFileName = "igor.exe";
		_game.sfxFileName = "igor.dat";
		_game.version = kIdSpaCD;
		_game.flags = kFlagTalkie;

		_game.language = Common::ES_ESP; // Assuming 0 represents the default language


		_currentPart = 850;

		// _currentPart = 171;
		// _currentPart = 62;
		// _currentPart = 40;
	}

	// if (_game.flags & kFlagFloppy) {
	// 	_midiPlayer = new Midi(this);
	// } else {
	// 	_midiPlayer = 0;
	// }

	_fastMode = 1;
}

IgorEngine::~IgorEngine() {
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
	free(_debugOverlayBuffer);

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
	_debugOverlayMode = kOverlayOff;
	_gameStateLoaded = false;

	memset(&_gameState, 0, sizeof(_gameState));
	_nextTimer = 0;
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
	_part07FirstVisitDone = false; // cseg197:0795-0845; original global s3:0xED2E
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
	// _previousMusic = 0;
	// _musicData = 0;
	_scrollInventory = false;
	_roomCursorOn = true;
	_currentCursor = 0;
	_dialogueCursorOn = true;
	_updateDialogue = 0;
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

	readTableFile();
	loadMainTexts();
	loadIgorFrames();

	// _gameState.talkMode = kTalkModeTextOnly;
	// _gameState.talkMode = kTalkModeSpeechOnly;
	_gameState.talkMode = kTalkModeSpeechAndText;
	_gameState.talkSpeed = 3;
	_talkSpeechCounter = 5;
	_eventQuitGame = false;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	PART_MAIN();
	_ovlFile.close();
	_sndFile.close();

	return Common::kNoError;
}


void IgorEngine::enterPartLoop() {
	if (!_gameState.dialogueTextRunning) {
		showCursor();
	}
	_gameState.igorMoving = false;
	// if (_game.version == kIdEngDemo110) {
	// 	CHECK_FOR_END_OF_DEMO();
	// }
}

bool IgorEngine::restoreRoomAfterLoad(bool drawIgor) {
	if (!_gameStateLoaded)
		return false;

	memcpy(_screenVGA, _screenLayer1, 46080);
	if (drawIgor) {
		if (!_gameState.igorMoving)
			_walkDataCurrentIndex = _walkDataLastIndex - 1;
		WalkData *wd = &_walkData[_walkDataCurrentIndex];
		moveIgor(wd->posNum, wd->frameNum);
		_walkDataCurrentIndex = _walkDataLastIndex;
	}
	drawInventory(_inventoryInfo[72], 0);
	fadeIn(768);
	_gameStateLoaded = false;
	return true;
}

void IgorEngine::leavePartLoop() {
	hideCursor();
	SET_EXEC_ACTION_FUNC(1, 0);
	_updateRoomBackground = 0;

}

void IgorEngine::runPartLoop() {
	handleRoomInput();
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
		handleRoomLight();
	}
	if (_updateRoomBackground) {
		(this->*_updateRoomBackground)();
	}

	waitForTimer();
}

void IgorEngine::handleRoomDialogue() {
	if (_gameState.dialogueTextRunning) {
		// Speech completion ends the sentence, exactly as in the two blocking
		// dialogue loops. Without this the sentence only ever ends on a click:
		if (_gameState.talkMode != kTalkModeTextOnly && !isDialogueSpeechPlaying()) {
			_talkDelayCounter = _talkDelay;
		}
		if (_talkDelayCounter == _talkDelay) {
			animateIgorTalking(0);
			memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
			if (_dialogueTextsCount == 0) {
				// Speech must stop with the last line, otherwise it keeps playing
				// after the text is gone and nothing is left to stop it.
				stopDialogueSpeech();
				_gameState.dialogueTextRunning = false;
				showCursor();
			} else {
				++_dialogueTextsStart;
				startIgorDialogue();
			}
		} else {
			animateIgorTalking(getRandomNumber(5));
			++_talkDelayCounter;
		}
	}
}


void IgorEngine::handleRoomLight() {
	if (_gameState.dialogueTextRunning || _gameState.igorMoving) {
		_gameState.updateLight = false;
	} else if (_gameState.updateLight) {
		updateRoomLight(0);
		_gameState.updateLight = 0;
	} else if (getRandomNumber(10) == 0) {
		updateRoomLight(1);
		_gameState.updateLight = true;
	}
}

void IgorEngine::updateRoomLight(int fl) {
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	if (wd->scaleHeight != 50 || _gameState.dialogueTextRunning) {
		return;
	}
	int offset = 320 * (wd->y + 1 - wd->scaleWidth);
	int x = wd->x - _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (x <= 0) {
		return;
	}
	offset += x;
	RoomObjectArea *roa;
	int color = (fl == 0) ? 196 : 195;
	switch (wd->posNum) {
	case 2:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1298]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1298] = color;
		}
		break;
	case 3:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1293]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1293] = color;
		}
		color = (fl == 0) ? 196 : 195;
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1296]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1296] = color;
		}
		break;
	case 4:
		roa = &_roomObjectAreasTable[_screenLayer2[offset + 1291]];
		if (wd->y > roa->y1Lum) {
			if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
				color -= roa->deltaLum;
			}
			_screenVGA[offset + 1291] = color;
		}
		break;
	}
}

void IgorEngine::handleRoomInventoryScroll() {
	if (_scrollInventory) {
		scrollInventory();
	}
}

void IgorEngine::scrollInventory() {
	if (_scrollInventoryStartY == _scrollInventoryEndY) {
		memcpy(_screenVGA + 54400, _inventoryPanelBuffer + (_scrollInventoryStartY - 1) * 320, 9600);
		_scrollInventory = false;
	} else {
		int offset = 54420;
		for (int y = _scrollInventoryStartY; y < _scrollInventoryStartY + 29; ++y) {
			memcpy(_screenVGA + offset, _inventoryPanelBuffer + 320 * y - 300, 280);
			offset += 320;
		}
		_scrollInventoryStartY += _scrollInventoryDy;
	}
}

void IgorEngine::drawInventory(int start, int mode) {
	loadData(IMG_InventoryPanel, _inventoryPanelBuffer);
	loadData(IMG_Objects, _inventoryImagesBuffer);
	int y, i;
	int end = start + 7; // seven slots
	int x = 1;
	// Paint inventory icons
	for (y = start; y != end; ++y) {
		if (_inventoryInfo[y - 1] == 0) {
			for (i = 1; i <= 30; ++i) {
				memset(_inventoryPanelBuffer + x * 40 - 20 + (i - 1) * 320, 0, 40);
			}
		} else {
			for (i = 1; i <= 30; ++i) {
				int img = _inventoryInfo[y - 1];
				assert(img >= 1);
				memcpy(_inventoryPanelBuffer + x * 40 - 20 + i * 320 - 321, _inventoryImagesBuffer + (i - 1) * 40 + (_inventoryImages[img - 1] - 1) * 1200, 40);
			}
		}
		++x;
	}
	// Hide arrows
	if (_inventoryInfo[72] == 1) {
		// 'hide' scroll up
		for (y = 5; y <= 11; ++y) {
			for (x = 4; x <= 12; ++x) {
				uint8 *p = _inventoryPanelBuffer + y * 320 + x - 321;
				if (*p == 0xF2) {
					*p = 0xF3;
					p = _inventoryPanelBuffer + y * 320 + x + 305 - 321;
					*p = 0xF3;
				}
			}
		}
	}
	if (_inventoryInfo[73] <= _inventoryInfo[72] + 6 || _inventoryInfo[72] >= _inventoryInfo[73] - 6) {
		// 'hide' scroll down
		for (y = 19; y <= 25; ++y) {
			for (x = 4; x <= 12; ++x) {
				uint8 *p = _inventoryPanelBuffer + y * 320 + x - 321;
				if (*p == 0xF2) {
					*p = 0xF3;
					p = _inventoryPanelBuffer + y * 320 + x + 305 - 321;
					*p = 0xF3;
				}
			}
		}
	}
	switch (mode) {
	case 0: // normal inventory rendering
		memcpy(_screenVGA + 54400, _inventoryPanelBuffer, 9600);
		_scrollInventory = false;
		break;
	case 1: // animation scrolling up
		for (y = 0; y <= 11; ++y) {
			for (x = 0; x <= 14; ++x) {
				uint8 *p = _screenVGA + x + y * 320 + 59520;
				if ((*p & 0x80) != 0) {
					*p += 8;
					p = _screenVGA + x + y * 320 + 59825;
					*p += 8;
				}
			}
		}
		memmove(_inventoryPanelBuffer + 9600, _inventoryPanelBuffer, 9600);
		memcpy(_inventoryPanelBuffer, _screenVGA + 54400, 9600);
		_scrollInventoryStartY = 7;
		_scrollInventoryEndY = 31;
		_scrollInventoryDy = 6;
		_scrollInventory = true;
		break;
	case 2: // animation scrolling down
		for (y = 0; y <= 11; ++y) {
			for (x = 0; x <= 14; ++x) {
				uint8 *p = _screenVGA + x + y * 320 + 55040;
				if ((*p & 0x80) != 0) {
					*p += 8;
					p = _screenVGA + x + y * 320 + 55345;
					*p += 8;
				}
			}
		}
		memmove(_inventoryPanelBuffer + 9600, _inventoryPanelBuffer, 9600);
		memcpy(_inventoryPanelBuffer + 9600, _screenVGA + 54400, 9600);
		_scrollInventoryStartY = 25;
		_scrollInventoryEndY = 1;
		_scrollInventoryDy = -6;
		_scrollInventory = true;
		break;
	}
}


void IgorEngine::addObjectToInventory(int object, int index) {
	debugC(9, kDebugEngine, "addObjectToInventory %d %d", object, index);
	++_inventoryInfo[73];
	_inventoryInfo[_inventoryInfo[73] - 1] = object;
	_inventoryInfo[index] = _inventoryInfo[73];
	_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	drawInventory(_inventoryInfo[72], 0);
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
	if (_inventoryInfo[index] != 0 || _inventoryInfo[73] >= ARRAYSIZE(_inventoryImages)) {
		return false;
	}

	addObjectToInventory(object, index);
	return true;
}

void IgorEngine::removeObjectFromInventory(int index) {
	_inventoryInfo[_inventoryInfo[index] - 1] = 0;
	_inventoryInfo[index] = 0;
	packInventory();
	if (_inventoryInfo[72] > _inventoryInfo[73]) {
		_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	}
	drawInventory(_inventoryInfo[72], 0);
	playSound(63, 1);
}

int IgorEngine::getObjectFromInventory(int x) const {
	if (x >= 20 && x <= 299) {
		int i = (x - 20) / 40 + _inventoryInfo[72];
		if (i <= _inventoryInfo[73]) {
			return _inventoryInfo[i - 1];
		}
	}
	return 0;
}

void IgorEngine::packInventory() {
	for (int i = 1; i <= _inventoryInfo[73]; ++i) {

		if (_inventoryInfo[i - 1] != 0) {
			continue;
		}
		int count = _inventoryInfo[73] - 1;
		for (int index = i; index <= count; ++index) {
			_inventoryInfo[index - 1] = _inventoryInfo[index];
			_inventoryInfo[_inventoryInfo[index - 1] + ARRAYSIZE(_inventoryImages) - 1] = index;
		}
		_inventoryInfo[_inventoryInfo[73] - 1] = 0;
		--_inventoryInfo[73];
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
	drawActionSentence(getString(STR_GamePaused), 0xFB);
	do {
		waitForTimer();
	} while (!_inputVars[kInputPause]);
	memset(_inputVars, 0, sizeof(_inputVars));
}

Common::Error IgorEngine::syncGame(Common::Serializer &s) {
	// Serialize in the same order as reference implementation

	// 1. Walk data (100 entries)
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

	// 2. Walk path state
	s.skip(20);
	s.syncAsByte(_walkDataCurrentIndex);
	s.syncAsByte(_walkDataLastIndex);
	s.syncAsByte(_walkCurrentFrame);
	s.syncAsByte(_walkCurrentPos);
	s.skip(23);

	// 3. Current action
	s.syncAsByte(_currentAction.verb);
	s.syncAsByte(_currentAction.object1Num);
	s.syncAsByte(_currentAction.object1Type);
	s.syncAsByte(_currentAction.verbType);
	s.syncAsByte(_currentAction.object2Num);
	s.syncAsByte(_currentAction.object2Type);
	s.skip(10);

	// 4. Part/state
	s.syncAsSint16LE(_currentPart);
	s.skip(8);

	// 5. Action state
	s.syncAsByte(_actionCode);
	s.syncAsByte(_actionWalkPoint);
	s.skip(2);

	// 6. Cursor position
	s.syncAsSint16LE(_inputVars[kInputCursorXPos]);
	s.syncAsSint16LE(_inputVars[kInputCursorYPos]);

	// 7. Game state
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
	s.syncAsByte(_gameState.nextMusicCounter);
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

	// 8. Object states
	for (int i = 0; i < 112; ++i) {
		s.syncAsByte(_objectsState[i]);
	}

	// 9. Inventory
	for (int i = 0; i < 74; ++i) {
		s.syncAsByte(_inventoryInfo[i]);
	}

	if (s.isLoading()) {
		_gameStateLoaded = true;
		memcpy(_igorPalette, (_currentPart == 760) ? PAL_IGOR_1 : PAL_IGOR_1, 48);
		UPDATE_OBJECT_STATE(255);
		playMusic(_gameState.musicNum);
		_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
		if (_currentPart < 900) {
			showCursor();
		}
	}

	return Common::kNoError;
}

} // End of namespace Igor
