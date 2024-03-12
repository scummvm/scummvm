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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "lilliput/lilliput.h"
#include "engines/util.h"
#include "lilliput/script.h"
#include "lilliput/sound.h"

namespace Lilliput {

LilliputEngine *LilliputEngine::s_Engine = nullptr;

static const byte _basisPalette[768] = {
	0,  0,  0,  0,  0,  42, 0,  42, 0,  0,  42, 42,
	42, 0,  0,  42, 0,  42, 42, 21, 0,  42, 42, 42,
	21, 21, 21, 21, 21, 63, 21, 63, 21, 21, 63, 63,
	63, 21, 21, 63, 21, 63, 63, 63, 21, 63, 63, 63,
	63, 63, 63, 59, 59, 59, 54, 54, 54, 50, 50, 50,
	46, 46, 46, 42, 42, 42, 38, 38, 38, 33, 33, 33,
	29, 29, 29, 25, 25, 25, 21, 21, 21, 17, 17, 17,
	13, 13, 13, 8,  8,  8,  4,  4,  4,  0,  0,  0,
	63, 54, 54, 63, 46, 46, 63, 39, 39, 63, 31, 31,
	63, 23, 23, 63, 16, 16, 63, 8,  8,  63, 0,  0,
	57, 0,  0,  51, 0,  0,  45, 0,  0,  39, 0,  0,
	33, 0,  0,  28, 0,  0,  22, 0,  0,  16, 0,  0,
	63, 58, 54, 63, 54, 46, 63, 50, 39, 63, 46, 31,
	63, 42, 23, 63, 38, 16, 63, 34, 8,  63, 30, 0,
	57, 27, 0,  51, 24, 0,  45, 21, 0,  39, 19, 0,
	33, 16, 0,  28, 14, 0,  22, 11, 0,  16, 8,  0,
	63, 63, 54, 63, 63, 46, 63, 63, 39, 63, 63, 31,
	63, 62, 23, 63, 61, 16, 63, 61, 8,  63, 61, 0,
	57, 54, 0,  51, 49, 0,  45, 43, 0,  39, 39, 0,
	33, 33, 0,  28, 27, 0,  22, 21, 0,  16, 16, 0,
	62, 63, 54, 59, 61, 47, 56, 59, 42, 53, 58, 36,
	50, 56, 32, 47, 54, 26, 44, 52, 22, 41, 50, 17,
	36, 46, 14, 32, 42, 11, 28, 37, 8,  24, 33, 6,
	20, 29, 4,  16, 25, 2,  13, 20, 1,  10, 16, 0,
	54, 63, 54, 48, 61, 48, 43, 59, 43, 38, 58, 38,
	33, 56, 33, 29, 54, 29, 25, 52, 24, 21, 50, 20,
	16, 46, 16, 14, 42, 13, 10, 37, 9,  8,  33, 7,
	6,  29, 4,  4,  25, 2,  2,  20, 1,  1,  16, 0,
	59, 63, 63, 53, 63, 63, 47, 62, 63, 41, 61, 62,
	35, 60, 62, 30, 59, 62, 24, 57, 62, 18, 55, 62,
	20, 52, 56, 15, 47, 50, 11, 42, 45, 8,  37, 39,
	5,  32, 33, 3,  27, 27, 1,  22, 22, 0,  16, 16,
	54, 59, 63, 46, 56, 63, 39, 53, 63, 31, 50, 63,
	23, 47, 63, 16, 44, 63, 8,  42, 63, 0,  39, 63,
	0,  35, 57, 0,  31, 51, 0,  27, 45, 0,  23, 39,
	0,  19, 33, 0,  16, 28, 0,  12, 22, 0,  9,  16,
	54, 54, 63, 46, 47, 63, 39, 39, 63, 31, 32, 63,
	23, 24, 63, 16, 16, 63, 8,  9,  63, 0,  1,  63,
	0,  1,  57, 0,  1,  51, 0,  0,  45, 0,  0,  39,
	0,  0,  33, 0,  0,  28, 0,  0,  22, 0,  0,  16,
	54, 63, 54, 47, 63, 46, 39, 63, 39, 32, 63, 31,
	24, 63, 23, 16, 63, 16, 8,  63, 8,  0,  63, 0,
	0,  56, 0,  0,  49, 0,  0,  43, 0,  0,  36, 0,
	0,  30, 0,  0,  23, 0,  0,  16, 0,  0,  10, 0,
	63, 54, 63, 63, 46, 63, 63, 39, 63, 63, 31, 63,
	63, 23, 63, 63, 16, 63, 63, 8,  63, 63, 0,  63,
	56, 0,  57, 50, 0,  51, 45, 0,  45, 39, 0,  39,
	33, 0,  33, 27, 0,  28, 22, 0,  22, 16, 0,  16,
	63, 58, 55, 63, 56, 52, 63, 54, 49, 63, 53, 47,
	63, 51, 44, 63, 49, 41, 63, 47, 39, 63, 46, 36,
	63, 44, 32, 63, 41, 28, 63, 39, 24, 60, 37, 23,
	58, 35, 22, 55, 34, 21, 52, 32, 20, 50, 31, 19,
	47, 30, 18, 45, 28, 17, 42, 26, 16, 40, 25, 15,
	39, 24, 14, 36, 23, 13, 34, 22, 12, 32, 20, 11,
	29, 19, 10, 27, 18, 9,  23, 16, 8,  21, 15, 7,
	18, 14, 6,  16, 12, 6,  14, 11, 5,  10, 8,  3,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63
};


LilliputEngine::LilliputEngine(OSystem *syst, const LilliputGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	_system = syst;

	setDebugger(new LilliputConsole(this));
	_rnd = nullptr;
	_mousePos = Common::Point(0, 0);
	_oldMousePos = Common::Point(0, 0);
	_mouseDisplayPos = Common::Point(0, 0);
	_mouseButton = 0;
	_mouseClicked = false;
	_savedMousePosDivided = Common::Point(-1, -1);
	_mousePreviousEventType = Common::EVENT_INVALID;
	_skipDisplayFlag1 = 1;
	_skipDisplayFlag2 = 0;
	_displayMap = false;
	_debugFlag = 0;
	_debugFlag2 = 0;

	_scriptHandler = new LilliputScript(this);
	_soundHandler = new LilliputSound();

	_handleOpcodeReturnCode = 0;
	_delayedReactivationAction = false;
	_selectedCharacterId = -1;
	_numCharactersToDisplay = 0;
	_nextDisplayCharacterPos = Common::Point(0, 0);
	_animationTick = 0;
	_byte12A05 = 10; // Used to trigger sound and animations in int8, 1 time out of 10
	_refreshScreenFlag = false;
	_byte16552 = 0;
	_lastInterfaceHotspotIndex = -1;
	_lastInterfaceHotspotButton = 0;
	_lastAnimationTick = 0;

	_currentScriptCharacter = 0;
	_currentScriptCharacterPos = Common::Point(0, 0);
	_host = 0;
	_nextCharacterIndex = 0;
	_waitingSignal = -1;
	_waitingSignalCharacterId = -1;
	_newModesEvaluatedNumber = 0;
	_savedSurfaceUnderMousePos = Common::Point(0, 0);
	_displayGreenHand = false;
	_isCursorGreenHand = false;
	_displayStringIndex = 0;
	_signalTimer = 0;
	_numCharacters = 0;

	_saveFlag = true;
	_actionType = kActionNone;

	_doorEntranceMask[0] = _doorExitMask[3] = 1;
	_doorEntranceMask[1] = _doorExitMask[2] = 2;
	_doorEntranceMask[2] = _doorExitMask[1] = 4;
	_doorEntranceMask[3] = _doorExitMask[0] = 8;

	for (int i = 0; i < 3; i++)
		_codeEntered[i] = 0;

	for (int i = 0; i < 4; i++)
		_homeInDirLikelyhood[i] = 0;

	for (int i = 0; i < 40; i++) {
		_characterTargetPos[i] = Common::Point(0, 0);
		_charactersToDisplay[i] = 0;
		_characterRelativePos[i] = Common::Point(-1, -1);
		_characterDisplay[i] = Common::Point(0, 0);
		_characterMagicPuffFrame[i] = -1;
		_characterSubTargetPos[i] = Common::Point(-1, -1);
		_specialCubes[i] = 0;

		_characterSignals[i] = -1;
		_characterPos[i] = Common::Point(-1, -1);
		_characterPosAltitude[i] = 0;
		_characterFrameArray[i] = 0;
		_characterCarried[i] = -1;
		_characterBehindDist[i] = 4;
		_characterAboveDist[i] = 0;
		_spriteSizeArray[i] = 20;
		_characterDirectionArray[i] = 0;
		_characterMobility[i] = 0;
		_characterTypes[i] = 0;
		_characterBehaviour[i] = 0;
		_characterHomePos[i] = Common::Point(0, 0);
		_signalArr[i] = -1;
	}

	for (int i = 0; i < 30; i++)
		_signalArray[i] = -1;

	for (int i = 0; i < 256; i++)
		_savedSurfaceUnderMouse[i] = 0;

	for (int i = 0; i < 160; i++)
		_displayStringBuf[i] = 0;

	for (int i = 0; i < 1400 + 3120; i++) {
		_characterVariables[i] = 0;
	}

	_currentCharacterAttributes = nullptr;
	_bufferIdeogram = nullptr;
	_bufferMen = nullptr;
	_bufferMen2 = nullptr;
	_bufferIsoChars = nullptr;
	_bufferIsoMap = nullptr;
	_bufferCubegfx = nullptr;

	_sequencesArr = nullptr;
	_packedStringIndex = nullptr;
	_packedStringNumb = 0;
	_packedStrings = nullptr;
	_initScript = nullptr;
	_initScriptSize = 0;
	_menuScript = nullptr;
	_menuScriptSize = 0;
	_arrayGameScriptIndex = nullptr;
	_gameScriptIndexSize = 0;
	_arrayGameScripts = nullptr;
	_listNumb = 0;
	_listIndex = nullptr;
	_listArr = nullptr;
	_rectNumb = 0;
	for (int i = 0; i < 40; ++i)
		_enclosureRect[i] = Common::Rect(0, 0, 0, 0);

	_interfaceHotspotNumb = 0;
	for (int i = 0; i < 20; ++i)
		_keyboardMapping[i] = Common::KEYCODE_DOLLAR;

	_mainSurface = nullptr;
	_smallAnimsFrameIndex = 0;
	_keyDelay = 0;
	_int8Timer = 0;
	_keyboard_nextIndex = 0;
	_keyboard_oldIndex = 0;
	_normalCursor = nullptr;
	_greenCursor = nullptr;
	_word10800_ERULES = 0;
	_currentDisplayCharacter = 0;

	_shouldQuit = false;
	_eventMan = nullptr;
	_lastTime = 0;
	_gameType = kGameTypeNone;
	_platform = Common::kPlatformUnknown;
}

LilliputEngine::~LilliputEngine() {
	delete _soundHandler;
	delete _scriptHandler;
	delete _rnd;
}

void LilliputEngine::update() {
	// update every 20 ms.
	int currentTime = _system->getMillis();
	if (currentTime - _lastTime > 20) {
		_lastTime += ((currentTime - _lastTime) / 20) * 20;
		newInt8();
		pollEvent();
		if (_displayGreenHand == true && _isCursorGreenHand == false) {
			_isCursorGreenHand = true;
			CursorMan.pushCursor(_greenCursor, 16, 16, 0, 0, 0);
		} else if (_displayGreenHand == false && _isCursorGreenHand == true) {
			_isCursorGreenHand = false;
			CursorMan.popCursor();
		}

		_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
		_system->updateScreen();
	}
}

void LilliputEngine::newInt8() {
	_soundHandler->refresh();

	if (_byte12A05 != 0)
		--_byte12A05;
	else {
		_byte12A05 = 10;
		if (_int8Timer != 0)
			--_int8Timer;

		_animationTick ^= 1;
		if (!_refreshScreenFlag)
			displayRefreshScreen();
	}
}

bool LilliputEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *LilliputEngine::getCopyrightString() const {
	return "copyright S.L.Grand, Brainware, 1991 - 1992";
}

GameType LilliputEngine::getGameType() const {
	return _gameType;
}

Common::Platform LilliputEngine::getPlatform() const {
	return _platform;
}

void LilliputEngine::displayCharacter(int index, Common::Point pos, int flags) {
	debugC(2, kDebugEngine, "displayCharacter(%d, %d - %d, %d)", index, pos.x, pos.y, flags);

	byte *buf = _savedSurfaceGameArea1 + (pos.y * 256) + pos.x;

	byte *src = _bufferMen;
	if (index < 0) {
		src = _bufferIdeogram;
		index = -index;
	} else if (index >= 0xF0) {
		src = _bufferMen2;
		index -= 0xF0;
	}

	src += (index * 256);

	if ((flags & 2) == 0) {
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				if (src[x] != 0)
					buf[x] = src[x];
			}
			src += 16;
			buf += 256;
		}
	} else {
		// Sprite mirror
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				// May need a hack of 1 pixel
				if (src[15 - x] != 0)
					buf[x] = src[15 - x];
			}
			src += 16;
			buf += 256;
		}
	}
}

void LilliputEngine::display16x16IndexedBuf(byte *buf, int index, Common::Point pos, bool transparent, bool updateScreen) {
	debugC(2, kDebugEngine, "display16x16IndexedBuf(buf, %d, %d - %d)", index, pos.x, pos.y);

	int index1 = index * 16 * 16;
	byte *newBuf = &buf[index1];

	int vgaIndex = pos.x + (pos.y * 320);

	for (int i = 0; i < 16; i++) {
		// clip on y
		if (pos.y + i < 200) {
			for (int j = 0; j < 16; j++) {
				// clip on x
				if ((newBuf[j] != 0 || !transparent) && (pos.x + j < 320))
					((byte *)_mainSurface->getPixels())[vgaIndex + j] = newBuf[j];
			}
		}
		vgaIndex += 320;
		newBuf += 16;
	}

	if (updateScreen) {
		_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
		_system->updateScreen();
	}
}

void LilliputEngine::display16x16Buf(byte *buf, Common::Point pos, bool transparent, bool updateScreen) {
	debugC(2, kDebugEngine, "display16x16Buf(buf, %d, %d)", pos.x, pos.y);

	display16x16IndexedBuf(buf, 0, pos, transparent, updateScreen);
}

void LilliputEngine::fill16x16Rect(byte col, Common::Point pos) {
	debugC(2, kDebugEngineTBC, "fill16x16Rect(%d, %d - %d)", col, pos.x, pos.y);

	int index = pos.x + (pos.y * 320);
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			((byte *)_mainSurface->getPixels())[index + j] = col;
		}
		index += 320;
	}
}

void LilliputEngine::saveSurfaceGameArea() {
	debugC(2, kDebugEngine, "saveSurfaceGameArea()");

	int index = (16 * 320) + 64; // 5184
	for (int i = 0; i < 176; i++) {
		for (int j = 0; j < 256; j++)
			_savedSurfaceGameArea3[(i * 256) + j] = ((byte *)_mainSurface->getPixels())[index + j];
		index += 320;
	}
}

void LilliputEngine::saveSurfaceSpeech() {
	debugC(2, kDebugEngine, "saveSurfaceSpeech()");

	int index = 66;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 252; j++)
			_savedSurfaceSpeech[(i * 252) + j] = ((byte *)_mainSurface->getPixels())[index + j];
		index += 320;
	}
}

void LilliputEngine::restoreSurfaceSpeech() {
	debugC(2, kDebugEngine, "restoreSurfaceSpeech()");

	int index = 66;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 252; j++)
			((byte *)_mainSurface->getPixels())[index + j] = _savedSurfaceSpeech[(i * 252) + j];
		index += 320;
	}
}


void LilliputEngine::displayInterfaceHotspots() {
	debugC(2, kDebugEngine, "displayInterfaceHotspots()");

	if (_displayMap)
		return;

	for (int index = 0; index < _interfaceHotspotNumb; index++) {
		int tmpVal = _scriptHandler->_interfaceHotspotStatus[index] * 20;
		display16x16IndexedBuf(_bufferIdeogram, tmpVal + index, _interfaceHotspots[index]);
	}
}

void LilliputEngine::displayLandscape() {
	debugC(2, kDebugEngine, "displayLandscape()");

	memcpy(_savedSurfaceGameArea2, _savedSurfaceGameArea3, 176 * 256); // 45056

	int index = (_scriptHandler->_viewportPos.y * 64 + _scriptHandler->_viewportPos.x) * 4;

	for (int posY = 0; posY < 8; posY++) {
		for (int posX = 0; posX < 8 ; posX++) {
			assert (index < 16384);
			displayIsometricBlock(_savedSurfaceGameArea2, _bufferIsoMap[index], posX, posY, 0);
			index += 4;
		}
		index += 224;
	}
}

// Display dialog bubble
void LilliputEngine::displaySpeechBubble() {
	debugC(2, kDebugEngine, "displaySpeechBubble()");
	static const byte _array15976[16] = {244, 248, 250, 250, 252, 252, 252, 252, 252, 252, 252, 252, 250, 250, 248, 244};

	int index = 192;

	for (int i = 0; i < 16; i++) {
		int var3 = _array15976[i];
		int tmpIndex = index - (var3 / 2);
		var3 &= 0xFE;
		for (int j = 0; j < var3; j++) {
			((byte *)_mainSurface->getPixels())[tmpIndex + j] = 17;
		}
		index += 320;
	}
}

void LilliputEngine::displaySpeechLine(int vgaIndex, byte *srcBuf, int &bufIndex) {
	debugC(2, kDebugEngine, "displaySpeechLine()");

	int var3 = 0;
	int var1;
	int bckIndex = bufIndex;

	for (;;) {
		var1 = srcBuf[bufIndex];
		if ((var1 == 0) || (var1 == '|'))
			break;

		++bufIndex;
		++var3;
	}

	var1 = (0x3D - var3) * 2;
	vgaIndex += var1;

	bufIndex = bckIndex;
	for (;;) {
		var1 = srcBuf[bufIndex];
		++bufIndex;
		if ((var1 == 0) || (var1 == '|'))
			break;

		displayChar(vgaIndex, var1);
		vgaIndex += 4;
	}
}

void LilliputEngine::displaySpeech(byte *buf) {
	debugC(2, kDebugEngine, "displaySpeech(%s)", buf);

	int vgaIndex = 70;
	int bufIndex = 0;

	bool multiLineFlag = false;
	byte var1;

	for (;;) {
		var1 = buf[bufIndex];
		++bufIndex;
		if (var1 == 0) {
			vgaIndex += (4 * 320);
			break;
		} else if (var1 == '|') {
			multiLineFlag = true;
			break;
		}
	}

	bufIndex = 0;
	displaySpeechLine(vgaIndex, buf, bufIndex);
	if (multiLineFlag) {
		vgaIndex += (8 * 320);
		displaySpeechLine(vgaIndex, buf, bufIndex);
	}
}

void LilliputEngine::initGameAreaDisplay() {
	debugC(1, kDebugEngine, "initGameAreaDisplay()");

	// display background
	byte *tmpBuf = loadVGA("SCREEN.GFX", 320 * 200, true);
	memcpy(_mainSurface->getPixels(), tmpBuf, 320 * 200);
	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();

	// display game area on top of background
	saveSurfaceGameArea();
	saveSurfaceSpeech();
	displayInterfaceHotspots();
	displayLandscape();
	prepareGameArea();
	displayGameArea();

	free(tmpBuf);
}

void LilliputEngine::displayIsometricBlock(byte *buf, int var1, int posX, int posY, int var3) {
	debugC(1, kDebugEngine, "displayIsometricBlock(buf, %d, %d - %d, %d)", var1, posX, posY, var3);

	byte tmpByte1 = ((7 + posX - posY) << 4) & 0xFF;
	byte tmpByte2 = ((4 + posX + posY - (var3 >> 7)) << 3) & 0xFF;

	int index = (tmpByte2 << 8) + tmpByte1;
	int index2 = var1 << 10;

	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			if (_bufferCubegfx[index2 + j] != 0)
				buf[index + j] = _bufferCubegfx[index2 + j];
		}
		index2 += 32;
		index += 256;
	}
}

void LilliputEngine::displayGameArea() {
	debugC(2, kDebugEngine, "displayGameArea()");

	if (_displayMap)
		return;

	int index = (16 * 320) + 64; // 5184
	for (int i = 0; i < 176; i++) {
		for (int j = 0; j < 256; j++)
			((byte *)_mainSurface->getPixels())[index + j] = _savedSurfaceGameArea1[(i * 256) + j];
		index += 320;
	}

	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void LilliputEngine::restoreMapPoints() {
	debugC(2, kDebugEngine, "restoreMapPoints()");

	byte *buf = (byte *)_mainSurface->getPixels();
	for (byte index = 0; index < _numCharacters; index++) {
		buf[_mapSavedPixelIndex[index]] = _mapSavedPixel[index];
	}
}

void LilliputEngine::displayCharactersOnMap() {
	debugC(2, kDebugEngineTBC, "displayCharactersOnMap()");

	moveCharacters();

	byte *buf = (byte *)_mainSurface->getPixels();
	for (int index = _numCharacters - 1; index >= 0; index--) {
		if (((_characterTypes[index] & 2) == 0) && (_scriptHandler->_characterTilePos[index].y != -1)) {
			// FIXME: This is still wrong, but less. The values in both arrays should be verified now!
			int pixIndex = 320 + ((15 * _scriptHandler->_characterTilePos[index].y) / 4) + (_scriptHandler->_characterTilePos[index].x * 4) + 1;

			_mapSavedPixelIndex[index] = pixIndex;
			_mapSavedPixel[index] = buf[pixIndex];
			buf[pixIndex] = _scriptHandler->_characterMapPixelColor[index];
		}
	}
}

void LilliputEngine::moveCharacters() {
	debugC(2, kDebugEngine, "moveCharacters()");

	_numCharactersToDisplay = 0;
	byte index = _numCharacters - 1;
	Common::Point pos16213 = Common::Point(_scriptHandler->_viewportPos.x << 3, _scriptHandler->_viewportPos.y << 3);

	for (int i = index; i >= 0; i--) {
		if (_characterCarried[i] != -1) {
			int index2 = _characterCarried[i];
			_characterPosAltitude[i] = _characterPosAltitude[index2] + _characterAboveDist[i];
			int8 behindDist = _characterBehindDist[i];
			_characterDirectionArray[i] = _characterDirectionArray[index2];
			int nextPosX = _characterPos[index2].x;
			int nextPosY = _characterPos[index2].y;

			switch (_characterDirectionArray[i]) {
			case 0:
				nextPosX -= behindDist;
				break;
			case 1:
				nextPosY += behindDist;
				break;
			case 2:
				nextPosY -= behindDist;
				break;
			default:
				nextPosX += behindDist;
				break;
			}

			_characterPos[i] = Common::Point(nextPosX, nextPosY);
		}

		_scriptHandler->_characterTilePos[i] = Common::Point(_characterPos[i].x >> 3, _characterPos[i].y >> 3);
		_characterRelativePos[i] = Common::Point(-1, -1);
		_characterDisplay[i] = Common::Point(-1, -1);

		int tileX = (_characterPos[i].x >> 3) - _scriptHandler->_viewportPos.x;
		int tileY = (_characterPos[i].y >> 3) - _scriptHandler->_viewportPos.y;
		if ((tileX >= 0) && (tileX <= 7) && (tileY >= 0) && (tileY <= 7)) {
			_characterRelativePos[i] = Common::Point(tileX, tileY);
			int tempX = _characterPos[i].x - pos16213.x;
			int tempY = _characterPos[i].y - pos16213.y;
			_characterDisplay[i].x = ((60 + tempX - tempY) * 2) & 0xFF;
			_characterDisplay[i].y = (20 + tempX + tempY - _characterPosAltitude[i]) & 0xFF;
			_charactersToDisplay[_numCharactersToDisplay] = i;
			++_numCharactersToDisplay;
		}
	}

	sortCharacters();
}

void LilliputEngine::setNextDisplayCharacter(int var1) {
	debugC(2, kDebugEngine, "setNextDisplayCharacter(%d)", var1);

	byte charNum = var1 & 0xFF;
	if (charNum < _numCharactersToDisplay) {
		int index = _charactersToDisplay[charNum];
		_nextDisplayCharacterPos = _characterRelativePos[index];
	} else
		_nextDisplayCharacterPos = Common::Point(-1, -1);
}

void LilliputEngine::prepareGameArea() {
	debugC(2, kDebugEngine, "prepareGameArea()");

	moveCharacters();
	_currentDisplayCharacter = 0;
	setNextDisplayCharacter(0);

	memcpy(_savedSurfaceGameArea1, _savedSurfaceGameArea2, 176 * 256); // 45056;

	int index1 = (_scriptHandler->_viewportPos.y * 64 + _scriptHandler->_viewportPos.x) * 4;
	assert(index1 < 16384);
	byte *map = &_bufferIsoMap[index1];

	for (int posY = 0; posY < 8; posY++) {
		for (int posX = 0; posX < 8; posX++) {
			if (map[1] != 0xFF) {
				int var1 = map[1];
				if ((_cubeFlags[var1] & 128) != 0)
					var1 += _animationTick;
				displayIsometricBlock(_savedSurfaceGameArea1, var1, posX, posY, 1 << 8);
			}
			renderCharacters(map, Common::Point(posX, posY));

			if (map[2] != 0xFF) {
				int var1 = map[2];
				if ((_cubeFlags[var1] & 128) != 0)
					var1 += _animationTick;
				displayIsometricBlock(_savedSurfaceGameArea1, var1, posX, posY, 2 << 8);
			}
			map += 4;
		}
		map += 224;
	}
}

void LilliputEngine::displayRefreshScreen() {
	debugC(2, kDebugEngine, "displayRefreshScreen()");

	if (_displayMap) {
		bool forceReturnFl = false;
		checkMapClosing(forceReturnFl);
		if (forceReturnFl)
			return;

		restoreMapPoints();
		updateCharPosSequence();
		handleCharacterTimers();
		checkInteractions();
		checkSpecialCubes();
		handleSignals();
		displayCharactersOnMap();
	} else {
		scrollToViewportCharacterTarget();
		checkSpeechClosing();
		prepareGameArea();
		displayGameArea();
		updateCharPosSequence();
		handleCharacterTimers();
		checkInteractions();
		checkSpecialCubes();
		handleSignals();
		handleGameMouseClick();
		checkInterfaceActivationDelay();
		displayHeroismIndicator();
	}
}

void LilliputEngine::resetSmallAnims() {
	debugC(2, kDebugEngine, "resetSmallAnims()");

	_smallAnims[0]._active = false;
	_smallAnims[1]._active = false;
	_smallAnims[2]._active = false;
	_smallAnims[3]._active = false;
	_smallAnimsFrameIndex = 0;
}

void LilliputEngine::displaySmallIndexedAnim(byte index, byte subIndex) {
	debugC(2, kDebugEngine, "displaySmallIndexedAnim(%d, %d)", index, subIndex);

	if (!_smallAnims[index]._active)
		return;

	display16x16IndexedBuf(_bufferIdeogram, _smallAnims[index]._frameIndex[subIndex], _smallAnims[index]._pos, false);
}

void LilliputEngine::displaySmallAnims() {
	debugC(2, kDebugEngine, "displaySmallAnims()");

	if (_animationTick == _lastAnimationTick)
		return;

	_lastAnimationTick = _animationTick;

	assert(_smallAnimsFrameIndex < 8);
	int subIndex = _smallAnimsFrameIndex;
	displaySmallIndexedAnim(0, subIndex);
	displaySmallIndexedAnim(1, subIndex);
	displaySmallIndexedAnim(2, subIndex);
	displaySmallIndexedAnim(3, subIndex);

	++subIndex;
	if (subIndex == 8)
		subIndex = 0;

	_smallAnimsFrameIndex = subIndex;
}

void LilliputEngine::paletteFadeOut() {
	debugC(2, kDebugEngine, "paletteFadeOut()");

	resetSmallAnims();
	byte palette[768];
	for (int fade = 256; fade >= 0; fade -= 8) {
		for (int i = 0; i < 768; i++) {
			palette[i] = (_curPalette[i] * fade) >> 8;
		}
		_system->getPaletteManager()->setPalette(palette, 0, 256);
		_system->updateScreen();
		_system->delayMillis(20);
		pollEvent();
	}
}

void LilliputEngine::paletteFadeIn() {
	debugC(2, kDebugEngine, "paletteFadeIn()");

	byte palette[768];
	for (int fade = 8; fade <= 256; fade += 8) {
		for (int i = 0; i < 768; i++) {
			palette[i] = (_curPalette[i] * fade) >> 8;
		}
		_system->getPaletteManager()->setPalette(palette, 0, 256);
		_system->updateScreen();
		_system->delayMillis(20);
		pollEvent();
	}
}

int16 LilliputEngine::checkObstacle(int x1, int y1, int x2, int y2) {
	debugC(2, kDebugEngine, "checkObstacle(%d, %d, %d, %d)", x1, y1, x2, y2);

	int index = ((y1 * 64) + x1) * 4;
	assert((index > 0) && (index <= 16380));
	byte *isoMap = &_bufferIsoMap[index + 1];

	int16 dx = x2 - x1;
	int16 dy = y2 - y1;

	int16 tmpMapMoveX = 0;
	int16 tmpMapMoveY = 0;
	int16 mapMoveY = 0;
	int16 mapMoveX = 0;

	int16 nonDiagdelta = 0;
	int16 diagDelta = 0;

	if (dx < 0) {
		dx = -dx;
		tmpMapMoveX = -4;
	} else {
		tmpMapMoveX = 4;
	}

	if (dy < 0) {
		dy = -dy;
		tmpMapMoveY = -256;
	} else {
		tmpMapMoveY = 256;
	}

	if (dx >= dy) {
		mapMoveY = 0;
		mapMoveX = tmpMapMoveX;
	} else {
		int16 tmp = dy;
		dy = dx;
		dx = tmp;
		mapMoveX = 0;
		mapMoveY = tmpMapMoveY;
	}

	nonDiagdelta = dy * 2;
	int16 var1 = nonDiagdelta - dx;
	diagDelta = nonDiagdelta - (dx * 2);

	mapMoveX += mapMoveY;
	tmpMapMoveX += tmpMapMoveY;

	int count = 0;

	while (*isoMap == 0xFF) {
		if (var1 >= 0) {
			isoMap += tmpMapMoveX;
			var1 += diagDelta;
		} else {
			isoMap += mapMoveX;
			var1 += nonDiagdelta;
		}

		count++;
		if (count > dx) {
			return 0;
		}
	}
	return tmpMapMoveY;
}

void LilliputEngine::startNavigateFromMap() {
	debugC(2, kDebugEngine, "startNavigateFromMap()");

	_selectedCharacterId = -1;
	_savedMousePosDivided = Common::Point(-1, -1);
	byte newX = _mousePos.x / 4;
	byte newY = _mousePos.y / 3;

	if ((newX >= 64) || (newY >= 64))
		return;

	_savedMousePosDivided = Common::Point(newX, newY);
	_actionType = kCubeSelected;
}

void LilliputEngine::unselectInterfaceHotspots() {
	debugC(2, kDebugEngine, "unselectInterfaceHotspots()");

	for (int index = 0; index < _interfaceHotspotNumb; index++) {
		if (_scriptHandler->_interfaceHotspotStatus[index] == kHotspotSelected)
			_scriptHandler->_interfaceHotspotStatus[index] = kHotspotEnabled;
	}
}

void LilliputEngine::checkMapClosing(bool &forceReturnFl) {
	debugC(2, kDebugEngineTBC, "checkMapClosing()");

	forceReturnFl = false;
	if (!_displayMap)
		return;

	pollEvent();
	if (!_keyboard_checkKeyboard()) {
		_keyboard_getch();
	} else {
		if (_mouseButton != 1)
			return;

		_mouseButton = 0;
		startNavigateFromMap();
	}

	_displayMap = false;
	paletteFadeOut();
	_displayGreenHand = false;
	unselectInterfaceHotspots();
	initGameAreaDisplay();
	_scriptHandler->_heroismLevel = 0;
	moveCharacters();
	paletteFadeIn();
	forceReturnFl = true;
}

void LilliputEngine::checkInteractions() {
	debugC(2, kDebugEngine, "checkInteractions()");

	for (int index = _numCharacters - 1; index >= 0; index--) {
		if (_characterTypes[index] & 1)
			continue;

		int c1 = _scriptHandler->_characterTilePos[index].x;
		int c2 = _scriptHandler->_characterTilePos[index].y;

		// Hack: Skip if disabled (c2 negative)
		if (c2 == -1)
			continue;

		for (int index2 = _numCharacters - 1; index2 >= 0; index2--) {
			byte _newStatus = 0;
			if ((index != index2) &&
				(_characterCarried[index] != index2) &&
				(_characterCarried[index2] != index) &&
				(_characterTypes[index2] & 2) == 0) {
				int d1 = _scriptHandler->_characterTilePos[index2].x;
				int d2 = _scriptHandler->_characterTilePos[index2].y;

				if (d1 != -1) {
					int x = c1 - d1;
					if ((x > -6) && (x < 6)) {
						int y = c2 - d2;
						if ((y > -6) && (y < 6)) {
							_newStatus = 1;

							if ((c1 == d1) && (c2 == d2)) {
								_newStatus = 4;
							} else if ((_characterTypes[index] & 4) != 0) {
								_newStatus = 0;
							} else {
								switch (_characterDirectionArray[index]) {
								case 0:
									if (d1 > c1) {
										_newStatus = 2;

										if (d2 == c2)
											_newStatus = 3;

										if (checkObstacle(c1, c2, d1, d2) != 0)
											_newStatus = 1;
									}
									break;
								case 1:
									if (d2 < c2) {
										_newStatus = 2;

										if (d1 == c1)
											_newStatus = 3;

										if (checkObstacle(c1, c2, d1, d2) != 0)
											_newStatus = 1;
									}
									break;
								case 2:
									if (d2 > c2) {
										_newStatus = 2;

										if (d1 == c1)
											_newStatus = 3;

										if (checkObstacle(c1, c2, d1, d2) != 0)
											_newStatus = 1;
									}
									break;
								default:
									if (d1 < c1) {
										_newStatus = 2;

										if (d2 == c2)
											_newStatus = 3;

										if (checkObstacle(c1, c2, d1, d2) != 0)
											_newStatus = 1;
									}
									break;
								}
							}
						}
					}
				}
			}

			int8 v2 = _scriptHandler->_interactions[index2 + (index * 40)] & 0xFF;
			int8 v1 = v2;

			if (v2 != _newStatus) {
				_scriptHandler->_characterScriptEnabled[index] = 1;
				v2 =  _newStatus;
			}
			_scriptHandler->_interactions[index2 + (index * 40)] = (v1 << 8) + v2;
		}
	}
}

void LilliputEngine::displayCharacterStatBar(int8 type, int16 averagePosX, int8 score, int16 posY) {
	debugC(2, kDebugEngine, "displayCharacterStatBar(%d, %d, %d, %d)", type, averagePosX, score, posY);

	int16 posX = averagePosX;

	// If var equals 45 ('-'), score bar from -x to +x. If not (usually 43 '+'), score bar from 0 to x.
	if (type == 45) {
		posX += 35;
		score -= 35;

		if (score < 0) {
			posX += score;
			score = -score;
		}
	}

	byte *vgaBuf = (byte *)_mainSurface->getPixels();
	int vgaIndex = posX + (320 * posY);

	if (score == 0)
		++score;

	// Draw bar, color green, high = 4, width = score
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < score; j++) {
			vgaBuf[vgaIndex + j] = 2;
		}
		vgaIndex += 320;
	}
}

void LilliputEngine::displayString(byte *buf, Common::Point pos) {
	debugC(2, kDebugEngine, "displayString(%s, %d - %d)", buf, pos.x, pos.y);

	int index = (pos.y * 320) + pos.x;

	int i = 0;
	while (buf[i] != 0) {
		displayChar(index, buf[i]);
		++i;
		index += 4;
	}
}

void LilliputEngine::displayChar(int index, int var1) {
	debugC(2, kDebugEngine, "displayChar(%d, %d)", index, var1);

	int indexVga = index;
	int indexChar = var1 << 5;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++)
			((byte *)_mainSurface->getPixels())[indexVga + j] = _bufferIsoChars[indexChar + j];
		indexVga += 320;
		indexChar += 4;
	}

}

void LilliputEngine::sortCharacters() {
	debugC(2, kDebugEngine, "sortCharacters()");

	if (_numCharactersToDisplay <= 1)
		return;

	for (int var4 = _numCharactersToDisplay - 1; var4 > 0; var4--) {
		bool found = false;

		for (int var2 = 0; var2 < var4; var2++) {
			int index1 = _charactersToDisplay[var2];
			int index2 = _charactersToDisplay[var2 + 1];

			if (_characterRelativePos[index1].y < _characterRelativePos[index2].y)
				continue;

			if (_characterRelativePos[index1].y == _characterRelativePos[index2].y) {
				if (_characterRelativePos[index1].x < _characterRelativePos[index2].x)
					continue;

				if (_characterRelativePos[index1].x == _characterRelativePos[index2].x) {
					if (_characterPosAltitude[index1] < _characterPosAltitude[index2])
						continue;

					if (_characterPosAltitude[index1] == _characterPosAltitude[index2]) {
						if (_characterDisplay[index1].y < _characterDisplay[index2].y)
							continue;
					}
				}
			}

			byte tmpVal = _charactersToDisplay[var2];
			_charactersToDisplay[var2] = _charactersToDisplay[var2 + 1];
			_charactersToDisplay[var2 + 1] = tmpVal;
			found = true;
		}

		if (!found)
			return;
	}
}

void LilliputEngine::scrollToViewportCharacterTarget() {
	debugC(2, kDebugEngine, "scrollToViewportCharacterTarget()");

	if (_scriptHandler->_viewportCharacterTarget == -1)
		return;

	int tileX = (_characterPos[_scriptHandler->_viewportCharacterTarget].x >> 3) - _scriptHandler->_viewportPos.x;
	int tileY = (_characterPos[_scriptHandler->_viewportCharacterTarget].y >> 3) - _scriptHandler->_viewportPos.y;
	Common::Point newPos = _scriptHandler->_viewportPos;

	if (tileX >= 1) {
		if (tileX > 6){
			newPos.x += 4;
			if (newPos.x > 56)
				newPos.x = 56;
		}
	} else {
		newPos.x -= 4;
		if (newPos.x < 0)
			newPos.x = 0;
	}

	if ((tileY < 1) && (newPos.y < 4))
		newPos.y = 0;
	else {
		if (tileY < 1)
			newPos.y -= 4;

		if (tileY > 6) {
			newPos.y += 4;
			if (newPos.y >= 56)
				newPos.y = 56;
		}
	}
	viewportScrollTo(newPos);
}

void LilliputEngine::viewportScrollTo(Common::Point goalPos) {
	debugC(2, kDebugEngine, "viewportScrollTo(%d, %d)", goalPos.x, goalPos.y);

	if (goalPos == _scriptHandler->_viewportPos)
		return;

	int16 dx = 0;
	if (goalPos.x != _scriptHandler->_viewportPos.x) {
		if (goalPos.x < _scriptHandler->_viewportPos.x)
			--dx;
		else
			++dx;
	}

	int16 dy = 0;
	if (goalPos.y != _scriptHandler->_viewportPos.y) {
		if (goalPos.y < _scriptHandler->_viewportPos.y)
			--dy;
		else
			++dy;
	}

	do {
		_scriptHandler->_viewportPos.x += dx;
		_scriptHandler->_viewportPos.y += dy;

		displayLandscape();
		prepareGameArea();
		displayGameArea();

		if (goalPos.x == _scriptHandler->_viewportPos.x)
			dx = 0;

		if (goalPos.y == _scriptHandler->_viewportPos.y)
			dy = 0;
	} while ((dx != 0) || (dy != 0));

	_soundHandler->update();
}

void LilliputEngine::renderCharacters(byte *buf, Common::Point pos) {
	debugC(2, kDebugEngine, "renderCharacters(buf, %d - %d)", pos.x, pos.y);

	if (_nextDisplayCharacterPos != pos)
		return;

	_byte16552 = 0;

	if (buf[1] != 0xFF) {
		int tmpIndex = buf[1];
		if ((_cubeFlags[tmpIndex] & 16) == 0)
			++_byte16552;
	}

	int index = _charactersToDisplay[_currentDisplayCharacter];
	Common::Point characterPos = _characterDisplay[index];

	if (index == _scriptHandler->_talkingCharacter)
		displaySpeechBubbleTail(characterPos);

	if (_byte16552 != 1) {
		byte flag = _characterDirectionArray[index];
		int16 frame = _characterFrameArray[index];

		if (frame != -1) {
			frame += _scriptHandler->_characterPose[index];
			if ((flag & 1) == 1)
				frame += _spriteSizeArray[index];

			if (_characterMagicPuffFrame[index] != -1) {
				frame = _characterMagicPuffFrame[index] + 82;
				--_characterMagicPuffFrame[index];
				frame = -frame;
			}

			displayCharacter(frame, characterPos, flag);
		}
	}

	++_currentDisplayCharacter;
	setNextDisplayCharacter(_currentDisplayCharacter);

	renderCharacters(buf, pos);
}

void LilliputEngine::displaySpeechBubbleTail(Common::Point displayPos) {
	debugC(2, kDebugEngine, "displaySpeechBubbleTail(%d, %d)", displayPos.x, displayPos.y);

	int orgX = displayPos.x + 8;
	int orgY = displayPos.y;
	int var2 = 0;

	int x = orgX;
	int y = orgY;
	do {
		displaySpeechBubbleTailLine(Common::Point(x, y), var2);
		--x;
		y /= 2;
	} while (y != 0);

	x = orgX + 1;
	y = orgY / 2;

	while (y != 0) {
		displaySpeechBubbleTailLine(Common::Point(x, y), var2);
		++x;
		y /= 2;
	}
}

void LilliputEngine::displaySpeechBubbleTailLine(Common::Point pos, int var2) {
	debugC(2, kDebugEngine, "displaySpeechBubbleTailLine(%d - %d, %d)", pos.x, pos.y, var2);

	int index = pos.x + (var2 * 256);
	for (int i = 1 + pos.y - var2; i > 0; i--) {
		_savedSurfaceGameArea1[index] = 17;
		index += 256;
	}
}

void LilliputEngine::checkSpeechClosing() {
	debugC(2, kDebugEngine, "checkSpeechClosing()");

	if (_scriptHandler->_speechTimer != 0) {
		--_scriptHandler->_speechTimer;
		if (_scriptHandler->_speechTimer == 0) {
			restoreSurfaceSpeech();
			_scriptHandler->_talkingCharacter = -1;
		}
	}
}

byte LilliputEngine::getDirection(Common::Point param1, Common::Point param2) {
	debugC(2, kDebugEngine, "getDirection(%d - %d, %d - %d)", param1.x, param1.y, param2.x, param2.y);

	static const byte _directionsArray[8] = {0, 2, 0, 1, 3, 2, 3, 1};

	Common::Point var1 = param2;
	Common::Point var2 = param1;

	int8 var1h = var1.x - var2.x;
	int8 var1l = var1.y - var2.y;
	int8 var2l = 0;

	if (var1h < 0) {
		var2l |= 4;
		var1h = -var1h;
	}

	if (var1l < 0) {
		var2l |= 2;
		var1l = -var1l;
	}

	if (var1h < var1l)
		var2l |= 1;

	return _directionsArray[var2l];
}

byte LilliputEngine::sequenceCharacterHomeIn(int index, Common::Point param1) {
	debugC(2, kDebugEngine, "sequenceCharacterHomeIn(%d, %d - %d)", index, param1.x, param1.y);

	Common::Point target = _characterSubTargetPos[index];

	if (target.x != -1) {
		if (target != _scriptHandler->_characterTilePos[index]) {
			homeInChooseDirection(index);
			_scriptHandler->_characterNextSequence[index] -= (param1.x & 0x0F);
			return kSeqNoInc | kSeqRepeat;
		}

		if (target == _characterTargetPos[index])
			return kSeqRepeat;
	}

	homeInPathFinding(index);

	Common::Point pos1 = _scriptHandler->_characterTilePos[index];
	Common::Point pos2 = _characterSubTargetPos[index];

	_characterDirectionArray[index] = getDirection(pos1, pos2);

	homeInChooseDirection(index);
	_scriptHandler->_characterNextSequence[index] -= (param1.x & 0x0F);
	return kSeqNoInc | kSeqRepeat;
}

void LilliputEngine::homeInPathFinding(int index) {
	debugC(2, kDebugEngine, "homeInPathFinding(%d)", index);

	int16 enclosureSrc = checkEnclosure(_scriptHandler->_characterTilePos[index]);
	int16 enclosureDst = checkEnclosure(_characterTargetPos[index]);

	if (enclosureSrc == enclosureDst) {
		_characterSubTargetPos[index] = _characterTargetPos[index];
		return;
	}

	if (enclosureSrc == -1) {
		int tmpVal = checkOuterEnclosure(_characterTargetPos[index]);
		if (tmpVal == -1)
			warning("homeInPathFinding: Unexpected negative index");
		else
			_characterSubTargetPos[index] = _portalPos[tmpVal];
		return;
	}

	if ((enclosureDst != -1) &&
		(_characterTargetPos[index].x >= _enclosureRect[enclosureSrc].left) &&
		(_characterTargetPos[index].x <= _enclosureRect[enclosureSrc].right) &&
		(_characterTargetPos[index].y >= _enclosureRect[enclosureSrc].top) &&
		(_characterTargetPos[index].y <= _enclosureRect[enclosureSrc].bottom)) {
		_characterSubTargetPos[index] = _portalPos[enclosureDst];
		return;
	}

	_characterSubTargetPos[index] = _portalPos[enclosureSrc];

	if (_enclosureRect[enclosureSrc].left != _enclosureRect[enclosureSrc].right) {
		if (_portalPos[enclosureSrc].x == _enclosureRect[enclosureSrc].left) {
			_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x - 1, _portalPos[enclosureSrc].y);
			return;
		}

		if (_portalPos[enclosureSrc].x == _enclosureRect[enclosureSrc].right) {
			_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x + 1, _portalPos[enclosureSrc].y);
			return;
		}

		if (_enclosureRect[enclosureSrc].bottom != _enclosureRect[enclosureSrc].top) {
			if (_portalPos[enclosureSrc].y == _enclosureRect[enclosureSrc].top)
				_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x, _portalPos[enclosureSrc].y - 1);
			else // CHECKME: Should be a check on y == bottom
				_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x, _portalPos[enclosureSrc].y + 1);

			return;
		}
	}

	int mapIndex = (_portalPos[enclosureSrc].y * 64 + _portalPos[enclosureSrc].x) * 4;
	assert(mapIndex < 16384);

	int tmpVal = _bufferIsoMap[mapIndex + 3];
	if ((tmpVal & 8) != 0)
		_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x + 1, _portalPos[enclosureSrc].y);
	else if ((tmpVal & 4) != 0)
		_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x, _portalPos[enclosureSrc].y - 1);
	else if ((tmpVal & 2) != 0)
		_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x, _portalPos[enclosureSrc].y + 1);
	else
		_characterSubTargetPos[index] = Common::Point(_portalPos[enclosureSrc].x - 1, _portalPos[enclosureSrc].y);

	return;
}

void LilliputEngine::homeInChooseDirection(int index) {
	debugC(2, kDebugEngine, "homeInChooseDirection(%d)", index);

	static const int16 mapArrayMove[4] = {4, -256, 256, -4};

	_curCharacterTilePos = _scriptHandler->_characterTilePos[index];

	evaluateDirections(index);
	int direction = (_characterDirectionArray[index] ^ 3);

	_homeInDirLikelyhood[direction] -= 8;
	byte closeWallFl = 0;

	int mapIndex = ((_curCharacterTilePos.y * 64) + _curCharacterTilePos.x) * 4;
	int retVal = 0;
	for (int i = 3; i >= 0; i--) {
		int mapIndexDiff = mapArrayMove[i];
		assert(mapIndex + mapIndexDiff + 3 < 16384);
		if (((_bufferIsoMap[mapIndex + mapIndexDiff + 3] & _doorEntranceMask[i]) != 0) && ((_bufferIsoMap[mapIndex + 3] & _doorExitMask[i]) != 0)) {
			if ((_bufferIsoMap[mapIndex + mapIndexDiff + 3] & 0x80) != 0 && (homeInAvoidDeadEnds(i, index) != 0)) {
				_homeInDirLikelyhood[i] -= 20;
			}

			int tmpVal = ((_characterMobility[index] & 7) ^ 7);
			retVal = _cubeFlags[_bufferIsoMap[mapIndex + mapIndexDiff]];
			tmpVal &= retVal;
			if (tmpVal == 0)
				continue;
		}
		_homeInDirLikelyhood[i] = -98;
		++closeWallFl;
	}

	if (closeWallFl != 0)
		_homeInDirLikelyhood[_characterDirectionArray[index]] += 3;

	int tmpVal = -99;
	for (int i = 3; i >= 0; i--) {
		if (tmpVal < _homeInDirLikelyhood[i]) {
			retVal = i;
			tmpVal = _homeInDirLikelyhood[i];
		}
	}

	_characterDirectionArray[index] = retVal;
}

byte LilliputEngine::homeInAvoidDeadEnds(int indexb, int indexs) {
	debugC(2, kDebugEngine, "homeInAvoidDeadEnds(%d, %d)", indexb, indexs);

	static const int8 constDirX[4] = {1, 0, 0, -1};
	static const int8 constDirY[4] = {0, -1, 1, 0};

	Common::Point tmpPos = Common::Point(_curCharacterTilePos.x + constDirX[indexb], _curCharacterTilePos.y + constDirY[indexb]);

	int16 idx = checkEnclosure(tmpPos);
	if (idx == -1)
		return 1;

	if ((tmpPos.x >= _enclosureRect[idx].left) && (tmpPos.x <= _enclosureRect[idx].right) && (tmpPos.y >= _enclosureRect[idx].top) && (tmpPos.y <= _enclosureRect[idx].bottom))
		return 0;

	if ((tmpPos.x >= _enclosureRect[idx].left) && (tmpPos.x <= _enclosureRect[idx].right) && (tmpPos.y >= _enclosureRect[idx].top) && (tmpPos.y <= _enclosureRect[idx].bottom))
		return 0;

	return 1;
}

int16 LilliputEngine::checkEnclosure(Common::Point pos) {
	debugC(2, kDebugEngine, "checkEnclosure(%d, %d)", pos.x, pos.y);

	for (int i = 0; i < _rectNumb; ++i) {
		if ((pos.x >= _enclosureRect[i].left) && (pos.x <= _enclosureRect[i].right) && (pos.y >= _enclosureRect[i].top) && (pos.y <= _enclosureRect[i].bottom))
			return i;
	}
	return -1;
}

int16 LilliputEngine::checkOuterEnclosure(Common::Point pos) {
	debugC(2, kDebugEngine, "checkOuterEnclosure(%d, %d)", pos.x, pos.y);

	for (int i = _rectNumb - 1; i >= 0 ; --i) {
		if ((pos.x >= _enclosureRect[i].left) && (pos.x <= _enclosureRect[i].right) && (pos.y >= _enclosureRect[i].top) && (pos.y <= _enclosureRect[i].bottom))
			return i;
	}
	return -1;
}

void LilliputEngine::evaluateDirections(int index) {
	debugC(2, kDebugEngine, "evaluateDirections(%d)", index);

	static const int8 arrayMoveX[4] = {1, 0, 0, -1};
	static const int8 arrayMoveY[4] = {0, -1, 1, 0};

	int16 arrayDistance[4];

	for (int i = 3; i >= 0; i--) {
		int16 var1h = _curCharacterTilePos.x + arrayMoveX[i] - _characterSubTargetPos[index].x;
		int16 var1l = _curCharacterTilePos.y + arrayMoveY[i] - _characterSubTargetPos[index].y;
		arrayDistance[i] = (var1l * var1l) + (var1h * var1h);
	}

	for (int i = 0; i < 4; i++)
		_homeInDirLikelyhood[i] = 0;

	int8 tmpIndex = 0;
	for (int i = 3; i > 0; i--) {
		int16 smallestDistance = 0x7FFF;
		for (int j = 0; j < 4; j++) {
			if (smallestDistance > arrayDistance[j]) {
				smallestDistance = arrayDistance[j];
				tmpIndex = j;
			}
		}
		arrayDistance[tmpIndex] = 0x7FFF;
		_homeInDirLikelyhood[tmpIndex] = i;
	}
}

void LilliputEngine::addCharToBuf(byte character) {
	debugC(2, kDebugEngine, "addCharToBuf(%c)", character);

	_displayStringBuf[_displayStringIndex] = character;
	if (_displayStringIndex < 158)
		++_displayStringIndex;
}

void LilliputEngine::numberToString(int param1) {
	debugC(2, kDebugEngine, "numberToString(%d)", param1);

	static const int exp10[6] = {10000, 1000, 100, 10, 1};

	int var1 = param1;
	bool hideZeros = true;
	for (int i = 0; i < 5; i++) {
		int count = 0;
		while (var1 >= 0) {
			++count;
			var1 -= exp10[i];
		}
		var1 += exp10[i];
		--count;

		byte tmpVal = count + 0x30;

		if (i == 4)
			addCharToBuf(tmpVal);
		else if ((count != 0) || (!hideZeros)) {
			hideZeros = false;
			addCharToBuf(tmpVal);
		}
	}
}

void LilliputEngine::updateCharPosSequence() {
	debugC(2, kDebugEngine, "updateCharPosSequence()");

	int index = _numCharacters - 1;
	byte result;
	while (index >= 0) {
		result = kSeqRepeat;
		while (result & kSeqRepeat) {
			if (_scriptHandler->_characterNextSequence[index] == 16)
				break;

			uint16 index2 = _scriptHandler->_characterNextSequence[index] + (index * 16);
			Common::Point var1 = _scriptHandler->_sequenceArr[index2];

			// /8, then /2 as the function array is a word array
			int16 posSeqType = var1.x / 16;

			switch (posSeqType) {
			case 0: // Move
				// x stands for moveType, y for poseType
				result = sequenceMoveCharacter(index, var1.x, var1.y);
				break;
			case 1: // Face direction
				// x stands for the next direction, y for the poseType
				result = sequenceSetCharacterDirection(index, var1.x, var1.y);
				break;
			case 10: // Seek move target
				result = sequenceSeekMovingCharacter(index, var1);
				break;
			case 11: // Sound
				result = sequenceSound(index, var1);
				break;
			case 12: // Home in target
				result = sequenceCharacterHomeIn(index, var1);
				break;
			case 13: // Character mobility
				result = sequenceSetMobility(index, var1);
				break;
			case 14: // Repeat sequence
				result = sequenceRepeat(index, var1, index2);
				break;
			case 15: // End
				result = sequenceEnd(index);
				break;
			default:
				result = kSeqNone;
				break;
			}

			if ((result & kSeqNoInc) == 0) {
				++_scriptHandler->_characterNextSequence[index];
				if (_scriptHandler->_characterNextSequence[index] == 16)
					_scriptHandler->_characterScriptEnabled[index] = 1;
			}
		}
		--index;
	}
}

byte LilliputEngine::sequenceEnd(int index) {
	debugC(2, kDebugEngine, "sequenceEnd(%d)", index);

	_scriptHandler->_characterNextSequence[index] = 16;
	_scriptHandler->_characterScriptEnabled[index] = 1;

	return kSeqNoInc;
}

byte LilliputEngine::sequenceRepeat(int index, Common::Point var1, int tmpVal) {
	debugC(2, kDebugEngine, "sequenceRepeat(%d, %d - %d, %d)", index, var1.x, var1.y, tmpVal);

	byte counter = var1.y;
	if (counter != 0) {
		if ((counter & 0xF0) == 0)
			counter |= (counter << 4);

		counter -= 16;
		_scriptHandler->_sequenceArr[tmpVal] = Common::Point(var1.x, counter);

		if ((counter & 0xF0) == 0)
			return kSeqRepeat;
	}

	_scriptHandler->_characterNextSequence[index] -= (var1.x & 0x0F);
	return kSeqNoInc | kSeqRepeat;
}

byte LilliputEngine::sequenceSetCharacterDirection(int index, int direction, int poseType) {
	debugC(2, kDebugEngine, "sequenceSetCharacterDirection(%d, %d - %d)", index, direction, poseType);

	char newDir = direction & 3;
	_characterDirectionArray[index] = newDir;
	setCharacterPose(index, poseType);

	return kSeqNone;
}

byte LilliputEngine::sequenceSetMobility(int index, Common::Point var1) {
	debugC(2, kDebugEngine, "sequenceSetMobility(%d, %d - %d)", index, var1.x, var1.y);

	_characterMobility[index] = var1.y;
	return kSeqRepeat;
}

byte LilliputEngine::sequenceSound(int index, Common::Point var1) {
	debugC(2, kDebugEngine, "sequenceSound(%d, %d - %d)", index, var1.x, var1.y);

	int param4x = ((index | 0xFF00) >> 8);
	_soundHandler->playSound(var1.y, _scriptHandler->_viewportPos,
		_scriptHandler->_characterTilePos[index], Common::Point(param4x, 0));
	return kSeqRepeat;
}

byte LilliputEngine::sequenceSeekMovingCharacter(int index, Common::Point var1) {
	debugC(2, kDebugEngine, "sequenceSeekMovingCharacter(%d, %d - %d)", index, var1.x, var1.y);

	int charIndex = _scriptHandler->_characterSeek[index];
	Common::Point charPos = _scriptHandler->_characterTilePos[charIndex];

	if ((_characterSubTargetPos[index].x != -1) && (_characterSubTargetPos[index] == _characterTargetPos[index]))
		_characterSubTargetPos[index] = charPos;

	_characterTargetPos[index] = charPos;

	return sequenceCharacterHomeIn(index, var1);
}

void LilliputEngine::checkSpecialCubes() {
	debugC(2, kDebugEngine, "checkSpecialCubes()");

	for (int index1 = _numCharacters - 1; index1 >= 0; index1--) {
		// Hack: The original doesn't check if it's disabled, which looks wrong
		if ((_scriptHandler->_characterTilePos[index1].x == -1) || (_scriptHandler->_characterTilePos[index1].y == -1))
			continue;
		//

		int mapIndex = 3 + (_scriptHandler->_characterTilePos[index1].y * 64 + _scriptHandler->_characterTilePos[index1].x) * 4;
		assert((mapIndex >= 0) && (mapIndex < 16384));
		byte var1 = _bufferIsoMap[mapIndex] & 0x40;

		if (var1 == _specialCubes[index1])
			continue;

		_specialCubes[index1] = var1;
		if (var1 != 0)
			_scriptHandler->_characterScriptEnabled[index1] = 1;
	}
}

void LilliputEngine::handleCharacterTimers() {
	debugC(2, kDebugEngine, "handleCharacterTimers()");

	int index1 = _animationTick + 2;

	for (byte i = 0; i < _numCharacters; i++) {
		byte *varPtr = getCharacterAttributesPtr(index1);
		if (varPtr[0] != 0) {
			if (varPtr[0] == 1) {
				varPtr[0] = 0;
			} else {
				--varPtr[0];
				if (varPtr[0] == 1)
					_scriptHandler->_characterScriptEnabled[i] = 1;
			}
		}

		index1 += 32;
	}
}

void LilliputEngine::keyboard_handleInterfaceShortcuts(bool &forceReturnFl) {
	debugC(2, kDebugEngine, "keyboard_handleInterfaceShortcuts()");

	forceReturnFl = false;

	if (!_keyboard_checkKeyboard())
		return;

	Common::Event event = _keyboard_getch();

	int8 index = -1;
	for (int8 i = 0; i < _interfaceHotspotNumb; i++) {
		if (event.kbd.keycode == _keyboardMapping[i]) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		byte button = 1;
		if (event.type == Common::EVENT_KEYUP)
			button = 2;
		handleInterfaceHotspot(index, button);
		forceReturnFl = true;
	}
}

void LilliputEngine::checkNumericCode() {
	debugC(2, kDebugEngine, "checkNumericCode()");

	static bool altKeyFl = false;
	static int16 keyCount = 0;

	if (_keyboard_oldIndex == _keyboard_nextIndex)
		return;

	Common::Event oldEvent = _keyboard_buffer[_keyboard_oldIndex];
	if ((oldEvent.kbd.keycode == Common::KEYCODE_LALT) || (oldEvent.kbd.keycode == Common::KEYCODE_RALT)) {
		if (oldEvent.type == Common::EVENT_KEYDOWN) {
			altKeyFl = true;
			keyCount = 0;
			return;
		} else if (oldEvent.type == Common::EVENT_KEYUP) {
			altKeyFl = false;
			if (keyCount == 3)
				_actionType = kCodeEntered;
			return;
		}
	}

	if (keyCount >= 3)
		return;

	if ((altKeyFl) && (oldEvent.type == Common::EVENT_KEYDOWN)) {
		switch (oldEvent.kbd.keycode) {
		case Common::KEYCODE_KP0:
		case Common::KEYCODE_KP1:
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_KP3:
		case Common::KEYCODE_KP4:
		case Common::KEYCODE_KP5:
		case Common::KEYCODE_KP6:
		case Common::KEYCODE_KP7:
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_KP9:
		case Common::KEYCODE_0:
		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
		case Common::KEYCODE_6:
		case Common::KEYCODE_7:
		case Common::KEYCODE_8:
		case Common::KEYCODE_9:
			_codeEntered[keyCount] = oldEvent.kbd.keycode - Common::KEYCODE_0;
			++keyCount;
			break;
		default:
			break;
		}
	}
}

void LilliputEngine::handleGameMouseClick() {
	debugC(2, kDebugEngine, "handleGameMouseClick()");

	checkNumericCode();

	bool forceReturnFl = false;
	keyboard_handleInterfaceShortcuts(forceReturnFl);
	if (forceReturnFl)
		return;

	if (_mouseButton == 0) {
		if (!_mouseClicked)
			return;
		_mouseClicked = false;
		_mouseButton = 2;
	}

	int button = _mouseButton;
	_mouseButton = 0;

	if (button == 2) {
		if (_lastInterfaceHotspotIndex != -1)
			handleInterfaceHotspot(_lastInterfaceHotspotIndex, button);
		return;
	}

	forceReturnFl = false;
	checkInterfaceHotspots(forceReturnFl);
	if (forceReturnFl)
		return;

	Common::Point pos = Common::Point(_mousePos.x - 64, _mousePos.y - 16);

	if ((pos.x < 0) || (pos.x > 255) || (pos.y < 0) || (pos.y > 176))
		return;

	forceReturnFl = false;
	checkClickOnCharacter(pos, forceReturnFl);
	if (forceReturnFl)
		return;

	checkClickOnGameArea(pos);
}

void LilliputEngine::checkClickOnGameArea(Common::Point pos) {
	debugC(2, kDebugEngine, "checkClickOnGameArea(%d, %d)", pos.x, pos.y);

	int x = pos.x - 8;
	int y = pos.y - 4;

	x = (x / 16) - 7;
	y = (y / 8) - 4;

	int arrowY = (y - x) >> 1;
	int arrowX = y - arrowY;

	if ((arrowX >= 0) && (arrowY >= 0) && (arrowX < 8) && (arrowY < 8)) {
		arrowX += _scriptHandler->_viewportPos.x;
		arrowY += _scriptHandler->_viewportPos.y;
		_savedMousePosDivided = Common::Point(arrowX, arrowY);
		_actionType = kCubeSelected;
	}
}

void LilliputEngine::checkClickOnCharacter(Common::Point pos, bool &forceReturnFl) {
	debugC(2, kDebugEngine, "checkClickOnCharacter(%d, %d)", pos.x, pos.y);

	forceReturnFl = false;

	for (int8 i = 0; i < _numCharacters; i++) {
		// check if position is over a character
		if ((pos.x >= _characterDisplay[i].x) && (pos.x <= _characterDisplay[i].x + 17) && (pos.y >= _characterDisplay[i].y) && (pos.y <= _characterDisplay[i].y + 17) && (i != _host)) {
			_selectedCharacterId = i;
			_actionType = kActionGoto;
			if (_delayedReactivationAction)
				_actionType = kActionTalk;

			forceReturnFl = true;
			return;
		}
	}
}

void LilliputEngine::checkInterfaceHotspots(bool &forceReturnFl) {
	debugC(2, kDebugEngine, "checkInterfaceHotspots()");

	forceReturnFl = false;
	for (int index = _interfaceHotspotNumb - 1; index >= 0; index--) {
		if (isMouseOverHotspot(_mousePos, _interfaceHotspots[index])) {
			handleInterfaceHotspot(index, 1);
			forceReturnFl = true;
			return;
		}
	}
}

bool LilliputEngine::isMouseOverHotspot(Common::Point mousePos, Common::Point hotspotPos) {
	debugC(2, kDebugEngine, "isMouseOverHotspot(%d - %d, %d - %d)", mousePos.x, mousePos.y, hotspotPos.x, hotspotPos.y);

	if ((mousePos.x < hotspotPos.x) || (mousePos.y < hotspotPos.y) || (mousePos.x > hotspotPos.x + 16) || (mousePos.y > hotspotPos.y + 16))
		return false;

	return true;
}

void LilliputEngine::handleInterfaceHotspot(byte index, byte button) {
	debugC(2, kDebugEngine, "handleInterfaceHotspot(%d, %d)", index, button);

	if (_scriptHandler->_interfaceHotspotStatus[index] < kHotspotEnabled)
		return;

	_lastInterfaceHotspotIndex = index;
	_lastInterfaceHotspotButton = button;

	if (button == 2) {
		if (!_delayedReactivationAction) {
			_scriptHandler->_interfaceHotspotStatus[index] = kHotspotEnabled;
			_actionType = kButtonReleased;
			displayInterfaceHotspots();
		}
		return;
	}

	if (_delayedReactivationAction) {
		unselectInterfaceButton();
		return;
	}

	unselectInterfaceHotspots();
	_scriptHandler->_interfaceHotspotStatus[index] = kHotspotSelected;
	if (_interfaceTwoStepAction[index] == 1) {
		_delayedReactivationAction = true;
		_displayGreenHand = true;
	} else {
		_actionType = kButtonPressed;
	}

	displayInterfaceHotspots();
}

void LilliputEngine::setCharacterPose(int charIdx, int poseIdx) {
	debugC(2, kDebugEngine, "setCharacterPose(%d, %d)", charIdx, poseIdx);

	// CHECKME: Add an assert on poseIdx to check if it's between 0 and 31?
	int index = (charIdx * 32) + poseIdx;
	_scriptHandler->_characterPose[charIdx] = _poseArray[index];
}

byte LilliputEngine::sequenceMoveCharacter(int idx, int moveType, int poseType) {
	debugC(2, kDebugEngine, "sequenceMoveCharacter(%d, %d - %d)", idx, moveType, poseType);

	setCharacterPose(idx, poseType);

	int index = idx;
	switch (moveType) {
	case 0:
		// No movement
		break;
	case 1:
		moveCharacterSpeed2(index);
		break;
	case 2:
		moveCharacterSpeed4(index);
		break;
	case 3:
		moveCharacterBack2(index);
		break;
	case 4:
		turnCharacter1(index);
		break;
	case 5:
		turnCharacter2(index);
		break;
	case 6:
		moveCharacterUp1(index);
		break;
	case 7:
		moveCharacterUp2(index);
		break;
	case 8:
		moveCharacterDown1(index);
		break;
	case 9:
		moveCharacterDown2(index);
		break;
	case 10:
		moveCharacterSpeed3(index);
		break;
	default:
		// CHECKME: It's so bad it could be an error()
		warning("sequenceMoveCharacter - Unexpected value %d", moveType);
	}

	return kSeqNone;
}

void LilliputEngine::turnCharacter1(int index) {
	debugC(2, kDebugEngine, "turnCharacter1(%d)", index);

	static const byte nextDirection[4] = {1, 3, 0, 2};
	_characterDirectionArray[index] = nextDirection[_characterDirectionArray[index]];
}

void LilliputEngine::turnCharacter2(int index) {
	debugC(2, kDebugEngine, "turnCharacter2(%d)", index);

	static const byte nextDirection[4] = {2, 0, 3, 1};
	_characterDirectionArray[index] = nextDirection[_characterDirectionArray[index]];
}

void LilliputEngine::moveCharacterUp1(int index) {
	debugC(2, kDebugEngine, "moveCharacterUp1(%d)", index);

	_characterPosAltitude[index] += 1;
}

void LilliputEngine::moveCharacterUp2(int index) {
	debugC(2, kDebugEngine, "moveCharacterUp2(%d)", index);

	_characterPosAltitude[index] += 2;
}

void LilliputEngine::moveCharacterDown1(int index) {
	debugC(2, kDebugEngine, "moveCharacterDown1(%d)", index);

	_characterPosAltitude[index] -= 1;
}

void LilliputEngine::moveCharacterDown2(int index) {
	debugC(2, kDebugEngine, "moveCharacterDown2(%d)", index);

	_characterPosAltitude[index] -= 2;
}

void LilliputEngine::moveCharacterSpeed2(int index) {
	debugC(2, kDebugEngine, "moveCharacterSpeed2(%d)", index);

	moveCharacterForward(index, 2);
}

void LilliputEngine::moveCharacterSpeed4(int index) {
	debugC(2, kDebugEngine, "moveCharacterSpeed4(%d)", index);

	moveCharacterForward(index, 4);
}

void LilliputEngine::moveCharacterBack2(int index) {
	debugC(2, kDebugEngine, "moveCharacterBack2(%d)", index);

	moveCharacterForward(index, -2);
}

void LilliputEngine::moveCharacterSpeed3(int index) {
	debugC(2, kDebugEngine, "moveCharacterSpeed3(%d)", index);

	moveCharacterForward(index, 3);
}

void LilliputEngine::moveCharacterForward(int index, int16 speed) {
	debugC(2, kDebugEngine, "moveCharacterForward(%d, %d)", index, speed);

	int16 newX = _characterPos[index].x;
	int16 newY = _characterPos[index].y;
	switch (_characterDirectionArray[index]) {
	case 0:
		newX += speed;
		break;
	case 1:
		newY -= speed;
		break;
	case 2:
		newY += speed;
		break;
	default:
		newX -= speed;
		break;
	}
	checkCollision(index, Common::Point(newX, newY), _characterDirectionArray[index]);
}

void LilliputEngine::checkCollision(int index, Common::Point pos, int direction) {
	debugC(2, kDebugEngine, "checkCollision(%d, %d - %d, %d)", index, pos.x, pos.y, direction);

	int16 diffX = pos.x >> 3;
	if (((diffX & 0xFF) == _scriptHandler->_characterTilePos[index].x) && ((pos.y >> 3) == _scriptHandler->_characterTilePos[index].y)) {
		_characterPos[index] = pos;
		return;
	}

	if ((pos.x < 0) || (pos.x >= 512) || (pos.y < 0) || (pos.y >= 512))
		return;

	int mapIndex = (_scriptHandler->_characterTilePos[index].y * 64 + _scriptHandler->_characterTilePos[index].x) * 4;
	assert(mapIndex < 16384);

	if ((_bufferIsoMap[mapIndex + 3] & _doorExitMask[direction]) == 0)
		return;

	mapIndex = ((pos.y & 0xFFF8) << 3) + diffX;
	mapIndex <<= 2;

	if ((_bufferIsoMap[mapIndex + 3] & _doorEntranceMask[direction]) == 0)
		return;

	byte var1 = _characterMobility[index];
	var1 &= 7;
	var1 ^= 7;

	if ((var1 & _cubeFlags[_bufferIsoMap[mapIndex]]) != 0)
		return;

	_characterPos[index] = pos;
}

void LilliputEngine::signalDispatcher(byte type, byte index, int var4) {
	debugC(2, kDebugEngine, "signalDispatcher(%d, %d, %d)", type, index, var4);

	if (type == 0) { // Message sent to one target character
		sendMessageToCharacter(index, var4);
		return;
	}

	if (type == 3) { // Broadcast - Sent to all characters
		for (int i = _numCharacters - 1; i >= 0; i--)
			sendMessageToCharacter(i, var4);
		return;
	}

	int index2 = var4 & 0xFF;
	for (byte i = 0; i < _numCharacters; i++) {
		if ((_scriptHandler->_interactions[index2] & 0xFF) >= type)
			sendMessageToCharacter(i, var4);
		index2 += 40;
	}
}

void LilliputEngine::sendMessageToCharacter(byte index, int var4) {
	debugC(2, kDebugEngine, "sendMessageToCharacter(%d, %d)", index, var4);

	if (_characterSignals[index] != -1) {
		_signalArr[index] = var4;
	} else {
		_scriptHandler->_characterScriptEnabled[index] = 1;
		_characterSignals[index] = var4;
	}
}

void LilliputEngine::handleSignals() {
	debugC(2, kDebugEngine, "handleSignals()");

	for (byte i = 0; i < _numCharacters; i++) {
		if (_signalArr[i] != -1) {
			_characterSignals[i] = _signalArr[i];
			_signalArr[i] = -1;
			_scriptHandler->_characterScriptEnabled[i] = 1;
		}
	}

	++_signalTimer;

	for (int i = 0; i < 10; i++) {
		if ((_signalArray[(3 * i) + 1] != -1) && (_signalArray[3 * i] == _signalTimer)) {
			int16 var1 = _signalArray[(3 * i) + 1];
			int var4 = _signalArray[(3 * i) + 2];
			_signalArray[(3 * i) + 1] = -1;

			byte type = var1 >> 8;
			byte index = var1 & 0xFF;

			signalDispatcher(type, index, var4);
		}
	}
}

void LilliputEngine::checkInterfaceActivationDelay() {
	debugC(2, kDebugEngine, "checkInterfaceActivationDelay()");

	if (_animationTick != 1)
		return;

	bool needRedraw = false;
	for (int i = 0; i < _interfaceHotspotNumb; i++) {
		if (_scriptHandler->_interfaceButtonActivationDelay[i] != 0) {
			--_scriptHandler->_interfaceButtonActivationDelay[i];
			if (_scriptHandler->_interfaceButtonActivationDelay[i] == 0) {
				_scriptHandler->_interfaceHotspotStatus[i] = kHotspotEnabled;
				needRedraw = true;
			}
		}
	}

	if (needRedraw)
		displayInterfaceHotspots();
}

void LilliputEngine::displayHeroismIndicator() {
	debugC(2, kDebugEngine, "displayHeroismIndicator()");

	if (_scriptHandler->_barAttrPtr == nullptr)
		return;

	int var1 = (_scriptHandler->_barAttrPtr[0] * 25) >> 8;

	if (var1 == _scriptHandler->_heroismLevel)
		return;

	int var2 = 1;
	if (var1 > _scriptHandler->_heroismLevel)
		var1 = 150;
	else {
		var2 = -1;
		var1 = 40;
	}

	_scriptHandler->_heroismLevel += var2;

	int index = _scriptHandler->_heroismBarX + (_scriptHandler->_heroismBarBottomY * 320);

	var2 = _scriptHandler->_heroismLevel & 0xFF;
	if (var2 != 0) {
		for (int i = 0; i < (var2 << 2); i++) {
			((byte *)_mainSurface->getPixels())[index] = var1;
			((byte *)_mainSurface->getPixels())[index + 1] = var1;
			((byte *)_mainSurface->getPixels())[index + 2] = var1;
			index -= 320;
		}
	}

	if (25 - _scriptHandler->_heroismLevel != 0) {
		var2 = (25 - _scriptHandler->_heroismLevel) << 2;
		for (int i = 0; i < var2; i++) {
			((byte *)_mainSurface->getPixels())[index] = 23;
			((byte *)_mainSurface->getPixels())[index + 1] = 23;
			((byte *)_mainSurface->getPixels())[index + 2] = 23;
			index -= 320;
		}
	}
}

void LilliputEngine::pollEvent() {
	debugC(2, kDebugEngine, "pollEvent()");

	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			Common::Point newMousePos = Common::Point(CLIP<int>(event.mouse.x, 0, 304), CLIP<int>(event.mouse.y, 0, 184));

			if (_mousePreviousEventType != event.type) {
				_mousePreviousEventType = event.type;
				if (_mouseButton != 1) {
					_mouseButton = 2;
					if (event.type != Common::EVENT_MOUSEMOVE) {
						_mouseButton = 1;
						_mousePos = Common::Point(newMousePos.x + 5, newMousePos.y + 1);
					}
				} else {
					_mouseClicked = true;
				}
			}

			if (newMousePos != _oldMousePos) {
				_oldMousePos = newMousePos;
				_mouseDisplayPos = newMousePos;
			}
			_lastEventType = event.type;
			}
			break;
		case Common::EVENT_QUIT:
			_shouldQuit = true;
			break;
		case Common::EVENT_KEYUP:
		case Common::EVENT_KEYDOWN: {
			if ((event.type == _lastKeyPressed.type) && (event.kbd == _lastKeyPressed.kbd))
				break;

			_lastKeyPressed = event;
			int nextIndex = (_keyboard_nextIndex + 1) % 8;
			if (_keyboard_oldIndex != nextIndex) {
				_keyboard_buffer[_keyboard_nextIndex] = event;
				_keyboard_nextIndex = nextIndex;
			}

			_lastEventType = event.type;
			}
			break;
		default:
			break;
		}
	}
}

byte *LilliputEngine::loadVGA(const Common::Path &filename, int expectedSize, bool loadPal) {
	debugC(1, kDebugEngine, "loadVGA(%s, %d, %d)", filename.toString().c_str(), expectedSize, (loadPal) ? 1 : 0);

	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.toString().c_str());

	int remainingSize = f.size();
	if (loadPal) {
		for (int i = 0; i < 768; ++i)
			_curPalette[i] = f.readByte();
		remainingSize -= 768;

		fixPaletteEntries(_curPalette, 256);
	}

	uint8 curByte;
	byte *decodeBuffer = (byte *)malloc(expectedSize);
	int size = 0;

	for (; (remainingSize > 0) && (size < expectedSize);) {
		curByte = f.readByte();
		--remainingSize;

		if (curByte == 0xFF)
			break;

		if (curByte & 0x80) {
			// Compressed
			int compSize = (curByte & 0x7F);
			curByte = f.readByte();
			--remainingSize;

			for (int i = 0; i < compSize; ++i) {
				decodeBuffer[size] = curByte;
				++size;
				if (size == expectedSize)
					break;
			}
		} else {
			// Not compressed
			int rawSize = (curByte & 0xFF);
			for (int i = 0; i < rawSize; ++i) {
				decodeBuffer[size] = f.readByte();
				--remainingSize;
				++size;
				if (size == expectedSize)
					break;
			}
		}
	}
	f.close();

	for (int i = size; i < expectedSize; i++)
		decodeBuffer[i] = 0;

	return decodeBuffer;
}

byte *LilliputEngine::loadRaw(const Common::Path &filename, int filesize) {
	debugC(1, kDebugEngine, "loadRaw(%s)", filename.toString().c_str());

	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.toString().c_str());

	byte *res = (byte *)malloc(sizeof(byte) * filesize);
	for (int i = 0; i < filesize; ++i)
		res[i] = f.readByte();

	f.close();
	return res;
}

void LilliputEngine::loadRules() {
	debugC(1, kDebugEngine, "loadRules()");

	static const Common::KeyCode keybMappingArray[26] = {
		Common::KEYCODE_a, Common::KEYCODE_b, Common::KEYCODE_c, Common::KEYCODE_d, Common::KEYCODE_e,
		Common::KEYCODE_f, Common::KEYCODE_g, Common::KEYCODE_h, Common::KEYCODE_i, Common::KEYCODE_j,
		Common::KEYCODE_k, Common::KEYCODE_l, Common::KEYCODE_m, Common::KEYCODE_n, Common::KEYCODE_o,
		Common::KEYCODE_p, Common::KEYCODE_q, Common::KEYCODE_r, Common::KEYCODE_s, Common::KEYCODE_t,
		Common::KEYCODE_u, Common::KEYCODE_v, Common::KEYCODE_w, Common::KEYCODE_x, Common::KEYCODE_y,
		Common::KEYCODE_z};
	Common::File f;
	uint16 curWord;

	Common::Path filename("ERULES.PRG");
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));

	switch (lang) {
	case Common::EN_ANY:
		break;
	case Common::FR_FRA:
		filename = "FRULES.PRG";
		break;
	case Common::IT_ITA:
		filename = "IRULES.PRG";
		break;
	case Common::DE_DEU:
		filename = "GRULES.PRG";
		break;
	default:
		warning("unsupported language, switching back to English");
	}

	if (!f.open(filename))
		error("Missing game file %s", filename.toString().c_str());

	_word10800_ERULES = f.readUint16LE();

	// Chunk 1 : Sequences
	int size = f.readUint16LE();
	_sequencesArr = (byte *)malloc(sizeof(byte) * size);
	for (int i = 0; i < size; ++i)
		_sequencesArr[i] = f.readByte();

	// Chunk 2 : Characters
	_numCharacters = (f.readUint16LE() & 0xFF);
	assert(_numCharacters <= 40);

	for (int i = _numCharacters, j = 0; i != 0; i--, j++) {
		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_characterPos[j].x = curWord;

		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_characterPos[j].y = curWord;

		_characterPosAltitude[j] = (f.readUint16LE() & 0xFF);
		_characterFrameArray[j] = f.readUint16LE();
		_characterCarried[j] = (int8)f.readByte();
		_characterBehindDist[j] = (int8)f.readByte();
		_characterAboveDist[j] = f.readByte();
		_spriteSizeArray[j] = f.readByte();
		_characterDirectionArray[j] = f.readByte();
		_characterMobility[j] = f.readByte();
		_characterTypes[j] = f.readByte();
		_characterBehaviour[j] = f.readByte();
		_characterHomePos[j].x = f.readByte();
		_characterHomePos[j].y = f.readByte();

		for (int k = 0; k < 32; k++)
			_characterVariables[(j * 32) + k] = f.readByte();

		for (int k = 0; k < 32; k++)
			_poseArray[(j * 32) + k] = f.readByte();
	}

	// Chunk 3 & 4 : Packed strings & associated indexes
	_packedStringNumb = f.readSint16LE();
	curWord = f.readSint16LE();

	_packedStringIndex = (int *)malloc(sizeof(int) * _packedStringNumb);
	for (int i = 0; i < _packedStringNumb; ++i)
		_packedStringIndex[i] = f.readUint16LE();

	_packedStrings = (char *)malloc(curWord);
	for (int i = 0; i < curWord; ++i)
		_packedStrings[i] = f.readByte();

	// Chunk 5: Scripts
	// Use byte instead of int, therefore multiply by two the size.
	// This is for converting it into a memory read stream
	_initScriptSize = f.readUint16LE() * 2;
	_initScript = (byte *)malloc(_initScriptSize);
	for (int i = 0; i < _initScriptSize; ++i)
		_initScript[i] = f.readByte();

	// Chunk 6: Menu Script
	_menuScriptSize = f.readUint16LE() * 2;
	_menuScript = (byte *)malloc(sizeof(byte) * _menuScriptSize);
	for (int i = 0; i < _menuScriptSize; ++i)
		_menuScript[i] = f.readByte();

	// Chunk 7 & 8: Game scripts and indexes
	_gameScriptIndexSize = f.readUint16LE();
	// Added one position to keep the total size too, as it's useful later
	_arrayGameScriptIndex = (int *)malloc(sizeof(int) * (_gameScriptIndexSize + 1));
	for (int i = 0; i < _gameScriptIndexSize; ++i)
		_arrayGameScriptIndex[i] = f.readUint16LE();

	curWord = f.readUint16LE();
	_arrayGameScriptIndex[_gameScriptIndexSize] = curWord;

	_arrayGameScripts = (byte *)malloc(sizeof(byte) * curWord);
	for (int i = 0; i < curWord; ++i)
		_arrayGameScripts[i] = f.readByte();

	// Chunk 9 : Cube flags
	for (int i = 0; i < 60; i++)
		_cubeFlags[i] = f.readByte();

	// Chunk 10 & 11 : Lists
	_listNumb = f.readByte();
	assert(_listNumb <= 20);

	if (_listNumb != 0) {
		_listIndex = (int16 *)malloc(sizeof(int16) * _listNumb);
		int totalSize = 0;
		for (int i = 0; i < _listNumb; ++i) {
			_listIndex[i] = totalSize;
			totalSize += f.readByte();
		}
		if (totalSize != 0) {
			_listArr = (byte *)malloc(sizeof(byte) * totalSize);
			for (int i = 0; i < totalSize; i++)
				_listArr[i] = f.readByte();
		}
	}

	// Chunk 12
	_rectNumb = f.readUint16LE();
	assert((_rectNumb >= 0) && (_rectNumb <= 40));

	for (int i = 0; i < _rectNumb; i++) {
		_enclosureRect[i].right = (int16)f.readByte();
		_enclosureRect[i].left = (int16)f.readByte();
		_enclosureRect[i].bottom = (int16)f.readByte();
		_enclosureRect[i].top = (int16)f.readByte();

		int16 tmpValY = (int16)f.readByte();
		int16 tmpValX = (int16)f.readByte();
		_keyPos[i] = Common::Point(tmpValX, tmpValY);

		tmpValY = (int16)f.readByte();
		tmpValX = (int16)f.readByte();
		_portalPos[i] = Common::Point(tmpValX, tmpValY);
	}

	// Chunk 13
	_interfaceHotspotNumb = f.readUint16LE();
	for (int i = 0 ; i < 20; i++)
		_interfaceTwoStepAction[i] = f.readByte();

	for (int i = 0 ; i < 20; i++)
		_interfaceHotspots[i].x = f.readSint16LE();

	for (int i = 0 ; i < 20; i++)
		_interfaceHotspots[i].y = f.readSint16LE();

	for (int i = 0; i < 20; i++) {
		byte curByte = f.readByte();

		if (curByte == 0x20)
			_keyboardMapping[i] = Common::KEYCODE_SPACE;
		else if (curByte == 0xD)
			_keyboardMapping[i] = Common::KEYCODE_RETURN;
		// Hack to avoid xlat out of bounds
		else if (curByte == 0xFF)
			_keyboardMapping[i] = Common::KEYCODE_INVALID; // 0x21; ?
		// Hack to avoid xlat out of bounds
		else if (curByte == 0x00)
			_keyboardMapping[i] = Common::KEYCODE_INVALID; // 0xB4; ?
		else {
			assert((curByte > 0x40) && (curByte <= 0x41 + 26));
			_keyboardMapping[i] = keybMappingArray[curByte - 0x41];
		}
	}
	f.close();
}

void LilliputEngine::displayVGAFile(const Common::Path &fileName) {
	debugC(1, kDebugEngine, "displayVGAFile(%s)", fileName.toString().c_str());

	byte *buffer = loadVGA(fileName, 64000, true);
	memcpy(_mainSurface->getPixels(), buffer, 320*200);
	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
	free(buffer);
}

void LilliputEngine::fixPaletteEntries(uint8 *palette, int num) {
	debugC(1, kDebugEngine, "fixPaletteEntries(palette, %d)", num);
	// Color values are coded on 6bits (for old 6bits DAC)
	for (int32 i = 0; i < num * 3; i++) {
		int32 col = palette[i];
		assert(col < 64);

		col = (col << 2) | (col >> 4);
		if (col > 255)
			col = 255;
		palette[i] = col;
	}
}

void LilliputEngine::initPalette() {
	debugC(1, kDebugEngine, "initPalette()");

	for (int i = 0; i < 768; i++)
		_curPalette[i] = _basisPalette[i];

	fixPaletteEntries(_curPalette, 256);
	_system->getPaletteManager()->setPalette(_curPalette, 0, 256);
}

void LilliputEngine::setCurrentCharacter(int index) {
	debugC(1, kDebugEngine, "setCurrentCharacter(%d)", index);

	assert(index < 40);
	_currentScriptCharacter = index;
	_currentScriptCharacterPos = Common::Point(_characterPos[index].x >> 3, _characterPos[index].y >> 3);
	_currentCharacterAttributes = getCharacterAttributesPtr(_currentScriptCharacter * 32);
}

void LilliputEngine::unselectInterfaceButton() {
	debugC(1, kDebugEngine, "unselectInterfaceButton()");

	_delayedReactivationAction = false;
	_displayGreenHand = false;
	_lastInterfaceHotspotButton = 0;
	unselectInterfaceHotspots();
	displayInterfaceHotspots();
}

void LilliputEngine::handleMenu() {
	debugC(1, kDebugEngine, "handleMenu()");

	if (_actionType == kActionNone)
		return;

	if (_delayedReactivationAction && (_actionType != kActionTalk))
		return;

	setCurrentCharacter(_host);
	debugC(1, kDebugScriptTBC, "========================== Menu Script ==============================");
	_scriptHandler->runMenuScript(ScriptStream(_menuScript, _menuScriptSize));
	debugC(1, kDebugScriptTBC, "========================== End of Menu Script==============================");
	_savedMousePosDivided = Common::Point(-1, -1);
	_selectedCharacterId = -1;

	if (_actionType == kActionTalk)
		unselectInterfaceButton();

	_actionType = kActionNone;
}

void LilliputEngine::handleGameScripts() {
	debugC(1, kDebugEngine, "handleGameScripts()");

	int index = _nextCharacterIndex;
	int i;
	for (i = 0; (_scriptHandler->_characterScriptEnabled[index] == 0) && (i < _numCharacters); i++) {
		++index;
		if (index >= _numCharacters)
			index = 0;
	}

	if (i > _numCharacters)
		return;

	_nextCharacterIndex = (index + 1) % _numCharacters;

	_scriptHandler->_characterScriptEnabled[index] = 0;
	setCurrentCharacter(index);

	_waitingSignal = _characterSignals[index] >> 8;
	_waitingSignalCharacterId = _characterSignals[index] & 0xFF;
	_characterSignals[index] = -1;
	_newModesEvaluatedNumber = 0;

	int tmpVal = _characterBehaviour[index];
	if (tmpVal == 0xFF)
		return;

	/* Decompiler follows

		//_scriptHandler->listAllTexts();

		debugC(1, kDebugEngineTBC, "================= Menu Script ==================");
		ScriptStream script = ScriptStream(_menuScript, _menuScriptSize);
		_scriptHandler->disasmScript(script);
		debugC(1, kDebugEngineTBC, "============= End Menu Script ==================");


		for (int i = 0; i < _gameScriptIndexSize; i++) {
			assert(tmpVal < _gameScriptIndexSize);
			debugC(1, kDebugEngineTBC, "================= Game Script %d ==================", i);
			ScriptStream script = ScriptStream(&_arrayGameScripts[_arrayGameScriptIndex[i]], _arrayGameScriptIndex[i + 1] - _arrayGameScriptIndex[i]);
			_scriptHandler->disasmScript(script);
			debugC(1, kDebugEngineTBC, "============= End Game Script %d ==================", i);
		}

		while (1);
	*/

	//i = index;
	//debugC(1, kDebugEngineTBC, "before char %d, pos %d %d, var0 %d, var1 %d, var2 %d var16 %d, script enabled %d", i, _characterPositionX[i], _characterPositionY[i], *getCharacterVariablesPtr(i * 32 + 0), *getCharacterVariablesPtr(i * 32 + 1), *getCharacterVariablesPtr(i * 32 + 2),  *getCharacterVariablesPtr(i * 32 + 22), _scriptHandler->_characterScriptEnabled[i]);

	assert(tmpVal < _gameScriptIndexSize);
	debugC(1, kDebugEngine, "================= Game Script %d for character %d ==================", tmpVal, index);
	_scriptHandler->runScript(ScriptStream(&_arrayGameScripts[_arrayGameScriptIndex[tmpVal]], _arrayGameScriptIndex[tmpVal + 1] - _arrayGameScriptIndex[tmpVal]));
	debugC(1, kDebugEngine, "============= End Game Script %d for character %d ==================", tmpVal, index);

	//warning("dump char stat");
	//debugC(1, kDebugEngineTBC, "after char %d, pos %d %d, var0 %d, var1 %d, var2 %d var16 %d, script enabled %d", i, _characterPositionX[i], _characterPositionY[i], *getCharacterVariablesPtr(i * 32 + 0), *getCharacterVariablesPtr(i * 32 + 1), *getCharacterVariablesPtr(i * 32 + 2),  *getCharacterVariablesPtr(i * 32 + 22), _scriptHandler->_characterScriptEnabled[i]);
}

Common::Error LilliputEngine::run() {
	debugC(1, kDebugEngine, "run()");

	s_Engine = this;
	initialize();
	initGraphics(320, 200);
	_mainSurface = new Graphics::Surface();
	_mainSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	// Setup mixer
	syncSoundSettings();
	_soundHandler->init();

	// Init palette
	initPalette();

	// Load files. In the original, the size was hardcoded
	_bufferIdeogram = loadVGA("IDEOGRAM.VGA", 25600, false);
	_bufferMen = loadVGA("MEN.VGA", 61440, false);
	_bufferMen2 = loadVGA("MEN2.VGA", 61440, false);
	_bufferIsoChars = loadVGA("ISOCHARS.VGA", 4096, false);
	_bufferIsoMap = loadRaw("ISOMAP.DTA", 16384);
	_normalCursor = &_bufferIdeogram[80 * 16 * 16];
	_greenCursor = &_bufferIdeogram[81 * 16 * 16];

	CursorMan.replaceCursor(_normalCursor, 16, 16, 0, 0, 0);
	CursorMan.showMouse(true);

	loadRules();

	_lastTime = _system->getMillis();
	_scriptHandler->runScript(ScriptStream(_initScript, _initScriptSize));

	while (!_shouldQuit) {
		handleMenu();
		handleGameScripts();
		// To be removed when handled in the previous fonctions
		update();
	}

	return Common::kNoError;
}

void LilliputEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_rnd = new Common::RandomSource("robin");
	_rnd->setSeed(42);                              // Kick random number generator
	_shouldQuit = false;

	for (int i = 0; i < 4; i++) {
		_smallAnims[i]._active = false;
		_smallAnims[i]._pos = Common::Point(0, 0);
		for (int j = 0; j < 8; j ++)
			_smallAnims[i]._frameIndex[j] = 0;
	}
}

byte *LilliputEngine::getCharacterAttributesPtr(int16 index) {
	debugC(1, kDebugEngine, "getCharacterVariablesPtr(%d)", index);

	assert((index > -3120) && (index < 1400));
	if (index >= 0)
		return &_characterVariables[index];
	else
		return &_characterVariables[1400 - index];
}

void LilliputEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String LilliputEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}

Common::Event LilliputEngine::_keyboard_getch() {
	warning("getch()");
	while(_keyboard_nextIndex == _keyboard_oldIndex)
		pollEvent();

	Common::Event tmpEvent = _keyboard_buffer[_keyboard_oldIndex];
	_keyboard_oldIndex = (_keyboard_oldIndex + 1) % 8;

	return tmpEvent;
}

bool LilliputEngine::_keyboard_checkKeyboard() {
	return (_keyboard_nextIndex != _keyboard_oldIndex);
}

void LilliputEngine::_keyboard_resetKeyboardBuffer() {
	_keyboard_nextIndex = _keyboard_oldIndex = 0;
}

} // End of namespace Lilliput
