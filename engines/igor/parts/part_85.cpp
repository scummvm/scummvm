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

#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

static int VAR_CURRENT_TALKING_ACTOR;
static bool VAR_WATER_SOUND_PLAYING;


void IgorEngine::part85UpdatePhilipLauraDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		decodeAnimFrame(_animFramesBuffer - 1 + READ_LE_UINT16(_animFramesBuffer + 0x7480), _screenVGA, true);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR != 0) {
			_gameState.counter[0] = getRandomNumber(4) + 8;
		} else {
			_gameState.counter[0] = getRandomNumber(4) + 13;
		}
		decodeAnimFrame(getAnimFrame(0, 0x7480, _gameState.counter[0]), _screenVGA, true);
		break;
	}
}

void IgorEngine::part85DisplayLogo() {
	// The CD executable preserves the last intro frame while it displays a
	// 320x122 title card, then restores that frame before entering part 61.
	memcpy(_screenTextLayer, _screenVGA, 46080);
	playRoomSound(65, 0);

	memcpy(_paletteBuffer, _currentPalette, 768);
	loadData(PAL_IgorIntroLogo, _currentPalette);
	memset(_paletteBuffer + 765, 63, 3);
	uint8 logoColor255[3];
	memcpy(logoColor255, _currentPalette + 765, 3);
	memset(_currentPalette + 765, 0, 3);
	updatePalette(768);
	loadData(IMG_IgorIntroLogo, _screenVGA);

	// Keep the title and subtitle holds on the same refresh cadence as the fades.
	const int refreshRate = 70086; // VGA refresh rate in millihertz.
	auto waitForLogoHold = [this](int timerTicks, int refreshFrames, bool canSkip) {
		const uint32 start = _system->getMillis();
		for (int ticks = 0; ticks < timerTicks; ticks += kTimerTicksCount) {
			part85UpdateBackground();
			if (_eventQuitGame) {
				return;
			}
			if (canSkip && (_inputVars[kInputConfirm] || _inputVars[kInputClick] ||
					_inputVars[kInputEscape] || _inputVars[kInputRightClick])) {
				_inputVars[kInputConfirm] = 0;
				_inputVars[kInputClick] = 0;
				_inputVars[kInputEscape] = 0;
				_inputVars[kInputRightClick] = 0;
				return;
			}
			const int elapsedTicks = MIN(ticks + kTimerTicksCount, timerTicks);
			const uint32 deadline = start + static_cast<uint64>(elapsedTicks) * refreshFrames * 1000000 /
					(timerTicks * refreshRate * _fastMode);
			waitForTimer(-1, true, deadline);
		}
	};
	waitForLogoHold(200, 53, false);

	uint8 *subtitle = loadData(FRM_IgorIntroLogo);
	for (int y = 0; y < 13; ++y) {
		for (int x = 0; x < 206; ++x) {
			const uint8 color = subtitle[y * 206 + x];
			if (color != 255) {
				_screenVGA[(130 + y) * 320 + 57 + x] = (color == 241) ? 0 : 255;
			}
		}
	}
	free(subtitle);

	// Each palette change becomes visible on the next refresh, without
	// advancing the logical game ticks.
	uint32 fadeStart = _system->getMillis();
	int fadeStep = 0;
	// Fade color 255 from black to the logo's white.
	for (int threshold = 63; threshold >= 0; threshold -= 3) {
		waitForTimer(0, true, fadeStart + ++fadeStep * 1000000 / refreshRate / _fastMode);
		for (int i = 765; i < 768; ++i) {
			if (logoColor255[i - 765] >= threshold) {
				const int color = _currentPalette[i] + 3;
				_currentPalette[i] = MIN<int>(color, logoColor255[i - 765]);
			}
		}
		setPaletteRange(255, 255);
		waitForTimer(0);
	}

	waitForLogoHold(1500, 398, true);

	memcpy(_screenVGA, _screenTextLayer, 39040);
	memcpy(_currentPalette, _paletteBuffer, 765);
	setPaletteRange(0, 255);
	// Restoring the scene palette also consumes one refresh.
	waitForTimer(0, true, _system->getMillis() + 1000000 / refreshRate / _fastMode);
	fadeStart = _system->getMillis();
	fadeStep = 0;

	// Fade the subtitle, which remains below the 122 restored rows and uses
	// palette color 255.
	for (int threshold = 0; threshold <= 63; threshold += 3) {
		waitForTimer(0, true, fadeStart + ++fadeStep * 1000000 / refreshRate / _fastMode);
		for (int i = 765; i < 768; ++i) {
			if (logoColor255[i - 765] >= threshold) {
				_currentPalette[i] = (_currentPalette[i] < 3) ? 0 : _currentPalette[i] - 3;
			}
		}
		setPaletteRange(255, 255);
		waitForTimer(0);
	}

	memcpy(_screenVGA, _screenTextLayer, 46080);
	_currentPalette[765] = 63;
	_currentPalette[766] = 63;
	_currentPalette[767] = 63;
	setPaletteRange(255, 255);
	waitForTimer(0);
}

void IgorEngine::part85PlayAnimationFrame(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay) {
	do {
		if (compareGameTick(0, delay)) {
			decodeAnimFrame(_animFramesBuffer + frameOffset2 - 1 + READ_LE_UINT16(_animFramesBuffer + frameOffset1 + firstFrame * 2 - 3), _screenVGA, true);
			++firstFrame;
		}
		part85UpdateBackground();
		if (_inputVars[kInputEscape]) return;
		waitForTimer();
	} while (firstFrame <= lastFrame);
}

void IgorEngine::part85ScrollRight() {
	int x = 1;
	do {
		if (compareGameTick(0, 16)) {
			copyArea(_screenTextLayer, 0, 320, _screenLayer1 + x * 8, 320, 320 - x * 8, 144);
			copyArea(_screenTextLayer, 320 - x * 8, 320, _screenLayer2 + 96, 320, x * 8, 144);
			memcpy(_screenVGA, _screenTextLayer, 46080);
			++x;
		}
		part85UpdateBackground();
		waitForTimer();
	} while (x != 29 && !_inputVars[kInputEscape]);
}

void IgorEngine::part85UpdateBackground() {
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
	if (!VAR_WATER_SOUND_PLAYING) {
		return;
	}
	if (_game.flags & kFlagFloppy /*|| !_gameState.dialogueTextRunning*/) {
		playSound(17, 1);
	}
}

void IgorEngine::part85AnimateIgorHead(int frame) {
	const int offset = (_walkCurrentPos == 4) ? 24564 : 22643;
	// Animate Igor's head by copying the relevant area from the screen and overlaying the head frame.
	copyArea(_screenVGA, offset, 320, _screenLayer2 + offset, 320, 14, 8);
	copyArea(_screenVGA, offset, 320, _igorHeadFrames + (_walkCurrentPos - 1) * 924 + frame * 154, 14, 14, 8, true);
}

// Introduction cutscene
void IgorEngine::part85() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	if(_game.flags & kFlagFloppy) {
		playMusic(2);
	} else playMusic(10);

	// loads part5 room into memory
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	// copies current screenLayer1 into screenLayer2 to hold a copy
	memcpy(_screenLayer2, _screenLayer1, 46080);
	// loads part6 room into memory, intro starts there
	loadRoomData(PAL_SpringBridgeIntro, IMG_SpringBridgeIntro, 0, 0, TXT_SpringBridgeIntro);
	static const int anm[] = { ANM_PhilipLauraIntro, AOF_PhilipLauraIntro, ANM_LauraIntro, AOF_LauraIntro, 0 };
	loadAnimData(anm);
	// copies part6 into screen buffer
	memcpy(_screenVGA, _screenLayer1, 46080);

	_updateDialogue = &IgorEngine::part85UpdatePhilipLauraDialogue;
	_updateRoomBackground = &IgorEngine::part85UpdateBackground;

	// In the CD version the water sound is embedded in the CD track
	if(_game.flags & kFlagFloppy) {
		VAR_WATER_SOUND_PLAYING = true;
		playSound(17, 1);
	}

	fadeIn(768);
	//decode first frame and paint it
	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);

	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// Laura does her hair
	part85PlayAnimationFrame(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;

	// 3 lines of text
	ADD_DIALOGUE_TEXT(201, 3, 545);
	// 1 line in a new screen
	ADD_DIALOGUE_TEXT(204, 1, 546);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(89, 56, 63, 63, 0);

	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	cutsceneSayStart(113, 59, 63, 0, 38, 205, 3, 547);
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	cutsceneSayStart(89, 56, 63, 63, 0, 208, 1, 548);
	VAR_CURRENT_TALKING_ACTOR = 0;
	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	cutsceneSayStart(113, 59, 63, 0, 38, { { 209, 2, 549 }, { 211, 1, 550 }, { 212, 1, 551 }, { 213, 1, 552 }, { 214, 1, 553 } });
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85PlayAnimationFrame(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	cutsceneSayStart(89, 56, 63, 63, 0, { { 215, 1, 554 }, { 216, 2, 555 }, { 218, 1, 556 } });
	VAR_CURRENT_TALKING_ACTOR = 0;
	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	cutsceneSayStart(113, 59, 63, 0, 38, { { 219, 2, 557 }, { 221, 2, 558 } });
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);
	part85PlayAnimationFrame(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85PlayAnimationFrame(0, 0x7481, 18, 37, 20);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85ScrollRight();
	VAR_WATER_SOUND_PLAYING = false;
	stopSound();
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85PlayAnimationFrame(0x74CA, 0xA6C4, 1, 6, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	for (int i = 0; i <= 200 / kTimerTicksCount; ++i) {
		part85UpdateBackground();
		if (_inputVars[kInputEscape]) goto PART_85_EXIT;
		waitForTimer();
	}
	_walkData[0].x = 251;
	_walkData[0].y = 125;
	_walkData[0].scaleWidth = 50;
	_walkDataLastIndex = 1;
	_walkCurrentPos = 4;
	igorSayAndWait(223, 2, 559);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85PlayAnimationFrame(0x74CA, 0xA6C4, 7, 9, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	_walkData[0].x = 250;
	_walkData[0].y = 119;
	_walkCurrentPos = 3;
	igorSayAndWait({ { 225, 2, 560 }, { 227, 3, 561 } });
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	part85PlayAnimationFrame(0x74CA, 0xA6C4, 10, 24, 16);
	if (_game.version == kIdSpaCD) {
		part85DisplayLogo();
	}

PART_85_EXIT:
	stopSound();
	_gameState.dialogueTextRunning = false;
	_inputVars[kInputEscape] = 0;
	_walkData[0].setPos(155, 121, 4, 0);
	_walkData[0].setDefaultScale();
	copyArea(_screenLayer2, 23180, 320, _facingIgorFrames[3], 30, 30, 50, true);
	memcpy(_screenVGA, _screenLayer2, 46080);
	// sets all colors after 624 as black
	memset(_currentPalette + 0x270, 0, 0x8D);
	setPaletteRange(208, 254);
	_currentPart = 61;
	_updateDialogue = 0;
	_updateRoomBackground = 0;
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor
