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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lilliput/lilliput.h"
#include "lilliput/script.h"
#include "common/debug.h"

namespace Lilliput {

LilliputScript::LilliputScript(LilliputEngine *vm) : _vm(vm), _currScript(NULL) {
	_byte129A0 = 0xFF;
	_byte16F08 = 0;
	_byte1855D = 0;
	_byte12A04 = 0;
	_byte10806 = 0;
	_byte12FE4 = 0xFF;
	_byte16F02 = 0;

	_word1855E = 0;
	_word16F00 = -1;
	_word10802 = -1;
	_word10804 = 0;
	_word15FFB = 0;
	_word15FFD = 0;
	_word12A00 = 0;
	_word12A02 = 0;
	_word18776 = 0;

	_savedBuffer215Ptr = NULL;

	for (int i = 0; i < 20; i++) {
		_array122E9[i] = 0;
		_array122FD[i] = 0;
	}

	for (int i = 0; i < 40; i++) {
		_array10B29[i] = 1;
		_array128EF[i] = 15;
		_array10AB1[i] = 0;
		_array12811[i] = 16;
		_array12839[i] = 0xFF;
		_array16123[i] = 0;
		_array1614B[i] = 0;
		_array16173[i] = 0xFF;
	}

	for (int i = 0; i < 640; i++) {
		_array12311[i] = 0xFFFF;
	}
}

LilliputScript::~LilliputScript() {
}

byte LilliputScript::handleOpcodeType1(int curWord) {
	debugC(2, kDebugScript, "handleOpcodeType1(0x%x)", curWord);
	switch (curWord) {
	case 0x0:
		return OC_sub173DF();
		break;
	case 0x1:
		return OC_sub173F0();
		break;
	case 0x2:
		return OC_sub1740A();
		break;
	case 0x3:
		return OC_sub17434();
		break;
	case 0x4:
		return OC_sub17468();
		break;
	case 0x5:
		return OC_getRandom();
		break;
	case 0x6:
		return OC_sub1748C();
		break;
	case 0x7:
		return OC_compWord18776();
		break;
	case 0x8:
		return OC_checkSaveFlag();
		break;
	case 0x9:
		return OC_sub174C8();
		break;
	case 0xA:
		return OC_sub174D8();
		break;
	case 0xB:
		return OC_sub1750E();
		break;
	case 0xC:
		return OC_compareCoords_1();
		break;
	case 0xD:
		return OC_compareCoords_2();
		break;
	case 0xE:
		return OC_sub1757C();
		break;
	case 0xF:
		return OC_sub1759E();
		break;
	case 0x10:
		return OC_compWord16EF8();
		break;
	case 0x11:
		return OC_sub175C8();
		break;
	case 0x12:
		return OC_sub17640();
		break;
	case 0x13:
		return OC_sub176C4();
		break;
	case 0x14:
		return OC_compWord10804();
		break;
	case 0x15:
		return OC_sub17766();
		break;
	case 0x16:
		return OC_sub17782();
		break;
	case 0x17:
		return OC_sub1779E();
		break;
	case 0x18:
		return OC_sub177C6();
		break;
	case 0x19:
		return OC_compWord16EFE();
		break;
	case 0x1A:
		return OC_sub177F5();
		break;
	case 0x1B:
		return OC_sub17812();
		break;
	case 0x1C:
		return OC_sub17825();
		break;
	case 0x1D:
		return OC_sub17844();
		break;
	case 0x1E:
		return OC_sub1785C();
		break;
	case 0x1F:
		return OC_sub17886();
		break;
	case 0x20:
		return OC_sub178A8();
		break;
	case 0x21:
		return OC_sub178BA();
		break;
	case 0x22:
		return OC_sub178C2();
		break;
	case 0x23:
		return OC_sub178D2();
		break;
	case 0x24:
		return OC_sub178E8();
		break;
	case 0x25:
		return OC_sub178FC();
		break;
	case 0x26:
		return OC_sub1790F();
		break;
	case 0x27:
		return OC_sub1792A();
		break;
	case 0x28:
		return OC_sub1793E();
		break;
	case 0x29:
		return OC_sub1795E();
		break;
	case 0x2A:
		return OC_sub1796E();
		break;
	case 0x2B:
		return OC_sub17984();
		break;
	case 0x2C:
		return OC_checkSavedMousePos();
		break;
	case 0x2D:
		return OC_sub179AE();
		break;
	case 0x2E:
		return OC_sub179C2();
		break;
	case 0x2F:
		return OC_sub179E5();
		break;
	case 0x30:
		return OC_sub17A07();
		break;
	case 0x31:
		return OC_sub17757();
		break;
	default:
		error("Unexpected opcode %d", curWord);
		break;
	}
}

void LilliputScript::handleOpcodeType2(int curWord) {
	debugC(2, kDebugScript, "handleOpcodeType2(0x%x)", curWord);
	switch (curWord) {
	case 0x0:
		OC_setWord18821();
		break;
	case 0x1:
		OC_sub17A3E();
		break;
	case 0x2:
		OC_sub17D57();
		break;
	case 0x3:
		OC_sub17D7F();
		break;
	case 0x4:
		OC_sub17DB9();
		break;
	case 0x5:
		OC_sub17DF9();
		break;
	case 0x6:
		OC_sub17E07();
		break;
	case 0x7:
		OC_sub17E15();
		break;
	case 0x8:
		OC_sub17B03();
		break;
	case 0x9:
		OC_getRandom_type2();
		break;
	case 0xA:
		OC_sub17A66();
		break;
	case 0xB:
		OC_sub17A8D();
		break;
	case 0xC:
		OC_saveAndQuit();
		break;
	case 0xD:
		OC_sub17B93();
		break;
	case 0xE:
		OC_sub17E37();
		break;
	case 0xF:
		OC_resetByte1714E();
		break;
	case 0x10:
		OC_deleteSavegameAndQuit();
		break;
	case 0x11:
		OC_incByte16F04();
		break;
	case 0x12:
		OC_sub17BA5();
		break;
	case 0x13:
		OC_setByte18823();
		break;
	case 0x14:
		OC_sub17BB7();
		break;
	case 0x15:
		OC_sub17BF2();
		break;
	case 0x16:
		OC_sub17ACC();
		break;
	case 0x17:
		OC_resetByte16F04();
		break;
	case 0x18:
		OC_sub17AE1();
		break;
	case 0x19:
		OC_sub17AEE();
		break;
	case 0x1A:
		OC_setWord10804();
		break;
	case 0x1B:
		OC_sub17C0E();
		break;
	case 0x1C:
		OC_sub17C55();
		break;
	case 0x1D:
		OC_sub17C76();
		break;
	case 0x1E:
		OC_sub17AFC();
		break;
	case 0x1F:
		OC_sub17C8B();
		break;
	case 0x20:
		OC_sub17CA2();
		break;
	case 0x21:
		OC_sub17CB9();
		break;
	case 0x22:
		OC_sub17CD1();
		break;
	case 0x23:
		OC_resetWord16EFE();
		break;
	case 0x24:
		OC_sub17CEF();
		break;
	case 0x25:
		OC_sub17D1B();
		break;
	case 0x26:
		OC_sub17D23();
		break;
	case 0x27:
		OC_sub17E6D();
		break;
	case 0x28:
		OC_sub17E7E();
		break;
	case 0x29:
		OC_sub17E99();
		break;
	case 0x2A:
		OC_sub17EC5();
		break;
	case 0x2B:
		OC_sub17EF4();
		break;
	case 0x2C:
		OC_sub17F08();
		break;
	case 0x2D:
		OC_sub17F4F();
		break;
	case 0x2E:
		OC_sub17F68();
		break;
	case 0x2F:
		OC_getNextVal();
		break;
	case 0x30:
		OC_sub17FD2();
		break;
	case 0x31:
		OC_sub17FDD();
		break;
	case 0x32:
		OC_setByte10B29();
		break;
	case 0x33:
		OC_sub18007();
		break;
	case 0x34:
		OC_sub18014();
		break;
	case 0x35:
		OC_sub1801D();
		break;
	case 0x36:
		OC_sub1805D();
		break;
	case 0x37:
		OC_sub18074();
		break;
	case 0x38:
		OC_sub1808B();
		break;
	case 0x39:
		OC_sub18099();
		break;
	case 0x3A:
		OC_sub180C3();
		break;
	case 0x3B:
		OC_sub1810A();
		break;
	case 0x3C:
		OC_sub1812D();
		break;
	case 0x3D:
		OC_sub1817F();
		break;
	case 0x3E:
		OC_sub181BB();
		break;
	case 0x3F:
		OC_sub18213();
		break;
	case 0x40:
		OC_sub18252();
		break;
	case 0x41:
		OC_sub18260();
		break;
	case 0x42:
		OC_sub182EC();
		break;
	case 0x43:
		OC_unkPaletteFunction_1();
		break;
	case 0x44:
		OC_unkPaletteFunction_2();
		break;
	case 0x45:
		OC_loadAndDisplayCUBESx_GFX();
		break;
	case 0x46:
		OC_sub1834C();
		break;
	case 0x47:
		OC_sub18359();
		break;
	case 0x48:
		OC_sub18367();
		break;
	case 0x49:
		OC_sub17D04();
		break;
	case 0x4A:
		OC_sub18387();
		break;
	case 0x4B:
		OC_setByte14835();
		break;
	case 0x4C:
		OC_setByte14837();
		break;
	case 0x4D:
		OC_sub183A2();
		break;
	case 0x4E:
		OC_sub183C6();
		break;
	case 0x4F:
		OC_loadFile_AERIAL_GFX();
		break;
	case 0x50:
		OC_sub17E22();
		break;
	case 0x51:
		OC_sub1844A();
		break;
	case 0x52:
		OC_sub1847F();
		break;
	case 0x53:
		OC_displayVGAFile();
		break;
	case 0x54:
		OC_sub184D7();
		break;
	case 0x55:
		OC_displayTitleScreen();
		break;
	case 0x56:
		OC_sub1853B();
		break;
	case 0x57:
		OC_sub1864D();
		break;
	case 0x58:
		OC_initArr18560();
		break;
	case 0x59:
		OC_sub18678();
		break;
	case 0x5A:
		OC_sub18690();
		break;
	case 0x5B:
		OC_setWord10802();
		break;
	case 0x5C:
		OC_sub186A1();
		break;
	case 0x5D:
		OC_sub186E5_snd();
		break;
	case 0x5E:
		OC_sub1870A_snd();
		break;
	case 0x5F:
		OC_sub18725_snd();
		break;
	case 0x60:
		OC_sub18733_snd();
		break;
	case 0x61:
		OC_sub1873F_snd();
		break;
	case 0x62:
		OC_sub18746_snd();
		break;
	case 0x63:
		OC_sub1875D_snd();
		break;
	case 0x64:
		OC_sub18764();
		break;
	case 0x65:
		OC_sub1853B();
		break;
	default:
		error("Unknown opcode %d", curWord);
		break;
	}
}

int LilliputScript::handleOpcode(Common::MemoryReadStream *script) {
	debugC(2, kDebugScript, "handleOpcode");
	_currScript = script;
	uint16 curWord = _currScript->readUint16LE();
	if (curWord == 0xFFF6)
		return 0xFF;

	for (; curWord != 0xFFF8; curWord = _currScript->readUint16LE()) {
		byte mask = 0; 
		if (curWord > 1000) {
			curWord -= 1000;
			mask = 1;
		}
		byte result = handleOpcodeType1(curWord);
		if ((result ^ mask) == 0) {
			do {
				curWord = _currScript->readUint16LE();
			} while (curWord != 0xFFF7);
			return 0;
		}
	}

	_vm->_byte1714E = 1;

	for (;;) {
		curWord = _currScript->readUint16LE();
		if (curWord == 0xFFF7)
			return _vm->_byte1714E;

		handleOpcodeType2(curWord);
	}
}

void LilliputScript::runScript(Common::MemoryReadStream script) {
	debugC(1, kDebugScript, "runScript");
	_byte16F05_ScriptHandler = 1;
	
	while (handleOpcode(&script) != 0xFF)
		;
}

void LilliputScript::runMenuScript(Common::MemoryReadStream script) {
	debugC(1, kDebugScript, "runMenuScript");
	warning("========================== Menu Script ==============================");
	_byte16F05_ScriptHandler = 0;
	
	while (handleOpcode(&script) == 0)
		;
}

void LilliputScript::sub185ED(byte index, byte subIndex) {
	debugC(2, kDebugScript, "sub185ED");
	if (_vm->_arr18560[index]._field0 != 1)
		return;

	warning("TODO: display function #1");
}

byte LilliputScript::compareValues(byte var1, int oper, int var2) {
	debugC(2, kDebugScript, "compareValues(%d, %c, %d)", var1, oper & 0xFF, var2);
	switch (oper & 0xFF) {
	case '<':
		return (var1 < var2);
	case '>':
		return (var1 > var2);
	default:
		return (var1 == var2);
		break;
	}
}

void LilliputScript::computeOperation(byte *bufPtr, int oper, int var2) {
	debugC(1, kDebugScript, "computeOperation(bufPtr, %c, %d)", oper & 0xFF, var2 & 0xFF);

	switch (oper & 0xFF) {
	case '=':
		bufPtr[0] = var2 & 0xFF;
		break;
	case '+': {
		int tmpVal = bufPtr[0] + var2;
		if (tmpVal > 0xFF)
			bufPtr[0] = 0xFF;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '-': {
		int tmpVal = bufPtr[0] - var2;
		if (tmpVal < 0)
			bufPtr[0] = 0;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '*': {
		int tmpVal = bufPtr[0] * var2;
		bufPtr[0] = tmpVal & 0xFF;
		}
		break;
	case '/': {
		if (var2 != 0)
			bufPtr[0] /= var2;
		}
		break;
	default: {
		warning("computeOperation : oper %d", oper);
		if (var2 != 0) {
			byte tmpVal = bufPtr[0] / var2;
			if (tmpVal < 0)
				bufPtr[0] = 0xFF;
			else 
				bufPtr[0] = 0;
		}
		break;
		}
	}
}

void LilliputScript::sub185B4_display() {
	if (_byte12A04 == _byte1855D)
		return;

	_byte1855D = _byte12A04;
	
	assert(_word1855E < 8);
	int subIndex = _word1855E;
	sub185ED(0, subIndex);
	sub185ED(1, subIndex);
	sub185ED(2, subIndex);
	sub185ED(3, subIndex);

	// In the original, increment by 2 as it's an array of words
	++subIndex;
	if (subIndex == 8)
		subIndex = 0;

	_word1855E = subIndex;
}

void LilliputScript::sub1823E(byte var1, byte var2, byte *curBufPtr) {
	debugC(1, kDebugScript, "sub1823E(%d, %d, curBufPtr)", var1, var2);

	assert ((var1 >= 0) && (var1 < 40));
	_array10B29[var1] = 0;
	curBufPtr[0] = var2;
	curBufPtr[1] = 0;
	curBufPtr[2] = 0;
	curBufPtr[3] = 0;
}

void LilliputScript::sub17B6C(int var1) {
	debugC(1, kDebugScript, "sub17B6C(%d)", var1);

	if (var1 == 0) {
		int curWord = 0;
		while (curWord != 0xFFF6)
			curWord = _currScript->readUint16LE();
		
		_currScript->seek(_currScript->pos() - 4);
		return;
	}

	++var1;
	int curVal = 0;
	int tmpVal;
	while (curVal < var1) {
		tmpVal = _currScript->readUint16LE();
		if (tmpVal == 0xFFF7)
			++curVal;
	}

	_currScript->seek(_currScript->pos() - 2);
}

void LilliputScript::sub16C86(int index, byte *buf) {
	debugC(1, kDebugScript, "sub16C86()");

	_array12811[index] = 0;

	for (int i = 0; i < 16; i++) {
		_array12311[(index * 16) + i] = (buf[2 * i] << 8) + buf[(2 * i) + 1];
	}
}

void LilliputScript::sub16C5C(int index, byte var3) {
	debugC(1, kDebugScript, "sub16C5C(%d, %d)", index, var3);

	assert(index < 40);
	_array12839[index] = var3;

	byte *buf = _vm->_rulesChunk1;
	if (var3 != 0) {
		int count = 0;
		while (count < var3) {
			if ((buf[0] == 0xFF) && (buf[1] == 0xFF))
				++count;
			buf = &buf[2];
		}
	}

	sub16C86(index, buf);
}

int LilliputScript::sub17D40(int var) {
	debugC(1, kDebugScript, "sub17D40(%d)", var);

	if ((_byte16F08 != 1) && (_array16173[_vm->_rulesBuffer2PrevIndx] != 0xFF))
		return var;

	warning("sub17D40() - FIXME: Unexpected POP");
	return var;
}

void LilliputScript::sub18A56(byte *buf) {
	warning("TODO: sub18A56(buf)");
}

void LilliputScript::sub18B3C(int var) {
	debugC(2, kDebugScript, "sub18B3C(%d)", var);

	if (var == 0xFFFF)
		return;

	_word18776 = var;

	int index = _vm->_rulesChunk3[var];
	int count = 0;
	while (_vm->_rulesChunk4[index + count] != 0x5B)
		++count;

	int i = 0;
	if (count != 0) {
		int tmpVal = _vm->_rnd->getRandomNumber(count + 1);
		if (tmpVal != 0) {
			int i = 0;
			for (int j = 0; j < tmpVal; j++) {
				do
					++i;
				while (_vm->_rulesChunk4[index + count + i] != 0x5B);
			}
		}
	}

	sub18A56(&_vm->_rulesChunk4[index + count + i]);
}

int LilliputScript::getValue1() {
	debugC(2, kDebugScript, "getValue1()");

	int curWord = _currScript->readUint16LE();
	if (curWord < 1000)
		return curWord;

	switch (curWord) {
	case 1000:
		return (int)_byte129A0;
	case 1001:
		return _vm->_rulesBuffer2PrevIndx;
	case 1002:
		return _word16F00;
	case 1003:
		return (int)_vm->_rulesBuffer2_15[6];
	case 1004:
		return _word10804;
	default:
		warning("getValue1: Unexpected large value %d", curWord);
		return curWord;
	}
}

int LilliputScript::getValue2() {
	debugC(2, kDebugScript, "getValue2()");

	int curWord = _currScript->readUint16LE();
	int tmpVal = curWord >> 8;
	switch(tmpVal) {
	case 0xFF:
		assert((_vm->_rulesBuffer2PrevIndx >= 0) && (_vm->_rulesBuffer2PrevIndx < 40));
		return ((_vm->_rulesBuffer2_13[_vm->_rulesBuffer2PrevIndx] << 8) + _vm->_rulesBuffer2_14[_vm->_rulesBuffer2PrevIndx]);
	case 0xFE: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return ((_vm->_rulesBuffer2_13[index] << 8) + _vm->_rulesBuffer2_14[index]);
		}
	case 0xFD:
		return _vm->_word16EFA;
	case 0xFC: {
		int index = curWord & 0xFF;
		assert(index < 40);
		byte var1 = _vm->_rulesBuffer2_1[index] >> 3;
		byte var2 = _vm->_rulesBuffer2_2[index] >> 3;

		return (var1 << 8) + var2;
		}
	case 0xFB: {
		int index = _word16F00;
		assert(index < 40);
		byte var1 = _vm->_rulesBuffer2_1[index] >> 3;
		byte var2 = _vm->_rulesBuffer2_2[index] >> 3;

		return (var1 << 8) + var2;
		}
	case 0xFA:
		return ((_vm->_byte10999[_vm->_rulesBuffer2PrevIndx] << 8) + _vm->_byte109C1[_vm->_rulesBuffer2PrevIndx]);
	case 0xF9:
		return ((_vm->_rulesBuffer2_15[4] << 8) + _vm->_rulesBuffer2_15[5]);
	case 0xF8: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return _vm->_rulesBuffer12_3[index];
		}
	case 0xF7: {
		int index = _vm->_rulesBuffer2_15[6];
		assert(index < 40);
		byte var1 = _vm->_rulesBuffer2_1[index] >> 3;
		byte var2 = _vm->_rulesBuffer2_2[index] >> 3;

		return (var1 << 8) + var2;
		}
	case 0xF6:
		return _vm->_savedMousePosDivided;
	default:
		warning("getValue2 - 0x%x - High value %d", tmpVal, curWord);
		return curWord;
	}
}

void LilliputScript::sub130B6() {
	debugC(1, kDebugScript, "sub130B6()");
	assert(_vm->_word12F68_ERULES <= 20);
	for (int i = 0; i < _vm->_word12F68_ERULES; i++) {
		if (_array122E9[i] == 3)
			_array122E9[i] = 2;
	}
}

byte *LilliputScript::getBuffer215Ptr() {
	debugC(2, kDebugScript, "getBuffer215Ptr()");
	int tmpVal = getValue1();
	tmpVal *= 32;
	tmpVal += _currScript->readUint16LE();

	assert(tmpVal < 40 * 32);
	return &_vm->_rulesBuffer2_15[tmpVal];
}

byte LilliputScript::OC_sub173DF() {
	warning("OC_sub173DF");
	return 0;
}
byte LilliputScript::OC_sub173F0() {
	warning("OC_sub173F0");
	return 0;
}
byte LilliputScript::OC_sub1740A() {
	warning("OC_sub1740A");
	return 0;
}

byte LilliputScript::OC_sub17434() {
	debugC(1, kDebugScript, "OC_sub17434()");

	byte *tmpArr = getBuffer215Ptr();
	byte var1 = tmpArr[0];
	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readUint16LE();

	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_sub17468() {
	warning("OC_sub17468");
	return 0;
}

byte LilliputScript::OC_getRandom() {
	debugC(1, kDebugScript, "OC_getRandom()");

	int maxVal = _currScript->readUint16LE();
	int rand = _vm->_rnd->getRandomNumber(maxVal);
	_byte16F02 = (rand & 0xFF);
	
	if (rand == 0)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub1748C() {
	warning("OC_sub1748C");
	return 0;
}
byte LilliputScript::OC_compWord18776() {
	warning("OC_compWord18776");
	return 0;
}
byte LilliputScript::OC_checkSaveFlag() {
	debugC(1, kDebugScript, "OC_checkSaveFlag()");

	if (_vm->_saveFlag)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub174C8() {
	warning("OC_sub174C8");
	return 0;
}

byte LilliputScript::OC_sub174D8() {
	debugC(1, kDebugScript, "OC_sub174D8()");

	byte tmpVal = getValue1() & 0xFF;
	int curWord = _currScript->readUint16LE();
	
	if (curWord == 3000) {
		int index;
		for (index = 0; index < _vm->_word10807_ERULES; index++) {
			if (_vm->_rulesBuffer2_5[index] == tmpVal) {
				_word16F00 = index;
				return 1;
			}
		}
	} else {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		assert(index < 40);
		if (_vm->_rulesBuffer2_5[index] == tmpVal) {
			_word16F00 = index;
			return 1;
		}
	}

	return 0;
}

byte LilliputScript::OC_sub1750E() {
	warning("OC_sub1750E");
	return 0;
}
byte LilliputScript::OC_compareCoords_1() {
	warning("compareCoords_1");
	return 0;
}
byte LilliputScript::OC_compareCoords_2() {
	warning("compareCoords_2");
	return 0;
}
byte LilliputScript::OC_sub1757C() {
	warning("OC_sub1757C");
	return 0;
}
byte LilliputScript::OC_sub1759E() {
	warning("OC_sub1759E");
	return 0;
}

byte LilliputScript::OC_compWord16EF8() {
	debugC(1, kDebugScript, "OC_compWord16EF8()");
	
	int tmpVal = getValue1();
	if (tmpVal == _vm->_rulesBuffer2PrevIndx)
		return 1;
	return 0;
}

byte LilliputScript::OC_sub175C8() {
	debugC(1, kDebugScript, "OC_sub175C8()");
	
	byte var4 = _currScript->readUint16LE() & 0xFF;

	int tmpVal = _currScript->readUint16LE();
	
	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + index];
		if ((var1 & 0xFF) < var4)
			return 0;
		
		_word16F00 = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_word10807_ERULES; i++) {
			int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + i];
			if ((var1 & 0xFF) >= var4) {
				_word16F00 = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_word10807_ERULES; i++) {
		int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + i];
		if ((var1 & 0xFF) >= var4) {
			if (_vm->_rulesBuffer2_12[i] == var4b) {
				_word16F00 = i;
				return 1;
			}
		}
	}	
	
	return 0;
}
byte LilliputScript::OC_sub17640() {
	warning("OC_sub17640");
	return 0;
}

byte LilliputScript::OC_sub176C4() {
	debugC(1, kDebugScript, "OC_sub176C4()");
	
	byte var4 = _currScript->readUint16LE() & 0xFF;

	int tmpVal = _currScript->readUint16LE();
	
	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + index];
		if (((var1 & 0xFF) >= var4) || ((var1 >> 8) < var4))
			return 0;
		
		_word16F00 = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_word10807_ERULES; i++) {
			int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + i];
			if (((var1 & 0xFF) < var4) && ((var1 >> 8) >= var4)) {
				_word16F00 = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_word10807_ERULES; i++) {
		int var1 = _array10B51[(_vm->_rulesBuffer2PrevIndx * 40) + i];
		if (((var1 & 0xFF) < var4) && ((var1 >> 8) >= var4)) {
			if (_vm->_rulesBuffer2_12[i] == var4b) {
				_word16F00 = i;
				return 1;
			}
		}
	}	
	
	return 0;
}

byte LilliputScript::OC_compWord10804() {
	debugC(1, kDebugScript, "OC_compWord10804()");
	
	byte tmpVal = getValue1();
	
	if (tmpVal == _word10804)
		return 1;

	return 0;
}
byte LilliputScript::OC_sub17766() {
	warning("OC_sub17766");
	return 0;
}
byte LilliputScript::OC_sub17782() {
	warning("OC_sub17782");
	return 0;
}
byte LilliputScript::OC_sub1779E() {
	warning("OC_sub1779E");
	return 0;
}
byte LilliputScript::OC_sub177C6() {
	debugC(1, kDebugScript, "OC_sub177C6()");

	int index = _currScript->readUint16LE();
	if (_vm->_rulesBuffer2_1[index] == 0xFFFF)
		return 0;

	return 1;
}
byte LilliputScript::OC_compWord16EFE() {
	debugC(1, kDebugScript, "OC_compWord16EFE()");

	byte curByte = _currScript->readUint16LE() & 0xFF;
	byte tmpVal = _vm->_word16EFE >> 8;

	if (curByte != tmpVal)
		return 0;

	_word16F00 = (_vm->_word16EFE & 0xFF);
	return 1;
}

byte LilliputScript::OC_sub177F5() {
	debugC(1, kDebugScript, "OC_sub177F5()");

	byte var1 = _currScript->readUint16LE() & 0xFF;
	byte var2 = _currScript->readUint16LE() & 0xFF;

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);

	if ((var1 == _vm->_ptr_rulesBuffer2_15[0]) && (var2 == _vm->_ptr_rulesBuffer2_15[1]))
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17812() {
	debugC(1, kDebugScript, "OC_sub17812()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	if (_vm->_ptr_rulesBuffer2_15[0] == curByte)
		return 1;
	return 0;
}

byte LilliputScript::OC_sub17825() {
	debugC(1, kDebugScript, "OC_sub17825()");

	byte tmpVal = (_currScript->readUint16LE() & 0xFF);
	
	if ((_vm->_byte16F07_menuId != 1) && (_vm->_byte16F07_menuId != 3))
		return 0;

	if (tmpVal == _byte12FE4)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17844() {
	warning("OC_sub17844");
	return 0;
}

byte LilliputScript::OC_sub1785C() {
	debugC(1, kDebugScript, "OC_sub1785C()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	int count = 0;

	for (int i = 0; i < _vm->_word10807_ERULES; i++) {
		if (curByte == _vm->_rulesBuffer2_15[(32 * i)])
			++count;
	}

	int oper = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();

	return compareValues(count, oper, var2);
}

byte LilliputScript::OC_sub17886() {
	warning("OC_sub17886");
	return 0;
}
byte LilliputScript::OC_sub178A8() {
	warning("OC_sub178A8");
	return 0;
}
byte LilliputScript::OC_sub178BA() {
	warning("OC_sub178BA");
	return 0;
}
byte LilliputScript::OC_sub178C2() {
	debugC(1, kDebugScript, "OC_sub178C2()");

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	if (_vm->_ptr_rulesBuffer2_15[2] == 1)
		return 1;
	return 0;
}
byte LilliputScript::OC_sub178D2() {
	warning("OC_sub178D2");
	return 0;
}
byte LilliputScript::OC_sub178E8() {
	warning("OC_sub178E8");
	return 0;
}
byte LilliputScript::OC_sub178FC() {
	warning("OC_sub178FC");
	return 0;
}
byte LilliputScript::OC_sub1790F() {
	warning("OC_sub1790F");
	return 0;
}

byte LilliputScript::OC_sub1792A() {
	debugC(1, kDebugScript, "OC_sub1792A()");

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (_vm->_ptr_rulesBuffer2_15[1] == curByte)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub1793E() {
	warning("OC_sub1793E");
	return 0;
}

byte LilliputScript::OC_sub1795E() {
	debugC(1, kDebugScript, "OC_sub1795E()");

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	if (_vm->_ptr_rulesBuffer2_15[3] == 1)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub1796E() {
	warning("OC_sub1796E");
	return 0;
}
byte LilliputScript::OC_sub17984() {
	debugC(1, kDebugScript, "OC_sub17984()");

	int index = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();

	assert(index < 20);

	if (_array122E9[index] == (var2 & 0xFF))
		return 1;

	return 0;
}
byte LilliputScript::OC_checkSavedMousePos() {
	warning("OC_checkSavedMousePos");
	return 0;
}
byte LilliputScript::OC_sub179AE() {
	warning("OC_sub179AE");
	return 0;
}
byte LilliputScript::OC_sub179C2() {
	warning("OC_sub179C2");
	return 0;
}
byte LilliputScript::OC_sub179E5() {
	warning("OC_sub179E5");
	return 0;
}
byte LilliputScript::OC_sub17A07() {
	warning("OC_sub17A07");
	return 0;
}
byte LilliputScript::OC_sub17757() {
	warning("OC_sub17757");
	return 0;
}

void LilliputScript::OC_setWord18821() {
	warning("OC_setWord18821");
}
void LilliputScript::OC_sub17A3E() {
	warning("OC_sub17A3E");
}

void LilliputScript::OC_sub17D57() {
	debugC(1, kDebugScript, "OC_sub17D57()");

	int curWord = _currScript->readUint16LE();
	curWord = sub17D40(curWord);

	_word1881B = _vm->_rulesBuffer2PrevIndx;

	sub18B3C(curWord);
}

void LilliputScript::OC_sub17D7F() {
	warning("OC_sub17D7F");
}
void LilliputScript::OC_sub17DB9() {
	warning("OC_sub17DB9");
}

void LilliputScript::OC_sub17DF9() {
	debugC(1, kDebugScript, "OC_sub17DF9()");

	if ((_word1881B & 0xFF) == 0xFF) {
		OC_sub17D57();
		return;
	}

	_currScript->readUint16LE();
}

void LilliputScript::OC_sub17E07() {
	warning("OC_sub17E07");
}
void LilliputScript::OC_sub17E15() {
	warning("OC_sub17E15");
}

void LilliputScript::OC_sub17B03() {
	debugC(1, kDebugScript, "OC_sub17B03()");

	byte *bufPtr = getBuffer215Ptr();
	int oper = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();

	computeOperation(bufPtr, oper, var2);
}

void LilliputScript::OC_getRandom_type2() {
	warning("OC_getRandom_type2");
}

void LilliputScript::OC_sub17A66() {
	debugC(1, kDebugScript, "OC_sub17A66()");
	
	int index = getValue1();
	int tmpVal = getValue2();

	int var2 = ((tmpVal >> 8) << 3) + 4;
	int var4 = ((tmpVal & 0xFF) << 3) + 4;

	assert(index < 40);
	_vm->_rulesBuffer2_1[index] = var2;
	_vm->_rulesBuffer2_2[index] = var4;
}

void LilliputScript::OC_sub17A8D() {
	debugC(1, kDebugScript, "OC_sub17A8D()");

	int tmpVal = getValue1();
	assert(tmpVal < 40);

	if (tmpVal == _word10804)
		_word10802 = 0xFFFF;

	_vm->_rulesBuffer2_1[tmpVal] = 0xFFFF;
	_vm->_rulesBuffer2_2[tmpVal] = 0xFFFF;
}
void LilliputScript::OC_saveAndQuit() {
	warning("OC_saveAndQuit");
}
void LilliputScript::OC_sub17B93() {
	debugC(1, kDebugScript, "OC_sub17B93()");
	int var1 = _currScript->readUint16LE();

	sub17B6C(var1);
}

void LilliputScript::OC_sub17E37() {
	warning("OC_sub17E37");
}
void LilliputScript::OC_resetByte1714E() {
	warning("OC_resetByte1714E");
}
void LilliputScript::OC_deleteSavegameAndQuit() {
	warning("OC_deleteSavegameAndQuit");
}
void LilliputScript::OC_incByte16F04() {
	warning("OC_incByte16F04");
}

void LilliputScript::OC_sub17BA5() {
	debugC(1, kDebugScript, "OC_sub17BA5()");
	
	byte *tmpArr = getBuffer215Ptr();
	byte oper = (_currScript->readUint16LE() & 0xFF);
	byte var2 = getBuffer215Ptr()[0];
	computeOperation(tmpArr, oper, var2);
}

void LilliputScript::OC_setByte18823() {
	warning("OC_setByte18823");
}
void LilliputScript::OC_sub17BB7() {
	debugC(1, kDebugScript, "OC_sub17BB7()");

	int index = _currScript->readUint16LE();
	int var1 = getValue1();

	_vm->sub170EE(var1);
	int tmpIndex = _vm->_rulesBuffer2PrevIndx;

	assert(index < _vm->_gameScriptIndexSize);
	int scriptIndex = _vm->_arrayGameScriptIndex[index];

	_scriptStack.push(_currScript);
	warning("===> push");
	if (_byte16F05_ScriptHandler == 0) {
		_vm->_byte1714E = 0;
		runMenuScript(Common::MemoryReadStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
	} else {
		runScript(Common::MemoryReadStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
	}
	warning("===> pop");
	_currScript = _scriptStack.pop();

	_vm->sub170EE(tmpIndex);
}

void LilliputScript::OC_sub17BF2() {
	debugC(1, kDebugScript, "OC_sub17BF2()");

	OC_sub17BB7();
	sub17B6C(0);
}

void LilliputScript::OC_sub17ACC() {
	warning("OC_sub17ACC");
}
void LilliputScript::OC_resetByte16F04() {
	warning("OC_resetByte16F04");
}

void LilliputScript::OC_sub17AE1() {
	debugC(1, kDebugScript, "OC_sub17AE1()");

	byte var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_rulesBuffer2PrevIndx, var3);
}

void LilliputScript::OC_sub17AEE() {
	debugC(1, kDebugScript, "OC_sub17AEE()");

	byte var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_rulesBuffer2PrevIndx + 1, var3);
}

void LilliputScript::OC_setWord10804() {
	debugC(1, kDebugScript, "OC_setWord10804()");

	_word10804 = getValue1();
}
void LilliputScript::OC_sub17C0E() {
	warning("OC_sub17C0E");
}
void LilliputScript::OC_sub17C55() {
	warning("OC_sub17C55");
}
void LilliputScript::OC_sub17C76() {
	warning("OC_sub17C76");
}
void LilliputScript::OC_sub17AFC() {
	warning("OC_sub17AFC");
}
void LilliputScript::OC_sub17C8B() {
	warning("OC_sub17C8B");
}
void LilliputScript::OC_sub17CA2() {
	warning("OC_sub17CA2");
}
void LilliputScript::OC_sub17CB9() {
	warning("OC_sub17CB9");
}
void LilliputScript::OC_sub17CD1() {
	warning("OC_sub17CD1");
}
void LilliputScript::OC_resetWord16EFE() {
	warning("OC_resetWord16EFE");
}
void LilliputScript::OC_sub17CEF() {
	warning("OC_sub17CEF");
}

void LilliputScript::OC_sub17D1B() {
	debugC(1, kDebugScript, "OC_sub17D1B()");

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	++_vm->_ptr_rulesBuffer2_15[1];
}

void LilliputScript::OC_sub17D23() {
	warning("OC_sub17D23");
}
void LilliputScript::OC_sub17E6D() {
	warning("OC_sub17E6D");
}
void LilliputScript::OC_sub17E7E() {
	warning("OC_sub17E7E");
}
void LilliputScript::OC_sub17E99() {
	warning("OC_sub17E99");
}
void LilliputScript::OC_sub17EC5() {
	warning("OC_sub17EC5");
}
void LilliputScript::OC_sub17EF4() {
	warning("OC_sub17EF4");
}
void LilliputScript::OC_sub17F08() {
	warning("OC_sub17F08");
}
void LilliputScript::OC_sub17F4F() {
	warning("OC_sub17F4F");
}
void LilliputScript::OC_sub17F68() {
	warning("OC_sub17F68");
}
void LilliputScript::OC_getNextVal() {
	warning("OC_getNextVal");
}
void LilliputScript::OC_sub17FD2() {
	warning("OC_sub17FD2");
}

void LilliputScript::OC_sub17FDD() {
	debugC(1, kDebugScript, "OC_sub17FDD()");

	int index = _currScript->readUint16LE();
	
	int tmpVal = (_vm->_rulesBuffer2PrevIndx * 32) + index;
	assert (tmpVal < 40 * 32);
	_array10AB1[_vm->_rulesBuffer2PrevIndx] = _vm->_rulesBuffer2_16[tmpVal];
	_array12811[_vm->_rulesBuffer2PrevIndx] = 16;
}

void LilliputScript::OC_setByte10B29() {
	warning("OC_setByte10B29");
}

void LilliputScript::OC_sub18007() {
	debugC(1, kDebugScript, "OC_sub18007()");

	int curWord = _currScript->readUint16LE();
	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	_vm->_ptr_rulesBuffer2_15[2] = curWord & 0xFF;
}

void LilliputScript::OC_sub18014() {
	debugC(1, kDebugScript, "OC_sub18014()");

	assert(_vm->_ptr_rulesBuffer2_15 != NULL);
	_vm->_ptr_rulesBuffer2_15[2] = 0;
}

void LilliputScript::OC_sub1801D() {
	warning("OC_sub1801D");
}
void LilliputScript::OC_sub1805D() {
	warning("OC_sub1805D");
}
void LilliputScript::OC_sub18074() {
	warning("OC_sub18074");
}
void LilliputScript::OC_sub1808B() {
	warning("OC_sub1808B");
}
void LilliputScript::OC_sub18099() {
	debugC(1, kDebugScript, "OC_sub18099()");

	int index = _currScript->readUint16LE();
	assert((index >= 0) && (index < 20));
	int curWord = _currScript->readUint16LE();

	_array122E9[index] = (curWord & 0xFF);
	_array122FD[index] = (curWord >> 8);

	warning("TODO: display function #8");
	
}
void LilliputScript::OC_sub180C3() {
	warning("OC_sub180C3");
}
void LilliputScript::OC_sub1810A() {
	warning("OC_sub1810A");
}

void LilliputScript::OC_sub1812D() {
	debugC(1, kDebugScript, "OC_sub1812D()");

	_vm->_rulesBuffer2_3[_vm->_rulesBuffer2PrevIndx] = (_currScript->readUint16LE() & 0xFF);
}

void LilliputScript::OC_sub1817F() {
	warning("OC_sub1817F");
}
void LilliputScript::OC_sub181BB() {
	warning("OC_sub181BB");
}
void LilliputScript::OC_sub18213() {
	warning("OC_sub18213");
}
void LilliputScript::OC_sub18252() {
	warning("OC_sub18252");
}
void LilliputScript::OC_sub18260() {
	warning("OC_sub18260");
}
void LilliputScript::OC_sub182EC() {
	warning("OC_sub182EC");
}
void LilliputScript::OC_unkPaletteFunction_1() {
	warning("OC_unkPaletteFunction_1");
}
void LilliputScript::OC_unkPaletteFunction_2() {
	warning("OC_unkPaletteFunction_2");
}

void LilliputScript::OC_loadAndDisplayCUBESx_GFX() {
	debugC(1, kDebugScript, "OC_loadAndDisplayCUBESx_GFX()");

	int curWord = _currScript->readUint16LE();
	assert((curWord >= 0) && (curWord <= 9));
	Common::String fileName = Common::String::format("CUBES%d.GFX", curWord);
	_byte10806 = curWord + 0x30;
	warning("TODO: load %s then display things", fileName.c_str());
}

void LilliputScript::OC_sub1834C() {
	debugC(1, kDebugScript, "OC_sub1834C()");

	byte curWord = _currScript->readUint16LE() & 0xFF;
	assert(_vm->_ptr_rulesBuffer2_15 != NULL);

	_vm->_ptr_rulesBuffer2_15[3] = curWord;
}
void LilliputScript::OC_sub18359() {
	warning("OC_sub18359");
}
void LilliputScript::OC_sub18367() {
	warning("OC_sub18367");
}
void LilliputScript::OC_sub17D04() {
	debugC(1, kDebugScript, "OC_sub17D04()");

	byte var1 = getValue1();
	byte var2 = _currScript->readUint16LE() & 0xFF;
	
	sub1823E(var1, var2, &_vm->_rulesBuffer2_15[var1]);
}

void LilliputScript::OC_sub18387() {
	warning("OC_sub18387");
}
void LilliputScript::OC_setByte14835() {
	warning("OC_setByte14835");
}
void LilliputScript::OC_setByte14837() {
	warning("OC_setByte14837");
}
void LilliputScript::OC_sub183A2() {
	warning("OC_sub183A2");
}
void LilliputScript::OC_sub183C6() {
	warning("OC_sub183C6");
}
void LilliputScript::OC_loadFile_AERIAL_GFX() {
	warning("OC_loadFile_AERIAL_GFX");
}
void LilliputScript::OC_sub17E22() {
	warning("OC_sub17E22");
}
void LilliputScript::OC_sub1844A() {
	warning("OC_sub1844A");
}
void LilliputScript::OC_sub1847F() {
	debugC(1, kDebugScript, "OC_sub1847F()");
	byte *buf215Ptr = getBuffer215Ptr();
	byte tmpVal = buf215Ptr[0];
	int curWord = _currScript->readUint16LE();
	assert(curWord != 0);
	int var1 = tmpVal / curWord;
	int var2 = _currScript->readUint16LE();
	int var3 = _currScript->readUint16LE();

	if (_byte16F08 != 1) {
		warning("TODO: OC_sub1847F - Display Function 5");
		warning("TODO: OC_sub1847F - sub_18BE6");
		warning("TODO: OC_sub1847F - Display Function 4");
	}
}

void LilliputScript::OC_displayVGAFile() {
	debugC(1, kDebugScript, "OC_displayVGAFile()");

	_byte12A09 = 1;
	warning("TODO: unkPaletteFunction_1");
	int curWord = _currScript->readUint16LE();
	int index = _vm->_rulesChunk3[curWord];
	Common::String fileName = Common::String((const char *)&_vm->_rulesChunk4[index]);
	_word1881B = -1;
	_vm->displayVGAFile(fileName);
	warning("TODO: unkPaletteFunction_2");
}
void LilliputScript::OC_sub184D7() {
	warning("OC_sub184D7");
}
void LilliputScript::OC_displayTitleScreen() {
	debugC(1, kDebugScript, "OC_displayTitleScreen()");

	_vm->_byte184F4 = (_currScript->readUint16LE() & 0xFF);
	_vm->_sound_byte16F06 = _vm->_byte184F4;

	// TODO: Rewrite keyboard handling (this code was in a separated function)
	_vm->_keyboard_nextIndex = 0;
	_vm->_keyboard_oldIndex = 0;
	//
	_vm->_mouseButton = 0;
	_vm->_byte16F09 = 0;

	for (;;) {
		sub185B4_display();
		_vm->pollEvent();
		if (_vm->_keyboard_nextIndex != _vm->_keyboard_oldIndex) {
			_vm->_byte16F09 = _vm->_keyboard_getch();
			_vm->_keyboard_getch();
			break;
		}
		
		if (_vm->_mouseButton & 1)
			break;
		
		if ((_vm->_byte184F4 != 0) && (_vm->_sound_byte16F06 == 0))
			break;
	}
	
	_vm->_mouseButton = 0;
}

void LilliputScript::OC_sub1853B() {
	debugC(1, kDebugScript, "OC_initArr1853B()");

	warning("TODO: unkPaletteFunction_1");
	_byte16F08 = 0;
	_byte15FFA = 0;
	sub130B6();
	warning("TODO: Display function sub15CBC();");
	warning("TODO: unkPaletteFunction_2");
	_byte12A09 = 0;
	warning("TODO: call sound function #5");
}

void LilliputScript::OC_sub1864D() {
	warning("OC_sub1864D");
}

void LilliputScript::OC_initArr18560() {
	debugC(1, kDebugScript, "OC_initArr18560()");

	int curWord = _currScript->readUint16LE();
	assert (curWord < 4);
	_vm->_arr18560[curWord]._field0 = 1;
	_vm->_arr18560[curWord]._field1 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field3 = _currScript->readUint16LE();

	for (int i = 0; i < 8; i++)
		_vm->_arr18560[curWord]._field5[i] = _currScript->readUint16LE();
}

void LilliputScript::OC_sub18678() {
	debugC(1, kDebugScript, "OC_initArr18578()");
	_savedBuffer215Ptr = getBuffer215Ptr();
	_word15FFB = _currScript->readUint16LE();
	_word15FFD = _currScript->readUint16LE();
}
void LilliputScript::OC_sub18690() {
	warning("OC_sub18690");
}
void LilliputScript::OC_setWord10802() {
	debugC(1, kDebugScript, "OC_setWord10802()");

	_word10802 = getValue1();
}

void LilliputScript::OC_sub186A1() {
	warning("OC_sub186A1");
}

void LilliputScript::OC_sub186E5_snd() {
	debugC(1, kDebugScript, "OC_sub186E5_snd()");
	int index = getValue1();
	assert(index < 40);

	byte var4h = 0xFF;
	byte var4l = (index & 0xFF);
	byte var3h = _array16123[index];
	byte var3l = _array1614B[index];
	byte var2h = (_word12A00 & 0xFF);
	byte var2l = (_word12A02 & 0xFF);
	int var1 = _currScript->readUint16LE();

	warning("TODO: call sound function #2");
}

void LilliputScript::OC_sub1870A_snd() {
	debugC(1, kDebugScript, "OC_sub1870A_snd()");

	int var3 = getValue2();
	int var4 = var3;
	int var2 = (_word12A00 << 8) + _word12A02;
	int var1 = (_currScript->readUint16LE() & 0xFF);
	warning("TODO: ovlContentOVL Function 2");
}

void LilliputScript::OC_sub18725_snd() {
	debugC(1, kDebugScript, "OC_sub18725_snd()");

	int var4 = getValue1() | 0xFF00;
	warning("TODO: ovlContentOVL Function 3");
}

void LilliputScript::OC_sub18733_snd() {
	debugC(1, kDebugScript, "OC_sub18733_snd()");

	int var4 = getValue2();
	warning("TODO: ovlContentOVL Function 3");
}

void LilliputScript::OC_sub1873F_snd() {
	debugC(1, kDebugScript, "OC_sub1873F_snd()");

	warning("TODO: ovlContentOVL Function 4");
}

void LilliputScript::OC_sub18746_snd() {
	debugC(1, kDebugScript, "OC_sub18746_snd()");

	int var4 = -1;
	int var2 = (_word12A00 << 8) + _word12A02;
	int var1 = _currScript->readUint16LE() & 0xFF;
	warning("TODO: ovlContentOVL Function 2");
}

void LilliputScript::OC_sub1875D_snd() {
	debugC(1, kDebugScript, "OC_sub1875D_snd()");

	warning("TODO: ovlContentOVL Function 6");
}

void LilliputScript::OC_sub18764() {
	debugC(1, kDebugScript, "OC_sub18764()");

	int index = getValue1();
	int var1 = _currScript->readUint16LE();

	assert(index < 40);
	_array128EF[index] = var1 & 0xFF;
}

} // End of namespace
