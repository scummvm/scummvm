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


static const int kPart70BoatOffset = 6 * 320 + 265;
static const int kPart70BoatWidth = 55;
static const int kPart70BoatHeight = 39;

static const int kPart70ExitStates[4] = { 501, 720, 680, 810 };

void IgorEngine::part70ExecuteAction(int action) {
	switch (action) {
	case 101:
	case 102:
	case 103:
	case 104:
		_currentPart = kPart70ExitStates[action - 101];
		break;
	default:
		error("part70ExecuteAction unhandled action %d", action);
		break;
	}
}

/**
 * Draws the boat if we havent sent the message yet
 */
void IgorEngine::part70DrawObjectState() {
	const int frameSize = kPart70BoatWidth * kPart70BoatHeight;
	const int frame = _objectsState[kLighthouseMessageSentStateIndex] == 0 ? 0 : 1;
	copyArea(_screenLayer1, kPart70BoatOffset, 320, _animFramesBuffer + frame * frameSize, kPart70BoatWidth,
			 kPart70BoatWidth, kPart70BoatHeight);
}

/**
 * The colors 192 to 207 rotate for the water animation.
 */
void IgorEngine::part70UpdateBackground() {
	if (compareGameTick(3, 32)) {
		scrollPalette(192, 207);
		setPaletteRange(192, 207);
	}
}

void IgorEngine::part70() {
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part70);
	loadRoomData(PAL_Part70, IMG_Part70, BOX_Part70, MSK_Part70, TXT_Part70);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part70, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_70_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, -1, false);
	partMainSetExecuteAction(1, &IgorEngine::part70ExecuteAction);
	_updateRoomBackground = &IgorEngine::part70UpdateBackground;
	part70DrawObjectState();
	memcpy(_screenVGA, _screenLayer1, 46080);

	// Igor is not shown in the map
	if (!restoreRoomAfterLoad(false)) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		_walkData[0].x = 160;
		_walkData[0].y = 133;
		_walkData[0].scaleWidth = 49;
		_walkData[0].scaleHeight = 49;
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();
	while (_currentPart == 700 && !_gameStateLoaded) {
		runPartLoop();
	}

	memcpy(_paletteBuffer, _currentPalette, 624);
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
