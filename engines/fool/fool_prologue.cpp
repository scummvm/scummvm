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

#include <cstdint>
#include <cstdlib>

#include "fool/fool_prologue.h"

namespace Fool {

void FoolPrologue::run() {

/*
Found custom CODE block 128
0x0004: JSR - "ZBASIC_176"
0x0008: TRAP - _ClearMenuBar
0x000a: TRAP - _HideCursor
0x000c: PUSH.W - 0 (unsafe)
0x000e: TRAP - _CurResFile
0x0010: POP.W - D0
0x0012: EXT.L - D0
0x0014: MOVE.W - D0,VAR(0x0)
0x0018: PUSH.W - VAR(0x0)
0x001c: TRAP - _UseResFile
*/
	g_toolbox->ClearMenuBar();
	g_toolbox->HideCursor();

/*
0x001e: MOVEQ - 0x0,D0
0x0020: MOVE.W - D0,VAR(0x74)

0x0024: MOVE.W - VAR(0x74),D0
0x0028: EXT.L - D0
0x002a: LSL.L - 0x2,D0
0x002c: ADDI.L - HEAP(0x41296),D0
0x0036: PUSH.L - D0
0x0038: MOVE.W - VAR(0x74),D0
0x003c: EXT.L - D0
0x003e: MOVE.L - D0,D1
0x0040: MOVE.L - 0x2ac0,D0
0x0046: JSR - "MUL_INT"
0x004a: LSL.L - 0x1,D0
0x004c: ADDI.L - HEAP(0xb54),D0
0x0056: POP.L - A0
0x0058: MOVE.L - D0,(A0)
0x005a: MOVEQ - 0xb,D0
0x005c: ADDQ.W - 0x1,VAR(0x74)
0x0060: CMP.W - VAR(0x74),D0
0x0064: BGE - [0x0024]
*/
	// 128:001e
	// b54 must be the 1-bit screen double buffer?
	for (int i = 0; i < 12; i++) {
		this->arr_i32_41296[i] = &this->arr_i32_b54[SCREEN_PAGE_SIZE*i]

	}
/*
0x0066: MOVE.L - HEAP(0x412c2),D0
0x0070: MOVEA.L - D0,A0
0x0072: MOVE.L - (A0),D0
0x0074: MOVE.L - D0,VAR(0x4e)
*/
	// 128:0066
	this->var_i32_4e = this->arr_i32_412c2[0];
	/*
0x0078: MOVEQ - 0x0,D0
0x007a: MOVE.W - D0,VAR(0x74)

0x007e: MOVEQ - 0x0,D0
0x0080: MOVEA.W - VAR(0x74),A0
0x0084: ADD.L - A0,D0
0x0086: LSL.L - 0x1,D0
0x0088: ADDI.L - HEAP(0x194),D0
0x0092: MOVEA.L - D0,A0
0x0094: CLR.W - (A0)

0x0096: MOVEQ - 0x4,D0
0x0098: MOVEA.W - VAR(0x74),A0
0x009c: ADD.L - A0,D0
0x009e: LSL.L - 0x1,D0
0x00a0: ADDI.L - HEAP(0x194),D0
0x00aa: MOVEA.L - D0,A0
0x00ac: MOVE.W - 0xaa55,(A0)

0x00b0: MOVEQ - 0x8,D0
0x00b2: MOVEA.W - VAR(0x74),A0
0x00b6: ADD.L - A0,D0
0x00b8: LSL.L - 0x1,D0
0x00ba: ADDI.L - HEAP(0x194),D0
0x00c4: MOVEA.L - D0,A0
0x00c6: MOVEQ - -0x1,D0
0x00c8: MOVE.W - D0,(A0)

0x00ca: MOVEQ - 0x3,D0
0x00cc: ADDQ.W - 0x1,VAR(0x74)
0x00d0: CMP.W - VAR(0x74),D0
0x00d4: BGE - [0x007e]
*/
	// 128:0078
	// stipple pattern buffer?
	for (int i = 0; i < 3; i++) {
		this->arr_i16_194[i] = 0;
		this->arr_i16_194[i+4] = 0xaa55;
		this->arr_i16_194[i+8] = 0xffff;
	}

	/*
0x00d6: MOVE.L - HEAP(0x1ac),D0
0x00e0: MOVEA.L - D0,A0
0x00e2: MOVE.W - 0x107,(A0)

0x00e6: MOVEQ - 0xc,D0
0x00e8: ADDQ.L - 0x1,D0
0x00ea: LSL.L - 0x1,D0
0x00ec: ADDI.L - HEAP(0x194),D0
0x00f6: MOVEA.L - D0,A0
0x00f8: MOVE.W - 0xe0c,(A0)

0x00fc: MOVEQ - 0xc,D0
0x00fe: ADDQ.L - 0x2,D0
0x0100: LSL.L - 0x1,D0
0x0102: ADDI.L - HEAP(0x194),D0
0x010c: MOVEA.L - D0,A0
0x010e: MOVE.W - 0x7c70,(A0)

0x0112: MOVEQ - 0xc,D0
0x0114: ADDQ.L - 0x3,D0
0x0116: LSL.L - 0x1,D0
0x0118: ADDI.L - HEAP(0x194),D0
0x0122: MOVEA.L - D0,A0
0x0124: MOVE.W - 0xc0c0,(A0)

0x0128: MOVE.L - HEAP(0x1b4),D0
0x0132: MOVEA.L - D0,A0
0x0134: MOVE.W - 0x8800,(A0)

0x0138: MOVEQ - 0x10,D0
0x013a: ADDQ.L - 0x1,D0
0x013c: LSL.L - 0x1,D0
0x013e: ADDI.L - HEAP(0x194),D0
0x0148: MOVEA.L - D0,A0
0x014a: MOVE.W - 0x2200,(A0)

0x014e: MOVEQ - 0x10,D0
0x0150: ADDQ.L - 0x2,D0
0x0152: LSL.L - 0x1,D0
0x0154: ADDI.L - HEAP(0x194),D0
0x015e: MOVEA.L - D0,A0
0x0160: MOVE.W - 0x8800,(A0)

0x0164: MOVEQ - 0x10,D0
0x0166: ADDQ.L - 0x3,D0
0x0168: LSL.L - 0x1,D0
0x016a: ADDI.L - HEAP(0x194),D0
0x0174: MOVEA.L - D0,A0
0x0176: MOVE.W - 0x2200,(A0)

*/
	// 128:00d6
	this->arr_i16_1ac[0] = 0x0107;
	this->arr_i16_194[0xd] = 0x0e0c;
	this->arr_i16_194[0xe] = 0x7c70;
	this->arr_i16_194[0xf] = 0xc0c0;
	this->arr_i16_1b4[0] = 0x8800;
	this->arr_i16_194[0x11] = 0x2200;
	this->arr_i16_194[0x12] = 0x8800;
	this->arr_i16_194[0x13] = 0x2200;

	/*
0x017a: MOVEA.L - STR(0x0),A0  # b'\xa9 1987 by Cliff Johnson ALL RIGHTS RESERVED'
0x0182: JSR - "BUFFER_CONCAT"
0x0186: LEA - VAR(0x76),A0
0x018a: JSR - "BUFFER_FLUSH"
0x018e: MOVEA.L - STR(0x2c),A0  # b'This software was created using the ZBasic\xaa compiler.'
0x0196: JSR - "BUFFER_CONCAT"
0x019a: LEA - VAR(0x76),A0
0x019e: JSR - "BUFFER_FLUSH"
0x01a2: MOVEA.L - STR(0x62),A0  # b'Portions of this code are \xa9 Copyrighted, 1985 by Zedcor Inc.'
0x01aa: JSR - "BUFFER_CONCAT"
0x01ae: LEA - VAR(0x76),A0
0x01b2: JSR - "BUFFER_FLUSH"
*/
	//this->var_str_76 = STR(0x0);
	//this->var_str_76 = STR(0x2c);
	//this->var_str_76 = STR(0x62);
}

/*
0x01b6: JMP - [0x1f0]
--------
*/
/*
0x01ba: MOVE.W - D0,VAR(0x176)
0x01be: MOVE.W - VAR(0x176),D0
0x01c2: EXT.L - D0
0x01c4: LSL.L - 0x2,D0
0x01c6: ADDI.L - HEAP(0x41296),D0
0x01d0: MOVEA.L - D0,A0
0x01d2: MOVE.L - (A0),D0
0x01d4: MOVE.L - D0,VAR(0x40)
0x01d8: PEA - VAR(0x32)
0x01dc: PEA - VAR(0x40)
0x01e0: PEA - VAR(0x38)
0x01e4: PEA - VAR(0x38)
0x01e8: PUSH.W - 0
0x01ea: PUSH.L - 0
0x01ec: TRAP - _CopyBits
0x01ee: RTS
*/
void FoolPrologue::sub_128_1ba(int16_t screen_page) {
	this->var_40 = &this->arr_i32_41296[screen_page];
	g_toolbox->CopyBits(this->var_32, this->var_40, this->var_38, this->var_38, 0, 0);
}
	/*
--------
0x01f0: JMP - [0x21a]
--------
0x01f4: MOVE.W - D0,VAR(0x176)
0x01f8: MOVE.W - VAR(0x176),D0
0x01fc: EXT.L - D0
0x01fe: LSL.L - 0x2,D0
0x0200: ADDI.L - HEAP(0x41296),D0
0x020a: MOVEA.L - D0,A0
0x020c: MOVE.L - (A0),D0
0x020e: MOVE.L - D0,VAR(0x40)
0x0212: PEA - VAR(0x40)
0x0216: TRAP - _SetPortBits
0x0218: RTS
--------
*/
void FoolPrologue::sub_128_1f4(int16_t screen_page) {
	this->var_40 = &this->arr_i32_41296[screen_page];
	g_toolbox->SetPortBits(this->var_40);
}
/*

0x021a: JMP - [0x246]
--------
0x021e: MOVE.W - D0,VAR(0x178)
0x0222: PUSH.L - 0 (unsafe)
0x0224: TRAP - _TickCount
0x0226: POP.L - D0
0x0228: MOVE.L - D0,VAR(0x17a)
0x022c: PUSH.L - 0 (unsafe)
0x022e: TRAP - _TickCount
0x0230: POP.L - D0
0x0232: PUSH.L - D0
0x0234: MOVE.L - VAR(0x17a),D0
0x0238: MOVEA.W - VAR(0x178),A0
0x023c: ADD.L - A0,D0
0x023e: POP.L - D1
0x0240: CMP.L - D0,D1
0x0242: BLT - [0x022c]
0x0244: RTS
--------
*/
void FoolPrologue::sub_128_21e(int16_t numTicks) {
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	g_toolbox->Delay(numTicks);
}
/*

0x0246: JMP - [0x268]
--------
0x024a: MOVE.W - D0,VAR(0x17e)
0x024e: PUSH.L - 0 (unsafe)
0x0250: TRAP - _TickCount
0x0252: POP.L - D0
0x0254: PUSH.L - D0
0x0256: MOVE.L - VAR(0x2),D0
0x025a: MOVEA.W - VAR(0x17e),A0
0x025e: ADD.L - A0,D0
0x0260: POP.L - D1
0x0262: CMP.L - D0,D1
0x0264: BLE - [0x024e]
0x0266: RTS
--------
*/
void FoolPrologue::sub_128_24a(int16_t numTicks) {
	int32_t delay = MAX(this->var_i32_2 + numTicks + 1 - g_toolbox->TickCount(), 0);
	g_toolbox->Delay(delay);
}
/*
0x0268: JMP - [0x2a2]
--------
0x026c: MOVE.W - D0,VAR(0x180)
0x0270: POP.L - D0
0x0272: MOVE.W - D0,VAR(0x176)
0x0276: PUSH.W - 0 (unsafe)
0x0278: PEA - VAR(0x76)
0x027c: TRAP - _StringWidth
0x027e: POP.W - D0
0x0280: EXT.L - D0
0x0282: MOVE.W - D0,VAR(0x182)
0x0286: MOVE.W - VAR(0x176),D0
0x028a: EXT.L - D0
0x028c: MOVEA.W - VAR(0x182),A0
0x0290: SUB.L - A0,D0
0x0292: PUSH.W - D0
0x0294: PUSH.W - VAR(0x180)
0x0298: TRAP - _MoveTo
0x029a: PEA - VAR(0x76)
0x029e: TRAP - _DrawString
0x02a0: RTS
*/
void FoolPrologue::sub_128_26c(int16_t unk1, int16_t unk2) {
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(this->var_i32_182 - unk2, unk1);
	g_toolbox->DrawString(this->var_str_76);
}
/*
--------
0x02a2: JMP - [0x2ec]
--------
0x02a6: MOVE.W - D0,VAR(0x180)
0x02aa: POP.L - D0
0x02ac: MOVE.W - D0,VAR(0x176)
0x02b0: PUSH.W - 0 (unsafe)
0x02b2: PEA - VAR(0x76)
0x02b6: TRAP - _StringWidth
0x02b8: POP.W - D0
0x02ba: EXT.L - D0
0x02bc: MOVE.W - D0,VAR(0x182)
0x02c0: MOVE.W - VAR(0x176),D0
0x02c4: EXT.L - D0
0x02c6: PUSH.L - D0
0x02c8: MOVE.W - VAR(0x182),D0
0x02cc: EXT.L - D0
0x02ce: MOVE.L - D0,D1
0x02d0: MOVEQ - 0x2,D0
0x02d2: JSR - "DIV_INT"
0x02d6: POP.L - D1
0x02d8: EXG - D0,D1
0x02da: SUB.L - D1,D0
0x02dc: PUSH.W - D0
0x02de: PUSH.W - VAR(0x180)
0x02e2: TRAP - _MoveTo
0x02e4: PEA - VAR(0x76)
0x02e8: TRAP - _DrawString
0x02ea: RTS
--------
*/
void FoolPrologue::sub_128_2a6(int16_t unk1, int16_t unk2) {
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(unk2 - (this->var_i32_182 / 2), unk1);
	g_toolbox->DrawString(this->var_str_76);
}

/*

0x02ec: JMP - [0x350]
--------
0x02f0: MOVE.W - D0,VAR(0x186)
0x02f4: POP.L - D0
0x02f6: MOVE.W - D0,VAR(0x184)
0x02fa: POP.L - D0
0x02fc: MOVE.W - D0,VAR(0x182)
0x0300: POP.L - D0
0x0302: MOVE.W - D0,VAR(0x180)
0x0306: POP.L - D0
0x0308: MOVE.W - D0,VAR(0x176)
0x030c: MOVE.L - HEAP(0x41af4),D0
0x0316: PUSH.L - D0
0x0318: PUSH.W - VAR(0x180)
0x031c: PUSH.W - VAR(0x176)
0x0320: PUSH.W - VAR(0x184)
0x0324: PUSH.W - VAR(0x182)
0x0328: TRAP - _SetRect
0x032a: MOVE.L - HEAP(0x41af4),D0
0x0334: PUSH.L - D0
0x0336: MOVE.W - VAR(0x186),D0
0x033a: EXT.L - D0
0x033c: LSL.L - 0x2,D0
0x033e: LSL.L - 0x1,D0
0x0340: ADDI.L - HEAP(0x194),D0
0x034a: PUSH.L - D0
0x034c: TRAP - _FillRect
0x034e: RTS
--------
*/
void FoolPrologue::sub_128_2f0(int16_t unk, int16_t right, int16_t bottom, int16_t left, int16_t top) {
	g_toolbox->SetRect(this->rect_41af4, left, top, right, bottom);
	g_toolbox->FillRect(this->rect_41af4, &this->arr_i16_194[unk*4])
}
/*

0x0350: JMP - [0x3ea]
--------
0x0354: MOVE.W - D0,VAR(0x18a)
0x0358: POP.L - D0
0x035a: MOVE.W - D0,VAR(0x188)
0x035e: MOVE.L - HEAP(0x1bc),D0
0x0368: PUSH.L - D0
0x036a: PUSH.W - 0
0x036c: PUSH.W - 0
0x036e: PUSH.W - 0x200
0x0372: PUSH.W - 0x156
0x0376: TRAP - _SetRect

0x0378: MOVE.W - VAR(0x188),D0
0x037c: EXT.L - D0
0x037e: LSL.L - 0x2,D0
0x0380: LSL.L - 0x1,D0
0x0382: ADDI.L - HEAP(0x194),D0
0x038c: PUSH.L - D0
0x038e: TRAP - _PenPat

0x0390: PUSH.W - VAR(0x18a)
0x0394: TRAP - _PenMode
0x0396: MOVEQ - 0x6,D0
0x0398: PUSH.W - D0
0x039a: MOVEQ - 0x4,D0
0x039c: PUSH.W - D0
0x039e: TRAP - _PenSize

0x03a0: MOVEQ - 0x0,D0
0x03a2: MOVE.W - D0,VAR(0x74)
0x03a6: PUSH.L - 0 (unsafe)
0x03a8: TRAP - _TickCount
0x03aa: POP.L - D0
0x03ac: MOVE.L - D0,VAR(0x2)

0x03b0: MOVE.L - HEAP(0x1bc),D0
0x03ba: PUSH.L - D0
0x03bc: TRAP - _FrameRect
0x03be: MOVE.L - HEAP(0x1bc),D0
0x03c8: PUSH.L - D0
0x03ca: MOVEQ - 0x6,D0
0x03cc: PUSH.W - D0
0x03ce: MOVEQ - 0x4,D0
0x03d0: PUSH.W - D0
0x03d2: TRAP - _InsetRect
0x03d4: MOVEQ - 0x1,D0
0x03d6: BSR - [0x024a]
0x03da: ADDQ.W - 0x3,VAR(0x74)
0x03de: CMPI.W - 0x82,VAR(0x74)
0x03e4: BLE - [0x03a6]
0x03e6: TRAP - _PenNormal
0x03e8: RTS
--------
*/
void FoolPrologue::sub_128_354(int16_t unk1, int16_t unk2) {
	g_toolbox->SetRect(this->rect_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenPat(this->arr_i16_194[unk2*4]);
	g_toolbox->PenMode(unk1);
	g_toolbox->PenSize(6, 4);

	for (int i = 0; i < 130 i += 3) {
		this->var_i32_2 = g_toolbox->TickCount();

		g_toolbox->FrameRect(this->rect_1bc);
		g_toolbox->InsetRect(this->rect_1bc, 6, 4);
		this->sub_128_24a(1);
	}
	g_toolbox->PenNormal();
}

/*

0x03ea: JMP - [0x506]
--------
0x03ee: MOVE.W - D0,VAR(0x176)
0x03f2: MOVEQ - 0x5,D0
0x03f4: MOVE.W - D0,VAR(0x18c)
0x03f8: CMPI.W - 0x28,VAR(0x18e)
0x03fe: BLE - [0x0408]

0x0402: MOVEQ - 0x8,D0
0x0404: MOVE.W - D0,VAR(0x18c)
0x0408: CMPI.W - 0x50,VAR(0x18e)
0x040e: BLE - [0x0418]

0x0412: MOVEQ - 0xb,D0
0x0414: MOVE.W - D0,VAR(0x18c)
0x0418: CMPI.W - 0x64,VAR(0x18e)
0x041e: BLE - [0x0428]

0x0422: MOVEQ - 0xe,D0
0x0424: MOVE.W - D0,VAR(0x18c)

0x0428: MOVEQ - 0x1,D0
0x042a: MOVE.W - D0,VAR(0x74)
0x042e: PUSH.L - 0 (unsafe)
0x0430: TRAP - _TickCount
0x0432: POP.L - D0
0x0434: MOVE.L - D0,VAR(0x2)

0x0438: MOVEQ - 0x0,D0
0x043a: MOVEA.W - VAR(0x74),A0
0x043e: ADD.L - A0,D0
0x0440: LSL.L - 0x1,D0
0x0442: ADDI.L - HEAP(0x1e8),D0
0x044c: MOVEA.L - D0,A0
0x044e: MOVE.W - (A0),D0
0x0450: EXT.L - D0
0x0452: MOVEA.W - VAR(0x176),A0
0x0456: SUB.L - A0,D0
0x0458: PUSH.W - D0

0x045a: MOVE.L - 0x1f6,D0
0x0460: MOVEA.W - VAR(0x74),A0
0x0464: ADD.L - A0,D0
0x0466: LSL.L - 0x1,D0
0x0468: ADDI.L - HEAP(0x1e8),D0
0x0472: MOVEA.L - D0,A0
0x0474: MOVE.W - (A0),D0
0x0476: EXT.L - D0
0x0478: MOVEA.W - VAR(0x176),A0
0x047c: SUB.L - A0,D0
0x047e: PUSH.W - D0
0x0480: TRAP - _MoveTo

0x0482: MOVE.L - 0xfb,D0
0x0488: MOVEA.W - VAR(0x74),A0
0x048c: ADD.L - A0,D0
0x048e: LSL.L - 0x1,D0
0x0490: ADDI.L - HEAP(0x1e8),D0
0x049a: MOVEA.L - D0,A0
0x049c: MOVE.W - (A0),D0
0x049e: EXT.L - D0
0x04a0: MOVEA.W - VAR(0x176),A0
0x04a4: ADD.L - A0,D0
0x04a6: PUSH.W - D0

0x04a8: MOVE.L - 0x1f6,D0
0x04ae: MOVEA.W - VAR(0x74),A0
0x04b2: ADD.L - A0,D0
0x04b4: LSL.L - 0x1,D0
0x04b6: ADDI.L - HEAP(0x1e8),D0
0x04c0: MOVEA.L - D0,A0
0x04c2: MOVE.W - (A0),D0
0x04c4: EXT.L - D0
0x04c6: MOVEA.W - VAR(0x176),A0
0x04ca: SUB.L - A0,D0
0x04cc: PUSH.W - D0
0x04ce: TRAP - _LineTo

0x04d0: MOVE.W - VAR(0x74),D0
0x04d4: EXT.L - D0
0x04d6: MOVE.L - D0,D1
0x04d8: MOVE.W - VAR(0x18c),D0
0x04dc: EXT.L - D0
0x04de: JSR - "MOD_INT"
0x04e2: CMPI.L - 0x0,D0
0x04e8: BNE - [0x04f2]
0x04ec: MOVEQ - 0x0,D0
0x04ee: BSR - [0x024a]
0x04f2: MOVE.W - VAR(0x18e),D0
0x04f6: EXT.L - D0
0x04f8: ADDQ.W - 0x1,VAR(0x74)
0x04fc: CMP.W - VAR(0x74),D0
0x0500: BGE - [0x042e]

0x0504: RTS
--------
*/
void FoolPrologue::sub_128_3ee(int16_t unk1) {
	this->var_18c = 0x5;
	if (this->var_i16_18e > 0x28) {
		this->var_i16_18c = 0x8;
	}
	if (this->var_i16_18e > 0x50) {
		this->var_i16_18c = 0xb;
	}
	if (this->var_i16_18e > 0x64) {
		this->var_i16_18c = 0xe;
	}
	for (int i = 1; i < this->var_i16_18e + 1; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(
			unk1 - this->arr_i16_1e8[i],
			unk1 - this->arr_i16_1e8[i + 0x1f6],
		);
		g_toolbox->LineTo(
			unk1 + this->arr_i16_1e8[i + 0xfb],
			unk1 - this->arr_i16_1e8[i + 0x1f6],
		);
		if (i % this->var_i16_18c == 0) {
			this->sub_128_24a(0);
		};
	}

}
/*

0x0506: JMP - [0x60c]
--------
0x050a: MOVE.W - D0,VAR(0x184)
0x050e: POP.L - D0
0x0510: MOVE.W - D0,VAR(0x182)
0x0514: POP.L - D0
0x0516: MOVE.W - D0,VAR(0x180)
0x051a: POP.L - D0
0x051c: MOVE.W - D0,VAR(0x176)
0x0520: MOVE.W - VAR(0x176),D0
0x0524: EXT.L - D0
0x0526: LSL.L - 0x2,D0
0x0528: ADDI.L - HEAP(0x41296),D0
0x0532: MOVEA.L - D0,A0
0x0534: MOVE.L - (A0),D0
0x0536: MOVE.L - D0,VAR(0x40)

0x053a: MOVE.W - VAR(0x180),VAR(0x5e)
0x0540: MOVE.W - VAR(0x182),VAR(0x62)
0x0546: MOVE.W - VAR(0x184),D0
0x054a: BNE - [0x0554]
0x054e: MOVEQ - 0x14,D0
0x0550: MOVE.W - D0,VAR(0x184)

0x0554: CMPI.W - 0x1,VAR(0x184)
0x055a: BNE - [0x0564]
0x055e: MOVEQ - 0x8,D0
0x0560: MOVE.W - D0,VAR(0x184)

0x0564: CMPI.W - 0x2,VAR(0x184)
0x056a: BNE - [0x0574]
0x056e: MOVEQ - 0x6,D0
0x0570: MOVE.W - D0,VAR(0x184)

0x0574: CMPI.W - 0x3,VAR(0x184)
0x057a: BNE - [0x0584]
0x057e: MOVEQ - 0x5,D0
0x0580: MOVE.W - D0,VAR(0x184)

0x0584: MOVEQ - 0x1,D0
0x0586: MOVE.W - D0,VAR(0x190)
0x058a: PUSH.L - 0 (unsafe)
0x058c: TRAP - _TickCount
0x058e: POP.L - D0
0x0590: MOVE.L - D0,VAR(0x2)

0x0594: MOVEA.W - VAR(0x190),A0
0x0598: ADDA.L - A0,A0
0x059a: ADDA.L - HEAP(0x412ea),A0
0x05a4: MOVE.W - (A0),D0
0x05a6: EXT.L - D0
0x05a8: SUBQ.L - 0x1,D0
0x05aa: MOVE.W - D0,VAR(0x5c)

0x05ae: MOVEA.W - VAR(0x190),A0
0x05b2: ADDA.L - A0,A0
0x05b4: ADDA.L - HEAP(0x412ea),A0
0x05be: MOVE.W - (A0),D0
0x05c0: EXT.L - D0
0x05c2: MOVE.W - D0,VAR(0x60)

0x05c6: PEA - VAR(0x40)
0x05ca: PEA - VAR(0x32)
0x05ce: PEA - VAR(0x5c)
0x05d2: PEA - VAR(0x5c)
0x05d6: PUSH.W - 0
0x05d8: PUSH.L - 0
0x05da: TRAP - _CopyBits

0x05dc: MOVE.W - VAR(0x190),D0
0x05e0: EXT.L - D0
0x05e2: MOVE.L - D0,D1
0x05e4: MOVE.W - VAR(0x184),D0
0x05e8: EXT.L - D0
0x05ea: JSR - "MOD_INT"

0x05ee: CMPI.L - 0x0,D0
0x05f4: BNE - [0x05fe]
0x05f8: MOVEQ - 0x0,D0
0x05fa: BSR - [0x024a]
0x05fe: ADDQ.W - 0x1,VAR(0x190)
0x0602: CMPI.W - 0x156,VAR(0x190)
0x0608: BLE - [0x058a]

0x060a: RTS
--------
*/
void FoolPrologue::sub_128_50a(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4) {
	this->var_i32_40 = this->arr_i32_41296[unk4];
	this->var_i16_5e = unk3;
	this->var_i16_62 = unk2;
	if (unk1 == 0) {
		unk1 = 0x14;
	}
	if (unk1 == 1) {
		unk1 = 0x8;
	}
	if (unk1 == 2) {
		unk1 = 0x6;
	}
	if (unk1 == 3) {
		unk1 = 0x5;
	}
	for (int i = 1; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c = this->arr_i16_412ea[i] - 1;
		this->var_i16_60 = this->arr_i16_412ea[i];
		g_toolbox->CopyBits(this->var_i32_40, this->var_32, this->var_i16_5c, this->var_i16_5c, 0, 0);
		if (i % unk1 == 0) {
			this->sub_128_24a(0);
		}
	}
}

/*

0x060c: JMP - [0x646]
--------
0x0610: MOVE.W - D0,VAR(0x176)
0x0614: MOVE.W - VAR(0x176),D0
0x0618: EXT.L - D0
0x061a: LSL.L - 0x2,D0
0x061c: ADDI.L - HEAP(0x41296),D0
0x0626: MOVEA.L - D0,A0
0x0628: MOVE.L - (A0),D0
0x062a: MOVE.L - D0,VAR(0x40)
0x062e: PEA - VAR(0x40)
0x0632: PEA - VAR(0x32)
0x0636: PEA - VAR(0x38)
0x063a: PEA - VAR(0x38)
0x063e: PUSH.W - 0
0x0640: PUSH.L - 0
0x0642: TRAP - _CopyBits
0x0644: RTS
--------
*/
void FoolPrologue::sub_128_610(int16_t unk) {
	this->var_i32_40 = this->arr_i32_41296[unk];
	g_toolbox->CopyBits(this->var_i32_40, this->var_32, this->var_38, this->var_38, 0, 0);
}

/*

0x0646: JMP - [0x6e0]
--------
0x064a: MOVE.W - D0,VAR(0x176)
0x064e: TRAP - _PenNormal
0x0650: MOVE.W - VAR(0x176),D0
0x0654: EXT.L - D0
0x0656: LSL.L - 0x2,D0
0x0658: LSL.L - 0x1,D0
0x065a: ADDI.L - HEAP(0x194),D0
0x0664: PUSH.L - D0
0x0666: TRAP - _PenPat

0x0668: MOVEQ - 0x0,D0
0x066a: MOVE.W - D0,VAR(0x192)

0x066e: PUSH.L - 0 (unsafe)
0x0670: TRAP - _TickCount
0x0672: POP.L - D0
0x0674: MOVE.L - D0,VAR(0x2)

0x0678: PUSH.W - 0
0x067a: MOVEA.W - VAR(0x192),A0
0x067e: ADDA.L - A0,A0
0x0680: ADDA.L - HEAP(0x412ea),A0
0x068a: MOVE.W - (A0),D0
0x068c: EXT.L - D0
0x068e: SUBQ.L - 0x1,D0
0x0690: PUSH.W - D0
0x0692: TRAP - _MoveTo

0x0694: PUSH.W - 0x200
0x0698: MOVEA.W - VAR(0x192),A0
0x069c: ADDA.L - A0,A0
0x069e: ADDA.L - HEAP(0x412ea),A0
0x06a8: MOVE.W - (A0),D0
0x06aa: EXT.L - D0
0x06ac: SUBQ.L - 0x1,D0
0x06ae: PUSH.W - D0
0x06b0: TRAP - _LineTo

0x06b2: MOVE.W - VAR(0x192),D0
0x06b6: EXT.L - D0
0x06b8: MOVE.L - D0,D1
0x06ba: MOVEQ - 0x5,D0
0x06bc: JSR - "MOD_INT"
0x06c0: CMPI.L - 0x0,D0
0x06c6: BNE - [0x06d0]
0x06ca: MOVEQ - 0x0,D0
0x06cc: BSR - [0x024a]
0x06d0: ADDQ.W - 0x1,VAR(0x192)
0x06d4: CMPI.W - 0x156,VAR(0x192)
0x06da: BLE - [0x066e]
0x06dc: TRAP - _PenNormal
0x06de: RTS
--------
*/
void FoolPrologue::sub_128_64a(int16_t unk1) {
	g_toolbox->PenNormal();
	g_toolbox->PenPat(&this->arr_i16_194[unk1*4]);
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(0, this->arr_i16_412ea[i] - 1);
		g_toolbox->LineTo(SCREEN_WIDTH, this->arr_i16_412ea[i] - 1);
		if (i % 5 == 0) {
			this->sub_128_24a(0);
		}
	}
	g_toolbox->PenNormal();
}

/*

0x06e0: JMP - [0x7fc]
--------
0x06e4: MOVE.W - D0,VAR(0x176)
0x06e8: MOVE.W - VAR(0x176),D0
0x06ec: EXT.L - D0
0x06ee: LSL.L - 0x2,D0
0x06f0: ADDI.L - HEAP(0x41296),D0
0x06fa: MOVEA.L - D0,A0
0x06fc: MOVE.L - (A0),D0
0x06fe: MOVE.L - D0,VAR(0x40)

0x0702: MOVEQ - 0x1,D0
0x0704: MOVE.W - D0,VAR(0x74)

0x0708: PUSH.L - 0 (unsafe)
0x070a: TRAP - _TickCount
0x070c: POP.L - D0
0x070e: MOVE.L - D0,VAR(0x2)

0x0712: MOVE.L - 0xab,D0
0x0718: PUSH.L - D0
0x071a: MOVE.W - VAR(0x74),D0
0x071e: EXT.L - D0
0x0720: JSR - "BCD_FROM_INT"
0x0724: JSR - "BCD_PUSH_LHS"
0x0728: JSR - "BCD_FROM_CONST" 3.33
0x0732: JSR - "BCD_PUSH_RHS"
0x0736: JSR - "MUL_BCD"
0x073a: JSR - "CAST_INT"
0x073e: POP.L - D1
0x0740: EXG - D0,D1
0x0742: SUB.L - D1,D0
0x0744: MOVE.W - D0,VAR(0x5c)

0x0748: MOVE.L - 0x100,D0
0x074e: PUSH.L - D0
0x0750: MOVE.W - VAR(0x74),D0
0x0754: EXT.L - D0
0x0756: MOVE.L - D0,D1
0x0758: MOVEQ - 0x5,D0
0x075a: JSR - "MUL_INT"
0x075e: POP.L - D1
0x0760: EXG - D0,D1
0x0762: SUB.L - D1,D0
0x0764: MOVE.W - D0,VAR(0x5e)

0x0768: MOVE.L - 0xab,D0
0x076e: PUSH.L - D0
0x0770: MOVE.W - VAR(0x74),D0
0x0774: EXT.L - D0
0x0776: JSR - "BCD_FROM_INT"
0x077a: JSR - "BCD_PUSH_LHS"
0x077e: JSR - "BCD_FROM_CONST" 3.33
0x0788: JSR - "BCD_PUSH_RHS"
0x078c: JSR - "MUL_BCD"
0x0790: JSR - "CAST_INT"
0x0794: POP.L - D1
0x0796: ADD.L - D1,D0
0x0798: MOVE.W - D0,VAR(0x60)

0x079c: MOVE.L - 0x100,D0
0x07a2: PUSH.L - D0
0x07a4: MOVE.W - VAR(0x74),D0
0x07a8: EXT.L - D0
0x07aa: MOVE.L - D0,D1
0x07ac: MOVEQ - 0x5,D0
0x07ae: JSR - "MUL_INT"
0x07b2: POP.L - D1
0x07b4: ADD.L - D1,D0
0x07b6: MOVE.W - D0,VAR(0x62)

0x07ba: PEA - VAR(0x40)
0x07be: PEA - VAR(0x32)
0x07c2: PEA - VAR(0x5c)
0x07c6: PEA - VAR(0x5c)
0x07ca: PUSH.W - 0
0x07cc: PUSH.L - 0
0x07ce: TRAP - _CopyBits

0x07d0: MOVEQ - 0x1,D0
0x07d2: BSR - [0x024a]
0x07d6: MOVEQ - 0x34,D0
0x07d8: ADDQ.W - 0x1,VAR(0x74)
0x07dc: CMP.W - VAR(0x74),D0
0x07e0: BGE - [0x0708]

0x07e4: PEA - VAR(0x40)
0x07e8: PEA - VAR(0x32)
0x07ec: PEA - VAR(0x38)
0x07f0: PEA - VAR(0x38)
0x07f4: PUSH.W - 0
0x07f6: PUSH.L - 0
0x07f8: TRAP - _CopyBits
0x07fa: RTS
--------
*/
void FoolPrologue::sub_128_6e4(int16_t unk) {
	this->var_40 = this->arr_i32_41296[unk];
	for (int i = 0; i < 0x36; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c = (SCREEN_HEIGHT/2) - (int)(i*3.33);
		this->var_i16_5e = (SCREEN_WIDTH/2) - i*5;
		this->var_i16_60 = (SCREEN_HEIGHT/2) + (int)(i*3.33);
		this->var_i16_62 = (SCREEN_WIDTH/2) + i*5;
		g_toolbox->CopyBits(this->var_40, this->var_32, this->var_i16_5c, this->var_i16_5c, 0, 0);
	}
	g_toolbox->CopyBits(this->var_40, this->var_32, this->var_38, this->var_38, 0, 0);
}

/*

0x07fc: JMP - [0xa68]
--------
0x0800: MOVE.W - D0,VAR(0x1a2)
0x0804: POP.L - D0
0x0806: MOVE.W - D0,VAR(0x1a0)
0x080a: POP.L - D0
0x080c: MOVE.W - D0,VAR(0x19e)
0x0810: POP.L - D0
0x0812: MOVE.W - D0,VAR(0x19c)
0x0816: POP.L - D0
0x0818: MOVE.W - D0,VAR(0x19a)
0x081c: POP.L - D0
0x081e: MOVE.W - D0,VAR(0x198)
0x0822: POP.L - D0
0x0824: MOVE.W - D0,VAR(0x196)
0x0828: POP.L - D0
0x082a: MOVE.W - D0,VAR(0x194)
0x082e: POP.L - D0
0x0830: MOVE.W - D0,VAR(0x178)

0x0834: MOVE.L - HEAP(0x41af4),D0
0x083e: MOVEA.L - D0,A0
0x0840: MOVE.W - VAR(0x178),(A0)

0x0844: MOVE.L - HEAP(0x41af6),D0
0x084e: MOVEA.L - D0,A0
0x0850: MOVE.W - VAR(0x194),(A0)

0x0854: MOVE.L - HEAP(0x41af8),D0
0x085e: MOVEA.L - D0,A0
0x0860: MOVE.W - VAR(0x196),(A0)

0x0864: MOVE.L - HEAP(0x41afa),D0
0x086e: MOVEA.L - D0,A0
0x0870: MOVE.W - VAR(0x198),(A0)

0x0874: MOVE.L - HEAP(0x41afc),D0
0x087e: MOVEA.L - D0,A0
0x0880: MOVE.W - VAR(0x19a),(A0)

0x0884: MOVE.L - HEAP(0x41afe),D0
0x088e: MOVEA.L - D0,A0
0x0890: MOVE.W - VAR(0x19c),(A0)

0x0894: MOVE.L - HEAP(0x41b00),D0
0x089e: MOVEA.L - D0,A0
0x08a0: MOVE.W - VAR(0x19e),(A0)

0x08a4: MOVE.L - HEAP(0x41b02),D0
0x08ae: MOVEA.L - D0,A0
0x08b0: MOVE.W - VAR(0x1a0),(A0)

0x08b4: TRAP - _PenNormal
0x08b6: MOVEQ - 0xa,D0
0x08b8: PUSH.W - D0
0x08ba: TRAP - _PenMode

0x08bc: MOVEQ - 0x0,D0
0x08be: MOVE.W - D0,VAR(0x74)
0x08c2: MOVE.W - VAR(0x74),D0
0x08c6: EXT.L - D0
0x08c8: LSL.L - 0x2,D0
0x08ca: ADDI.L - HEAP(0x41bbe),D0
0x08d4: PUSH.L - D0

0x08d6: MOVEA.W - VAR(0x74),A0
0x08da: ADDA.L - A0,A0
0x08dc: ADDA.L - HEAP(0x41af4),A0
0x08e6: MOVE.W - (A0),D0
0x08e8: EXT.L - D0
0x08ea: JSR - "BCD_FROM_INT"
0x08ee: POP.L - A0
0x08f0: JSR - "BCD_TO_VAR"

0x08f4: MOVE.W - VAR(0x74),D0
0x08f8: EXT.L - D0
0x08fa: ADDQ.L - 0x4,D0
0x08fc: LSL.L - 0x2,D0
0x08fe: ADDI.L - HEAP(0x41bbe),D0
0x0908: PUSH.L - D0

0x090a: MOVE.W - VAR(0x74),D0
0x090e: EXT.L - D0
0x0910: ADDQ.L - 0x4,D0
0x0912: LSL.L - 0x1,D0
0x0914: ADDI.L - HEAP(0x41af4),D0
0x091e: MOVEA.L - D0,A0
0x0920: MOVE.W - (A0),D0
0x0922: EXT.L - D0
0x0924: PUSH.L - D0
0x0926: MOVEA.W - VAR(0x74),A0
0x092a: ADDA.L - A0,A0
0x092c: ADDA.L - HEAP(0x41af4),A0
0x0936: MOVE.W - (A0),D0
0x0938: EXT.L - D0
0x093a: POP.L - D1

0x093c: EXG - D0,D1
0x093e: SUB.L - D1,D0
0x0940: JSR - "BCD_FROM_INT"
0x0944: JSR - "BCD_PUSH_LHS"
0x0948: MOVE.W - VAR(0x1a2),D0
0x094c: EXT.L - D0
0x094e: JSR - "BCD_FROM_INT"
0x0952: JSR - "BCD_PUSH_RHS"
0x0956: JSR - "DIV_BCD"
0x095a: POP.L - A0
0x095c: JSR - "BCD_TO_VAR"
0x0960: MOVEQ - 0x3,D0
0x0962: ADDQ.W - 0x1,VAR(0x74)
0x0966: CMP.W - VAR(0x74),D0
0x096a: BGE - [0x08c2]

0x096e: MOVE.L - HEAP(0x41af4),D0
0x0978: PUSH.L - D0
0x097a: TRAP - _PaintRect

0x097c: MOVEQ - 0x1,D0
0x097e: MOVE.W - D0,VAR(0x192)
0x0982: PUSH.L - 0 (unsafe)
0x0984: TRAP - _TickCount
0x0986: POP.L - D0
0x0988: MOVE.L - D0,VAR(0x2)

0x098c: MOVEQ - 0x0,D0
0x098e: MOVE.W - D0,VAR(0x74)
0x0992: MOVE.W - VAR(0x74),D0
0x0996: EXT.L - D0
0x0998: LSL.L - 0x2,D0
0x099a: ADDI.L - HEAP(0x41bbe),D0
0x09a4: PUSH.L - D0
0x09a6: MOVE.W - VAR(0x74),D0
0x09aa: EXT.L - D0
0x09ac: LSL.L - 0x2,D0
0x09ae: ADDI.L - HEAP(0x41bbe),D0
0x09b8: MOVEA.L - D0,A0
0x09ba: JSR - "BCD_FROM_VAR"
0x09be: JSR - "BCD_PUSH_LHS"
0x09c2: MOVE.W - VAR(0x74),D0
0x09c6: EXT.L - D0
0x09c8: ADDQ.L - 0x4,D0
0x09ca: LSL.L - 0x2,D0
0x09cc: ADDI.L - HEAP(0x41bbe),D0
0x09d6: MOVEA.L - D0,A0
0x09d8: JSR - "BCD_FROM_VAR"
0x09dc: JSR - "ADD_BCD"
0x09e0: POP.L - A0
0x09e2: JSR - "BCD_TO_VAR"

0x09e6: MOVE.W - VAR(0x74),D0
0x09ea: EXT.L - D0
0x09ec: ADDI.L - 0x0000000b,D0
0x09f2: LSL.L - 0x1,D0
0x09f4: ADDI.L - HEAP(0x41af4),D0
0x09fe: PUSH.L - D0
0x0a00: MOVE.W - VAR(0x74),D0
0x0a04: EXT.L - D0
0x0a06: LSL.L - 0x2,D0
0x0a08: ADDI.L - HEAP(0x41bbe),D0
0x0a12: MOVEA.L - D0,A0
0x0a14: JSR - "BCD_FROM_VAR"
0x0a18: JSR - "CAST_INT"
0x0a1c: POP.L - A0
0x0a1e: MOVE.W - D0,(A0)
0x0a20: MOVEQ - 0x3,D0
0x0a22: ADDQ.W - 0x1,VAR(0x74)
0x0a26: CMP.W - VAR(0x74),D0
0x0a2a: BGE - [0x0992]

0x0a2e: MOVE.L - HEAP(0x41b0a),D0
0x0a38: PUSH.L - D0
0x0a3a: TRAP - _PaintRect
0x0a3c: MOVEQ - 0x0,D0
0x0a3e: BSR - [0x024a]
0x0a42: MOVE.W - VAR(0x1a2),D0
0x0a46: EXT.L - D0
0x0a48: SUBQ.L - 0x1,D0
0x0a4a: ADDQ.W - 0x1,VAR(0x192)
0x0a4e: CMP.W - VAR(0x192),D0
0x0a52: BGE - [0x0982]

0x0a56: MOVE.L - HEAP(0x41afc),D0
0x0a60: PUSH.L - D0
0x0a62: TRAP - _PaintRect
0x0a64: TRAP - _PenNormal
0x0a66: RTS
--------
*/
void FoolPrologue::sub_128_800(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4, int16_t unk5, int16_t unk6, int16_t unk7, int16_t unk8, int16_t unk9) {
	this->arr_i16_41af4[0] = unk9;
	this->arr_i16_41af4[1] = unk8;
	this->arr_i16_41af4[2] = unk7;
	this->arr_i16_41af4[3] = unk6;
	this->arr_i16_41af4[4] = unk5;
	this->arr_i16_41af4[5] = unk4;
	this->arr_i16_41af4[6] = unk3;
	this->arr_i16_41af4[7] = unk2;
	g_toolbox->PenNormal();
	g_toolbox->PenMode(0xa);
	for (int i = 0; i < 4; i++) {
		this->arr_f64_41bbe[i] = (double)this->arr_i16_41af4[i];
		this->arr_f64_41bbe[i+4] = (double)(this->arr_i16_41af4[i] - this->arr_i16_41af4[i+4])/(double)(this->var_i32_182);
	}
	g_toolbox->PaintRect(&this->arr_i16_41af4);
	for (int i = 1; i < unk1 + 1; i++) {
		this->var_i32_2 = g_toolbox->TickCount();

		for (int j = 0; j < 4; j++) {
			this->arr_f64_41bbe[j] = this->arr_f64_41bbe[j] + this->arr_f64_41bbe[j+4];
			this->arr_i16_41af4[j+11] = (int16_t)this->arr_f64_41bbe[j];
		}
		g_toolbox->PaintRect(&this->arr_i16_41b0a);
		this->sub_128_24a(0);
	}
	g_toolbox->PaintRect(&this->arr_i16_41afc);
	g_toolbox->PenNormal();
}

/*

0x0a68: JMP - [0xa88]
--------
0x0a6c: MOVE.W - D0,VAR(0x194)
0x0a70: POP.L - D0
0x0a72: MOVE.W - D0,VAR(0x178)
0x0a76: PUSH.W - VAR(0x178)
0x0a7a: PUSH.W - VAR(0x194)
0x0a7e: TRAP - _MoveTo
0x0a80: PEA - VAR(0x76)
0x0a84: TRAP - _DrawString
0x0a86: RTS
--------
*/
void FoolPrologue::sub_128_a6c(int16_t unk1, int16_t unk2) {
	g_toolbox->MoveTo(unk2, unk1);
	g_toolbox->DrawString(this->var_str_76);
}
/*
0x0a88: JMP - [0xc8a]
--------
0x0a8c: MOVE.W - D0,VAR(0x1a4)
0x0a90: MOVEQ - 0xa,D0
0x0a92: PUSH.W - D0
0x0a94: TRAP - _PenMode
0x0a96: JSR - 130,0xe82
0x0a9a: MOVEQ - 0x0,D0
0x0a9c: MOVEA.W - VAR(0x6),A0
0x0aa0: ADD.L - A0,D0
0x0aa2: LSL.L - 0x1,D0
0x0aa4: ADDI.L - HEAP(0x1e8),D0
0x0aae: PUSH.L - D0
0x0ab0: MOVEQ - 0x0,D0
0x0ab2: MOVEA.W - VAR(0x6),A0
0x0ab6: ADD.L - A0,D0
0x0ab8: LSL.L - 0x1,D0
0x0aba: ADDI.L - HEAP(0x1e8),D0
0x0ac4: MOVEA.L - D0,A0
0x0ac6: MOVE.W - (A0),D0
0x0ac8: EXT.L - D0
0x0aca: PUSH.L - D0
0x0acc: MOVE.L - 0x2f1,D0
0x0ad2: MOVEA.W - VAR(0x6),A0
0x0ad6: ADD.L - A0,D0
0x0ad8: LSL.L - 0x1,D0
0x0ada: ADDI.L - HEAP(0x1e8),D0
0x0ae4: MOVEA.L - D0,A0
0x0ae6: MOVE.W - (A0),D0
0x0ae8: EXT.L - D0
0x0aea: POP.L - D1
0x0aec: ADD.L - D1,D0
0x0aee: POP.L - A0
0x0af0: MOVE.W - D0,(A0)

0x0af2: MOVE.L - 0xfb,D0
0x0af8: MOVEA.W - VAR(0x6),A0
0x0afc: ADD.L - A0,D0
0x0afe: LSL.L - 0x1,D0
0x0b00: ADDI.L - HEAP(0x1e8),D0
0x0b0a: PUSH.L - D0
0x0b0c: MOVE.L - 0xfb,D0
0x0b12: MOVEA.W - VAR(0x6),A0
0x0b16: ADD.L - A0,D0
0x0b18: LSL.L - 0x1,D0
0x0b1a: ADDI.L - HEAP(0x1e8),D0
0x0b24: MOVEA.L - D0,A0
0x0b26: MOVE.W - (A0),D0
0x0b28: EXT.L - D0
0x0b2a: PUSH.L - D0
0x0b2c: MOVE.L - 0x2f1,D0
0x0b32: MOVEA.W - VAR(0x6),A0
0x0b36: ADD.L - A0,D0
0x0b38: LSL.L - 0x1,D0
0x0b3a: ADDI.L - HEAP(0x1e8),D0
0x0b44: MOVEA.L - D0,A0
0x0b46: MOVE.W - (A0),D0
0x0b48: EXT.L - D0
0x0b4a: POP.L - D1
0x0b4c: ADD.L - D1,D0
0x0b4e: POP.L - A0
0x0b50: MOVE.W - D0,(A0)

*/
void FoolPrologue::sub_128_a8c(int16_t unk) {
	this->var_i16_1a4 = unk;
	g_toolbox->PenMode(0xa);
	do {
		// 128:0a96
		this->sub_130_e82();
		this->arr_i16_1e8[this->var_i16_6] =
		this->arr_i16_1e8[0x2f1+this->var_i16_6]
		+ this->arr_i16_1e8[this->var_i16_6];

		this->arr_i16_1e8[0xfb+this->var_i16_6] =
		this->arr_i16_1e8[0xfb+this->var_i16_6]
		+ this->arr_i16_1e8[0x2f1+this->var_i16_6];

		// 128:0b52
		if ((this->arr_i16_1e8[this->var_i16_6] < 0x1f4) || (this->arr_i16_1e8[this->var_i16_6 + 0xfb] < 0x140)) {

/*
0x0b52: MOVEQ - 0x0,D0
0x0b54: MOVEA.W - VAR(0x6),A0
0x0b58: ADD.L - A0,D0
0x0b5a: LSL.L - 0x1,D0
0x0b5c: ADDI.L - HEAP(0x1e8),D0
0x0b66: MOVEA.L - D0,A0
0x0b68: MOVE.W - (A0),D0

0x0b6a: EXT.L - D0
0x0b6c: CMPI.L - 0x1f4,D0
0x0b72: SGT - D0
0x0b74: EXT.W - D0
0x0b76: EXT.L - D0
0x0b78: PUSH.L - D0

0x0b7a: MOVE.L - 0xfb,D0
0x0b80: MOVEA.W - VAR(0x6),A0
0x0b84: ADD.L - A0,D0
0x0b86: LSL.L - 0x1,D0
0x0b88: ADDI.L - HEAP(0x1e8),D0
0x0b92: MOVEA.L - D0,A0
0x0b94: MOVE.W - (A0),D0
0x0b96: EXT.L - D0
0x0b98: CMPI.L - 0x140,D0
0x0b9e: SGT - D0

0x0ba0: EXT.W - D0
0x0ba2: EXT.L - D0
0x0ba4: POPOR.L - D0
0x0ba6: TST.L - D0
0x0ba8: BNE - [0x0bae]
0x0baa: JMP - [0xc56]
--------


0x0bae: MOVEQ - 0x0,D0
0x0bb0: MOVEA.W - VAR(0x6),A0
0x0bb4: ADD.L - A0,D0
0x0bb6: LSL.L - 0x1,D0
0x0bb8: ADDI.L - HEAP(0x1e8),D0
0x0bc2: PUSH.L - D0
0x0bc4: MOVE.L - 0x264,D0
0x0bca: JSR - "RND_INT"
0x0bce: SUBI.L - 0xc8,D0
0x0bd4: POP.L - A0
0x0bd6: MOVE.W - D0,(A0)

0x0bd8: MOVE.L - 0xfb,D0
0x0bde: MOVEA.W - VAR(0x6),A0
0x0be2: ADD.L - A0,D0
0x0be4: LSL.L - 0x1,D0
0x0be6: ADDI.L - HEAP(0x1e8),D0
0x0bf0: PUSH.L - D0
0x0bf2: MOVE.L - 0x1ba,D0
0x0bf8: JSR - "RND_INT"
0x0bfc: SUBI.L - 0xc8,D0
0x0c02: POP.L - A0
0x0c04: MOVE.W - D0,(A0)

0x0c06: MOVE.L - 0x1f6,D0
0x0c0c: MOVEA.W - VAR(0x6),A0
0x0c10: ADD.L - A0,D0
0x0c12: LSL.L - 0x1,D0
0x0c14: ADDI.L - HEAP(0x1e8),D0
0x0c1e: PUSH.L - D0
0x0c20: MOVEQ - 0x5,D0
0x0c22: JSR - "RND_INT"
0x0c26: ADDQ.L - 0x1,D0
0x0c28: POP.L - A0
0x0c2a: MOVE.W - D0,(A0)

0x0c2c: MOVE.L - 0x2f1,D0
0x0c32: MOVEA.W - VAR(0x6),A0
0x0c36: ADD.L - A0,D0
0x0c38: LSL.L - 0x1,D0
0x0c3a: ADDI.L - HEAP(0x1e8),D0
0x0c44: PUSH.L - D0
0x0c46: MOVEQ - 0xa,D0
0x0c48: JSR - "RND_INT"
0x0c4c: ADDI.L - 0x00000019,D0
0x0c52: POP.L - A0
0x0c54: MOVE.W - D0,(A0)

0x0c56: JSR - 130,0xe82

0x0c5a: ADDQ.W - 0x1,VAR(0x6)
0x0c5e: CMPI.W - 0xb5,VAR(0x6)
0x0c64: BLE - [0x0c6e]
0x0c68: MOVEQ - 0x1,D0
0x0c6a: MOVE.W - D0,VAR(0x6)

0x0c6e: PUSH.L - 0 (unsafe)
0x0c70: TRAP - _TickCount
0x0c72: POP.L - D0
0x0c74: PUSH.L - D0
0x0c76: MOVE.L - VAR(0x2),D0
0x0c7a: MOVEA.W - VAR(0x1a4),A0
0x0c7e: ADD.L - A0,D0
0x0c80: POP.L - D1
0x0c82: CMP.L - D0,D1
0x0c84: BLE - [0x0a96]
0x0c88: RTS
--------
*/
		    // 128:0bae
			this->arr_i16_1e8[this->var_i16_6] = _vm->_rnd->getRandomNumber(0x264) - 0xc8;
			this->arr_i16_1e8[0xfb + this->var_i16_6] = (std::rand() % 0x1ba) - 0xc8;
			this->arr_i16_1e8[0x1f6 + this->var_i16_6] = (std::rand() % 0x5) + 0x1;
			this->arr_i16_1e8[0x2f1 + this->var_i16_6] = (std::rand() % 0xa) + 0x19;
		}
		// 128:0c56
		this->sub_130_e82();
		this->var_i16_6 += 1;
		if (this->var_i16_6 > 0xb5) {
			this->var_i16_6 = 0;
		}
	} while (this->var_i32_2 + this->var_i16_1a4 <= g_toolbox->TickCount())
}

/*

0x0c8a: JSR - 129,0x4
0x0c8e: MOVEQ - -0x1,D0
0x0c90: JSR - "MEM"
0x0c94: MOVE.L - D0,VAR(0x1a6)
0x0c98: CMPI.W - 0x1,VAR(0x1aa)
0x0c9e: BNE - [0x0cb0]
0x0ca2: JSR - 130,0x4
0x0ca6: MOVEQ - -0x1,D0
0x0ca8: JSR - "MEM"
0x0cac: MOVE.L - D0,VAR(0x1a6)
0x0cb0: CMPI.W - 0x2,VAR(0x1aa)
0x0cb6: BNE - [0x0cc8]
0x0cba: JSR - 131,0x4
0x0cbe: MOVEQ - -0x1,D0
0x0cc0: JSR - "MEM"
0x0cc4: MOVE.L - D0,VAR(0x1a6)
0x0cc8: JMP - "ZBASIC_UNK4"
0x0ccc: JSR - "ZBASIC_20"
0x0cd0: MOVE.L - HEAP(0x41598),D0
0x0cda: MOVE.L - D0,VAR(0x1ac)
0x0cde: MOVE.L - HEAP(0x41846),D0
0x0ce8: MOVE.L - D0,VAR(0x1b0)
0x0cec: MOVE.W - VAR(0x18e),D0
0x0cf0: EXT.L - D0
0x0cf2: MOVE.L - D0,D1
0x0cf4: MOVEQ - 0x2,D0
0x0cf6: JSR - "MUL_INT"
0x0cfa: ADDQ.L - 0x2,D0
0x0cfc: MOVE.W - D0,VAR(0x1b4)

0x0d00: MOVEQ - 0x1,D0
0x0d02: MOVE.W - D0,VAR(0x74)
0x0d06: MOVEA.W - VAR(0x74),A0
0x0d0a: ADDA.L - A0,A0
0x0d0c: ADDA.L - HEAP(0x41598),A0
0x0d16: MOVE.W - VAR(0x74),(A0)
0x0d1a: MOVE.W - VAR(0x18e),D0
0x0d1e: EXT.L - D0
0x0d20: ADDQ.W - 0x1,VAR(0x74)
0x0d24: CMP.W - VAR(0x74),D0
0x0d28: BGE - [0x0d06]

0x0d2a: MOVE.W - VAR(0x18e),VAR(0x74)
0x0d30: MOVE.W - VAR(0x74),D0
0x0d34: EXT.L - D0
0x0d36: JSR - "RND_INT"
0x0d3a: MOVE.W - D0,VAR(0x1b6)

0x0d3e: MOVE.W - VAR(0x74),D0
0x0d42: EXT.L - D0
0x0d44: LSL.L - 0x1,D0
0x0d46: ADDI.L - HEAP(0x412ea),D0
0x0d50: PUSH.L - D0
0x0d52: MOVEA.W - VAR(0x1b6),A0
0x0d56: ADDA.L - A0,A0
0x0d58: ADDA.L - HEAP(0x41598),A0
0x0d62: MOVE.W - (A0),D0
0x0d64: EXT.L - D0
0x0d66: POP.L - A0
0x0d68: MOVE.W - D0,(A0)

0x0d6a: MOVE.W - VAR(0x1b6),D0
0x0d6e: EXT.L - D0
0x0d70: MOVE.L - D0,D1
0x0d72: MOVEQ - 0x2,D0
0x0d74: JSR - "MUL_INT"
0x0d78: ADDQ.L - 0x2,D0
0x0d7a: MOVE.W - D0,VAR(0x1b8)

0x0d7e: MOVE.L - VAR(0x1ac),D0
0x0d82: MOVEA.W - VAR(0x1b8),A0
0x0d86: ADD.L - A0,D0
0x0d88: PUSH.L - D0
0x0d8a: MOVE.L - VAR(0x1b0),D0
0x0d8e: MOVEA.W - VAR(0x1b8),A0
0x0d92: ADD.L - A0,D0
0x0d94: PUSH.L - D0
0x0d96: MOVE.W - VAR(0x1b4),D0
0x0d9a: EXT.L - D0
0x0d9c: MOVEA.W - VAR(0x1b8),A0
0x0da0: SUB.L - A0,D0
0x0da2: JSR - "BLOCKMOVE"

0x0da6: MOVE.L - VAR(0x1b0),D0
0x0daa: MOVEA.W - VAR(0x1b8),A0
0x0dae: ADD.L - A0,D0
0x0db0: PUSH.L - D0
0x0db2: MOVE.L - VAR(0x1ac),D0
0x0db6: MOVEA.W - VAR(0x1b8),A0
0x0dba: ADD.L - A0,D0
0x0dbc: SUBQ.L - 0x2,D0
0x0dbe: PUSH.L - D0
0x0dc0: MOVE.W - VAR(0x1b4),D0
0x0dc4: EXT.L - D0
0x0dc6: MOVEA.W - VAR(0x1b8),A0
0x0dca: SUB.L - A0,D0
0x0dcc: JSR - "BLOCKMOVE"

0x0dd0: MOVEQ - 0x1,D0
0x0dd2: ADDI.W - 0xffff,VAR(0x74)
0x0dd8: CMP.W - VAR(0x74),D0
0x0ddc: BLE - [0x0d30]
0x0de0: RTS
--------
*/

void FoolPrologue::sub_128_c8a() {
	this->sub_129_004();
	this->var_i32_1a6 = zbasic_mem(-1);
	if (this->var_i16_1aa == 1) {
		this->sub_130_004();
		this->var_i32_1a6 = zbasic_mem(-1);
	}
	// 128:0cb0
	if (this->var_i16_1aa == 2) {
		this->sub_131_004();
		this->var_i32_1a6 = zbasic_mem(-1);
	}
	// 128:0cc8
	zbasic_unk4();
	zbasic_20();
	this->arr_i32_1ac = this->arr_i32_41598;
	this->arr_i32_1b0 = this->arr_i32_41846;
	this->var_i16_1b4 = this->var_i16_18e * 2 + 2;
	// 128:0d00
	for (int i=1; i < this->var_i16_18e; i++) {
		this->arr_i16_41598[i] = i;
	}
	// 128:0d2a
	for (int i=this->var_i16_18e; i<=1; i--) {
		this->var_i16_1b6 = _vm->_rnd->getRandomNumber(i);
		this->arr_i16_412ea[i] = this->arr_i16_41598[this->var_i16_1b6];
		this->var_i16_1b8 = this->var_i16_1b6 * 2 + 2;
		zbasic_blockmove(this->var_i16_1b4 - this->var_i16_1b8, this->var_i32_1ac + this->var_i16_1b8, this->var_i32_1b0 + this->var_i16_1b8);
		zbasic_blockmove(this->var_i16_1b4 - this->var_i16_1b8, this->var_i32_1b0 + this->var_i16_1b8, this->var_i32_1ac + this->var_i16_1b8);

	}
}

/*

0x0de2: PUSH.W - 0xfb
0x0de6: MOVEQ - 0x9,D0
0x0de8: PUSH.W - D0
0x0dea: PUSH.W - 0
0x0dec: MOVEQ - 0x2,D0
0x0dee: PUSH.W - D0
0x0df0: JSR - "TEXT"
0x0df4: MOVEA.L - STR(0xa0),A0  # b'Click Mouse to Continue'
0x0dfc: JSR - "BUFFER_CONCAT"
0x0e00: LEA - VAR(0x76),A0
0x0e04: JSR - "BUFFER_FLUSH"
0x0e08: PEA - [0x0e1a]
0x0e0c: MOVEQ - 0x5,D0
0x0e0e: PUSH.L - D0
0x0e10: MOVE.L - 0x151,D0
0x0e16: BRA - [0x0a6c]
0x0e1a: RTS
--------
*/
void FoolPrologue::sub_128_de2() {
	zbasic_text(0xfb, 0x9, 0x0, 0x2);
	this->var_str_76 = STR(0xa0); // "Click Mouse to Continue"
	this->sub_128_a6c(0x151, 0x5);
}

/*

0x0e1c: PUSH.W - 0xfb
0x0e20: MOVEQ - 0x9,D0
0x0e22: PUSH.W - D0
0x0e24: PUSH.W - 0
0x0e26: MOVEQ - 0x2,D0
0x0e28: PUSH.W - D0
0x0e2a: JSR - "TEXT"
0x0e2e: MOVEA.L - STR(0xb8),A0  # b'Click Mouse to Continue'
0x0e36: JSR - "BUFFER_CONCAT"
0x0e3a: LEA - VAR(0x76),A0
0x0e3e: JSR - "BUFFER_FLUSH"
0x0e42: PEA - [0x0e56]
0x0e46: PUSH.L - 0x000001f7
0x0e4c: MOVE.L - 0x154,D0
0x0e52: BRA - [0x026c]
0x0e56: RTS
--------
*/
void FoolPrologue::sub_128_e1c() {
	zbasic_text(0xfb, 0x9, 0x0, 0x2);
	this->var_str_76 = "Click Mouse to Continue"; // STR(0xb8)
	this->sub_128_26c(0x154, 0x1f7);
}

/*
0x0e58: JSR - [0xe80]
0x0e5c: PUSH.W - 0 (unsafe)
0x0e5e: MOVEQ - 0x2,D0
0x0e60: PUSH.W - D0
0x0e62: PEA - VAR(0x22)
0x0e66: TRAP - _GetNextEvent
0x0e68: POP.B - D0
0x0e6a: EXT.W - D0
0x0e6c: EXT.L - D0
0x0e6e: MOVE.W - D0,VAR(0x1ba)
0x0e72: CMPI.W - 0x1,VAR(0x22)
0x0e78: BNE - [0x0e5c]
0x0e7a: JSR - [0xe80]
0x0e7e: RTS
--------
*/

void FoolPrologue::sub_128_e58() {
	this->sub_128_e80();
	while (true) {
		this->var_i16_1ba = g_toolbox->GetNextEvent(0, 0x2, &this->var_22);
		if (this->var_22 == 1)
			break;
	}
	this->sub_128_e80();
}

/*

0x0e80: PUSH.W - 0 (unsafe)
0x0e82: MOVEQ - -0x1,D0
0x0e84: PUSH.W - D0
0x0e86: PEA - VAR(0x22)
0x0e8a: TRAP - _GetNextEvent
0x0e8c: POP.B - D0
0x0e8e: EXT.W - D0
0x0e90: EXT.L - D0
0x0e92: MOVE.W - D0,VAR(0x1ba)
0x0e96: CMPI.W - 0x6,VAR(0x22)
0x0e9c: BNE - [0x0ea4]
0x0ea0: JSR - [0xed2]
0x0ea4: CMPI.W - 0x7,VAR(0x22)
0x0eaa: BNE - [0x0eb2]
0x0eae: JSR - [0xee0]
0x0eb2: MOVE.W - VAR(0x22),D0
0x0eb6: SEQ - D0
0x0eb8: EXT.W - D0
0x0eba: EXT.L - D0
0x0ebc: PUSH.L - D0
0x0ebe: MOVE.W - VAR(0x30),D0
0x0ec2: EXT.L - D0
0x0ec4: ANDI.L - 0x80,D0
0x0eca: POPAND.L - D0
0x0ecc: TST.L - D0
0x0ece: BEQ - [0x0e80]
0x0ed0: RTS
--------
*/
void FoolPrologue::sub_128_e80() {
	while (true) {
		this->var_i16_1ba = (int)g_toolbox->GetNextEvent(-1, &this->var_22);
		if (this->var_22 == 6) {
			this->sub_128_ed2();
		}
		if (this->var_22 == 7) {
			this->sub_128_ee0();
		}
		if ((this->var_i16_30 & 0x80) && (this->var_22 == 0)) {
			break;
		}
	}

}

/*
0x0ed2: PUSH.L - VAR(0x24)
0x0ed6: TRAP - _BeginUpdate
0x0ed8: PUSH.L - VAR(0x24)
0x0edc: TRAP - _EndUpdate
0x0ede: RTS
--------
*/
void FoolPrologue::sub_128_ed2() {
	g_toolbox->BeginUpdate(this->var_window_24);
	g_toolbox->EndUpdate(this->var_window_24);
}

/*
0x0ee0: LEA - VAR(0x24),A0
0x0ee4: EXG - D0,A0
0x0ee6: JSR - "PEEKWORD"
0x0eea: MOVE.W - D0,VAR(0x1bc)
0x0eee: LEA - VAR(0x24),A0
0x0ef2: EXG - D0,A0
0x0ef4: ADDQ.L - 0x2,D0
0x0ef6: JSR - "PEEKWORD"
0x0efa: MOVE.W - D0,VAR(0x1be)
0x0efe: MOVE.W - VAR(0x1be),D0
0x0f02: EXT.L - D0
0x0f04: JSR - "ZBASIC_UNK11"
0x0f08: RTS
--------
*/
void FoolPrologue::sub_128_ee0() {
	this->var_i16_1bc = this->var_window_24.port;
	this->var_i16_1be = this->var_window_24.windowKind;
	zbasic_unk11(this->var_i16_1be);
}
/*

0x0f0a: MOVE.W - 0x80,D1
0x0f0e: JMP - "ZBASIC_199"
0x0f12: JMP - 129,0x4
*/

// Sources:
// QuickDraw types, Inside Macintosh I-201
// QuickDraw global list, Inside Macintosh I-162

// QuickDraw globals; offset below the A5 address.
// GrafPtr thePort = 0x00
// Pattern white = 0x04
// Pattern black = 0x0c
// Pattern gray = 0x14
// Pattern ltGray = 0x1c
// Pattern dkGray = 0x24
// Arrow cursor = 0x2c
// - Bits16 cursor.data = 0x2c
// - Bits16 cursor.mask = 0x4c
// - Point cursor.hotSpot = 0x6c
// BitMap screenBits = 0x70
// - Ptr screenBits.baseAddr = 0x70
// - INTEGER screenBits.rowBytes = 0x74
// - Rect screenBits.bounds = 0x76
// LONGINT randSeed = 0x7e
/*
Found custom CODE block 129
0x0004: MOVE.L - 0x904,D0
0x000a: JSR - "PEEKLONG"   # current A5
0x000e: SUBI.L - 0x72,D0
0x0014: JSR - "PEEKWORD" # quickdraw globals,
0x0018: MOVE.W - D0,VAR(0x10)

0x001c: MOVE.L - 0x904,D0
0x0022: JSR - "PEEKLONG"  # current A5
0x0026: SUBI.L - 0x74,D0
0x002c: JSR - "PEEKWORD"
0x0030: MOVE.W - D0,VAR(0x12)
*/

void FoolPrologue::sub_129_004() {
	// get the screen width and height from QuickDraw
	this->var_i16_10 = SCREEN_WIDTH;  // window width?
	this->var_i16_12 = SCREEN_HEIGHT; // window height?

/*
0x0034: MOVE.W - VAR(0x10),D0
0x0038: EXT.L - D0
0x003a: SUBI.L - 0x200,D0
0x0040: MOVE.L - D0,D1
0x0042: MOVEQ - 0x2,D0
0x0044: JSR - "DIV_INT"
0x0048: MOVE.W - D0,VAR(0x14)

0x004c: MOVE.W - VAR(0x12),D0
0x0050: EXT.L - D0
0x0052: SUBI.L - 0x156,D0
0x0058: MOVE.L - D0,D1
0x005a: MOVEQ - 0x2,D0
0x005c: JSR - "DIV_INT"
0x0060: MOVE.W - D0,VAR(0x16)
*/
	// 129:0034
	// set left and top offsets based on a drawable area of 512x342
	this->var_i16_14 = (this->var_i16_10 - 0x200)/2;
	this->var_i16_16 = (this->var_i16_12 - 0x156)/2;

	/*
0x0064: MOVEQ - 0x1,D0
0x0066: PUSH.W - D0
0x0068: PEA - STR(0xd0) # b''
0x0070: PUSH.W - 0
0x0072: PUSH.W - 0
0x0074: MOVEQ - -0x1,D0
0x0076: PUSH.W - D0
0x0078: PUSH.W - VAR(0x10)
0x007c: PUSH.W - VAR(0x12)
0x0080: MOVEQ - -0x1,D0
0x0082: PUSH.W - D0
0x0084: MOVEQ - 0x3,D0
0x0086: PUSH.W - D0
0x0088: JSR - "WINDOW"
0x008c: JSR - "COORDINATE_WINDOW"
0x0090: MOVEQ - 0x0,D0
0x0092: MOVE.W - D0,VAR(0x1c)
0x0096: MOVEQ - 0x0,D0
0x0098: MOVE.W - D0,VAR(0x1e)
0x009c: MOVEQ - 0x0,D0
0x009e: MOVE.W - D0,VAR(0x20)
*/
	// 129:0064
	zbasic_window(1, "", 0, 0, this->var_i16_10, this->var_i16_12, 3);
	zbasic_coordinate_window();

	this->var_i16_1c = 0;
	this->var_i16_1e = 0;
	this->var_i16_20 = 0;
/*
0x00a2: MOVE.L - 0x28e,D0  # ROM85
0x00a8: JSR - "BCD_FROM_INT"
0x00ac: JSR - "CAST_INT"
0x00b0: JSR - "PEEKWORD"
0x00b4: CMPI.L - 0x3fff,D0
0x00ba: BEQ - [0x00c0]
0x00bc: JMP - [0xee]
--------
0x00c0: MOVE.W - 0x4e20,VAR(0x1c)
0x00c6: MOVE.W - 0x4e20,VAR(0x1e)
0x00cc: MOVE.W - 0x4e20,VAR(0x20)
0x00d2: MOVEQ - 0x64,D0
0x00d4: PUSH.W - D0
0x00d6: MOVEQ - 0x64,D0
0x00d8: PUSH.W - D0
0x00da: PEA - VAR(0x1c)
0x00de: TRAP - _SetCPixel
0x00e0: MOVEQ - 0x64,D0
0x00e2: PUSH.W - D0
0x00e4: MOVEQ - 0x64,D0
0x00e6: PUSH.W - D0
0x00e8: PEA - VAR(0x1c)
0x00ec: TRAP - _GetCPixel

	*/
	// code checks Rom85 for presence of 128K ROM before doing next bit
	// 129:00a2
	if (true) {
		this->var_i16_1c = 0x4e20;
		this->var_i16_1e = 0x4e20;
		this->var_i16_20 = 0x4e20;
		g_toolbox->SetCPixel(0x64, 0x64, &this->var_i16_1c);
		g_toolbox->GetCPixel(0x64, 0x64, &this->var_i16_1c);
	}

/*
0x00ee: MOVE.L - HEAP(0xa8a),D0
0x00f8: MOVE.L - D0,VAR(0xc)
0x00fc: PUSH.L - VAR(0xc)
0x0100: TRAP - _OpenPort
0x0102: MOVE.L - HEAP(0x9c0),D0
0x010c: MOVE.L - D0,VAR(0x8)
0x0110: PUSH.L - VAR(0x8)
0x0114: TRAP - _OpenPort
0x0116: CMPI.W - 0x200,VAR(0x10)
0x011c: SNE - D0
0x011e: EXT.W - D0
0x0120: EXT.L - D0
0x0122: PUSH.L - D0
0x0124: CMPI.W - 0x156,VAR(0x12)
0x012a: SNE - D0
0x012c: EXT.W - D0
0x012e: EXT.L - D0
0x0130: POPOR.L - D0
0x0132: TST.L - D0
0x0134: BNE - [0x013a]
0x0136: JMP - [0x1b0]
--------
 */

	// 129:00ee
	this->var_i32_c = this->arr_i21_a8a[0];
	g_toolbox->OpenPort(this->var_i32_c);
	this->var_i32_8 = this->arr_i21_9c0[0];
	g_toolbox->OpenPort(this->var_i32_8);

	if ((this->var_i16_10 != SCREEN_WIDTH || this->var_i16_12 != SCREEN_HEIGHT)) {

/*

0x013a: PEA - [0x0156]
0x013e: MOVEQ - 0x14,D0
0x0140: PUSH.L - D0
0x0142: PUSH.L - 0
0x0144: MOVEA.W - VAR(0x12),A0
0x0148: PUSH.L - A0
0x014a: MOVEA.W - VAR(0x10),A0
0x014e: PUSH.L - A0
0x0150: MOVEQ - 0x2,D0
0x0152: JMP - 128,0x2f0

0x0156: MOVE.L - HEAP(0x1bc),D0
0x0160: PUSH.L - D0
0x0162: MOVE.W - VAR(0x14),D0
0x0166: EXT.L - D0
0x0168: SUBQ.L - 0x2,D0
0x016a: PUSH.W - D0
0x016c: MOVE.W - VAR(0x16),D0
0x0170: EXT.L - D0
0x0172: SUBQ.L - 0x2,D0
0x0174: PUSH.W - D0
0x0176: MOVE.W - VAR(0x14),D0
0x017a: EXT.L - D0
0x017c: ADDI.L - 0x00000202,D0
0x0182: PUSH.W - D0
0x0184: MOVE.W - VAR(0x16),D0
0x0188: EXT.L - D0
0x018a: ADDI.L - 0x00000158,D0
0x0190: PUSH.W - D0
0x0192: TRAP - _SetRect

0x0194: MOVE.L - HEAP(0x19c),D0
0x019e: PUSH.L - D0
0x01a0: TRAP - _PenPat
0x01a2: MOVE.L - HEAP(0x1bc),D0
0x01ac: PUSH.L - D0
0x01ae: TRAP - _FrameRect
 * */
		// 129:013a
		this->sub_128_2f0(0x2, 0x14, 0, this->var_i16_12, this->var_i16_10);
		g_toolbox->SetRect(this->arr_i32_1bc[0], this->var_i16_14 - 2, this->var_i16_16 - 2, this->var_i16_14 + SCREEN_WIDTH + 2, this->var_i16_16 + SCREEN_HEIGHT + 2);
		g_toolbox->PenPat(this->arr_i32_19c[0]);
		g_toolbox->FrameRect(this->arr_i32_1bc[0]);
	}

/*
0x01b0: PUSH.L - VAR(0x8)
0x01b4: TRAP - _SetPort

0x01b6: PUSH.W - 0x200
0x01ba: PUSH.W - 0x156
0x01be: TRAP - _PortSize

0x01c0: PUSH.W - VAR(0x14)
0x01c4: PUSH.W - VAR(0x16)
0x01c8: TRAP - _MovePortTo

0x01ca: MOVE.L - HEAP(0x1bc),D0
0x01d4: PUSH.L - D0
0x01d6: PUSH.W - 0
0x01d8: PUSH.W - 0
0x01da: PUSH.W - 0x200
0x01de: PUSH.W - 0x156
0x01e2: TRAP - _SetRect

0x01e4: MOVE.L - HEAP(0x1bc),D0
0x01ee: PUSH.L - D0
0x01f0: TRAP - _ClipRect
 */

	// 129:01b0
	g_toolbox->SetPort(this->var_i32_8);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_14, this->var_i16_16);
	g_toolbox->SetRect(this->arr_i32_1bc[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->ClipRect(this->arr_i32_1bc[0]);
/*
0x01f2: CMPI.W - 0x156,VAR(0x12)
0x01f8: BEQ - [0x01fe]
0x01fa: JMP - [0x238]
--------
0x01fe: MOVE.L - HEAP(0x1bc),D0
0x0208: PUSH.L - D0
0x020a: PUSH.W - 0
0x020c: MOVEQ - 0x14,D0
0x020e: PUSH.W - D0
0x0210: PUSH.W - 0x200
0x0214: PUSH.W - 0x156
0x0218: TRAP - _SetRect
0x021a: MOVE.L - HEAP(0x1bc),D0
0x0224: PUSH.L - D0
0x0226: MOVE.L - HEAP(0x19c),D0
0x0230: PUSH.L - D0
0x0232: TRAP - _FillRect
0x0234: JMP - [0x26c]
--------
0x0238: MOVE.L - HEAP(0x1bc),D0
0x0242: PUSH.L - D0
0x0244: PUSH.W - 0
0x0246: PUSH.W - 0
0x0248: PUSH.W - 0x200
0x024c: PUSH.W - 0x156
0x0250: TRAP - _SetRect
0x0252: MOVE.L - HEAP(0x1bc),D0
0x025c: PUSH.L - D0
0x025e: MOVE.L - HEAP(0x19c),D0
0x0268: PUSH.L - D0
0x026a: TRAP - _FillRect
 * */
	// 129:01f2
	if (this->var_i16_12 == SCREEN_HEIGHT) {
		g_toolbox->SetRect(this->arr_i32_1bc, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(this->arr_i32_1bc, this->arr_i32_19c);
	} else {
		g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(this->arr_i32_1bc, this->arr_i32_19c);
	}

	/*
0x026c: MOVE.L - VAR(0x8),D0
0x0270: ADDQ.L - 0x2,D0
0x0272: JSR - "PEEKLONG"
0x0276: MOVE.L - D0,VAR(0x32)

0x027a: MOVEQ - 0x40,D0
0x027c: MOVE.W - D0,VAR(0x36)

0x0280: MOVE.W - VAR(0x16),D0
0x0284: EXT.L - D0
0x0286: NEG.L - D0
0x0288: MOVE.W - D0,VAR(0x38)

0x028c: MOVE.W - VAR(0x14),D0
0x0290: EXT.L - D0
0x0292: NEG.L - D0
0x0294: MOVE.W - D0,VAR(0x3a)

0x0298: MOVE.W - VAR(0x16),D0
0x029c: EXT.L - D0
0x029e: ADDI.L - 0x00000156,D0
0x02a4: MOVE.W - D0,VAR(0x3c)

0x02a8: MOVE.W - VAR(0x14),D0
0x02ac: EXT.L - D0
0x02ae: ADDI.L - 0x00000200,D0
0x02b4: MOVE.W - D0,VAR(0x3e)

0x02b8: MOVEQ - 0x40,D0
0x02ba: MOVE.W - D0,VAR(0x44)

0x02be: MOVEQ - 0x0,D0
0x02c0: MOVE.W - D0,VAR(0x46)

0x02c4: MOVEQ - 0x0,D0
0x02c6: MOVE.W - D0,VAR(0x48)

0x02ca: MOVE.W - 0x156,VAR(0x4a)

0x02d0: MOVE.W - 0x200,VAR(0x4c)

0x02d6: MOVEQ - 0x40,D0
0x02d8: MOVE.W - D0,VAR(0x52)

0x02dc: MOVEQ - 0x0,D0
0x02de: MOVE.W - D0,VAR(0x54)

0x02e2: MOVEQ - 0x0,D0
0x02e4: MOVE.W - D0,VAR(0x56)

0x02e8: MOVE.W - 0x156,VAR(0x58)

0x02ee: MOVE.W - 0x200,VAR(0x5a)

0x02f4: MOVEQ - -0x1,D0
0x02f6: JSR - "MEM"
0x02fa: MOVE.L - D0,VAR(0x1c0)
*/
	// 129:026c
	this->var_i32_32 = this->var_i32_8.portBits;
	this->var_i16_36 = 0x40;
	this->var_i16_38 = -this->var_i16_16;
	this->var_i16_3a = -this->var_i16_14;
	this->var_i16_3c = this->var_i16_16 + SCREEN_HEIGHT;
	this->var_i16_3e = this->var_i16_14 + SCREEN_WIDTH;
	this->var_i16_44 = 0x40;
	this->var_i16_46 = 0x0;
	this->var_i16_48 = 0x0;
	this->var_i16_4a = SCREEN_HEIGHT;
	this->var_i16_4c = SCREEN_WIDTH;
	this->var_i16_52 = 0x40;
	this->var_i16_54 = 0x0;
	this->var_i16_56 = 0x0;
	this->var_i16_58 = SCREEN_HEIGHT;
	this->var_i16_5a = SCREEN_WIDTH;

	this->var_i32_1c0 = zbasic_mem(-1);
	/*

0x02fe: CMPI.L - 0xd6d8,VAR(0x1c0)
0x0306: BLT - [0x030c]
0x0308: JMP - [0x4a0]
--------
0x030c: TRAP - _InitCursor
0x030e: MOVEQ - 0x0,D0
0x0310: MOVE.L - D0,D1
0x0312: MOVEQ - 0x14,D0
0x0314: JSR - "GET_FROM"

0x0318: PUSH.L - 0x000001fc
0x031e: PUSH.L - 0x00000152
0x0324: MOVE.L - HEAP(0xb54),D0
0x032e: MOVEA.L - D0,A0
0x0330: POP.L - D0
0x0332: POP.L - D1
0x0334: JSR - "GET_TO"

0x0338: MOVEQ - 0x19,D0
0x033a: PUSH.W - D0
0x033c: MOVEQ - 0x64,D0
0x033e: PUSH.L - D0
0x0340: PUSH.W - 0xff
0x0344: PUSH.W - 0
0x0346: JSR - "ZBASIC_UNK6"

0x034a: PEA - [0x0368]
0x034e: MOVEQ - 0x46,D0
0x0350: PUSH.L - D0
0x0352: MOVEQ - 0x64,D0
0x0354: PUSH.L - D0
0x0356: PUSH.L - 0x000000d8
0x035c: PUSH.L - 0x00000198
0x0362: MOVEQ - 0x2,D0
0x0364: JMP - 128,0x2f0

0x0368: PEA - [0x0386]
0x036c: MOVEQ - 0x4b,D0
0x036e: PUSH.L - D0
0x0370: MOVEQ - 0x69,D0
0x0372: PUSH.L - D0
0x0374: PUSH.L - 0x000000d3
0x037a: PUSH.L - 0x00000193
0x0380: MOVEQ - 0x1,D0
0x0382: JMP - 128,0x2f0

*/
	// 129:02fe
	if (this->var_i32_1c0 < 0xd6d8) {
		g_toolbox->InitCursor();
		zbasic_get_from(0x0, 0x14);
		zbasic_get_to(0x1fc, 0x152, this->arr_i32_b54);
		zbasic_unk6(0x19, 0x64, 0xff, 0x0);
		this->sub_128_2f0(2, 0x46, 0x64, 0xd8, 0x198);
		this->sub_128_2f0(1, 0x4b, 0x69, 0xd3, 0x193);
	/*


0x0386: PEA - [0x03a4]
0x038a: MOVEQ - 0x50,D0
0x038c: PUSH.L - D0
0x038e: MOVEQ - 0x6e,D0
0x0390: PUSH.L - D0
0x0392: PUSH.L - 0x000000ce
0x0398: PUSH.L - 0x0000018e
0x039e: MOVEQ - 0x2,D0
0x03a0: JMP - 128,0x2f0

0x03a4: PUSH.W - 0
0x03a6: MOVEQ - 0xc,D0
0x03a8: PUSH.W - D0
0x03aa: PUSH.W - 0
0x03ac: MOVEQ - 0x3,D0
0x03ae: PUSH.W - D0
0x03b0: JSR - "TEXT"

0x03b4: MOVEA.L - STR(0xd2),A0  # b'There is not enough available memory'
0x03bc: JSR - "BUFFER_CONCAT"
0x03c0: LEA - VAR(0x76),A0
0x03c4: JSR - "BUFFER_FLUSH"
0x03c8: PEA - [0x03d8]
0x03cc: PUSH.L - 0x000000fc
0x03d2: MOVEQ - 0x64,D0
0x03d4: JMP - 128,0x2a6

0x03d8: MOVEA.L - STR(0xf8),A0  # b'to run the program at this time.'
0x03e0: JSR - "BUFFER_CONCAT"
0x03e4: LEA - VAR(0x76),A0
0x03e8: JSR - "BUFFER_FLUSH"
0x03ec: PEA - [0x03fc]
0x03f0: PUSH.L - 0x000000fc
0x03f6: MOVEQ - 0x78,D0
0x03f8: JMP - 128,0x2a6

0x03fc: MOVEA.L - STR(0x11a),A0  # b'Check your instruction manual for '
0x0404: JSR - "BUFFER_CONCAT"
0x0408: LEA - VAR(0x76),A0
0x040c: JSR - "BUFFER_FLUSH"
0x0410: PEA - [0x0424]
0x0414: PUSH.L - 0x000000fc
0x041a: MOVE.L - 0x96,D0
0x0420: JMP - 128,0x2a6

0x0424: MOVEA.L - STR(0x13e),A0  # b'for possible solutions.'
0x042c: JSR - "BUFFER_CONCAT"
0x0430: LEA - VAR(0x76),A0
0x0434: JSR - "BUFFER_FLUSH"
0x0438: PEA - [0x044c]
0x043c: PUSH.L - 0x000000fc
0x0442: MOVE.L - 0xaa,D0
0x0448: JMP - 128,0x2a6

0x044c: MOVEA.L - STR(0x156),A0  # b'(click mouse to quit)'
0x0454: JSR - "BUFFER_CONCAT"
0x0458: LEA - VAR(0x76),A0
0x045c: JSR - "BUFFER_FLUSH"
0x0460: PEA - [0x0474]
0x0464: PUSH.L - 0x000000fc
0x046a: MOVE.L - 0xcb,D0
0x0470: JMP - 128,0x2a6

0x0474: JSR - 128,0xe58

0x0478: MOVEQ - 0x0,D0
0x047a: MOVE.L - D0,D1
0x047c: MOVEQ - 0x14,D0
0x047e: JSR - "GET_FROM"

0x0482: PUSH.W - 0
0x0484: MOVE.L - HEAP(0xb54),D0
0x048e: PUSH.L - D0
0x0490: MOVEQ - 0x0,D0
0x0492: JSR - "ZBASIC_194"

0x0496: MOVEQ - 0x0,D0
0x0498: MOVE.W - D0,VAR(0x1aa)
0x049c: JMP - [0x772]
--------
*/
		// 129:0386
		this->sub_128_2f0(2, 0x50, 0x6e, 0xc3, 0x18e);
		zbasic_text(0, 0xc, 0, 3);
		this->var_str_76 = STR(0xd2);
		this->sub_128_2a6(0x64, 0xfc);
		this->var_str_76 = STR(0xf8);
		this->sub_128_2a6(0x78, 0xfc);
		this->var_str_76 = STR(0x11a);
		this->sub_128_2a6(0x96, 0xfc);
		this->var_str_76 = STR(0x13e);
		this->sub_128_2a6(0xaa, 0xfc);
		this->var_str_76 = STR(0x156);
		this->sub_128_2a6(0xcb, 0xfc);
		this->sub_128_e58();
		zbasic_get_from(0x0, 0x14);

		zbasic_194(0, 0, this->arr_i32_b54);
		this->var_i16_1aa = 0;

	} else {
		// 129:04a0
		this->var_i16_180 = 1;
		this->var_i16_1aa = 1;
		this->var_i16_176 = zbasic_249(&this->var_i16_180, &this->var_i16_1c4, &this->var_i16_2c4, &this->var_i16_2c8);
		if (zbasic_106(STR(0x16c), &this->var_i16_1c4)) {
			// 129:04de
			this->var_str_76 = STR(0x16e);
			for (int i = 1; i < 0xa; i++) {
				this->var_str_76 += zbasic_chr(this->var_i16_74);
			}

			// 129:0522
			this->var_i16_2ca = this->var_str_76.size();
			this->var_i16_2cc = 0;
			// 129:0534
			// ??????
/*
0x0534: LEA - [0x0764],A0
0x0538: MOVE.L - A0,-0x8ee(A5)
0x053c: SF - 0x8,D0
0x053e: MOVEQ - 0x1,D0
0x0540: PUSH.W - D0
0x0542: PEA - VAR(0x1c4)
0x0546: PUSH.L - 0x000003fc
0x054c: PUSH.W - VAR(0x2c8)
0x0550: JSR - "ZBASIC_135"

0x0554: MOVEQ - 0x1,D0
0x0556: MOVE.W - D0,-0x914(A5)
0x055a: PEA - VAR(0x2ce)
0x055e: MOVE.W - VAR(0x2ca),D0
0x0562: EXT.L - D0
0x0564: POP.L - A0
0x0566: JSR - "ZBASIC_163"

0x056a: MOVEQ - 0x1,D0
0x056c: JSR - "CLOSE"

0x0570: CLR.L - -0x8ee(A5)
0x0574: PEA - VAR(0x76)
0x0578: LEA - VAR(0x2ce),A0
0x057c: POP.L - A1
0x057e: JSR - "ZBASIC_103"

0x0582: PUSH.L - D0
0x0584: MOVE.W - VAR(0x2cc),D0
0x0588: SEQ - D0
0x058a: EXT.W - D0
0x058c: EXT.L - D0
0x058e: POPAND.L - D0
0x0590: TST.L - D0
0x0592: BEQ - [0x05a0]
0x0596: MOVEQ - 0x2,D0
0x0598: MOVE.W - D0,VAR(0x1aa)
0x059c: BRA - [0x05a6]
0x05a0: MOVEQ - 0x0,D0
0x05a2: MOVE.W - D0,VAR(0x1aa)
	*/

		}
		// 129:05a6
		if (this->var_i16_1c + this->var_i16_20 + this->var_i16_1e != 0) {
			g_toolbox->InitCursor();
			zbasic_get_from(0x0, 0x14);
			zbasic_get_to(0x1fc, 0x152, this->arr_i32_b54);
			zbasic_unk6(0x19, 0x64, 0xff, 0x00);
			this->sub_128_2f0(2, 0x64, 0x7e, 0xd8, 0x17a);
			this->sub_128_2f0(1, 0x69, 0x83, 0xd3, 0x175);
			this->sub_128_2f0(2, 0x6e, 0x88, 0xce, 0x170);
			// 129:0662
			zbasic_text(0, 0xc, 0, 0x3);
			this->var_str_76 = STR(0x1ac);
			this->sub_128_2a6(0x82, 0xfc);
			this->var_str_76 = STR(0x1c0);
			this->sub_128_2a6(0x96, 0xfc);

			this->var_str_76 = STR(0x1d8); // start the prologue again
			if (this->var_i16_1aa == 2) {
				this->var_str_76 = STR(0x1f6); // start the finale again
			}
			this->sub_128_2a6(0xaa, 0xfc);

			this->var_str_76 = STR(0x212);
			this->sub_128_2a6(0xcb, 0xfc);
			// 129:0730
			this->sub_128_e58();
			zbasic_get_from(0x0, 0x14);
			zbasic_194(0x0, 0x0, this->arr_i32_b54);
			this->var_i16_1aa = 0;
		} else {
			// 129:075c
			this->sub_128_e80();
		}
		// 129:0772
		zbasic_199(0x81);
		this->sub_130_004();

/*
0x04a0: MOVEQ - 0x1,D0
0x04a2: MOVE.W - D0,VAR(0x180)
0x04a6: MOVEQ - 0x1,D0
0x04a8: MOVE.W - D0,VAR(0x1aa)
0x04ac: PEA - VAR(0x180)
0x04b0: PEA - VAR(0x1c4)
0x04b4: PEA - VAR(0x2c4)
0x04b8: PEA - VAR(0x2c8)
0x04bc: JSR - "ZBASIC_249"
0x04c0: MOVE.W - D0,VAR(0x176)

0x04c4: PEA - VAR(0x1c4)
0x04c8: MOVEA.L - STR(0x16c),A0  # b''
0x04d0: POP.L - A1
0x04d2: JSR - "ZBASIC_106"
0x04d6: TST.L - D0
0x04d8: BNE - [0x04de]
0x04da: JMP - [0x5a6]
--------

0x04de: MOVEA.L - STR(0x16e),A0  # b"And now it is time to show the Finale for the Fool's Errand."
0x04e6: JSR - "BUFFER_CONCAT"
0x04ea: LEA - VAR(0x76),A0
0x04ee: JSR - "BUFFER_FLUSH"
0x04f2: MOVEQ - 0x1,D0
0x04f4: MOVE.W - D0,VAR(0x74)

0x04f8: LEA - VAR(0x76),A0
0x04fc: JSR - "BUFFER_CONCAT"

0x0500: MOVE.W - VAR(0x74),D0
0x0504: EXT.L - D0
0x0506: JSR - "CHR$"
0x050a: JSR - "BUFFER_CONCAT"
0x050e: LEA - VAR(0x76),A0
0x0512: JSR - "BUFFER_FLUSH"

0x0516: MOVEQ - 0xa,D0
0x0518: ADDQ.W - 0x1,VAR(0x74)
0x051c: CMP.W - VAR(0x74),D0
0x0520: BGE - [0x04f8]

0x0522: LEA - VAR(0x76),A0
0x0526: JSR - "LEN"
0x052a: MOVE.W - D0,VAR(0x2ca)

0x052e: MOVEQ - 0x0,D0
0x0530: MOVE.W - D0,VAR(0x2cc)


0x05a6: MOVE.W - VAR(0x1c),D0
0x05aa: EXT.L - D0
0x05ac: MOVEA.W - VAR(0x20),A0
0x05b0: ADD.L - A0,D0
0x05b2: MOVEA.W - VAR(0x1e),A0
0x05b6: ADD.L - A0,D0
0x05b8: CMPI.L - 0x0,D0
0x05be: BNE - [0x05c4]
0x05c0: JMP - [0x75c]
--------
0x05c4: TRAP - _InitCursor
0x05c6: MOVEQ - 0x0,D0
0x05c8: MOVE.L - D0,D1
0x05ca: MOVEQ - 0x14,D0
0x05cc: JSR - "GET_FROM"
0x05d0: PUSH.L - 0x000001fc
0x05d6: PUSH.L - 0x00000152
0x05dc: MOVE.L - HEAP(0xb54),D0
0x05e6: MOVEA.L - D0,A0
0x05e8: POP.L - D0
0x05ea: POP.L - D1
0x05ec: JSR - "GET_TO"

0x05f0: MOVEQ - 0x19,D0
0x05f2: PUSH.W - D0
0x05f4: MOVEQ - 0x64,D0
0x05f6: PUSH.L - D0
0x05f8: PUSH.W - 0xff
0x05fc: PUSH.W - 0
0x05fe: JSR - "ZBASIC_UNK6"

0x0602: PEA - [0x0622]
0x0606: MOVEQ - 0x64,D0
0x0608: PUSH.L - D0
0x060a: PUSH.L - 0x0000007e
0x0610: PUSH.L - 0x000000d8
0x0616: PUSH.L - 0x0000017a
0x061c: MOVEQ - 0x2,D0
0x061e: JMP - 128,0x2f0

0x0622: PEA - [0x0642]
0x0626: MOVEQ - 0x69,D0
0x0628: PUSH.L - D0
0x062a: PUSH.L - 0x00000083
0x0630: PUSH.L - 0x000000d3
0x0636: PUSH.L - 0x00000175
0x063c: MOVEQ - 0x1,D0
0x063e: JMP - 128,0x2f0

0x0642: PEA - [0x0662]
0x0646: MOVEQ - 0x6e,D0
0x0648: PUSH.L - D0
0x064a: PUSH.L - 0x00000088
0x0650: PUSH.L - 0x000000ce
0x0656: PUSH.L - 0x00000170
0x065c: MOVEQ - 0x2,D0
0x065e: JMP - 128,0x2f0

0x0662: PUSH.W - 0
0x0664: MOVEQ - 0xc,D0
0x0666: PUSH.W - D0
0x0668: PUSH.W - 0
0x066a: MOVEQ - 0x3,D0
0x066c: PUSH.W - D0
0x066e: JSR - "TEXT"

0x0672: MOVEA.L - STR(0x1ac),A0  # b'Set your monitor to'
0x067a: JSR - "BUFFER_CONCAT"
0x067e: LEA - VAR(0x76),A0
0x0682: JSR - "BUFFER_FLUSH"
0x0686: PEA - [0x069a]
0x068a: PUSH.L - 0x000000fc
0x0690: MOVE.L - 0x82,D0
0x0696: JMP - 128,0x2a6

0x069a: MOVEA.L - STR(0x1c0),A0  # b'2 color black and white'
0x06a2: JSR - "BUFFER_CONCAT"
0x06a6: LEA - VAR(0x76),A0
0x06aa: JSR - "BUFFER_FLUSH"
0x06ae: PEA - [0x06c2]
0x06b2: PUSH.L - 0x000000fc
0x06b8: MOVE.L - 0x96,D0
0x06be: JMP - 128,0x2a6

0x06c2: MOVEA.L - STR(0x1d8),A0  # b'and start the Prologue again.'
0x06ca: JSR - "BUFFER_CONCAT"
0x06ce: LEA - VAR(0x76),A0
0x06d2: JSR - "BUFFER_FLUSH"
0x06d6: CMPI.W - 0x2,VAR(0x1aa)
0x06dc: BNE - [0x06f4]
0x06e0: MOVEA.L - STR(0x1f6),A0  # b'and start the Finale again.'
0x06e8: JSR - "BUFFER_CONCAT"
0x06ec: LEA - VAR(0x76),A0
0x06f0: JSR - "BUFFER_FLUSH"
0x06f4: PEA - [0x0708]
0x06f8: PUSH.L - 0x000000fc
0x06fe: MOVE.L - 0xaa,D0
0x0704: JMP - 128,0x2a6

0x0708: MOVEA.L - STR(0x212),A0  # b'(click mouse to quit)'
0x0710: JSR - "BUFFER_CONCAT"
0x0714: LEA - VAR(0x76),A0
0x0718: JSR - "BUFFER_FLUSH"
0x071c: PEA - [0x0730]
0x0720: PUSH.L - 0x000000fc
0x0726: MOVE.L - 0xcb,D0
0x072c: JMP - 128,0x2a6

0x0730: JSR - 128,0xe58
0x0734: MOVEQ - 0x0,D0
0x0736: MOVE.L - D0,D1
0x0738: MOVEQ - 0x14,D0
0x073a: JSR - "GET_FROM"

0x073e: PUSH.W - 0
0x0740: MOVE.L - HEAP(0xb54),D0
0x074a: PUSH.L - D0
0x074c: MOVEQ - 0x0,D0
0x074e: JSR - "ZBASIC_194"
0x0752: MOVEQ - 0x0,D0
0x0754: MOVE.W - D0,VAR(0x1aa)
0x0758: JMP - [0x772]
--------


0x075c: JSR - 128,0xe80
0x0760: JMP - [0x772]
--------
0x0764: MOVEQ - 0x1,D0
0x0766: MOVE.W - D0,VAR(0x2cc)
0x076a: MOVEQ - 0x0,D0
0x076c: JSR - "ZBASIC_130"
0x0770: RTS
--------
0x0772: MOVE.W - 0x81,D1
0x0776: JMP - "ZBASIC_199"
0x077a: JMP - 130,0x4
 */
	}
}

void FoolPrologue::sub_129_764() {
	this->var_i16_2cc = 1;
	zbasic_130(0);
}


void FoolPrologue::sub_130_004() {
	this->var_i16_3ce = 5;
	this->sub_130_d28();
	this->var_i16_18e = 0x156;
	this->sub_128_ccc();

	this->glob_i32_2ce = g_toolbox->GetPicture(0x47);
	this->var_i16_3ce = 0xa;
	this->sub_130_d28();
	this->sub_128_1f4(0xb);
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0x74, 0xaa, 0x00, 0x00, this->glob_i32_2ce);
	g_toolbox->ReleaseResource(*this->glob_i32_2ce);

	// 130:007a
	zbasic_text(0xfb, 0x9, 0x0, 0x3);
	this->var_str_76 = STR(0x228);
	this->sub_128_2a6(0x10d, 0xdd);
	g_toolbox->SetPortBits(&this->var_i32_32);
	for (int i = 1; i < 5; i++) {
		this->sub_130_cea();
	}
	this->sub_128_1f4(7);

	// 130:00d6
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0, 0x96, 0, this->arr_u8_4);
	g_toolbox->ReleaseResource(*this->arr_u8_4);
	this->sub_128_1f4(8);

	// 130:011c
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0, 0x96, 0, this->arr_u8_8);
	g_toolbox->ReleaseResource(*this->arr_u8_8);

	// 130:015c
	for (int i = 3; i < 5; i++) {
		zbasic_blockmove(this->arr_i32_412b2[0], this->arr_i32_41296[i], 0x5580);
	}
	this->sub_130_cea();
	this->sub_128_1f4(0);

	// 130:01aa
	zbasic_picture(0, 0, 0, this->arr_u8_18);
	g_toolbox->ReleaseResource(*this->arr_u8_18);
	this->sub_130_cea();

	zbasic_blockmove(this->arr_i32_41296[0], this->arr_i32_41296[1], 0x5580);
	this->sub_128_1f4(1);

	// 130:0202
	zbasic_picture(0xa2, 0x3c, 0, this->arr_u8_1c);
	g_toolbox->ReleaseResource(*this->arr_u8_1c);
	this->sub_130_cea();
	this->sub_128_1f4(2);

	// 130:0234
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0x5a, 0x2e, 0, this->arr_u8_20);
	g_toolbox->ReleaseResource(*this->arr_u8_20);

	// 130:0276
	for (int j = 9; j < 0xb; j++) {
		this->sub_130_cea();

		if (j == 9) {
			this->var_i16_74 = 0;
		} else {
			this->var_i16_74 = 0x81;
		}

		// 130:029a
		this->sub_128_1f4(j - 6);
		zbasic_picture(0, 0, 0, this->arr_i32_0[j]);
		zbasic_picture(this->var_i16_74, 0x96, 0, this->arr_i32_0[j - 6]);
		g_toolbox->ReleaseResource(*this->arr_i32_0[j]);
		g_toolbox->ReleaseResource(*this->arr_i32_0[j - 6]);
	}
	this->sub_130_cea();

	// 130:0320
	this->sub_128_1f4(6);
	zbasic_picture(0, 0, 0, this->arr_i32_0[0xc]);
	g_toolbox->ReleaseResource(*this->arr_i32_0[0xc]);

	// 130:034a
	for (int i = 0xd; i < 0x12; i++) {
		this->sub_130_cea();
	}

	this->arr_i32_120[0] = g_toolbox->GetPicture(0x48);
	g_toolbox->SetPortBits(&this->var_i32_32);

	// 130:0380
	if ((this->var_i16_10 == 0x200) && (this->var_i16_12 == 0x156)) {
		this->sub_128_2f0(1, 0, 0, 0x14, 0x1fc);
	} else {
		g_toolbox->SetPort(this->var_i32_c);
		this->sub_128_2f0(2, 0, 0, 0x14, this->var_i16_10);
		g_toolbox->SetPort(this->var_i32_8);
	}
	// 130:03e4
	this->sub_128_21e(0x3c);
	this->sub_128_64a(0x2);
	this->sub_128_21e(0x1e);
	this->sub_128_610(0xb);
	this->sub_128_21e(0x5a);
	this->sub_130_db0();
	g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, 0x200, 0x156);
	g_toolbox->SetRect(this->arr_i32_1c4, 0, 0x96, 0x200, 0x118);

	// 130:043c
	for (int i = 1; i < 2; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->sub_128_a8c(0xb4);
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_i32_1bc);
		this->sub_128_24a(1);
		g_toolbox->InvertRect(this->arr_i32_1bc);
		// 130:04ca
		for (int j = 0; j < 3; j++) {
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_412b2[1];
			// 130:049e
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);
			this->sub_128_a8c(4);
			this->var_i16_192 = 0;
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = *this->arr_i32_412a2;
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);
			this->sub_128_a8c(5);
			// 130:0518
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = *this->arr_i32_412a6;
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);
			this->sub_128_a8c(5);
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = *this->arr_i32_412aa;
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);
			this->sub_128_a8c(5);
		}
		// 130:05b6
		this->var_i32_40 = *this->arr_i32_412b2;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);

	}

	// 130:05fc
	this->sub_128_610(0);
	this->sub_130_db0();
	this->var_i32_2 = g_toolbox->TickCount();
	this->sub_128_a8c(0xb4);
	this->var_i32_40 = *this->arr_i32_4129a;

	// 130:062c
	this->var_i16_74 = 0;
	while (this->var_i16_74 != 0x156) {
		this->var_i16_62 = 0x200;
		this->var_i32_2 = g_toolbox->TickCount();
		for (int j = 1; j < 0x7; j++) {
			this->var_i16_192 = 1;
			if (this->var_i16_74 < 0x156) {
				this->var_i16_74 += 1;
			}
			this->var_i16_5c = this->arr_i16_412ea[this->var_i16_74] - 1;
			this->var_i16_60 = this->arr_i16_412ea[this->var_i16_74];
			// 130:068e
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_5c, this->arr_i32_5c, 0, 0);
		}
		// 130:06b0
		this->sub_128_a8c(1);
	}
	this->var_i32_2 = g_toolbox->TickCount();
	this->sub_128_a8c(0xa);
	this->sub_128_610(1);
	// 130:06d4
	for (int i = 1; i < 4; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_i32_1bc);
		this->sub_128_a8c(0x2);
	}
	// 130:0704
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->InvertRect(this->arr_i32_1bc);
	this->sub_128_a8c(0xf);
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->InvertRect(this->arr_i32_1bc);
	this->sub_128_a8c(0x78);
	this->sub_128_610(2);
	this->sub_130_db0();
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->SetRect(this->arr_i32_1c4, 0, 0, 0x150, 0x96);

	// 130:0778
	for (int j = 0; j < 8; j++) {
		for (int i = 3; i < 5; i++) {
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[i];
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, 0, 0);
			this->sub_128_a8c(3);
		}
	}
	// 130:07ec
	this->sub_128_610(6);
	g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, 0x200, 0x156);
	for (int i = 0; i < 0x2d; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->sub_128_24a(1);
	}
	// 130:083c
	zbasic_picture(0, 0, 0, this->arr_i32_34);
	zbasic_picture(5, 0, 0, this->arr_i32_38);
	g_toolbox->ReleaseResource(*this->arr_i32_34);
	g_toolbox->ReleaseResource(*this->arr_i32_38);
	this->var_i32_2 = g_toolbox->TickCount();
	// 130:0890
	zbasic_52(0);
	this->sub_128_1f4(0);
	this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0, 0x17, 0, this->arr_i32_3c);
	g_toolbox->ReleaseResource(*this->arr_i32_3c);
	// 130:08dc
	this->sub_130_f48();
	this->sub_128_de2();
	this->sub_128_1f4(1);
	this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0xf0, 0xba, 0, this->arr_i32_40);
	g_toolbox->ReleaseResource(*this->arr_i32_40);
	this->sub_130_f48();
	this->sub_128_de2();
	// 130:0934
	for (int i = 2; i < 5; i++) {
		zbasic_blockmove(*this->arr_i32_4129a, this->arr_i32_41296[i], 0x5580);
	}
	// 130:0978
	for (int i = 1; i < 5; i++) {
		this->sub_128_1f4(i);
		g_toolbox->PenPat(this->arr_i32_19c);
		g_toolbox->SetRect(this->arr_i32_1bc, 0x3c, 0x3c, 0x8c, 0x8c);
		g_toolbox->FrameOval(this->arr_i32_1bc);
		g_toolbox->PenPat(this->arr_i16_1b4);
		// 130:09d0
		g_toolbox->InsetRect(this->arr_i32_1bc, -i, -i);
		for (int j = 2; j < 0xd; j++) {
			g_toolbox->InsetRect(this->arr_i32_1bc, -j, -j);
			g_toolbox->FrameOval(this->arr_i32_1bc);
		}
	}
	// 130:0a42
	this->sub_128_1f4(6);
	this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
	zbasic_picture(0x166, 0x77, 0, this->arr_i32_44);
	this->sub_128_de2();
	zbasic_blockmove(*this->arr_i32_412ae, *this->arr_i32_412b2, 0x5580);
	this->sub_130_f48();
	g_toolbox->ReleaseResource(*this->arr_i32_44);
	this->sub_128_1f4(7);
	// 130:0ac2
	zbasic_picture(-0x14, -0x1e, 0, this->arr_i32_48);
	this->sub_128_1f4(8);
	zbasic_blockmove(*this->arr_i32_412b2, *this->arr_i32_412b6, 0x5580);
	this->sub_128_2f0(0, 0x73, 0x15f, 0xf6, 0x1dc);
	zbasic_picture(0x16d, 0x80, 0, this->arr_i32_120);
	g_toolbox->ReleaseResource(*this->arr_i32_120);
	g_toolbox->SetPortBits(&this->var_i32_32);
	// 130:0b74
	this->sub_128_24a(0xd2);
	this->sub_128_e80();
	this->sub_128_64a(1);
	this->sub_128_21e(0x14);
	this->sub_128_6e4(0);
	this->sub_128_e58();
	// 130:0b98

}

} // End of namespace Fool
