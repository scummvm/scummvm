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

#ifndef LILLIPUT_SCRIPT_H
#define LILLIPUT_SCRIPT_H

#include "common/memstream.h"

namespace Lilliput {

class LilliputEngine;
class LilliputScript {
public:
	LilliputScript(LilliputEngine *vm);
	~LilliputScript();

	void runScript(Common::MemoryReadStream script);
private:
	LilliputEngine *_vm;

	byte _byte16F05_ScriptHandler;

	int handleOpcode(Common::MemoryReadStream script);
	byte handleOpcodeType1(int curWord);
	void handleOpcodeType2(int curWord);

	//Opcodes Type 1
	byte OC_sub173DF();
	byte OC_sub173F0();
	byte OC_sub1740A();
	byte OC_sub17434();
	byte OC_sub17468();
	byte OC_getRandom();
	byte OC_sub1748C();
	byte OC_compWord18776();
	byte OC_checkSaveFlag();
	byte OC_sub174C8();
	byte OC_sub174D8();
	byte OC_sub1750E();
	byte OC_compareCoords_1();
	byte OC_compareCoords_2();
	byte OC_sub1757C();
	byte OC_sub1759E();
	byte OC_compWord16EF8();
	byte OC_sub175C8();
	byte OC_sub17640();
	byte OC_sub176C4();
	byte OC_compWord10804();
	byte OC_sub17766();
	byte OC_sub17782();
	byte OC_sub1779E();
	byte OC_sub177C6();
	byte OC_compWord16EFE();
	byte OC_sub177F5();
	byte OC_sub17812();
	byte OC_sub17825();
	byte OC_sub17844();
	byte OC_sub1785C();
	byte OC_sub17886();
	byte OC_sub178A8();
	byte OC_sub178BA();
	byte OC_sub178C2();
	byte OC_sub178D2();
	byte OC_sub178E8();
	byte OC_sub178FC();
	byte OC_sub1790F();
	byte OC_sub1792A();
	byte OC_sub1793E();
	byte OC_sub1795E();
	byte OC_sub1796E();
	byte OC_sub17984();
	byte OC_checkSavedMousePos();
	byte OC_sub179AE();
	byte OC_sub179C2();
	byte OC_sub179E5();
	byte OC_sub17A07();
	byte OC_sub17757();

	// Opcodes Type 2
	void OC_setWord18821();
	void OC_sub17A3E();
	void OC_sub17D57();
	void OC_sub17D7F();
	void OC_sub17DB9();
	void OC_sub17DF9();
	void OC_sub17E07();
	void OC_sub17E15();
	void OC_sub17B03();
	void OC_getRandom_type2();
	void OC_sub17A66();
	void OC_sub17A8D();
	void OC_saveAndQuit();
	void OC_sub17B93();
	void OC_sub17E37();
	void OC_resetByte1714E();
	void OC_deleteSavegameAndQuit();
	void OC_incByte16F04();
	void OC_sub17BA5();
	void OC_setByte18823();
	void OC_sub17BB7();
	void OC_sub17BF2();
	void OC_sub17ACC();
	void OC_resetByte16F04();
	void OC_sub17AE1();
	void OC_sub17AEE();
	void OC_setWord10804();
	void OC_sub17C0E();
	void OC_sub17C55();
	void OC_sub17C76();
	void OC_sub17AFC();
	void OC_sub17C8B();
	void OC_sub17CA2();
	void OC_sub17CB9();
	void OC_sub17CD1();
	void OC_resetWord16EFE();
	void OC_sub17CEF();
	void OC_sub17D1B();
	void OC_sub17D23();
	void OC_sub17E6D();
	void OC_sub17E7E();
	void OC_sub17E99();
	void OC_sub17EC5();
	void OC_sub17EF4();
	void OC_sub17F08();
	void OC_sub17F4F();
	void OC_sub17F68();
	void OC_getNextVal();
	void OC_sub17FD2();
	void OC_sub17FDD();
	void OC_setByte10B29();
	void OC_sub18007();
	void OC_sub18014();
	void OC_sub1801D();
	void OC_sub1805D();
	void OC_sub18074();
	void OC_sub1808B();
	void OC_sub18099();
	void OC_sub180C3();
	void OC_sub1810A();
	void OC_sub1812D();
	void OC_sub1817F();
	void OC_sub181BB();
	void OC_sub18213();
	void OC_sub18252();
	void OC_sub18260();
	void OC_sub182EC();
	void OC_unkPaletteFunction_1();
	void OC_unkPaletteFunction_2();
	void OC_loadAndDisplayCUBESx_GFX();
	void OC_sub1834C();
	void OC_sub18359();
	void OC_sub18367();
	void OC_sub17D04();
	void OC_sub18387();
	void OC_setByte14835();
	void OC_setByte14837();
	void OC_sub183A2();
	void OC_sub183C6();
	void OC_loadFile_AERIAL_GFX();
	void OC_sub17E22();
	void OC_sub1844A();
	void OC_sub1847F();
	void OC_sub184AA();
	void OC_sub184D7();
	void OC_sub184F5();
	void OC_sub1853B();
	void OC_sub1864D();
	void OC_sub18608();
	void OC_sub18678();
	void OC_sub18690();
	void OC_setWord10802();
	void OC_sub186A1();
	void OC_sub186E5_snd();
	void OC_sub1870A_snd();
	void OC_sub18725_snd();
	void OC_sub18733_snd();
	void OC_sub1873F_snd();
	void OC_sub18746_snd();
	void OC_sub1875D_snd();
	void OC_sub18764();
};

} // End of namespace Lilliput

#endif

