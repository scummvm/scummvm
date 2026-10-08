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

static const int kPart67FlameX = 81;
static const int kPart67FlameY = 52;
static const int kPart67FlameWidth = 19;
static const int kPart67FlameHeight = 29;
static const int kPart67FlameFrameSize = kPart67FlameWidth * kPart67FlameHeight;

// The palette is darkened by this amount in the colors of the room and of Igor.
static const int kPart67PaletteDarkness = 8;

static int PART_67_FLAME_FRAME;

void IgorEngine::PART_67_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // go back to church
		PART_67_ACTION_101_goToChurch();
		break;
	case 102:
		PART_67_ACTION_102_goRight();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(211, 2, 1155);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(201, 2, 1149);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_67_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Igor walks back to where he came from and the church puzzle room is entered again.
 */
void IgorEngine::PART_67_ACTION_101_goToChurch() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[29594]].area = 1;
	buildWalkPathSimple(154, 106, 154, 92);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::PART_67_UPDATE_FLICKER);
	_currentPart = 142;
}

/**
 * Igor walks off the right edge of the picture to the next location of the park.
 */
void IgorEngine::PART_67_ACTION_102_goRight() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[41258]].area = 1;
	buildWalkPathSimple(248, 128, 298, 128);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::PART_67_UPDATE_FLICKER);
	_mazeLocation = 0x55;
	_currentPart = 643;
}

/**
 * Draws a frame of the flame, keeping the dialogue text on screen. The empty pixels of the
 * frame show the room and the pixels with color 1 show the same pixel of the first frame.
 */
void IgorEngine::PART_67_DRAW_FLAME(int frame) {
	const int screenOffset = kPart67FlameY * 320 + kPart67FlameX;
	const uint8 *frames = _animFramesBuffer + (frame + 1) * kPart67FlameFrameSize;
	uint8 tmp[kPart67FlameFrameSize];
	for (int y = 0; y < kPart67FlameHeight; ++y) {
		for (int x = 0; x < kPart67FlameWidth; ++x) {
			const uint8 pixel = _screenVGA[screenOffset + y * 320 + x];
			uint8 color;
			if (pixel == 0xF0 || pixel == 0xF1) {
				color = pixel;
			} else {
				color = frames[y * kPart67FlameWidth + x];
				if (color == 0) {
					color = _screenLayer1[screenOffset + y * 320 + x];
				}
				if (color == 1) {
					color = _animFramesBuffer[y * kPart67FlameWidth + x];
				}
			}
			tmp[y * kPart67FlameWidth + x] = color;
		}
	}
	for (int y = 0; y < kPart67FlameHeight; ++y) {
		memcpy(_screenVGA + screenOffset + y * 320, tmp + y * kPart67FlameWidth, kPart67FlameWidth);
	}
}

/**
 * Changes the flame to another random frame and makes the light of the room flicker: the colors
 * of the room and of Igor are set to the (darkened) palette changed by 0, -1 or -2.
 */
void IgorEngine::PART_67_FLICKER() {
	int frame;
	do {
		frame = getRandomNumber(9);
	} while (frame == PART_67_FLAME_FRAME);
	PART_67_FLAME_FRAME = frame;
	PART_67_DRAW_FLAME(frame);

	int delta = 0;
	switch (getRandomNumber(2)) {
	case 0:
		delta = -2;
		break;
	case 1:
		delta = -1;
		break;
	case 2:
		delta = 0;
		break;
	}
	for (int color = 1; color <= 207; ++color) {
		if (color > 183 && color < 192) {
			continue;
		}
		for (int i = 0; i < 3; ++i) {
			const int value = _paletteBuffer[color * 3 + i] + delta;
			_currentPalette[color * 3 + i] = value < 1 ? 0 : value;
		}
	}
	setPaletteRange(1, 183);
	setPaletteRange(192, 207);
}

/**
 * The flame and the light flicker.
 */
void IgorEngine::PART_67_UPDATE_FLICKER() {
	// the original phases 11, 35 and 59 are the ticks 8, 32 and 56
	if (compareGameTick(16, 24)) {
		PART_67_FLICKER();
	}
}

void IgorEngine::PART_67_ENTER_FROM_CHURCH_PUZZLE() {
	WalkData *wd = &_walkData[0];
	wd->setPos(154, 92, kFacingPositionFront, 0);
	wd->setDefaultScale();
	_walkDataLastIndex = 0;
	const int area = _screenLayer2[29594];
	_roomObjectAreasTable[area].area = 1;
	buildWalkPathSimple(154, 92, 154, 110);
	_roomObjectAreasTable[area].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::PART_67_UPDATE_FLICKER);
}

void IgorEngine::PART_67_ENTER_FROM_RIGHT() {
	WalkData *wd = &_walkData[0];
	wd->setPos(298, 128, kFacingPositionLeft, 0);
	wd->setDefaultScale();
	_walkDataLastIndex = 0;
	const int area = _screenLayer2[41258];
	_roomObjectAreasTable[area].area = 1;
	buildWalkPathSimple(298, 128, 240, 128);
	_roomObjectAreasTable[area].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::PART_67_UPDATE_FLICKER);
}

/**
 * The flame and the light flicker; screams are heard from time to time, only if no speech is
 * playing.
 */
void IgorEngine::PART_67_UPDATE_ROOM_BACKGROUND() {
	PART_67_UPDATE_FLICKER();
	if (compareGameTick(0x3D) && getRandomNumber(24) == 0 && !isDialogueSpeechPlaying()) {
		switch (getRandomNumber(4)) {
		case 0:
			playSound(55, 1);
			break;
		case 1:
			playSound(56, 1);
			break;
		case 2:
			playSound(57, 1);
			break;
		case 3:
			playSound(58, 1);
			break;
		case 4:
			playSound(59, 1);
			break;
		}
	}
}

void IgorEngine::PART_67() {
	playMusic(5);
	_gameState.enableLight = 1;
	loadRoomData(PAL_MazeEntrance, IMG_MazeEntrance, BOX_MazeEntrance, MSK_MazeEntrance, TXT_MazeEntrance);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	static const int anm[] = { FRM_MazeEntrance1, 0 };
	loadAnimData(anm);
	loadActionData(DAT_MazeEntrance);
	_roomDataOffsets = PART_67_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(112, 0, 248, 128);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_67_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_67_UPDATE_ROOM_BACKGROUND;
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the whole room is darker than its palette
	for (int color = 1; color <= 207; ++color) {
		if (color > 183 && color < 192) {
			continue;
		}
		for (int i = 0; i < 3; ++i) {
			const int value = _paletteBuffer[color * 3 + i] - kPart67PaletteDarkness;
			_paletteBuffer[color * 3 + i] = value < 1 ? 0 : value;
		}
	}

	PART_67_FLAME_FRAME = 0;
	// draws first frame of flame
	PART_67_DRAW_FLAME(0);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 670) {
			PART_67_ENTER_FROM_CHURCH_PUZZLE();
		} else {
			PART_67_ENTER_FROM_RIGHT();
		}
	}
	enterPartLoop();
	while ((_currentPart == 670 || _currentPart == 671) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
