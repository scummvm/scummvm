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

	_console = new LilliputConsole(this);
	_rnd = 0;
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

	_scriptHandler = new LilliputScript(this);

	_byte1714E = 0;
	_byte12FCE = 0;
	_byte129A0 = 0xFF;
	_byte160FA = 0;
	_byte16529 = 0;
	_byte1652A = 0;

	_rulesBuffer2PrevIndx = 0;
	_word16EFA = 0;
	_word10804 = 0;
	_word17081_nextIndex = 0;
	_word16EFE = 0xFFFF;
	_word1817B = 0;
	_word15BC8 = 0;
	_word15BCA = 0;
	_word15AC2 = 0;
	_word16213 = 0;
	_word16215 = 0;

	_saveFlag = false;
	_byte16F07_menuId = 0;

	for (int i = 0; i < 40; i++) {
		_array10999[i] = 0;
		_array109C1[i] = 0;
		_array160FB[i] = 0;
		_array16173[i] = 0xFF;
		_array1619B[i] = 0xFF;
		_array161C3[i] = 0;
		_array161EB[i] = 0;

		_array11D49[i] = 0xFFFF;
		_rulesBuffer2_1[i] = 0xFFFF;
		_rulesBuffer2_2[i] = 0xFFFF;
		_rulesBuffer2_3[i] = 0;
		_rulesBuffer2_4[i] = 0;
		_rulesBuffer2_5[i] = 0xFF;
		_rulesBuffer2_6[i] = 4;
		_rulesBuffer2_7[i] = 0;
		_rulesBuffer2_8[i] = 20;
		_rulesBuffer2_9[i] = 0;
		_rulesBuffer2_10[i] = 0;
		_rulesBuffer2_11[i] = 0;
		_rulesBuffer2_12[i] = 0;
		_rulesBuffer2_13[i] = 0;
		_rulesBuffer2_14[i] = 0;
	}

	for (int i = 0; i < 256; i++)
		_array15AC8[i] = 0;

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

// display mouse cursor, if any
void LilliputEngine::displayFunction1(byte *buf, int var1, int var2, int var4) {
	debugC(2, kDebugEngine, "displayFunction1(buf, %d, %d, %d)", var1, var2, var4);

	int index1 = ((var1 & 0xFF) << 8) + (var1 >> 8);
	byte *newBuf = &buf[index1];

	int tmpVal = ((var4 & 0xFF) << 8) + (var4 >> 8);
	int index2 = var2 + tmpVal + (tmpVal >> 2);

	for (int i = 0; i < 16; i++) {
		if (newBuf[0] != 0)
			((byte *)_mainSurface->getPixels())[index2] = newBuf[0];
		if (newBuf[1] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 1] = newBuf[1];
		if (newBuf[2] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 2] = newBuf[2];
		if (newBuf[3] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 3] = newBuf[3];
		if (newBuf[4] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 4] = newBuf[4];
		if (newBuf[5] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 5] = newBuf[5];
		if (newBuf[6] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 6] = newBuf[6];
		if (newBuf[7] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 7] = newBuf[7];
		if (newBuf[8] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 8] = newBuf[8];
		if (newBuf[9] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 9] = newBuf[9];
		if (newBuf[10] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 10] = newBuf[10];
		if (newBuf[11] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 11] = newBuf[11];
		if (newBuf[12] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 12] = newBuf[12];
		if (newBuf[13] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 13] = newBuf[13];
		if (newBuf[14] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 14] = newBuf[14];
		if (newBuf[15] != 0)
			((byte *)_mainSurface->getPixels())[index2 + 15] = newBuf[15];

		index2 += 320;
		newBuf = &newBuf[16];
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

	if (_scriptHandler->_byte16F08 == 1)
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

	int var1 = (_scriptHandler->_word12A02 >> 8) + ((_scriptHandler->_word12A02 & 0xFF) << 8) + (_scriptHandler->_word12A00 << 2);
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
		for (int j = 0; j < 16; j++) {
			((byte *)_mainSurface->getPixels())[tmpIndex + j] = 17;
		}
		index += 320;
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

	if (_scriptHandler->_byte16F08 == 1)
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
};

void LilliputEngine::sub16217() {
	debugC(2, kDebugEngine, "sub16217()");

	_byte160FA = 0;
	int index = _word10807_ERULES - 1;
	_word16213 = _scriptHandler->_word12A00 << 3;
	_word16215 = _scriptHandler->_word12A02 << 3;

	for (int i = index; i >= 0; i--) {
		if (_rulesBuffer2_5[i] != 0xFF) {
			int index2 = _rulesBuffer2_5[i];
			_rulesBuffer2_3[i] = _rulesBuffer2_3[index2] + _rulesBuffer2_7[i];
			int tmpVal = _rulesBuffer2_6[i];
			_rulesBuffer2_9[i] = _rulesBuffer2_9[index2];
			int var3 = _rulesBuffer2_1[index2];
			int var4 = _rulesBuffer2_2[index2];

			switch (_rulesBuffer2_9[i]) {
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

			_rulesBuffer2_1[i] = var3;
			_rulesBuffer2_2[i] = var4;
		}

		_scriptHandler->_array16123[i] = (_rulesBuffer2_1[i] & 0xFF);
		_scriptHandler->_array1614B[i] = (_rulesBuffer2_2[i] & 0xFF);
		_scriptHandler->_array16173[i] = 0xFF;
		_array1619B[i] = 0xFF;
		_array161C3[i] = 0xFF;
		_array161EB[i] = 0xFF;

		int tmpVal2 = _rulesBuffer2_1[i] - _scriptHandler->_word12A00;
		int tmpVal3 = _rulesBuffer2_2[i] - _scriptHandler->_word12A02;
		if ((tmpVal2 >= 0) && (tmpVal2 <= 7) && (tmpVal3 >= 0) && (tmpVal3 <= 7)) {
			_array16173[i] = tmpVal2;
			_array1619B[i] = tmpVal3;
			tmpVal2 = _rulesBuffer2_1[i] - _word16213;
			tmpVal3 = _rulesBuffer2_2[i] - _word16213;
			int tmpVal4 = _rulesBuffer2_3[i];
			_array161C3[i] = ((60 + tmpVal2 - tmpVal3) * 2) & 0xFF;
			_array161EB[i] = (20 + tmpVal2 + tmpVal3 - tmpVal4) & 0xFF;
			_array160FB[_byte160FA] = i;
			++_byte160FA;
		}
	}

	warning("sub_161323");

}

void LilliputEngine::sub1652B(int var1) {
	debugC(2, kDebugEngine, "sub1652B(%d)", var1);

	if (_byte160FA < (var1 & 0xFF)) {
		int index = _array160FB[var1 & 0xFF];
		_byte16529 = _array16173[index];
		_byte1652A = _array1619B[index];
	} else {
		_byte16529 = 0xFF;
		_byte1652A = 0xFF;
	}
}

void LilliputEngine::displayFunction15() {
	debugC(2, kDebugEngine, "displayFunction15()");

	sub16217();
	_word16550 = 0;
	sub1652B(0);

	memcpy(_buffer1_45k, _buffer2_45k, 45056);

	int index1 = (_scriptHandler->_word12A02 >> 8) + ((_scriptHandler->_word12A02 & 0xFF) << 8) + (_scriptHandler->_word12A00 << 2);
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
			warning("sub_16553");

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
	_word10807_ERULES = f.readSint16LE();
	assert(_word10807_ERULES <= 40);

	for (int i = _word10807_ERULES, j = 0; i != 0; i--, j++) {
		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_rulesBuffer2_1[j] = curWord;

		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_rulesBuffer2_2[j] = curWord;

		_rulesBuffer2_3[j] = (f.readUint16LE() & 0xFF);
		_rulesBuffer2_4[j] = f.readUint16LE();
		_rulesBuffer2_5[j] = f.readByte();
		_rulesBuffer2_6[j] = f.readByte();
		_rulesBuffer2_7[j] = f.readByte();
		_rulesBuffer2_8[j] = f.readByte();
		_rulesBuffer2_9[j] = f.readByte();
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
	int var2 = _rulesBuffer2_1[index];
	int var4 = _rulesBuffer2_2[index];

	_word16EFA = (((var2 >> 3) & 0xFF) << 8) + ((var4 >> 3) & 0xFF);
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
	_scriptHandler->runMenuScript(Common::MemoryReadStream(_menuScript, _menuScript_size));
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
	for (i = 0; (_scriptHandler->_array10B29[index] == 0) && (i < _word10807_ERULES); i++) {
		++index;
		if (index >= _word10807_ERULES)
			index = 0;
	}

	if (i > _word10807_ERULES)
		return;

	_scriptHandler->_array10B29[index] = 0;
	int tmpVal = index + 1;
	if (tmpVal >= _word10807_ERULES)
		tmpVal = 0;

	_word17081_nextIndex = tmpVal;
	sub170EE(index);

	_word16EFE = _array11D49[index];
	_array11D49[index] = 0xFFFF;
	_word1817B = 0;

	tmpVal = _rulesBuffer2_12[index];
	if (tmpVal == 0xFF)
		return;

	assert(tmpVal < _gameScriptIndexSize);
	debugC(1, kDebugEngine, "========================== Game Script %d ==========================", tmpVal);

	_scriptHandler->runScript(Common::MemoryReadStream(&_arrayGameScripts[_arrayGameScriptIndex[tmpVal]], _arrayGameScriptIndex[tmpVal + 1] - _arrayGameScriptIndex[tmpVal]));
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

	//TODO: Init sound/music player
	_scriptHandler->runScript(Common::MemoryReadStream(_initScript, _initScript_size));

	while(!_shouldQuit) {
		handleMenu();
		handleGameScripts();
		// To be removed when handled in the previous fonctions
		pollEvent();
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
