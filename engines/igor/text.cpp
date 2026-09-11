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
	// if (_gameState.talkMode == kTalkModeTextOnly) {
	// 	playSound(24, 0);
	// }
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

} // End of namespace Igor
