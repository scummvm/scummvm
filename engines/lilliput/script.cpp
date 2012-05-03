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

#include "common/system.h"

namespace Lilliput {

LilliputScript::LilliputScript(LilliputEngine *vm) : _vm(vm), _currScript(NULL) {
	_byte129A0 = 0xFF;
	_byte1855D = 0;
	_byte12A04 = 0;
	_byte10806 = 0;
	_byte12FE4 = 0xFF;
	_byte16F02 = 0;
	_byte16F04 = 0;
	_byte1881A = 0;
	_byte18823 = 0;
	_byte1881E = 3;
	_byte1881D = 0;
	_word1855E = 0;
	_word16F00 = -1;
	_viewportCharacterTarget = -1;
	_word10804 = 0;
	_heroismBarX = 0;
	_heroismBarBottomY = 0;
	_viewportX = 0;
	_viewportY = 0;
	_word18776 = 0;

	_savedBuffer215Ptr = NULL;

	for (int i = 0; i < 20; i++) {
		_array122E9[i] = 0;
		_array122FD[i] = 0;
	}

	for (int i = 0; i < 32; i++)
		_array1813B[i] = 0;

	for (int i = 0; i < 40; i++) {
		_characterScriptEnabled[i] = 1;
		_array128EF[i] = 15;
		_array10AB1[i] = 0;
		_array12811[i] = 16;
		_array12839[i] = 0xFF;
		_array16123[i] = 0;
		_array1614B[i] = 0;
		_array122C1[i] = 0;
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
		return OC_for();
		break;
	case 0x7:
		return OC_compWord18776();
		break;
	case 0x8:
		return OC_checkSaveFlag();
		break;
	case 0x9:
		return OC_compByte16F04();
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
		return OC_checkCharacterDirection();
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
		OC_setCharacterPosition();
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
		OC_callScript();
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
		OC_changeCurrentCharacterSprite();
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
		OC_setCurrentCharacterDirection();
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
		OC_PaletteFadeOut();
		break;
	case 0x44:
		OC_PaletteFadeIn();
		break;
	case 0x45:
		OC_loadAndDisplayCUBESx_GFX();
		break;
	case 0x46:
		OC_sub1834C();
		break;
	case 0x47:
		OC_setArray122C1();
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
		OC_setDebugFlag();
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
		OC_setViewPortCharacterTarget();
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



static const OpCode opCodes1[] = {
	{ "OC_sub173DF", 1, kGetValue2, kNone, kNone, kNone, kNone },
	{ "OC_sub173F0", 2, kGetValue1, kGetValue2, kNone, kNone, kNone },
	{ "OC_sub1740A", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17434", 4, kGetValue1, kImmediateValue, kCompareOperation, kImmediateValue, kNone },
	{ "OC_sub17468", 2, kCompareOperation, kImmediateValue, kNone, kNone, kNone },
	{ "OC_getRandom", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_for", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_compWord18776", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_checkSaveFlag", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_compByte16F04", 2, kCompareOperation, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub174D8", 2, kGetValue1, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub1750E", 5, kGetValue1, kImmediateValue, kCompareOperation, kGetValue1, kImmediateValue },
	{ "OC_compareCoords_1", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_compareCoords_2", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub1757C", 3, kGetValue2, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_sub1759E", 3, kGetValue1, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_compWord16EF8", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_sub175C8", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub17640", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub176C4", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_compWord10804", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_sub17766", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17782", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub1779E", 4, kGetValue2, kImmediateValue, kImmediateValue, kCompareOperation, kNone },
	{ "OC_sub177C6", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_compWord16EFE", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub177F5", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub17812", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17825", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17844", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub1785C", 3, kImmediateValue, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_sub17886", 1, kGetValue2, kNone, kNone, kNone, kNone },
	{ "OC_sub178A8", 2, kGetValue1, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub178BA", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub178C2", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub178D2", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub178E8", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },
	{ "OC_sub178FC", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub1790F", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_sub1792A", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub1793E", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub1795E", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub1796E", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub17984", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_checkSavedMousePos", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub179AE", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub179C2", 1, kGetValue2, kNone, kNone, kNone, kNone },
	{ "OC_sub179E5", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17A07", 3, kImmediateValue, kImmediateValue, kImmediateValue, kNone, kNone },
	{ "OC_sub17757", 1, kGetValue1, kNone, kNone, kNone, kNone },
};


static const OpCode opCodes2[] = {
/* 0x00 */	{ "OC_setWord18821", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x01 */	{ "OC_sub17A3E", 3, kGetValue2, kImmediateValue, kImmediateValue, kNone, kNone },
/* 0x02 */	{ "OC_sub17D57", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x03 */	{ "OC_sub17D7F", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone },
/* 0x04 */	{ "OC_sub17DB9", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, // todo
/* 0x05 */	{ "OC_sub17DF9", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x06 */	{ "OC_sub17E07", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, // pb
/* 0x07 */	{ "OC_sub17E15", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x08 */	{ "OC_sub17B03", 4, kGetValue1, kImmediateValue, kComputeOperation, kImmediateValue, kNone },
/* 0x09 */	{ "OC_getRandom_type2", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },
/* 0x0a */	{ "OC_setCharacterPosition", 2, kGetValue1, kGetValue2, kNone, kNone, kNone },
/* 0x0b */	{ "OC_sub17A8D", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x0c */	{ "OC_saveAndQuit", 0, kNone, kNone, kNone, kNone, kNone },
/* 0x0d */	{ "OC_sub17B93", 1, kImmediateValue, kNone, kNone, kNone, kNone }, // todo : jump to other opcode
/* 0x0e */	{ "OC_sub17E37", 0, kNone, kNone, kNone, kNone, kNone },  // todo
/* 0x0f */	{ "OC_resetByte1714E", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x10 */	{ "OC_deleteSavegameAndQuit", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x11 */	{ "OC_incByte16F04", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x12 */	{ "OC_sub17BA5", 5, kGetValue1, kImmediateValue,kComputeOperation, kGetValue1, kImmediateValue },
/* 0x13 */	{ "OC_setByte18823", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },  
/* 0x14 */	{ "OC_callScript", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },  // run script
/* 0x15 */	{ "OC_sub17BF2", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },  // run script then stop
/* 0x16 */	{ "OC_sub17ACC", 1, kGetValue2, kNone, kNone, kNone, kNone },  
/* 0x17 */	{ "OC_resetByte16F04", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x18 */	{ "OC_sub17AE1", 1, kImmediateValue, kNone, kNone, kNone, kNone },  
/* 0x19 */	{ "OC_sub17AEE", 1, kImmediateValue, kNone, kNone, kNone, kNone },  
/* 0x1a */	{ "OC_setWord10804", 1, kGetValue1, kNone, kNone, kNone, kNone },  
/* 0x1b */	{ "OC_sub17C0E", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x1c */ 	{ "OC_sub17C55", 4, kGetValue1, kGetValue1, kImmediateValue, kImmediateValue, kNone }, 
/* 0x1d */	{ "OC_sub17C76", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x1e */	{ "OC_sub17AFC", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x1f */	{ "OC_sub17C8B", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x20 */	{ "OC_sub17CA2", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x21 */	{ "OC_sub17CB9", 3, kImmediateValue, kGetValue1, kImmediateValue, kNone, kNone }, 
/* 0x22 */	{ "OC_sub17CD1", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x23 */	{ "OC_resetWord16EFE", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x24 */	{ "OC_sub17CEF", 1, kImmediateValue, kNone, kNone, kNone, kNone },   // stop script
/* 0x25 */	{ "OC_sub17D1B", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x26 */	{ "OC_sub17D23", 2, kImmediateValue, kGetValue2, kNone, kNone, kNone }, 
/* 0x27 */	{ "OC_sub17E6D", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x28 */	{ "OC_sub17E7E", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x29 */	{ "OC_sub17E99", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x2a */	{ "OC_sub17EC5", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x2b */	{ "OC_sub17EF4", 1, kGetValue2, kNone, kNone, kNone, kNone }, 
/* 0x2c */	{ "OC_sub17F08", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x2d */	{ "OC_sub17F4F", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x2e */	{ "OC_sub17F68", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x2f */	{ "OC_getNextVal", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x30 */	{ "OC_sub17FD2", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x31 */	{ "OC_sub17FDD", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x32 */	{ "OC_setByte10B29", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x33 */	{ "OC_sub18007", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x34 */	{ "OC_sub18014", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x35 */	{ "OC_sub1801D", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x36 */	{ "OC_sub1805D", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x37 */	{ "OC_sub18074", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x38 */	{ "OC_setCurrentCharacterDirection", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x39 */	{ "OC_sub18099", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x3a */	{ "OC_sub180C3", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x3b */	{ "OC_sub1810A", 1, kGetValue2, kNone, kNone, kNone, kNone }, 
/* 0x3c */	{ "OC_sub1812D", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x3d */	{ "OC_sub1817F", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x3e */	{ "OC_sub181BB", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x3f */	{ "OC_sub18213", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x40 */	{ "OC_sub18252", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x41 */	{ "OC_sub18260", 2, kGetValue1, kGetValue2, kNone, kNone, kNone }, // TODO
/* 0x42 */	{ "OC_sub182EC", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x43 */	{ "OC_PaletteFadeOut", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x44 */	{ "OC_PaletteFadeIn", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x45 */	{ "OC_loadAndDisplayCUBESx_GFX", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x46 */	{ "OC_sub1834C", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x47 */	{ "OC_setArray122C1", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x48 */	{ "OC_sub18367", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x49 */	{ "OC_sub17D04", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x4a */	{ "OC_sub18387", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x4b */	{ "OC_setDebugFlag", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4c */	{ "OC_setByte14837", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4d */	{ "OC_sub183A2", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4e */	{ "OC_sub183C6", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },  // TODO
/* 0x4f */	{ "OC_loadFile_AERIAL_GFX", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x50 */	{ "OC_sub17E22", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x51 */	{ "OC_sub1844A", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x52 */	{ "OC_sub1847F", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x53 */	{ "OC_displayVGAFile", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x54 */	{ "OC_sub184D7", 1, kImmediateValue, kNone, kNone, kNone, kNone },   // TODO
/* 0x55 */	{ "OC_displayTitleScreen", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x56 */	{ "OC_sub1853B", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x57 */	{ "OC_sub1864D", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone },  // TODO
/* 0x58 */	{ "OC_initArr18560", 11, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x59 */	{ "OC_sub18678", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x5a */	{ "OC_sub18690", 2, kGetValue1, kGetValue2, kNone, kNone, kNone },  //TODO
/* 0x5b */	{ "OC_setViewPortCharacterTarget", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x5c */	{ "OC_sub186A1", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },  //TODO
/* 0x5d */	{ "OC_sub186E5_snd", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x5e */	{ "OC_sub1870A_snd", 2, kGetValue2, kImmediateValue, kNone, kNone, kNone }, 
/* 0x5f */	{ "OC_sub18725_snd", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x60 */	{ "OC_sub18733_snd", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x61 */	{ "OC_sub1873F_snd", 1, kGetValue2, kNone, kNone, kNone, kNone }, 
/* 0x62 */	{ "OC_sub18746_snd", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x63 */	{ "OC_sub1875D_snd", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x64 */	{ "OC_sub18764", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x65 */	{ "OC_sub1853B", 0, kNone, kNone, kNone, kNone, kNone }
};

Common::String LilliputScript::getArgumentString(KValueType type, ScriptStream& script) {
	
	Common::String str;
	if(type == kImmediateValue) {
		str =  Common::String::format("0x%x", script.readUint16LE());
	} else if (type == kGetValue1) {
		int val = script.readUint16LE();
		if(val < 1000) { 
			str = Common::String::format("0x%x", val);
		} else if (val > 1004) { 
			str = Common::String::format("getValue1(0x%x)", val);
		} else if ( val == 1000 ) {
			str = Common::String("_byte129A0");
		} else if( val == 1001 ) {
			str = Common::String("characterIndex");
		} else if( val == 1002 ) {
			str = Common::String("_word16F00");
		} else if( val == 1003 ) {
			str = Common::String("_currentCharacterVariables[6]");
		} else if( val == 1004 ) {
			str = Common::String("_word10804");
		}
	} else if (type == kGetValue2) {
		int curWord = script.readUint16LE();
		int tmpVal = curWord >> 8;
		switch(tmpVal) {
	case 0xFF:
		str = "(_rulesBuffer2_13[currentCharacter],_rulesBuffer2_14[currentCharacter])";
		break;
	case 0xFE: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		str = Common::String::format("_vm->_rulesBuffer2_13[%d],_vm->_rulesBuffer2_14[%d]", index, index);
		break;
			   }
	case 0xFD:
		str = "_currentScriptCharacterPosition";
		break;
	case 0xFC: {
		int index = curWord & 0xFF;
		assert(index < 40);
		str = Common::String::format("(characterPositionTileX[%d], characterPositionTileY[%d])", index, index);
		break;
			   }
	case 0xFB: {
		str = "(characterPositionTileX[_word16F00], characterPositionTileY[_word16F00])";
		break;
			   }
	case 0xFA:
		str = Common::String::format("(_array10999[currentCharacter], _array109C1[currentCharacter])");
		break;
	case 0xF9:
		str = Common::String::format("(_currentCharacterVariables[4], _currentCharacterVariables[5])");
		break;
	case 0xF8: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		str = Common::String::format("_vm->_rulesBuffer12_3[%d]", index);
	}
	case 0xF7: {
		str = Common::String::format("(_characterPositionTileX[_currentCharacterVariables[6]], _characterPositionTileY[_currentCharacterVariables[6]])");
		break;			 
	}
	case 0xF6:
		str = "_savedMousePosDivided";
		break;
	default:
		str = Common::String::format("(0x%x,0x%x)", curWord >> 8, curWord & 0xFF);
		break;
		}
	} else if (type == kCompareOperation) {
		int comp = script.readUint16LE();
		if(comp != '<' && comp != '>')
			comp = '=';
		str = Common::String::format("%c", comp );
	}
	else if (type == kComputeOperation) {
		int comp = script.readUint16LE();
		str = Common::String::format("%c", comp );
	}
	return str;
}

void LilliputScript::disasmScript( ScriptStream script) {
	
	while(!script.eos()) {
		uint16 val = script.readUint16LE();
		if (val == 0xFFF6) // end of script
			return;

		bool hasIf = false;

		if(val != 0xFFF8) {
			hasIf = true;
			
		}
		bool firstIf = true; 


		// check the conditions.
		while (val != 0xFFF8) {

			bool neg = false;

			if (val >= 1000) {
				val -= 1000;
				// negative condition
				neg = true;
			}

			// op code type 1
			assert(val < sizeof(opCodes1)/sizeof(OpCode));
			const OpCode* opCode = &opCodes1[val];
			const KValueType* opArgType = &opCode->_arg1;

			Common::String str;

			if(firstIf) {
				str = "if (";
				firstIf = false;
			} else {
				str = "    ";
			}
			if (neg) str += "not ";
			str += Common::String(opCode->_opName);
			str += "(";

			for (int p = 0; p < opCode->_numArgs; p++) {
				
				str += getArgumentString(*opArgType, script);
				if(p != opCode->_numArgs - 1)
					str += ", ";

				opArgType++;
			}
			str += ")";

			

			val = script.readUint16LE();

			if(val == 0xFFF8) {
				str += ")";
			}


			debugC(2, kDebugScript, str.c_str());

			
		}

		debugC(2, kDebugScript,"{ ");

		val = script.readUint16LE();

		while (val != 0xFFF7) {
			
			// op code type 2 
			assert(val < sizeof(opCodes2)/sizeof(OpCode));
			const OpCode* opCode = &opCodes2[val];
			const KValueType* opArgType = &opCode->_arg1;

			Common::String str;

			str = "    ";
			str += Common::String(opCode->_opName);
			str += "(";

			for (int p = 0; p < opCode->_numArgs; p++) {
				str += getArgumentString(*opArgType, script);

				if (p != opCode->_numArgs - 1)
					str += ", ";

				if ( p < 5 )
					opArgType++;
			}
			str += ");";

			debugC(2, kDebugScript, str.c_str());


			val = script.readUint16LE();
		}

		debugC(2, kDebugScript,"} ");
		debugC(2, kDebugScript," ");
	}
}

int LilliputScript::handleOpcode(ScriptStream *script) {
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

void LilliputScript::runScript(ScriptStream script) {
	debugC(1, kDebugScript, "runScript");

	_byte16F05_ScriptHandler = 1;
	
	while (handleOpcode(&script) != 0xFF)
		_vm->update();
}

void LilliputScript::runMenuScript(ScriptStream script) {
	debugC(1, kDebugScript, "runMenuScript");

	warning("========================== Menu Script ==============================");
	_byte16F05_ScriptHandler = 0;
	
	while (handleOpcode(&script) == 0)
		_vm->update();
}

void LilliputScript::sub185ED(byte index, byte subIndex) {
	debugC(2, kDebugScript, "sub185ED");

	if (_vm->_arr18560[index]._field0 != 1)
		return;

	_vm->displayFunction1(_vm->_bufferIdeogram, _vm->_arr18560[index]._field5[subIndex], _vm->_arr18560[index]._field1, _vm->_arr18560[index]._field3);
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

void LilliputScript::computeOperation(byte *bufPtr, int oper, int var3) {
	debugC(1, kDebugScript, "computeOperation(bufPtr, %c, %d)", oper & 0xFF, var3 & 0xFF);

	switch (oper & 0xFF) {
	case '=':
		bufPtr[0] = var3 & 0xFF;
		break;
	case '+': {
		int tmpVal = bufPtr[0] + var3;
		if (tmpVal > 0xFF)
			bufPtr[0] = 0xFF;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '-': {
		int tmpVal = bufPtr[0] - var3;
		if (tmpVal < 0)
			bufPtr[0] = 0;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '*': {
		int tmpVal = bufPtr[0] * var3;
		bufPtr[0] = tmpVal & 0xFF;
		}
		break;
	case '/': {
		if (var3 != 0)
			bufPtr[0] /= var3;
		}
		break;
	default: {
		warning("computeOperation : oper %d", oper);
		if (var3 != 0) {
			int tmpVal = bufPtr[0] / var3;
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
	if (_vm->_byte12A04 == _byte1855D)
		return;

	_byte1855D = _vm->_byte12A04;
	
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

void LilliputScript::sub1823E(byte index, byte var1, byte *curBufPtr) {
	debugC(1, kDebugScript, "sub1823E(%d, %d, curBufPtr)", index, var1);

	assert (index < 40);
	_characterScriptEnabled[index] = 1;
	curBufPtr[0] = var1;
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
		_array12311[(index * 16) + i] = (buf[(2 * i) + 1] << 8) + buf[2 * i];
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

void LilliputScript::sub17D40(bool &forceReturnFl) {
	debugC(1, kDebugScript, "sub17D40()");

	forceReturnFl = false;
	if ((_vm->_displayMap != 1) && (_vm->_characterRelativePositionX[_vm->_currentScriptCharacter] != 0xFF))
		return;

	forceReturnFl = true;
	return;
}

void LilliputScript::sub189F5() {
	debugC(2, kDebugScript, "sub189F5()");

	int index = 0;
	int var2 = 0x100;
	int var1;

	for (;;) {
		var1 = _vm->_displayStringBuf[index];
		if (var1 == 0)
			break;

		if (var1 == '|') {
			var2 &= 0xFF;
			++var2;
			continue;
		}

		var2 += 0x100;
		if ((var2 >> 8) < 61)
			continue;

		if ((var2 & 0xFF) == 1) {
			_vm->_displayStringBuf[index - 1] = 0;
			break;
		}

		--index;
		while (_vm->_displayStringBuf[index] != ' ')
			--index;

		_vm->_displayStringBuf[index] = '|';
		++var2;
		var2 &= 0xFF;
		++index;
	}
}

void LilliputScript::sub189B8() {
	debugC(2, kDebugScript, "sub189B8()");

	sub189F5();
	int index = 0;

	for (;;) {
		if (_vm->_displayStringBuf[index] == 0)
			break;
		++index;
	}

	index /= _byte1881E;
	index += 4;
	_byte1881D = index;
	_vm->displayFunction10();
	_vm->displayFunction11(_vm->_displayStringBuf);
}

void LilliputScript::sub18A56(byte *buf) {
	debugC(2, kDebugScript, "sub18A56(buf)");

	static const char *nounsArrayPtr = "I am |You are |you are |hou art |in the |is the |is a |in a |To the |to the |by |going |here |The|the|and |some |build|not |way|I |a |an |from |of |him|her|by |his |ing |tion|have |you|I''ve |can''t |up |to |he |she |down |what|What|with|are |and|ent|ian|ome|ed |me|my|ai|it|is|of|oo|ea|er|es|th|we|ou|ow|or|gh|go|er|st|ee|th|sh|ch|ct|on|ly|ng|nd|nt|ty|ll|le|de|as|ie|in|ss|''s |''t |re|gg|tt|pp|nn|ay|ar|wh|";

	_vm->_displayStringIndex = 0;
	_byte1881A = 0;
	int index = 0;
	byte var1 = 0;
	for (;;) {
		var1 = buf[index];
		++index;
		if (var1 == ']')
			var1 = 0;

		if (var1 < 0x80) {
			if (var1 == '@') {
				var1 = buf[index];
				++index;
				if (var1 == '#') {
					_vm->prepareGoldAmount(_byte18823);
				}
			} else {
				_vm->addCharToBuf(var1);
				if (var1 == 0)
					break;
			}
		} else {
			int nounIndex = 0;
			byte var3 = 0xFF - var1;
			for (int i = 0; i < var3; i++) {
				for (;;) {
					var1 = nounsArrayPtr[nounIndex];
					++nounIndex;

					if (var1 == '|')
						break;
				}
			}

			for (;;) {
				var1 = nounsArrayPtr[nounIndex];
				++nounIndex;

				if (var1 == '|')
					break;

				_vm->addCharToBuf(var1);
			}
		}
	}

	sub189B8();
}

int LilliputScript::sub18BB7(int index) {
	debugC(2, kDebugScript, "sub18BB7(%d)", index);
	
	int chunk4Index = _vm->_rulesChunk3[index];
	int result = 0;
	while (_vm->_rulesChunk4[chunk4Index + result] == 0x5B)
		++result;
	
	return result + 1;
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
		return _vm->_currentScriptCharacter;
	case 1002:
		return _word16F00;
	case 1003:
		return (int)_vm->_currentCharacterVariables[6];
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
		assert((_vm->_currentScriptCharacter >= 0) && (_vm->_currentScriptCharacter < 40));
		return ((_vm->_rulesBuffer2_13[_vm->_currentScriptCharacter] << 8) + _vm->_rulesBuffer2_14[_vm->_currentScriptCharacter]);
	case 0xFE: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return ((_vm->_rulesBuffer2_13[index] << 8) + _vm->_rulesBuffer2_14[index]);
		}
	case 0xFD:
		return _vm->_currentScriptCharacterPosition;
	case 0xFC: {
		int index = curWord & 0xFF;
		assert(index < 40);
		byte var1 = _vm->_characterPositionX[index] >> 3;
		byte var2 = _vm->_characterPositionY[index] >> 3;

		return (var1 << 8) + var2;
		}
	case 0xFB: {
		int index = _word16F00;
		assert(index < 40);
		byte var1 = _vm->_characterPositionX[index] >> 3;
		byte var2 = _vm->_characterPositionY[index] >> 3;

		return (var1 << 8) + var2;
		}
	case 0xFA:
		return ((_vm->_array10999[_vm->_currentScriptCharacter] << 8) + _vm->_array109C1[_vm->_currentScriptCharacter]);
	case 0xF9:
		return ((_vm->_currentCharacterVariables[4] << 8) + _vm->_currentCharacterVariables[5]);
	case 0xF8: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return _vm->_rulesBuffer12_3[index];
		}
	case 0xF7: {
		int index = _vm->_currentCharacterVariables[6];
		assert(index < 40);
		byte var1 = _vm->_characterPositionX[index] >> 3;
		byte var2 = _vm->_characterPositionY[index] >> 3;

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

byte *LilliputScript::getCharacterVariablePtr() {
	debugC(2, kDebugScript, "getCharacterVariablePtr()");

	int tmpVal = getValue1();
	tmpVal *= 32;
	tmpVal += _currScript->readUint16LE();

	assert(tmpVal < 40 * 32);
	return &_vm->_characterVariables[tmpVal];
}

byte LilliputScript::OC_sub173DF() {
	debugC(2, kDebugScript, "OC_sub173F0()");

	if (_vm->_currentScriptCharacterPosition == getValue2()) {
		return 1;
	}
	return 0;
}

byte LilliputScript::OC_sub173F0() {
	debugC(2, kDebugScript, "OC_sub173F0()");

	int index = getValue1();
	byte d1 = _array16123[index];
	byte d2 = _array1614B[index];
	int var1 = getValue2();

	if (var1 == (d1 << 8) + d2)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub1740A() {
	debugC(1, kDebugScript, "OC_sub1740A()");

	int var = _vm->_currentScriptCharacterPosition;
	if (var == 0xFFFF) {
		_currScript->readUint16LE();
		return 0;
	}

	byte *isoMapBuf = getMapPtr(var);
	int var2 = isoMapBuf[3];

	int var3 = _currScript->readUint16LE();
	int var4 = 8 >> var3;

	if( var2 & var4 ) {
		return 1;
	} else { 
		return 0;
	}
}

byte LilliputScript::OC_sub17434() {
	debugC(1, kDebugScript, "OC_sub17434()");

	byte *tmpArr = getCharacterVariablePtr();
	byte var1 = tmpArr[0];
	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readUint16LE();

	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_sub17468() {
	debugC(1, kDebugScript, "OC_sub17468()");

	int operation = _currScript->readUint16LE();
	int val2 = _currScript->readUint16LE();
	return compareValues(_byte16F02, operation, val2);
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

byte LilliputScript::OC_for() {
	debugC(1, kDebugScript, "OC_for()");

	int var1 = _currScript->readUint16LE();
	int tmpVal = _currScript->readUint16LE() + 1;
	// no need to seek later, the move is already done
	_currScript->writeUint16LE(tmpVal, -2);
	// overwrite the recently used "variable" in the script
	if (tmpVal < var1)
		return 0;

	_currScript->writeUint16LE(0, -2);
	return 1;
}

byte LilliputScript::OC_compWord18776() {
	debugC(1, kDebugScript, "OC_compWord18776()");

	int var1 = _currScript->readUint16LE();

	if (var1 == _word18776)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkSaveFlag() {
	debugC(1, kDebugScript, "OC_checkSaveFlag()");

	if (_vm->_saveFlag)
		return 1;

	return 0;
}

byte LilliputScript::OC_compByte16F04() {
	warning("OC_compByte16F04");

	byte var1 = _byte16F04;
	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readUint16LE();

	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_sub174D8() {
	debugC(1, kDebugScript, "OC_sub174D8()");

	byte tmpVal = getValue1() & 0xFF;
	int curWord = _currScript->readUint16LE();
	
	if (curWord == 3000) {
		int index;
		for (index = 0; index < _vm->_numCharacters; index++) {
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
	debugC(1, kDebugScript, "OC_sub1750E()");

	byte* buf1 = getCharacterVariablePtr();
	int var1 = *buf1;

	int operation = _currScript->readUint16LE();

	byte* buf2 = getCharacterVariablePtr();
	int var2 = *buf2;

	return compareValues(var1, operation, var2);
}

byte LilliputScript::OC_compareCoords_1() {
	debugC(1, kDebugScript, "OC_compareCoords_1()");

	int index = _currScript->readUint16LE();
	assert(index < 40);

	int var3 = _vm->_rulesBuffer12_1[index];
	int var4 = _vm->_rulesBuffer12_2[index];
	int var1 = _vm->_currentScriptCharacterPosition;

	if (((var1 >> 8) < (var3 >> 8)) || ((var1 >> 8) > (var3 & 0xFF)) || ((var1 & 0xFF) < (var4 >> 8)) || ((var1 & 0xFF) > (var4 & 0xFF)))
		return 0;

	return 1;
}

byte LilliputScript::OC_compareCoords_2() {
	debugC(1, kDebugScript, "OC_compareCoords_2()");
	int var1 = getValue1();
	var1 = (_array16123[var1] << 8) + _array1614B[var1];
	int var2 = _currScript->readUint16LE();
	int var3 = _vm->_rulesBuffer12_1[var2];
	int var4 = _vm->_rulesBuffer12_2[var2];

	if (((var1 >> 8) < (var3 >> 8)) || ((var1 >> 8) > (var3 & 0xFF)) || ((var1 & 0xFF) < (var4 >> 8)) || ((var1 & 0xFF) > (var4 & 0xFF)))
		return 0;
	return 1;
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
	if (tmpVal == _vm->_currentScriptCharacter)
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
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + index];
		if ((var1 & 0xFF) < var4)
			return 0;
		
		_word16F00 = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_numCharacters; i++) {
			int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
			if ((var1 & 0xFF) >= var4) {
				_word16F00 = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_numCharacters; i++) {
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
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
	debugC(1, kDebugScript, "OC_sub176C4()");

	int var4 = _currScript->readUint16LE();
	int index = _vm->_currentScriptCharacter * 40;
	int subIndex = 0xFFFF;

	int tmpVal = _currScript->readUint16LE();

	if (tmpVal >= 2000) {
		int var1 = tmpVal;

		if (var1 == 3000) {
			subIndex = 0;
			for (int i = 0; i < _vm->_numCharacters; i++) {
				tmpVal = _array10B51[index + i];
				byte v1 = tmpVal & 0xFF;
				byte v2 = tmpVal >> 8;
				if ((v1 >= (var4 & 0xFF)) && (v2 < (var4 & 0xFF))) {
					_word16F00 = subIndex;
					return 1;
				}
			}
			return 0;
		} else {
			var1 -= 2000;
			var4 &= ((var1 & 0xFF) << 8);
			for (int i = 0; i < _vm->_numCharacters; i++) {
				tmpVal = _array10B51[index + i];
				byte v1 = tmpVal & 0xFF;
				byte v2 = tmpVal >> 8;
				if ((v1 >= (var4 & 0xFF)) && (v2 < (var4 & 0xFF)) && (_vm->_rulesBuffer2_12[subIndex] != (var4 >> 8))) {
					_word16F00 = subIndex;
					return 1;
				}
			}
			return 0;
		}
	} else {
		_currScript->seek(_currScript->pos() - 2);
		subIndex = getValue1();
		tmpVal = _array10B51[index + subIndex];
		byte v1 = tmpVal & 0xFF;
		byte v2 = tmpVal >> 8;
		if ((v1 < (var4 & 0xFF)) || (v2 >= (var4 & 0xFF)))
			return 0;
		_word16F00 = subIndex;
		return 1;
	}
}

byte LilliputScript::OC_sub176C4() {
	debugC(1, kDebugScript, "OC_sub176C4()");
	
	byte var4 = _currScript->readUint16LE() & 0xFF;

	int tmpVal = _currScript->readUint16LE();
	
	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + index];
		if (((var1 & 0xFF) >= var4) || ((var1 >> 8) < var4))
			return 0;
		
		_word16F00 = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_numCharacters; i++) {
			int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
			if (((var1 & 0xFF) < var4) && ((var1 >> 8) >= var4)) {
				_word16F00 = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_numCharacters; i++) {
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
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
	debugC(1, kDebugScript, "OC_sub17766()");

	byte var1 = (_currScript->readUint16LE() & 0xFF);
	if ((var1 == _array12839[_vm->_currentScriptCharacter]) && (_array12811[_vm->_currentScriptCharacter] != 16))
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17782() {
	debugC(1, kDebugScript, "OC_sub17782()");

	byte var1 = (_currScript->readUint16LE() & 0xFF);
	if ((var1 == _array12839[_vm->_currentScriptCharacter]) && (_array12811[_vm->_currentScriptCharacter] == 16))
		return 1;

	return 0;
}

byte *LilliputScript::getMapPtr(int val) {
	debugC(1, kDebugScript, "getMapPtr(%d)", val);

	int x = (val >> 8);
	int y = (val & 0xFF);
	return &_vm->_bufferIsoMap[(y * 320 + x) << 2];
}

byte LilliputScript::OC_sub1779E() {
	debugC(1, kDebugScript, "OC_sub1779E()");

	int tmpVal = getValue2();

	if (tmpVal == 0xFFFF) {
		_currScript->seek(_currScript->pos() + 6);
		return 0;
	}
	int var2 = _currScript->readUint16LE();
	byte *buf = getMapPtr(tmpVal);
	byte var1 = buf[var2];
	var2 = _currScript->readUint16LE();
	int oper = _currScript->readUint16LE();
	
	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_sub177C6() {
	debugC(1, kDebugScript, "OC_sub177C6()");

	int index = getValue1();
	if (_vm->_characterPositionX[index] == 0xFFFF)
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

	assert(_vm->_currentCharacterVariables != NULL);

	if ((var1 == _vm->_currentCharacterVariables[0]) && (var2 == _vm->_currentCharacterVariables[1]))
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17812() {
	debugC(1, kDebugScript, "OC_sub17812()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[0] == curByte)
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
	debugC(1, kDebugScript, "OC_sub17844()");

	int tmpVal = _currScript->readUint16LE();

	if ((_vm->_byte16F07_menuId == 2) || ((tmpVal & 0xFF) != _byte12FE4))
		return 0;

	return 1;
}

byte LilliputScript::OC_sub1785C() {
	debugC(1, kDebugScript, "OC_sub1785C()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	int count = 0;

	for (int i = 0; i < _vm->_numCharacters; i++) {
		if (curByte == _vm->_characterVariables[(32 * i)])
			++count;
	}

	int oper = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();

	return compareValues(count, oper, var2);
}

byte LilliputScript::OC_sub17886() {
	debugC(1, kDebugScript, "OC_sub17886()");

	int var1 = getValue2();
	int x = var1 >> 8;
	int y = var1 & 0xFF;

	int dx = x - _viewportX;
	int dy = y - _viewportY;

	if ( dx >= 0 && dx < 8 && dy >= 0 && dy < 8)
		return 1;
	return 0;
}
byte LilliputScript::OC_sub178A8() {
	debugC(1, kDebugScript, "OC_sub178A8()");

	int var1 = getValue1();
	int var2 = getValue2();
	if (var1 == var2)
		return 1;
	return 0;
}
byte LilliputScript::OC_sub178BA() {
	_currScript->readUint16LE();
	return 1;
}

byte LilliputScript::OC_sub178C2() {
	debugC(1, kDebugScript, "OC_sub178C2()");

	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[2] == 1)
		return 1;
	return 0;
}

byte LilliputScript::OC_sub178D2() {
	debugC(1, kDebugScript, "OC_sub178D2()");

	int index = getValue1();
	assert (index < 40);

	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (curByte == _vm->_rulesBuffer2_12[index])
		return 1;

	return 0;
}

byte LilliputScript::OC_sub178E8() {
	debugC(1, kDebugScript, "OC_sub178E8()");

	byte *bufPtr = getCharacterVariablePtr();
	byte var1 = bufPtr[0];
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (var1 & curByte)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub178FC() {
	debugC(1, kDebugScript, "OC_sub178FC()");

	assert(_vm->_currentCharacterVariables != NULL);
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (curByte <= _vm->_currentCharacterVariables[0])
		return 1;
	return 0;
}

byte LilliputScript::OC_sub1790F() {
	debugC(1, kDebugScript, "OC_sub1790F()");

	int index = getValue1();
	assert(index < 40);
	if (_vm->_rulesBuffer2_5[index] == 0xFF)
		return 0;

	_word16F00 = _vm->_rulesBuffer2_5[index];

	return 1;
}

byte LilliputScript::OC_sub1792A() {
	debugC(1, kDebugScript, "OC_sub1792A()");

	assert(_vm->_currentCharacterVariables != NULL);
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (_vm->_currentCharacterVariables[1] == curByte)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub1793E() {
	debugC(1, kDebugScript, "OC_sub1793E()");

	if (_vm->_currentScriptCharacterPosition == 0xFFFF)
		return 0;

	if (_vm->_array16E94[_vm->_currentScriptCharacter] == 0)
		return 0;

	return 1;
}

byte LilliputScript::OC_sub1795E() {
	debugC(1, kDebugScript, "OC_sub1795E()");

	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[3] == 1)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkCharacterDirection() {
	debugC(1, kDebugScript, "OC_checkCharacterDirection()");

	int var1 = getValue1();
	int var2 = _currScript->readUint16LE();

	if (_vm->_characterDirectionArray[var1] == var2)
		return 1;
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
	debugC(1, kDebugScript, "OC_checkSavedMousePos()");

	if ((_byte129A0 != 0xFF) || (_vm->_savedMousePosDivided == 0xFFFF))
		return 0;

	return 1;
}

byte LilliputScript::OC_sub179AE() {
	debugC(1, kDebugScript, "OC_sub179AE()");

	if ((_vm->_byte12FCE == 1) || (_byte129A0 == 0xFF))
		return 0;

	return 1;
}

byte LilliputScript::OC_sub179C2() {
	debugC(1, kDebugScript, "OC_sub179C2()");
	int var1 = getValue2();

	if ((_vm->_array10999[_vm->_currentScriptCharacter] == (var1 >> 8))
		 && (_vm->_array109C1[_vm->_currentScriptCharacter] == (var1 & 0xFF)))
		return 1;

	return 0;
}
byte LilliputScript::OC_sub179E5() {
	debugC(1, kDebugScript, "OC_sub17A07()");

	static const byte _byte179DB[10] = {0x44, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43};

	int var1 = (_currScript->readUint16LE() & 0xFF) - 0x30;
	
	if (_byte179DB[var1] == _vm->_byte16F09)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17A07() {
	debugC(1, kDebugScript, "OC_sub17A07()");

	static const byte _array179FD[10] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	if (_vm->_byte16F07_menuId == 6) {
		int index = _currScript->readUint16LE();
		if (_array179FD[index] == _vm->_array147D1[0]) {
			index = _currScript->readUint16LE();
			if (_array179FD[index] == _vm->_array147D1[1]) {
				index = _currScript->readUint16LE();
				if (_array179FD[index] == _vm->_array147D1[2]) {
					return 1;
				}
			}
		}
	}
	
	_currScript->seek(_currScript->pos() + 6);
	return 0;
}

byte LilliputScript::OC_sub17757() {
	debugC(1, kDebugScript, "OC_sub17757()");

	int var1 = getValue1();
	if ( var1 == _viewportCharacterTarget )
		return 1;
	
	return 0;
}

void LilliputScript::OC_setWord18821() {
	debugC(1, kDebugScript, "OC_setWord18821()");
	_word18821 = getValue1();
}
void LilliputScript::OC_sub17A3E() {
	debugC(1, kDebugScript, "OC_sub17A3E()");
	int var1 = getValue2();
	int var2 = _currScript->readUint16LE();
	int var3 = _currScript->readUint16LE();

	byte* mapPtr = getMapPtr(var1);
	
	int mask = 8 >> var2;
	mask = ~mask;
	mapPtr[3] &= mask;

	if (var3 > 0) {
		mask = ~mask;
		mapPtr[3] |= mask;
	}
}

void LilliputScript::OC_sub17D57() {
	debugC(1, kDebugScript, "OC_sub17D57()");

	int curWord = _currScript->readUint16LE();

	bool forceReturnFl = false;
	sub17D40(forceReturnFl);
	if (forceReturnFl)
		return;

	_word1881B = _vm->_currentScriptCharacter;
	sub18B3C(curWord);

}

void LilliputScript::sub18B7C(int var1, int var3) {
	debugC(2, kDebugScript, "sub18B7C(%d, %d)", var1, var3);

	if (var1 == 0xFFFF) 
		return;

	_word18776 = var1;
	int index = _vm->_rulesChunk3[var1];

	while (_vm->_rulesChunk4[index] == 91)
		++index;

	for (int i = 0; i < var3; i++) {
		int tmpVal = 93;
		while (tmpVal == 93) {
			tmpVal = _vm->_rulesChunk4[index];
			++index;
		}
	}
	
	if (_vm->_rulesChunk4[index] == 0)
		return;

	sub18A56(&_vm->_rulesChunk4[index]);
}

void LilliputScript::OC_sub17D7F() {
	debugC(1, kDebugScript, "OC_sub17D7F()");

	int var1 = getCharacterVariablePtr()[0];
	int var2 = (_currScript->readUint16LE() & 0xFF);
	int var3 = var1 / var2;
	
	var1 = _currScript->readUint16LE();

	bool forceReturnFl = false;
	sub17D40(forceReturnFl);
	if (forceReturnFl)
		return;

	_word1881B = _vm->_currentScriptCharacter;

	sub18B7C(var1, var3);
}

void LilliputScript::OC_sub17DB9() {
	debugC(1, kDebugScript, "OC_sub17DB9()");

	int index = _currScript->readUint16LE();
	int maxValue = sub18BB7(index);
	
	int tmpVal = _currScript->readUint16LE() + 1;
	int oldVal = tmpVal;
	if (tmpVal >= maxValue)
		tmpVal = 0;
	_currScript->writeUint16LE(tmpVal, -2); 

	bool forceReturnFl = false;
	sub17D40(forceReturnFl);
	if (forceReturnFl)
		return;

	_word1881B = _vm->_currentScriptCharacter;

	sub18B7C(index, oldVal);
	
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
	debugC(1, kDebugScript, "OC_sub17E07()");

	if ((_word1881B & 0xFF) == 0xFF) {
		OC_sub17D7F();
		return;
	}
	_currScript->readUint16LE();
	_currScript->readUint16LE();
	_currScript->readUint16LE();
	_currScript->readUint16LE();

}
void LilliputScript::OC_sub17E15() {
	debugC(1, kDebugScript, "OC_sub17E15()");
	if ((_word1881B & 0xFF) == 0xFF) {
		OC_sub17DB9();
		return;
	}
	_currScript->readUint16LE();
	_currScript->readUint16LE();
}

void LilliputScript::OC_sub17B03() {
	debugC(1, kDebugScript, "OC_sub17B03()");

	byte *bufPtr = getCharacterVariablePtr();
	int oper = _currScript->readUint16LE();
	int var3 = _currScript->readUint16LE();

	computeOperation(bufPtr, oper, var3);
}

void LilliputScript::OC_getRandom_type2() {
	debugC(1, kDebugScript, "OC_getRandom_type2()");

	byte* bufPtr = getCharacterVariablePtr();
	int maxVal = _currScript->readUint16LE(); 
	int randomVal = _vm->_rnd->getRandomNumber(maxVal);
	*bufPtr = randomVal;
}

void LilliputScript::OC_setCharacterPosition() {
	debugC(1, kDebugScript, "OC_setCharacterPosition()");
	
	int index = getValue1();
	int tmpVal = getValue2();

	int var2 = ((tmpVal >> 8) << 3) + 4;
	int var4 = ((tmpVal & 0xFF) << 3) + 4;

	assert(index < 40);
	_vm->_characterPositionX[index] = var2;
	_vm->_characterPositionY[index] = var4;
}

void LilliputScript::OC_sub17A8D() {
	debugC(1, kDebugScript, "OC_sub17A8D()");

	int tmpVal = getValue1();
	assert(tmpVal < 40);

	if (tmpVal == _word10804)
		_viewportCharacterTarget = 0xFFFF;

	_vm->_characterPositionX[tmpVal] = 0xFFFF;
	_vm->_characterPositionY[tmpVal] = 0xFFFF;
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
	debugC(1, kDebugScript, "OC_resetByte1714E()");

	_vm->_byte1714E = 0;
}

void LilliputScript::OC_deleteSavegameAndQuit() {
	warning("OC_deleteSavegameAndQuit");
}
void LilliputScript::OC_incByte16F04() {
	debugC(1, kDebugScript, "OC_incByte16F04()");

	++_byte16F04;
}

void LilliputScript::OC_sub17BA5() {
	debugC(1, kDebugScript, "OC_sub17BA5()");
	
	byte *tmpArr = getCharacterVariablePtr();
	byte oper = (_currScript->readUint16LE() & 0xFF);
	byte var3 = getCharacterVariablePtr()[0];
	computeOperation(tmpArr, oper, var3);
}

void LilliputScript::OC_setByte18823() {
	debugC(1, kDebugScript, "OC_setByte18823()");
	byte *tmpArr = getCharacterVariablePtr();
	_byte18823 = *tmpArr;
}

void LilliputScript::OC_callScript() {
	debugC(1, kDebugScript, "OC_callScript()");

	int index = _currScript->readUint16LE();
	int var1 = getValue1();

	_vm->sub170EE(var1);
	int tmpIndex = _vm->_currentScriptCharacter;

	assert(index < _vm->_gameScriptIndexSize);
	int scriptIndex = _vm->_arrayGameScriptIndex[index];

	_scriptStack.push(_currScript);

	if (_byte16F05_ScriptHandler == 0) {
		_vm->_byte1714E = 0;
		runMenuScript(ScriptStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
	} else {
		runScript(ScriptStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
	}

	_currScript = _scriptStack.pop();

	_vm->sub170EE(tmpIndex);
}

void LilliputScript::OC_sub17BF2() {
	debugC(1, kDebugScript, "OC_sub17BF2()");

	OC_callScript();
	sub17B6C(0);
}

void LilliputScript::OC_sub17ACC() {
	debugC(1, kDebugScript, "OC_sub17ACC()");

	int var = getValue2();
	_vm->_array10999[_vm->_currentScriptCharacter] = var >> 8;
	_vm->_array109C1[_vm->_currentScriptCharacter] = var & 0xFF;
	_vm->_array109E9[_vm->_currentScriptCharacter] = 0xFF;
}

void LilliputScript::OC_resetByte16F04() {
	debugC(1, kDebugScript, "OC_resetByte16F04()");

	_byte16F04 = 0;
}

void LilliputScript::OC_sub17AE1() {
	debugC(1, kDebugScript, "OC_sub17AE1()");

	byte var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_currentScriptCharacter, var3);
}

void LilliputScript::OC_sub17AEE() {
	debugC(1, kDebugScript, "OC_sub17AEE()");

	byte var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_currentScriptCharacter + 1, var3);
}

void LilliputScript::OC_setWord10804() {
	debugC(1, kDebugScript, "OC_setWord10804()");

	_word10804 = getValue1();
}

void LilliputScript::OC_sub17C0E() {
	debugC(1, kDebugScript, "OC_sub17C0E()");

	assert(_vm->_currentCharacterVariables != NULL);
	byte var1 = (_vm->_currentCharacterVariables[4] << 8) + _vm->_currentCharacterVariables[5];
	byte b2 = _vm->_currentCharacterVariables[6];

	byte *mapPtr = getMapPtr(var1);
	mapPtr[b2] = _vm->_currentCharacterVariables[7];
	mapPtr[3] = _vm->_currentCharacterVariables[8];

	if (b2 == 0) {
		_byte12A09 = 1;
		_vm->displayFunction9();
		_byte12A09 = 0;
	}
}

void LilliputScript::OC_sub17C55() {
	debugC(1, kDebugScript, "OC_sub17C55()");
	int var1 = getValue1();
	int var2 = getValue1();

	int var3 = _currScript->readUint16LE();
	int var4 = _currScript->readUint16LE();

	_vm->_rulesBuffer2_5[var2] = var1 & 0xFF;
	_vm->_rulesBuffer2_6[var2] = var3 & 0xFF;
	_vm->_rulesBuffer2_7[var2] = var4 & 0xFF;
}
void LilliputScript::OC_sub17C76() {
	debugC(1, kDebugScript, "OC_sub17C76()");
	
	int var1 = getValue1();
	_vm->_rulesBuffer2_5[var1] = 0xFF;
	_vm->_characterPositionAltitude[var1] = 0;
	_characterScriptEnabled[var1] = 1;

}
void LilliputScript::OC_sub17AFC() {
	debugC(1, kDebugScript, "OC_sub17AFC()");
	int var1 = getValue1();
	_vm->sub170EE(var1);
}

void LilliputScript::sub171AF(int var1, int var2, int var4) {
	debugC(2, kDebugScript, "sub171AF()");

	int index = 0;
	for (int i = 0; i < 10; i++) {
		if (_vm->_array12861[index + 1] == 0xFFFF) {
			_vm->_array12861[index + 1] = var1;
			_vm->_array12861[index + 2] = var2;
			_vm->_array12861[index + 0] = _vm->_word1289D + var4;
		}
		index += 3;
	}
}

void LilliputScript::OC_sub17C8B() {
	debugC(1, kDebugScript, "OC_sub17C8B()");

	int var1 = 2 << 8;
	int var4 = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();
	var2 = ((var2 & 0xFF) << 8 ) + _vm->_currentScriptCharacter;
	
	sub171AF(var1, var2, var4);
}

void LilliputScript::OC_sub17CA2() {
	debugC(1, kDebugScript, "OC_sub17CA2()");

	int var1 = 1 << 8;
	int var4 = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();
	var2 = ((var2 & 0xFF) << 8 ) + _vm->_currentScriptCharacter;
	
	sub171AF(var1, var2, var4);
}

void LilliputScript::OC_sub17CB9() {
	debugC(1, kDebugScript, "OC_sub17CB9()");

	int var4 = _currScript->readUint16LE();
	int var1 = getValue1();
	int var2 = _currScript->readUint16LE();
	var2 = ((var2 & 0xFF) << 8 ) + _vm->_currentScriptCharacter;
	
	sub171AF(var1, var2, var4);
}

void LilliputScript::OC_sub17CD1() {
	debugC(1, kDebugScript, "OC_sub17CD1()");

	int var1 = 3 << 8;
	int var4 = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();
	var2 = ((var2 & 0xFF) << 8 ) + _vm->_currentScriptCharacter;
	
	sub171AF(var1, var2, var4);
}

void LilliputScript::OC_resetWord16EFE() {
	debugC(1, kDebugScript, "OC_resetWord16EFE()");

	_vm->_word16EFE = 0xFFFF;
}

void LilliputScript::OC_sub17CEF() {
	debugC(1, kDebugScript, "OC_sub17CEF()");

	int var1 = _currScript->readUint16LE();
	sub1823E(_vm->_currentScriptCharacter , var1, _vm->_currentCharacterVariables);
	sub17B6C(0);
}

void LilliputScript::OC_sub17D1B() {
	debugC(1, kDebugScript, "OC_sub17D1B()");

	assert(_vm->_currentCharacterVariables != NULL);
	++_vm->_currentCharacterVariables[1];
}

void LilliputScript::OC_sub17D23() {
	debugC(1, kDebugScript, "OC_sub17D23()");

	int var1 = _currScript->readUint16LE();
	int var2 = getValue2();
	byte* buf = _vm->_currentCharacterVariables + 4;
	computeOperation(buf, var1, var2 >> 8);
	computeOperation(buf + 1, var1, var2 & 0xFF);
}

void LilliputScript::OC_sub17E6D() {
	debugC(1, kDebugScript, "OC_sub17E6D()");
	
	int var1 = _currScript->readUint16LE();
	_vm->_rulesBuffer2_12[_vm->_currentScriptCharacter] = (var1 - 2000) & 0xFF;
}

void LilliputScript::OC_changeCurrentCharacterSprite() {
	debugC(2, kDebugScript, "OC_changeCurrentCharacterSprite()");

	int var1 = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();
	_vm->_characterFrameArray[_vm->_currentScriptCharacter] = var1;
	_vm->_spriteSizeArray[_vm->_currentScriptCharacter] = var2;

}

byte *LilliputScript::sub173D2() {
	debugC(2, kDebugScript, "sub173D2()");

	int index = _currScript->readUint16LE();	
	return &_vm->_currentCharacterVariables[index];
}

void LilliputScript::OC_sub17E99() {
	debugC(1, kDebugScript, "OC_sub17E99()");

	byte *compBuf = sub173D2();
	int oper = _currScript->readUint16LE();
	int index = _currScript->readUint16LE();	

	byte *buf = sub173D2();
	byte var1 = buf[0];
	byte var3 = _vm->_rulesChunk11[var1 + _vm->_rulesChunk10[index]];

	computeOperation(compBuf, oper, var3);
}

void LilliputScript::OC_sub17EC5() {
	//debugC(1, kDebugScript, "OC_sub17EC5()");
	warning("OC_sub17EC5");
	/*byte *compBuf = sub173D2();
	int oper = _currScript->readUint16LE();
	int index = _currScript->readUint16LE();	

	byte *buf = sub173D2();
	byte var1 = buf[0];
	byte var3 = _vm->_rulesChunk11[var1 + _vm->_rulesChunk10[index]];

	computeOperation(compBuf, oper, var3);*/
}

int LilliputScript::sub17285(int index) {
	return ((_vm->_characterPositionX[index] >> 3) << 8) + (_vm->_characterPositionY[index] >> 3);
}

void LilliputScript::OC_sub17EF4() {
	debugC(1, kDebugScript, "OC_sub17EF4()");

	int var1 = getValue2();
	int posTile = sub17285(_vm->_currentScriptCharacter);
	int dir = _vm->sub16B0C(posTile, var1);
	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = dir;
}

void LilliputScript::OC_sub17F08() {
	debugC(1, kDebugScript, "OC_sub17F08()");
	
	int index = getValue1();

	static const byte _directionsArray[] = { 0, 2, 0, 1, 3, 2, 3, 1 };

	int dx = _vm->_characterPositionX[index] - _vm->_characterDisplayX[_vm->_currentScriptCharacter];
	int dy = _vm->_characterPositionY[index] - _vm->_characterDisplayY[_vm->_currentScriptCharacter];

	int flag = 0;
	if (dx < 0) {
		dx = -dx;
		flag |= 4;
	}
	if (dy < 0) {
		dy = -dy;
		flag |= 2;
	} 
	if (dx < dy) {
		flag |= 1;
	}

	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = _directionsArray[flag];
}

void LilliputScript::OC_sub17F4F() {
	debugC(1, kDebugScript, "OC_sub17F4F()");
	int var = getValue1();
	_array10A39[_vm->_currentScriptCharacter] = var;
	_vm->_array109E9[_vm->_currentScriptCharacter] = 0xFF;
}

void LilliputScript::OC_sub17F68() {
	if (_vm->_currentScriptCharacter != _viewportCharacterTarget)
		return;

	static const byte _byte_17F60[] = { 0xFF, 0xFD, 0xFD, 0xFA };
	static const byte _byte_17F64[] = { 0xFD, 0xFA, 0xFF, 0xFD };
	
	int cx = _byte_17F60[_vm->_characterDirectionArray[_vm->_currentScriptCharacter]];
	int cy = _byte_17F64[_vm->_characterDirectionArray[_vm->_currentScriptCharacter]];

	int pos = sub17285(_vm->_currentScriptCharacter);

	int posx = pos >> 8;
	int posy = pos & 0xFF;

	int newPosX = posx + cx;
	int newPosY = posy + cy;

	if (newPosX < 0)
		newPosX = 0;

	if (newPosX > 56)
		newPosX = 56;

	if (newPosY < 0)
		newPosY = 0;

	if (newPosY > 56)
		newPosY = 56;

	_byte12A09 = 1;
	_vm->viewportScrollTo(newPosX, newPosY);
	_byte12A09 = 0;

}
void LilliputScript::OC_getNextVal() {
	debugC(1, kDebugScript, "OC_getNextVal()");
	 _currScript->readUint16LE();
}
void LilliputScript::OC_sub17FD2() {
	debugC(1, kDebugScript, "OC_sub17FD2()");
	
	int var1 = getValue1();
	_vm->_currentCharacterVariables[6] = var1 & 0xFF;

}

void LilliputScript::OC_sub17FDD() {
	debugC(1, kDebugScript, "OC_sub17FDD()");

	int index = _currScript->readUint16LE();
	
	int tmpVal = (_vm->_currentScriptCharacter * 32) + index;
	assert (tmpVal < 40 * 32);
	_array10AB1[_vm->_currentScriptCharacter] = _vm->_rulesBuffer2_16[tmpVal];
	_array12811[_vm->_currentScriptCharacter] = 16;
}

void LilliputScript::OC_setByte10B29() {
	debugC(1, kDebugScript, "OC_setByte10B29()");
	int var1 = getValue1();
	_characterScriptEnabled[var1] = 1;
}

void LilliputScript::OC_sub18007() {
	debugC(1, kDebugScript, "OC_sub18007()");

	int curWord = _currScript->readUint16LE();
	assert(_vm->_currentCharacterVariables != NULL);
	_vm->_currentCharacterVariables[2] = curWord & 0xFF;
}

void LilliputScript::OC_sub18014() {
	debugC(1, kDebugScript, "OC_sub18014()");

	assert(_vm->_currentCharacterVariables != NULL);
	_vm->_currentCharacterVariables[2] = 0;
}

void LilliputScript::OC_sub1801D() {
	debugC(1, kDebugScript, "OC_sub18014()");

	int var1 = getValue1();

	int x = _vm->_characterPositionX[var1] & 0xFFF8;
	x += _currScript->readUint16LE();
	_vm->_characterPositionX[var1] = x;

	int y = _vm->_characterPositionY[var1] & 0xFFF8;
	y += _currScript->readUint16LE();
	_vm->_characterPositionY[var1] = y;

	_vm->_characterPositionAltitude[var1]  = _currScript->readUint16LE();
	_vm->_characterDirectionArray[var1] = _currScript->readUint16LE();

}
void LilliputScript::OC_sub1805D() {
	debugC(1, kDebugScript, "OC_sub1805D()");
	
	int var1 = getValue1();
	
}

void LilliputScript::OC_sub18074() {
	debugC(1, kDebugScript, "OC_sub18074()");

	int var2 = _currScript->readUint16LE();
	byte var1 = (_currScript->readUint16LE() & 0xFF);

	_vm->_rulesBuffer2_16[(_vm->_currentScriptCharacter * 32) + var2] = var1;
}

void LilliputScript::OC_setCurrentCharacterDirection() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterDirection()");

	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = (_currScript->readUint16LE() & 0xFF);
}

void LilliputScript::OC_sub18099() {
	debugC(1, kDebugScript, "OC_sub18099()");

	int index = _currScript->readUint16LE();
	assert((index >= 0) && (index < 20));
	int curWord = _currScript->readUint16LE();

	_array122E9[index] = (curWord & 0xFF);
	_array122FD[index] = (curWord >> 8);

	_vm->displayFunction8();
}

void LilliputScript::OC_sub180C3() {
	debugC(1, kDebugScript, "OC_sub180C3()");
	_viewportCharacterTarget = 0xFFFF;

	int var1 = _currScript->readUint16LE();
	
	static const char _byte180B3[] = { 6, 0, 0, -6 };
	static const char _byte180BB[] = { 0, -6, 6, 0 };
	
	int x = _viewportX + _byte180B3[var1];
	int y = _viewportY + _byte180BB[var1];

	if ( x < 0 )
		x = 0;

	if ( y < 0 )
		y = 0;
	
	if ( x > 56 )
		x = 56;

	if ( y > 56 )
		y = 56;

	_byte12A09 = 1;
	_vm->viewportScrollTo(x, y);
	_byte12A09 = 0;
}

void LilliputScript::OC_sub1810A() {
	debugC(1, kDebugScript, "OC_sub1810A()");

	_viewportCharacterTarget = 0xFFFF;
	int var1 = getValue2();

	_viewportX = var1 >> 8;
	_viewportY = var1 & 0xFF;

	_vm->displayFunction9();
	_vm->displayFunction15();
}

void LilliputScript::OC_sub1812D() {
	debugC(1, kDebugScript, "OC_sub1812D()");

	_vm->_characterPositionAltitude[_vm->_currentScriptCharacter] = (_currScript->readUint16LE() & 0xFF);
}

void LilliputScript::OC_sub1817F() {
	debugC(1, kDebugScript, "OC_sub1817F()");

	int var1 = _currScript->readUint16LE();
	int var2 = _currScript->readUint16LE();
	
	int b1 = var1 & 0xFF;
	int b2 = var2 & 0xFF;
	sub1818B(b1,b2);
}

void LilliputScript::sub1818B(int b1, int b2) {
	debugC(2, kDebugScript, "sub1818B(%d, %d)", b1, b2);
	for (int i = 0; i <  _vm->_word1817B; i++) {
		if ((_array1813B[i] >> 8) == b2 ) {
			b2 += _array1813B[i] & 0xFF;
			if (b2 > 0xFF) {
				b2 = 0xFF;
				++b1;
			}
			_array1813B[i] = (b1 << 8) + b2;
			return;
		}
	}

	_array1813B[_vm->_word1817B++] = (b1 << 8) + b2;
}

void LilliputScript::OC_sub181BB() {
	debugC(1, kDebugScript, "OC_sub181BB()");
	
	int b = _currScript->readUint16LE();
	int d = _currScript->readUint16LE() & 0xFF;
	int s = _currScript->readUint16LE();
	int c = _vm->_currentCharacterVariables[s];
	int c2 = 0;

	if ( d == 0x2D ) {
		c = - 1 - c;
	} else if ( d == 0x3E ) {
		c = c - 0x80;
		if ( c < 0 ) 
			c = 0;
		c = c * 2;
	} else if ( d == 0x3C ) {
		c = -1 - c;
		c = c - 0x80;
		if ( c < 0 )
			c = 0;
		c = c * 2;
	}

	int a = _currScript->readUint16LE() * c + (c & 0xFF);
	b = b & 0xFF00 + a;
	sub1818B(b & 0xFF, b >> 8);

}

void LilliputScript::OC_sub18213() {
	debugC(1, kDebugScript, "OC_sub18213()");

	int var1 = _currScript->readUint16LE();

	int maxValue = 0;
	int maxItem = var1 & 0xFF;

	for (int i = 0; i < _vm->_word1817B; i++) {
		if ((_array1813B[i] & 0xFF) > maxValue) {
			maxValue = _array1813B[i] & 0xFF;
			maxItem = _array1813B[i] >> 8;
		}
	}
	sub1823E(_vm->_currentScriptCharacter, maxItem, _vm->_currentCharacterVariables);
}

void LilliputScript::OC_sub18252() {
	debugC(1, kDebugScript, "OC_sub18252()");

	int index = getValue1();
	assert(index < 40);

	_vm->_array12299[index] = 4;
}

void LilliputScript::OC_sub18260() {
	debugC(1, kDebugScript, "OC_sub18260()");
	
	int var1 = getValue1();
	int var2 = getValue2();

	int x = var1 + _viewportX;
	int y = var2 + _viewportY;

	byte* mapPtr = getMapPtr((x << 8) + (y & 0xff));

	int bx = 0;
	int by = 0;

	if (mapPtr[1] == 0xFF) {
		// TODO
	}
}

void LilliputScript::OC_sub182EC() {
	debugC(1, kDebugScript, "OC_sub182EC()");

	byte *tmpArr = getCharacterVariablePtr();

	byte var1 = (_currScript->readUint16LE() & 0xFF);
	byte var2 = (_currScript->readUint16LE() & 0xFF);

	if (var2 == 0)
		tmpArr[0] = (0xFF ^ var1);
	else
		tmpArr[0] |= var1;
}

void LilliputScript::OC_PaletteFadeOut() {
	debugC(1, kDebugScript, "OC_PaletteFadeOut()");

	_byte12A09 = 1;
	_vm->paletteFadeOut();
	_byte12A09 = 0;
}

void LilliputScript::OC_PaletteFadeIn() {
	debugC(1, kDebugScript, "OC_PaletteFadeIn()");

	_byte12A09 = 1;
	_vm->paletteFadeIn();
	_byte12A09 = 0;
}

void LilliputScript::OC_loadAndDisplayCUBESx_GFX() {
	debugC(1, kDebugScript, "OC_loadAndDisplayCUBESx_GFX()");

	int curWord = _currScript->readUint16LE();
	assert((curWord >= 0) && (curWord <= 9));
	Common::String fileName = Common::String::format("CUBES%d.GFX", curWord);
	_byte10806 = curWord + 0x30;

	_vm->_bufferCubegfx = _vm->loadVGA(fileName, false);
	_vm->displayFunction9();
	_vm->displayFunction15();
}

void LilliputScript::OC_sub1834C() {
	debugC(1, kDebugScript, "OC_sub1834C()");

	byte curWord = _currScript->readUint16LE() & 0xFF;
	assert(_vm->_currentCharacterVariables != NULL);

	_vm->_currentCharacterVariables[3] = curWord;
}

void LilliputScript::OC_setArray122C1() {
	debugC(1, kDebugScript, "OC_setArray122C1()");

	int var1 = _currScript->readUint16LE();
	_array122C1[_vm->_currentScriptCharacter] = var1;
}

void LilliputScript::OC_sub18367() {
	debugC(1, kDebugScript, "OC_sub18367()");

	_characterScriptEnabled[_vm->_currentScriptCharacter] = 1;
	_vm->_currentCharacterVariables[0] = _array122C1[_vm->_currentScriptCharacter];
	_vm->_currentCharacterVariables[1] = 0;
	_vm->_currentCharacterVariables[2] = 0;
	_vm->_currentCharacterVariables[3] = 0;
}

void LilliputScript::OC_sub17D04() {
	debugC(1, kDebugScript, "OC_sub17D04()");

	byte var1 = getValue1();
	byte var2 = _currScript->readUint16LE() & 0xFF;
	
	sub1823E(var1, var2, &_vm->_characterVariables[var1]);
}

void LilliputScript::OC_sub18387() {
	debugC(1, kDebugScript, "OC_sub18387()");

	int index = getValue1();
	byte var1 = _currScript->readUint16LE() & 0xFF;

	assert(index < 40);
	_vm->_rulesBuffer2_10[index] = var1;
}

void LilliputScript::OC_setDebugFlag() {
	debugC(1, kDebugScript, "OC_setDebugFlag()");

	_vm->_debugFlag = 1;
}

void LilliputScript::OC_setByte14837() {
	debugC(1, kDebugScript, "OC_setByte14837()");

	_vm->_byte14837 = 1;
}

void LilliputScript::OC_sub183A2() {
	warning("OC_sub183A2");
}
void LilliputScript::OC_sub183C6() {
	warning("OC_sub183C6");
}

void LilliputScript::OC_loadFile_AERIAL_GFX() {
	debugC(1, kDebugScript, "OC_loadFile_AERIAL_GFX()");
	
	int var1 = _currScript->readUint16LE() & 0xff;
	_vm->_byte15EAD = var1;

	_byte12A09 = 1;
	_word1881B = 0xFFFF;
	OC_PaletteFadeOut();
	_vm->_word15AC2 = 1;
	_vm->displayVGAFile("AERIAL.GFX");
	OC_PaletteFadeIn();

	_vm->displayCharactersOnMap();
	_vm->_byte16F08 = 1;
	_vm->_keyboard_oldIndex = 0;
	_vm->_keyboard_nextIndex = 0;

	_vm->_byte12A09 = 0;
}

void LilliputScript::OC_sub17E22() {
	warning("OC_sub17E22");
}
void LilliputScript::OC_sub1844A() {
	warning("OC_sub1844A");
}

void LilliputScript::OC_sub1847F() {
	debugC(1, kDebugScript, "OC_sub1847F()");

	byte *buf215Ptr = getCharacterVariablePtr();
	byte tmpVal = buf215Ptr[0];
	int curWord = _currScript->readUint16LE();
	assert(curWord != 0);
	int var1 = tmpVal / (curWord & 0xFF);
	int var2 = _currScript->readUint16LE();
	int var4 = _currScript->readUint16LE();

	if (_vm->_displayMap != 1) {
		_vm->displayFunction5();
		sub18BE6(var1 & 0xFF, var2, var4);
		_vm->displayFunction4();
	}
}

void LilliputScript::sub18BE6(byte var1, int var2, int var4) {
	debugC(1, kDebugScript, "sub18BE6(%d, %d, %d)", var1, var2, var4);

	_vm->_displayStringIndex = 0;
	_vm->_displayStringBuf[0] = 32;
	_vm->_displayStringBuf[1] = 32;
	_vm->_displayStringBuf[2] = 32;
	_vm->_displayStringBuf[3] = 0;

	_vm->prepareGoldAmount(var1);
	_vm->displayString(_vm->_displayStringBuf, var2, var4);
}

void LilliputScript::OC_displayVGAFile() {
	debugC(1, kDebugScript, "OC_displayVGAFile()");

	_byte12A09 = 1;
	OC_PaletteFadeOut();
	int curWord = _currScript->readUint16LE();
	int index = _vm->_rulesChunk3[curWord];
	Common::String fileName = Common::String((const char *)&_vm->_rulesChunk4[index]);
	_word1881B = 0xFFFF;
	_vm->displayVGAFile(fileName);
	OC_PaletteFadeIn();
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
		_vm->update();
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
	debugC(1, kDebugScript, "OC_sub1853B()");

	OC_PaletteFadeOut();
	_vm->_displayMap = 0;
	_heroismLevel = 0;
	sub130B6();

	_vm->displayFunction12();

	OC_PaletteFadeIn();
	_byte12A09 = 0;
	
	_vm->_soundHandler->contentFct5();
}

void LilliputScript::OC_sub1864D() {
	debugC(1, kDebugScript, "OC_sub1864D()");

	byte *tmpArr = getCharacterVariablePtr();
	int var1 = (_currScript->readUint16LE() & 0xFF);
	int var3 = ((70 * tmpArr[0]) / (_currScript->readUint16LE() & 0xFF) & 0xFF);
	int var2 = _currScript->readUint16LE();
	int var4 = _currScript->readUint16LE();
	
	_vm->displayFunction18(var1, var2, var3, var4);
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
	_savedBuffer215Ptr = getCharacterVariablePtr();
	_heroismBarX = _currScript->readUint16LE();
	_heroismBarBottomY = _currScript->readUint16LE();
}

void LilliputScript::OC_sub18690() {
	warning("OC_sub18690");
}

void LilliputScript::OC_setViewPortCharacterTarget() {
	debugC(1, kDebugScript, "OC_setViewPortCharacterTarget()");

	_viewportCharacterTarget = getValue1();
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
	byte var2h = (_viewportX & 0xFF);
	byte var2l = (_viewportY & 0xFF);
	int var1 = _currScript->readUint16LE();

	_vm->_soundHandler->contentFct2();
}

void LilliputScript::OC_sub1870A_snd() {
	debugC(1, kDebugScript, "OC_sub1870A_snd()");

	int var3 = getValue2();
	int var4 = var3;
	int var2 = (_viewportX << 8) + _viewportY;
	int var1 = (_currScript->readUint16LE() & 0xFF);

	_vm->_soundHandler->contentFct2();
}

void LilliputScript::OC_sub18725_snd() {
	debugC(1, kDebugScript, "OC_sub18725_snd()");

	int var4 = getValue1() | 0xFF00;

	_vm->_soundHandler->contentFct3();
}

void LilliputScript::OC_sub18733_snd() {
	debugC(1, kDebugScript, "OC_sub18733_snd()");

	int var4 = getValue2();

	_vm->_soundHandler->contentFct3();
}

void LilliputScript::OC_sub1873F_snd() {
	debugC(1, kDebugScript, "OC_sub1873F_snd()");

	_vm->_soundHandler->contentFct4();
}

void LilliputScript::OC_sub18746_snd() {
	debugC(1, kDebugScript, "OC_sub18746_snd()");

	int var4 = -1;
	int var2 = (_viewportX << 8) + _viewportY;
	int var1 = _currScript->readUint16LE() & 0xFF;

	_vm->_soundHandler->contentFct2();
}

void LilliputScript::OC_sub1875D_snd() {
	debugC(1, kDebugScript, "OC_sub1875D_snd()");

	_vm->_soundHandler->contentFct6();
}

void LilliputScript::OC_sub18764() {
	debugC(1, kDebugScript, "OC_sub18764()");

	int index = getValue1();
	int var1 = _currScript->readUint16LE();

	assert(index < 40);
	_array128EF[index] = var1 & 0xFF;
}

} // End of namespace
