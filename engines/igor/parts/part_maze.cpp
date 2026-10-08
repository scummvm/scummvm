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

// The rooms of the maze (parts 51 to 66) are all the same room with different pictures: a corridor with torches
// and one to four ways out. Room data is in part_maze_data.cpp.


static const int kMazeFlameWidth = 19;
static const int kMazeFlameHeight = 29;
static const int kMazeFlameFrameSize = kMazeFlameWidth * kMazeFlameHeight;

// Row of the floor where Igor appears or leaves when he comes from or goes to the stairs.
static const int kMazeStairsY = 143;

/**
 * The track depends on the location: the maze is divided in three zones.
 */
int IgorEngine::MAZE_MUSIC_TRACK() const {
	uint8 location = _mazeLocation;
	while (location > 21) {
		location -= 21;
	}
	--location;
	return location / 7 + 5;
}

/**
 * Draws a frame [frame] of a flame at (x, y). The empty pixels of the frame show the
 * room and the pixels with color 1 show either the first frame or the next pixel of the room.
 */
void IgorEngine::mazeDrawFlameFrame(int x, int y, int frame) {
	const int screenOffset = y * 320 + x;
	const uint8 *frames = _animFramesBuffer + (frame + 1) * kMazeFlameFrameSize;
	uint8 tmp[kMazeFlameFrameSize];
	for (int j = 0; j < kMazeFlameHeight; ++j) {
		for (int i = 0; i < kMazeFlameWidth; ++i) {
			const uint8 pixel = _screenVGA[screenOffset + j * 320 + i];
			uint8 color;
			if (pixel == 0xF0 || pixel == 0xF1) {
				color = pixel;
			} else {
				color = frames[j * kMazeFlameWidth + i];
				if (color == 0) {
					color = _screenLayer1[screenOffset + j * 320 + i];
				}
				if (color == 1) {
					if (_mazeRoom->flameMode == kMazeFlameLayer1) {
						color = _screenLayer1[screenOffset + j * 320 + i + 1];
					} else {
						color = _animFramesBuffer[j * kMazeFlameWidth + i];
					}
				}
			}
			tmp[j * kMazeFlameWidth + i] = color;
		}
	}
	for (int j = 0; j < kMazeFlameHeight; ++j) {
		memcpy(_screenVGA + screenOffset + j * 320, tmp + j * kMazeFlameWidth, kMazeFlameWidth);
	}
}

/**
 * Changes the flames random frames and makes the light of the room flicker: the colors of the room and of
 * Igor are set to the (darkened) palette changed by 0, -1 or -2.
 */
void IgorEngine::mazeFlicker() {
	int frames[2] = { 0, 0 };
	// random flame frames
	for (int i = 0; i < _mazeRoom->numFlames; ++i) {
		int frame;
		do {
			frame = getRandomNumber(9);
		} while (frame == _mazeFlameFrame[i]);
		_mazeFlameFrame[i] = frame;
		frames[i] = frame;
	}
	for (int i = 0; i < _mazeRoom->numFlames; ++i) {
		mazeDrawFlameFrame(_mazeRoom->flames[i].x, _mazeRoom->flames[i].y, frames[i]);
	}

	// random palette delta for room flickering
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


void IgorEngine::maybeUpdateFlicker() {
	// the original phases 11, 35 and 59 are the ticks 8, 32 and 56
	if (compareGameTick(16, 24)) {
		mazeFlicker();
	}
}

/**
 * The flames and the light flicker; spooky sounds are heard from time to time, only if no speech is playing.
 */
void IgorEngine::PART_MAZE_UPDATE_ROOM_BACKGROUND() {
	if (_mazeRoom->numFlames > 0) {
		maybeUpdateFlicker();
	}
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

/**
 * Igor appears at the entry and walks to its destination.
 */
void IgorEngine::MAZE_ENTER_WALK(const MazeEntry &entry) {
	WalkData *wd = &_walkData[0];
	wd->setPos(entry.x, entry.y, entry.facing, 0);
	wd->setDefaultScale();
	_walkDataLastIndex = 0;
	const int area = _screenLayer2[entry.y * 320 + entry.x];
	_roomObjectAreasTable[area].area = 1;
	buildWalkPathSimple(entry.x, entry.y, entry.destX, entry.destY);
	_roomObjectAreasTable[area].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(_mazeRoom->numFlames > 0 ? &IgorEngine::maybeUpdateFlicker : 0);
}

void IgorEngine::enterFromStairs(const MazeEntry &entry) {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	WalkData *wd = &_walkData[0];
	for (int i = 0; i < 10;) {
		if (compareGameTick(1, 16)) {
			wd->x = entry.x;
			wd->y = kMazeStairsY;
			wd->posNum = kFacingPositionBack;
			if (i == 9) {
				_walkCurrentFrame = 0;
			}
			wd->frameNum = _walkCurrentFrame;
			WalkData::setNextFrame(wd->posNum, _walkCurrentFrame);
			wd->clipSkipX = 1;
			wd->clipWidth = 30;
			wd->scaleWidth = 23 + i * 3;
			wd->xPosChanged = 1;
			wd->dxPos = 0;
			wd->yPosChanged = 1;
			wd->dyPos = 3;
			wd->scaleHeight = 50;
			moveIgor(wd->posNum, wd->frameNum);
			++i;
		}
		if (_mazeRoom->numFlames > 0) {
			maybeUpdateFlicker();
		}
		waitForTimer();
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
}

/**
 * Igor walks to the exit and the maze moves to the neighbour location.
 */
void IgorEngine::mazeWalkToExit(const MazeAction &action) {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[action.y2 * 320 + action.x2]].area = 1;
	buildWalkPathSimple(action.x1, action.y1, action.x2, action.y2);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(_mazeRoom->numFlames > 0 ? &IgorEngine::maybeUpdateFlicker : 0);
}

void IgorEngine::mazeExitThroughStairs(const MazeAction &action) {
	if (_walkCurrentFrame > 6) {
		_walkCurrentFrame = 1;
	}
	_walkDataCurrentIndex = 0;
	WalkData *wd = &_walkData[0];
	for (int i = 10; i > 0;) {
		if (compareGameTick(1, 16)) {
			wd->x = action.stairsX;
			wd->y = kMazeStairsY;
			wd->posNum = kFacingPositionFront;
			wd->frameNum = _walkCurrentFrame;
			WalkData::setNextFrame(wd->posNum, _walkCurrentFrame);
			wd->clipSkipX = 1;
			wd->clipWidth = 30;
			wd->scaleWidth = 23 + (i - 1) * 3;
			wd->xPosChanged = 1;
			wd->dxPos = 0;
			wd->yPosChanged = 1;
			wd->dyPos = 3;
			wd->scaleHeight = 50;
			moveIgor(wd->posNum, wd->frameNum);
			--i;
		}
		if (_mazeRoom->numFlames > 0) {
			maybeUpdateFlicker();
		}
		waitForTimer();
	}
}

/**
 * Moves to the location next to the current one. The state is the shape of the new room and the side of it Igor
 * comes in from: the opposite of the side he left.
 */
void IgorEngine::mazeGoToNeighbor(int dir) {
	_mazeLocation = MAZE_NODES[_mazeLocation].neighbors[dir];
	_currentPart = MAZE_NODES[_mazeLocation].shape * 10 + 500 + (dir + 2) % 4;
}

void IgorEngine::PART_MAZE_EXEC_ACTION(int action) {
	for (int i = 0; i < _mazeRoom->numActions; ++i) {
		const MazeAction &a = _mazeRoom->actions[i];
		if (a.code != action) {
			continue;
		}
		switch (a.kind) {
		case kMazeActionExit:
			mazeWalkToExit(a);
			mazeGoToNeighbor(a.dir);
			break;
		case kMazeActionStairsExit:
			mazeExitThroughStairs(a);
			mazeGoToNeighbor(a.dir);
			break;
		case kMazeActionExitFixed:
			mazeWalkToExit(a);
			if (a.location >= 0) {
				_mazeLocation = a.location;
			}
			_currentPart = a.state;
			if (a.objectState >= 0) {
				_objectsState[a.objectState] = a.objectStateValue;
			}
			break;
		case kMazeActionDialogue:
			for (int j = 0; j < a.numLines; ++j) {
				ADD_DIALOGUE_TEXT(a.lines[j].text, a.lines[j].count, a.lines[j].sound);
			}
			SET_DIALOGUE_TEXT(a.dialogueStart, a.dialogueCount);
			startIgorDialogue();
			break;
		}
		return;
	}
	error("PART_MAZE_EXEC_ACTION unhandled action %d", action);
}

void IgorEngine::PART_MAZE() {
	const MazeRoom *room = getMazeRoom(getPart());
	assert(room);
	_mazeRoom = room;

	playMusic(room->music >= 0 ? room->music : MAZE_MUSIC_TRACK());
	_gameState.enableLight = 1;
	if (room->saveLocation) {
		_mazeSavedLocation = _mazeLocation;
	}
	loadRoomData(room->pal, room->img, room->box, room->msk, room->txt);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	static const int anm[] = { FRM_MazeEntrance1, 0 };
	loadAnimData(anm);
	loadActionData(room->dat);
	_roomDataOffsets = *room->offsets;
	_roomGiveObjectSize = room->giveObjectSize;
	setRoomClickFix(room->clickFix.yMax, room->clickFix.xMin, room->clickFix.xMax, room->clickFix.scanUp);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_MAZE_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_MAZE_UPDATE_ROOM_BACKGROUND;
	if (room->objectName3) {
		Common::strlcpy(_roomObjectNames[3], room->objectName3, sizeof(_roomObjectNames[3]));
	}
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the whole room is darker than its palette
	for (int color = 1; color <= 207; ++color) {
		if (color > 183 && color < 192) {
			continue;
		}
		for (int i = 0; i < 3; ++i) {
			const int value = _paletteBuffer[color * 3 + i] - room->darkness;
			_paletteBuffer[color * 3 + i] = value < 1 ? 0 : value;
		}
	}

	// draws the first frames of the flames
	for (int i = 0; i < room->numFlames; ++i) {
		mazeDrawFlameFrame(room->flames[i].x, room->flames[i].y, room->flameFrames[i]);
	}
	_mazeFlameFrame[0] = 0;
	_mazeFlameFrame[1] = room->secondFlameFrame;

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		for (int i = 0; i < room->numEntries; ++i) {
			const MazeEntry &entry = room->entries[i];
			if (entry.state == _currentPart) {
				if (entry.kind == kMazeEntryWalk) {
					MAZE_ENTER_WALK(entry);
				} else {
					enterFromStairs(entry);
				}
			}
		}
	}
	enterPartLoop();
	for (;;) {
		bool inRoom = false;
		for (int i = 0; i < room->numEntries; ++i) {
			inRoom |= room->entries[i].state == _currentPart;
		}
		if (!inRoom || _gameStateLoaded || (room->saveLocation && _mazeLocation != _mazeSavedLocation)) {
			break;
		}
		if (_inputVars[kInputEscape] && !_eventQuitGame) {
			// leaves the maze
			_inputVars[kInputEscape] = 0;
			_currentPart = (_objectsState[70] == 1 && _objectsState[71] == 0) ? 500 : 142;
			break;
		}
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
