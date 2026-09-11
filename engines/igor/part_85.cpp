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

void IgorEngine::PART_85() {
	// playMusic(2);
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	memcpy(_screenLayer2, _screenLayer1, 46080);
	loadRoomData(PAL_SpringBridgeIntro, IMG_SpringBridgeIntro, 0, 0, TXT_SpringBridgeIntro);
	static const int anm[] = { ANM_PhilipLauraIntro, AOF_PhilipLauraIntro, ANM_LauraIntro, AOF_LauraIntro, 0 };
	loadAnimData(anm);
	memcpy(_screenVGA, _screenLayer1, 46080);

	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);

	fadeInPalette(768);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// Laura touching her hair
	PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;

	if (_inputVars[kInputEscape])
		goto PART_85_EXIT;

PART_85_EXIT:
	// stopSound();
	_inputVars[kInputEscape] = 0;

	_walkData[0].setPos(155, 121, 4, 0);
	_walkData[0].setDefaultScale();
	copyArea(_screenLayer2, 23180, 320, _facingIgorFrames[3], 30, 30, 50, true);
	memcpy(_screenVGA, _screenLayer2, 46080);
	memset(_currentPalette + 0x270, 0, 0x8D);
	setPaletteRange(208, 254);
	_currentPart = 61;
	_updateRoomBackground = 0;
}

void IgorEngine::PART_85_HELPER_1(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay) {
	do {
		if (compareGameTick(0, delay)) {
			decodeAnimFrame(_animFramesBuffer + frameOffset2 - 1 + READ_LE_UINT16(_animFramesBuffer + frameOffset1 + firstFrame * 2 - 3), _screenVGA, true);
			++firstFrame;
		}
		PART_85_UPDATE_ROOM_BACKGROUND();
		if (_inputVars[kInputEscape]) return;
		waitForTimer();
	} while (firstFrame <= lastFrame);
}

void IgorEngine::PART_85_UPDATE_ROOM_BACKGROUND() {
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
	// if (!VAR_WATER_SOUND_PLAYING) {
	// 	return;
	// }
	// if (_gameState.talkMode == kTalkModeTextOnly || !_gameState.dialogueTextRunning) {
	// 	playSound(17, 1);
	// }
}

} // End of namespace Igor
