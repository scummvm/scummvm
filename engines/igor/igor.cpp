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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "sound/voc.h"

#include "igor/igor.h"
#include "igor/midi.h"

namespace Igor {

IgorEngine::IgorEngine(OSystem *system, const DetectedGameVersion *dgv)
	: Engine(system), _game(*dgv) {

	_screenVGA = (uint8 *)malloc(320 * 200);
	for (int i = 0; i < 4; ++i) {
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

	Common::addDebugChannel(kDebugEngine,   "Engine",   "Engine debug level");
	Common::addDebugChannel(kDebugResource, "Resource", "Resource debug level");
	Common::addDebugChannel(kDebugScreen,   "Screen",   "Screen debug level");
	Common::addDebugChannel(kDebugWalk,     "Walk",     "Walk debug level");
	Common::addDebugChannel(kDebugGame,     "Game",     "Game debug level");

	if (_game.flags & kFlagFloppy) {
		_midiPlayer = new MidiPlayer(this);
	} else {
		_midiPlayer = 0;
	}
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

	Common::clearAllDebugChannels();

	delete _midiPlayer;
}

void IgorEngine::restart() {
	_screenVGAVOffset = 0;

	memset(&_gameState, 0, sizeof(_gameState));
	_nextTimer = 0;
	_fastMode = false;
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
	_musicData = 0;

	_talkDelay = _talkSpeechCounter = _talkDelayCounter = 0;
	memset(_dialogueTextsTable, 0, sizeof(_dialogueTextsTable));
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
	_actionCode = 0;
	_actionWalkPoint = 0;
	memset(_inputVars, 0, sizeof(_inputVars));
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
	initGraphics(320, 200, false);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));

	restart();
	setupDefaultPalette();
	_currentPart = ConfMan.getInt("boot_param");
	if (_currentPart == 0) {
		_currentPart = kStartupPart;
	}
	if (!_ovlFile.open(_game.ovlFileName)) {
		error("Unable to open '%s'", _game.ovlFileName);
	}
	if (!_sndFile.open(_game.sfxFileName)) {
		error("Unable to open '%s'", _game.sfxFileName);
	}
	readTableFile();
	loadMainTexts();
	loadIgorFrames();
	_gameState.talkMode = kTalkModeTextOnly;
	_gameState.talkSpeed = 3;
	_talkSpeechCounter = 5;
	_eventQuitGame = false;
	PART_MAIN();
	_ovlFile.close();
	_sndFile.close();
	return Common::kNoError;
}

void IgorEngine::readTableFile() {
	Common::File tblFile;
	uint32 stringsEntriesOffset = 0, resourcesEntriesOffset = 0, soundEntriesOffset = 0;
	if (tblFile.open("IGOR.TBL") && tblFile.readUint32BE() == MKID_BE('ITBL') && tblFile.readUint32BE() == 4) {
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

const char *IgorEngine::getString(int id) const {
	const char *str = 0;
	for (Common::Array<StringEntry>::const_iterator it = _stringEntries.begin(); it != _stringEntries.end(); ++it) {
		if ((*it).id == id) {
			str = (*it).str.c_str();
			break;
		}
	}
	return str;
}

void IgorEngine::waitForTimer(int ticks) {
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	_system->updateScreen();
	uint32 endTicks = (ticks == -1) ? _nextTimer : _system->getMillis() + ticks * 1000 / kTickDelay;
	do {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				_inputVars[kInputEscape] = 1;
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_inputVars[kInputEscape] = 1;
				} else if (ev.kbd.keycode == Common::KEYCODE_SPACE) {
					_inputVars[kInputOptions] = 1;
				} else if (ev.kbd.keycode == Common::KEYCODE_p) {
					_inputVars[kInputPause] = 1;
				} else if (ev.kbd.keycode == Common::KEYCODE_F11) {
					sprintf(_saveStateDescriptions[kQuickSaveSlot], "Quicksave part %d", _currentPart);
					saveGameState(kQuickSaveSlot);
				} else if (ev.kbd.keycode == Common::KEYCODE_F12) {
					loadGameState(kQuickSaveSlot);
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				_inputVars[kInputCursorXPos] = ev.mouse.x;
				_inputVars[kInputCursorYPos] = ev.mouse.y;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_inputVars[kInputSkipDialogue] = 1;
				break;
			case Common::EVENT_LBUTTONDOWN:
				_inputVars[kInputClick] = 1;
				_inputVars[kInputCursorXPos] = ev.mouse.x;
				_inputVars[kInputCursorYPos] = ev.mouse.y;
				break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		if (_system->getMillis() >= endTicks) {
			break;
		}
	} while (!_fastMode);
	_nextTimer = _system->getMillis() + kTimerTicksCount * 1000 / kTickDelay;
	if (ticks != -1) {
		return;
	}
	_gameTicks += kTimerTicksCount;
	if ((_gameTicks & 31) == 0) {
		setCursor(_currentCursor);
		_currentCursor = (_currentCursor + 1) & 3;
	}
	if (_game.flags & kFlagFloppy) {
		updateMusic();
	}
	if (_gameTicks == 64) {
		_gameTicks = 0;
	}
}

void IgorEngine::copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent) {
	uint8 *p = dst + dstOffset;
	for (int y = 0; y < h; ++y) {
		if (transparent) {
			for (int x = 0; x < w; ++x) {
				if (src[x] != 0) {
					p[x] = src[x];
				}
			}
		} else {
			memcpy(p, src, w);
		}
		p += dstPitch;
		src += srcPitch;
	}
}

int IgorEngine::getRandomNumber(int m) {
	assert(m > 0);
	return _rnd.getRandomNumber(m - 1);
}

void IgorEngine::startMusic(int cmf) {
	_midiPlayer->stopMusic();
	free(_musicData);
	int musicDataSize;
	_musicData = loadData(cmf, 0, &musicDataSize);
	_midiPlayer->playMusic(_musicData, musicDataSize);
}

void IgorEngine::playMusic(int num) {
	debugC(9, kDebugEngine, "playMusic() %d", num);
	if (_game.flags & kFlagFloppy) {
		static const int cmf[] = { 0, 0, CMF_2_1, CMF_3, CMF_4, 0, 0, CMF_7_1, CMF_8, CMF_9, CMF_10, CMF_11, CMF_12 };
		assert(num < ARRAYSIZE(cmf) && cmf[num] != 0);
		_gameState.musicNum = num;
		_gameState.musicSequenceIndex = 1;
		startMusic(cmf[num]);
	} else {
		// TODO: play CD track
	}
}

void IgorEngine::updateMusic() {
	static const int cmf2Seq[] = { CMF_2_1, CMF_2_2, CMF_2_3, CMF_2_4 };
	static const int cmf7Seq[] = { CMF_7_1, CMF_7_2, CMF_7_3, CMF_7_4 };
	if (_gameState.jumpToNextMusic && 0) { // TODO: enable
		switch (_gameState.musicNum) {
		case 2:
			_gameState.musicSequenceIndex = getRandomNumber(4) + 1;
			startMusic(cmf2Seq[_gameState.musicSequenceIndex - 1]);
//			_timerHandler0x1CCounter = 5;
			break;
		case 7:
			if (_gameState.musicSequenceIndex == 4) {
				_gameState.musicSequenceIndex = 1;
			} else {
				++_gameState.musicSequenceIndex;
			}
			startMusic(cmf7Seq[_gameState.musicSequenceIndex - 1]);
//			_timerHandler0x1CCounter = 5;
			break;
		case 3:
		case 4:
		case 8:
		case 9:
		case 10:
//			_timerHandler0x1CCounter = 50;
			break;
		case 11:
//			_timerHandler0x1CCounter = 5;
			break;
		}
	}
}

void IgorEngine::playSound(int num, int type) {
	debugC(9, kDebugEngine, "playSound() %d", num);
	--num;
	int soundOffset = -1;
	Audio::Mixer::SoundType soundType;
	Audio::SoundHandle *soundHandle = 0;
	if (type == 1) {
		if (_mixer->isSoundHandleActive(_sfxHandle)) {
			return;
		}
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSFXSoundType;
		soundHandle = &_sfxHandle;
	} else if (type == 0 && (_game.flags & kFlagTalkie) != 0 && num != kNoSpeechSound) {
		if (_mixer->isSoundHandleActive(_speechHandle)) {
			_mixer->stopHandle(_speechHandle);
		}
		num += 100;
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSpeechSoundType;
		soundHandle = &_speechHandle;
	} else {
		return;
	}
	_sndFile.seek(soundOffset);
	Audio::AudioStream *stream = Audio::makeVOCStream(_sndFile, Audio::Mixer::FLAG_UNSIGNED);
	if (stream) {
		_mixer->playInputStream(soundType, soundHandle, stream);
	}
}

void IgorEngine::stopSound() {
	_mixer->stopHandle(_sfxHandle);
}

void IgorEngine::loadIgorFrames() {
	loadData(FRM_IgorDirBack, _facingIgorFrames[0]);
	loadData(FRM_IgorDirRight, _facingIgorFrames[1]);
	loadData(FRM_IgorDirFront, _facingIgorFrames[2]);
	loadData(FRM_IgorDirLeft, _facingIgorFrames[3]);
	loadData(FRM_IgorHead, _igorHeadFrames);
	memcpy(_igorPalette, PAL_IGOR_1, 48);
}

void IgorEngine::loadIgorFrames2() {
	loadData(FRM_IgorDirBack2, _facingIgorFrames[0]);
	loadData(FRM_IgorDirRight2, _facingIgorFrames[1]);
	loadData(FRM_IgorDirFront2, _facingIgorFrames[2]);
	loadData(FRM_IgorDirLeft2, _facingIgorFrames[3]);
	loadData(FRM_IgorHead2, _igorHeadFrames);
	memcpy(_igorPalette, PAL_IGOR_2, 48);
}

void IgorEngine::fixDialogueTextPosition(int num, int count, int *x, int *y) {
	int textLineWidth = 0;
	for (int i = 0; i < count; ++i) {
		int w = getStringWidth(_globalDialogueTexts[num + i]);
		if (w > textLineWidth) {
			textLineWidth = w;
		}
	}
	int textX = *x;
	textX += textLineWidth / 2 - 1;
	if (textX > 318) {
		textX = 317 - textLineWidth;
	} else {
		textX -= textLineWidth;
	}
	if (textX < 1) {
		textX = textLineWidth / 2 + 1;
	} else {
		textX += textLineWidth / 2;
	}
	*x = textX;

	int textY = *y;
	textY -= count * 10;
	if (textY < 1) {
		textY = 1;
	}
	*y = textY;
}

void IgorEngine::startCutsceneDialogue(int x, int y, int r, int g, int b) {
	debugC(9, kDebugEngine, "startCutsceneDialogue() pos %d,%d color %d,%d,%d", x, y, r, g, b);
	--_dialogueTextsCount;
	int talkX = x;
	int talkY = y;
	const DialogueText *dt = &_dialogueTextsTable[_dialogueTextsStart];
	fixDialogueTextPosition(dt->num, dt->count, &talkX, &talkY);
	_dialogueDirtyRectY = talkY * 320;
	_dialogueDirtyRectSize = dt->count * 11 * 320;
	assert(_dialogueDirtyRectSize < 320 * 72);
	memcpy(_screenTextLayer, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	memcpy(_screenTextLayer + 320 * 72, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	for (int i = 0; i < dt->count; ++i) {
		const char *textLine = _globalDialogueTexts[dt->num + i];
		int textLineWidth = getStringWidth(textLine);
		int textX = talkX - textLineWidth / 2;
		int textY = i * 10;
		drawString(_screenTextLayer, textLine, textX, textY, kTalkColor, kTalkShadowColor, kTalkShadowColor);
	}
	setPaletteColor(kTalkColor, r, g, b);
	setPaletteColor(kTalkShadowColor, 0, 0, 0);
	if (_gameState.talkMode != kTalkModeSpeechOnly) {
		memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer, _dialogueDirtyRectSize);
	}
	if (_gameState.talkMode == kTalkModeTextOnly) {
		_talkDelay = (2 * dt->count) * _talkDelays[_gameState.talkSpeed];
		_talkDelayCounter = 0;
	} else {
		_talkDelay = -1;
		_talkDelayCounter = 0;
	}
	if (_gameState.talkMode == kTalkModeTextOnly) {
		playSound(24, 0);
	}
	_gameState.dialogueTextRunning = true;
	_inputVars[kInputSkipDialogue] = 0;
}

void IgorEngine::waitForEndOfCutsceneDialogue(int x, int y, int r, int g, int b) {
	do {
		if (_gameState.dialogueTextRunning && _inputVars[kInputSkipDialogue]) {
			_talkDelayCounter = _talkDelay;
			_inputVars[kInputSkipDialogue] = 0;
		}
		if (compareGameTick(19, 32) && _gameState.dialogueTextRunning) {
			if (_talkSpeechCounter > 2) {
				if (_gameState.talkMode != kTalkModeTextOnly) {
					_talkDelayCounter = _talkDelay;
				}
				if (_talkDelay == _talkDelayCounter) {
					if (_updateDialogue) {
						(this->*_updateDialogue)(kUpdateDialogueAnimEndOfSentence);
					}
					memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
					if (_dialogueTextsCount == 0) {
						_gameState.dialogueTextRunning = 0;
					} else {
						++_dialogueTextsStart;
						if (_gameState.talkMode != kTalkModeTextOnly) {
							if (_talkSpeechCounter != -1) {
								_talkSpeechCounter = 0;
							} else {
								_talkSpeechCounter = 5;
								startCutsceneDialogue(x, y, r, g, b);
							}
						} else {
							startCutsceneDialogue(x, y, r, g, b);
						}
					}
				} else {
					++_talkDelayCounter;
					if (_updateDialogue) {
						(this->*_updateDialogue)(kUpdateDialogueAnimMiddleOfSentence);
					}
				}
			} else {
				if (_talkSpeechCounter == 2) {
					startCutsceneDialogue(x, y, r, g, b);
				}
				++_talkSpeechCounter;
			}
		}
		if (_updateRoomBackground) {
			(this->*_updateRoomBackground)();
		}
		if (_inputVars[kInputEscape]) return;
		waitForTimer();
	} while (_gameState.dialogueTextRunning);
}

void IgorEngine::fixIgorDialogueTextPosition(int num, int count, int *x, int *y) {
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	*x = wd->x;
	*y = wd->y - wd->scaleWidth - 3;
	fixDialogueTextPosition(num, count, x, y);
}

void IgorEngine::startIgorDialogue() {
	debugC(9, kDebugEngine, "startIgorDialogue()");
	--_dialogueTextsCount;
	int talkX, talkY;
	const DialogueText *dt = &_dialogueTextsTable[_dialogueTextsStart];
	fixIgorDialogueTextPosition(dt->num, dt->count, &talkX, &talkY);
	_dialogueDirtyRectY = talkY * 320;
	_dialogueDirtyRectSize = dt->count * 11 * 320;
	assert(_dialogueDirtyRectSize < 320 * 72);
	memcpy(_screenTextLayer, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);

	if (_currentPart == 690) {
		memcpy(_screenTextLayer + 320 * 72, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
		if (_currentAction.verb == kVerbLook && _currentAction.object1Num == 15) {
			memcpy(_screenTextLayer + 320 * 72, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
		}
		if (_currentAction.verb == kVerbLook && _currentAction.object1Num == 19) {
			memcpy(_screenTextLayer + 320 * 72, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
		}
	} else {
		memcpy(_screenTextLayer + 320 * 72, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	for (int i = 0; i < dt->count; ++i) {
		const char *textLine = _globalDialogueTexts[dt->num + i];
		int textLineWidth = getStringWidth(textLine);
		int textX = talkX - textLineWidth / 2;
		int textY = i * 10;
		drawString(_screenTextLayer, textLine, textX, textY, kTalkColor, kTalkShadowColor, kTalkShadowColor);
	}
	setPaletteColor(kTalkColor, _dialogueColor[0], _dialogueColor[1], _dialogueColor[2]);
	setPaletteColor(kTalkShadowColor, 0, 0, 0);
	if (_gameState.talkMode != kTalkModeSpeechOnly) {
		memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer, _dialogueDirtyRectSize);
	}
	if (_gameState.talkMode == kTalkModeTextOnly) {
		_talkDelay = (2 * dt->count) * _talkDelays[_gameState.talkSpeed];
		_talkDelayCounter = 0;
	} else {
		_talkDelay = -1;
		_talkDelayCounter = 0;
	}
	if (_gameState.talkMode == kTalkModeTextOnly) {
		playSound(24, 0);
	}
	_gameState.dialogueTextRunning = true;
	_inputVars[kInputSkipDialogue] = 0;
}

void IgorEngine::waitForEndOfIgorDialogue() {
	do {
		if (_gameState.dialogueTextRunning && _inputVars[kInputSkipDialogue]) {
			_talkDelayCounter = _talkDelay;
			_inputVars[kInputSkipDialogue] = 0;
		}
		if (compareGameTick(19, 32) && _gameState.dialogueTextRunning) {
			if (_talkSpeechCounter > 2) {
				if (_gameState.talkMode != kTalkModeTextOnly) {
					_talkDelayCounter = _talkDelay;
				}
				if (_talkDelay == _talkDelayCounter) {
					animateIgorTalking(0);
					memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
					if (_dialogueTextsCount == 0) {
						_gameState.dialogueTextRunning = 0;
					} else {
						++_dialogueTextsStart;
						if (_gameState.talkMode != kTalkModeTextOnly) {
							if (_talkSpeechCounter != -1) {
								_talkSpeechCounter = 0;
							} else {
								_talkSpeechCounter = 5;
								startIgorDialogue();
							}
						} else {
							startIgorDialogue();
						}
					}
				} else {
					animateIgorTalking(getRandomNumber(6));
					++_talkDelayCounter;
				}
			} else {
				if (_talkSpeechCounter == 2) {
					startIgorDialogue();
				}
				++_talkSpeechCounter;
			}
		}
		if (_updateRoomBackground) {
			(this->*_updateRoomBackground)();
		}
		if (_inputVars[kInputEscape]) return;
		waitForTimer();
	} while (_gameState.dialogueTextRunning);
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
	debugC(9, kDebugResource, "loadData() id %d", id);
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
				debugC(9, kDebugResource, "decodeRoomStrings() _roomObjectNames[%d] '%s'", index, _roomObjectNames[index]);
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
			debugC(9, kDebugResource, "decodeRoomStrings() _globalDialogueTexts[%d] '%s'", index, _globalDialogueTexts[index]);
		}
		code = *p++;
	}
}

void IgorEngine::decodeRoomText(const uint8 *p) {
	debugC(9, kDebugResource, "decodeRoomText()");
	memcpy(_walkXScaleRoom, p, 320); p += 320;
	memcpy(_walkYScaleRoom, p, 432); p += 432;
	decodeRoomStrings(p);
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
		int len = READ_LE_UINT16(p); p += 2;
		if (len > sz) {
			len = sz;
		}
		memset(dst, b, len);
		dst += len;
		sz -= len;
	}
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

void IgorEngine::loadAnimData(const int *anm, int loadOffset) {
	if (loadOffset == 0) {
		memset(_animFramesBuffer, 0, 65535);
	}
	while (*anm) {
		int dataSize;
		loadData(*anm++, &_animFramesBuffer[loadOffset], &dataSize);
		loadOffset += dataSize;
	}
}

void IgorEngine::loadActionData(int act) {
	if (act != 0) {
		assert(findData(act)->size <= 0x2000);
		loadData(act, _roomActionsTable);
	}
}

void IgorEngine::loadDialogueData(int dlg) {
	uint8 *p = loadData(dlg);
	int dialogueDataSize = _roomDataOffsets.dlg.questionsOffset + 164 + 41;
	assert(dialogueDataSize <= 500);
	memcpy(_gameState.dialogueData, p, dialogueDataSize);
	assert(_roomDataOffsets.dlg.questionsSize <= MAX_DIALOGUE_QUESTIONS);
	for (int i = 0; i < _roomDataOffsets.dlg.questionsSize; ++i) {
		for (int n = 0; n < 2; ++n) {
			const uint8 *src = p + _roomDataOffsets.dlg.questionsOffset + (i + 1) * 164 + _language * 82 + (n + 1) * 41;
			int len = *src++;
			if (len != 0) {
				decodeRoomString(src, _dialogueQuestions[i][n], len);
				_dialogueQuestions[i][n][len] = '\0';
				debugC(9, kDebugResource, "loadDialogueData() _dialogueQuestions[%d][%d] '%s'", i, n, _dialogueQuestions[i][n]);
			} else {
				_dialogueQuestions[i][n][0] = '\0';
			}
		}
	}
	assert(_roomDataOffsets.dlg.repliesSize <= MAX_DIALOGUE_REPLIES);
	for (int i = 0; i < _roomDataOffsets.dlg.repliesSize; ++i) {
		const uint8 *src = p + _roomDataOffsets.dlg.repliesOffset + (i + 1) * 102 + _language * 51;
		int len = *src++;
		if (len != 0) {
			decodeRoomString(src, _dialogueReplies[i], len);
			_dialogueReplies[i][len] = '\0';
			debugC(9, kDebugResource, "loadDialogueData() _dialogueReplies[%d] '%s'", i, _dialogueReplies[i]);
		} else {
			_dialogueReplies[i][0] = '\0';
		}
	}
	free(p);
}

static void decodeMainString(const uint8 *src, char *dst) {
	int sz = *src - 0x6D;
	if (sz != 0) {
		++src;
		for (int i = 0; i < sz; ++i) {
			uint8 code = src[i] - 0x6D;
			dst[i] = (char)code;
		}
	}
	dst[sz] = '\0';
}

void IgorEngine::loadMainTexts() {
	loadData(IMG_VerbsPanel, _verbsPanelBuffer);
	if (_game.version == kIdSpaCD) {
		const struct {
			int strId;
			int x;
		} verbTexts[] = {
			{ STR_Talk,   21 },
			{ STR_Take,   67 },
			{ STR_Look,  113 },
			{ STR_Use,   159 },
			{ STR_Open,  205 },
			{ STR_Close, 251 },
			{ STR_Give,  297 }
		};
		for (int i = 0; i < 7; ++i) {
			const char *s = getString(verbTexts[i].strId);
			int x = verbTexts[i].x - getStringWidth(s) / 2;
			drawString(_verbsPanelBuffer, s, x, 0, 0xF2, -1, 0);
		}
	}
	int dataSize;
	uint8 *p = loadData(TXT_MainTable, 0, &dataSize);
	const uint8 *src = &p[0] + _language * 7;
	for (int i = 0; i < 3; ++i, src += 7 * 2) {
		decodeMainString(src, _verbPrepositions[i]);
		debugC(9, kDebugResource, "loadMainTexts() _verbPrepositions[%d] '%s'", i, _verbPrepositions[i]);
	}
	src = &p[0x2A] + _language * 31;
	for (int i = 0; i < 35; ++i, src += 31 * 2) {
		decodeMainString(src, _globalObjectNames[i]);
		debugC(9, kDebugResource, "loadMainTexts() _globalObjectNames[%d] '%s'", i, _globalObjectNames[i]);
	}
	src = &p[0x8BA] + _language * 51;
	for (int i = 0; i < 250; ++i, src += 51 * 2) {
		decodeMainString(src, _globalDialogueTexts[i]);
		debugC(9, kDebugResource, "loadMainTexts() _globalDialogueTexts[%d] '%s'", i, _globalDialogueTexts[i]);
	}
	src = &p[0x6CA4] + _language * 12;
	for (int i = 0; i < 9; ++i, src += 12 * 2) {
		decodeMainString(src, _verbsName[i]);
		debugC(9, kDebugResource, "loadMainTexts() _verbsName[%d] '%s'", i, _verbsName[i]);
	}
	free(p);
}

void IgorEngine::setupDefaultPalette() {
	memset(_currentPalette, 0, 255 * 3);
	memset(&_currentPalette[255 * 3], 63, 3);
	updatePalette(768);
}

void IgorEngine::updatePalette(int count) {
	assert(count <= 768);
	uint8 pal[1024];
	for (int j = 0, i = 0; i < count; ++i) {
		pal[j++] = (_currentPalette[i] << 2) | (_currentPalette[i] >> 4);
		if (((i + 1) % 3) == 0) {
			pal[j++] = 0;
		}
	}
	_system->setPalette(pal, 0, count / 3);
}

void IgorEngine::clearPalette() {
	memset(_currentPalette, 0, 768);
	updatePalette(768);
}

void IgorEngine::setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b) {
	uint8 pal[4];
	pal[0] = (r << 2) | (r >> 4);
	pal[1] = (g << 2) | (g >> 4);
	pal[2] = (b << 2) | (b >> 4);
	pal[3] = 0;
	_system->setPalette(pal, index, 1);
}

void IgorEngine::setPaletteRange(int startColor, int endColor) {
	debugC(9, kDebugScreen, "setPaletteRange(%d, %d)", startColor, endColor);
	assert(endColor - startColor + 1 <= 256);
	for (int i = startColor; i <= endColor; ++i) {
		setPaletteColor(i, _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
	}
}

void IgorEngine::fadeInPalette(int count) {
	debugC(9, kDebugScreen, "fadeInPalette(%d)", count);
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
	debugC(9, kDebugScreen, "fadeOutPalette(%d)", count);
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

void IgorEngine::scrollPalette(int startColor, int endColor) {
	debugC(9, kDebugScreen, "scrollPalette(%d, %d)", startColor, endColor);
	uint8 c[3];
	memcpy(c, &_currentPalette[startColor * 3], 3);
	memmove(&_currentPalette[startColor * 3], &_currentPalette[(startColor + 1) * 3], (endColor - startColor) * 3);
	memcpy(&_currentPalette[endColor * 3], c, 3);
}

void IgorEngine::drawChar(uint8 *dst, int chr, int x, int y, int color1, int color2, int color3) {
	dst += y * 320 + x;
	for (int j = 0; j < 11; ++j, dst += 320) {
		uint32 chrLineMask = _fontData[chr * 11 + j];
		for (int i = 0; i < 9; ++i, chrLineMask >>= 2) {
			switch (chrLineMask & 3) {
			case 1:
				dst[i] = color1;
				break;
			case 2:
				if (color2 != -1) {
					dst[i] = color2;
				}
				break;
			case 3:
				if (color3 != -1) {
					dst[i] = color3;
				}
				break;
			}
		}
	}
}

void IgorEngine::drawString(uint8 *dst, const char *s, int x, int y, int color1, int color2, int color3) {
	for (; *s; ++s) {
		if (*s == ' ') {
			x += 5;
		} else {
			int chr = _fontCharIndex[(uint8)*s];
			if (chr == 99) {
				continue;
			}
			if (x + _fontCharWidth[chr] > 320) {
				break;
			}
			drawChar(dst, chr, x, y, color1, color2, color3);
			x += _fontCharWidth[chr];
		}
	}
}

int IgorEngine::getStringWidth(const char *s) const {
	int w = 0;
	for (; *s; ++s) {
		if (*s == ' ') {
			w += 5;
		} else {
			int chr = _fontCharIndex[(uint8)*s];
			if (chr == 99) {
				continue;
			}
			w += _fontCharWidth[chr];
		}
	}
	return w;
}

void IgorEngine::drawActionSentence(const char *sentence, uint8 color) {
	memset(_screenVGA + 144 * 320, 0, 11 * 320);
	int w = getStringWidth(sentence);
	int x = (320 - w) / 2;
	drawString(_screenVGA, sentence, x, 144, color, 0, 0);
}

void IgorEngine::formatActionSentence(uint8 color) {
	char actionSentence[512];
	if (_currentAction.verb == kVerbWalk && _inputVars[kInputCursorYPos] > 143) {
		_currentAction.object1Num = 0;
	}
	strcpy(actionSentence, _verbsName[_currentAction.verb]);
	if (_currentAction.object1Num != 0) {
		if (_currentAction.object1Type == kObjectTypeInventory) {
			strcat(actionSentence, _globalObjectNames[_currentAction.object1Num]);
		} else {
			strcat(actionSentence, _roomObjectNames[_currentAction.object1Num]);
		}
		if (_currentAction.verbType != 0) {
			strcat(actionSentence, _verbPrepositions[_currentAction.verbType]);
			if (_currentAction.object2Num != 0) {
				if (_currentAction.object2Type == kObjectTypeInventory) {
					strcat(actionSentence, _globalObjectNames[_currentAction.object2Num]);
				} else {
					strcat(actionSentence, _roomObjectNames[_currentAction.object2Num]);
				}
			}
		}
	}
	drawActionSentence(actionSentence, _sentenceColorIndex[color]);
}

const uint8 *IgorEngine::getAnimFrame(int baseOffset, int tableOffset, int frame) {
	const uint8 *src = _animFramesBuffer + baseOffset;
	assert(frame >= 1);
	int frameOffset = READ_LE_UINT16(src + tableOffset + (frame - 1) * 2);
	return src + frameOffset - 1;
}

void IgorEngine::decodeAnimFrame(const uint8 *src, uint8 *dst, bool preserveText) {
	int y = READ_LE_UINT16(src) * 320; src += 2;
	int h = READ_LE_UINT16(src); src += 2;
	while (h--) {
		int w = *src++;
		int pos = y;
		while (w--) {
			pos += *src++;
			int len = *src++;
			if (len & 0x80) {
				uint8 color = *src++;
				len = 256 - len;
				if (preserveText) {
					for (int i = pos; i < pos + len; ++i) {
						if (dst[i] != kTalkColor && dst[i] != kTalkShadowColor) {
							dst[i] = color;
						}
					}
				} else {
					memset(dst + pos, color, len);
				}
				pos += len;
			} else {
				if (preserveText) {
					for (int i = pos; i < pos + len; ++i) {
						if (dst[i] != kTalkColor && dst[i] != kTalkShadowColor) {
							dst[i] = src[i - pos];
						}
					}
				} else {
					memcpy(dst + pos, src, len);
				}
				src += len;
				pos += len;
			}
		}
		y += 320;
	}
}

void IgorEngine::copyAnimFrame(int srcOffset, int frame, int frameSize, int w, int h, int dstOffset) {
	for (int y = 0; y <= h; ++y) {
		memcpy(_screenLayer1 + y * 320 + dstOffset, _animFramesBuffer + frame * frameSize + y * w + srcOffset, w);
	}
}

void IgorEngine::setCursor(int num) {
	uint8 cursor[11 * 11];
	memset(cursor, 0, 11 * 11);
	const uint8 *mask = &_mouseCursorMask[num * 24];
	for (int i = 0; i < 24; ++i) {
		if (mask[i]) {
			const int offset = ((int8)_mouseCursorData[i + 24] + 5) * 11 + ((int8)_mouseCursorData[i] + 5);
			cursor[offset] = 255;
		}
	}
	CursorMan.replaceCursor(cursor, 11, 11, 5, 5, 0);
}

void IgorEngine::showCursor() {
	debugC(9, kDebugEngine, "showCursor()");
	_roomCursorOn = true;
	CursorMan.showMouse(_roomCursorOn);
}

void IgorEngine::hideCursor() {
	debugC(9, kDebugEngine, "hideCursor()");
	_roomCursorOn = false;
	CursorMan.showMouse(_roomCursorOn);
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

void IgorEngine::drawVerbsPanel() {
	memcpy(_screenVGA + 320 * 156, _verbsPanelBuffer, 320 * 12);
}

void IgorEngine::redrawVerb(uint8 verb, bool highlight) {
	uint8 verbBitmap[44 * 12];
	if (verb >= 2 && verb <= 8) {
		verb -= 2;
		for (int i = 0; i <= 11; ++i) {
			for (int j = 0; j <= 43; ++j) {
				uint8 color = _verbsPanelBuffer[i * 320 + verb * 46 + j];
				if (highlight && color != 0) {
					color += 8;
				}
				verbBitmap[i * 44 + j] = color;
			}
		}
		for (int i = 0; i <= 11; ++i) {
			memcpy(_screenVGA + 320 * 156 + verb * 46 + i * 320, verbBitmap + i * 44, 44);
		}
	}
}

void IgorEngine::drawInventory(int start, int mode) {
	loadData(IMG_InventoryPanel, _inventoryPanelBuffer);
	loadData(IMG_Objects, _inventoryImagesBuffer);
	int y, i;
	int end = start + 6;
	int x = 1;
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
	case 0:
		memcpy(_screenVGA + 54400, _inventoryPanelBuffer, 9600);
		_scrollInventory = false;
		break;
	case 1:
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
	case 2:
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

void IgorEngine::packInventory() {
	for (int i = 1; i <= _inventoryInfo[73]; ++i) {
		if (_inventoryImages[i - 1] != 0) {
			continue;
		}
		int count = _inventoryInfo[73] - 1;
		for (int index = i; index <= count; ++index) {
			_inventoryImages[index - 1] = _inventoryImages[index];
			_inventoryImages[_inventoryImages[index - 1] - 1] = index;
		}
		_inventoryImages[_inventoryInfo[73] - 1] = 0;
		--_inventoryInfo[73];
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

void IgorEngine::addObjectToInventory(int object, int index) {
	++_inventoryInfo[73];
	_inventoryInfo[_inventoryInfo[73] - 1] = object;
	_inventoryInfo[index] = _inventoryInfo[73];
	_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	drawInventory(_inventoryInfo[72], 0);
	playSound(51, 1);
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

void IgorEngine::executeAction(int action) {
	debugC(9, kDebugEngine, "executeAction %d", action);
	assert(action < 200);
	if (action <= 100) {
		(this->*_executeMainAction)(action);
	} else {
		(this->*_executeRoomAction)(action);
	}
}

void IgorEngine::clearAction() {
	redrawVerb(_currentAction.verb, false);
	memset(&_currentAction, 0, sizeof(_currentAction));
	_currentAction.verb = kVerbWalk;
	_actionCode = 0;
	_actionWalkPoint = 0;
}

void IgorEngine::handleRoomInput() {
	if (_inputVars[kInputPause]) {
		_inputVars[kInputPause] = 0;
		handlePause();
	}
	if (_inputVars[kInputOptions]) {
		_inputVars[kInputOptions] = 0;
		handleOptionsMenu();
	}
	if (_inputVars[kInputSkipDialogue] && _gameState.dialogueTextRunning) {
		_talkDelayCounter = _talkDelay;
		if (_gameState.talkMode != kTalkModeTextOnly && _talkSpeechCounter > 2) {
			stopSound();
			_talkSpeechCounter = -1;
		}
		_inputVars[kInputSkipDialogue] = 0;
	}
	if (!_roomCursorOn || _gameState.dialogueTextRunning || _scrollInventory) {
		return;
	}
	if (_inputVars[kInputCursorYPos] >= 156 && _inputVars[kInputCursorYPos] <= 167) {
		if (_inputVars[kInputClick]) {
			int verb = getVerbUnderCursor(_inputVars[kInputCursorXPos]);
			if (verb != _currentAction.verb) {
				redrawVerb(_currentAction.verb, false);
				_currentAction.verb = verb;
				redrawVerb(_currentAction.verb, true);
			}
			// reset action command
			memset(&_currentAction, 0, sizeof(_currentAction));
			_currentAction.verb = verb;
			_currentAction.verbType = 0;
			formatActionSentence(0);
			_inputVars[kInputClick] = 0;
		}
		return;
	}
	if (_inputVars[kInputCursorYPos] >= 172 && _inputVars[kInputCursorYPos] <= 183 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
		if (_inputVars[kInputClick]) {
			if (_inventoryInfo[72] > 1) {
				_inventoryInfo[72] -= 7;
				drawInventory(_inventoryInfo[72], 2);
			}
			_inputVars[kInputClick] = 0;
		}
		return;
	}
	if (_inputVars[kInputCursorYPos] >= 186 && _inputVars[kInputCursorYPos] <= 197 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
		if (_inputVars[kInputClick]) {
			if (_inventoryInfo[73] > _inventoryInfo[72] + 6) {
				_inventoryInfo[72] += 7;
				drawInventory(_inventoryInfo[72], 1);
			}
			_inputVars[kInputClick] = 0;
		}
		return;
	}

/*	if (_inputVars[kInputClick]) {
		if (_gameState.igorMoving) {
			_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex - 1].x;
			_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex - 1].y;
			if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX].area == 0) {
				return;
			}
			_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex + 1].x;
			_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex + 1].y;
			if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX].area == 0) {
				return;
			}
		}
		_inputVars[kInputClick] = 0;
	}*/

	bool actionHovering = !_inputVars[kInputClick];
	_inputVars[kInputClick] = 0;

	if (actionHovering && _actionCode != 0) {
		return;
	}

	//Action previousAction = _currentAction;
	if (_inputVars[kInputCursorYPos] >= 170 && _inputVars[kInputCursorYPos] <= 199) {
		int object = getObjectFromInventory(_inputVars[kInputCursorXPos]);
		if (_currentAction.verbType == 0) {
			_currentAction.object1Num = object;
			_currentAction.object1Type = kObjectTypeInventory;
			if (_currentAction.verb == kVerbUse && _roomActionsTable[_roomDataOffsets.action.useVerb + 10 + _currentAction.object1Num] != 0) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 1;
				}
				return;
			}
			if (_currentAction.verb == kVerbGive && _roomActionsTable[_roomDataOffsets.action.giveVerb + 10 + _currentAction.object1Num] != 0) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 2;
				}
				return;
			}
		} else {
			_currentAction.object2Num = object;
			_currentAction.object2Type = kObjectTypeInventory;
		}
	} else if (_inputVars[kInputCursorYPos] < 144) {
		int area = _screenLayer2[_inputVars[kInputCursorYPos] * 320 + _inputVars[kInputCursorXPos]];
		int object = _roomObjectAreasTable[area].object;
		if (_currentAction.verbType == 0) {
			_currentAction.object1Num = object;
			_currentAction.object1Type = kObjectTypeRoom;
			if (_currentAction.verb == kVerbUse && _roomActionsTable[_roomDataOffsets.action.useVerb + 48 + _currentAction.object1Num] != 0) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 1;
				}
				return;
			}
			if (_currentAction.verb == kVerbGive && _roomActionsTable[_roomDataOffsets.action.giveVerb + 48 + _currentAction.object1Num] != 0) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 2;
				}
				return;
			}
		} else {
			_currentAction.object2Num = object;
			_currentAction.object2Type = kObjectTypeRoom;
		}
	} else {
		return;
	}

	if (_currentAction.verbType == 0) {
		if (_currentAction.object1Type == kObjectTypeInventory) {
			_actionCode = _inventoryActionsTable[(_currentAction.verb - 1) * 2 + _currentAction.object1Num * 20];
		} else {
			_actionCode = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + _currentAction.object1Num * 20];
		}
	}
	if (_currentAction.verbType == 1) {
		int offset = _roomActionsTable[_roomDataOffsets.action.object2 + _currentAction.object2Num + _currentAction.object2Type * 38] * 2;
		offset += _roomActionsTable[_roomDataOffsets.action.object1 + _currentAction.object1Num + _currentAction.object1Type * 38] * _roomDataOffsets.action.objectSize;
		_actionCode = _roomActionsTable[_roomDataOffsets.action.useVerb + offset];
	}
	if (_currentAction.verbType == 2) {
		int offset = _roomActionsTable[_roomDataOffsets.action.object2 + _currentAction.object2Num + _currentAction.object2Type * 38] * 2;
		offset += _roomActionsTable[_roomDataOffsets.action.object1 + _currentAction.object1Num + _currentAction.object1Type * 38] * _roomDataOffsets.action.objectSize;
		_actionCode = _roomActionsTable[_roomDataOffsets.action.giveVerb + offset];
	}

	if (actionHovering) {
		formatActionSentence(0);
		_currentAction.object2Num = 0;
		_actionCode = 0;
		return;
	}
	debugC(9, kDebugEngine, "handleRoomInput() actionCode %d", _actionCode);
	if (_actionCode == 0) {
		clearAction();
		return;
	}

	formatActionSentence(1);
	if (_currentAction.verbType == 0) {
		if (_currentAction.object1Type == kObjectTypeRoom) {
			_actionWalkPoint = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + _currentAction.object1Num * 20 + 1];
			if (_actionWalkPoint > 0) {
				if (_currentAction.object1Num == 0) {
					// no object selected, just walk
					_walkToObjectPosX = _inputVars[kInputCursorXPos];
					_walkToObjectPosY = _inputVars[kInputCursorYPos];
					if (_roomObjectAreasTable[_screenLayer2[_walkToObjectPosY * 320 + _walkToObjectPosX]].area == 0) {
						fixWalkPosition(&_walkToObjectPosX, &_walkToObjectPosY);
					}
				} else {
					// walk to object
					int offset = READ_LE_UINT16(_roomActionsTable + _roomDataOffsets.obj.walkPoints + _currentAction.object1Num * 2);
					_walkToObjectPosX = offset % 320;
					_walkToObjectPosY = offset / 320;
					debugC(9, kDebugEngine, "handleRoomInput() walkToObject offset %d (0x%X)", offset, _roomDataOffsets.obj.walkPoints);
				}
				if (_gameState.igorMoving) {
					// stop igor at the current position
					_walkDataLastIndex = _walkDataCurrentIndex - 1;
					_walkDataCurrentPosX = _walkData[_walkDataLastIndex].x;
					_walkDataCurrentPosY = _walkData[_walkDataLastIndex].y;
					_walkCurrentFrame = _walkData[_walkDataLastIndex].frameNum;
					_walkCurrentPos = _walkData[_walkDataLastIndex].posNum;
					WalkData::setNextFrame(_walkCurrentPos, _walkCurrentFrame);
				} else {
					--_walkDataLastIndex;
					_walkDataCurrentPosX = _walkData[_walkDataLastIndex].x;
					_walkDataCurrentPosY = _walkData[_walkDataLastIndex].y;
					_walkCurrentPos = _walkData[_walkDataLastIndex].posNum;
					_walkCurrentFrame = 1;
				}
				if (_walkDataCurrentPosX != _walkToObjectPosX || _walkDataCurrentPosY != _walkToObjectPosY) {
					if (_roomDataOffsets.area.boxSize == 0) {
						buildWalkPathSimple(_walkDataCurrentPosX, _walkDataCurrentPosY, _walkToObjectPosX, _walkToObjectPosY);
					} else {
						buildWalkPath(_walkDataCurrentPosX, _walkDataCurrentPosY, _walkToObjectPosX, _walkToObjectPosY);
					}
					if (_actionWalkPoint != 3) {
						_walkCurrentFrame = 0;
						_walkData[_walkDataLastIndex].frameNum = 0;
					}
					if (_actionWalkPoint == 1) {
						_walkCurrentPos = _roomActionsTable[_roomDataOffsets.obj.walkFacingPosition + _currentAction.object1Num];
						_walkData[_walkDataLastIndex].posNum = _walkCurrentPos;
					}
					_walkDataCurrentIndex = 1;
					_gameState.igorMoving = true;
				}
				return;
			}
		}
		hideCursor();
		executeAction(_actionCode);
		if (!_gameState.dialogueTextRunning) {
			showCursor();
		}
		clearAction();
		return;
	}
}

void IgorEngine::animateIgorTalking(int frame) {
	if (getPart() == 4) {
		return;
	}
	if (getPart() == 85) {
		PART_85_HELPER_6(frame);
		return;
	}
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int y = (wd->y - wd->scaleWidth + 1) * 320;
	int delta = wd->x - _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (delta > 0) {
		y += delta;
	}
	for (int yOffset = 0; yOffset < wd->scaleWidth; y += 320, ++yOffset) {
		int index = READ_LE_UINT16(_walkScaleTable + 0x6CE + wd->scaleHeight * 2) + yOffset;
		uint8 yScale = _walkScaleTable[index];
		if (yScale >= 11) {
			continue;
		}
		for (int x = 0, xOffset = wd->clipSkipX - 1; x < wd->clipWidth; ++x, ++xOffset) {
			index = READ_LE_UINT16(_walkScaleTable + 0x734 + _walkWidthScaleTable[wd->scaleHeight - 1] * 2) + xOffset;
			uint8 xScale = _walkScaleTable[0x4FC + index];
			if (xScale < 8 || xScale > 21) {
				continue;
			}
			uint8 screenColor = _screenVGA[y + x];
			if (screenColor < kTalkColor || screenColor > kTalkShadowColor) {
				int offset = yScale * 14 + frame * 154 + (wd->posNum - 1) * 924 + (xScale - 8);
				uint8 srcColor = _igorHeadFrames[offset];
				if (srcColor == 0) {
					_screenVGA[y + x] = _screenLayer1[y + x];
					continue;
				}
				RoomObjectArea *roa = &_roomObjectAreasTable[_screenLayer2[y + x]];
				if (wd->y <= roa->y1Lum) {
					_screenVGA[y + x] = _screenLayer1[y + x];
					continue;
				}
				if (wd->y <= roa->y2Lum && _gameState.enableLight == 1) {
					srcColor -= roa->deltaLum;
				}
				_screenVGA[y + x] = srcColor;
			}
		}
	}
}

void IgorEngine::handleRoomDialogue() {
	if (_gameState.dialogueTextRunning) {
		if (_talkDelayCounter == _talkDelay) {
			animateIgorTalking(0);
			memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
			if (_dialogueTextsCount == 0) {
				_gameState.dialogueTextRunning = false;
				showCursor();
			} else {
				++_dialogueTextsStart;
				startIgorDialogue();
			}
		} else {
			animateIgorTalking(getRandomNumber(6));
			++_talkDelayCounter;
		}
	}
}

void IgorEngine::handleRoomIgorWalk() {
	if (_walkDataCurrentIndex > _walkDataLastIndex) {
		_gameState.igorMoving = false;
		_walkDataLastIndex = _walkDataCurrentIndex;
		if (_actionCode > 0) {
			hideCursor();
			executeAction(_actionCode);
			if (!_gameState.dialogueTextRunning) {
				showCursor();
			}
			clearAction();
		}
	}
	if (_gameState.igorMoving) {
		moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
		++_walkDataCurrentIndex;
	}
}

void IgorEngine::handleRoomInventoryScroll() {
	if (_scrollInventory) {
		scrollInventory();
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

void IgorEngine::enterPartLoop() {
	if (!_gameState.dialogueTextRunning) {
		showCursor();
	}
	_gameState.igorMoving = false;
	if (_game.version == kIdEngDemo110) {
		CHECK_FOR_END_OF_DEMO();
	}
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

int IgorEngine::lookupScale(int xOffset, int yOffset, int h) const {
	int index = READ_LE_UINT16(_walkScaleTable + 0x734 + _walkWidthScaleTable[h - 1] * 2);
	int offset = _walkScaleTable[0x4FC + index + xOffset];
	index = READ_LE_UINT16(_walkScaleTable + 0x6CE + h * 2);
	offset += _walkScaleTable[index + yOffset] * 30;
	return offset;
}

void IgorEngine::moveIgor(int pos, int frame) {
	assert(_gameState.enableLight == 1 || _gameState.enableLight == 2);
	debugC(9, kDebugWalk, "moveIgorHelper _walkDataCurrentIndex %d pos %d frame %d", _walkDataCurrentIndex, pos, frame);
	WalkData *wd = &_walkData[_walkDataCurrentIndex];
	uint8 _walkClipSkipX = wd->clipSkipX;
	uint8 _walkHeightScale = wd->scaleHeight;
	int16 _walkClipWidth = wd->clipWidth;
	uint16 _walkScaleWidth = wd->scaleWidth;
	uint8 _walkXPosChanged = wd->xPosChanged;
	int16 _walkDxPos = wd->dxPos + 1;
	uint8 _walkYPosChanged = wd->yPosChanged;
	int16 _walkDyPos = wd->dyPos;
	int16 _walkDataCurrentPosX2 = wd->x;
	int16 _walkDataCurrentPosY2 = wd->y;

	uint16 _walkDataDrawOffset = (wd->y - wd->scaleWidth + 1) * 320;

	int xPos = _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (wd->x > xPos) {
		_walkDataDrawOffset += wd->x - xPos;
	}
	if (_walkXPosChanged != 0) {
		_walkDataDrawOffset -= _walkDxPos;
	}
	if (_walkYPosChanged != 0) {
		_walkDataDrawOffset -= _walkDyPos * 320;
	}
	if (_gameState.enableLight == 2) {
		int8 colorLum = _roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY2 * 320 + _walkDataCurrentPosX2]].y2Lum;
		if (_gameState.colorLum != colorLum) {
			for (int color = 192 * 3; color <= 207 * 3; ++color) {
				int c = _currentPalette[color] + colorLum;
				if (c < 1) {
					c = 0;
				} else if (c > 62) {
					c = 63;
				}
				_currentPalette[color] = c;
			}
			setPaletteRange(192, 207);
			_gameState.colorLum = colorLum;
		}
	}
	uint16 screenIgorDrawOffset = _walkDataDrawOffset;
	uint16 igorScaledWidth = _walkDxPos + _walkClipWidth;
	uint16 igorScaledHeight = _walkHeightScale + _walkDyPos;
	uint16 igorBodyScanLine = 0;
	if (_walkYPosChanged != 0) {
		for (int i = 1; i <= _walkDyPos; ++i) {
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, igorScaledWidth);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	if (_walkXPosChanged != 0) {
		for (int yOffset = 0; yOffset < _walkScaleWidth; ++yOffset) {
			assert(_walkDxPos > 0);
			assert(igorBodyScanLine * 50 + _walkDxPos <= 3000);
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, _walkDxPos);
			int xOffset = _walkClipSkipX - 1;
			for (int i = 0; i < _walkClipWidth; ++i) {
				int offset = lookupScale(xOffset, yOffset, _walkHeightScale);
				offset += frame * 1500;
				uint8 color = _facingIgorFrames[pos - 1][offset];
				if (color != 0) {
					assert(_walkDataDrawOffset + _walkDxPos + i >= 0);
					int index = _screenLayer2[_walkDataDrawOffset + _walkDxPos + i];
					int yPos = _roomObjectAreasTable[index].y1Lum;
					if (wd->y <= yPos) {
						_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = _screenLayer1[_walkDataDrawOffset + _walkDxPos + i];
					} else {
						if (_gameState.enableLight == 1 && wd->y <= _roomObjectAreasTable[index].y2Lum) {
							color -= _roomObjectAreasTable[index].deltaLum;
						}
						_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = color;
					}
				} else {
					_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = _screenLayer1[_walkDataDrawOffset + _walkDxPos + i];
				}
				++xOffset;
			}
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	} else {
		for (int yOffset = 0; yOffset < _walkScaleWidth; ++yOffset) {
			int xOffset = _walkClipSkipX - 1;
			for (int i = 0; i < _walkClipWidth; ++i) {
				int offset = lookupScale(xOffset, yOffset, _walkHeightScale);
				offset += frame * 1500;
				uint8 color = _facingIgorFrames[pos - 1][offset];
				if (color != 0) {
					assert(_walkDataDrawOffset + i >= 0);
					int index = _screenLayer2[_walkDataDrawOffset + i];
					int yPos = _roomObjectAreasTable[index].y1Lum;
					if (wd->y <= yPos) {
						_igorTempFrames[igorBodyScanLine * 50 + i] = _screenLayer1[_walkDataDrawOffset + i];
					} else {
						if (_gameState.enableLight == 1 && wd->y <= _roomObjectAreasTable[index].y2Lum) {
							color -= _roomObjectAreasTable[index].deltaLum;
						}
						_igorTempFrames[igorBodyScanLine * 50 + i] = color;
					}
				} else {
					_igorTempFrames[igorBodyScanLine * 50 + i] = _screenLayer1[_walkDataDrawOffset + i];
				}
				++xOffset;
			}
			const uint8 *src = _screenLayer1 + _walkDataDrawOffset + _walkClipWidth;
			memcpy(_igorTempFrames + igorBodyScanLine * 50 + _walkClipWidth, src, _walkDxPos);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	if (_walkYPosChanged == 0) {
		for (int i = 1; i <= _walkDyPos; ++i) {
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, igorScaledWidth);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	for (igorBodyScanLine = 0; igorBodyScanLine < igorScaledHeight; ++igorBodyScanLine) {
		assert(screenIgorDrawOffset + igorScaledWidth <= 320 * 200);
		memcpy(_screenVGA + screenIgorDrawOffset, _igorTempFrames + igorBodyScanLine * 50, igorScaledWidth);
		screenIgorDrawOffset += 320;
	}
}

void IgorEngine::buildWalkPathSimple(int srcX, int srcY, int dstX, int dstY) {
	debugC(9, kDebugWalk, "IgorEngine::buildWalkPathSimple(%d, %d, %d, %d)", srcX, srcY, dstX, dstY);
	if (srcX != dstX || srcY != dstY) {
		_walkData[0] = _walkData[_walkDataLastIndex];
		_walkDataLastIndex = 1;
		buildWalkPathArea(srcX, srcY, dstX, dstY);
		--_walkDataLastIndex;
	}
}

void IgorEngine::getClosestAreaTrianglePoint(int dstArea, int srcArea, int *dstY, int *dstX, int srcY, int srcX) {
	int minSqrDist = -1;
	assert(dstArea >= 1 && srcArea >= 1);
	const uint8 *p = _roomActionsTable + ((dstArea - 1) + (srcArea - 1) * _roomDataOffsets.area.boxSize) * 6;
	for (int i = 0; i < 3; ++i) {
		const uint16 offset = READ_LE_UINT16(p + i * 2);
		const int yPos = offset / 320;
		const int xPos = offset % 320;
		int y = srcY - yPos;
		int x = srcX - xPos;
		int sqrDist = y * y + x * x;
		if (minSqrDist == -1 || sqrDist < minSqrDist) {
			*dstY = yPos;
			*dstX = xPos;
			minSqrDist = sqrDist;
		}
	}
	debugC(9, kDebugWalk, "getClosestAreaTrianglePoint() sqrDist %d pos %d,%d", minSqrDist, *dstX, *dstY);
}

void IgorEngine::getClosestAreaTrianglePoint2(int dstArea, int srcArea, int *dstY, int *dstX, int srcY1, int srcX1, int srcY2, int srcX2) {
	int minSqrDist = -1;
	assert(dstArea >= 1 && srcArea >= 1);
	const uint8 *p = _roomActionsTable + ((dstArea - 1) + (srcArea - 1) * _roomDataOffsets.area.boxSize) * 6;
	for (int i = 0; i < 3; ++i) {
		const uint16 offset = READ_LE_UINT16(p + i * 2);
		const int yPos = offset / 320;
		const int xPos = offset % 320;
		int y1 = srcY1 - yPos;
		int x1 = srcX1 - xPos;
		int y2 = srcY2 - yPos;
		int x2 = srcX2 - xPos;
		int sqrDist = y1 * y1 + x1 * x1 + y2 * y2 + x2 * x2;
		if (minSqrDist == -1 || sqrDist < minSqrDist) {
			*dstY = yPos;
			*dstX = xPos;
			minSqrDist = sqrDist;
		}
	}
	debugC(9, kDebugWalk, "getClosestAreaTrianglePoint2() sqrDist %d pos %d,%d", minSqrDist, *dstX, *dstY);
}

void IgorEngine::buildWalkPath(int srcX, int srcY, int dstX, int dstY) {
	if (srcX != dstX || srcY != dstY) {
		_walkData[0] = _walkData[_walkDataLastIndex];
		_walkDataLastIndex = 1;
		int srcArea = _roomObjectAreasTable[_screenLayer2[srcY * 320 + srcX]].area;
		int dstArea = _roomObjectAreasTable[_screenLayer2[dstY * 320 + dstX]].area;
		debugC(9, kDebugWalk, "srcArea = %d dstArea = %d", srcArea, dstArea);
		int currentArea = srcArea;
		for (int i = 1; dstArea != currentArea; ++i) {
			const int boxOffset = srcArea * _roomDataOffsets.area.boxSrcSize + dstArea * _roomDataOffsets.area.boxDstSize;
			int nextArea = _roomActionsTable[boxOffset + i + _roomDataOffsets.area.box];
			debugC(9, kDebugWalk, "nextArea %d (%d,%d,%d)", nextArea, _roomDataOffsets.area.box, _roomDataOffsets.area.boxSrcSize, _roomDataOffsets.area.boxDstSize);
			int nextPosX, nextPosY;
			if (dstArea != nextArea) {
				getClosestAreaTrianglePoint(nextArea, currentArea, &nextPosY, &nextPosX, srcY, srcX);
			} else {
				getClosestAreaTrianglePoint2(nextArea, currentArea, &nextPosY, &nextPosX, dstY, dstX, srcY, srcX);
			}
			debugC(9, kDebugWalk, "buildWalkPath() transitionArea = %d next %d,%d pos %d,%d offset 0x%X", nextArea, nextPosX, nextPosY, dstX, dstY, _roomDataOffsets.area.box);
			buildWalkPathArea(srcX, srcY, nextPosX, nextPosY);
			srcX = nextPosX;
			srcY = nextPosY;
			currentArea = nextArea;
		}
		buildWalkPathArea(srcX, srcY, dstX, dstY);
		--_walkDataLastIndex;
	}
	debugC(9, kDebugWalk, "buildWalkPath() end _walkDataLastIndex %d", _walkDataLastIndex);
}

void IgorEngine::buildWalkPathArea(int srcX, int srcY, int dstX, int dstY) {
	if (srcX != dstX || srcY != dstY) {
		const int dx = dstX - srcX;
		const int dy = dstY - srcY;
		debugC(9, kDebugWalk, "buildWalkPathArea() dx = %d dy = %d src %d,%d dst %d,%d", dx, dy, srcX, srcY, dstX, dstY);
		assert(_walkDataLastIndex > 0);
		if (ABS(dy) * 2 > ABS(dx)) {
			if (srcY > dstY) {
				buildWalkPathAreaUpDirection(srcX, srcY, dstX, dstY);
			} else {
				buildWalkPathAreaDownDirection(srcX, srcY, dstX, dstY);
			}
		} else {
			if (srcX < dstX) {
				buildWalkPathAreaRightDirection(srcX, srcY, dstX, dstY);
			} else {
				buildWalkPathAreaLeftDirection(srcX, srcY, dstX, dstY);
			}
		}
	}
}

static int16 roundReal(float f) {
	return (int16)(f + .5);
}

static int16 truncReal(float f) {
	return (int16)f;
}

int IgorEngine::getVerticalStepsCount(int minX, int minY, int maxX, int maxY) {
	debugC(9, kDebugWalk, "getVerticalStepsCount() %d %d %d %d", minX, minY, maxX, maxY);
	int curX = 2;
	if ((_walkXScaleRoom[minX] != 1 || _walkXScaleRoom[maxX] != 3) && (_walkXScaleRoom[maxX] != 1 || _walkXScaleRoom[minX] != 3)) {
		curX = _walkXScaleRoom[minX];
	}
	int curY = minY;
	int count = 0;
	while (1) {
		uint8 scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
		scale = _walkScaleTable[0x901 + scale];
		if (maxY - curY <= scale) {
			break;
		}
		curY += scale;
		++count;
	}
	return count;
}

int IgorEngine::getHorizontalStepsCount(int minX, int minY, int maxX, int maxY) {
	uint8 scale, frame = _walkCurrentFrame;

	scale = _walkYScaleRoom[(_walkXScaleRoom[maxX] - 1) * 144 + maxY];
	float r1 = _walkScaleSpeedTable[scale - 1];
	scale = _walkYScaleRoom[(_walkXScaleRoom[minX] - 1) * 144 + minY];
	float r2 = _walkScaleSpeedTable[scale - 1];
	debugC(9, kDebugWalk, "getHorizontalStepsCount() maxX - minX = %d r1 = %f r2 = %f", maxX - minX, r1, r2);

	int16 steps = roundReal((maxX - minX) / ((r1 + r2) / 2.0f));
	int count = 0;
	if (steps != 0) {
		float r3 = (maxY - minY) / (float)steps;
		int curX = minX;
		int curY = minY;
		float r4 = r3;
		while (1) {
			scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
			uint8 c = _walkScaleTable[0x769 + scale * 8 + frame];
			if (maxX - curX <= c) {
				break;
			}
			curX += c;
			curY = minY + truncReal(r4);
			if (frame == 8) {
				frame = 1;
			} else {
				++frame;
			}
			r4 += r3;
			++count;
		}
	}
	return count;
}

void IgorEngine::lookupScale(int curX, int curY, uint8 &scale, uint8 &xScale, uint8 &yScale) const {
	scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
	yScale = _walkScaleTable[0x901 + scale];
	xScale = _walkWidthScaleTable[scale - 1];
}

void IgorEngine::buildWalkPathAreaUpDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == 3) {
		_walkCurrentPos = 1;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (dstX > srcX) ? kFacingPositionRight : kFacingPositionLeft;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = 1;
	}
	if (_walkCurrentFrame > 6) {
		_walkCurrentFrame -= 6;
	}
	WalkData *wd;
	uint8 scale, xScale, yScale;
	float vStepDist, vStepCur;
	int curX = srcX;
	int curY = srcY;
	if (srcX > dstX) {
		int vStepsCount = getVerticalStepsCount(dstX, dstY, srcX, srcY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (srcX - dstX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX - truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY -= yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 1, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 1 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 1;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 1, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale / 2);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
	} else {
		int vStepsCount = getVerticalStepsCount(dstX, dstY, srcX, srcY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (dstX - srcX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX + truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY -= yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 1, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 1 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 1;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 1, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			int16 _dx = x - xScale;
			int16 _cx = 319 - _dx;
			wd->clipWidth = _cx;
			wd->xPosChanged = 0;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
	}
}

void IgorEngine::buildWalkPathAreaDownDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == 1) {
		_walkCurrentPos = 3;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcX > dstX) ? kFacingPositionLeft : kFacingPositionRight;
		_walkData[_walkDataLastIndex].frameNum = 1;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = 3;
	}
	if (_walkCurrentFrame > 6) {
		_walkCurrentFrame -= 6;
	}
	WalkData *wd;
	uint8 scale, xScale, yScale;
	float vStepCur, vStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcX > dstX) {
		int vStepsCount = getVerticalStepsCount(srcX, srcY, dstX, dstY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (srcX - dstX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX - truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY += yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 3, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 3 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 3;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 3, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
	} else {
		int vStepsCount = getVerticalStepsCount(srcX, srcY, dstX, dstY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (dstX - srcX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX + truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY += yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 3, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 3 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 3;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 3, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
	}
}

void IgorEngine::buildWalkPathAreaRightDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == kFacingPositionLeft) {
		_walkCurrentPos = kFacingPositionRight;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcY > dstY) ? kFacingPositionBack : kFacingPositionFront;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = kFacingPositionRight;
	}
	WalkData *wd;
	uint8 scale, xScale, xSkip;
	float hStepCur, hStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcY > dstY) {
		int hStepsCount = getHorizontalStepsCount(srcX, dstY, dstX, srcY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (srcY - dstY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if ((curX + xSkip) >= dstX) {
					break;
				}
				curX += xSkip;
				curY = srcY - truncReal(hStepCur);
				wd->setPos(curX, curY, 2, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = xSkip;
				} else {
					x = x - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 2 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 2;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 2, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	} else {
		int hStepsCount = getHorizontalStepsCount(srcX, srcY, dstX, dstY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (dstY - srcY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if (curX + xSkip >= dstX) {
					break;
				}
				curX += xSkip;
				curY = srcY + truncReal(hStepCur);
				_walkData[_walkDataLastIndex].setPos(curX, curY, 2, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = xSkip;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 2 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 2;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 2, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
	}
}

void IgorEngine::buildWalkPathAreaLeftDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == kFacingPositionRight) {
		_walkCurrentPos = kFacingPositionLeft;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcY > dstY) ? kFacingPositionBack : kFacingPositionFront;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = kFacingPositionLeft;
	}
	WalkData *wd;
	uint8 scale, xScale, xSkip;
	float hStepCur, hStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcY > dstY) {
		int hStepsCount = getHorizontalStepsCount(dstX, dstY, srcX, srcY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (srcY - dstY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if ((curX - xSkip) <= dstX) {
					break;
				}
				curX -= xSkip;
				curY = srcY - truncReal(hStepCur);
				wd->setPos(curX, curY, 4, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 4 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 4;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 4, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = curY - dstY;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	} else {
		int hStepsCount = getHorizontalStepsCount(dstX, srcY, srcX, dstY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (dstY - srcY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if (curX - xSkip <= dstX) {
					break;
				}
				curX -= xSkip;
				curY = srcY + truncReal(hStepCur);
				wd->setPos(curX, curY, 4, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 4 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 4;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 4, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = curY - dstY;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	}
}

void IgorEngine::waitForIgorMove() {
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (_updateRoomBackground) {
			(this->*_updateRoomBackground)();
		}
		waitForTimer();
	} while (_gameState.igorMoving);
}

void IgorEngine::setRoomWalkBounds(int x1, int y1, int x2, int y2) {
	assert(x1 <= x2 && y1 <= y2);
	_roomWalkBounds.x1 = x1;
	_roomWalkBounds.x2 = x2;
	_roomWalkBounds.y1 = y1;
	_roomWalkBounds.y2 = y2;
}

void IgorEngine::fixWalkPosition(int *x, int *y) {
	int xPos = *x;
	if (xPos < _roomWalkBounds.x1) {
		xPos = _roomWalkBounds.x1;
	}
	if (xPos > _roomWalkBounds.x2) {
		xPos = _roomWalkBounds.x2;
	}
	if (getPart() == 22) {
		*x = xPos;
		*y = _roomWalkBounds.y1;
		return;
	}
	int yPos = *y;
	if (getPart() == 13) {
		if (xPos >= 92 && xPos <= 186 && yPos > 127) {
			*x = xPos;
			*y = 127;
			return;
		}
		if (xPos >= 191 && xPos <= 289 && yPos > 127) {
			*x = xPos;
			*y = 127;
			return;
		}
	}
	// skip areas from top to bottom
	while (_roomObjectAreasTable[_screenLayer2[yPos * 320 + xPos]].area == 0 && yPos < _roomWalkBounds.y2) {
		++yPos;
	}
	if (getPart() == 17) {
		if (yPos != 143 || _roomObjectAreasTable[_screenLayer2[45760 + xPos]].area != 0) {
			*x = xPos;
			*y = yPos;
			return;
		}
	}
	// skip areas from bottom to top
	while (_roomObjectAreasTable[_screenLayer2[yPos * 320 + xPos]].area == 0 && yPos > _roomWalkBounds.y1) {
		--yPos;
	}
	*x = xPos;
	*y = yPos;
}

void IgorEngine::recolorDialogueChoice(int num, bool highlight) {
	uint8 *p = _screenVGA + 320 * (11 * num + 135);
	for (int i = 0; i < 320 * 11; ++i) {
		if (highlight) {
			if (p[i] == 240) {
				p[i] = 241;
			}
		} else {
			if (p[i] == 241) {
				p[i] = 240;
			}
		}
	}
}

void IgorEngine::handleDialogue(int x, int y, int r, int g, int b) {
	_gameState.dialogueStarted = true;
	_gameState.dialogueChoiceStart = 1;
	_gameState.dialogueChoiceCount = 1;
	_dialogueEnded = false;
	if (getPart() == 12 && _objectsState[44] == 0) {
		_gameState.dialogueData[6] = 1;
		dialogueReplyToQuestion(x, y, r, g, b, 40);
	}
	do {
		if (getPart() == 15 && _objectsState[48] == 0) {
			_gameState.dialogueData[6] = 0;
		}
		drawDialogueChoices();
		(this->*_updateDialogue)(kUpdateDialogueAnimStanding);
		_dialogueChoiceSelected = selectDialogue();
		if (_dialogueChoiceSelected == 0) {
			break;
		} else if (_dialogueChoiceSelected == -1) {
			return;
		}
		dialogueAskQuestion();
		dialogueReplyToQuestion(x, y, r, g, b);
		int offset = (_dialogueInfo[_dialogueChoiceSelected] - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 + (_gameState.dialogueChoiceStart - 1) * _roomDataOffsets.dlg.matSize;
		int code = _gameState.dialogueData[offset + 5];
		if ((code >= 1 && code <= 99) || (getPart() == 15 && code == 1)) {
			_gameState.dialogueData[offset] = 0;
			if (getPart() == 21 && (code == 60 || code == 70 || code == 80) && _dialogueInfo[0] == 1) {
				_gameState.dialogueData[offset + 2] = 4;
			}
			if (getPart() == 33 && (code == 21 || code == 22 || code == 23) && _dialogueInfo[0] == 1) {
				_gameState.dialogueData[offset + 2] = 2;
			}
		}
		debugC(9, kDebugEngine, "handleDialogue() action %d offset %d", _gameState.dialogueData[offset + 2], offset);
		switch (_gameState.dialogueData[offset + 2]) {
		case 1:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			++_gameState.dialogueChoiceStart;
			break;
		case 2:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			--_gameState.dialogueChoiceStart;
			break;
		case 4:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			_gameState.dialogueChoiceStart -= 2;
			break;
		case 0:
			_dialogueEnded = true;
			break;
		}
		debugC(9, kDebugEngine, "handleDialogue() end %d start %d count %d", _dialogueEnded, _gameState.dialogueChoiceStart, _gameState.dialogueChoiceCount);
	} while (!_dialogueEnded);
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_gameState.dialogueStarted = false;
}

void IgorEngine::drawDialogueChoices() {
	memset(_screenVGA + 46080, 0, 56 * 320);
	setPaletteColor(240, 0, 0, 0);
	_dialogueInfo[0] = 0;
	for (int i = 1; i <= 5; ++i) {
		_dialogueInfo[i] = 0;
	}
	for (int i = 1; i <= 5; ++i) {
		int offset = (i - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 + (_gameState.dialogueChoiceStart - 1) * _roomDataOffsets.dlg.matSize;
		if (_gameState.dialogueData[offset] == 1) {
			++_dialogueInfo[0];
			_dialogueInfo[_dialogueInfo[0]] = i;
			int num = _gameState.dialogueData[offset + 3] - 1;
			char questionText[128];
			sprintf(questionText, "@%s %s", _dialogueQuestions[num][0], _dialogueQuestions[num][1]);
			drawString(_screenVGA, questionText, 0, _dialogueInfo[0] * 11 + 135, 240, 0, 0);
		}
		 debugC(9, kDebugEngine, "drawDialogueChoices() i %d state %d num %d", i, _gameState.dialogueData[offset], _gameState.dialogueData[offset + 3]);
	}
	setPaletteColor(240, _paletteBuffer[0x2F6 + 1], _paletteBuffer[0x2F6 + 2], _paletteBuffer[0x2F6 + 3]);
	setPaletteColor(241, _paletteBuffer[0x2F0 + 1], _paletteBuffer[0x2F0 + 2], _paletteBuffer[0x2F0 + 3]);
}

int IgorEngine::selectDialogue() {
	showCursor();
	int hoveredChoice = 0;
	bool end = false;
	do {
		int currentChoice = (_inputVars[kInputCursorYPos] - 134) / 11;
		if (currentChoice < 0) {
			currentChoice = 0;
		}

		if (currentChoice != hoveredChoice) {
			if (hoveredChoice != 0) {
				recolorDialogueChoice(hoveredChoice, false);
			}
			hoveredChoice = currentChoice;
			if (hoveredChoice != 0) {
				recolorDialogueChoice(hoveredChoice, true);
			}
		}
		if (_inputVars[kInputClick]) {
			if (hoveredChoice != 0 && hoveredChoice <= _dialogueInfo[0]) {
				end = true;
			}
			_inputVars[kInputClick] = 0;
		}

		waitForTimer();
	} while (!end && !_eventQuitGame);
	hideCursor();
	return hoveredChoice;
}

void IgorEngine::dialogueAskQuestion() {
	memset(_screenVGA + 46080, 0, 17920);
	int offset = (_dialogueInfo[_dialogueChoiceSelected] - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 + (_gameState.dialogueChoiceStart - 1) * _roomDataOffsets.dlg.matSize;
	int num = _gameState.dialogueData[offset + 3] - 1;
	if (getPart() == 17) {
		num = 5;
	}
	debugC(9, kDebugEngine, "dialogueAskQuestion() num %d offset %d", num, offset);
	strcpy(_globalDialogueTexts[250], _dialogueQuestions[num][0]);
	strcpy(_globalDialogueTexts[251], _dialogueQuestions[num][1]);
	if (_globalDialogueTexts[251][0]) {
		ADD_DIALOGUE_TEXT(250, 2);
	} else {
		ADD_DIALOGUE_TEXT(250, 1);
	}
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::dialogueReplyToQuestion(int x, int y, int r, int g, int b, int reply) {
	if (reply == 0) {
		int offset = (_dialogueInfo[_dialogueChoiceSelected] - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 + (_gameState.dialogueChoiceStart - 1) * _roomDataOffsets.dlg.matSize;
		reply = _gameState.dialogueData[offset + 4];
		debugC(9, kDebugEngine, "dialogueReplyToQuestion() dialogue choice %d reply %d", _dialogueChoiceSelected, reply);
		if (reply == 0) {
			return;
		}
	}
	int offset = 30 + _roomDataOffsets.dlg.matSize + reply;
	int count = _gameState.dialogueData[offset - 1];
	int dialogueIndex = 250;
	for (int i = 0; i < count; ++i) {
		int num = _gameState.dialogueData[offset] - 1;
		int len = _gameState.dialogueData[offset + 1];
		debugC(9, kDebugEngine, "dialogueReplyToQuestion() reply %d %d offset %d", num, len, offset);
		ADD_DIALOGUE_TEXT(dialogueIndex, len);
		for (int j = 0; j < len; ++j) {
			strcpy(_globalDialogueTexts[dialogueIndex], _dialogueReplies[num + j]);
			++dialogueIndex;
		}
		offset += 2;
	}
	SET_DIALOGUE_TEXT(1, count);
	startCutsceneDialogue(x, y, r, g, b);
	waitForEndOfCutsceneDialogue(x, y, r, g, b);
}

} // namespace Igor
