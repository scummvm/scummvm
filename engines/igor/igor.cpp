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
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
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

	{ // hardcoded now
		_game.ovlFileName = "igor.exe";
		_game.sfxFileName = "igor.dat";
		_currentPart = 50;
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

	// Common::clearAllDebugChannels();

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

	// memset(&_gameState, 0, sizeof(_gameState));
	_nextTimer = 0;
	// _fastMode = false;
	// _language = 0;

	// memset(_walkData, 0, sizeof(_walkData));
	// _walkCurrentPos = 0;
	// _walkDataLastIndex = _walkDataCurrentIndex = 0;
	// _walkCurrentFrame = 0;
	// _walkDataCurrentPosX = _walkDataCurrentPosY = 0;
	// _walkToObjectPosX = _walkToObjectPosY = 0;

	// memset(&_currentAction, 0, sizeof(_currentAction));
	// _currentAction.verb = kVerbWalk;
	// _actionCode = 0;
	// _actionWalkPoint = 0;
	// memset(_inputVars, 0, sizeof(_inputVars));
	// _musicData = 0;

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
	// memset(_verbPrepositions, 0, sizeof(_verbPrepositions));
	// memset(_globalObjectNames, 0, sizeof(_globalObjectNames));
	memset(_globalDialogueTexts, 0, sizeof(_globalDialogueTexts));
	// memset(_verbsName, 0, sizeof(_verbsName));
	memset(_roomObjectNames, 0, sizeof(_roomObjectNames));

	// _igorTempFrames = _facingIgorFrames[0] + 10500;

	memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
	// memset(_roomActionsTable, 0, sizeof(_roomActionsTable));
	// _executeMainAction = 0;
	// _executeRoomAction = 0;
	// _previousMusic = 0;
	// _musicData = 0;
	// _actionCode = 0;
	// _actionWalkPoint = 0;
	// memset(_inputVars, 0, sizeof(_inputVars));
	// _scrollInventory = false;
	// _roomCursorOn = true;
	// _currentCursor = 0;
	// _dialogueCursorOn = true;
	// _updateDialogue = 0;
	// _updateRoomBackground = 0;

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


void IgorEngine::setupDefaultPalette() {
	memset(_currentPalette, 0, 255 * 3);
	memset(&_currentPalette[255 * 3], 63, 3);
	g_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
}

void IgorEngine::readTableFile() {
	Common::File tblFile;
	uint32 stringsEntriesOffset = 0, resourcesEntriesOffset = 0, soundEntriesOffset = 0;
	if (tblFile.open("IGOR.TBL") && tblFile.readUint32BE() == 'ITBL' && tblFile.readUint32BE() == 4) {
		stringsEntriesOffset = tblFile.readUint32BE();
		uint32 borlandOverlaySize = _ovlFile.size();
		int gameVersionsCount = tblFile.readByte();
		for (int i = 0; i < gameVersionsCount; ++i) {
			uint32 size = tblFile.readUint32BE();
			if (size == borlandOverlaySize) {
				resourcesEntriesOffset = tblFile.readUint32BE();
				soundEntriesOffset = tblFile.readUint32BE();
				break;
			}
			tblFile.skip(8);
		}
	}
	if (resourcesEntriesOffset != 0 && soundEntriesOffset != 0) {
		tblFile.seek(resourcesEntriesOffset);
		_resourceEntriesCount = tblFile.readUint16BE();
		_resourceEntries = (ResourceEntry *)malloc(sizeof(ResourceEntry) * _resourceEntriesCount);
		for (int i = 0; i < _resourceEntriesCount; ++i) {
			_resourceEntries[i].id = tblFile.readUint16BE();
			_resourceEntries[i].offs = tblFile.readUint32BE();
			_resourceEntries[i].size = tblFile.readUint32BE();
		}
		tblFile.seek(soundEntriesOffset);
		_soundOffsetsCount = tblFile.readUint16BE();
		_soundOffsets = (uint32 *)malloc(sizeof(uint32) * _soundOffsetsCount);
		for (int i = 0; i < _soundOffsetsCount; ++i) {
			_soundOffsets[i] = tblFile.readUint32BE();
		}
		tblFile.seek(stringsEntriesOffset);
		int stringsCount = tblFile.readUint16BE();
		for (int i = 0; i < stringsCount; ++i) {
			int id = tblFile.readUint16BE();
			int lang = tblFile.readByte();
			int len = tblFile.readByte();
			bool skipString = (lang == 1 && _game.language != Common::EN_ANY) || (lang == 2 && _game.language != Common::ES_ESP);
			if (skipString) {
				tblFile.skip(len);
			} else {
				char buf[256];
				tblFile.read(buf, len);
				buf[len] = 0;
				_stringEntries.push_back(StringEntry(id, buf));
			}
		}
		return;
	}
	error("Unable to read 'IGOR.TBL'");
}

void IgorEngine::loadMainTexts() {
}

void IgorEngine::loadIgorFrames() {
	loadData(FRM_IgorDirBack, _facingIgorFrames[0]);
	loadData(FRM_IgorDirRight, _facingIgorFrames[1]);
	loadData(FRM_IgorDirFront, _facingIgorFrames[2]);
	loadData(FRM_IgorDirLeft, _facingIgorFrames[3]);
	loadData(FRM_IgorHead, _igorHeadFrames);
	memcpy(_igorPalette, PAL_IGOR_1, 48);
}

static int compareResourceEntry(const void *a, const void *b) {
	int id = *(const int *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return id - entry->id;
}

ResourceEntry *IgorEngine::findData(int id) {
	ResourceEntry *re = (ResourceEntry *)bsearch(&id, _resourceEntries, _resourceEntriesCount, sizeof(ResourceEntry), compareResourceEntry);
	assert(re);
	return re;
}

uint8 *IgorEngine::loadData(int id, uint8 *dst, int *size) {
	// debugC(9, kDebugResource, "loadData() id %d", id);
	ResourceEntry *re = findData(id);
	if (!dst) {
		dst = (uint8 *)malloc(re->size);
		if (!dst) {
			error("Unable to allocate %d bytes", re->size);
		}
	}
	_ovlFile.seek(re->offs);
	_ovlFile.read(dst, re->size);
	if (size) {
		*size = re->size;
	}
	return dst;
}

void IgorEngine::loadRoomData(int pal, int img, int box, int msk, int txt) {
	if (pal != 0) {
		loadData(pal, _paletteBuffer);
	}
	if (img != 0) {
		loadData(img, _screenLayer1);
	}
	if (box != 0) {
		int sz;
		uint8 *p = loadData(box, 0, &sz);
		memset(_roomObjectAreasTable, 0, sizeof(_roomObjectAreasTable));
		assert((sz % 5) == 0);
		decodeRoomAreas(p, sz / 5);
		free(p);
	}
	if (msk != 0) {
		uint8 *p = loadData(msk);
		decodeRoomMask(p);
		free(p);
	}
	if (txt != 0) {
		uint8 *p = loadData(txt);
		decodeRoomText(p);
		free(p);
	}
}

void IgorEngine::PART_05() {
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);

	//Enforce display of room
	{
		// g_system->getPaletteManager()->setPalette(_paletteBuffer, 0, 256);
		memcpy(_currentPalette, _paletteBuffer, 768);
		updatePalette(768);
		// fadeInPalette(768);
		memcpy(_screenVGA, _screenLayer1, 46080);
	}

	enterPartLoop();
	while (_currentPart >= 50 && _currentPart <= 52) {
		runPartLoop();
	}
	leavePartLoop();
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
	CursorMan.showMouse(true);
	// if (!_gameState.dialogueTextRunning) {
	// 	showCursor();
	// }
	// _gameState.igorMoving = false;
	// if (_game.version == kIdEngDemo110) {
	// 	CHECK_FOR_END_OF_DEMO();
	// }
}

void IgorEngine::leavePartLoop() {
	// hideCursor();
	// SET_EXEC_ACTION_FUNC(1, 0);
	// _updateRoomBackground = 0;
	CursorMan.showMouse(false);
}

void IgorEngine::runPartLoop() {
	handleRoomInput();
	// if (compareGameTick(1, 16)) {
	// 	handleRoomIgorWalk();
	// }
	// if (compareGameTick(19, 32)) {
	// 	handleRoomDialogue();
	// }
	// if (compareGameTick(4, 8)) {
	// 	handleRoomInventoryScroll();
	// }
	// if (compareGameTick(1)) {
	// 	handleRoomLight();
	// }
	// if (_updateRoomBackground) {
	// 	(this->*_updateRoomBackground)();
	// }
	{
		PART_05_UPDATE_ROOM_BACKGROUND();
	}
	waitForTimer();
}

void IgorEngine::scrollPalette(int startColor, int endColor) {
	uint8 c[3];
	memcpy(c, &_currentPalette[startColor * 3], 3);
	memmove(&_currentPalette[startColor * 3], &_currentPalette[(startColor + 1) * 3], (endColor - startColor) * 3);
	memcpy(&_currentPalette[endColor * 3], c, 3);
}

void IgorEngine::setPaletteRange(int startColor, int endColor) {
	// debugC(9, kDebugScreen, "setPaletteRange(%d, %d)", startColor, endColor);
	assert(endColor - startColor + 1 <= 256);
	for (int i = startColor; i <= endColor; ++i) {
		setPaletteColor(i, _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
	}
}

void IgorEngine::setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b) {
	uint8 pal[4];
	pal[0] = (r << 2) | (r >> 4);
	pal[1] = (g << 2) | (g >> 4);
	pal[2] = (b << 2) | (b >> 4);
	pal[3] = 0;
	g_system->getPaletteManager()->setPalette(pal, index, 1);
}

void IgorEngine::updatePalette(int count) {
	assert(count <= 768);
	uint8 pal[1024];
	for (int j = 0, i = 0; i < count; ++i) {
		pal[j++] = (_currentPalette[i] << 2);// | (_currentPalette[i] >> 4);
		// if (((i + 1) % 3) == 0) {
		// 	pal[j++] = 0;
		// }
	}
	g_system->getPaletteManager()->setPalette(pal, 0, count / 3);
}

void IgorEngine::fadeInPalette(int count) {
	// debugC(9, kDebugScreen, "fadeInPalette(%d)", count);
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	int m = 66;
	do {
		m -= 3;
		for (int i = 0; i < count; ++i) {
			if (_paletteBuffer[i] >= m) {
				uint8 color = _currentPalette[i] + 3;
				if (color > _paletteBuffer[i]) {
					color = _paletteBuffer[i];
				}
				_currentPalette[i] = color;
			}
		}
		updatePalette(count);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	} while (m > 0);
}

void IgorEngine::fadeOutPalette(int count) {
	// debugC(9, kDebugScreen, "fadeOutPalette(%d)", count);
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	memcpy(_paletteBuffer, _currentPalette, 768);
	int m = 0;
	do {
		for (int i = 0; i < count; ++i) {
			if (_paletteBuffer[i] >= m) {
				uint8 color = _currentPalette[i];
				if (color >= 3) {
					color -= 3;
				} else {
					color = 0;
				}
				_currentPalette[i] = color;
			}
		}
		updatePalette(count);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
		m += 3;
	} while (m < 66);
}

void IgorEngine::waitForTimer(int ticks) {
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	_system->updateScreen();
	debug("Screen updated!");
	uint32 endTicks = (ticks == -1) ? _nextTimer : _system->getMillis() + ticks * 1000 / kTickDelay;
	do {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				// _inputVars[kInputEscape] = 1;
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
				// 		case Common::EVENT_KEYDOWN:
				// 			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				// 				_inputVars[kInputEscape] = 1;
				// 			} else if (ev.kbd.keycode == Common::KEYCODE_SPACE) {
				// 				_inputVars[kInputOptions] = 1;
				// 			} else if (ev.kbd.keycode == Common::KEYCODE_p) {
				// 				_inputVars[kInputPause] = 1;
				// 			} else if (ev.kbd.keycode == Common::KEYCODE_F11) {
				// 				sprintf(_saveStateDescriptions[kQuickSaveSlot], "Quicksave part %d", _currentPart);
				// 				saveGameState(kQuickSaveSlot);
				// 			} else if (ev.kbd.keycode == Common::KEYCODE_F12) {
				// 				loadGameState(kQuickSaveSlot);
				// 			}
				// 			break;
				// 		case Common::EVENT_MOUSEMOVE:
				// 			_inputVars[kInputCursorXPos] = ev.mouse.x;
				// 			_inputVars[kInputCursorYPos] = ev.mouse.y;
				// 			break;
				// 		case Common::EVENT_RBUTTONDOWN:
				// 			_inputVars[kInputSkipDialogue] = 1;
				// 			break;
				// 		case Common::EVENT_LBUTTONDOWN:
				// 			_inputVars[kInputClick] = 1;
				// 			_inputVars[kInputCursorXPos] = ev.mouse.x;
				// 			_inputVars[kInputCursorYPos] = ev.mouse.y;
				// 			break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		if (_system->getMillis() >= endTicks) {
			break;
		}
		debug("Waiting for timer: %d ms left", endTicks - _system->getMillis());
	} while (true);
	_nextTimer = _system->getMillis() + kTimerTicksCount * 1000 / kTickDelay;
	if (ticks != -1) {
		return;
	}
	_gameTicks += kTimerTicksCount;
	// if ((_gameTicks & 31) == 0) {
	// 	setCursor(_currentCursor);
	// 	_currentCursor = (_currentCursor + 1) & 3;
	// }
	// if (_game.flags & kFlagFloppy) {
	// 	updateMusic();
	// }
	if (_gameTicks == 64) {
		_gameTicks = 0;
	}
}

void IgorEngine::decodeRoomAreas(const uint8 *p, int count) {
	for (int i = 0; i < count; ++i) {
		RoomObjectArea *roa = &_roomObjectAreasTable[i];
		roa->area = *p++;
		assert((roa->area & 0x80) == 0);
		roa->object = *p++;
		roa->y1Lum = *p++;
		roa->y2Lum = *p++;
		roa->deltaLum = *p++;
	}
}

void IgorEngine::decodeRoomMask(const uint8 *p) {
	uint8 *dst = _screenLayer2;
	int sz = 320 * 144;
	while (sz != 0) {
		uint8 b = *p++;
		int len = READ_LE_UINT16(p);
		p += 2;
		if (len > sz) {
			len = sz;
		}
		memset(dst, b, len);
		dst += len;
		sz -= len;
	}
}

static void decodeRoomString(const uint8 *src, char *dst, int sz) {
	for (int i = 0; i < sz; ++i) {
		uint8 code = *src++;
		if ((code >= 0xAE && code <= 0xC7) || (code >= 0xCE && code <= 0xE7)) {
			code -= 0x6D;
		} else if (code > 0xE7) {
			switch (code) {
			case 0xE8:
				code = 0xA0;
				break;
			case 0xE9:
				code = 0x82;
				break;
			case 0xEA:
				code = 0xA1;
				break;
			case 0xEB:
				code = 0xA2;
				break;
			case 0xEC:
				code = 0xA3;
				break;
			case 0xED:
				code = 0xA4;
				break;
			case 0xEE:
				code = 0xA5;
				break;
			}
		}
		*dst++ = (char)code;
	}
}

void IgorEngine::decodeRoomStrings(const uint8 *p, bool skipObjectNames) {
	if (!skipObjectNames) {
		for (int i = 0; i < 20; ++i) {
			_roomObjectNames[i][0] = '\0';
		}
		uint8 code = *p++;
		int index = -1;
		while (code != 0xF6) {
			if (code == 0xF4) {
				++index;
			}
			int len = *p++;
			if (len != 0) {
				assert(index >= 0);
				decodeRoomString(p, _roomObjectNames[index], len);
				p += len;
				_roomObjectNames[index][len] = '\0';
				// debugC(9, kDebugResource, "decodeRoomStrings() _roomObjectNames[%d] '%s'", index, _roomObjectNames[index]);
			}
			code = *p++;
		}
	}
	for (int i = 200; i < 250; ++i) {
		_globalDialogueTexts[i][0] = '\0';
	}
	uint8 code = *p++;
	int index = 200;
	while (code != 0xF6) {
		if (code == 0xF4) {
			++index;
		}
		int len = *p++;
		if (len != 0) {
			decodeRoomString(p, _globalDialogueTexts[index], len);
			p += len;
			_globalDialogueTexts[index][len] = '\0';
			// debugC(9, kDebugResource, "decodeRoomStrings() _globalDialogueTexts[%d] '%s'", index, _globalDialogueTexts[index]);
		}
		code = *p++;
	}
}

void IgorEngine::decodeRoomText(const uint8 *p) {
	// debugC(9, kDebugResource, "decodeRoomText()");
	memcpy(_walkXScaleRoom, p, 320);
	p += 320;
	memcpy(_walkYScaleRoom, p, 432);
	p += 432;
	decodeRoomStrings(p);
}

void IgorEngine::PART_MAIN() {
	while (!g_engine->shouldQuit() && !_eventQuitGame) {
		switch (_currentPart) {

		case 50:
			PART_05(); // SpringRock
			break;

		default:
			error("Unknown or unhandled room part %d", _currentPart);
			break;
		}
	}
}

void IgorEngine::handleRoomInput() {
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
