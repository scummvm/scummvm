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

#include "lilliput/lilliput.h"
#include "engines/util.h"
#include "lilliput/script.h"
#include "lilliput/sound.h"

namespace Lilliput {

LilliputEngine *LilliputEngine::s_Engine = 0;
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

LilliputEngine::LilliputEngine(OSystem *syst, const LilliputGameDescription *gd) : Engine(syst), _gameDescription(gd)
{
	_system = syst;
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Script debug level");
	DebugMan.addDebugChannel(kDebugSound, "Sound", "Sound debug level");

	_console = new LilliputConsole(this);
	_rnd = 0;
	_int8installed = false;
	_mouseX = 0;
	_mouseY = 0;
	_oldMouseX = 0;
	_oldMouseY = 0;
	_mouseDisplayX = 0;
	_mouseDisplayY = 0;
	_mouseButton = 0;
	_savedMousePosDivided = 0xFFFF;
	_skipDisplayFlag1 = 1;
	_skipDisplayFlag2 = 0;
	_displayMap = 0;
	_debugFlag = 0;
	_byte14837 = 0;

	_scriptHandler = new LilliputScript(this);
	_soundHandler = new LilliputSound(this);

	_byte1714E = 0;
	_byte12FCE = 0;
	_byte129A0 = 0xFF;
	_numCharactersToDisplay = 0;
	_nextDisplayCharacterX = 0;
	_nextDisplayCharacterY = 0;
	_byte12A04 = 0;
	_byte12A05 = 10;
	_byte12A06 = 2;
	_byte12A07 = 0;
	_byte12A08 = 0;
	_byte12A09 = 0;
	_byte1881D = 0;
	_byte16552 = 0;
	_byte12FE4 = 0xFF;
	_byte12FE3 = 0;
	_byte16F08 = 0;

	_rulesBuffer2PrevIndx = 0;
	_currentScriptCharacterPosition = 0;
	_word10804 = 0;
	_word17081_nextIndex = 0;
	_word16EFE = 0xFFFF;
	_word1817B = 0;
	_word15BC8 = 0;
	_word15BCA = 0;
	_word15AC2 = 0;
	_word16213 = 0;
	_word16215 = 0;
	_word15AC2 = 0;
	_displayStringIndex = 0;
	_word1289D = 0;

	_saveFlag = false;
	_byte16F07_menuId = 0;

	_array16C54[0] = _array16C54[3] = 1;
	_array16C54[1] = _array16C54[2] = 2;
	_array16C54[2] = _array16C54[1] = 4;
	_array16C54[3] = _array16C54[0] = 8;

	for (int i = 0; i < 3; i++)
		_array147D1[i] = 0;

	for (int i = 0; i < 40; i++) {
		_array10999[i] = 0;
		_array109C1[i] = 0;
		_charactersToDisplay[i] = 0;
		_characterRelativePositionX[i] = 0xFF;
		_characterRelativePositionY[i] = 0xFF;
		_characterDisplayX[i] = 0;
		_characterDisplayY[i] = 0;
		_array12299[i] = 0xFF;
		_array109E9[i] = 0xFF;
		_array10A11[i] = 0xFF;
		_array16E94[i] = 0;

		_array11D49[i] = 0xFFFF;
		_characterPositionX[i] = 0xFFFF;
		_characterPositionY[i] = 0xFFFF;
		_rulesBuffer2_3[i] = 0;
		_characterFrameArray[i] = 0;
		_rulesBuffer2_5[i] = 0xFF;
		_rulesBuffer2_6[i] = 4;
		_rulesBuffer2_7[i] = 0;
		_spriteSizeArray[i] = 20;
		_characterDirectionArray[i] = 0;
		_rulesBuffer2_10[i] = 0;
		_rulesBuffer2_11[i] = 0;
		_rulesBuffer2_12[i] = 0;
		_rulesBuffer2_13[i] = 0;
		_rulesBuffer2_14[i] = 0;
		_array1289F[i] = 0xFFFF;
	}

	for (int i = 0; i < 30; i++)
		_array12861[i] = 0xFFFF;

	for (int i = 0; i < 256; i++)
		_array15AC8[i] = 0;

	for (int i = 0; i < 160; i++)
		_displayStringBuf[i] = 0;

	_ptr_rulesBuffer2_15 = NULL;
	_bufferIdeogram = NULL;
	_bufferMen = NULL;
	_bufferMen2 = NULL;
	_bufferIsoChars = NULL;
	_bufferIsoMap = NULL;
	_bufferCubegfx = NULL;
}

LilliputEngine::~LilliputEngine() {

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GUI::Debugger *LilliputEngine::getDebugger() {
	return _console;
}

void LilliputEngine::update() {
	// update every 20 ms.
	int currentTime = _system->getMillis();
	if(currentTime - _lastTime > 20) {
		_lastTime += ((currentTime - _lastTime) / 20) * 20;
		newInt8();
		pollEvent();
	}
}

void LilliputEngine::newInt8() {

	if (_byte12A06 == 0) {
		_byte12A06 = 2;
		_byte12A07 ^= 1;
	}
	--_byte12A06;
	// TODO: check 'out 20h, 20h'

	// hack for the title stars because _int8installed is not set at the good place for the moment
	//if (!_int8installed)
	//	return;

	// if (_soundEnabled)
	_soundHandler->contentFct1();

	if (_byte12A08 != 1) {
		_byte12A08 = 1;
		if (_byte12A05 != 0)
			--_byte12A05;
		else {
			_byte12A05 = 10;
			if (_sound_byte16F06 != 0)
				--_sound_byte16F06;

			_byte12A04 ^= 1;
			if (_byte12A09 != 1 && _int8installed) // hack for the title stars because _int8installed is not set at the good place for the moment
				displayFunction16();
		}
		_byte12A08 = 0;
	}
}

bool LilliputEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *LilliputEngine::getCopyrightString() const {
	return "copyright S.L.Grand, Brainware, 1991";
}

GameType LilliputEngine::getGameType() const {
	return _gameType;
}

Common::Platform LilliputEngine::getPlatform() const {
	return _platform;
}

void LilliputEngine::displayCharacter(int index, int x, int y, int flags) {
	debugC(2, kDebugEngine, "displayCharacter(%d, %d, %d, %d)", index, x, y, flags);

	byte *buf = _buffer1_45k + (y << 8) + x;

	byte *src = _bufferMen;
	if (index < 0) {
		src = _bufferIdeogram;
		index = -index;
	} else if (index >= 0xF0) {
		src = _bufferMen2;
		index -= 0xF0;
	}

	src += ((index & 0xFF) << 8) + (index >> 8);

	if ( (flags & 2) == 0 ) {
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

// display mouse cursor, if any
void LilliputEngine::displayFunction1(byte *buf, int var1, int var2, int var4) {
	debugC(2, kDebugEngine, "displayFunction1(buf, %d, %d, %d)", var1, var2, var4);

	int index1 = ((var1 & 0xFF) << 8) + (var1 >> 8);
	byte *newBuf = &buf[index1];

	int tmpVal = ((var4 & 0xFF) << 8) + (var4 >> 8);
	int index2 = var2 + tmpVal + (tmpVal >> 2);

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (newBuf[j] != 0)
				((byte *)_mainSurface->getPixels())[index2 + j] = newBuf[j];
		}
		index2 += 320;
		newBuf += 16;
	}
	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void LilliputEngine::displayFunction1a(byte *buf, int var2, int var4) {
	debugC(2, kDebugEngine, "displayFunction1a(buf, %d, %d)", var2, var4);

	displayFunction1(buf, 0, var2, var4);
}

// save area under mouse cursor
void LilliputEngine::displayFunction2(byte *buf, int var2, int var4) {
	debugC(2, kDebugEngine, "displayFunction2(buf, %d, %d)", var2, var4);

	int tmpVal = ((var4 & 0xFF) << 8) + (var4 >> 8);
	int index2 = var2 + tmpVal + (tmpVal >> 2);

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			buf[(i * 16) + j] = ((byte *)_mainSurface->getPixels())[index2 + j];
		}
		index2 += 320;
	}
}

// Fill 16x16 rect
void LilliputEngine::displayFunction3(int var1, int var2, int var4) {
	debugC(2, kDebugEngine, "displayFunction3(%d, %d, %d)", var1, var2, var4);

	int tmpVal = ((var4 >> 8) + (var4 & 0xFF));
	int index = var2 + tmpVal + (tmpVal >> 2);
	int col = var1 & 0xFF;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			((byte *)_mainSurface->getPixels())[index + j] = col;
		}
		index += 320;
	}
}

// display mouse cursor
void LilliputEngine::displayFunction4() {
	debugC(2, kDebugEngine, "displayFunction4()");

	if ((_skipDisplayFlag1 != 1) && (_skipDisplayFlag2 != 1)) {
		_skipDisplayFlag2 = 1;

		_word15BC8 = _mouseDisplayX;
		_word15BCA = _mouseDisplayY;

		displayFunction2(_array15AC8, _mouseDisplayX, _mouseDisplayY);

		displayFunction1(_bufferIdeogram, _word15AC2 + 80, _mouseDisplayX, _mouseDisplayY);

		_skipDisplayFlag1 = 1;
		_skipDisplayFlag2 = 0;
	}
}

void LilliputEngine::displayFunction5() {
	debugC(2, kDebugEngine, "displayFunction5()");

	if ((_skipDisplayFlag1 != 0) && (_skipDisplayFlag2 != 1)) {
		_skipDisplayFlag2 = 1;
		displayFunction1a(_array15AC8, _word15BC8, _word15BCA);
		_skipDisplayFlag1 = 0;
		_skipDisplayFlag2 = 0;
	}
}

// save game area
void LilliputEngine::displayFunction6() {
	debugC(2, kDebugEngine, "displayFunction6()");

	displayFunction5();

	int index = (16 * 320) + 64;
	for (int i = 0; i < 176; i++) {
		for (int j = 0; j < 256; j++)
			_buffer3_45k[(i * 256) + j] = ((byte *)_mainSurface->getPixels())[index + j];
		index += 320;
	}

	displayFunction4();
}

// save speech zone
void LilliputEngine::displayFunction7() {
	debugC(2, kDebugEngine, "displayFunction7()");

	displayFunction5();

	int index = 66;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 252; j++)
			_buffer10_4032[(i * 252) + j] = ((byte *)_mainSurface->getPixels())[index + j];
		index += 320;
	}

	displayFunction4();
}

void LilliputEngine::displayFunction8() {
	debugC(2, kDebugEngine, "displayFunction8()");

	if (_displayMap == 1)
		return;

	displayFunction5();

	int index = 0;
	int tmpVal;
	for (int i = 0; i < _word12F68_ERULES; i++) {
		tmpVal = ((_scriptHandler->_array122E9[index] << 2) + (_scriptHandler->_array122E9[index] << 4)) & 0xFF;
		displayFunction1(_bufferIdeogram, tmpVal + index, _rulesBuffer13_2[index], _rulesBuffer13_3[index]);
	}

	displayFunction4();
}

void LilliputEngine::displayFunction9() {
	debugC(2, kDebugEngine, "displayFunction9()");

	memcpy(_buffer2_45k, _buffer3_45k, 45056);

	int var1 = (_scriptHandler->_viewportY >> 8) + ((_scriptHandler->_viewportY & 0xFF) << 8) + (_scriptHandler->_viewportX << 2);
	int var2;
	int index = 0;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8 ; j++) {
			var2 = (j << 8) + i;
			displayFunction13(_buffer2_45k, _bufferIsoMap[var1 + index], var2, 0);
			index += 4;
		}
		index += 224;
	}

}

// Display dialog bubble
void LilliputEngine::displayFunction10() {
	debugC(2, kDebugEngine, "displayFunction10()");
	static const byte _array15976[16] = {244, 248, 250, 250, 252, 252, 252, 252, 252, 252, 252, 252, 250, 250, 248, 244};

	displayFunction5();

	int index = 192;
	int tmpIndex;
	int var3;

	for (int i = 0; i < 16; i++) {
		var3 = _array15976[i];
		tmpIndex = index - (var3 / 2);
		var3 &= 0xFE;
		for (int j = 0; j < var3; j++) {
			((byte *)_mainSurface->getPixels())[tmpIndex + j] = 17;
		}
		index += 320;
	}

	displayFunction4();
}

void LilliputEngine::sub15A4C(int &vgaIndex, byte *srcBuf, int &bufIndex) {
	debugC(2, kDebugEngine, "sub15A4C()");

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

	var1 = (0x3D - var3) < 1;
	vgaIndex += var1;

	bufIndex = bckIndex;
	for (;;) {
		var1 = srcBuf[bufIndex];
		++bufIndex;
		if ((var1 == 0) || (var1 == '|'))
			break;

		displayChar(vgaIndex, srcBuf[bufIndex]);
		vgaIndex += 4;
	}

}

void LilliputEngine::displayFunction11(byte *buf) {
	debugC(2, kDebugEngine, "displayFunction11(%s)", buf);

	displayFunction5();

	int vgaIndex = 70;
	int bufIndex = 0;

	byte _byte15A0C = 0;
	int var1;

	for (;;) {
		var1 = buf[bufIndex];
		++bufIndex;
		if (var1 == 0) {
			vgaIndex += (4 * 320);
			break;
		} else if (var1 == 0x7C) {
			_byte15A0C = 1;
			break;
		}
	}

	bufIndex = 0;
	sub15A4C(vgaIndex, buf, bufIndex);
	if (_byte15A0C == 1) {
		vgaIndex += (8 * 320);
		sub15A4C(vgaIndex, buf, bufIndex);
	}

	displayFunction4();
}

void LilliputEngine::displayFunction12() {
	debugC(1, kDebugEngine, "displayFunction12()");

	displayFunction5();

	byte *tmpBuf = loadVGA("SCREEN.GFX", true);
	memcpy(_mainSurface->getPixels(), tmpBuf, 320*200);
	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();

	displayFunction6();
	displayFunction7();
	displayFunction8();
	displayFunction9();
	displayFunction15();
	displayFunction14();

	displayFunction4();
	free(tmpBuf);
}

void LilliputEngine::displayFunction13(byte *buf, int var1, int var2, int var3) {
	debugC(1, kDebugEngine, "displayFunction13(buf, %d, %d, %d)", var1, var2, var3);

	byte tmpByte1 = ((7 + (var2 >> 8) - (var2 & 0xFF)) << 4) & 0xFF;
	byte tmpByte2 = ((4 + (var2 >> 8) + (var2 & 0xFF) - (var3 >> 7) ) << 3) & 0xFF;

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

void LilliputEngine::displayFunction14() {
	debugC(2, kDebugEngine, "displayFunction14()");

	if (_displayMap == 1)
		return;

	if (_mouseDisplayX > 48)
		displayFunction5();

	int index = (16 * 320) + 64;
	for (int i = 0; i < 176; i++) {
		for (int j = 0; j < 256; j++)
			((byte *)_mainSurface->getPixels())[index + j] = _buffer1_45k[(i * 256) + j];
		index += 320;
	}

	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();

	displayFunction4();
}

void LilliputEngine::restoreMapPoints() {
	debugC(2, kDebugEngine, "restoreMapPoints()");

	displayFunction5();

	byte *buf = (byte *)_mainSurface->getPixels();
	for (int index = 0; index < _numCharacters; index++) {
		buf[_word15E5D[index]] = _byte15E35[index];
	}

	displayFunction4();
}

void LilliputEngine::displayCharactersOnMap() {
	debugC(2, kDebugEngine, "displayCharactersOnMap()");

	sub16217();
	displayFunction5();

	byte *buf = (byte *)_mainSurface->getPixels();
	for (int index = _numCharacters - 1; index >=0; index--) {
		if (((_rulesBuffer2_11[index] & 2) == 0) && (_scriptHandler->_array1614B[index] != -1)) {
			 int y = (3 * _scriptHandler->_array1614B[index]) + 1;
			 int x = (_scriptHandler->_array16123[index] * 4) + 1;

			 _word15E5D[index] = y * 320 + x;
			 _byte15E35[index] = buf[y * 320 + x];
			 buf[y * 320 + x] = _scriptHandler->_array128EF[index];
		}
	}
	displayFunction4();
}

void LilliputEngine::sub16217() {
	debugC(2, kDebugEngine, "sub16217()");

	_numCharactersToDisplay = 0;
	int index = _numCharacters - 1;
	_word16213 = _scriptHandler->_viewportX << 3;
	_word16215 = _scriptHandler->_viewportY << 3;

	for (int i = index; i >= 0; i--) {
		if (_rulesBuffer2_5[i] != 0xFF) {
			int index2 = _rulesBuffer2_5[i];
			_rulesBuffer2_3[i] = _rulesBuffer2_3[index2] + _rulesBuffer2_7[i];
			int tmpVal = _rulesBuffer2_6[i];
			_characterDirectionArray[i] = _characterDirectionArray[index2];
			int var3 = _characterPositionX[index2];
			int var4 = _characterPositionY[index2];

			switch (_characterDirectionArray[i]) {
			case 0:
				var3 -= tmpVal;
				break;
			case 1:
				var4 += tmpVal;
				break;
			case 2:
				var4 -= tmpVal;
				break;
			default:
				var3 += tmpVal;
				break;
			}

			_characterPositionX[i] = var3;
			_characterPositionY[i] = var4;
		}

		_scriptHandler->_array16123[i] = _characterPositionX[i] >> 3;
		_scriptHandler->_array1614B[i] = _characterPositionY[i] >> 3;
		_characterRelativePositionX[i] = 0xFF;
		_characterRelativePositionY[i] = 0xFF;
		_characterDisplayX[i] = 0xFF;
		_characterDisplayY[i] = 0xFF;

		int tmpVal2 = (_characterPositionX[i] >> 3) - _scriptHandler->_viewportX;
		int tmpVal3 = (_characterPositionY[i] >> 3) - _scriptHandler->_viewportY;
		if ((tmpVal2 >= 0) && (tmpVal2 <= 7) && (tmpVal3 >= 0) && (tmpVal3 <= 7)) {
			_characterRelativePositionX[i] = tmpVal2;
			_characterRelativePositionY[i] = tmpVal3;
			tmpVal2 = _characterPositionX[i] - _word16213;
			tmpVal3 = _characterPositionY[i] - _word16215;
			int tmpVal4 = _rulesBuffer2_3[i];
			_characterDisplayX[i] = ((60 + tmpVal2 - tmpVal3) * 2) & 0xFF;
			_characterDisplayY[i] = (20 + tmpVal2 + tmpVal3 - tmpVal4) & 0xFF;
			_charactersToDisplay[_numCharactersToDisplay] = i;
			++_numCharactersToDisplay;
		}
	}

	sortCharacters();
}

void LilliputEngine::setNextDisplayCharacter(int var1) {
	debugC(2, kDebugEngine, "setNextDisplayCharacter(%d)", var1);

	byte charNum = var1 & 0xFF;
	if ( charNum < _numCharactersToDisplay) {
		int index = _charactersToDisplay[charNum];
		_nextDisplayCharacterX = _characterRelativePositionX[index];
		_nextDisplayCharacterY = _characterRelativePositionY[index];
	} else {
		_nextDisplayCharacterX = 0xFF;
		_nextDisplayCharacterY = 0xFF;
	}
}

void LilliputEngine::displayFunction15() {
	debugC(2, kDebugEngine, "displayFunction15()");

	sub16217();
	_currentDisplayCharacter = 0;
	setNextDisplayCharacter(0);

	memcpy(_buffer1_45k, _buffer2_45k, 45056);

	int index1 = (_scriptHandler->_viewportY >> 8) + ((_scriptHandler->_viewportY & 0xFF) << 8) + (_scriptHandler->_viewportX << 2);
	byte *map = &_bufferIsoMap[index1];

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int tmpVal = (j << 8) + i;
			if (map[1] != 0xFF) {
				int var1 = map[1];
				if (_rulesChunk9[var1] != 128)
					var1 += _scriptHandler->_byte12A04;
				displayFunction13(_buffer1_45k, var1, tmpVal, 1 << 8);
			}
			renderCharacters(map, j, i);

			if (map[2] != 0xFF) {
				int var1 = map[2];
				if (_rulesChunk9[var1] != 128)
					var1 += _scriptHandler->_byte12A04;
				displayFunction13(_buffer1_45k, var1, tmpVal, 2 << 8);
			}
			map += 4;
		}
		map += 224;
	}
}

void LilliputEngine::displayFunction16() {
	debugC(2, kDebugEngine, "displayFunction16()");

	if (_displayMap == 1) {
		bool forceReturnFl = false;
		sub15F31(forceReturnFl);
		if (forceReturnFl)
			return;

		restoreMapPoints();
		sub16626();
		sub12F37();
		sub16CA0();
		sub16EBC();
		sub171CF();
		displayCharactersOnMap();
	} else {
		scrollToViewportCharacterTarget();
		sub189DE();
		displayFunction15();
		displayFunction14();
		sub16626();
		sub12F37();
		sub16CA0();
		sub16EBC();
		sub171CF();
		sub130EE();
		sub12FE5();
		displayHeroismIndicator();
	}
}

void LilliputEngine::sub1863B() {
	_arr18560[0]._field0 = 0;
	_arr18560[1]._field0 = 0;
	_arr18560[2]._field0 = 0;
	_arr18560[3]._field0 = 0;
	_scriptHandler->_word1855E = 0;
}

void LilliputEngine::paletteFadeOut() {
	debugC(2, kDebugEngine, "paletteFadeOut()");

	sub1863B();
	byte palette[768];
	for (int fade = 256; fade >= 0;	fade -= 8) {
		for (int i = 0; i < 768; i++) {
			palette[i] = (_curPalette[i] * fade) >> 8;
		}
		_system->getPaletteManager()->setPalette(palette, 0, 256);
		_system->updateScreen();
		_system->delayMillis(20);
	}
}

void LilliputEngine::paletteFadeIn() {
	debugC(2, kDebugEngine, "paletteFadeIn()");

	byte palette[768];
	for (int fade = 8; fade <= 256;	fade += 8) {
		for (int i = 0; i < 768; i++) {
			palette[i] = (_curPalette[i] * fade) >> 8;
		}
		_system->getPaletteManager()->setPalette(palette, 0, 256);
		_system->updateScreen();
		_system->delayMillis(20);
	}
}

int LilliputEngine::sub16DD5(int x1, int y1, int x2, int y2) {
	debugC(2, kDebugEngine, "sub16DD5(%d, %d, %d, %d)", x1, y1, x2, y2);

	byte *isoMap = _bufferIsoMap + (x1 << 8) + (y1 << 2) + 1;

	int dx = x2 - x1;
	int dy = y2 - y1;

	int word16DCB = 0;
	int word16DCD = 0;
	int word16DD1 = 0;
	int word16DCF = 0;

	byte byte16DD4 = 0;
	byte byte16DD3 = 0;

	if(dx < 0) {
		dx = -dx;
		word16DCB = -4;
	} else {
		word16DCB = 4;
	}

	if(dy < 0) {
		dy = -dy;
		word16DCD = -256;
	} else {
		word16DCD = 256;
	}

	if(dy > dx) {
		word16DD1 = 0;
		word16DCF = word16DCB;
	} else {
		int tmp = dx;
		dy = dx;
		dx = tmp;
		word16DCF = 0;
		word16DD1 = word16DCD;
	}

	byte16DD4 = (dx << 1) - dy;
	byte16DD3 = (dx << 1) - 2 * dy;
	word16DCF += word16DD1;
	word16DCB += word16DCD;

	int var1 = byte16DD4;
	int count = 0;

	while ( *isoMap == 0xFF ) {
		if (var1 > 0) {
			isoMap += word16DCB;
			var1 += byte16DD3;
		} else {
			isoMap += word16DCF;
			var1 += byte16DD4;
		}

		count++;
		if (count == dy) {
			return 0;
		}
	}
	return 1;
}

void LilliputEngine::sub15F75() {
	debugC(2, kDebugEngine, "sub15F75()");

	_byte129A0 = 0xFF;
	_savedMousePosDivided = 0xFFFF;
	byte newX = _mouseX >> 2; 
	byte newY = _mouseY / 3;

	if ((newX >= 64) || (newY >= 64))
		return;
	
	_savedMousePosDivided = (newX << 8) + newY;
	_byte16F07_menuId = 5;
}

void LilliputEngine::sub130B6() {
	debugC(2, kDebugEngine, "sub130B6()");

	for (int index = 0; index < _word12F68_ERULES; index++) {
		if (_scriptHandler->_array122E9[index] == 3)
			_scriptHandler->_array122E9[index] = 2;
	}
}

void LilliputEngine::sub15F31(bool &forceReturnFl) {
	debugC(2, kDebugEngine, "sub15F31()");

	forceReturnFl = false;
	if (_displayMap != 1)
		return;

	pollEvent();
	warning("sub15F31- TODO: Check keyboard");

	if ((_mouseButton & 1) == 0)
		return;

	_mouseButton = 0;
	sub15F75();
	
	_displayMap = 0;
	paletteFadeOut();
	_word15AC2 = 0;
	sub130B6();
	displayFunction12();
	_scriptHandler->_heroismLevel = 0;
	sub16217();
	paletteFadeIn();
	forceReturnFl = true;
}

void LilliputEngine::sub16CA0() {
	debugC(2, kDebugEngine, "sub16CA0()");

	for (int index = _numCharacters - 1; index >= 0; index--) {
		if (_rulesBuffer2_11[index] & 1)
			continue;

		int c1 = _scriptHandler->_array16123[index];
		int c2 = _scriptHandler->_array1614B[index];

		for (int index2 = _numCharacters - 1; index2 >= 0; index2--) {
			_byte16C9F = 0;
			if ((index != index2 ) &&
				(_rulesBuffer2_5[index] != index2) &&
				(_rulesBuffer2_5[index2] != index) &&
				(_rulesBuffer2_11[index2] & 2) == 0) {
				int d1 = _scriptHandler->_array16123[index2];
				int d2 = _scriptHandler->_array1614B[index2];

				if (d1 != 0xFF) {
					int x = c1 - d1;
					if ((x > -6) && (x < 6)) {
						int y = c2 - d2;
						if ((y > -6) && (y < 6)) {
							_byte16C9F = 1;

							if ((c1 == d1) && (c2 == d2)) {
								_byte16C9F = 4;
							} else if((_rulesBuffer2_11[index] & 4) != 0) {
								_byte16C9F = 0;
							} else {
								if (_characterDirectionArray[index] == 0) {
									if (d1 > c1) {
										_byte16C9F = 2;

										if (d2 == c2)
											_byte16C9F = 3;

										if (sub16DD5(c1, d1, c2, d2) != 0)
											_byte16C9F = 1;
									}
								} else if (_characterDirectionArray[index] == 1) {
									if (d2 < c2) {
										_byte16C9F = 2;

										if(d1 == c1)
											_byte16C9F = 3;

										if (sub16DD5(c1, d1, c2, d2) != 0)
											_byte16C9F = 1;
									}
								} else if (_characterDirectionArray[index] == 2) {
									if (d2 > c2) {
										_byte16C9F = 2;

										if(d1 == c1)
											_byte16C9F = 3;

										if (sub16DD5(c1, d1, c2, d2) != 0)
											_byte16C9F = 1;
									}
								} else {
									if (d1 < c1) {
										_byte16C9F = 2;

										if (d2 == c2)
											_byte16C9F = 3;

										if (sub16DD5(c1, d1, c2, d2) != 0)
											_byte16C9F = 1;
									}
								}
							}
						}
					}
				}
			}

			int val = _scriptHandler->_array10B51[index2 + index * 40];
			val = (val & 0xFF) + ((val & 0xFF) << 8);
			if( (val & 0xFF) != _byte16C9F ) {
				_scriptHandler->_characterScriptEnabled[index] = 1;
				val = (val & 0xFF00) | _byte16C9F;
			}
			_scriptHandler->_array10B51[index2 + index * 40] = val;

		}

	}
}

void LilliputEngine::displayFunction17() {
	debugC(2, kDebugEngine, "displayFunction17()");

	displayFunction5();

	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 252; j++)
			((byte *)_mainSurface->getPixels())[66 + (i * 320) + j] = _buffer10_4032[(252 * i) + j];

	displayFunction4();
}

void LilliputEngine::displayString(byte *buf, int var2, int var4) {
	debugC(2, kDebugEngine, "displayString(buf, %d, %d)", var2, var4);

	int index = var2;
	int tmpVar4 = (var4 >> 8) + ((var4 & 0xFF) << 8);
	index = index + tmpVar4 + (tmpVar4 >> 2);

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

	int var3;
	for (int var4 = _numCharactersToDisplay - 1; var4 != 0; var4--) {
		var3 = 0;

		for (int var2 = 0; var2 < var4; var2++) {
			int index1 = _charactersToDisplay[var2];
			int index2 = _charactersToDisplay[var2 + 1];

			if (_characterRelativePositionY[index1] < _characterRelativePositionY[index2])
				continue;

			if (_characterRelativePositionY[index1] == _characterRelativePositionY[index2]) {
				if (_characterRelativePositionX[index1] < _characterRelativePositionX[index2])
					continue;

				if (_characterRelativePositionX[index1] == _characterRelativePositionX[index2]) {
					if (_rulesBuffer2_3[index1] < _rulesBuffer2_3[index2])
						continue;

					if (_rulesBuffer2_3[index1] == _rulesBuffer2_3[index2]) {
						if (_characterDisplayY[index1] < _characterDisplayY[index2])
							continue;
					}
				}
			}

			byte tmpVal = _charactersToDisplay[var2];
			_charactersToDisplay[var2] = _charactersToDisplay[var2 + 1];
			_charactersToDisplay[var2 + 1] = tmpVal;
			++var3;
		}

		if (var3 == 0)
			return;
	}
}

// Move view port to x/y
void LilliputEngine::scrollToViewportCharacterTarget() {
	debugC(2, kDebugEngine, "scrollToViewportCharacterTarget()");

	if (_scriptHandler->_viewportCharacterTarget == 0xFFFF)
		return;

	int var2 = (_characterPositionX[_scriptHandler->_viewportCharacterTarget] >> 3) - _scriptHandler->_viewportX;
	int var4 = (_characterPositionY[_scriptHandler->_viewportCharacterTarget] >> 3) - _scriptHandler->_viewportY;
	int var1 = _scriptHandler->_viewportX;

	if (var2 >= 1) {
		if (var2 >= 6) {
			var1 += 4;
			if (var1 > 56)
				var1 = 56;
		}
	} else {
		var1 -= 4;
		if (var1 < 0)
			var1 = 0;
	}

	int var3 = _scriptHandler->_viewportY;
	if (var4 >= 1) {
		if (var4 > 6) {
			var3 += 4;
			if (var3 >= 56)
				var3 = 56;
		}
	} else {
		var3 -= 4;
		if (var3 < 0)
			var3 = 0;
	}

	viewportScrollTo(var1, var3);
}

void LilliputEngine::viewportScrollTo(int goalX, int goalY) {
	debugC(2, kDebugEngine, "viewportScrollTo(%d, %d)", goalX, goalY);

	if ((goalX == _scriptHandler->_viewportX) && (goalY == _scriptHandler->_viewportY))
		return;

	int dx = 0;
	if (goalX != _scriptHandler->_viewportX) {
		if (goalX < _scriptHandler->_viewportX)
			--dx;
		else
			++dx;
	}

	int dy = 0;
	if (goalY!= _scriptHandler->_viewportY) {
		if (goalY < _scriptHandler->_viewportY)
			--dy;
		else
			++dy;
	}

	do {
		_scriptHandler->_viewportX += dx;
		_scriptHandler->_viewportY += dy;

		displayFunction9();
		displayFunction15();
		displayFunction14();

		if (goalX == _scriptHandler->_viewportX)
			dx = 0;

		if (goalY == _scriptHandler->_viewportY)
			dy = 0;
	} while ((dx != 0) && (dy != 0));

	_soundHandler->contentFct5();
}

void LilliputEngine::renderCharacters(byte *buf, byte x, byte y) {
	debugC(2, kDebugEngine, "renderCharacters(buf, %d, %d)", x, y);

	if ((_nextDisplayCharacterX != x) || (_nextDisplayCharacterY != y))
		return;

	byte _byte16552 = 0;

	if (buf[1] != 0xFF) {
		int tmpIndex = buf[1];
		if ((_rulesChunk9[tmpIndex] & 16) == 0)
			++_byte16552;
	}

	int index = _charactersToDisplay[_currentDisplayCharacter];
	int displayX = _characterDisplayX[index];
	int displayY = _characterDisplayY[index];

	if (index == _scriptHandler->_word1881B)
		sub1546F(displayX, displayY);

	if (_byte16552 != 1) {
		int flag = _characterDirectionArray[index];
		int frame = _characterFrameArray[index];

		if (frame != 0xFFFF) {
			frame += _scriptHandler->_array10AB1[index];
			if ((flag & 1) == 1)
				frame += _spriteSizeArray[index];

			if (_array12299[index] != 0xFF) {
				frame = _array12299[index] + 82;
				--_array12299[index];
				frame = -frame;
			}

			displayCharacter(frame, displayX, displayY, flag);
		}
	}

	++_currentDisplayCharacter;
	setNextDisplayCharacter(_currentDisplayCharacter);

	renderCharacters(buf, x, y);
}

void LilliputEngine::sub1546F(byte displayX, byte displayY) {
	debugC(2, kDebugEngine, "sub1546F(%d, %d)", displayX, displayY);

	int orgX = displayX + 8;
	int orgY = displayY;
	int var2 = 0;

	int x = orgX;
	int y = orgY;
	do {
		sub15498(x, y, var2);
		--x;
		y >>= 1;
	} while (y != 0);

	x = orgX + 1;
	y = orgY >> 1;

	while (y != 0) {
		sub15498(x, y, var2);
		++x;
		y >>= 1;
	}
}

void LilliputEngine::sub15498(byte x, byte y, int var2) {
	debugC(2, kDebugEngine, "sub15498(%d, %d, %d)", x, y, var2);

	int index = x + ((var2 & 0xFF) << 8) + (var2 >> 8);
	for (int i = 1 + y - var2; i > 0; i--) {
		_buffer1_45k[index] = 17;
		index += 256;
	}
}

void LilliputEngine::sub189DE() {
	debugC(2, kDebugEngine, "sub189DE()");

	if (_byte1881D != 0) {
		--_byte1881D;
		if (_byte1881D == 0) {
			displayFunction17();
			_scriptHandler->_word1881B = 0xFFFF;
		}
	}
}

int LilliputEngine::sub16B0C(int param1, int param2) {
	debugC(2, kDebugEngine, "sub16B0C(%d, %d)", param1, param2);

	static const byte _array16B04[8] = {0, 2, 0, 1, 3, 2, 3, 1};

	int var1 = param2;
	int var2 = param1;

	int8 var1h = (var1 >>8) - (var2 >>8);
	int8 var1l = (var1 & 0xFF) - (var2 & 0xFF);
	int8 var2h = 0;
	int8 var2l = 0;

	if (var1h < var2h) {
		var2l |= 4;
		var1h = -var1h;
	}

	if (var1l < var2h) {
		var2l |= 2;
		var1l = -var1l;
	}

	if (var1h < var1l)
		var2l |= 1;

	return _array16B04[var2l];
}

int LilliputEngine::sub16799(int param1, int index) {
	debugC(2, kDebugEngine, "sub16799(%d, %d)", param1, index);

	byte var3h = _array109E9[index];
	byte var3l = _array10A11[index];

	if (var3h != 0xFF) {
		if ((var3h != _scriptHandler->_array16123[index]) || (var3l != _scriptHandler->_array1614B[index])) {
			warning("sub_1693A");
			_scriptHandler->_array12811[index] -= (param1 >> 8) & 0x0F;
			return 3;
		}

		if ((var3h == _scriptHandler->_array12811[index]) && (var3l == _array109C1[index]))
			return 2;
	}

	warning("sub_167EF");
	int var1 = (_scriptHandler->_array16123[index] << 8) + _scriptHandler->_array1614B[index];
	int var2 = (_array109E9[index] << 8) + _array10A11[index];

	_characterDirectionArray[index] = sub16B0C(var1, var2);

	warning("sub_1693A");
	_scriptHandler->_array12811[index] -= (param1 >> 8) & 0x0F;
	return 3;

}

void LilliputEngine::addCharToBuf(byte character) {
	debugC(2, kDebugEngine, "addCharToBuf(%c)", character);

	_displayStringBuf[_displayStringIndex] = character;
	if (_displayStringIndex < 158)
		++_displayStringIndex;
}

void LilliputEngine::prepareGoldAmount(int param1) {
	debugC(2, kDebugEngine, "prepareGoldAmount(%d)", param1);

	static const int _array18AE3[6] = {10000, 1000, 100, 10, 1};

	int count;
	int var1 = param1;
	bool hideZeros = true;
	for (int i = 0; i < 5; i++) {
		count = 0;
		while (var1 >= 0) {
			++count;
			var1 -= _array18AE3[i];
		}
		var1 += _array18AE3[i];
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

void LilliputEngine::sub16626() {
	debugC(2, kDebugEngine, "sub16626()");

	int index = _numCharacters - 1;
	int result;
	while (index >= 0) {
		result = 2;
		while (result & 2) {
			int var2 = _scriptHandler->_array12811[index];
			if (var2 == 16)
				break;

			var2 = (2 * (var2 & 0xFF)) + (index << 5);
			int var1 = _scriptHandler->_array12311[var2 / 2];
			int tmpVal = var2;
			var2 = ((var1 >> 8) >> 3);
			var2 &= 0xFE;

			switch (var2 / 2) {
			case 0:
				result = sub16675(index, var1);
				break;
			case 1:
				result = sub166DD(index, var1);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				result = 0;
				break;
			case 10:
				result = sub1675D(index, var1);
				break;
			case 11:
				warning("result = sub_16729");
				break;
			case 12:
				result = sub16799(var1, index);
				break;
			case 13:
				warning("result = sub_16722");
				break;
			case 14:
				warning("result = sub_166F7");
				break;
			case 15:
				warning("result = sub_166EA");
				break;
			default:
				warning("sub16626 - unexpected value %d", var2 / 2);
				break;
			}

			if ((result & 1) == 0) {
				++_scriptHandler->_array12811[index];
				if (_scriptHandler->_array12811[index] == 16)
					_scriptHandler->_characterScriptEnabled[index] = 1;
			}
		}
		--index;
	}
}

int LilliputEngine::sub166DD(int index, int var1) {

	_characterDirectionArray[index] = (var1 >> 8) & 3;
	sub16685(index, var1 & 0xFF);
	return 0;
}

int LilliputEngine::sub1675D(int index, int var1) {
	debugC(2, kDebugEngine, "sub1675D(%d, %d)", index, var1);

	int var2 = _scriptHandler->_array10A39[index];
	int var1h = _scriptHandler->_array16123[var2];
	int var1l = _scriptHandler->_array1614B[var2];
	int var3 = _array109E9[index];

	if ((var3 != 0xFF) && (var3 == _array10999[index])) {
		var3 = _array10A11[index];
		if (var3 == _array109C1[index]) {
			_array109E9[index] = var1h;
			_array10A11[index] = var1l;
		}
	}

	_array10999[index] = var1h;
	_array109C1[index] = var1l;

	return sub16799(index, var1);
}

void LilliputEngine::sub16EBC() {
	debugC(2, kDebugEngine, "sub16EBC()");

	int index2 = 3;

	for (int index1 = _numCharacters - 1; index1 >= 0; index1--) {
		int var2 = (_scriptHandler->_array1614B[index1] << 8) + (_scriptHandler->_array16123[index1] << 2);
		int var1 = (_bufferIsoMap[index2 + var2] & 0x40);

		if (var1 == _array16E94[index1])
			continue;

		_array16E94[index1] = var1;
		if (var1 != 0)
			_scriptHandler->_characterScriptEnabled[index1] = 1;
	}
}

void LilliputEngine::sub12F37() {
	debugC(2, kDebugEngine, "sub12F37()");

	int index1 = _byte12A04 + 2;
	int index2 = 0;

	for (int i = 0; i < _numCharacters; i++) {
		if (_rulesBuffer2_15[index1] != 0 ) {
			if (_rulesBuffer2_15[index1] == 1) {
				_rulesBuffer2_15[index1] = 0;
			} else {
				--_rulesBuffer2_15[index1];
				if (_rulesBuffer2_15[index1] == 1)
					_scriptHandler->_characterScriptEnabled[index2] = 1;
			}
		}

		index1 += 32;
		++index2;
	}
}

void LilliputEngine::sub130EE() {
	debugC(2, kDebugEngine, "sub130EE()");

	warning("sub147D7");
	warning("sub13156");

	if (_mouseButton == 0)
		// TODO: check _mouse_byte1299F
		return;

	int button = _mouseButton;
	_mouseButton = 0;

	if (button & 2) {
		if (_byte12FE4 != 0xFF)
			sub1305C(_byte12FE4, button);
		return;
	}

	bool forceReturnFl = false;
	sub13184(forceReturnFl);
	if (forceReturnFl)
		return;

	int posX = _mouseX - 64;
	int posY = _mouseY - 16;

	if ((posX < 0) || (posX > 255))
		return;

	if ((posY < 0) || (posY > 176))
		return;

	forceReturnFl = false;
	sub131B2(posX, posY, forceReturnFl);
	if (forceReturnFl)
		return;

	sub131FC(posX, posY);
}

void LilliputEngine::sub131FC(int var2, int var4) {
	debugC(2, kDebugEngine, "sub131FC(%d, %d)", var2, var4);

	int x = var2 - 8;
	int y = var4 - 4;

	x = (x >> 4) - 7;
	y = (y >> 3) - 4;

	int diff = (y - x) >> 1;
	y = y - diff;

	if ((y >= 0) && (diff >= 0) && (y < 8) && (diff < 8)) {
		y += _scriptHandler->_viewportX;
		diff += _scriptHandler->_viewportY;
		_savedMousePosDivided = (y << 8) + diff;
		_byte16F07_menuId = 5;
	}
}

void LilliputEngine::sub131B2(int var2, int var4, bool &forceReturnFl) {
	debugC(2, kDebugEngine, "sub131B2(%d, %d)", var2, var4);

	forceReturnFl = false;

	for (int i = 0; i < _numCharacters; i++) {
		if ((var2 >= _characterDisplayX[i]) && (var2 <= _characterDisplayX[i] + 17) && (var4 >= _characterDisplayY[i]) && (var4 <= _characterDisplayY[i] + 17) && (i != _word10804)) {
			_byte129A0 = i;
			_byte16F07_menuId = 4;
			if (_byte12FCE == 1)
				_byte16F07_menuId = 3;

			return;
		}
	}
	forceReturnFl = true;
	return;
}

void LilliputEngine::sub13184(bool &forceReturnFl) {
	debugC(2, kDebugEngine, "sub13184()");

	forceReturnFl = false;
	for (int index = _word12F68_ERULES - 1; index >= 0; index--) {
		if (sub13240(_mouseX, _mouseY, _rulesBuffer13_2[index], _rulesBuffer13_3[index]) == 0) {
			sub1305C(index, 1);
			forceReturnFl = true;
			return;
		}
	}
}

int LilliputEngine::sub13240(int posX, int posY, int var3, int var4) {
	debugC(2, kDebugEngine, "sub13240(%d, %d, %d, %d)", posX, posY, var3, var4);

	if ((posX < var3) || (posY < var4))
		return -1;

	var3 += 16;
	var4 += 16;

	if ((posX > var3) || (posY > var4))
		return -1;

	return 0;
}

void LilliputEngine::sub1305C(byte index, byte button) {
	debugC(2, kDebugEngine, "sub1305C(%d, %d)", index, button);

	if (_scriptHandler->_array122E9[index] < 2)
		return;

	_byte12FE4 = index;
	_byte12FE3 = button;

	if (button == 2) {
		if (_byte12FCE != 1) {
			_scriptHandler->_array122E9[index] = 2;
			_byte16F07_menuId = 2;
			displayFunction8();
		}
		return;
	}

	if (_byte12FCE == 1) {
		sub130DD();
		return;
	}

	_scriptHandler->sub130B6();
	_scriptHandler->_array122E9[index] = 3;
	if (_rulesBuffer13_1[index] == 1) {
		_byte12FCE = 1;
		_word15AC2 = 1;
	} else {
		_byte16F07_menuId = 1;
	}

	displayFunction8();
}

void LilliputEngine::sub16685(int idx, int var1) {
	debugC(2, kDebugEngine, "sub16685(%d, %d)", idx, var1);

	int index = (idx << 5) + (var1 & 0xFF);
	_scriptHandler->_array10AB1[idx] = _rulesBuffer2_16[index];
}

int LilliputEngine::sub16675(int idx, int var1) {
	debugC(2, kDebugEngine, "sub16675(%d, %d)", idx, var1);

	sub16685(idx, var1);
	int index = (var1 & 0xFF);	
	switch (var1 >> 8) {
	case 0:
		break;
	case 1:
		sub166B1(index);
		break;
	case 2:
		sub166B6(index);
		break;
	case 3:
		sub166BB(index);
		break;
	case 4:
		sub16B63(index);
		break;
	case 5:
		sub16B76(index);
		break;
	case 6:
		sub166C0(index);
		break;
	case 7:
		sub166C6(index);
		break;
	case 8:
		sub166CC(index);
		break;
	case 9:
		sub166D2(index);
		break;
	case 10:
		sub166D8(index);
		break;
	default:
		warning("sub16675 - Unexpected value %d", var1 >> 8);
	}

	return 0;
}

void LilliputEngine::sub16B63(int index) {
	debugC(2, kDebugEngine, "sub16B63(%d)", index);

	static const byte nextFrame[4] = {1, 3, 0, 2};
	_characterDirectionArray[index] = nextFrame[_characterDirectionArray[index]];
}

void LilliputEngine::sub16B76(int index) {
	debugC(2, kDebugEngine, "sub16B76(%d)", index);

	static const byte nextFrame[4] = {2, 0, 3, 1};
	_characterDirectionArray[index] = nextFrame[_characterDirectionArray[index]];
}

void LilliputEngine::sub166C0(int index) {
	debugC(2, kDebugEngine, "sub166C0(%d)", index);

	_rulesBuffer2_3[index] += 1;
}

void LilliputEngine::sub166C6(int index) {
	debugC(2, kDebugEngine, "sub166C6(%d)", index);

	_rulesBuffer2_3[index] += 2;
}

void LilliputEngine::sub166CC(int index) {
	debugC(2, kDebugEngine, "sub166CC(%d)", index);

	_rulesBuffer2_3[index] -= 1;
}

void LilliputEngine::sub166D2(int index) {
	debugC(2, kDebugEngine, "sub166D2(%d)", index);

	_rulesBuffer2_3[index] -= 2;
}

void LilliputEngine::sub166B1(int index) {
	debugC(2, kDebugEngine, "sub166B1(%d)", index);

	sub16B31(index, 2);
}

void LilliputEngine::sub166B6(int index) {
	debugC(2, kDebugEngine, "sub166B6(%d)", index);

	sub16B31(index, 4);
}

void LilliputEngine::sub166BB(int index) {
	debugC(2, kDebugEngine, "sub166BB(%d)", index);

	sub16B31(index, 0xFE);
}

void LilliputEngine::sub166D8(int index) {
	debugC(2, kDebugEngine, "sub166D8(%d)", index);

	sub16B31(index, 3);
}

void LilliputEngine::sub16B31(int index, int val) {
	debugC(2, kDebugEngine, "sub16B31(%d, %d)", index, val);

	int newX = _characterPositionX[index];
	int newY = _characterPositionY[index];
	switch (_characterDirectionArray[index]) {
	case 0:
		newX += val;
		break;
	case 1:
		newY -= val;
		break;
	case 2:
		newY += val;
		break;
	default:
		newX -= val;
		break;
	}
	sub16B8F(index, newX, newY, _characterDirectionArray[index]);
}

void LilliputEngine::sub16B8F(int index, int x, int y, int flag) {
	debugC(2, kDebugEngine, "sub16B8F(%d, %d, %d)", index, x, y);

	int diffX = x >> 3;
	if (((diffX & 0xFF) == _scriptHandler->_array16123[index]) && ((y >> 3) == _scriptHandler->_array1614B[index])) {
		_characterPositionX[index] = x;
		_characterPositionY[index] = y;
	}

	if ((x < 0) || (x >= 512) || (y < 0) || (y >= 512))
		return;

	int mapIndex = (_scriptHandler->_array1614B[index] << 6) + _scriptHandler->_array16123[index];
	mapIndex <<= 2;

	if ((_bufferIsoMap[mapIndex + 3] & _array16C58[flag]) == 0)
		return;

	mapIndex = ((y & 0xFFF8) << 3) + diffX;
	mapIndex <<= 2;

	if ((_bufferIsoMap[mapIndex + 3] & _array16C54[flag]) == 0)
		return;

	int var1 = _rulesBuffer2_10[index];
	var1 &= 7;
	var1 ^= 7;

	if ((var1 & _rulesChunk9[_bufferIsoMap[mapIndex]]) != 0)
		return;

	_characterPositionX[index] = x;
	_characterPositionY[index] = y;
}

void LilliputEngine::sub17224(int var1, int var4) {
	debugC(2, kDebugEngine, "sub17224(%d, %d)", var1, var4);

	byte type = (var1 >> 8);
	if (type == 0) {
		sub17264(var1, var4);
		return;
	}
	
	if (type == 3) {
		for (int i = _numCharacters - 1; i >= 0; i--)
			sub17264(i, var4);
		return;
	}

	int index = var4 & 0xFF;
	for (int i = 0; i < _numCharacters; i++) {
		if (_scriptHandler->_array10B51[index] >= type)
			sub17264(i, var4);
		index += 40;
	}
}

void LilliputEngine::sub17264(int index, int var4) {
	debugC(2, kDebugEngine, "sub17264(%d, %d)", index, var4);

	if (_array11D49[index] != 0xFFFF) {
		_array1289F[index] = var4;
	} else {
		_scriptHandler->_characterScriptEnabled[index] = 1;
		_array11D49[index] = var4;
	}
}

void LilliputEngine::sub171CF() {
	debugC(2, kDebugEngine, "sub171CF()");

	for (int i = 0; i < _numCharacters; i++) {
		if (_array1289F[i] != 0xFFFF) {
			_array11D49[i] = _array1289F[i];
			_array1289F[i] = 0xFFFF;
			_scriptHandler->_characterScriptEnabled[i] = 1;
		}
	}

	++_word1289D;

	for (int i = 0; i < 10; i++) {
		if ((_array12861[(3 * i) + 1] != 0xFFFF) && (_array12861[3 * i] == _word1289D)) {
			int var1 = _array12861[(3 * i) + 1];
			int var4 = _array12861[(3 * i) + 2];
			_array12861[(3 * i) + 1] = 0xFFFF;

			sub17224(var1, var4);
		}
	}
}

void LilliputEngine::sub12FE5() {
	debugC(2, kDebugEngine, "sub12FE5()");

	if (_byte12A04 != 1)
		return;

	int index = 0;
	int count = 0;
	for (int i = 0; i < _word12F68_ERULES; i++) {
		if (_scriptHandler->_array122FD[index] != 0) {
			--_scriptHandler->_array122FD[index];
			if (_scriptHandler->_array122FD[index] == 0) {
				_scriptHandler->_array122E9[index] = 2;
				++count;
			}
		}
	}

	if (count !=0)
		displayFunction8();
}

void LilliputEngine::displayHeroismIndicator() {
	debugC(2, kDebugEngine, "displayHeroismIndicator()");

	if (_scriptHandler->_savedBuffer215Ptr == NULL)
		return;

	int var1 = (_scriptHandler->_savedBuffer215Ptr[0] * 25) >> 8;

	if (var1 == _scriptHandler->_heroismLevel)
		return;

	displayFunction5();
	int var2 = 1;
	if (var1 > _scriptHandler->_heroismLevel)
		var1 = 150;
	else {
		var2 = -1;
		var1 = 40;
	}

	_scriptHandler->_heroismLevel += var2;

	int index = _scriptHandler->_heroismBarX + _scriptHandler->_heroismBarBottomY * 320 ;

	var2 = _scriptHandler->_heroismLevel & 0xFF;
	if (var2 != 0) {
//		sub16064(var1, _scriptHandler->_byte15FFA);
		for (int i = 0; i < (var2 << 2); i++) {
			((byte *)_mainSurface->getPixels())[index] = var1;
			((byte *)_mainSurface->getPixels())[index + 1] = var1;
			((byte *)_mainSurface->getPixels())[index + 2] = var1;
			index -= 320;
		}
	}

	if (25 - _scriptHandler->_heroismLevel != 0) {
//		sub16064(23, 25 - _scriptHandler->_byte15FFA);
		var2 = (25 - _scriptHandler->_heroismLevel) << 2;
		for (int i = 0; i < var2; i++) {
			((byte *)_mainSurface->getPixels())[index] = 23;
			((byte *)_mainSurface->getPixels())[index + 1] = 23;
			((byte *)_mainSurface->getPixels())[index + 2] = 23;
			index -= 320;
		}
	}

	displayFunction4();
}

void LilliputEngine::pollEvent() {
	debugC(2, kDebugEngine, "pollEvent()");

	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouseX = CLIP<int>(event.mouse.x, 0, 304) + 5;
			_mouseY = CLIP<int>(event.mouse.y, 0, 184) + 1;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseButton |= 1;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButton |= 2;
			break;
		case Common::EVENT_QUIT:
			_shouldQuit = true;
			break;
		// TODO: handle keyboard
		default:
			break;
		}
	}

	if ((_mouseX != _oldMouseX) || (_mouseY != _oldMouseY)) {
		_oldMouseX = _mouseX;
		_oldMouseY = _mouseY;
		if (_skipDisplayFlag1 != 0) {
			displayFunction5();
			_mouseDisplayX = _mouseX;
			_mouseDisplayY = _mouseY;
			displayFunction4();
		} else {
			_mouseDisplayX = _mouseX;
			_mouseDisplayY = _mouseY;
		}
	}
}

byte *LilliputEngine::loadVGA(Common::String filename, bool loadPal) {
	debugC(1, kDebugEngine, "loadVGA(%s, %d)", filename.c_str(), (loadPal) ? 1 : 0);

	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.c_str());

	int remainingSize = f.size();
	if (loadPal) {
		for (int i = 0; i < 768; ++i)
			_curPalette[i] = f.readByte();
		remainingSize -= 768;

		fixPaletteEntries(_curPalette, 256);
	}

	uint8 curByte;
	byte decodeBuffer[100000];
	int size = 0;

	for (;remainingSize > 0;) {
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
			}
		} else {
			// Not compressed
			int rawSize = (curByte & 0xFF);
			for (int i = 0; i < rawSize; ++i) {
				decodeBuffer[size] = f.readByte();
				--remainingSize;
				++size;
			}
		}
	}

	f.close();

	byte *res = (byte *)malloc(sizeof(byte) * size);
	memcpy(res, decodeBuffer, size);
	return res;
}

byte *LilliputEngine::loadRaw(Common::String filename) {
	debugC(1, kDebugEngine, "loadRaw(%s)", filename.c_str());

	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.c_str());

	int size = f.size();
	byte *res = (byte *)malloc(sizeof(byte) * size);
	for (int i = 0; i < size; ++i)
		res[i] = f.readByte();

	f.close();
	return res;
}

void LilliputEngine::loadRules() {
	debugC(1, kDebugEngine, "loadRules()");

	static const byte _rulesXlatArray[26] = {30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44};
	Common::File f;
	uint16 curWord;

	if (!f.open("ERULES.PRG"))
		error("Missing game file ERULES.PRG");

	_word10800_ERULES = f.readUint16LE();

	// Chunk 1
	int size = f.readUint16LE();
	_rulesChunk1 = (byte *)malloc(sizeof(byte) * size);
	for (int i = 0; i < size; ++i)
		_rulesChunk1[i] = f.readByte();

	// Chunk 2
	_numCharacters = f.readSint16LE();
	assert(_numCharacters <= 40);

	for (int i = _numCharacters, j = 0; i != 0; i--, j++) {
		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_characterPositionX[j] = curWord;

		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_characterPositionY[j] = curWord;

		_rulesBuffer2_3[j] = (f.readUint16LE() & 0xFF);
		_characterFrameArray[j] = f.readUint16LE();
		_rulesBuffer2_5[j] = f.readByte();
		_rulesBuffer2_6[j] = f.readByte();
		_rulesBuffer2_7[j] = f.readByte();
		_spriteSizeArray[j] = f.readByte();
		_characterDirectionArray[j] = f.readByte();
		_rulesBuffer2_10[j] = f.readByte();
		_rulesBuffer2_11[j] = f.readByte();
		_rulesBuffer2_12[j] = f.readByte();
		_rulesBuffer2_13[j] = f.readByte();
		_rulesBuffer2_14[j] = f.readByte();

		for (int k = 0; k < 32; k++)
			_rulesBuffer2_15[(j * 32) + k] = f.readByte();

		for (int k = 0; k < 32; k++)
			_rulesBuffer2_16[(j * 32) + k] = f.readByte();
	}

	// Chunk 3 & 4
	_rulesChunk3_size = f.readSint16LE();
	curWord = f.readSint16LE();

	_rulesChunk3 = (int *)malloc(sizeof(int) * _rulesChunk3_size);
	for (int i = 0; i < _rulesChunk3_size; ++i)
		_rulesChunk3[i] = f.readUint16LE();

	_rulesChunk4 = (byte *)malloc(sizeof(byte) * curWord);
	for (int i = 0; i < curWord; ++i)
		_rulesChunk4[i] = f.readByte();

	// Chunk 5: Scripts
	// Use byte instead of int, therefore multiply by two the size.
	// This is for changing that into a memory read stream
	_initScript_size = f.readUint16LE() * 2;
	_initScript = (byte *)malloc(sizeof(byte) * _initScript_size);
	for (int i = 0; i < _initScript_size; ++i)
		_initScript[i] = f.readByte();

	// Chunk 6
	_menuScript_size = f.readUint16LE() * 2;
	_menuScript = (byte *)malloc(sizeof(byte) * _menuScript_size);
	for (int i = 0; i < _menuScript_size; ++i)
		_menuScript[i] = f.readByte();

	// Chunk 7 & 8
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

	// Chunk 9
	for (int i = 0; i < 60; i++)
		_rulesChunk9[i] = f.readByte();

	// Chunk 10 & 11
	_rulesChunk10_size = f.readByte();
	assert(_rulesChunk10_size <= 20);

	if (_rulesChunk10_size != 0) {
		_rulesChunk10 = (int *)malloc(sizeof(int) * _rulesChunk10_size);
		int totalSize = 0;
		for (int i = 0; i < _rulesChunk10_size; ++i) {
			_rulesChunk10[i] = totalSize;
			totalSize += f.readByte();
		}
		if (totalSize != 0) {
			_rulesChunk11 = (byte *)malloc(sizeof(byte) * totalSize);
			for (int i = 0; i < totalSize; i++)
				_rulesChunk11[i] = f.readByte();
		}
	}

	// Chunk 12
	_rulesChunk12_size = f.readUint16LE();
	assert(_rulesChunk12_size <= 40);

	for (int i = 0; i < _rulesChunk12_size; i++) {
		_rulesBuffer12_1[i] = f.readUint16LE();
		_rulesBuffer12_2[i] = f.readUint16LE();
		_rulesBuffer12_3[i] = f.readUint16LE();
		_rulesBuffer12_4[i] = f.readUint16LE();
	}

	// Chunk 13
	_word12F68_ERULES = f.readUint16LE();
	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_1[i] = f.readByte();

	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_2[i] = f.readUint16LE();

	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_3[i] = f.readUint16LE();

	for (int i = 0; i < 20; i++) {
		byte curByte = f.readByte();

		if (curByte == 0x20)
			_rulesBuffer13_4[i] = 0x39;
		else if (curByte == 0xD)
			_rulesBuffer13_4[i] = 0x1C;
		// Hack to avoid xlat out of bounds
		else if (curByte == 0xFF)
			_rulesBuffer13_4[i] = 0x21;
		// Hack to avoid xlat out of bounds
		else if (curByte == 0x00)
			_rulesBuffer13_4[i] = 0xB4;
		else {
			assert((curByte > 0x40) && (curByte <= 0x41 + 26));
			_rulesBuffer13_4[i] = _rulesXlatArray[curByte - 0x41];
		}
	}
	f.close();

	// Skipped: Load Savegame
}

void LilliputEngine::displayVGAFile(Common::String fileName) {
	debugC(1, kDebugEngine, "displayVGAFile(%s)", fileName.c_str());

	displayFunction4();

	byte *buffer = loadVGA(fileName, true);
	memcpy(_mainSurface->getPixels(), buffer, 320*200);
	_system->copyRectToScreen((byte *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();

	displayFunction5();
}

void LilliputEngine::fixPaletteEntries(uint8 *palette, int num) {
	debugC(1, kDebugEngine, "fixPaletteEntries(palette, %d)", num);
	// Color values are coded on 6bits ( for old 6bits DAC )
	for (int32 i = 0; i < num * 3; i++) {
		int32 a = palette[i];
		assert(a < 64);

		a =  (a << 2) | (a >> 4);
		if (a > 255)
			a = 255;
		palette[i] = a;
	}
}

void LilliputEngine::initPalette() {
	debugC(1, kDebugEngine, "initPalette()");

	for (int i = 0; i < 768; i++)
		_curPalette[i] = _basisPalette[i];

	fixPaletteEntries(_curPalette, 256);
	_system->getPaletteManager()->setPalette(_curPalette, 0, 256);
}

void LilliputEngine::sub170EE(int index) {
	debugC(1, kDebugEngine, "sub170EE(%d)", index);

	_rulesBuffer2PrevIndx = index;

	assert (index < 40);
	int var2 = _characterPositionX[index];
	int var4 = _characterPositionY[index];

	_currentScriptCharacterPosition = (((var2 >> 3) & 0xFF) << 8) + ((var4 >> 3) & 0xFF);
	_ptr_rulesBuffer2_15 = &_rulesBuffer2_15[_rulesBuffer2PrevIndx * 32];
}

void LilliputEngine::sub130DD() {
	warning("sub130DD()");
}

void LilliputEngine::handleMenu() {
	debugC(1, kDebugEngine, "handleMenu()");

	if (_byte16F07_menuId == 0)
		return;

	if ((_byte12FCE == 1) && (_byte16F07_menuId != 3))
		return;

	sub170EE(_word10804);
	_scriptHandler->runMenuScript(ScriptStream(_menuScript, _menuScript_size));
	_savedMousePosDivided = 0xFFFF;
	_byte129A0 = 0xFF;

	if (_byte16F07_menuId == 3)
		sub130DD();

	_byte16F07_menuId = 0;
}

void LilliputEngine::handleGameScripts() {
	debugC(1, kDebugEngine, "handleGameScripts()");

	int index = _word17081_nextIndex;
	int i;
	for (i = 0; (_scriptHandler->_characterScriptEnabled[index] == 0) && (i < _numCharacters); i++) {
		++index;
		if (index >= _numCharacters)
			index = 0;
	}

	if (i > _numCharacters)
		return;

	_word17081_nextIndex = (index + 1) % _numCharacters;


	_scriptHandler->_characterScriptEnabled[index] = 0;
	sub170EE(index);


	_word16EFE = _array11D49[index];
	_array11D49[index] = 0xFFFF;
	_word1817B = 0;

	int tmpVal = _rulesBuffer2_12[index];
	if (tmpVal == 0xFF)
		return;

	for (int i = 0; i < _gameScriptIndexSize; i++) {
		assert(tmpVal < _gameScriptIndexSize);
		debugC(1, kDebugEngine, "================= Game Script %d ==================", i);
		ScriptStream script = ScriptStream(&_arrayGameScripts[_arrayGameScriptIndex[i]], _arrayGameScriptIndex[i + 1] - _arrayGameScriptIndex[i]);
//		_scriptHandler->disasmScript(script);
		debugC(1, kDebugEngine, "============= End Game Script %d ==================", i);

	}

	assert(tmpVal < _gameScriptIndexSize);
	debugC(1, kDebugEngine, "================= Game Script %d for character %d ==================", tmpVal, index);
	_scriptHandler->runScript(ScriptStream(&_arrayGameScripts[_arrayGameScriptIndex[tmpVal]], _arrayGameScriptIndex[tmpVal + 1] - _arrayGameScriptIndex[tmpVal]));
	debugC(1, kDebugEngine, "============= End Game Script %d for character %d ==================", tmpVal, index);

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

	initPalette();

	// Load files. In the original, the size was hardcoded
	_bufferIdeogram = loadVGA("IDEOGRAM.VGA", false);
	_bufferMen = loadVGA("MEN.VGA", false);
	_bufferMen2 = loadVGA("MEN2.VGA", false);
	_bufferIsoChars = loadVGA("ISOCHARS.VGA", false);
	_bufferIsoMap = loadRaw("ISOMAP.DTA");

	loadRules();

	_lastTime = _system->getMillis();



	//TODO: Init sound/music player
	_scriptHandler->runScript(ScriptStream(_initScript, _initScript_size));

	_int8installed = true;

	while(!_shouldQuit) {
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
		_arr18560[i]._field0 = 0;
		_arr18560[i]._field1 = 0;
		_arr18560[i]._field3 = 0;
		for (int j = 0; j < 8; j ++)
			_arr18560[i]._field5[j] = 0;
	}
}

void LilliputEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String LilliputEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}

byte LilliputEngine::_keyboard_getch() {
	warning("getch()");
	return ' ';
}

} // End of namespace Lilliput
