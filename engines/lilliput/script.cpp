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

namespace Lilliput {

LilliputScript::LilliputScript(LilliputEngine *vm) : _vm(vm), _currScript(NULL) {
}

LilliputScript::~LilliputScript() {
}

byte LilliputScript::handleOpcodeType1(int curWord) {
	warning("handleOpcodeType1: %d", curWord);
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
	warning("handleOpcodeType1: %d", curWord);
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
		OC_sub184F5();
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

int LilliputScript::handleOpcode(Common::MemoryReadStream script) {
	_currScript = &script;
	uint16 curWord = script.readUint16LE();
	if (curWord == 0xFFF6)
		return -1;

	while (curWord != 0xFFF8) {
		byte mask = 0; 
		if (curWord > 1000) {
			curWord -= 1000;
			mask = 1;
		}
		byte result = handleOpcodeType1(curWord);
		if ((result ^ mask) == 0) {
			do {
				curWord = script.readUint16LE();
			} while (curWord != 0xFFF7);
			return 0;
		}
	}

	_vm->_vm_byte1714E = 1;

	for (;;) {
		curWord = script.readUint16LE();
		if (curWord == 0xFFF7)
			return _vm->_vm_byte1714E;

		handleOpcodeType2(curWord);
	}
}

void LilliputScript::runScript(Common::MemoryReadStream script) {
	_byte16F05_ScriptHandler = 1;
	
	while (handleOpcode(script) != 0xFF)
		;
	
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
	warning("OC_sub17434");
	return 0;
}
byte LilliputScript::OC_sub17468() {
	warning("OC_sub17468");
	return 0;
}
byte LilliputScript::OC_getRandom() {
	warning("OC_getRandom");
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
	warning("OC_checkSaveFlag");
	return 0;
}
byte LilliputScript::OC_sub174C8() {
	warning("OC_sub174C8");
	return 0;
}
byte LilliputScript::OC_sub174D8() {
	warning("OC_sub174D8");
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
	warning("OC_compWord16EF8");
	return 0;
}
byte LilliputScript::OC_sub175C8() {
	warning("OC_sub175C8");
	return 0;
}
byte LilliputScript::OC_sub17640() {
	warning("OC_sub17640");
	return 0;
}
byte LilliputScript::OC_sub176C4() {
	warning("OC_sub176C4");
	return 0;
}
byte LilliputScript::OC_compWord10804() {
	warning("OC_compWord10804");
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
	warning("OC_sub177C6");
	return 0;
}
byte LilliputScript::OC_compWord16EFE() {
	warning("OC_compWord16EFE");
	return 0;
}
byte LilliputScript::OC_sub177F5() {
	warning("OC_sub177F5");
	return 0;
}
byte LilliputScript::OC_sub17812() {
	warning("OC_sub17812");
	return 0;
}
byte LilliputScript::OC_sub17825() {
	warning("OC_sub17825");
	return 0;
}
byte LilliputScript::OC_sub17844() {
	warning("OC_sub17844");
	return 0;
}
byte LilliputScript::OC_sub1785C() {
	warning("OC_sub1785C");
	return 0;
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
	warning("OC_sub178C2");
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
	warning("OC_sub1792A");
	return 0;
}
byte LilliputScript::OC_sub1793E() {
	warning("OC_sub1793E");
	return 0;
}
byte LilliputScript::OC_sub1795E() {
	warning("OC_sub1795E");
	return 0;
}
byte LilliputScript::OC_sub1796E() {
	warning("OC_sub1796E");
	return 0;
}
byte LilliputScript::OC_sub17984() {
	warning("OC_sub17984");
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
	warning("OC_sub17D57");
}
void LilliputScript::OC_sub17D7F() {
	warning("OC_sub17D7F");
}
void LilliputScript::OC_sub17DB9() {
	warning("OC_sub17DB9");
}
void LilliputScript::OC_sub17DF9() {
	warning("OC_sub17DF9");
}
void LilliputScript::OC_sub17E07() {
	warning("OC_sub17E07");
}
void LilliputScript::OC_sub17E15() {
	warning("OC_sub17E15");
}
void LilliputScript::OC_sub17B03() {
	warning("OC_sub17B03");
}
void LilliputScript::OC_getRandom_type2() {
	warning("OC_getRandom_type2");
}
void LilliputScript::OC_sub17A66() {
	warning("OC_sub17A66");
}
void LilliputScript::OC_sub17A8D() {
	warning("OC_sub17A8D");
}
void LilliputScript::OC_saveAndQuit() {
	warning("OC_saveAndQuit");
}
void LilliputScript::OC_sub17B93() {
	warning("OC_sub17B93");
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
	warning("OC_sub17BA5");
}
void LilliputScript::OC_setByte18823() {
	warning("OC_setByte18823");
}
void LilliputScript::OC_sub17BB7() {
	warning("OC_sub17BB7");
}
void LilliputScript::OC_sub17BF2() {
	warning("OC_sub17BF2");
}
void LilliputScript::OC_sub17ACC() {
	warning("OC_sub17ACC");
}
void LilliputScript::OC_resetByte16F04() {
	warning("OC_resetByte16F04");
}
void LilliputScript::OC_sub17AE1() {
	warning("OC_sub17AE1");
}
void LilliputScript::OC_sub17AEE() {
	warning("OC_sub17AEE");
}
void LilliputScript::OC_setWord10804() {
	warning("OC_setWord10804");
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
	warning("OC_sub17D1B");
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
	warning("OC_sub17FDD");
}
void LilliputScript::OC_setByte10B29() {
	warning("OC_setByte10B29");
}
void LilliputScript::OC_sub18007() {
	warning("OC_sub18007");
}
void LilliputScript::OC_sub18014() {
	warning("OC_sub18014");
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
	warning("OC_sub18099");
}
void LilliputScript::OC_sub180C3() {
	warning("OC_sub180C3");
}
void LilliputScript::OC_sub1810A() {
	warning("OC_sub1810A");
}
void LilliputScript::OC_sub1812D() {
	warning("OC_sub1812D");
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
	warning("OC_loadAndDisplayCUBESx_GFX");
}
void LilliputScript::OC_sub1834C() {
	warning("OC_sub1834C");
}
void LilliputScript::OC_sub18359() {
	warning("OC_sub18359");
}
void LilliputScript::OC_sub18367() {
	warning("OC_sub18367");
}
void LilliputScript::OC_sub17D04() {
	warning("OC_sub17D04");
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
	warning("OC_sub1847F");
}
void LilliputScript::OC_displayVGAFile() {
	warning("OC_displayVGAFile");

	_vm_byte12A09 = 1;
	warning("TODO: unkPaletteFunction_1");
	int curWord = _currScript->readUint16LE();
	int index = _vm->_rulesChunk3[curWord];
	Common::String fileName = Common::String((const char *)&_vm->_rulesChunk4[index]);
	_vm_word1881B = -1;
	warning("TODO: guess_displayFunction_VGAFile");
	warning("TODO: unkPaletteFunction_2");
}
void LilliputScript::OC_sub184D7() {
	warning("OC_sub184D7");
}
void LilliputScript::OC_sub184F5() {
	warning("OC_sub184F5");
}
void LilliputScript::OC_sub1853B() {
	warning("OC_sub1853B");
}
void LilliputScript::OC_sub1864D() {
	warning("OC_sub1864D");
}
void LilliputScript::OC_initArr18560() {
	warning("OC_initArr18560");
	int curWord = _currScript->readUint16LE();
	assert (curWord < 4);
	_vm->_arr18560[curWord]._field0 = 1;
	_vm->_arr18560[curWord]._field1 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field3 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field5 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field7 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field9 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._fieldB = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._fieldD = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._fieldF = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field11 = _currScript->readUint16LE();
	_vm->_arr18560[curWord]._field13 = _currScript->readUint16LE();
}
void LilliputScript::OC_sub18678() {
	warning("OC_sub18678");
}
void LilliputScript::OC_sub18690() {
	warning("OC_sub18690");
}
void LilliputScript::OC_setWord10802() {
	warning("OC_setWord10802");
}
void LilliputScript::OC_sub186A1() {
	warning("OC_sub186A1");
}
void LilliputScript::OC_sub186E5_snd() {
	warning("OC_sub186E5_snd");
}
void LilliputScript::OC_sub1870A_snd() {
	warning("OC_sub1870A_snd");
}
void LilliputScript::OC_sub18725_snd() {
	warning("OC_sub18725_snd");
}
void LilliputScript::OC_sub18733_snd() {
	warning("OC_sub18733_snd");
}
void LilliputScript::OC_sub1873F_snd() {
	warning("OC_sub1873F_snd");
}
void LilliputScript::OC_sub18746_snd() {
	warning("OC_sub18746_snd");
	int curWord = _currScript->readUint16LE();
	curWord = (2 << 8) + (curWord & 0xFF);
	warning("TODO: ovlContentOVL Function 2, init DX and BX");
}
void LilliputScript::OC_sub1875D_snd() {
	warning("OC_sub1875D_snd");
}
void LilliputScript::OC_sub18764() {
	warning("OC_sub18764");
}
} // End of namespace
