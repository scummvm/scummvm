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

namespace Igor {

const uint8 _talkDelays[] = { 0, 27, 22, 17, 12, 7, 0 };

const uint8 IgorEngine::_dialogueColor[] = { 0x3F, 0x3F, 0x3F };

const uint8 IgorEngine::_sentenceColorIndex[]= { 0xFD, 0xFB, 0xF1 };

/**
 * appends one DialogueText{num, count, sound} record into _dialogueTextsTable[] at index _dialogueTextsCount, then increments it.
 * MAX_DIALOGUE_TEXTS = 6
 */
void IgorEngine::ADD_DIALOGUE_TEXT(int num, int count, int sound) {
	assert(_dialogueTextsCount < MAX_DIALOGUE_TEXTS);
	DialogueText *dt = &_dialogueTextsTable[_dialogueTextsCount];
	dt->num = num;
	dt->count = count;
	dt->sound = sound;
	++_dialogueTextsCount;
}

void IgorEngine::SET_DIALOGUE_TEXT(int start, int count) {
	_dialogueTextsStart = start - 1;
	_dialogueTextsCount = count;
}

void IgorEngine::fixDialogueTextPosition(int num, int count, int *x, int *y) {
    int textLineWidth = 0;
	for (int i = 0; i < count; ++i) {
		int w = _font.getStringWidth(_globalDialogueTexts[num + i]);
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

	/**
	 * Clear previous text line
	 */
	memcpy(_screenTextLayer, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	memcpy(_screenTextLayer + 320 * 72, _screenVGA + _dialogueDirtyRectY, _dialogueDirtyRectSize);

	for (int i = 0; i < dt->count; ++i) {
		const char *textLine = _globalDialogueTexts[dt->num + i];

		int textLineWidth = _font.getStringWidth(textLine);;
		int textX = talkX - textLineWidth / 2;
		int textY = i * 10;
		_font.drawString(_screenTextLayer, textLine, textX, textY, kTalkColor, kTalkShadowColor, kTalkShadowColor);
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
	if (_gameState.talkMode != kTalkModeTextOnly) {
		playSound(dt->sound, 0);
	}
	_gameState.dialogueTextRunning = true;
	_inputVars[kInputSkipDialogue] = 0;
}

void IgorEngine::waitForEndOfCutsceneDialogue(int x, int y, int r, int g, int b) {
	do {
		if (_gameState.dialogueTextRunning && _inputVars[kInputSkipDialogue]) {
			if (_mixer->isSoundHandleActive(_speechHandle)) {
				_mixer->stopHandle(_speechHandle);
			}
			_talkDelayCounter = _talkDelay;
			_inputVars[kInputSkipDialogue] = 0;
		}
		// Speech completion ends the sentence; active speech does not suspend
		// character animation. See cseg209:0x0802-0x0843.
		if (_gameState.dialogueTextRunning && _gameState.talkMode != kTalkModeTextOnly && !isDialogueSpeechPlaying()) {
			_talkDelayCounter = _talkDelay;
		}
		if (compareGameTick(19, 32) && _gameState.dialogueTextRunning) {
			if (_talkSpeechCounter > 2) {
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

bool IgorEngine::isDialogueSpeechPlaying() const {
	return _gameState.talkMode != kTalkModeTextOnly && _mixer->isSoundHandleActive(_speechHandle);
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
		int textLineWidth = _font.getStringWidth(textLine);
		int textX = talkX - textLineWidth / 2;
		int textY = i * 10;
		_font.drawString(_screenTextLayer, textLine, textX, textY, kTalkColor, kTalkShadowColor, kTalkShadowColor);
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
	if (_gameState.talkMode != kTalkModeTextOnly) {
		playSound(dt->sound, 0);
	}
	_gameState.dialogueTextRunning = true;
	_inputVars[kInputSkipDialogue] = 0;
}

void IgorEngine::waitForEndOfIgorDialogue() {
	do {
		if (_gameState.dialogueTextRunning && _inputVars[kInputSkipDialogue]) {
			if (_mixer->isSoundHandleActive(_speechHandle)) {
				_mixer->stopHandle(_speechHandle);
			}
			_talkDelayCounter = _talkDelay;
			_inputVars[kInputSkipDialogue] = 0;
		}
		// Speech completion ends the sentence; active speech does not suspend
		// Igor's head animation. See cseg209:0x0B3D-0x0B7E.
		if (_gameState.dialogueTextRunning && _gameState.talkMode != kTalkModeTextOnly && !isDialogueSpeechPlaying()) {
			_talkDelayCounter = _talkDelay;
		}
		if (compareGameTick(19, 32) && _gameState.dialogueTextRunning) {
			if (_talkSpeechCounter > 2) {
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
					animateIgorTalking(getRandomNumber(5));
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

void IgorEngine::animateIgorTalking(int frame) {
	debug("Animating Igor talking with frame: %d", frame);
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

} // End of namespace Igor
