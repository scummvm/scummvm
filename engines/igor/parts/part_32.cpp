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

// The colors of the buttons of the control panel in the mask of its picture.
enum {
	kPart32KeyBackspace = 11,
	kPart32KeyDeclination = 12,
	kPart32KeyAscension = 13,
	kPart32KeyConfirm = 14,
	kPart32KeyExit = 15
};

// Results of the control panel.
enum {
	kPart32ResultNone = 0,
	kPart32ResultTower = 1,
	kPart32ResultNothing = 2,
	kPart32ResultFinal = 3
};

// The glyphs of the digits are 6x7 pixels, drawn 7 pixels apart.
static const int kPart32GlyphWidth = 6;
static const int kPart32GlyphHeight = 7;
static const int kPart32GlyphSize = kPart32GlyphWidth * kPart32GlyphHeight;
static const int kPart32DigitPitch = 7;
static const int kPart32DeclinationDigitsOffset = 48 * 320 + 115;
static const int kPart32AscensionDigitsOffset = 68 * 320 + 162;

// The palette colors that mark the two fields of the control panel (the blinking one is the selected field) and
// the color they have when not blinking.
static const int kPart32DeclinationColor = 12;
static const int kPart32AscensionColor = 13;
static const int kPart32FieldColor = 14;

// The declination is a horizontal line that rises from the bottom of its gauge, the ascension a vertical line that
// moves to the right. Both cross at the aimed point.
static const int kPart32DeclinationBottom = 0x75;
static const int kPart32DeclinationLineX = 0x23;
static const int kPart32DeclinationLineWidth = 0x70;
static const int kPart32AscensionLeft = 0x22;
static const int kPart32AscensionTop = 0x55;
static const int kPart32AscensionWidth = 0x70;
static const int kPart32AscensionMax = 360;
static const int kPart32LineColor = 0xE;

// The coordinates that open each scene.
static const int kPart32TowerDeclination = 74;
static const int kPart32TowerAscension = 207;
static const int kPart32FinalDeclination = 60;
static const int kPart32FinalAscension = 90;

// The room data has the table of the frames of the tower scene at its end; the frame offsets are relative to the
// start of the picture buffer.
static const int kPart32TowerFramesTable = 0x851E;

// Where Igor goes before using the control panel.
static const int kPart32ControlsX = 117;
static const int kPart32ControlsY = 122;

// The frames of the final scene, in order (1-based index in the table of the data).
static const uint8 kPart32FinalFrames[13] = { 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5 };

// The speakers of the tower scene.
static const int kPart32SpeakerAX = 147;
static const int kPart32SpeakerAY = 70;
static const int kPart32SpeakerBX = 166;
static const int kPart32SpeakerBY = 67;

void IgorEngine::PART_32_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 311;
		break;
	case 102:
		igorSay(201, 1, 828);
		break;
	case 103:
		PART_32_ACTION_103_useControls();
		break;
	case 104:
		igorSay(202, 2, 829);
		break;
	case 105:
		igorSay(204, 2, 830);
		break;
	case 106:
		PART_32_ACTION_106_walkToControls();
		break;
	default:
		error("PART_32_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Igor comes in from the left edge, walking to the right.
 */
void IgorEngine::PART_32_ENTER() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 143, kFacingPositionLeft, 0);
	wd->clipSkipX = 11;
	wd->clipWidth = 10;
	wd->scaleWidth = 33;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 33;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(0, 143, 40, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * Igor stops where he is, walks to the place in front of the control panel (unless he is already there) and
 * uses it.
 */
void IgorEngine::PART_32_ACTION_106_walkToControls() {
	if (_gameState.igorMoving) {
		_walkDataLastIndex = _walkDataCurrentIndex + 1;
		_walkData[_walkDataCurrentIndex].frameNum = 0;
		moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
		_gameState.igorMoving = false;
	}
	const WalkData &last = _walkData[_walkDataLastIndex - 1];
	if (last.x != kPart32ControlsX || last.y != kPart32ControlsY) {
		--_walkDataLastIndex;
		buildWalkPathSimple(_walkData[_walkDataLastIndex].x, _walkData[_walkDataLastIndex].y, kPart32ControlsX, kPart32ControlsY);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}
	PART_32_ACTION_103_useControls();
}

/**
 * The room is faded out and its screen and palette are kept; the control panel takes the screen and, depending
 * on the coordinates that were set, a scene follows. Then the room is loaded again and faded in.
 */
void IgorEngine::PART_32_ACTION_103_useControls() {
	fadeOut(624);
	memcpy(_animFramesBuffer, _screenVGA, 64000);
	memcpy(_animFramesBuffer + 64000, _paletteBuffer, 768);
	PART_32_CONTROLS();
	if (_part32Result >= kPart32ResultTower && _part32Result <= kPart32ResultFinal) {
		PART_32_SCENE();
	}
	loadRoomData(PAL_Part32, IMG_Part32, BOX_Part32, MSK_Part32, TXT_Part32);
	memcpy(_screenVGA, _animFramesBuffer, 64000);
	memcpy(_paletteBuffer, _animFramesBuffer + 64000, 768);
	fadeIn(768);
	if (_part32Result == kPart32ResultTower) {
		if (_objectsState[78] == 0 && _objectsState[4] == 2) {
			igorSay(206, 3, 831);
			_objectsState[78] = 1;
		}
	}
	if (_part32Result == kPart32ResultFinal) {
		if (_objectsState[79] == 0) {
			igorSay({ { 209, 1, 832 }, { 210, 1, 833 } });
			_objectsState[79] = 1;
		} else {
			igorSay(211, 1, 834);
		}
	}
	if (!_eventQuitGame) {
		_currentPart = 320;
	}
}

// The control panel

void IgorEngine::PART_32_CONTROLS_COPY_COLOR(int dstColor, int srcColor) {
	memcpy(_currentPalette + dstColor * 3, _currentPalette + srcColor * 3, 3);
}

/**
 * The button under the cursor; the part of the screen below the picture has none.
 */
int IgorEngine::PART_32_CONTROLS_KEY_AT_CURSOR() const {
	const int x = _inputVars[kInputCursorXPos];
	const int y = _inputVars[kInputCursorYPos];
	if (x < 0 || x >= 320 || y < 0 || y >= 144) {
		return 0;
	}
	return _screenLayer2[y * 320 + x];
}

void IgorEngine::PART_32_CONTROLS_DRAW_DIGIT(int fieldOffset, int index, int key) {
	copyArea(_screenVGA, fieldOffset + index * kPart32DigitPitch, 320, _part32Glyphs + (key - 1) * kPart32GlyphSize,
			 kPart32GlyphWidth, kPart32GlyphWidth, kPart32GlyphHeight);
}

void IgorEngine::PART_32_CONTROLS_ERASE_DIGIT(int fieldOffset, int index) {
	for (int row = 0; row < kPart32GlyphHeight; ++row) {
		memset(_screenVGA + fieldOffset + index * kPart32DigitPitch + row * 320, 0, kPart32GlyphWidth);
	}
}

/**
 * What every pass of a selected field does after its click: the escape key leaves the panel, the color that
 * marks the field blinks once in a while and the game waits for the next tick.
 */
void IgorEngine::PART_32_CONTROLS_IDLE(int fieldColor) {
	if (_inputVars[kInputEscape]) {
		PART_32_CONTROLS_EXIT();
	}
	if (compareGameTick(0)) {
		if (_part32Blink == 1) {
			PART_32_CONTROLS_COPY_COLOR(fieldColor, kPart32FieldColor);
		} else if (_part32Blink == 2) {
			PART_32_CONTROLS_COPY_COLOR(fieldColor, 0);
		}
		setPaletteRange(fieldColor, fieldColor);
		_part32Blink = _part32Blink == 1 ? 2 : 1;
	}
	waitForTimer();
}

void IgorEngine::PART_32_CONTROLS_EXIT() {
	playSound(getRandomNumber(1) + 0x42, 1);
	PART_32_CONTROLS_COPY_COLOR(kPart32DeclinationColor, kPart32FieldColor);
	PART_32_CONTROLS_COPY_COLOR(kPart32AscensionColor, kPart32FieldColor);
	setPaletteRange(kPart32DeclinationColor, kPart32AscensionColor);
	_part32Done = true;
}

/**
 * The declination field is selected: the digits typed go to it until the other field or a button is chosen.
 */
void IgorEngine::PART_32_CONTROLS_SELECT_DECLINATION() {
	playSound(31, 1);
	PART_32_CONTROLS_COPY_COLOR(kPart32DeclinationColor, kPart32FieldColor);
	PART_32_CONTROLS_COPY_COLOR(kPart32AscensionColor, kPart32FieldColor);
	setPaletteRange(kPart32AscensionColor, kPart32AscensionColor);
	_part32Blink = 2;
	do {
		if (_inputVars[kInputClick]) {
			_inputVars[kInputClick] = 0;
			const int key = PART_32_CONTROLS_KEY_AT_CURSOR();
			if (key != kPart32KeyAscension) {
				playSound(getRandomNumber(1) + 0x42, 1);
			}
			if (key >= 1 && key <= 10) {
				// the declination does not go over 90
				if (_part32DeclinationIndex == 1 && _part32DeclinationDigits[0] == 9 && key < 10) {
					continue;
				}
				PART_32_CONTROLS_DRAW_DIGIT(kPart32DeclinationDigitsOffset, _part32DeclinationIndex, key);
				_part32DeclinationDigits[_part32DeclinationIndex] = key % 10;
				if (_part32DeclinationIndex < 1) {
					++_part32DeclinationIndex;
				}
			} else if (key == kPart32KeyBackspace) {
				if (_part32DeclinationDigits[_part32DeclinationIndex] == 0xFF && _part32DeclinationIndex > 0) {
					--_part32DeclinationIndex;
				}
				PART_32_CONTROLS_ERASE_DIGIT(kPart32DeclinationDigitsOffset, _part32DeclinationIndex);
				_part32DeclinationDigits[_part32DeclinationIndex] = 0xFF;
			} else if (key == kPart32KeyAscension) {
				PART_32_CONTROLS_SELECT_ASCENSION();
			} else if (key == kPart32KeyConfirm) {
				PART_32_CONTROLS_CONFIRM();
			} else if (key == kPart32KeyExit) {
				PART_32_CONTROLS_EXIT();
			}
		}
		PART_32_CONTROLS_IDLE(kPart32DeclinationColor);
	} while (!_part32Done);
}

/**
 * The ascension field is selected.
 */
void IgorEngine::PART_32_CONTROLS_SELECT_ASCENSION() {
	playSound(31, 1);
	PART_32_CONTROLS_COPY_COLOR(kPart32DeclinationColor, kPart32FieldColor);
	setPaletteRange(kPart32DeclinationColor, kPart32DeclinationColor);
	_part32Blink = 2;
	do {
		if (_inputVars[kInputClick]) {
			_inputVars[kInputClick] = 0;
			const int key = PART_32_CONTROLS_KEY_AT_CURSOR();
			if (key != kPart32KeyDeclination) {
				playSound(getRandomNumber(1) + 0x42, 1);
			}
			if (key >= 1 && key <= 10) {
				// the ascension does not go over 360
				if (_part32AscensionIndex == 0 && key >= 4 && key <= 9) {
					continue;
				}
				if (_part32AscensionIndex == 1 && _part32AscensionDigits[0] == 3 && key >= 7 && key <= 9) {
					continue;
				}
				if (_part32AscensionIndex == 2 && _part32AscensionDigits[0] == 3 && _part32AscensionDigits[1] == 6 && key < 10) {
					continue;
				}
				PART_32_CONTROLS_DRAW_DIGIT(kPart32AscensionDigitsOffset, _part32AscensionIndex, key);
				_part32AscensionDigits[_part32AscensionIndex] = key % 10;
				if (_part32AscensionIndex < 2) {
					++_part32AscensionIndex;
				}
			} else if (key == kPart32KeyBackspace) {
				if (_part32AscensionDigits[_part32AscensionIndex] == 0xFF && _part32AscensionIndex > 0) {
					--_part32AscensionIndex;
				}
				PART_32_CONTROLS_ERASE_DIGIT(kPart32AscensionDigitsOffset, _part32AscensionIndex);
				_part32AscensionDigits[_part32AscensionIndex] = 0xFF;
			} else if (key == kPart32KeyDeclination) {
				PART_32_CONTROLS_SELECT_DECLINATION();
			} else if (key == kPart32KeyConfirm) {
				PART_32_CONTROLS_CONFIRM();
			} else if (key == kPart32KeyExit) {
				PART_32_CONTROLS_EXIT();
			}
		}
		PART_32_CONTROLS_IDLE(kPart32AscensionColor);
	} while (!_part32Done);
}

/**
 * The coordinates are set: the two lines move to them and the scene that belongs to them is chosen.
 */
void IgorEngine::PART_32_CONTROLS_CONFIRM() {
	playSound(getRandomNumber(1) + 0x42, 1);
	PART_32_CONTROLS_COPY_COLOR(kPart32DeclinationColor, kPart32FieldColor);
	PART_32_CONTROLS_COPY_COLOR(kPart32AscensionColor, kPart32FieldColor);
	setPaletteRange(kPart32DeclinationColor, kPart32AscensionColor);
	const int declination = PART_32_CONTROLS_ANIMATE_DECLINATION();
	const int ascension = PART_32_CONTROLS_ANIMATE_ASCENSION();
	waitForTimer(100);
	if (declination == kPart32TowerDeclination && ascension == kPart32TowerAscension) {
		_part32Result = kPart32ResultTower;
	} else if (declination == kPart32FinalDeclination && ascension == kPart32FinalAscension) {
		_part32Result = kPart32ResultFinal;
	} else {
		_part32Result = kPart32ResultNothing;
	}
	_part32Done = true;
}

/**
 * The declination typed (a single digit is the unit) and the horizontal line rising to it, 1 row per step.
 */
int IgorEngine::PART_32_CONTROLS_ANIMATE_DECLINATION() {
	playSound(30, 1);
	uint8 *digits = _part32DeclinationDigits;
	if (digits[1] == 0xFF) {
		digits[1] = digits[0];
		digits[0] = 0;
	}
	int declination = digits[0] == 0xFF ? 0 : digits[0] * 10;
	if (digits[1] != 0xFF) {
		declination += digits[1];
	}
	_part32LineRow = kPart32DeclinationBottom - (declination << 5) / 0x5A;
	int row = kPart32DeclinationBottom;
	do {
		if (compareGameTick(1, 8) && row > _part32LineRow) {
			memset(_screenVGA + row * 320 + kPart32DeclinationLineX, 0, kPart32DeclinationLineWidth);
			memset(_screenVGA + (row - 1) * 320 + kPart32DeclinationLineX, kPart32LineColor, kPart32DeclinationLineWidth);
			--row;
		}
		waitForTimer();
	} while (row != _part32LineRow);
	stopSound();
	return declination;
}

/**
 * The ascension typed (the last digit is the unit) and the vertical line moving to it, 1 column per step. It is
 * cut at the row of the horizontal line.
 */
int IgorEngine::PART_32_CONTROLS_ANIMATE_ASCENSION() {
	playSound(30, 1);
	uint8 *digits = _part32AscensionDigits;
	if (digits[2] == 0xFF) {
		digits[2] = digits[1];
		digits[1] = digits[0];
		digits[0] = 0;
	}
	if (digits[2] == 0xFF) {
		digits[2] = digits[1];
		digits[1] = 0;
	}
	int ascension = digits[0] == 0xFF ? 0 : digits[0] * 100;
	if (digits[1] != 0xFF) {
		ascension += digits[1] * 10;
	}
	if (digits[2] != 0xFF) {
		ascension += digits[2];
	}
	const int target = ascension * kPart32AscensionWidth / kPart32AscensionMax + kPart32AscensionLeft;
	int column = kPart32AscensionLeft;
	do {
		if (compareGameTick(1, 8) && column < target) {
			for (int y = kPart32AscensionTop; y <= kPart32DeclinationBottom; ++y) {
				if (y == _part32LineRow) {
					continue;
				}
				_screenVGA[y * 320 + column] = 0;
				_screenVGA[y * 320 + column + 1] = kPart32LineColor;
			}
			++column;
		}
		waitForTimer();
	} while (column != target);
	stopSound();
	return ascension;
}

/**
 * The picture with the digit boxes. Clicking a button of it does what the button says; the escape key and the exit
 * button leave.
 */
void IgorEngine::PART_32_CONTROLS() {
	loadData(PAL_Part32Lock, _paletteBuffer);
	loadData(IMG_Part32Lock, _screenLayer1);
	uint8 *mask = loadData(MSK_Part32Lock);
	decodeRoomMask(mask);
	free(mask);
	loadData(ANM_Part32Digits, _part32Glyphs);
	memcpy(_screenVGA, _screenLayer1, 46080);
	memset(_screenVGA + 46080, 0, 17920);
	fadeIn(624);
	showCursor();

	memset(_part32DeclinationDigits, 0xFF, sizeof(_part32DeclinationDigits));
	memset(_part32AscensionDigits, 0xFF, sizeof(_part32AscensionDigits));
	_part32DeclinationIndex = 0;
	_part32AscensionIndex = 0;
	_part32Done = false;
	_part32Result = kPart32ResultNone;
	do {
		if (_inputVars[kInputClick] && _inputVars[kInputCursorYPos] < 144) {
			_inputVars[kInputClick] = 0;
			switch (PART_32_CONTROLS_KEY_AT_CURSOR()) {
			case kPart32KeyDeclination:
				PART_32_CONTROLS_SELECT_DECLINATION();
				break;
			case kPart32KeyAscension:
				PART_32_CONTROLS_SELECT_ASCENSION();
				break;
			case kPart32KeyConfirm:
				PART_32_CONTROLS_CONFIRM();
				break;
			case kPart32KeyExit:
				PART_32_CONTROLS_EXIT();
				break;
			}
		}
		if (_inputVars[kInputEscape]) {
			PART_32_CONTROLS_EXIT();
		}
		PART_32_CONTROLS_COPY_COLOR(kPart32DeclinationColor, kPart32FieldColor);
		PART_32_CONTROLS_COPY_COLOR(kPart32AscensionColor, kPart32FieldColor);
		setPaletteRange(kPart32DeclinationColor, kPart32AscensionColor);
		// the original polls the input here without waiting
		waitForTimer();
	} while (!_part32Done);
	hideCursor();
	fadeOut(624);
}

// The scenes

/**
 * The tower scenes and the final one have their own pictures and palettes. The tower scenes also have their
 * own texts, which replace the ones of the room until it is loaded again.
 */
void IgorEngine::PART_32_LOAD_TOWER() {
	loadData(PAL_Part32Tower, _paletteBuffer);
	// the pictures start 1 byte into the buffer; the table of the offsets of the frames follows them
	loadData(ANM_Part32Tower, _screenLayer1 + 1);
	uint8 *texts = loadData(TXT_Part32Tower);
	decodeRoomStrings(texts, true);
	free(texts);
}

void IgorEngine::PART_32_LOAD_FINAL() {
	loadData(PAL_Part32Final, _paletteBuffer);
	loadData(IMG_Part32Final, _screenLayer1);
	// the offsets of the frames, and the frames after them
	loadData(ANM_Part32Final, _screenLayer2);
}

void IgorEngine::PART_32_DRAW_TOWER_FRAME(int frame) {
	const uint8 *src = _screenLayer1 + READ_LE_UINT16(_screenLayer1 + kPart32TowerFramesTable + frame * 2);
	decodeAnimFrame(src, _screenVGA, true);
}

/**
 * The man in the tower talks: his mouth is closed when a sentence ends and open in one of 5 ways while it goes on.
 */
void IgorEngine::PART_32_UPDATE_DIALOGUE_SPEAKER_A(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_32_DRAW_TOWER_FRAME(9);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_32_DRAW_TOWER_FRAME(getRandomNumber(4) + 9);
		break;
	}
}

/**
 * The other man in the tower talks.
 */
void IgorEngine::PART_32_UPDATE_DIALOGUE_SPEAKER_B(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_32_DRAW_TOWER_FRAME(4);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_32_DRAW_TOWER_FRAME(getRandomNumber(4) + 4);
		break;
	}
}

void IgorEngine::PART_32_SPEAKER_A_SAYS(const Common::Array<DialogueText> &lines) {
	cutsceneSayWithCallback(kPart32SpeakerAX, kPart32SpeakerAY, 63, 59, 0, lines,
							&IgorEngine::PART_32_UPDATE_DIALOGUE_SPEAKER_A);
}

void IgorEngine::PART_32_SPEAKER_B_SAYS(const Common::Array<DialogueText> &lines) {
	cutsceneSayWithCallback(kPart32SpeakerBX, kPart32SpeakerBY, 63, 21, 21, lines,
							&IgorEngine::PART_32_UPDATE_DIALOGUE_SPEAKER_B);
}

/**
 * The scene of the coordinates that were set.
 */
void IgorEngine::PART_32_SCENE() {
	const bool tower = _part32Result != kPart32ResultFinal;
	if (tower) {
		PART_32_LOAD_TOWER();
	} else {
		PART_32_LOAD_FINAL();
	}
	memset(_screenVGA, 0, 46080);
	switch (_part32Result) {
	case kPart32ResultTower:
		if (_objectsState[78] == 0 && _objectsState[4] == 2) {
			PART_32_SCENE_TOWER_CONVERSATION();
		} else {
			PART_32_SCENE_TOWER_EMPTY();
		}
		break;
	case kPart32ResultNothing:
		PART_32_SCENE_NOTHING();
		break;
	case kPart32ResultFinal:
		PART_32_SCENE_FINAL();
		break;
	}
	fadeOut(tower ? 624 : 768);
}

/**
 * The two men in the bell tower talk about the signal hidden in the bell. Seen once.
 */
void IgorEngine::PART_32_SCENE_TOWER_CONVERSATION() {
	PART_32_DRAW_TOWER_FRAME(2);
	PART_32_DRAW_TOWER_FRAME(3);
	fadeIn(624);
	PART_32_SPEAKER_A_SAYS({ { 201, 2, 835 } });
	PART_32_SPEAKER_B_SAYS({ { 203, 1, 836 }, { 204, 1, 837 }, { 205, 1, 838 }, { 206, 2, 839 } });
	PART_32_SPEAKER_A_SAYS({ { 208, 1, 840 } });
	PART_32_SPEAKER_B_SAYS({ { 209, 2, 841 }, { 211, 1, 842 } });
	PART_32_SPEAKER_A_SAYS({ { 212, 1, 843 } });
	PART_32_SPEAKER_B_SAYS({ { 213, 2, 844 }, { 215, 2, 845 } });
	PART_32_SPEAKER_A_SAYS({ { 217, 1, 846 } });
	PART_32_SPEAKER_B_SAYS({ { 218, 1, 847 }, { 219, 2, 848 }, { 221, 1, 849 }, { 222, 2, 850 } });
	PART_32_SPEAKER_A_SAYS({ { 224, 1, 851 } });
	waitForTimer(255);
}

/**
 * The tower again, with nobody there.
 */
void IgorEngine::PART_32_SCENE_TOWER_EMPTY() {
	PART_32_DRAW_TOWER_FRAME(2);
	fadeIn(624);
	waitForTimer(255);
	cutsceneSayStart(160, 70, 63, 63, 63, 227, 1, 853);
	waitForEndOfIgorDialogue(false);
	waitForTimer(255);
}

/**
 * Coordinates without anything special.
 */
void IgorEngine::PART_32_SCENE_NOTHING() {
	PART_32_DRAW_TOWER_FRAME(1);
	fadeIn(624);
	waitForTimer(255);
	cutsceneSayStart(160, 70, 63, 63, 63, 225, 2, 852);
	waitForEndOfIgorDialogue(false);
	waitForTimer(255);
}

/**
 * The animation of the last coordinates.
 */
void IgorEngine::PART_32_SCENE_FINAL() {
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	fadeIn(768);
	for (int i = 0; i < ARRAYSIZE(kPart32FinalFrames); ++i) {
		const int frame = kPart32FinalFrames[i];
		decodeAnimFrame(_screenLayer2 + READ_LE_UINT16(_screenLayer2 + frame * 2 - 2) + 9, _screenVGA, false);
		waitForTimer(50);
	}
	playSound(32, 1);
	waitForTimer(255);
}

void IgorEngine::PART_32() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadActionData(DAT_Part32);
	loadRoomData(PAL_Part32, IMG_Part32, BOX_Part32, MSK_Part32, TXT_Part32);
	SET_PAL_240_48_1();
	_roomDataOffsets = PART_32_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, 10, 120, true);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_32_EXEC_ACTION);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		PART_32_ENTER();
	}
	enterPartLoop();
	while (_currentPart == 320 && !_gameStateLoaded) {
		runPartLoop();
	}
	// the colors that changed are kept
	memcpy(_paletteBuffer, _currentPalette, 624);
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
