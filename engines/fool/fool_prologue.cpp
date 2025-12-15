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


#include "fool/fool_prologue.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"
#include "common/str.h"

namespace Fool {

ZBasic *g_zbasic;
Toolbox *g_toolbox;

void FoolPrologue::run() {

	// 128:0004
	g_zbasic->unk_176();
	g_toolbox->ClearMenuBar();
	g_toolbox->HideCursor();
	g_toolbox->UseResFile(g_toolbox->CurResFile());

	// 128:001e
	// b54 must be the 1-bit screen double buffer?
	for (int i = 0; i < 12; i++) {
		this->arr_i32_41296[i] = &this->arr_i32_b54[SCREEN_PAGE_SIZE*i];

	}
	// 128:0066
	this->var_i32_4e = this->arr_i32_412c2[0];
	// 128:0078
	// stipple pattern buffer?
	for (int i = 0; i < 3; i++) {
		this->arr_i16_194[i] = 0;
		this->arr_i16_194[i+4] = 0xaa55;
		this->arr_i16_194[i+8] = 0xffff;
	}

	// 128:00d6
	this->arr_i16_1ac[0] = 0x0107;
	this->arr_i16_194[0xd] = 0x0e0c;
	this->arr_i16_194[0xe] = 0x7c70;
	this->arr_i16_194[0xf] = 0xc0c0;
	this->arr_i16_1b4[0] = 0x8800;
	this->arr_i16_194[0x11] = 0x2200;
	this->arr_i16_194[0x12] = 0x8800;
	this->arr_i16_194[0x13] = 0x2200;

	// 128:017a
	this->var_str_76 = Common::String(_("© 1987 by Cliff Johnson ALL RIGHTS RESERVED"));
	g_zbasic->buffer_flush(this->var_str_76);
	this->var_str_76 = Common::String(_("This software was created using the ZBasic™ compiler."));
	g_zbasic->buffer_flush(this->var_str_76);
	this->var_str_76 = Common::String(_("Portions of this code are © Copyrighted, 1985 by Zedcor Inc."));
	g_zbasic->buffer_flush(this->var_str_76);

	// 128:01b6
	// JMP 0x1f0
	// JMP 0x21a
	// JMP 0x246
	// JMP 0x268
	// JMP 0x2a2
	// JMP 0x2ec
	// JMP 0x350
	// JMP 0x3ea
	// JMP 0x506
	// JMP 0x60c
	// JMP 0x646
	// JMP 0x6e0
	// JMP 0x7fc
	// JMP 0xa68
	// JMP 0xa88
	// JMP 0xc8a
	// JMP 129:0x4
	this->sub_129_004();
}

void FoolPrologue::sub_128_1ba(int16_t screen_page) {
	// 128:01ba
	this->var_40 = &this->arr_i32_41296[screen_page];
	g_toolbox->CopyBits(this->var_32, this->var_40, this->var_38, this->var_38, 0, 0);
}

void FoolPrologue::sub_128_1f4(int16_t screen_page) {
	// 128:01f4
	this->var_40 = &this->arr_i32_41296[screen_page];
	g_toolbox->SetPortBits(this->var_40);
}

void FoolPrologue::sub_128_21e(int16_t numTicks) {
	// 128:021e
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	g_toolbox->Delay(numTicks);
}

void FoolPrologue::sub_128_24a(int16_t numTicks) {
	// 128:024a
	int32_t delay = MAX(this->var_i32_2 + numTicks + 1 - g_toolbox->TickCount(), 0);
	// again, polling TickCount in a loop.
	g_toolbox->Delay(delay);
}

void FoolPrologue::sub_128_26c(int16_t unk1, int16_t unk2) {
	// 128:026c
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(this->var_i32_182 - unk2, unk1);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::sub_128_2a6(int16_t unk1, int16_t unk2) {
	// 128:02a6
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(unk2 - (this->var_i32_182 / 2), unk1);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::sub_128_2f0(int16_t unk, int16_t right, int16_t bottom, int16_t left, int16_t top) {
	// 128:02f0
	g_toolbox->SetRect(this->rect_41af4, left, top, right, bottom);
	g_toolbox->FillRect(this->rect_41af4, &this->arr_i16_194[unk*4])
}

void FoolPrologue::sub_128_354(int16_t unk1, int16_t unk2) {
	// 128:0354
	g_toolbox->SetRect(this->rect_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenPat(this->arr_i16_194[unk2*4]);
	g_toolbox->PenMode(unk1);
	g_toolbox->PenSize(6, 4);

	for (int i = 0; i <= 130 i += 3) {
		this->var_i32_2 = g_toolbox->TickCount();

		g_toolbox->FrameRect(this->rect_1bc);
		g_toolbox->InsetRect(this->rect_1bc, 6, 4);
		this->sub_128_24a(1);
	}
	g_toolbox->PenNormal();
}

void FoolPrologue::sub_128_3ee(int16_t unk1) {
	// 128:03ee
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
	// 128:0428
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

void FoolPrologue::sub_128_50a(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4) {
	// 128:050a
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
	// 128:0584
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

void FoolPrologue::sub_128_610(int16_t unk) {
	// 128:0610
	this->var_i32_40 = this->arr_i32_41296[unk];
	g_toolbox->CopyBits(this->var_i32_40, this->var_32, this->var_38, this->var_38, 0, 0);
}

void FoolPrologue::sub_128_64a(int16_t unk1) {
	// 128:064a
	g_toolbox->PenNormal();
	g_toolbox->PenPat(&this->arr_i16_194[unk1*4]);
	// 128:0668
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

void FoolPrologue::sub_128_6e4(int16_t unk) {
	// 128:06e4
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

void FoolPrologue::sub_128_800(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4, int16_t unk5, int16_t unk6, int16_t unk7, int16_t unk8, int16_t unk9) {
	// 128:0800
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
	// 128:08bc
	for (int i = 0; i < 4; i++) {
		this->arr_f64_41bbe[i] = (double)this->arr_i16_41af4[i];
		this->arr_f64_41bbe[i+4] = (double)(this->arr_i16_41af4[i] - this->arr_i16_41af4[i+4])/(double)(this->var_i32_182);
	}
	// 128:096e
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

void FoolPrologue::sub_128_a6c(int16_t unk1, int16_t unk2) {
	// 128:0a6c
	g_toolbox->MoveTo(unk2, unk1);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::sub_128_a8c(int16_t unk) {
	// 128:0a8c
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

		    // 128:0bae
			this->arr_i16_1e8[this->var_i16_6] = _vm->_rnd->getRandomNumber(0x264) - 0xc8;
			this->arr_i16_1e8[0xfb + this->var_i16_6] = _vm->_rnd->getRandomNumber(0x1ba) - 0xc8;
			this->arr_i16_1e8[0x1f6 + this->var_i16_6] = _vm->_rnd->getRandomNumber(0x5) + 0x1;
			this->arr_i16_1e8[0x2f1 + this->var_i16_6] = _vm->_rnd->getRandomNumber(0xa) + 0x19;
		}
		// 128:0c56
		this->sub_130_e82();
		this->var_i16_6 += 1;
		if (this->var_i16_6 > 0xb5) {
			this->var_i16_6 = 0;
		}
	} while (this->var_i32_2 + this->var_i16_1a4 <= g_toolbox->TickCount())
}

void FoolPrologue::sub_128_c8a() {
	// 128:0c8a
	this->sub_129_004();
	this->var_i32_1a6 = g_zbasic->mem(-1);
	if (this->var_i16_1aa == 1) {
		this->sub_130_004();
		this->var_i32_1a6 = g_zbasic->mem(-1);
	}
	// 128:0cb0
	if (this->var_i16_1aa == 2) {
		this->sub_131_004();
		this->var_i32_1a6 = g_zbasic->mem(-1);
	}
	// 128:0cc8
	g_zbasic->unk4();
	g_zbasic->20();
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
		g_zbasic->blockmove(this->var_i16_1b4 - this->var_i16_1b8, this->var_i32_1ac + this->var_i16_1b8, this->var_i32_1b0 + this->var_i16_1b8);
		g_zbasic->blockmove(this->var_i16_1b4 - this->var_i16_1b8, this->var_i32_1b0 + this->var_i16_1b8, this->var_i32_1ac + this->var_i16_1b8);

	}
}

void FoolPrologue::sub_128_de2() {
	// 128:0de2
	g_zbasic->text(0xfb, 0x9, 0x0, 0x2);
	this->var_str_76 = STR(0xa0); // "Click Mouse to Continue"
	this->sub_128_a6c(0x151, 0x5);
}

void FoolPrologue::sub_128_e1c() {
	// 128:0e1c
	g_zbasic->text(0xfb, 0x9, 0x0, 0x2);
	this->var_str_76 = "Click Mouse to Continue"; // STR(0xb8)
	this->sub_128_26c(0x154, 0x1f7);
}

void FoolPrologue::sub_128_e58() {
	// 128:0e58
	this->sub_128_e80();
	while (true) {
		this->var_i16_1ba = g_toolbox->GetNextEvent(0x2, &this->var_22);
		if (this->var_22 == 1)
			break;
	}
	this->sub_128_e80();
}

void FoolPrologue::sub_128_e80() {
	// 128:0e80
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

void FoolPrologue::sub_128_ed2() {
	// 128:0ed2
	g_toolbox->BeginUpdate(this->var_window_24);
	g_toolbox->EndUpdate(this->var_window_24);
}

void FoolPrologue::sub_128_ee0() {
	// 128:0ee0
	// done by using PEEKWORD into fixed offsets of the window pointer
	this->var_i16_1bc = this->var_window_24.port; // +0
	this->var_i16_1be = this->var_window_24.windowKind; // +2
	g_zbasic->unk11(this->var_i16_1be);
}

void FoolPrologue::sub_128_f0a() {
	g_zbasic->199(0x80); // switch CODE resource?
	this->sub_129_004();
}

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

void FoolPrologue::sub_129_004() {
	// get the screen width and height from QuickDraw
	// 0x0004: MOVE.L - 0x904,D0
	// 0x000a: JSR - "PEEKLONG"   # current A5
	// 0x000e: SUBI.L - 0x72,D0
	// 0x0014: JSR - "PEEKWORD" # quickdraw globals,
	this->var_i16_10 = _vm->_wm->_screenDims.width();  // window width?
	this->var_i16_12 = _vm->_wm->_screenDims.height(); // window height?

	// 129:0034
	// set left and top offsets based on a drawable area of 512x342
	this->var_i16_14 = (this->var_i16_10 - SCREEN_WIDTH)/2;
	this->var_i16_16 = (this->var_i16_12 - SCREEN_HEIGHT)/2;

	// 129:0064
	g_zbasic->window(1, "", 0, 0, this->var_i16_10, this->var_i16_12, 3);
	g_zbasic->coordinate_window();

	this->var_i16_1c = 0;
	this->var_i16_1e = 0;
	this->var_i16_20 = 0;

	// code checks Rom85 for presence of 128K ROM before doing next bit
	// 129:00a2
	if (true) {
		this->var_i16_1c = 0x4e20;
		this->var_i16_1e = 0x4e20;
		this->var_i16_20 = 0x4e20;
		g_toolbox->SetCPixel(0x64, 0x64, &this->var_i16_1c);
		g_toolbox->GetCPixel(0x64, 0x64, &this->var_i16_1c);
	}

	// 129:00ee
	this->var_i32_c = this->arr_i21_a8a[0];
	g_toolbox->OpenPort(this->var_i32_c);
	this->var_i32_8 = this->arr_i21_9c0[0];
	g_toolbox->OpenPort(this->var_i32_8);

	if ((this->var_i16_10 != SCREEN_WIDTH || this->var_i16_12 != SCREEN_HEIGHT)) {
		// 129:013a
		this->sub_128_2f0(0x2, 0x14, 0, this->var_i16_12, this->var_i16_10);
		g_toolbox->SetRect(&this->arr_i32_1bc, this->var_i16_14 - 2, this->var_i16_16 - 2, this->var_i16_14 + SCREEN_WIDTH + 2, this->var_i16_16 + SCREEN_HEIGHT + 2);
		g_toolbox->PenPat(this->arr_i32_19c[0]);
		g_toolbox->FrameRect(&this->arr_i32_1bc);
	}

	// 129:01b0
	g_toolbox->SetPort(this->var_i32_8);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_14, this->var_i16_16);
	g_toolbox->SetRect(&this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->ClipRect(&this->arr_i32_1bc);

	// 129:01f2
	if (this->var_i16_12 == SCREEN_HEIGHT) {
		g_toolbox->SetRect(&this->arr_i32_1bc, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(&this->arr_i32_1bc, this->arr_i32_19c);
	} else {
		g_toolbox->SetRect(&this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(&this->arr_i32_1bc, this->arr_i32_19c);
	}

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

	this->var_i32_1c0 = g_zbasic->mem(-1);

	// 129:02fe
	if (this->var_i32_1c0 < 0xd6d8) {
		g_toolbox->InitCursor();
		g_zbasic->get_from(0x0, 0x14);
		g_zbasic->get_to(0x1fc, 0x152, this->arr_i32_b54);
		g_zbasic->unk6(0x19, 0x64, 0xff, 0x0);
		this->sub_128_2f0(2, 0x46, 0x64, 0xd8, 0x198);
		this->sub_128_2f0(1, 0x4b, 0x69, 0xd3, 0x193);

		// 129:0386
		this->sub_128_2f0(2, 0x50, 0x6e, 0xc3, 0x18e);
		g_zbasic->text(0, 0xc, 0, 3);
		this->var_str_76 = STR(0xd2); // 'There is not enough available memory'
		this->sub_128_2a6(0x64, 0xfc);
		this->var_str_76 = STR(0xf8); // 'to run the program at this time.'
		this->sub_128_2a6(0x78, 0xfc);
		this->var_str_76 = STR(0x11a); // 'Check your instruction manual for '
		this->sub_128_2a6(0x96, 0xfc);
		this->var_str_76 = STR(0x13e); // 'for possible solutions.'
		this->sub_128_2a6(0xaa, 0xfc);
		this->var_str_76 = STR(0x156); // '(click mouse to quit)'
		this->sub_128_2a6(0xcb, 0xfc);
		this->sub_128_e58();
		g_zbasic->get_from(0x0, 0x14);

		g_zbasic->194(0, 0, this->arr_i32_b54);
		this->var_i16_1aa = 0;

	} else {
		// 129:04a0
		this->var_i16_180 = 1;
		this->var_i16_1aa = 1;
		this->var_i16_176 = g_zbasic->249(&this->var_i16_180, &this->var_i16_1c4, &this->var_i16_2c4, &this->var_i16_2c8);
		if (g_zbasic->106(STR(0x16c), &this->var_i16_1c4)) {
			// 129:04de
			this->var_str_76 = STR(0x16e); // "And now it is time to show the Finale for the Fool's Errand."
			for (int i = 1; i < 0xa; i++) {
				this->var_str_76 += g_zbasic->chr(this->var_i16_74);
			}

			// 129:0522
			this->var_i16_2ca = this->var_str_76.size();
			this->var_i16_2cc = 0;
			// 129:0534
			// what is this???
			// 0x0534: LEA - [0x0764],A0
			// 0x0538: MOVE.L - A0,-0x8ee(A5)
			// 0x053c: SF - 0x8,D0
			g_zbasic->135(1, &this->var_i16_1c4, 0x3fc, this->var_i16_2c8);

			// 0x0554: MOVEQ - 0x1,D0
			// 0x0556: MOVE.W - D0,-0x914(A5)
			g_zbasic->163(&this->var_i16_2ce, this->var_i16_2ca);
			g_zbasic->close(1);
			// 0x0570: CLR.L - -0x8ee(A5)
			// 128:0582
			if (g_zbasic->103(&this->var_i16_2ce, &this->var_str_76) &&
				this->var_i16_2cc == 0) {
				this->var_i16_1aa = 2;
			} else {
				this->var_i16_1aa = 0;
			}

		}
		// 129:05a6
		if (this->var_i16_1c + this->var_i16_20 + this->var_i16_1e != 0) {
			g_toolbox->InitCursor();
			g_zbasic->get_from(0x0, 0x14);
			g_zbasic->get_to(0x1fc, 0x152, this->arr_i32_b54);
			g_zbasic->unk6(0x19, 0x64, 0xff, 0x00);
			this->sub_128_2f0(2, 0x64, 0x7e, 0xd8, 0x17a);
			this->sub_128_2f0(1, 0x69, 0x83, 0xd3, 0x175);
			this->sub_128_2f0(2, 0x6e, 0x88, 0xce, 0x170);
			// 129:0662
			g_zbasic->text(0, 0xc, 0, 0x3);
			this->var_str_76 = STR(0x1ac); // 'Set your monitor to'
			this->sub_128_2a6(0x82, 0xfc);
			this->var_str_76 = STR(0x1c0); // '2 color black and white'
			this->sub_128_2a6(0x96, 0xfc);

			this->var_str_76 = STR(0x1d8); // 'and start the Prologue again.'
			if (this->var_i16_1aa == 2) {
				this->var_str_76 = STR(0x1f6); // 'and start the Finale again.'
			}
			this->sub_128_2a6(0xaa, 0xfc);

			this->var_str_76 = STR(0x212); // '(click mouse to quit)'
			this->sub_128_2a6(0xcb, 0xfc);
			// 129:0730
			this->sub_128_e58();
			g_zbasic->get_from(0x0, 0x14);
			g_zbasic->194(0x0, 0x0, this->arr_i32_b54);
			this->var_i16_1aa = 0;
		} else {
			// 129:075c
			this->sub_128_e80();
		}
		// 129:0772
		g_zbasic->199(0x81);
		this->sub_130_004();

	}
}

void FoolPrologue::sub_129_764() {
	// 129:0764
	this->var_i16_2cc = 1;
	g_zbasic->unk_130(0);
}

void FoolPrologue::sub_129_772() {
	g_zbasic->unk_199(0x81);
	this->sub_130_004();
}


void FoolPrologue::sub_130_004() {
	// 130:0004
	this->var_i16_3ce = 5;
	this->sub_130_d28();
	this->var_i16_18e = 0x156;
	this->sub_128_ccc();

	this->glob_i32_2ce = g_toolbox->GetPicture(0x47);
	this->var_i16_3ce = 0xa;
	this->sub_130_d28();
	this->sub_128_1f4(0xb);
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0x74, 0xaa, 0x00, 0x00, this->glob_i32_2ce);
	g_toolbox->ReleaseResource(*this->glob_i32_2ce);

	// 130:007a
	g_zbasic->text(0xfb, 0x9, 0x0, 0x3);
	this->var_str_76 = STR(0x228);
	this->sub_128_2a6(0x10d, 0xdd);
	g_toolbox->SetPortBits(&this->var_i32_32);
	for (int i = 1; i < 5; i++) {
		this->sub_130_cea();
	}
	this->sub_128_1f4(7);

	// 130:00d6
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0, 0x96, 0, this->arr_u8_4);
	g_toolbox->ReleaseResource(*this->arr_u8_4);
	this->sub_128_1f4(8);

	// 130:011c
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0, 0x96, 0, this->arr_u8_8);
	g_toolbox->ReleaseResource(*this->arr_u8_8);

	// 130:015c
	for (int i = 3; i < 5; i++) {
		g_zbasic->blockmove(this->arr_i32_412b2[0], this->arr_i32_41296[i], 0x5580);
	}
	this->sub_130_cea();
	this->sub_128_1f4(0);

	// 130:01aa
	g_zbasic->picture(0, 0, 0, this->arr_u8_18);
	g_toolbox->ReleaseResource(*this->arr_u8_18);
	this->sub_130_cea();

	g_zbasic->blockmove(this->arr_i32_41296[0], this->arr_i32_41296[1], 0x5580);
	this->sub_128_1f4(1);

	// 130:0202
	g_zbasic->picture(0xa2, 0x3c, 0, this->arr_u8_1c);
	g_toolbox->ReleaseResource(*this->arr_u8_1c);
	this->sub_130_cea();
	this->sub_128_1f4(2);

	// 130:0234
	this->sub_128_2f0(2, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0x5a, 0x2e, 0, this->arr_u8_20);
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
		g_zbasic->picture(0, 0, 0, this->arr_i32_0[j]);
		g_zbasic->picture(this->var_i16_74, 0x96, 0, this->arr_i32_0[j - 6]);
		g_toolbox->ReleaseResource(*this->arr_i32_0[j]);
		g_toolbox->ReleaseResource(*this->arr_i32_0[j - 6]);
	}
	this->sub_130_cea();

	// 130:0320
	this->sub_128_1f4(6);
	g_zbasic->picture(0, 0, 0, this->arr_i32_0[0xc]);
	g_toolbox->ReleaseResource(*this->arr_i32_0[0xc]);

	// 130:034a
	for (int i = 0xd; i < 0x12; i++) {
		this->sub_130_cea();
	}

	this->arr_i32_120[0] = g_toolbox->GetPicture(0x48);
	g_toolbox->SetPortBits(&this->var_i32_32);

	// 130:0380
	if ((this->var_i16_10 == SCREEN_WIDTH) && (this->var_i16_12 == 0x156)) {
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
	g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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
		this->var_i16_62 = SCREEN_WIDTH;
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
	g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int i = 0; i < 0x2d; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->sub_128_24a(1);
	}
	// 130:083c
	g_zbasic->picture(0, 0, 0, this->arr_i32_34);
	g_zbasic->picture(5, 0, 0, this->arr_i32_38);
	g_toolbox->ReleaseResource(*this->arr_i32_34);
	g_toolbox->ReleaseResource(*this->arr_i32_38);
	this->var_i32_2 = g_toolbox->TickCount();
	// 130:0890
	g_zbasic->52(0);
	this->sub_128_1f4(0);
	this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0, 0x17, 0, this->arr_i32_3c);
	g_toolbox->ReleaseResource(*this->arr_i32_3c);
	// 130:08dc
	this->sub_130_f48();
	this->sub_128_de2();
	this->sub_128_1f4(1);
	this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
	g_zbasic->picture(0xf0, 0xba, 0, this->arr_i32_40);
	g_toolbox->ReleaseResource(*this->arr_i32_40);
	this->sub_130_f48();
	this->sub_128_de2();
	// 130:0934
	for (int i = 2; i < 5; i++) {
		g_zbasic->blockmove(*this->arr_i32_4129a, this->arr_i32_41296[i], 0x5580);
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
	g_zbasic->picture(0x166, 0x77, 0, this->arr_i32_44);
	this->sub_128_de2();
	g_zbasic->blockmove(*this->arr_i32_412ae, *this->arr_i32_412b2, 0x5580);
	this->sub_130_f48();
	g_toolbox->ReleaseResource(*this->arr_i32_44);
	this->sub_128_1f4(7);
	// 130:0ac2
	g_zbasic->picture(-0x14, -0x1e, 0, this->arr_i32_48);
	this->sub_128_1f4(8);
	g_zbasic->blockmove(*this->arr_i32_412b2, *this->arr_i32_412b6, 0x5580);
	this->sub_128_2f0(0, 0x73, 0x15f, 0xf6, 0x1dc);
	g_zbasic->picture(0x16d, 0x80, 0, this->arr_i32_120);
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
	this->var_i16_74 = 0;
	this->var_i16_192 = 1;
	while (this->var_i16_22 != 1) {
		// 130:0ba4
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 += this->var_i16_192;
		// 130:0bbe
		if (this->var_i16_74 == 0x5) {
			this->var_i16_192 = -1;
		}
		if (this->var_i16_74 == 1) {
			this->var_i16_192 = 1;
		}
		this->sub_128_610(this->var_i16_74);
		this->sub_128_24a(0xa);
		// 130:0bee
		this->var_i16_1ba = g_toolbox->GetNextEvent(2, &this->var_i16_22);
	}
	while (!((this->var_i16_22 == 0) && (this->var_i16_30 & 0x80))) {
		// 130:0c0c
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 += this->var_i16_192;
		// 130:0c26
		if (this->var_i16_74 == 0x5) {
			this->var_i16_192 = -1;
		}
		if (this->var_i16_74 == 1) {
			this->var_i16_192 = 1;
		}
		this->sub_128_610(this->var_i16_74);
		this->sub_128_24a(0xa);
		// 130:0c56
		this->var_i16_1ba = g_toolbox->GetNextEvent(-1, &this->var_i16_22);
		if (this->var_i16_22 == 6) {
			this->sub_128_ed2();
		}
		if (this->var_i16_22 == 7) {
			this->sub_128_ee0();
		}
	}
	// 130:0ca8
	this->sub_128_610(0x6);
	this->sub_128_e58();
	this->sub_128_50a(0, 0x7, 0, 0x1fc);
	this->sub_128_21e(0xa);
	this->sub_128_610(0x8);
	this->sub_128_e58();
	this->sub_128_354(8, 1);
	// 130:0ce6
	// JMP 1002
	g_zbasic->199(0x82);
	this->sub_131_004();
}

void FoolPrologue::sub_130_cea() {
	// 130:0cea
	this->var_i16_3d4 += 1;
	this->arr_i32_0[this->var_i16_3d4] = g_toolbox->GetPicture(this->var_i16_3d4);
	// 130:0d0a
	this->var_i16_3ce = this->var_i16_3d4 * 5 + 0xa;
	this->sub_130_d28();
}

void FoolPrologue::sub_130_d28() {
	// 130:0d28
	g_toolbox->SetPort(this->var_i32_c);
	g_zbasic->text(0, 0xc, 0, 0x1);
	this->var_str_76 = STR(0x256); // 'Loading Prologue -'
	this->var_str_76 += Common::String::format("%d", this->var_i16_3ce);
	this->var_str_76 += STR(0x26a); // '%'
	// 130:0d70
	this->sub_128_2f0(0, 0, 0x7, 0x13, this->var_i16_10 - 7);
	this->sub_128_2a6(0xe, this->var_i16_10/2);
	g_toolbox->SetPort(this->var_i32_8);
}

void FoolPrologue::sub_130_db0() {
	// 130:0db0
	g_zbasic->20();
	g_toolbox->PenMode(0xa);
	this->var_i16_6 = 0x1;
	while (this->var_i16_6 <= 0xb5) {
		// 130:0dc0
		this->arr_i16_1e8[this->var_i16_6] = _vm->_rnd->getRandomNumber(0x264) - 0x64;
		this->arr_i16_1e8[this->var_i16_6 + 0xfb] = _vm->_rnd->getRandomNumber(0x1ba) - 0x64;
		this->arr_i16_1e8[this->var_i16_6 + 0x1f6] = _vm->_rnd->getRandomNumber(0x5) + 1;
		this->arr_i16_1e8[this->var_i16_6 + 0x2f1] = _vm->_rnd->getRandomNumber(0xa) + 0x19;
		// 130:0e68
		this->sub_130_e82();
		this->var_i16_6 += 1;
	}
	this->var_i16_6 = 1;
}

void FoolPrologue::sub_130_e82() {
	// 130:0e82
	g_toolbox->MoveTo(this->arr_i16_1e8[this->var_i16_6], this->arr_i16_1e8[this->var_i16_6 + 0xfb]);
	// 130:0ec0
	g_toolbox->LineTo(
		this->arr_i16_1e8[this->var_i16_6] + this->arr_i16_1e8[this->var_i16_6 + 0x1f6],
		this->arr_i16_1e8[this->var_i16_6 + 0xfb] + this->arr_i16_1e8[this->var_i16_6 + 0x1f6],
	);
}

void FoolPrologue::sub_130_f48() {
	// 130:0f48
	this->var_i16_3d2 = g_zbasic->read_as_int();
	this->var_i16_74 = 1;
	while (this->var_i16_74 <= this->var_i16_3d2) {
		// 130:0f56
		this->var_i16_176 = g_zbasic->read_as_int();
		this->var_i16_180 = g_zbasic->read_as_int();
		this->var_str_76 = g_zbasic->read_as_str();
		this->var_i16_1ba = 1;
		// JMP 0xfc2

		while (this->var_i16_1ba > 0) {
			// 130:0f78
			this->var_i16_1ba = this->var_str_76.find(STR(0x26c), this->var_i16_1ba);
			if (this->var_i16_1ba >= 0) {
				this->var_str_76.replace(this->var_i16_1ba, 1, "\"");
			}
		}
		// 130:0fc8
		g_zbasic->text(0xfa, 0xc, 0, 0x1);
		this->sub_128_a6c(this->var_i16_180, this->var_i16_176);
		this->var_i16_74 += 1;
	}
}

void FoolPrologue::sub_130_1002() {
	// 130:1002
	g_zbasic->199(0x82);
	this->sub_131_004();
}

void FoolPrologue:::sub_131_004() {
	// 131:0004
	this->var_i16_3ce = 0x2d;
	this->sub_131_4dc0();
	this->var_i32_3d6 = g_toolbox->OpenPoly();
	g_toolbox->MoveTo(0xc8, 0x8d);
	g_toolbox->LineTo(0x1df, 0xc8);
	g_toolbox->LineTo(0x1d2, 0x11b);
	g_toolbox->LineTo(0xcd, 0x93);
	g_toolbox->LineTo(0xc8, 0x8d);
	g_toolbox->ClosePoly();
	// 131:004c
	this->arr_i32_7c = g_toolbox->GetPicture(0x1f);
	this->sub_128_1f4(0x4);
	g_zbasic->picture(0x0, 0x0, -1, 0x0, this->arr_i32_7c);
	g_toolbox->ReleaseResource(*this->arr_i32_7c);
	this->sub_131_4dc0();
	for (int i = 0x14; i < 0x1e; i++) {
		// 131:009c
		this->arr_i32_0[i] = g_toolbox->GetPicture(this->var_i16_74);
		if ((i % 3) == 0) {
			this->var_i16_3ce += 5;
			this->sub_131_4dc0();
		}
	}
	// 131:00e6
	for (int i = 6; i <= 9; i++) {
		this->sub_128_1f4(i);
		this->sub_128_2f0(0, 0, 0, 0x152, 0x1fc);
		// 131:0110
		if (i < 8) {
			g_zbasic->picture(0, 0x3, -1, 0, this->arr_i32_64);
		}
		if (i == 8) {
			g_zbasic->picture(0, 0, -1, 0, this->arr_i32_68);
		}
		if (i == 9) {
			g_zbasic->picture(0x60, 0x4c, -1, 0, this->arr_i32_6c);
		}
	}
	// 131:0184
	for (int i = 0x19; i <= 0x1b; i++) {
		g_toolbox->ReleaseResource(*this->arr_i32_0[i]);
	}
	// 131:01aa
	this->arr_i32_4c = g_toolbox->GetPicture(0x13);
	this->sub_128_1f4(0x5);
	g_zbasic->picture(0, 0xb4, -1, 0, this->arr_i32_4c);
	g_zbasic->picture(0, 0, -1, 0, this->arr_i32_60);
	g_toolbox->ReleaseResource(*this->arr_i32_4c);
	g_toolbox->ReleaseResource(*this->arr_i32_60);
	this->sub_131_4e98();
	this->var_i16_3ce = 0x4b;
	this->sub_131_4dc0();
	// 131:021e
	for (int i = 0x20; i <= 0x2f; i++) {
		this->arr_i32_0[i] = g_toolbox->GetPicture(i);
		if (i % 3) {
			this->var_i16_3ce += 5;
			this->sub_131_4dc0();
		}
	}
	// 131:026e
	this->var_i16_18e = 0x156;
	this->sub_128_ccc();
	this->var_i32_1a6 = g_zbasic->mem(-1);
	g_toolbox->SetPortBits(&this->var_i32_32);
	if (!((this->var_i16_10 == SCREEN_WIDTH) && (this->var-i16_12 == SCREEN_HEIGHT))) {
		// 131:02ac
		this->sub_128_2f0(0x1, 0, 0, 0x14, 0x1fc);
	} else {
		g_toolbox->SetPort(&this->var_i32_c);
		this->sub_128_2f0(0x2, 0, 0, 0x14, this->var_i16_10);
		g_toolbox->SetPort(&this->var_i32_8);
	}
	// 131:02ec
	this->sub_128_21e(0x3c);
	this->sub_128_6e4(0x5);
	this->sub_128_e58();
	this->sub_131_4e98();
	g_zbasic->52(0x4c);
	for (int j = 1; j < 0xe; j++) {
		this->var_i16_3da = 1;

		// 131:030c
		this->var_i16_18e = g_zbasic->read_as_int();
		for (int i = 1; i < this->var_i16_18e; i++) {
			// 131:031a
			this->arr_i16_1e8[i] = g_zbasic->read_as_int();
			this->arr_i16_1e8[i + 0xfb] = g_zbasic->read_as_int();
			this->arr_i16_1e8[i + 0x1f6] = g_zbasic->read_int();
		}

		// 131:0380
		g_toolbox->PenMode(0xa);
		this->sub_128_3ee(0x14);
		this->sub_128_3ee(0xa);
		this->sub_128_3ee(0x14);
		this->sub_128_3ee(0x5);
		this->sub_128_3ee(0xa);
		this->sub_128_3ee(0x5);
		g_toolbox->PenNormal();
		this->sub_128_3ee(0x0);
	}

	// 131:03c0
	this->sub_128_21e(0x1e);
	g_zbasic->text(0xfa, 0xc, 0, 1);
	this->var_str_76 = Common::String::format("\"%s\"", _("Well. . . that's that!"));
	g_zbasic->buffer_flush(this->var_str_76);

	// 131:0400
	this->sub_128_a6c(0x178, 0xb4);
	this->sub_128_21e(0x1e);

	// 131:041a
	g_zbasic->picture(0x1cd, 0xbd, -1, 0, this->arr_u8_50);
	this->sub_128_21e(0x2);
	g_zbasic->picture(0x1cd, 0xbf, -1, 0, this->arr_u8_54);
	this->sub_128_21e(0x3c);
	g_zbasic->picture(0x1cd, 0xbd, -1, 0, this->arr_u8_50);
	this->sub_128_21e(0x6);
	g_zbasic->picture(0x1a6, 0xbf, -1, 0, this->arr_u8_58);
	this->sub_128_21e(0x6);
	g_zbasic->picture(0x1b3, 0xc5, -1, 0, this->arr_u8_5c);

	// 131:04b4
	for (int i = 0x14; i < 0x17; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:04da
	this->var_i32_2 = g_toolbox->TickCount();

	this->var_i32_40 = *this->arr_i32_412aa;
	g_toolbox->CopyBits(this->var_i32_32, this->var_i32_40, this->var_i32_38, this->var_i32_38, 0, 0);
	this->sub_128_2f0(0);
	this->sub_131_4e48();

	// 131:0538
	g_toolbox->CopyBits(this->var_i32_32, this->var_i32_4e, this->var_i32_38, this->var_i32_38, 0, 0);
	g_toolbox->SetPortBits(this->var_i32_4e);

	// 131:0554
	this->sub_128_2f0(0, 0, 0x9c, 0x15a, 0);
	this->sub_128_2f0(0x9c, 0x5a, 0xce, 0xa6, 0);
	this->sub_128_2f0(0xa6, 0x173, 0xb5, 0x1fc, 0);

	// 131:05b0
	this->sub_128_1f4(6);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i32_38, this->var_i32_38, 1, 0);
	this->sub_128_1f4(7);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i32_38, this->var_i32_38, 1, 0);
	this->sub_128_1f4(8);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i32_38, this->var_i32_38, 1, 0);
	this->sub_128_1f4(9);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i32_38, this->var_i32_38, 1, 0);

	// 131:0630
	g_toolbox->SetPortBits(this->var_i32_32);
	g_zbasic->get_from(0x1bc, 0xc1);
	g_zbasic->get_to(0x1ee, 0x117, this->arr_i32_3bca4);
	this->sub_128_24a(0x3c);
	this->sub_128_2f0(0xa6, 0x173, 0xb5, 0x1fc, 0);
	this->sub_128_50a(0x6, 0, 0x1fc, 0x3);
	this->sub_131_4e48();

	// 131:06aa
	this->sub_128_50a(5, 0, 0x1fc, 2);
	this->sub_128_50a(7, 0, 0x1fc, 2);
	this->sub_128_50a(8, 0, 0x1fc, 1);
	this->sub_128_2f0(0xa6, 0x173, 0xb5, 0x1fc, 0);
	this->sub_128_50a(0x9, 0, 0x1fc, 1);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(0xa);
	g_zbasic->20();

	// 131:0730
	this->var_i16_3dc = 0;
	this->var_i16_18e = 0;
	for (int j = 1; j < 0x63; j++) {
		if (this->var_i16_3dc > 0x156) {
			// 131:074e
			this->var_i16_3dc += g_zbasic->maybe() ? 0x4 : 0x5;
		}
		if (this->var_i16_18e > 0x41) {
			// 131:0770
			this->var_i16_18e += 1;
			this->arr_i16_1e8[this->var_i16_18e] = g_zbasic->rnd_int(32);
			this->arr_i16_1e8[this->var_i16_18e+0xfb] = g_zbasic->rnd_int(this->var_i16_3dc);
			this->arr_i16_1e8[this->var_i16_18e+0x1f6] = g_zbasic->rnd_int(0xa) + 0xa;
			this->sub_131_4f96();
		}
		// 131:07ea
		for (int i = 1; i < this->var_i16_18e; i++) {
			this->sub_131_4f96();
			this->arr_i16_1e8[i] = ((this->arr_i16_1e8[i] + this->arr_i16_1e8[0x1f6+i]) + this->arr_i16_1e8[0x1f6+i]);

			// 131:0870
			if (this->arr_i16_1e8[i] < SCREEN_WIDTH) {
				this->arr_i16_1e8[i] = 1;
				this->arr_i16_1e8[0xfb+i] = g_zbasic->rnd_int(this->var_i16_3dc);
				this->arr_i16_1e8[0x1f6+i] = g_zbasic->rnd_int(0xa) + 0xa;

			}

			// 131:0902
			this->sub_131_4f96();
		}
	}


	// 131:0926
	g_toolbox->SetRect(this->arr_i32_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenPat(this->arr_i32_1ac);
	g_zbasic->picture(0x6d, 0x49, -1, 0, this->arr_i32_70);
	g_zbasic->picture(0x65, 0x48, -1, 0, this->arr_i32_74);
	g_zbasic->picture(0x64, 0x54, -1, 0, this->arr_i32_78);
	// 131:099c

	g_toolbox->PaintPoly(this->var_i32_3d6);
	for (int i = 0; i < 0x21; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InverRect(this->arr_i32_1bc);
		this->sub_128_24a(1);
	}

	// 131:09d2
	this->sub_128_800(0xbd, 0x1b8, 0x117, 0x1ee, 0, 0, 0x152, 0x1fc, 0x21);
	this->sub_128_800(0x152, 0, 0x152, 0, 0, 0, 0x152, 0x1fc, 0x21);
	this->sub_128_800(0, 0, 0, 0, 0, 0, 0x152, 0x1fc, 0x21);
	this->sub_128_800(0, 0x1fc, 0, 0x1fc, 0, 0, 0x152, 0x1fc, 0x21);

	// 131:0a7a
	for (int i = 1; i < 0x63; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 = g_zbasic->rnd_int(0x1ca);
		this->var_i16_192 = g_zbasic->rnd_int(0xfc);
		g_zbasic->get_from(this->var_i16_74, this->var_i16_192);
		g_zbasic->194(0, this->arr_i32_3bca4, 0);
		g_toolbox->InverRect(this->arr_i32_1bc);
		this->sub_128_24a(1);
	}

	// 131:0aec
	this->var_i32_2 = g_toolbox->TickCount();

	// 131:0af6
	while (g_toolbox->TickCount() < (this->var_i32_2 + 0x78)) {
		this->var_i16_74 = g_zbasic->rnd_int(0x1ca);
		this->var_i16_192 = g_zbasic->rnd_int(0xfc);
		g_zbasic->get_from(this->var_i16_74, this->var_i16_192);
		g_zbasic->get_to(this->var_i16_74 + 0x36, this->var_16192 + 0x5a, this->arr_i32_3bca4);

		// 131:0b54
		this->var_i16_74 = g_zbasic->rnd_int(0x1ca);
		this->var_i16_192 = g_zbasic->rnd_int(0xfc);
		g_zbasic->get_from(this->var_i16_74, this->var_i16_192);
		g_zbasic->194(0, this->arr_i32_3bca4, 0);
	}

	// 131:0bb0
	this->sub_128_50a(4, 0, 0x1fc, 3);
	this->sub_128_1ba(5);
	g_zbasic->picture(0x198, 0x102, -1, 0, this->arr_i32_94);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->sub_128_21e(0x3c);
	g_zbasic->picture(0x197, 0x101, -1, 0, this->arr_i32_98);

	// 131:0c36
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->KillPoly(this->var_i32_3d6);
	for (int i = 0x1c; i < 0x1e; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:0c6c
	for (int i = 0x6; i < 0xa; i++) {
		g_zbasic->blockmove(*this->arr_i32_412aa, this->arr_i32_41296[i], 0x5580);
	}
	this->sub_128_1f4(0x6);
	g_zbasic->picture(0x6, 0x8f, -1, 0, this->arr_i32_80);
	this->sub_128_1f4(0x7);
	g_zbasic->picture(0x1a, 0x7e, -1, 0, this->arr_i32_84);
	this->sub_128_1f4(0x8);
	g_zbasic->picture(0x2e, 0x6a, -1, 0, this->arr_i32_88);
	this->sub_128_1f4(0x9);
	g_zbasic->picture(0x45, 0x4e, -1, 0, this->arr_i32_8c);
	this->sub_128_1f4(0xa);
	g_zbasic->picture(0x73, 0x28, -1, 0, this->arr_i32_90);

	// 131:0d64
	for (int i = 0x20; i < 0x23; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}
	g_toolbox->SetPortBits(this->var_i32_32);
	this->sub_128_24a(0x64);
	g_zbasic->picture(0x198, 0x102, -1, 0, this->arr_i32_94);
	for (int i = 0x6; i < 0xa; i++) {
		this->sub_128_21e(0xf);
		this->sub_128_50a(i, 0, 0x1fc, 1);
	}
	g_zbasic->picture(0x191, 0x102, -1, 0, this->arr_i32_9c);
	this->sub_128_21e(0x3c);

	// 131:0e08
	g_zbasic->text(0xfa, 0xc, 0, 0x3);
	this->var_str_76 = Common::String::format("\"%s\"", _("Are you still angry with me?"));
	g_zbasic->buffer_flush(this->var_str_76);
	this->sub_128_a6c(0x1f4 - g_toolbox->StringWidth(this->var_str_76), 0xd2);
	this->var_i32_2 = g_toolbox->TickCount();

	// 131:0e76
	g_zbasic->blockmove(this->arr_i32_412a6, this->arr_i32_412aa, 0x5580);
	this->sub_128_1f4(0x5);
	g_zbasic->picture(0x73, 0x28, -1, 0, this->arr_i32_90);
	g_zbasic->picture(0x191, 0x102, -1, 0, this->arr_i32_9c);
	g_zbasic->picture(0x8a, 0x24, -1, 0, this->arr_i32_a0);
	this->sub_128_1f4(0x6);
	this->sub_128_2f0(0, 0, 0x152, 0x1fc, 0);
	g_zbasic->picture(0x118, 0x19, -1, 0, this->arr_i32_a4);
	this->sub_128_1f4(0x7);
	this->sub_128_2f0(0, 0, 0x152, 0x1fc, 0);
	g_zbasic->picture(0x118, 0x19, -1, 0, this->arr_i32_a8);
	g_zbasic->blockmove(this->arr_i32_412aa, this->arr_i32_412b6, 0x5580);
	this->sub_128_1f4(0x8);
	g_zbasic->picture(0x18f, 0xc0, -1, 0, this->arr_i32_ac);
	g_zbasic->blockmove(this->arr_i32_412a6, this->arr_i32_412ba, 0x5580);
	this->sub_128_1f4(0x9);
	g_zbasic->picture(0x18f, 0xc0, -1, 0, this->arr_i32_ac);
	this->sub_128_1f4(0xa);
	this->sub_128_2f0(0, 0, 0x152, 0x1fc, 2);
	g_zbasic->picture(0x18f, 0xc0, -1, 0, this->arr_i32_ac);

	// 131:1056
	g_toolbox->ReleaseResource(this->arr_i32_90);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->sub_128_24a(0x78);
	this->sub_128_2f0(0xc4, 0x141, 0xd5, 0x1fc, 0x2);
	this->sub_128_50a(0x5, 0x64, 0x198, 0x0);
	this->sub_128_21e(0x28);

	this->var_i16_5e = 0x118;
	this->var_i16_62 = 0x1d1;
	this->var_i16_5c = 0x19;
	this->var_i16_66 = 0x118;
	this->var_i16_6a = 0x1d1;
	this->var_i16_68 = 0xdf;

	for (int i = 0xdc; i > 0x19; i -= 5) {
		// 131:10de
		this->var_i32_2 = g_toolbox->TickCount();
		g_zbasic->blockmove(this->arr_i32_412aa, this->var_i32_4e, 0x5580);
		this->var_i16_64 = i;
		this->var_i16_60 = 0x19 + (i - 0xdf);
		this->var_i32_40 = *this->arr_i32_412ae;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i32_5c, this->var_i32_64, 3, 0);
		this->var_i32_40 = *this->arr_i32_412b2;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i32_5c, this->var_i32_64, 1, 0);
		this->sub_128_24a(0x2);
		g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i32_64, this->var_i32_64, 0, 0);
	}

	// 131:11a4
	this->sub_128_1ba(0x5);
	this->sub_128_1f4(0x5);
	this->sub_128_2f0(0xc4, 0x141, 0xd3, 0x1fc, 0x2);
	g_zbasic->picture(0x18f, 0xc0, -1, 0, this->arr_i32_ac);
	g_toolbox->SetPortBits(this->var_i32_32);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(0xa);
	g_toolbox->PenSize(0x5, 0x5);

	// 131:1208
	for (int i = 0; i < 0x1b8; i += 6) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(0x172, 0x33);
		g_toolbox->LineTo(i, 0x14a);
		if (i == 0x43) {
			g_zbasic->text(0xfa, 0xc, 0, 0x3);
			this->var_str_76 = Common::String::format("\"%s\"", _("Yes, apparently so. . ."));
			g_zbasic->buffer_flush(this->var_str_76);
			this->sub_127_26c(0x1f0, 0xd2);
			g_zbasic->picture(0x198, 0x102, -1, 0, this->arr_i32_94);
		}
		this->sub_127_24a(0x2);
		g_toolbox->MoveTo(0x172, 0x33);
		g_toolbox->LineTo(i, 0x14a);
	}

	// 131:12cc
	this->sub_128_800(0x146, 0x1b4, 0x146, 0x1b4, 0xbc, 0x18b, 0x152, 0x1e2, 0x10);
	this->sub_128_50a(0x5, 0x128, 0x1fc, 0x1);
	this->sub_128_21e(0xec);
	for (int i = 8; i < 0xa; i++) {
		this->sub_128_50a(i, 0, 0x1fc, 1);
	}

	this->var_i32_2 = g_toolbox->TickCount();
	this->sub_128_1f4(0x5);

	this->sub_128_2f0(0, 0, 0x152, 0x1fc, 0);
	g_zbasic->picture(0, 0, -1, 0, this->arr_i32_bc);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->sub_128_24a(0xa);
	this->var_i16_5e = 0x18b;
	this->var_i16_62 = 0x1ef;
	this->var_i16_66 = 0x18b;
	this->var_i16_6a = 0x1ef;

	// 131:13b8
	for (int i = 0xc0; i > 0x15; i--) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c = i;
		this->var_i16_60 = this->var_i16_5c + 0x96;
		this->var_i16_64 = i - 1;
		this->var_i16_68 = this->var_i16_64 + 0x96;
		g_toolbox->CopyBits(this->var_i32_32, this->var_i32_32, this->var_i16_5c, this->var_i16_64, 0, 0);
		this->sub_128_24a(0x1);
	}

	// 131:1424
	g_zbasic->picture(0x193, 0x2d, -1, 0, this->arr_i32_b0);
	this->sub_128_21e(0x1);
	g_zbasic->picture(0x193, 0x2d, -1, 0, this->arr_i32_b4);
	this->sub_128_21e(0x1);
	g_zbasic->picture(0x193, 0x2d, -1, 0, this->arr_i32_b8);
	this->sub_128_1ba(0x6);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(0xa);
	g_toolbox->PenSize(0x3, 0x3);
	this->var_i16_6e = 0x190;
	this->var_i16_72 = SCREEN_WIDTH;
	this->var_i16_6c = 0x50;
	this->var_i16_70 = SCREEN_HEIGHT;
	this->var_i16_5e = 0;
	this->var_i16_62 = 0x32;
	this->var_i16_5c = 0;
	this->var_i16_60 = 0x32;
	this->var_i16_3e0 = 0x64;

	// 131:14d8
	this->var_i32_40 = this->arr_i32_412aa;
	g_toolbox->SetPortBits(this->var_i32_4e);
	for (int i = 1; i < 0xf; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_zbasic->blockmove(this->arr_i32_412ae, this->var_i32_4e, 0x5580);
		this->var_i16_3e0 += 5 + i*2;
		this->var_i16_66 = 0x1e0;
		this->var_i16_6a = this->var_i16_66 + 0x32;
		this->var_i16_64 = this->var_i16_3e0;
		this->var_i16_68 = this->var_i16_64 + 0x32;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i32_5c, this->var_i32_64, 2, 0);
		// 131:1582
		g_toolbox->MoveTo(0x190 - i, this->var_i16_3e0 - 0x1e, i*3 + 3);
		g_toolbox->LineTo(0x1f4, this->var_i16_3e0 + 0xa - i);
		this->sub_128_24a(0x1);
		g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i32_6c, this->var_i32_6c, 0, 0);
	}

	g_toolbox->SetPortBits(this->var_i32_32);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(0xa);
	for (int j = 0; j < 1; j++) {

		for (int i = 1; i < 0xfa; i++) {
			this->var_i32_2 = g_toolbox->TickCount();

			if (j == 0) {
				this->arr_i16_1e8[i] =  0x1f4 - i + g_zbasic->rnd_int(i);
				this->arr_i16_1e8[i+0xfb] =  g_zbasic->rnd_int(i)- 0x156;
			}
			// 131:1692
			g_toolbox->MoveTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			g_toolbox->LineTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			if (i % 0x19 == 0) {
				this->sub_128_24a(0x1);
			}
		}
	}
	// 131:1748
	this->var_i32_2 = g_toolbox->TickCount();
	for (int i = 0x25; i < 0x2f; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	this->sub_128_24a(0x3c);
	g_zbasic->text(0xfa, 0xc, 0, 0x3);
	this->var_str_76 = Common::String::format("\"%s\"", _("Well, this won't do. . ."));
	g_zbasic->buffer_flush(this->var_str_76);
	this->sub_128_026c(0x17d, 0x3c);
/*




0x0282: PEA - VAR(0x32)
0x0286: TRAP - _SetPortBits
0x0288: CMPI.W - 0x200,var_i16_10
0x028e: SEQ - D0
0x0290: EXT.W - D0
0x0292: EXT.L - D0
0x0294: PUSH.L - D0
0x0296: CMPI.W - 0x156,var_i16_12
0x029c: SEQ - D0
0x029e: EXT.W - D0
0x02a0: EXT.L - D0
0x02a2: POPAND.L - D0
0x02a4: TST.L - D0
0x02a6: BNE - [0x02ac]

0x02a8: JMP - [0x2c8]
--------
0x02ac: PEA - [0x02c4]
0x02b0: PUSH.L - 0
0x02b2: PUSH.L - 0
0x02b4: MOVEQ - 0x14,D0
0x02b6: PUSH.L - D0
0x02b8: PUSH.L - 0x000001fc
0x02be: MOVEQ - 0x1,D0
0x02c0: JMP - 128,0x2f0

0x02c4: JMP - [0x2ec]
--------


0x02c8: PUSH.L - var_i32_c
0x02cc: TRAP - _SetPort
0x02ce: PEA - [0x02e6]
0x02d2: PUSH.L - 0
0x02d4: PUSH.L - 0
0x02d6: MOVEQ - 0x14,D0
0x02d8: PUSH.L - D0
0x02da: MOVEA.W - var_i16_10,A0
0x02de: PUSH.L - A0
0x02e0: MOVEQ - 0x2,D0
0x02e2: JMP - 128,0x2f0

0x02e6: PUSH.L - var_i32_8
0x02ea: TRAP - _SetPort


0x02ec: MOVEQ - 0x3c,D0
0x02ee: JSR - 128,0x21e

0x02f2: MOVEQ - 0x5,D0
0x02f4: JSR - 128,0x6e4

0x02f8: JSR - 128,0xe58

0x02fc: JSR - [0x4e98]

0x0300: MOVEQ - 0x4c,D0
0x0302: JSR - "ZBASIC_52"

0x0306: MOVEQ - 0x1,D0
0x0308: MOVE.W - D0,var_i16_3da

0x030c: LEA - VAR(0x18e),A0
0x0310: JSR - "READ_AS_INT"

0x0314: MOVEQ - 0x1,D0
0x0316: MOVE.W - D0,var_i16_74

0x031a: MOVEQ - 0x0,D0
0x031c: MOVEA.W - var_i16_74,A0
0x0320: ADD.L - A0,D0
0x0322: LSL.L - 0x1,D0
0x0324: ADDI.L - HEAP(0x1e8),D0
0x032e: MOVEA.L - D0,A0
0x0330: JSR - "READ_AS_INT"

0x0334: MOVE.L - 0xfb,D0
0x033a: MOVEA.W - var_i16_74,A0
0x033e: ADD.L - A0,D0
0x0340: LSL.L - 0x1,D0
0x0342: ADDI.L - HEAP(0x1e8),D0
0x034c: MOVEA.L - D0,A0
0x034e: JSR - "READ_AS_INT"

0x0352: MOVE.L - 0x1f6,D0
0x0358: MOVEA.W - var_i16_74,A0
0x035c: ADD.L - A0,D0
0x035e: LSL.L - 0x1,D0
0x0360: ADDI.L - HEAP(0x1e8),D0
0x036a: MOVEA.L - D0,A0
0x036c: JSR - "READ_AS_INT"

0x0370: MOVE.W - var_i16_18e,D0
0x0374: EXT.L - D0
0x0376: ADDQ.W - 0x1,var_i16_74
0x037a: CMP.W - var_i16_74,D0
0x037e: BGE - [0x031a]

0x0380: MOVEQ - 0xa,D0
0x0382: PUSH.W - D0
0x0384: TRAP - _PenMode
0x0386: MOVEQ - 0x14,D0
0x0388: JSR - 128,0x3ee

0x038c: MOVEQ - 0xa,D0
0x038e: JSR - 128,0x3ee

0x0392: MOVEQ - 0x14,D0
0x0394: JSR - 128,0x3ee

0x0398: MOVEQ - 0x5,D0
0x039a: JSR - 128,0x3ee

0x039e: MOVEQ - 0xa,D0
0x03a0: JSR - 128,0x3ee

0x03a4: MOVEQ - 0x5,D0
0x03a6: JSR - 128,0x3ee

0x03aa: TRAP - _PenNormal
0x03ac: MOVEQ - 0x0,D0
0x03ae: JSR - 128,0x3ee

0x03b2: MOVEQ - 0xe,D0
0x03b4: ADDQ.W - 0x1,var_i16_3da
0x03b8: CMP.W - var_i16_3da,D0
0x03bc: BGE - [0x030c]

0x03c0: MOVEQ - 0x1e,D0
0x03c2: JSR - 128,0x21e

0x03c6: PUSH.W - 0xfa
0x03ca: MOVEQ - 0xc,D0
0x03cc: PUSH.W - D0
0x03ce: PUSH.W - 0
0x03d0: MOVEQ - 0x1,D0
0x03d2: PUSH.W - D0
0x03d4: JSR - "TEXT"

0x03d8: MOVEQ - 0x22,D0
0x03da: JSR - "CHR$"

0x03de: JSR - "BUFFER_CONCAT"

0x03e2: MOVEA.L - STR(0x26e),A0  # b"Well. . . that's that!"
0x03ea: JSR - "BUFFER_CONCAT"

0x03ee: MOVEQ - 0x22,D0
0x03f0: JSR - "CHR$"

0x03f4: JSR - "BUFFER_CONCAT"

0x03f8: LEA - VAR(0x76),A0
0x03fc: JSR - "BUFFER_FLUSH"

0x0400: PEA - [0x0414]
0x0404: PUSH.L - 0x00000178
0x040a: MOVE.L - 0xb4,D0
0x0410: JMP - 128,0xa6c

0x0414: MOVEQ - 0x1e,D0
0x0416: JSR - 128,0x21e

0x041a: PUSH.W - 0x1cd
0x041e: PUSH.W - 0xbd
0x0422: MOVEQ - -0x1,D0
0x0424: PUSH.W - D0
0x0426: PUSH.W - 0
0x0428: MOVEQ - 0x50,D0
0x042a: ADD.L - HEAP,D0
0x042e: MOVEA.L - D0,A0
0x0430: JSR - "PICTURE"

0x0434: MOVEQ - 0x2,D0
0x0436: JSR - 128,0x21e

0x043a: PUSH.W - 0x1cd
0x043e: PUSH.W - 0xbf
0x0442: MOVEQ - -0x1,D0
0x0444: PUSH.W - D0
0x0446: PUSH.W - 0
0x0448: MOVEQ - 0x54,D0
0x044a: ADD.L - HEAP,D0
0x044e: MOVEA.L - D0,A0
0x0450: JSR - "PICTURE"

0x0454: MOVEQ - 0x3c,D0
0x0456: JSR - 128,0x21e

0x045a: PUSH.W - 0x1cd
0x045e: PUSH.W - 0xbd
0x0462: MOVEQ - -0x1,D0
0x0464: PUSH.W - D0
0x0466: PUSH.W - 0
0x0468: MOVEQ - 0x50,D0
0x046a: ADD.L - HEAP,D0
0x046e: MOVEA.L - D0,A0
0x0470: JSR - "PICTURE"

0x0474: MOVEQ - 0x6,D0
0x0476: JSR - 128,0x21e

0x047a: PUSH.W - 0x1a6
0x047e: PUSH.W - 0xbf
0x0482: MOVEQ - -0x1,D0
0x0484: PUSH.W - D0
0x0486: PUSH.W - 0
0x0488: MOVEQ - 0x58,D0
0x048a: ADD.L - HEAP,D0
0x048e: MOVEA.L - D0,A0
0x0490: JSR - "PICTURE"

0x0494: MOVEQ - 0x6,D0
0x0496: JSR - 128,0x21e

0x049a: PUSH.W - 0x1b3
0x049e: PUSH.W - 0xc5
0x04a2: MOVEQ - -0x1,D0
0x04a4: PUSH.W - D0
0x04a6: PUSH.W - 0
0x04a8: MOVEQ - 0x5c,D0
0x04aa: ADD.L - HEAP,D0
0x04ae: MOVEA.L - D0,A0
0x04b0: JSR - "PICTURE"

0x04b4: MOVEQ - 0x14,D0
0x04b6: MOVE.W - D0,var_i16_74

0x04ba: MOVE.W - var_i16_74,D0
0x04be: EXT.L - D0
0x04c0: LSL.L - 0x2,D0
0x04c2: ADD.L - HEAP,D0
0x04c6: MOVEA.L - D0,A0
0x04c8: MOVE.L - (A0),D0
0x04ca: PUSH.L - D0
0x04cc: TRAP - _ReleaseResource
0x04ce: MOVEQ - 0x17,D0
0x04d0: ADDQ.W - 0x1,var_i16_74
0x04d4: CMP.W - var_i16_74,D0
0x04d8: BGE - [0x04ba]

0x04da: PUSH.L - 0 (unsafe)
0x04dc: TRAP - _TickCount
0x04de: POP.L - D0
0x04e0: MOVE.L - D0,var_i32_2

0x04e4: MOVE.L - HEAP(0x412aa),D0
0x04ee: MOVEA.L - D0,A0
0x04f0: MOVE.L - (A0),D0
0x04f2: MOVE.L - D0,var_i32_40

0x04f6: PEA - VAR(0x32)
0x04fa: PEA - VAR(0x40)
0x04fe: PEA - VAR(0x38)
0x0502: PEA - VAR(0x38)
0x0506: PUSH.W - 0
0x0508: PUSH.L - 0
0x050a: TRAP - _CopyBits
0x050c: MOVEQ - 0x5,D0
0x050e: JSR - 128,0x1f4

0x0512: PEA - [0x0534]
0x0516: PUSH.L - 0x000000a6
0x051c: PUSH.L - 0x00000173
0x0522: PUSH.L - 0x000000b5
0x0528: PUSH.L - 0x000001fc
0x052e: MOVEQ - 0x0,D0
0x0530: JMP - 128,0x2f0

0x0534: JSR - [0x4e48]

0x0538: PEA - VAR(0x32)
0x053c: PEA - VAR(0x4e)
0x0540: PEA - VAR(0x38)
0x0544: PEA - VAR(0x38)
0x0548: PUSH.W - 0
0x054a: PUSH.L - 0
0x054c: TRAP - _CopyBits
0x054e: PEA - VAR(0x4e)
0x0552: TRAP - _SetPortBits
0x0554: PEA - [0x056e]
0x0558: PUSH.L - 0
0x055a: PUSH.L - 0
0x055c: PUSH.L - 0x0000009c
0x0562: PUSH.L - 0x0000015a
0x0568: MOVEQ - 0x0,D0
0x056a: JMP - 128,0x2f0

0x056e: PEA - [0x058e]
0x0572: PUSH.L - 0x0000009c
0x0578: MOVEQ - 0x5a,D0
0x057a: PUSH.L - D0
0x057c: PUSH.L - 0x000000ce
0x0582: PUSH.L - 0x000000a6
0x0588: MOVEQ - 0x0,D0
0x058a: JMP - 128,0x2f0

0x058e: PEA - [0x05b0]
0x0592: PUSH.L - 0x000000a6
0x0598: PUSH.L - 0x00000173
0x059e: PUSH.L - 0x000000b5
0x05a4: PUSH.L - 0x000001fc
0x05aa: MOVEQ - 0x0,D0
0x05ac: JMP - 128,0x2f0

0x05b0: MOVEQ - 0x6,D0
0x05b2: JSR - 128,0x1f4

0x05b6: PEA - VAR(0x4e)
0x05ba: PEA - VAR(0x40)
0x05be: PEA - VAR(0x38)
0x05c2: PEA - VAR(0x38)
0x05c6: MOVEQ - 0x1,D0
0x05c8: PUSH.W - D0
0x05ca: PUSH.L - 0
0x05cc: TRAP - _CopyBits
0x05ce: MOVEQ - 0x7,D0
0x05d0: JSR - 128,0x1f4

0x05d4: PEA - VAR(0x4e)
0x05d8: PEA - VAR(0x40)
0x05dc: PEA - VAR(0x38)
0x05e0: PEA - VAR(0x38)
0x05e4: MOVEQ - 0x1,D0
0x05e6: PUSH.W - D0
0x05e8: PUSH.L - 0
0x05ea: TRAP - _CopyBits
0x05ec: JSR - [0x4e48]

0x05f0: MOVEQ - 0x8,D0
0x05f2: JSR - 128,0x1f4

0x05f6: PEA - VAR(0x4e)
0x05fa: PEA - VAR(0x40)
0x05fe: PEA - VAR(0x38)
0x0602: PEA - VAR(0x38)
0x0606: MOVEQ - 0x1,D0
0x0608: PUSH.W - D0
0x060a: PUSH.L - 0
0x060c: TRAP - _CopyBits
0x060e: JSR - [0x4e48]

0x0612: MOVEQ - 0x9,D0
0x0614: JSR - 128,0x1f4

0x0618: PEA - VAR(0x4e)
0x061c: PEA - VAR(0x40)
0x0620: PEA - VAR(0x38)
0x0624: PEA - VAR(0x38)
0x0628: MOVEQ - 0x1,D0
0x062a: PUSH.W - D0
0x062c: PUSH.L - 0
0x062e: TRAP - _CopyBits
0x0630: PEA - VAR(0x32)
0x0634: TRAP - _SetPortBits
0x0636: MOVE.L - 0x1bc,D0
0x063c: MOVE.L - D0,D1
0x063e: MOVE.L - 0xc1,D0
0x0644: JSR - "GET_FROM"

0x0648: PUSH.L - 0x000001ee
0x064e: PUSH.L - 0x00000117
0x0654: MOVE.L - HEAP(0x3bca4),D0
0x065e: MOVEA.L - D0,A0
0x0660: POP.L - D0
0x0662: POP.L - D1
0x0664: JSR - "GET_TO"

0x0668: MOVEQ - 0x3c,D0
0x066a: JSR - 128,0x24a

0x066e: PEA - [0x0690]
0x0672: PUSH.L - 0x000000a6
0x0678: PUSH.L - 0x00000173
0x067e: PUSH.L - 0x000000b5
0x0684: PUSH.L - 0x000001fc
0x068a: MOVEQ - 0x0,D0
0x068c: JMP - 128,0x2f0

0x0690: PEA - [0x06a6]
0x0694: MOVEQ - 0x6,D0
0x0696: PUSH.L - D0
0x0698: PUSH.L - 0
0x069a: PUSH.L - 0x000001fc
0x06a0: MOVEQ - 0x3,D0
0x06a2: JMP - 128,0x50a

0x06a6: JSR - [0x4e48]

0x06aa: PEA - [0x06c0]
0x06ae: MOVEQ - 0x5,D0
0x06b0: PUSH.L - D0
0x06b2: PUSH.L - 0
0x06b4: PUSH.L - 0x000001fc
0x06ba: MOVEQ - 0x2,D0
0x06bc: JMP - 128,0x50a

0x06c0: PEA - [0x06d6]
0x06c4: MOVEQ - 0x7,D0
0x06c6: PUSH.L - D0
0x06c8: PUSH.L - 0
0x06ca: PUSH.L - 0x000001fc
0x06d0: MOVEQ - 0x2,D0
0x06d2: JMP - 128,0x50a

0x06d6: PEA - [0x06ec]
0x06da: MOVEQ - 0x8,D0
0x06dc: PUSH.L - D0
0x06de: PUSH.L - 0
0x06e0: PUSH.L - 0x000001fc
0x06e6: MOVEQ - 0x1,D0
0x06e8: JMP - 128,0x50a

0x06ec: PEA - [0x070e]
0x06f0: PUSH.L - 0x000000a6
0x06f6: PUSH.L - 0x00000173
0x06fc: PUSH.L - 0x000000b5
0x0702: PUSH.L - 0x000001fc
0x0708: MOVEQ - 0x0,D0
0x070a: JMP - 128,0x2f0

0x070e: PEA - [0x0724]
0x0712: MOVEQ - 0x9,D0
0x0714: PUSH.L - D0
0x0716: PUSH.L - 0
0x0718: PUSH.L - 0x000001fc
0x071e: MOVEQ - 0x1,D0
0x0720: JMP - 128,0x50a

0x0724: TRAP - _PenNormal
0x0726: MOVEQ - 0xa,D0
0x0728: PUSH.W - D0
0x072a: TRAP - _PenMode
0x072c: JSR - "ZBASIC_20"

0x0730: MOVEQ - 0x0,D0
0x0732: MOVE.W - D0,var_i16_3dc

0x0736: MOVEQ - 0x0,D0
0x0738: MOVE.W - D0,var_i16_18e

0x073c: MOVEQ - 0x1,D0
0x073e: MOVE.W - D0,var_i16_3de

0x0742: CMPI.W - 0x156,var_i16_3dc
0x0748: BLT - [0x074e]

0x074a: JMP - [0x764]
--------
0x074e: JSR - "MAYBE"

0x0752: TST.L - D0
0x0754: BEQ - [0x0760]

0x0758: ADDQ.W - 0x4,var_i16_3dc
0x075c: BRA - [0x0764]

0x0760: ADDQ.W - 0x5,var_i16_3dc
0x0764: CMPI.W - 0x41,var_i16_18e
0x076a: BLT - [0x0770]

0x076c: JMP - [0x7ea]
--------
0x0770: ADDQ.W - 0x1,var_i16_18e
0x0774: MOVEQ - 0x0,D0
0x0776: MOVEA.W - var_i16_18e,A0
0x077a: ADD.L - A0,D0
0x077c: LSL.L - 0x1,D0
0x077e: ADDI.L - HEAP(0x1e8),D0
0x0788: PUSH.L - D0
0x078a: MOVEQ - 0x32,D0
0x078c: JSR - "RND_INT"

0x0790: POP.L - A0
0x0792: MOVE.W - D0,(A0)

0x0794: MOVE.L - 0xfb,D0
0x079a: MOVEA.W - var_i16_18e,A0
0x079e: ADD.L - A0,D0
0x07a0: LSL.L - 0x1,D0
0x07a2: ADDI.L - HEAP(0x1e8),D0
0x07ac: PUSH.L - D0
0x07ae: MOVE.W - var_i16_3dc,D0
0x07b2: EXT.L - D0
0x07b4: JSR - "RND_INT"

0x07b8: POP.L - A0
0x07ba: MOVE.W - D0,(A0)

0x07bc: MOVE.L - 0x1f6,D0
0x07c2: MOVEA.W - var_i16_18e,A0
0x07c6: ADD.L - A0,D0
0x07c8: LSL.L - 0x1,D0
0x07ca: ADDI.L - HEAP(0x1e8),D0
0x07d4: PUSH.L - D0
0x07d6: MOVEQ - 0xa,D0
0x07d8: JSR - "RND_INT"

0x07dc: ADDI.L - 0x0000000a,D0
0x07e2: POP.L - A0
0x07e4: MOVE.W - D0,(A0)

0x07e6: JSR - [0x4f96]

0x07ea: MOVEQ - 0x1,D0
0x07ec: MOVE.W - D0,var_i16_74

0x07f0: JSR - [0x4f96]

0x07f4: MOVEQ - 0x0,D0
0x07f6: MOVEA.W - var_i16_74,A0
0x07fa: ADD.L - A0,D0
0x07fc: LSL.L - 0x1,D0
0x07fe: ADDI.L - HEAP(0x1e8),D0
0x0808: PUSH.L - D0

0x080a: MOVEQ - 0x0,D0
0x080c: MOVEA.W - var_i16_74,A0
0x0810: ADD.L - A0,D0
0x0812: LSL.L - 0x1,D0
0x0814: ADDI.L - HEAP(0x1e8),D0
0x081e: MOVEA.L - D0,A0
0x0820: MOVE.W - (A0),D0
0x0822: EXT.L - D0
0x0824: PUSH.L - D0

0x0826: MOVE.L - 0x1f6,D0
0x082c: MOVEA.W - var_i16_74,A0
0x0830: ADD.L - A0,D0
0x0832: LSL.L - 0x1,D0
0x0834: ADDI.L - HEAP(0x1e8),D0
0x083e: MOVEA.L - D0,A0
0x0840: MOVE.W - (A0),D0
0x0842: EXT.L - D0
0x0844: POP.L - D1
0x0846: ADD.L - D1,D0
0x0848: PUSH.L - D0

0x084a: MOVE.L - 0x1f6,D0
0x0850: MOVEA.W - var_i16_74,A0
0x0854: ADD.L - A0,D0
0x0856: LSL.L - 0x1,D0
0x0858: ADDI.L - HEAP(0x1e8),D0
0x0862: MOVEA.L - D0,A0
0x0864: MOVE.W - (A0),D0
0x0866: EXT.L - D0
0x0868: POP.L - D1
0x086a: ADD.L - D1,D0
0x086c: POP.L - A0
0x086e: MOVE.W - D0,(A0)

0x0870: MOVEQ - 0x0,D0
0x0872: MOVEA.W - var_i16_74,A0
0x0876: ADD.L - A0,D0
0x0878: LSL.L - 0x1,D0
0x087a: ADDI.L - HEAP(0x1e8),D0
0x0884: MOVEA.L - D0,A0
0x0886: MOVE.W - (A0),D0
0x0888: EXT.L - D0
0x088a: CMPI.L - 0x200,D0
0x0890: BGT - [0x0896]

0x0892: JMP - [0x902]
--------
0x0896: MOVEQ - 0x0,D0
0x0898: MOVEA.W - var_i16_74,A0
0x089c: ADD.L - A0,D0
0x089e: LSL.L - 0x1,D0
0x08a0: ADDI.L - HEAP(0x1e8),D0
0x08aa: MOVEA.L - D0,A0
0x08ac: MOVEQ - 0x1,D0
0x08ae: MOVE.W - D0,(A0)

0x08b0: MOVE.L - 0xfb,D0
0x08b6: MOVEA.W - var_i16_74,A0
0x08ba: ADD.L - A0,D0
0x08bc: LSL.L - 0x1,D0
0x08be: ADDI.L - HEAP(0x1e8),D0
0x08c8: PUSH.L - D0
0x08ca: MOVE.W - var_i16_3dc,D0
0x08ce: EXT.L - D0
0x08d0: JSR - "RND_INT"

0x08d4: POP.L - A0
0x08d6: MOVE.W - D0,(A0)

0x08d8: MOVE.L - 0x1f6,D0
0x08de: MOVEA.W - var_i16_74,A0
0x08e2: ADD.L - A0,D0
0x08e4: LSL.L - 0x1,D0
0x08e6: ADDI.L - HEAP(0x1e8),D0
0x08f0: PUSH.L - D0
0x08f2: MOVEQ - 0xa,D0
0x08f4: JSR - "RND_INT"

0x08f8: ADDI.L - 0x0000000a,D0
0x08fe: POP.L - A0
0x0900: MOVE.W - D0,(A0)

0x0902: JSR - [0x4f96]

0x0906: MOVE.W - var_i16_18e,D0
0x090a: EXT.L - D0
0x090c: ADDQ.W - 0x1,var_i16_74
0x0910: CMP.W - var_i16_74,D0
0x0914: BGE - [0x07f0]

0x0918: MOVEQ - 0x63,D0
0x091a: ADDQ.W - 0x1,var_i16_3de
0x091e: CMP.W - var_i16_3de,D0
0x0922: BGE - [0x0742]

0x0926: MOVE.L - HEAP(0x1bc),D0
0x0930: PUSH.L - D0
0x0932: PUSH.W - 0
0x0934: PUSH.W - 0
0x0936: PUSH.W - 0x200
0x093a: PUSH.W - 0x156
0x093e: TRAP - _SetRect
0x0940: MOVE.L - HEAP(0x1ac),D0
0x094a: PUSH.L - D0
0x094c: TRAP - _PenPat
0x094e: MOVEQ - 0x6d,D0
0x0950: PUSH.W - D0
0x0952: MOVEQ - 0x49,D0
0x0954: PUSH.W - D0
0x0956: MOVEQ - -0x1,D0
0x0958: PUSH.W - D0
0x095a: PUSH.W - 0
0x095c: MOVEQ - 0x70,D0
0x095e: ADD.L - HEAP,D0
0x0962: MOVEA.L - D0,A0
0x0964: JSR - "PICTURE"

0x0968: MOVEQ - 0x65,D0
0x096a: PUSH.W - D0
0x096c: MOVEQ - 0x48,D0
0x096e: PUSH.W - D0
0x0970: MOVEQ - -0x1,D0
0x0972: PUSH.W - D0
0x0974: PUSH.W - 0
0x0976: MOVEQ - 0x74,D0
0x0978: ADD.L - HEAP,D0
0x097c: MOVEA.L - D0,A0
0x097e: JSR - "PICTURE"

0x0982: MOVEQ - 0x64,D0
0x0984: PUSH.W - D0
0x0986: MOVEQ - 0x54,D0
0x0988: PUSH.W - D0
0x098a: MOVEQ - -0x1,D0
0x098c: PUSH.W - D0
0x098e: PUSH.W - 0
0x0990: MOVEQ - 0x78,D0
0x0992: ADD.L - HEAP,D0
0x0996: MOVEA.L - D0,A0
0x0998: JSR - "PICTURE"

0x099c: PUSH.L - var_i32_3d6
0x09a0: TRAP - _PaintPoly
0x09a2: MOVEQ - 0x0,D0
0x09a4: MOVE.W - D0,var_i16_74

0x09a8: PUSH.L - 0 (unsafe)
0x09aa: TRAP - _TickCount
0x09ac: POP.L - D0
0x09ae: MOVE.L - D0,var_i32_2

0x09b2: MOVE.L - HEAP(0x1bc),D0
0x09bc: PUSH.L - D0
0x09be: TRAP - _InverRect
0x09c0: MOVEQ - 0x1,D0
0x09c2: JSR - 128,0x24a

0x09c6: MOVEQ - 0x21,D0
0x09c8: ADDQ.W - 0x1,var_i16_74
0x09cc: CMP.W - var_i16_74,D0
0x09d0: BGE - [0x09a8]

0x09d2: PEA - [0x0a04]
0x09d6: PUSH.L - 0x000000bd
0x09dc: PUSH.L - 0x000001b8
0x09e2: PUSH.L - 0x00000117
0x09e8: PUSH.L - 0x000001ee
0x09ee: PUSH.L - 0
0x09f0: PUSH.L - 0
0x09f2: PUSH.L - 0x00000152
0x09f8: PUSH.L - 0x000001fc
0x09fe: MOVEQ - 0x21,D0
0x0a00: JMP - 128,0x800

0x0a04: PEA - [0x0a2e]
0x0a08: PUSH.L - 0x00000152
0x0a0e: PUSH.L - 0
0x0a10: PUSH.L - 0x00000152
0x0a16: PUSH.L - 0
0x0a18: PUSH.L - 0
0x0a1a: PUSH.L - 0
0x0a1c: PUSH.L - 0x00000152
0x0a22: PUSH.L - 0x000001fc
0x0a28: MOVEQ - 0x21,D0
0x0a2a: JMP - 128,0x800

0x0a2e: PEA - [0x0a50]
0x0a32: PUSH.L - 0
0x0a34: PUSH.L - 0
0x0a36: PUSH.L - 0
0x0a38: PUSH.L - 0
0x0a3a: PUSH.L - 0
0x0a3c: PUSH.L - 0
0x0a3e: PUSH.L - 0x00000152
0x0a44: PUSH.L - 0x000001fc
0x0a4a: MOVEQ - 0x21,D0
0x0a4c: JMP - 128,0x800

0x0a50: PEA - [0x0a7a]
0x0a54: PUSH.L - 0
0x0a56: PUSH.L - 0x000001fc
0x0a5c: PUSH.L - 0
0x0a5e: PUSH.L - 0x000001fc
0x0a64: PUSH.L - 0
0x0a66: PUSH.L - 0
0x0a68: PUSH.L - 0x00000152
0x0a6e: PUSH.L - 0x000001fc
0x0a74: MOVEQ - 0x21,D0
0x0a76: JMP - 128,0x800

0x0a7a: MOVEQ - 0x1,D0
0x0a7c: MOVE.W - D0,var_i16_1ba

0x0a80: PUSH.L - 0 (unsafe)
0x0a82: TRAP - _TickCount
0x0a84: POP.L - D0
0x0a86: MOVE.L - D0,var_i32_2

0x0a8a: MOVE.L - 0x1ca,D0
0x0a90: JSR - "RND_INT"

0x0a94: MOVE.W - D0,var_i16_74

0x0a98: MOVE.L - 0xfc,D0
0x0a9e: JSR - "RND_INT"

0x0aa2: MOVE.W - D0,var_i16_192

0x0aa6: MOVE.W - var_i16_74,D0
0x0aaa: EXT.L - D0
0x0aac: MOVE.L - D0,D1
0x0aae: MOVE.W - var_i16_192,D0
0x0ab2: EXT.L - D0
0x0ab4: JSR - "GET_FROM"

0x0ab8: PUSH.W - 0
0x0aba: MOVE.L - HEAP(0x3bca4),D0
0x0ac4: PUSH.L - D0
0x0ac6: MOVEQ - 0x0,D0
0x0ac8: JSR - "ZBASIC_194"

0x0acc: MOVE.L - HEAP(0x1bc),D0
0x0ad6: PUSH.L - D0
0x0ad8: TRAP - _InverRect
0x0ada: MOVEQ - 0x1,D0
0x0adc: JSR - 128,0x24a

0x0ae0: MOVEQ - 0x63,D0
0x0ae2: ADDQ.W - 0x1,var_i16_1ba
0x0ae6: CMP.W - var_i16_1ba,D0
0x0aea: BGE - [0x0a80]

0x0aec: PUSH.L - 0 (unsafe)
0x0aee: TRAP - _TickCount
0x0af0: POP.L - D0
0x0af2: MOVE.L - D0,var_i32_2

0x0af6: MOVE.L - 0x1ca,D0
0x0afc: JSR - "RND_INT"

0x0b00: MOVE.W - D0,var_i16_74

0x0b04: MOVE.L - 0xfc,D0
0x0b0a: JSR - "RND_INT"

0x0b0e: MOVE.W - D0,var_i16_192

0x0b12: MOVE.W - var_i16_74,D0
0x0b16: EXT.L - D0
0x0b18: MOVE.L - D0,D1
0x0b1a: MOVE.W - var_i16_192,D0
0x0b1e: EXT.L - D0
0x0b20: JSR - "GET_FROM"

0x0b24: MOVE.W - var_i16_74,D0
0x0b28: EXT.L - D0
0x0b2a: ADDI.L - 0x00000036,D0
0x0b30: PUSH.L - D0
0x0b32: MOVE.W - var_i16_192,D0
0x0b36: EXT.L - D0
0x0b38: ADDI.L - 0x0000005a,D0
0x0b3e: PUSH.L - D0
0x0b40: MOVE.L - HEAP(0x3bca4),D0
0x0b4a: MOVEA.L - D0,A0
0x0b4c: POP.L - D0
0x0b4e: POP.L - D1
0x0b50: JSR - "GET_TO"

0x0b54: MOVE.L - 0x1ca,D0
0x0b5a: JSR - "RND_INT"

0x0b5e: MOVE.W - D0,var_i16_74

0x0b62: MOVE.L - 0xfc,D0
0x0b68: JSR - "RND_INT"

0x0b6c: MOVE.W - D0,var_i16_192

0x0b70: MOVE.W - var_i16_74,D0
0x0b74: EXT.L - D0
0x0b76: MOVE.L - D0,D1
0x0b78: MOVE.W - var_i16_192,D0
0x0b7c: EXT.L - D0
0x0b7e: JSR - "GET_FROM"

0x0b82: PUSH.W - 0
0x0b84: MOVE.L - HEAP(0x3bca4),D0
0x0b8e: PUSH.L - D0
0x0b90: MOVEQ - 0x0,D0
0x0b92: JSR - "ZBASIC_194"

0x0b96: PUSH.L - 0 (unsafe)
0x0b98: TRAP - _TickCount
0x0b9a: POP.L - D0
0x0b9c: PUSH.L - D0
0x0b9e: MOVE.L - var_i32_2,D0
0x0ba2: ADDI.L - 0x00000078,D0
0x0ba8: POP.L - D1
0x0baa: CMP.L - D0,D1
0x0bac: BLE - [0x0af6]

0x0bb0: PEA - [0x0bc6]
0x0bb4: MOVEQ - 0x4,D0
0x0bb6: PUSH.L - D0
0x0bb8: PUSH.L - 0
0x0bba: PUSH.L - 0x000001fc
0x0bc0: MOVEQ - 0x3,D0
0x0bc2: JMP - 128,0x50a

0x0bc6: MOVEQ - 0x5,D0
0x0bc8: JSR - 128,0x1f4

0x0bcc: MOVEQ - 0x5,D0
0x0bce: JSR - 128,0x1ba

0x0bd2: PUSH.W - 0x198
0x0bd6: PUSH.W - 0x102
0x0bda: MOVEQ - -0x1,D0
0x0bdc: PUSH.W - D0
0x0bde: PUSH.W - 0
0x0be0: MOVE.L - HEAP(0x94),D0
0x0bea: MOVEA.L - D0,A0
0x0bec: JSR - "PICTURE"

0x0bf0: PEA - VAR(0x32)
0x0bf4: TRAP - _SetPortBits
0x0bf6: MOVEQ - 0x3c,D0
0x0bf8: JSR - 128,0x21e

0x0bfc: PEA - [0x0c12]
0x0c00: MOVEQ - 0x5,D0
0x0c02: PUSH.L - D0
0x0c04: PUSH.L - 0
0x0c06: PUSH.L - 0x000001fc
0x0c0c: MOVEQ - 0x0,D0
0x0c0e: JMP - 128,0x50a

0x0c12: MOVEQ - 0x3c,D0
0x0c14: JSR - 128,0x21e

0x0c18: PUSH.W - 0x197
0x0c1c: PUSH.W - 0x101
0x0c20: MOVEQ - -0x1,D0
0x0c22: PUSH.W - D0
0x0c24: PUSH.W - 0
0x0c26: MOVE.L - HEAP(0x98),D0
0x0c30: MOVEA.L - D0,A0
0x0c32: JSR - "PICTURE"

0x0c36: PUSH.L - 0 (unsafe)
0x0c38: TRAP - _TickCount
0x0c3a: POP.L - D0
0x0c3c: MOVE.L - D0,var_i32_2

0x0c40: PUSH.L - var_i32_3d6
0x0c44: TRAP - _KillPoly
0x0c46: MOVEQ - 0x1c,D0
0x0c48: MOVE.W - D0,var_i16_74

0x0c4c: MOVE.W - var_i16_74,D0
0x0c50: EXT.L - D0
0x0c52: LSL.L - 0x2,D0
0x0c54: ADD.L - HEAP,D0
0x0c58: MOVEA.L - D0,A0
0x0c5a: MOVE.L - (A0),D0
0x0c5c: PUSH.L - D0
0x0c5e: TRAP - _ReleaseResource
0x0c60: MOVEQ - 0x1e,D0
0x0c62: ADDQ.W - 0x1,var_i16_74
0x0c66: CMP.W - var_i16_74,D0
0x0c6a: BGE - [0x0c4c]

0x0c6c: MOVEQ - 0x6,D0
0x0c6e: MOVE.W - D0,var_i16_74

0x0c72: MOVE.L - HEAP(0x412aa),D0
0x0c7c: MOVEA.L - D0,A0
0x0c7e: MOVE.L - (A0),D0
0x0c80: PUSH.L - D0
0x0c82: MOVE.W - var_i16_74,D0
0x0c86: EXT.L - D0
0x0c88: LSL.L - 0x2,D0
0x0c8a: ADDI.L - HEAP(0x41296),D0
0x0c94: MOVEA.L - D0,A0
0x0c96: MOVE.L - (A0),D0
0x0c98: PUSH.L - D0
0x0c9a: MOVE.L - 0x5580,D0
0x0ca0: JSR - "BLOCKMOVE"

0x0ca4: MOVEQ - 0xa,D0
0x0ca6: ADDQ.W - 0x1,var_i16_74
0x0caa: CMP.W - var_i16_74,D0
0x0cae: BGE - [0x0c72]

0x0cb0: MOVEQ - 0x6,D0
0x0cb2: JSR - 128,0x1f4

0x0cb6: MOVEQ - 0x6,D0
0x0cb8: PUSH.W - D0
0x0cba: PUSH.W - 0x8f
0x0cbe: MOVEQ - -0x1,D0
0x0cc0: PUSH.W - D0
0x0cc2: PUSH.W - 0
0x0cc4: MOVE.L - HEAP(0x80),D0
0x0cce: MOVEA.L - D0,A0
0x0cd0: JSR - "PICTURE"

0x0cd4: MOVEQ - 0x7,D0
0x0cd6: JSR - 128,0x1f4

0x0cda: MOVEQ - 0x1a,D0
0x0cdc: PUSH.W - D0
0x0cde: MOVEQ - 0x7e,D0
0x0ce0: PUSH.W - D0
0x0ce2: MOVEQ - -0x1,D0
0x0ce4: PUSH.W - D0
0x0ce6: PUSH.W - 0
0x0ce8: MOVE.L - HEAP(0x84),D0
0x0cf2: MOVEA.L - D0,A0
0x0cf4: JSR - "PICTURE"

0x0cf8: MOVEQ - 0x8,D0
0x0cfa: JSR - 128,0x1f4

0x0cfe: MOVEQ - 0x2e,D0
0x0d00: PUSH.W - D0
0x0d02: MOVEQ - 0x6a,D0
0x0d04: PUSH.W - D0
0x0d06: MOVEQ - -0x1,D0
0x0d08: PUSH.W - D0
0x0d0a: PUSH.W - 0
0x0d0c: MOVE.L - HEAP(0x88),D0
0x0d16: MOVEA.L - D0,A0
0x0d18: JSR - "PICTURE"

0x0d1c: MOVEQ - 0x9,D0
0x0d1e: JSR - 128,0x1f4

0x0d22: MOVEQ - 0x45,D0
0x0d24: PUSH.W - D0
0x0d26: MOVEQ - 0x4e,D0
0x0d28: PUSH.W - D0
0x0d2a: MOVEQ - -0x1,D0
0x0d2c: PUSH.W - D0
0x0d2e: PUSH.W - 0
0x0d30: MOVE.L - HEAP(0x8c),D0
0x0d3a: MOVEA.L - D0,A0
0x0d3c: JSR - "PICTURE"

0x0d40: MOVEQ - 0xa,D0
0x0d42: JSR - 128,0x1f4

0x0d46: MOVEQ - 0x73,D0
0x0d48: PUSH.W - D0
0x0d4a: MOVEQ - 0x28,D0
0x0d4c: PUSH.W - D0
0x0d4e: MOVEQ - -0x1,D0
0x0d50: PUSH.W - D0
0x0d52: PUSH.W - 0
0x0d54: MOVE.L - HEAP(0x90),D0
0x0d5e: MOVEA.L - D0,A0
0x0d60: JSR - "PICTURE"

0x0d64: MOVEQ - 0x20,D0
0x0d66: MOVE.W - D0,var_i16_74

0x0d6a: MOVE.W - var_i16_74,D0
0x0d6e: EXT.L - D0
0x0d70: LSL.L - 0x2,D0
0x0d72: ADD.L - HEAP,D0
0x0d76: MOVEA.L - D0,A0
0x0d78: MOVE.L - (A0),D0
0x0d7a: PUSH.L - D0
0x0d7c: TRAP - _ReleaseResource
0x0d7e: MOVEQ - 0x23,D0
0x0d80: ADDQ.W - 0x1,var_i16_74
0x0d84: CMP.W - var_i16_74,D0
0x0d88: BGE - [0x0d6a]

0x0d8a: PEA - VAR(0x32)
0x0d8e: TRAP - _SetPortBits
0x0d90: MOVEQ - 0x64,D0
0x0d92: JSR - 128,0x24a

0x0d96: PUSH.W - 0x198
0x0d9a: PUSH.W - 0x102
0x0d9e: MOVEQ - -0x1,D0
0x0da0: PUSH.W - D0
0x0da2: PUSH.W - 0
0x0da4: MOVE.L - HEAP(0x94),D0
0x0dae: MOVEA.L - D0,A0
0x0db0: JSR - "PICTURE"

0x0db4: MOVEQ - 0x6,D0
0x0db6: MOVE.W - D0,var_i16_74

0x0dba: MOVEQ - 0xf,D0
0x0dbc: JSR - 128,0x21e

0x0dc0: PEA - [0x0dd8]
0x0dc4: MOVEA.W - var_i16_74,A0
0x0dc8: PUSH.L - A0
0x0dca: PUSH.L - 0
0x0dcc: PUSH.L - 0x000001fc
0x0dd2: MOVEQ - 0x1,D0
0x0dd4: JMP - 128,0x50a

0x0dd8: MOVEQ - 0xa,D0
0x0dda: ADDQ.W - 0x1,var_i16_74
0x0dde: CMP.W - var_i16_74,D0
0x0de2: BGE - [0x0dba]

0x0de4: PUSH.W - 0x191
0x0de8: PUSH.W - 0x102
0x0dec: MOVEQ - -0x1,D0
0x0dee: PUSH.W - D0
0x0df0: PUSH.W - 0
0x0df2: MOVE.L - HEAP(0x9c),D0
0x0dfc: MOVEA.L - D0,A0
0x0dfe: JSR - "PICTURE"

0x0e02: MOVEQ - 0x3c,D0
0x0e04: JSR - 128,0x21e

0x0e08: PUSH.W - 0xfa
0x0e0c: MOVEQ - 0xc,D0
0x0e0e: PUSH.W - D0
0x0e10: PUSH.W - 0
0x0e12: MOVEQ - 0x3,D0
0x0e14: PUSH.W - D0
0x0e16: JSR - "TEXT"

0x0e1a: MOVEQ - 0x22,D0
0x0e1c: JSR - "CHR$"

0x0e20: JSR - "BUFFER_CONCAT"

0x0e24: MOVEA.L - STR(0x286),A0  # b'Are you still angry with me?'
0x0e2c: JSR - "BUFFER_CONCAT"

0x0e30: MOVEQ - 0x22,D0
0x0e32: JSR - "CHR$"

0x0e36: JSR - "BUFFER_CONCAT"

0x0e3a: LEA - VAR(0x76),A0
0x0e3e: JSR - "BUFFER_FLUSH"

0x0e42: PEA - [0x0e6c]
0x0e46: MOVE.L - 0x1f4,D0
0x0e4c: PUSH.L - D0
0x0e4e: PUSH.W - 0 (unsafe)
0x0e50: PEA - VAR(0x76)
0x0e54: TRAP - _StringWidth
0x0e56: POP.W - D0
0x0e58: EXT.L - D0
0x0e5a: POP.L - D1
0x0e5c: EXG - D0,D1
0x0e5e: SUB.L - D1,D0
0x0e60: PUSH.L - D0
0x0e62: MOVE.L - 0xd2,D0
0x0e68: JMP - 128,0xa6c

0x0e6c: PUSH.L - 0 (unsafe)
0x0e6e: TRAP - _TickCount
0x0e70: POP.L - D0
0x0e72: MOVE.L - D0,var_i32_2

0x0e76: MOVE.L - HEAP(0x412a6),D0
0x0e80: MOVEA.L - D0,A0
0x0e82: MOVE.L - (A0),D0
0x0e84: PUSH.L - D0
0x0e86: MOVE.L - HEAP(0x412aa),D0
0x0e90: MOVEA.L - D0,A0
0x0e92: MOVE.L - (A0),D0
0x0e94: PUSH.L - D0
0x0e96: MOVE.L - 0x5580,D0
0x0e9c: JSR - "BLOCKMOVE"

0x0ea0: MOVEQ - 0x5,D0
0x0ea2: JSR - 128,0x1f4

0x0ea6: MOVEQ - 0x73,D0
0x0ea8: PUSH.W - D0
0x0eaa: MOVEQ - 0x28,D0
0x0eac: PUSH.W - D0
0x0eae: MOVEQ - -0x1,D0
0x0eb0: PUSH.W - D0
0x0eb2: PUSH.W - 0
0x0eb4: MOVE.L - HEAP(0x90),D0
0x0ebe: MOVEA.L - D0,A0
0x0ec0: JSR - "PICTURE"

0x0ec4: PUSH.W - 0x191
0x0ec8: PUSH.W - 0x102
0x0ecc: MOVEQ - -0x1,D0
0x0ece: PUSH.W - D0
0x0ed0: PUSH.W - 0
0x0ed2: MOVE.L - HEAP(0x9c),D0
0x0edc: MOVEA.L - D0,A0
0x0ede: JSR - "PICTURE"

0x0ee2: PUSH.W - 0x8a
0x0ee6: MOVEQ - 0x24,D0
0x0ee8: PUSH.W - D0
0x0eea: MOVEQ - -0x1,D0
0x0eec: PUSH.W - D0
0x0eee: PUSH.W - 0
0x0ef0: MOVE.L - HEAP(0xa0),D0
0x0efa: MOVEA.L - D0,A0
0x0efc: JSR - "PICTURE"

0x0f00: MOVEQ - 0x6,D0
0x0f02: JSR - 128,0x1f4

0x0f06: PEA - [0x0f20]
0x0f0a: PUSH.L - 0
0x0f0c: PUSH.L - 0
0x0f0e: PUSH.L - 0x00000152
0x0f14: PUSH.L - 0x000001fc
0x0f1a: MOVEQ - 0x0,D0
0x0f1c: JMP - 128,0x2f0

0x0f20: PUSH.W - 0x118
0x0f24: MOVEQ - 0x19,D0
0x0f26: PUSH.W - D0
0x0f28: MOVEQ - -0x1,D0
0x0f2a: PUSH.W - D0
0x0f2c: PUSH.W - 0
0x0f2e: MOVE.L - HEAP(0xa4),D0
0x0f38: MOVEA.L - D0,A0
0x0f3a: JSR - "PICTURE"

0x0f3e: MOVEQ - 0x7,D0
0x0f40: JSR - 128,0x1f4

0x0f44: PEA - [0x0f5e]
0x0f48: PUSH.L - 0
0x0f4a: PUSH.L - 0
0x0f4c: PUSH.L - 0x00000152
0x0f52: PUSH.L - 0x000001fc
0x0f58: MOVEQ - 0x0,D0
0x0f5a: JMP - 128,0x2f0

0x0f5e: PUSH.W - 0x118
0x0f62: MOVEQ - 0x19,D0
0x0f64: PUSH.W - D0
0x0f66: MOVEQ - -0x1,D0
0x0f68: PUSH.W - D0
0x0f6a: PUSH.W - 0
0x0f6c: MOVE.L - HEAP(0xa8),D0
0x0f76: MOVEA.L - D0,A0
0x0f78: JSR - "PICTURE"

0x0f7c: MOVE.L - HEAP(0x412aa),D0
0x0f86: MOVEA.L - D0,A0
0x0f88: MOVE.L - (A0),D0
0x0f8a: PUSH.L - D0
0x0f8c: MOVE.L - HEAP(0x412b6),D0
0x0f96: MOVEA.L - D0,A0
0x0f98: MOVE.L - (A0),D0
0x0f9a: PUSH.L - D0
0x0f9c: MOVE.L - 0x5580,D0
0x0fa2: JSR - "BLOCKMOVE"

0x0fa6: MOVEQ - 0x8,D0
0x0fa8: JSR - 128,0x1f4

0x0fac: PUSH.W - 0x18f
0x0fb0: PUSH.W - 0xc0
0x0fb4: MOVEQ - -0x1,D0
0x0fb6: PUSH.W - D0
0x0fb8: PUSH.W - 0
0x0fba: MOVE.L - HEAP(0xac),D0
0x0fc4: MOVEA.L - D0,A0
0x0fc6: JSR - "PICTURE"

0x0fca: MOVE.L - HEAP(0x412a6),D0
0x0fd4: MOVEA.L - D0,A0
0x0fd6: MOVE.L - (A0),D0
0x0fd8: PUSH.L - D0
0x0fda: MOVE.L - HEAP(0x412ba),D0
0x0fe4: MOVEA.L - D0,A0
0x0fe6: MOVE.L - (A0),D0
0x0fe8: PUSH.L - D0
0x0fea: MOVE.L - 0x5580,D0
0x0ff0: JSR - "BLOCKMOVE"

0x0ff4: MOVEQ - 0x9,D0
0x0ff6: JSR - 128,0x1f4

0x0ffa: PUSH.W - 0x18f
0x0ffe: PUSH.W - 0xc0
0x1002: MOVEQ - -0x1,D0
0x1004: PUSH.W - D0
0x1006: PUSH.W - 0
0x1008: MOVE.L - HEAP(0xac),D0
0x1012: MOVEA.L - D0,A0
0x1014: JSR - "PICTURE"

0x1018: MOVEQ - 0xa,D0
0x101a: JSR - 128,0x1f4

0x101e: PEA - [0x1038]
0x1022: PUSH.L - 0
0x1024: PUSH.L - 0
0x1026: PUSH.L - 0x00000152
0x102c: PUSH.L - 0x000001fc
0x1032: MOVEQ - 0x2,D0
0x1034: JMP - 128,0x2f0

0x1038: PUSH.W - 0x18f
0x103c: PUSH.W - 0xc0
0x1040: MOVEQ - -0x1,D0
0x1042: PUSH.W - D0
0x1044: PUSH.W - 0
0x1046: MOVE.L - HEAP(0xac),D0
0x1050: MOVEA.L - D0,A0
0x1052: JSR - "PICTURE"

0x1056: MOVE.L - HEAP(0x90),D0
0x1060: MOVEA.L - D0,A0
0x1062: MOVE.L - (A0),D0
0x1064: PUSH.L - D0
0x1066: TRAP - _ReleaseResource
0x1068: PEA - VAR(0x32)
0x106c: TRAP - _SetPortBits
0x106e: MOVEQ - 0x78,D0
0x1070: JSR - 128,0x24a

0x1074: PEA - [0x1096]
0x1078: PUSH.L - 0x000000c4
0x107e: PUSH.L - 0x00000141
0x1084: PUSH.L - 0x000000d3
0x108a: PUSH.L - 0x000001fc
0x1090: MOVEQ - 0x2,D0
0x1092: JMP - 128,0x2f0

0x1096: PEA - [0x10ae]
0x109a: MOVEQ - 0x5,D0
0x109c: PUSH.L - D0
0x109e: MOVEQ - 0x64,D0
0x10a0: PUSH.L - D0
0x10a2: PUSH.L - 0x00000198
0x10a8: MOVEQ - 0x0,D0
0x10aa: JMP - 128,0x50a

0x10ae: MOVEQ - 0x28,D0
0x10b0: JSR - 128,0x21e

0x10b4: MOVE.W - 0x118,var_i16_5e

0x10ba: MOVE.W - 0x1d1,var_i16_62

0x10c0: MOVEQ - 0x19,D0
0x10c2: MOVE.W - D0,var_i16_5c

0x10c6: MOVE.W - 0x118,var_i16_66

0x10cc: MOVE.W - 0x1d1,var_i16_6a

0x10d2: MOVE.W - 0xdf,var_i16_68

0x10d8: MOVE.W - 0xdc,var_i16_74

0x10de: PUSH.L - 0 (unsafe)
0x10e0: TRAP - _TickCount
0x10e2: POP.L - D0
0x10e4: MOVE.L - D0,var_i32_2

0x10e8: MOVE.L - HEAP(0x412aa),D0
0x10f2: MOVEA.L - D0,A0
0x10f4: MOVE.L - (A0),D0
0x10f6: PUSH.L - D0
0x10f8: PUSH.L - var_i32_4e
0x10fc: MOVE.L - 0x5580,D0
0x1102: JSR - "BLOCKMOVE"

0x1106: MOVE.W - var_i16_74,var_i16_64

0x110c: MOVEQ - 0x19,D0
0x110e: PUSH.L - D0
0x1110: MOVE.L - 0xdf,D0
0x1116: MOVEA.W - var_i16_74,A0
0x111a: SUB.L - A0,D0
0x111c: POP.L - D1
0x111e: ADD.L - D1,D0
0x1120: MOVE.W - D0,var_i16_60

0x1124: MOVE.L - HEAP(0x412ae),D0
0x112e: MOVEA.L - D0,A0
0x1130: MOVE.L - (A0),D0
0x1132: MOVE.L - D0,var_i32_40

0x1136: PEA - VAR(0x40)
0x113a: PEA - VAR(0x4e)
0x113e: PEA - VAR(0x5c)
0x1142: PEA - VAR(0x64)
0x1146: MOVEQ - 0x3,D0
0x1148: PUSH.W - D0
0x114a: PUSH.L - 0
0x114c: TRAP - _CopyBits
0x114e: MOVE.L - HEAP(0x412b2),D0
0x1158: MOVEA.L - D0,A0
0x115a: MOVE.L - (A0),D0
0x115c: MOVE.L - D0,var_i32_40

0x1160: PEA - VAR(0x40)
0x1164: PEA - VAR(0x4e)
0x1168: PEA - VAR(0x5c)
0x116c: PEA - VAR(0x64)
0x1170: MOVEQ - 0x1,D0
0x1172: PUSH.W - D0
0x1174: PUSH.L - 0
0x1176: TRAP - _CopyBits
0x1178: MOVEQ - 0x2,D0
0x117a: JSR - 128,0x24a

0x117e: PEA - VAR(0x4e)
0x1182: PEA - VAR(0x32)
0x1186: PEA - VAR(0x64)
0x118a: PEA - VAR(0x64)
0x118e: PUSH.W - 0
0x1190: PUSH.L - 0
0x1192: TRAP - _CopyBits
0x1194: MOVEQ - 0x19,D0
0x1196: ADDI.W - 0xfffb,var_i16_74
0x119c: CMP.W - var_i16_74,D0
0x11a0: BLE - [0x10de]

0x11a4: MOVEQ - 0x5,D0
0x11a6: JSR - 128,0x1ba

0x11aa: MOVEQ - 0x5,D0
0x11ac: JSR - 128,0x1f4

0x11b0: PEA - [0x11d2]
0x11b4: PUSH.L - 0x000000c4
0x11ba: PUSH.L - 0x00000141
0x11c0: PUSH.L - 0x000000d3
0x11c6: PUSH.L - 0x000001fc
0x11cc: MOVEQ - 0x2,D0
0x11ce: JMP - 128,0x2f0

0x11d2: PUSH.W - 0x18f
0x11d6: PUSH.W - 0xc0
0x11da: MOVEQ - -0x1,D0
0x11dc: PUSH.W - D0
0x11de: PUSH.W - 0
0x11e0: MOVE.L - HEAP(0xac),D0
0x11ea: MOVEA.L - D0,A0
0x11ec: JSR - "PICTURE"

0x11f0: PEA - VAR(0x32)
0x11f4: TRAP - _SetPortBits
0x11f6: TRAP - _PenNormal
0x11f8: MOVEQ - 0xa,D0
0x11fa: PUSH.W - D0
0x11fc: TRAP - _PenMode
0x11fe: MOVEQ - 0x5,D0
0x1200: PUSH.W - D0
0x1202: MOVEQ - 0x5,D0
0x1204: PUSH.W - D0
0x1206: TRAP - _PenSize
0x1208: MOVEQ - 0x0,D0
0x120a: MOVE.W - D0,var_i16_74

0x120e: PUSH.L - 0 (unsafe)
0x1210: TRAP - _TickCount
0x1212: POP.L - D0
0x1214: MOVE.L - D0,var_i32_2

0x1218: PUSH.W - 0x172
0x121c: MOVEQ - 0x33,D0
0x121e: PUSH.W - D0
0x1220: TRAP - _MoveTo
0x1222: PUSH.W - var_i16_74
0x1226: PUSH.W - 0x14a
0x122a: TRAP - _LineTo
0x122c: CMPI.W - 0x4e,var_i16_74
0x1232: BEQ - [0x1238]

0x1234: JMP - [0x12a4]
--------
0x1238: PUSH.W - 0xfa
0x123c: MOVEQ - 0xc,D0
0x123e: PUSH.W - D0
0x1240: PUSH.W - 0
0x1242: MOVEQ - 0x3,D0
0x1244: PUSH.W - D0
0x1246: JSR - "TEXT"

0x124a: MOVEQ - 0x22,D0
0x124c: JSR - "CHR$"

0x1250: JSR - "BUFFER_CONCAT"

0x1254: MOVEA.L - STR(0x2a4),A0  # b'Yes, apparently so. . .'
0x125c: JSR - "BUFFER_CONCAT"

0x1260: MOVEQ - 0x22,D0
0x1262: JSR - "CHR$"

0x1266: JSR - "BUFFER_CONCAT"

0x126a: LEA - VAR(0x76),A0
0x126e: JSR - "BUFFER_FLUSH"

0x1272: PEA - [0x1286]
0x1276: PUSH.L - 0x000001f0
0x127c: MOVE.L - 0xd2,D0
0x1282: JMP - 128,0x26c

0x1286: PUSH.W - 0x198
0x128a: PUSH.W - 0x102
0x128e: MOVEQ - -0x1,D0
0x1290: PUSH.W - D0
0x1292: PUSH.W - 0
0x1294: MOVE.L - HEAP(0x94),D0
0x129e: MOVEA.L - D0,A0
0x12a0: JSR - "PICTURE"

0x12a4: MOVEQ - 0x2,D0
0x12a6: JSR - 128,0x24a

0x12aa: PUSH.W - 0x172
0x12ae: MOVEQ - 0x33,D0
0x12b0: PUSH.W - D0
0x12b2: TRAP - _MoveTo
0x12b4: PUSH.W - var_i16_74
0x12b8: PUSH.W - 0x14a
0x12bc: TRAP - _LineTo
0x12be: ADDQ.W - 0x6,var_i16_74
0x12c2: CMPI.W - 0x1b8,var_i16_74
0x12c8: BLE - [0x120e]

0x12cc: PEA - [0x1306]
0x12d0: PUSH.L - 0x00000146
0x12d6: PUSH.L - 0x000001b4
0x12dc: PUSH.L - 0x00000146
0x12e2: PUSH.L - 0x000001b4
0x12e8: PUSH.L - 0x000000bc
0x12ee: PUSH.L - 0x0000018b
0x12f4: PUSH.L - 0x00000152
0x12fa: PUSH.L - 0x000001e2
0x1300: MOVEQ - 0x10,D0
0x1302: JMP - 128,0x800

0x1306: PEA - [0x1320]
0x130a: MOVEQ - 0x5,D0
0x130c: PUSH.L - D0
0x130e: PUSH.L - 0x00000128
0x1314: PUSH.L - 0x000001fc
0x131a: MOVEQ - 0x1,D0
0x131c: JMP - 128,0x50a

0x1320: MOVEQ - 0x3c,D0
0x1322: JSR - 128,0x21e

0x1326: MOVEQ - 0x8,D0
0x1328: MOVE.W - D0,var_i16_74

0x132c: PEA - [0x1344]
0x1330: MOVEA.W - var_i16_74,A0
0x1334: PUSH.L - A0
0x1336: PUSH.L - 0
0x1338: PUSH.L - 0x000001fc
0x133e: MOVEQ - 0x1,D0
0x1340: JMP - 128,0x50a

0x1344: MOVEQ - 0xa,D0
0x1346: ADDQ.W - 0x1,var_i16_74
0x134a: CMP.W - var_i16_74,D0
0x134e: BGE - [0x132c]

0x1350: PUSH.L - 0 (unsafe)
0x1352: TRAP - _TickCount
0x1354: POP.L - D0
0x1356: MOVE.L - D0,var_i32_2

0x135a: MOVEQ - 0x5,D0
0x135c: JSR - 128,0x1f4

0x1360: PEA - [0x137a]
0x1364: PUSH.L - 0
0x1366: PUSH.L - 0
0x1368: PUSH.L - 0x00000152
0x136e: PUSH.L - 0x000001fc
0x1374: MOVEQ - 0x0,D0
0x1376: JMP - 128,0x2f0

0x137a: PUSH.W - 0
0x137c: PUSH.W - 0
0x137e: MOVEQ - -0x1,D0
0x1380: PUSH.W - D0
0x1382: PUSH.W - 0
0x1384: MOVE.L - HEAP(0xbc),D0
0x138e: MOVEA.L - D0,A0
0x1390: JSR - "PICTURE"

0x1394: PEA - VAR(0x32)
0x1398: TRAP - _SetPortBits
0x139a: MOVEQ - 0xa,D0
0x139c: JSR - 128,0x24a

0x13a0: MOVE.W - 0x18b,var_i16_5e

0x13a6: MOVE.W - 0x1ef,var_i16_62

0x13ac: MOVE.W - 0x18b,var_i16_66

0x13b2: MOVE.W - 0x1ef,var_i16_6a

0x13b8: MOVE.W - 0xc0,var_i16_74

0x13be: PUSH.L - 0 (unsafe)
0x13c0: TRAP - _TickCount
0x13c2: POP.L - D0
0x13c4: MOVE.L - D0,var_i32_2

0x13c8: MOVE.W - var_i16_74,var_i16_5c

0x13ce: MOVE.W - var_i16_5c,D0
0x13d2: EXT.L - D0
0x13d4: ADDI.L - 0x00000096,D0
0x13da: MOVE.W - D0,var_i16_60

0x13de: MOVE.W - var_i16_74,D0
0x13e2: EXT.L - D0
0x13e4: SUBQ.L - 0x1,D0
0x13e6: MOVE.W - D0,var_i16_64

0x13ea: MOVE.W - var_i16_64,D0
0x13ee: EXT.L - D0
0x13f0: ADDI.L - 0x00000096,D0
0x13f6: MOVE.W - D0,var_i16_68

0x13fa: PEA - VAR(0x32)
0x13fe: PEA - VAR(0x32)
0x1402: PEA - VAR(0x5c)
0x1406: PEA - VAR(0x64)
0x140a: PUSH.W - 0
0x140c: PUSH.L - 0
0x140e: TRAP - _CopyBits
0x1410: MOVEQ - 0x1,D0
0x1412: JSR - 128,0x24a

0x1416: MOVEQ - 0x15,D0
0x1418: ADDI.W - 0xffff,var_i16_74
0x141e: CMP.W - var_i16_74,D0
0x1422: BLE - [0x13be]

0x1424: PUSH.W - 0x193
0x1428: MOVEQ - 0x2d,D0
0x142a: PUSH.W - D0
0x142c: MOVEQ - -0x1,D0
0x142e: PUSH.W - D0
0x1430: PUSH.W - 0
0x1432: MOVE.L - HEAP(0xb0),D0
0x143c: MOVEA.L - D0,A0
0x143e: JSR - "PICTURE"

0x1442: MOVEQ - 0x1,D0
0x1444: JSR - 128,0x21e

0x1448: PUSH.W - 0x193
0x144c: MOVEQ - 0x2d,D0
0x144e: PUSH.W - D0
0x1450: MOVEQ - -0x1,D0
0x1452: PUSH.W - D0
0x1454: PUSH.W - 0
0x1456: MOVE.L - HEAP(0xb4),D0
0x1460: MOVEA.L - D0,A0
0x1462: JSR - "PICTURE"

0x1466: MOVEQ - 0x1,D0
0x1468: JSR - 128,0x21e

0x146c: PUSH.W - 0x193
0x1470: MOVEQ - 0x2d,D0
0x1472: PUSH.W - D0
0x1474: MOVEQ - -0x1,D0
0x1476: PUSH.W - D0
0x1478: PUSH.W - 0
0x147a: MOVE.L - HEAP(0xb8),D0
0x1484: MOVEA.L - D0,A0
0x1486: JSR - "PICTURE"

0x148a: MOVEQ - 0x6,D0
0x148c: JSR - 128,0x1ba

0x1490: TRAP - _PenNormal
0x1492: MOVEQ - 0xa,D0
0x1494: PUSH.W - D0
0x1496: TRAP - _PenMode
0x1498: MOVEQ - 0x3,D0
0x149a: PUSH.W - D0
0x149c: MOVEQ - 0x3,D0
0x149e: PUSH.W - D0
0x14a0: TRAP - _PenSize
0x14a2: MOVE.W - 0x190,var_i16_6e

0x14a8: MOVE.W - 0x200,var_i16_72

0x14ae: MOVEQ - 0x50,D0
0x14b0: MOVE.W - D0,var_i16_6c

0x14b4: MOVE.W - 0x156,var_i16_70

0x14ba: MOVEQ - 0x0,D0
0x14bc: MOVE.W - D0,var_i16_5e

0x14c0: MOVEQ - 0x32,D0
0x14c2: MOVE.W - D0,var_i16_62

0x14c6: MOVEQ - 0x0,D0
0x14c8: MOVE.W - D0,var_i16_5c

0x14cc: MOVEQ - 0x32,D0
0x14ce: MOVE.W - D0,var_i16_60

0x14d2: MOVEQ - 0x64,D0
0x14d4: MOVE.W - D0,var_i16_3e0

0x14d8: MOVE.L - HEAP(0x412aa),D0
0x14e2: MOVEA.L - D0,A0
0x14e4: MOVE.L - (A0),D0
0x14e6: MOVE.L - D0,var_i32_40

0x14ea: PEA - VAR(0x4e)
0x14ee: TRAP - _SetPortBits
0x14f0: MOVEQ - 0x1,D0
0x14f2: MOVE.W - D0,var_i16_74

0x14f6: PUSH.L - 0 (unsafe)
0x14f8: TRAP - _TickCount
0x14fa: POP.L - D0
0x14fc: MOVE.L - D0,var_i32_2

0x1500: MOVE.L - HEAP(0x412ae),D0
0x150a: MOVEA.L - D0,A0
0x150c: MOVE.L - (A0),D0
0x150e: PUSH.L - D0
0x1510: PUSH.L - var_i32_4e
0x1514: MOVE.L - 0x5580,D0
0x151a: JSR - "BLOCKMOVE"

0x151e: MOVE.W - var_i16_3e0,D0
0x1522: EXT.L - D0
0x1524: ADDQ.L - 0x5,D0
0x1526: PUSH.L - D0
0x1528: MOVE.W - var_i16_74,D0
0x152c: EXT.L - D0
0x152e: MOVE.L - D0,D1
0x1530: MOVEQ - 0x2,D0
0x1532: JSR - "MUL_INT"

0x1536: POP.L - D1
0x1538: ADD.L - D1,D0
0x153a: MOVE.W - D0,var_i16_3e0

0x153e: MOVE.W - 0x1e0,var_i16_66

0x1544: MOVE.W - var_i16_66,D0
0x1548: EXT.L - D0
0x154a: ADDI.L - 0x00000032,D0
0x1550: MOVE.W - D0,var_i16_6a

0x1554: MOVE.W - var_i16_3e0,var_i16_64

0x155a: MOVE.W - var_i16_64,D0
0x155e: EXT.L - D0
0x1560: ADDI.L - 0x00000032,D0
0x1566: MOVE.W - D0,var_i16_68

0x156a: PEA - VAR(0x40)
0x156e: PEA - VAR(0x4e)
0x1572: PEA - VAR(0x5c)
0x1576: PEA - VAR(0x64)
0x157a: MOVEQ - 0x2,D0
0x157c: PUSH.W - D0
0x157e: PUSH.L - 0
0x1580: TRAP - _CopyBits
0x1582: MOVE.L - 0x190,D0
0x1588: MOVEA.W - var_i16_74,A0
0x158c: SUB.L - A0,D0
0x158e: PUSH.W - D0
0x1590: MOVE.W - var_i16_3e0,D0
0x1594: EXT.L - D0
0x1596: SUBI.L - 0x1e,D0
0x159c: PUSH.L - D0
0x159e: MOVE.W - var_i16_74,D0
0x15a2: EXT.L - D0
0x15a4: MOVE.L - D0,D1
0x15a6: MOVEQ - 0x3,D0
0x15a8: JSR - "MUL_INT"

0x15ac: POP.L - D1
0x15ae: ADD.L - D1,D0
0x15b0: PUSH.W - D0
0x15b2: TRAP - _MoveTo
0x15b4: PUSH.W - 0x1f4
0x15b8: MOVE.W - var_i16_3e0,D0
0x15bc: EXT.L - D0
0x15be: ADDI.L - 0x0000000a,D0
0x15c4: MOVEA.W - var_i16_74,A0
0x15c8: SUB.L - A0,D0
0x15ca: PUSH.W - D0
0x15cc: TRAP - _LineTo
0x15ce: MOVEQ - 0x1,D0
0x15d0: JSR - 128,0x24a

0x15d4: PEA - VAR(0x4e)
0x15d8: PEA - VAR(0x32)
0x15dc: PEA - VAR(0x6c)
0x15e0: PEA - VAR(0x6c)
0x15e4: PUSH.W - 0
0x15e6: PUSH.L - 0
0x15e8: TRAP - _CopyBits
0x15ea: MOVEQ - 0xf,D0
0x15ec: ADDQ.W - 0x1,var_i16_74
0x15f0: CMP.W - var_i16_74,D0
0x15f4: BGE - [0x14f6]

0x15f8: PEA - VAR(0x32)
0x15fc: TRAP - _SetPortBits
0x15fe: TRAP - _PenNormal
0x1600: MOVEQ - 0xa,D0
0x1602: PUSH.W - D0
0x1604: TRAP - _PenMode
0x1606: MOVEQ - 0x0,D0
0x1608: MOVE.W - D0,var_i16_192

0x160c: MOVEQ - 0x1,D0
0x160e: MOVE.W - D0,var_i16_74

0x1612: PUSH.L - 0 (unsafe)
0x1614: TRAP - _TickCount
0x1616: POP.L - D0
0x1618: MOVE.L - D0,var_i32_2

0x161c: MOVE.W - var_i16_192,D0
0x1620: BEQ - [0x1626]

0x1622: JMP - [0x1692]
--------
0x1626: MOVEQ - 0x0,D0
0x1628: MOVEA.W - var_i16_74,A0
0x162c: ADD.L - A0,D0
0x162e: LSL.L - 0x1,D0
0x1630: ADDI.L - HEAP(0x1e8),D0
0x163a: PUSH.L - D0
0x163c: MOVE.L - 0x1f4,D0
0x1642: MOVEA.W - var_i16_74,A0
0x1646: SUB.L - A0,D0
0x1648: PUSH.L - D0
0x164a: MOVE.W - var_i16_74,D0
0x164e: EXT.L - D0
0x1650: JSR - "RND_INT"

0x1654: POP.L - D1
0x1656: ADD.L - D1,D0
0x1658: POP.L - A0
0x165a: MOVE.W - D0,(A0)

0x165c: MOVE.L - 0xfb,D0
0x1662: MOVEA.W - var_i16_74,A0
0x1666: ADD.L - A0,D0
0x1668: LSL.L - 0x1,D0
0x166a: ADDI.L - HEAP(0x1e8),D0
0x1674: PUSH.L - D0
0x1676: MOVE.L - 0x156,D0
0x167c: PUSH.L - D0
0x167e: MOVE.W - var_i16_74,D0
0x1682: EXT.L - D0
0x1684: JSR - "RND_INT"

0x1688: POP.L - D1
0x168a: EXG - D0,D1
0x168c: SUB.L - D1,D0
0x168e: POP.L - A0
0x1690: MOVE.W - D0,(A0)

0x1692: MOVEQ - 0x0,D0
0x1694: MOVEA.W - var_i16_74,A0
0x1698: ADD.L - A0,D0
0x169a: LSL.L - 0x1,D0
0x169c: ADDI.L - HEAP(0x1e8),D0
0x16a6: MOVEA.L - D0,A0
0x16a8: MOVE.W - (A0),D0
0x16aa: EXT.L - D0
0x16ac: PUSH.W - D0
0x16ae: MOVE.L - 0xfb,D0
0x16b4: MOVEA.W - var_i16_74,A0
0x16b8: ADD.L - A0,D0
0x16ba: LSL.L - 0x1,D0
0x16bc: ADDI.L - HEAP(0x1e8),D0
0x16c6: MOVEA.L - D0,A0
0x16c8: MOVE.W - (A0),D0
0x16ca: EXT.L - D0
0x16cc: PUSH.W - D0
0x16ce: TRAP - _MoveTo
0x16d0: MOVEQ - 0x0,D0
0x16d2: MOVEA.W - var_i16_74,A0
0x16d6: ADD.L - A0,D0
0x16d8: LSL.L - 0x1,D0
0x16da: ADDI.L - HEAP(0x1e8),D0
0x16e4: MOVEA.L - D0,A0
0x16e6: MOVE.W - (A0),D0
0x16e8: EXT.L - D0
0x16ea: PUSH.W - D0
0x16ec: MOVE.L - 0xfb,D0
0x16f2: MOVEA.W - var_i16_74,A0
0x16f6: ADD.L - A0,D0
0x16f8: LSL.L - 0x1,D0
0x16fa: ADDI.L - HEAP(0x1e8),D0
0x1704: MOVEA.L - D0,A0
0x1706: MOVE.W - (A0),D0
0x1708: EXT.L - D0
0x170a: PUSH.W - D0
0x170c: TRAP - _LineTo
0x170e: MOVE.W - var_i16_74,D0
0x1712: EXT.L - D0
0x1714: MOVE.L - D0,D1
0x1716: MOVEQ - 0x19,D0
0x1718: JSR - "MOD_INT"

0x171c: CMPI.L - 0x0,D0
0x1722: BNE - [0x172c]

0x1726: MOVEQ - 0x1,D0
0x1728: JSR - 128,0x24a

0x172c: ADDQ.W - 0x1,var_i16_74
0x1730: CMPI.W - 0xfa,var_i16_74
0x1736: BLE - [0x1612]

0x173a: MOVEQ - 0x1,D0
0x173c: ADDQ.W - 0x1,var_i16_192
0x1740: CMP.W - var_i16_192,D0
0x1744: BGE - [0x160c]

0x1748: PUSH.L - 0 (unsafe)
0x174a: TRAP - _TickCount
0x174c: POP.L - D0
0x174e: MOVE.L - D0,var_i32_2

0x1752: MOVEQ - 0x25,D0
0x1754: MOVE.W - D0,var_i16_74

0x1758: MOVE.W - var_i16_74,D0
0x175c: EXT.L - D0
0x175e: LSL.L - 0x2,D0
0x1760: ADD.L - HEAP,D0
0x1764: MOVEA.L - D0,A0
0x1766: MOVE.L - (A0),D0
0x1768: PUSH.L - D0
0x176a: TRAP - _ReleaseResource
0x176c: MOVEQ - 0x2f,D0
0x176e: ADDQ.W - 0x1,var_i16_74
0x1772: CMP.W - var_i16_74,D0
0x1776: BGE - [0x1758]

0x1778: MOVEQ - 0x3c,D0
0x177a: JSR - 128,0x24a

0x177e: PUSH.W - 0xfa
0x1782: MOVEQ - 0xc,D0
0x1784: PUSH.W - D0
0x1786: PUSH.W - 0
0x1788: MOVEQ - 0x3,D0
0x178a: PUSH.W - D0
0x178c: JSR - "TEXT"

0x1790: MOVEQ - 0x22,D0
0x1792: JSR - "CHR$"

0x1796: JSR - "BUFFER_CONCAT"

0x179a: MOVEA.L - STR(0x2bc),A0  # b"Well, this won't do. . ."
0x17a2: JSR - "BUFFER_CONCAT"

0x17a6: MOVEQ - 0x22,D0
0x17a8: JSR - "CHR$"

0x17ac: JSR - "BUFFER_CONCAT"

0x17b0: LEA - VAR(0x76),A0
0x17b4: JSR - "BUFFER_FLUSH"

0x17b8: PEA - [0x17c8]
0x17bc: PUSH.L - 0x0000017d
0x17c2: MOVEQ - 0x3c,D0
0x17c4: JMP - 128,0x26c

0x17c8: PUSH.L - 0 (unsafe)
0x17ca: TRAP - _TickCount
0x17cc: POP.L - D0
0x17ce: MOVE.L - D0,var_i32_2

0x17d2: MOVEQ - 0x30,D0
0x17d4: MOVE.W - D0,var_i16_74

0x17d8: MOVE.W - var_i16_74,D0
0x17dc: EXT.L - D0
0x17de: LSL.L - 0x2,D0
0x17e0: ADD.L - HEAP,D0
0x17e4: PUSH.L - D0
0x17e6: PUSH.L - 0 (unsafe)
0x17e8: PUSH.W - var_i16_74
0x17ec: TRAP - _GetPicture
0x17ee: POP.L - D0
0x17f0: POP.L - A0
0x17f2: MOVE.L - D0,(A0)

0x17f4: MOVEQ - 0x31,D0
0x17f6: ADDQ.W - 0x1,var_i16_74
0x17fa: CMP.W - var_i16_74,D0
0x17fe: BGE - [0x17d8]

0x1800: MOVEQ - 0x4b,D0
0x1802: MOVE.W - D0,var_i16_74

0x1806: MOVE.W - var_i16_74,D0
0x180a: EXT.L - D0
0x180c: LSL.L - 0x2,D0
0x180e: ADD.L - HEAP,D0
0x1812: PUSH.L - D0
0x1814: PUSH.L - 0 (unsafe)
0x1816: PUSH.W - var_i16_74
0x181a: TRAP - _GetPicture
0x181c: POP.L - D0
0x181e: POP.L - A0
0x1820: MOVE.L - D0,(A0)

0x1822: MOVEQ - 0x52,D0
0x1824: ADDQ.W - 0x1,var_i16_74
0x1828: CMP.W - var_i16_74,D0
0x182c: BGE - [0x1806]

0x182e: MOVE.L - 0xd2,D0
0x1834: JSR - 128,0x24a

0x1838: PEA - [0x1852]
0x183c: PUSH.L - 0
0x183e: PUSH.L - 0
0x1840: PUSH.L - 0x00000092
0x1846: PUSH.L - 0x00000187
0x184c: MOVEQ - 0x2,D0
0x184e: JMP - 128,0x2f0

0x1852: PUSH.W - 0x193
0x1856: MOVEQ - 0x2d,D0
0x1858: PUSH.W - D0
0x185a: MOVEQ - -0x1,D0
0x185c: PUSH.W - D0
0x185e: PUSH.W - 0
0x1860: MOVE.L - HEAP(0xc0),D0
0x186a: MOVEA.L - D0,A0
0x186c: JSR - "PICTURE"

0x1870: MOVEQ - 0x1,D0
0x1872: JSR - 128,0x21e

0x1876: PUSH.W - 0x193
0x187a: MOVEQ - 0x2d,D0
0x187c: PUSH.W - D0
0x187e: MOVEQ - -0x1,D0
0x1880: PUSH.W - D0
0x1882: PUSH.W - 0
0x1884: MOVE.L - HEAP(0xc4),D0
0x188e: MOVEA.L - D0,A0
0x1890: JSR - "PICTURE"

0x1894: PUSH.W - 0xfa
0x1898: MOVEQ - 0xc,D0
0x189a: PUSH.W - D0
0x189c: PUSH.W - 0
0x189e: MOVEQ - 0x3,D0
0x18a0: PUSH.W - D0
0x18a2: JSR - "TEXT"

0x18a6: MOVEQ - 0x22,D0
0x18a8: JSR - "CHR$"

0x18ac: JSR - "BUFFER_CONCAT"

0x18b0: MOVEA.L - STR(0x2d6),A0  # b'High Priestess!'
0x18b8: JSR - "BUFFER_CONCAT"

0x18bc: LEA - VAR(0x76),A0
0x18c0: JSR - "BUFFER_FLUSH"

0x18c4: PEA - [0x18d4]
0x18c8: PUSH.L - 0x0000017d
0x18ce: MOVEQ - 0x3c,D0
0x18d0: JMP - 128,0x26c

0x18d4: MOVEA.L - STR(0x2e6),A0  # b'  Am I to suffer this fate merely for uncovering'
0x18dc: JSR - "BUFFER_CONCAT"

0x18e0: LEA - VAR(0x76),A0
0x18e4: JSR - "BUFFER_FLUSH"

0x18e8: PEA - [0x18f8]
0x18ec: PUSH.L - 0x0000017d
0x18f2: MOVEQ - 0x4d,D0
0x18f4: JMP - 128,0x26c

0x18f8: MOVEA.L - STR(0x318),A0  # b' the fourteen lost treasures of the land?'
0x1900: JSR - "BUFFER_CONCAT"

0x1904: MOVEQ - 0x22,D0
0x1906: JSR - "CHR$"

0x190a: JSR - "BUFFER_CONCAT"

0x190e: LEA - VAR(0x76),A0
0x1912: JSR - "BUFFER_FLUSH"

0x1916: PEA - [0x1926]
0x191a: PUSH.L - 0x0000017d
0x1920: MOVEQ - 0x5e,D0
0x1922: JMP - 128,0x26c

0x1926: MOVEQ - 0x30,D0
0x1928: MOVE.W - D0,var_i16_74

0x192c: MOVE.W - var_i16_74,D0
0x1930: EXT.L - D0
0x1932: LSL.L - 0x2,D0
0x1934: ADD.L - HEAP,D0
0x1938: MOVEA.L - D0,A0
0x193a: MOVE.L - (A0),D0
0x193c: PUSH.L - D0
0x193e: TRAP - _ReleaseResource
0x1940: MOVEQ - 0x31,D0
0x1942: ADDQ.W - 0x1,var_i16_74
0x1946: CMP.W - var_i16_74,D0
0x194a: BGE - [0x192c]

0x194c: MOVEQ - 0x32,D0
0x194e: MOVE.W - D0,var_i16_74

0x1952: MOVE.W - var_i16_74,D0
0x1956: EXT.L - D0
0x1958: LSL.L - 0x2,D0
0x195a: ADD.L - HEAP,D0
0x195e: PUSH.L - D0
0x1960: PUSH.L - 0 (unsafe)
0x1962: PUSH.W - var_i16_74
0x1966: TRAP - _GetPicture
0x1968: POP.L - D0
0x196a: POP.L - A0
0x196c: MOVE.L - D0,(A0)

0x196e: CMPI.W - 0x37,var_i16_74
0x1974: BEQ - [0x197a]

0x1976: JMP - [0x19b2]
--------
0x197a: MOVEQ - 0x5,D0
0x197c: JSR - 128,0x1f4

0x1980: PUSH.W - 0
0x1982: PUSH.W - 0
0x1984: MOVEQ - -0x1,D0
0x1986: PUSH.W - D0
0x1988: PUSH.W - 0
0x198a: MOVE.L - HEAP(0xdc),D0
0x1994: MOVEA.L - D0,A0
0x1996: JSR - "PICTURE"

0x199a: MOVE.L - HEAP(0xdc),D0
0x19a4: MOVEA.L - D0,A0
0x19a6: MOVE.L - (A0),D0
0x19a8: PUSH.L - D0
0x19aa: TRAP - _ReleaseResource
0x19ac: PEA - VAR(0x32)
0x19b0: TRAP - _SetPortBits
0x19b2: MOVEQ - 0x38,D0
0x19b4: ADDQ.W - 0x1,var_i16_74
0x19b8: CMP.W - var_i16_74,D0
0x19bc: BGE - [0x1952]

0x19be: JSR - 128,0xe1c

0x19c2: JSR - 128,0xe58

0x19c6: JSR - 128,0xe1c

0x19ca: PEA - [0x19e4]
0x19ce: PUSH.L - 0
0x19d0: PUSH.L - 0
0x19d2: PUSH.L - 0x00000092
0x19d8: PUSH.L - 0x00000187
0x19de: MOVEQ - 0x2,D0
0x19e0: JMP - 128,0x2f0

0x19e4: PUSH.W - 0x193
0x19e8: MOVEQ - 0x2d,D0
0x19ea: PUSH.W - D0
0x19ec: MOVEQ - -0x1,D0
0x19ee: PUSH.W - D0
0x19f0: PUSH.W - 0
0x19f2: MOVE.L - HEAP(0xc8),D0
0x19fc: MOVEA.L - D0,A0
0x19fe: JSR - "PICTURE"

0x1a02: PUSH.W - 0xfa
0x1a06: MOVEQ - 0xc,D0
0x1a08: PUSH.W - D0
0x1a0a: PUSH.W - 0
0x1a0c: MOVEQ - 0x3,D0
0x1a0e: PUSH.W - D0
0x1a10: JSR - "TEXT"

0x1a14: MOVEQ - 0x22,D0
0x1a16: JSR - "CHR$"

0x1a1a: JSR - "BUFFER_CONCAT"

0x1a1e: MOVEA.L - STR(0x342),A0  # b'. . . for if that be true,'
0x1a26: JSR - "BUFFER_CONCAT"

0x1a2a: LEA - VAR(0x76),A0
0x1a2e: JSR - "BUFFER_FLUSH"

0x1a32: PEA - [0x1a42]
0x1a36: PUSH.L - 0x0000017d
0x1a3c: MOVEQ - 0x3c,D0
0x1a3e: JMP - 128,0x26c

0x1a42: MOVEA.L - STR(0x35e),A0  # b' I wish you had made the challenge more difficult!'
0x1a4a: JSR - "BUFFER_CONCAT"

0x1a4e: LEA - VAR(0x76),A0
0x1a52: JSR - "BUFFER_FLUSH"

0x1a56: PEA - [0x1a66]
0x1a5a: PUSH.L - 0x0000017d
0x1a60: MOVEQ - 0x4d,D0
0x1a62: JMP - 128,0x26c

0x1a66: MOVEA.L - STR(0x392),A0  # b' After all, I am just a fool and I should not'
0x1a6e: JSR - "BUFFER_CONCAT"

0x1a72: LEA - VAR(0x76),A0
0x1a76: JSR - "BUFFER_FLUSH"

0x1a7a: PEA - [0x1a8a]
0x1a7e: PUSH.L - 0x0000017d
0x1a84: MOVEQ - 0x5e,D0
0x1a86: JMP - 128,0x26c

0x1a8a: MOVEA.L - STR(0x3c0),A0  # b' have been able to find them so easily.'
0x1a92: JSR - "BUFFER_CONCAT"

0x1a96: MOVEQ - 0x22,D0
0x1a98: JSR - "CHR$"

0x1a9c: JSR - "BUFFER_CONCAT"

0x1aa0: LEA - VAR(0x76),A0
0x1aa4: JSR - "BUFFER_FLUSH"

0x1aa8: PEA - [0x1ab8]
0x1aac: PUSH.L - 0x0000017d
0x1ab2: MOVEQ - 0x6f,D0
0x1ab4: JMP - 128,0x26c

0x1ab8: MOVE.L - HEAP(0x41af4),D0
0x1ac2: MOVEA.L - D0,A0
0x1ac4: MOVEQ - 0x6,D0
0x1ac6: MOVE.W - D0,(A0)

0x1ac8: MOVE.L - HEAP(0x41af6),D0
0x1ad2: MOVEA.L - D0,A0
0x1ad4: MOVE.W - 0xd0,(A0)

0x1ad8: MOVE.L - HEAP(0x41af8),D0
0x1ae2: MOVEA.L - D0,A0
0x1ae4: MOVEQ - 0x6c,D0
0x1ae6: MOVE.W - D0,(A0)

0x1ae8: MOVE.L - HEAP(0x41afa),D0
0x1af2: MOVEA.L - D0,A0
0x1af4: MOVE.W - 0x133,(A0)

0x1af8: MOVE.L - HEAP(0x41afc),D0
0x1b02: MOVEA.L - D0,A0
0x1b04: MOVEQ - 0xa,D0
0x1b06: MOVE.W - D0,(A0)

0x1b08: MOVE.L - HEAP(0x41afe),D0
0x1b12: MOVEA.L - D0,A0
0x1b14: MOVE.W - 0xb4,(A0)

0x1b18: MOVE.L - HEAP(0x41b00),D0
0x1b22: MOVEA.L - D0,A0
0x1b24: MOVE.W - 0x14c,(A0)

0x1b28: MOVE.L - HEAP(0x41b02),D0
0x1b32: MOVEA.L - D0,A0
0x1b34: MOVE.W - 0x1f6,(A0)

0x1b38: MOVE.L - HEAP(0x41b04),D0
0x1b42: MOVEA.L - D0,A0
0x1b44: MOVEQ - 0x2,D0
0x1b46: MOVE.W - D0,(A0)

0x1b48: MOVE.L - HEAP(0x41b06),D0
0x1b52: MOVEA.L - D0,A0
0x1b54: MOVEQ - 0xa,D0
0x1b56: MOVE.W - D0,(A0)

0x1b58: MOVE.L - HEAP(0x41b08),D0
0x1b62: MOVEA.L - D0,A0
0x1b64: MOVEQ - 0xd,D0
0x1b66: MOVE.W - D0,(A0)

0x1b68: MOVEQ - 0x0,D0
0x1b6a: MOVE.W - D0,var_i16_74

0x1b6e: MOVE.W - var_i16_74,D0
0x1b72: EXT.L - D0
0x1b74: LSL.L - 0x2,D0
0x1b76: ADDI.L - HEAP(0x41bbe),D0
0x1b80: PUSH.L - D0
0x1b82: MOVEA.W - var_i16_74,A0
0x1b86: ADDA.L - A0,A0
0x1b88: ADDA.L - HEAP(0x41af4),A0
0x1b92: MOVE.W - (A0),D0
0x1b94: EXT.L - D0
0x1b96: JSR - "BCD_FROM_INT"

0x1b9a: POP.L - A0
0x1b9c: JSR - "BCD_TO_VAR"

0x1ba0: MOVE.W - var_i16_74,D0
0x1ba4: EXT.L - D0
0x1ba6: ADDQ.L - 0x4,D0
0x1ba8: LSL.L - 0x2,D0
0x1baa: ADDI.L - HEAP(0x41bbe),D0
0x1bb4: PUSH.L - D0
0x1bb6: MOVE.W - var_i16_74,D0
0x1bba: EXT.L - D0
0x1bbc: ADDQ.L - 0x4,D0
0x1bbe: LSL.L - 0x1,D0
0x1bc0: ADDI.L - HEAP(0x41af4),D0
0x1bca: MOVEA.L - D0,A0
0x1bcc: MOVE.W - (A0),D0
0x1bce: EXT.L - D0
0x1bd0: PUSH.L - D0
0x1bd2: MOVEA.W - var_i16_74,A0
0x1bd6: ADDA.L - A0,A0
0x1bd8: ADDA.L - HEAP(0x41af4),A0
0x1be2: MOVE.W - (A0),D0
0x1be4: EXT.L - D0
0x1be6: POP.L - D1
0x1be8: EXG - D0,D1
0x1bea: SUB.L - D1,D0
0x1bec: JSR - "BCD_FROM_INT"

0x1bf0: JSR - "BCD_PUSH_LHS"

0x1bf4: MOVE.L - HEAP(0x41b08),D0
0x1bfe: MOVEA.L - D0,A0
0x1c00: MOVE.W - (A0),D0
0x1c02: EXT.L - D0
0x1c04: JSR - "BCD_FROM_INT"

0x1c08: JSR - "BCD_PUSH_RHS"

0x1c0c: JSR - "DIV_BCD"

0x1c10: POP.L - A0
0x1c12: JSR - "BCD_TO_VAR"

0x1c16: MOVEQ - 0x3,D0
0x1c18: ADDQ.W - 0x1,var_i16_74
0x1c1c: CMP.W - var_i16_74,D0
0x1c20: BGE - [0x1b6e]

0x1c24: MOVE.L - HEAP(0x19c),D0
0x1c2e: PUSH.L - D0
0x1c30: TRAP - _PenPat
0x1c32: MOVEQ - 0xa,D0
0x1c34: PUSH.W - D0
0x1c36: TRAP - _PenMode
0x1c38: MOVEQ - 0x6,D0
0x1c3a: MOVE.W - D0,var_i16_192

0x1c3e: MOVE.W - var_i16_192,D0
0x1c42: EXT.L - D0
0x1c44: SUBQ.L - 0x1,D0
0x1c46: LSL.L - 0x2,D0
0x1c48: ADDI.L - HEAP(0x41296),D0
0x1c52: MOVEA.L - D0,A0
0x1c54: MOVE.L - (A0),D0
0x1c56: PUSH.L - D0
0x1c58: MOVE.W - var_i16_192,D0
0x1c5c: EXT.L - D0
0x1c5e: LSL.L - 0x2,D0
0x1c60: ADDI.L - HEAP(0x41296),D0
0x1c6a: MOVEA.L - D0,A0
0x1c6c: MOVE.L - (A0),D0
0x1c6e: PUSH.L - D0
0x1c70: MOVE.L - 0x5580,D0
0x1c76: JSR - "BLOCKMOVE"

0x1c7a: MOVE.W - var_i16_192,D0
0x1c7e: EXT.L - D0
0x1c80: JSR - 128,0x1f4

0x1c84: CMPI.W - 0x6,var_i16_192
0x1c8a: BNE - [0x1c9c]

0x1c8e: MOVE.L - HEAP(0x41af4),D0
0x1c98: PUSH.L - D0
0x1c9a: TRAP - _PaintOval
0x1c9c: MOVEQ - 0x0,D0
0x1c9e: MOVE.W - D0,var_i16_3e2

0x1ca2: MOVEQ - 0x0,D0
0x1ca4: MOVE.W - D0,var_i16_74

0x1ca8: MOVE.W - var_i16_74,D0
0x1cac: EXT.L - D0
0x1cae: LSL.L - 0x2,D0
0x1cb0: ADDI.L - HEAP(0x41bbe),D0
0x1cba: PUSH.L - D0
0x1cbc: MOVE.W - var_i16_74,D0
0x1cc0: EXT.L - D0
0x1cc2: LSL.L - 0x2,D0
0x1cc4: ADDI.L - HEAP(0x41bbe),D0
0x1cce: MOVEA.L - D0,A0
0x1cd0: JSR - "BCD_FROM_VAR"

0x1cd4: JSR - "BCD_PUSH_LHS"

0x1cd8: MOVE.W - var_i16_74,D0
0x1cdc: EXT.L - D0
0x1cde: ADDQ.L - 0x4,D0
0x1ce0: LSL.L - 0x2,D0
0x1ce2: ADDI.L - HEAP(0x41bbe),D0
0x1cec: MOVEA.L - D0,A0
0x1cee: JSR - "BCD_FROM_VAR"

0x1cf2: JSR - "ADD_BCD"

0x1cf6: POP.L - A0
0x1cf8: JSR - "BCD_TO_VAR"

0x1cfc: MOVE.W - var_i16_74,D0
0x1d00: EXT.L - D0
0x1d02: ADDI.L - 0x0000000b,D0
0x1d08: LSL.L - 0x1,D0
0x1d0a: ADDI.L - HEAP(0x41af4),D0
0x1d14: PUSH.L - D0
0x1d16: MOVE.W - var_i16_74,D0
0x1d1a: EXT.L - D0
0x1d1c: LSL.L - 0x2,D0
0x1d1e: ADDI.L - HEAP(0x41bbe),D0
0x1d28: MOVEA.L - D0,A0
0x1d2a: JSR - "BCD_FROM_VAR"

0x1d2e: JSR - "CAST_INT"

0x1d32: POP.L - A0
0x1d34: MOVE.W - D0,(A0)

0x1d36: MOVEQ - 0x3,D0
0x1d38: ADDQ.W - 0x1,var_i16_74
0x1d3c: CMP.W - var_i16_74,D0
0x1d40: BGE - [0x1ca8]

0x1d44: MOVE.L - HEAP(0x41b0a),D0
0x1d4e: PUSH.L - D0
0x1d50: TRAP - _PaintOval
0x1d52: MOVEQ - 0x2,D0
0x1d54: ADDQ.W - 0x1,var_i16_3e2
0x1d58: CMP.W - var_i16_3e2,D0
0x1d5c: BGE - [0x1ca2]

0x1d60: MOVEQ - 0x9,D0
0x1d62: ADDQ.W - 0x1,var_i16_192
0x1d66: CMP.W - var_i16_192,D0
0x1d6a: BGE - [0x1c3e]

0x1d6e: MOVE.L - HEAP(0x412ba),D0
0x1d78: MOVEA.L - D0,A0
0x1d7a: MOVE.L - (A0),D0
0x1d7c: PUSH.L - D0
0x1d7e: MOVE.L - HEAP(0x412be),D0
0x1d88: MOVEA.L - D0,A0
0x1d8a: MOVE.L - (A0),D0
0x1d8c: PUSH.L - D0
0x1d8e: MOVE.L - 0x5580,D0
0x1d94: JSR - "BLOCKMOVE"

0x1d98: MOVEQ - 0xa,D0
0x1d9a: JSR - 128,0x1f4

0x1d9e: MOVE.L - HEAP(0x41afc),D0
0x1da8: PUSH.L - D0
0x1daa: TRAP - _PaintOval
0x1dac: MOVEQ - 0x5,D0
0x1dae: JSR - 128,0x1f4

0x1db2: MOVE.L - HEAP(0x1bc),D0
0x1dbc: PUSH.L - D0
0x1dbe: PUSH.W - 0xae
0x1dc2: PUSH.W - 0x8c
0x1dc6: PUSH.W - 0xb6
0x1dca: PUSH.W - 0x94
0x1dce: TRAP - _SetRect
0x1dd0: MOVE.L - HEAP(0x1bc),D0
0x1dda: PUSH.L - D0
0x1ddc: TRAP - _InvertOval
0x1dde: MOVE.L - HEAP(0x1bc),D0
0x1de8: PUSH.L - D0
0x1dea: PUSH.W - 0x151
0x1dee: PUSH.W - 0x86
0x1df2: PUSH.W - 0x159
0x1df6: PUSH.W - 0x8e
0x1dfa: TRAP - _SetRect
0x1dfc: MOVE.L - HEAP(0x1bc),D0
0x1e06: PUSH.L - D0
0x1e08: TRAP - _InvertOval
0x1e0a: PEA - VAR(0x32)
0x1e0e: TRAP - _SetPortBits
0x1e10: JSR - 128,0xe1c

0x1e14: JSR - 128,0xe58

0x1e18: PEA - [0x1e32]
0x1e1c: PUSH.L - 0
0x1e1e: PUSH.L - 0
0x1e20: PUSH.L - 0x00000092
0x1e26: PUSH.L - 0x00000187
0x1e2c: MOVEQ - 0x2,D0
0x1e2e: JMP - 128,0x2f0

0x1e32: PEA - [0x1e54]
0x1e36: PUSH.L - 0x00000128
0x1e3c: PUSH.L - 0x0000015a
0x1e42: PUSH.L - 0x00000152
0x1e48: PUSH.L - 0x000001fc
0x1e4e: MOVEQ - 0x2,D0
0x1e50: JMP - 128,0x2f0

0x1e54: MOVEQ - 0x0,D0
0x1e56: MOVE.W - D0,var_i16_18e

0x1e5a: MOVEQ - 0xa,D0
0x1e5c: MOVE.W - D0,var_i16_74

0x1e60: MOVE.L - 0x190,D0
0x1e66: PUSH.L - D0
0x1e68: MOVE.W - var_i16_74,D0
0x1e6c: EXT.L - D0
0x1e6e: ADDI.L - 0x0000000a,D0
0x1e74: POP.L - D1
0x1e76: JSR - "GET_FROM"

0x1e7a: PUSH.L - 0x000001e6
0x1e80: MOVE.W - var_i16_74,D0
0x1e84: EXT.L - D0
0x1e86: ADDI.L - 0x00000014,D0
0x1e8c: PUSH.L - D0
0x1e8e: MOVE.L - 0x1d8a8,D0
0x1e94: PUSH.L - D0
0x1e96: MOVE.W - var_i16_74,D0
0x1e9a: EXT.L - D0
0x1e9c: MOVE.L - D0,D1
0x1e9e: MOVEQ - 0x32,D0
0x1ea0: JSR - "MUL_INT"

0x1ea4: POP.L - D1
0x1ea6: ADD.L - D1,D0
0x1ea8: LSL.L - 0x1,D0
0x1eaa: ADDI.L - HEAP(0xb54),D0
0x1eb4: MOVEA.L - D0,A0
0x1eb6: POP.L - D0
0x1eb8: POP.L - D1
0x1eba: JSR - "GET_TO"

0x1ebe: MOVEQ - 0x0,D0
0x1ec0: MOVEA.W - var_i16_74,A0
0x1ec4: ADD.L - A0,D0
0x1ec6: LSL.L - 0x1,D0
0x1ec8: ADDI.L - HEAP(0x1e8),D0
0x1ed2: MOVEA.L - D0,A0
0x1ed4: MOVE.W - 0x190,(A0)

0x1ed8: ADDQ.W - 0x1,var_i16_18e
0x1edc: CMPI.W - 0x4,var_i16_18e
0x1ee2: BLE - [0x1eec]

0x1ee6: MOVEQ - 0x1,D0
0x1ee8: MOVE.W - D0,var_i16_18e

0x1eec: MOVE.L - 0xfb,D0
0x1ef2: MOVEA.W - var_i16_74,A0
0x1ef6: ADD.L - A0,D0
0x1ef8: LSL.L - 0x1,D0
0x1efa: ADDI.L - HEAP(0x1e8),D0
0x1f04: MOVEA.L - D0,A0
0x1f06: MOVE.W - var_i16_18e,(A0)

0x1f0a: ADDI.W - 0x000a,var_i16_74
0x1f10: CMPI.W - 0x96,var_i16_74
0x1f16: BLE - [0x1e60]

0x1f1a: MOVEQ - 0x1,D0
0x1f1c: MOVE.W - D0,var_i16_192

0x1f20: CMPI.W - 0x9,var_i16_192
0x1f26: BGE - [0x1f30]

0x1f2a: PEA - VAR(0x38)
0x1f2e: TRAP - _InverRect
0x1f30: PUSH.L - 0 (unsafe)
0x1f32: TRAP - _TickCount
0x1f34: POP.L - D0
0x1f36: MOVE.L - D0,var_i32_2

0x1f3a: MOVEQ - 0xa,D0
0x1f3c: MOVE.W - D0,var_i16_74

0x1f40: MOVEQ - 0x0,D0
0x1f42: MOVEA.W - var_i16_74,A0
0x1f46: ADD.L - A0,D0
0x1f48: LSL.L - 0x1,D0
0x1f4a: ADDI.L - HEAP(0x1e8),D0
0x1f54: MOVEA.L - D0,A0
0x1f56: MOVE.W - (A0),D0
0x1f58: EXT.L - D0
0x1f5a: PUSH.L - D0
0x1f5c: MOVE.W - var_i16_74,D0
0x1f60: EXT.L - D0
0x1f62: ADDI.L - 0x0000000a,D0
0x1f68: POP.L - D1
0x1f6a: JSR - "GET_FROM"

0x1f6e: PUSH.W - 0
0x1f70: MOVE.L - 0x1d8a8,D0
0x1f76: PUSH.L - D0
0x1f78: MOVE.W - var_i16_74,D0
0x1f7c: EXT.L - D0
0x1f7e: MOVE.L - D0,D1
0x1f80: MOVEQ - 0x32,D0
0x1f82: JSR - "MUL_INT"

0x1f86: POP.L - D1
0x1f88: ADD.L - D1,D0
0x1f8a: LSL.L - 0x1,D0
0x1f8c: ADDI.L - HEAP(0xb54),D0
0x1f96: PUSH.L - D0
0x1f98: MOVEQ - 0x6,D0
0x1f9a: JSR - "ZBASIC_194"

0x1f9e: MOVEQ - 0x0,D0
0x1fa0: MOVEA.W - var_i16_74,A0
0x1fa4: ADD.L - A0,D0
0x1fa6: LSL.L - 0x1,D0
0x1fa8: ADDI.L - HEAP(0x1e8),D0
0x1fb2: PUSH.L - D0
0x1fb4: MOVEQ - 0x0,D0
0x1fb6: MOVEA.W - var_i16_74,A0
0x1fba: ADD.L - A0,D0
0x1fbc: LSL.L - 0x1,D0
0x1fbe: ADDI.L - HEAP(0x1e8),D0
0x1fc8: MOVEA.L - D0,A0
0x1fca: MOVE.W - (A0),D0
0x1fcc: EXT.L - D0
0x1fce: PUSH.L - D0
0x1fd0: MOVE.L - 0xfb,D0
0x1fd6: MOVEA.W - var_i16_74,A0
0x1fda: ADD.L - A0,D0
0x1fdc: LSL.L - 0x1,D0
0x1fde: ADDI.L - HEAP(0x1e8),D0
0x1fe8: MOVEA.L - D0,A0
0x1fea: MOVE.W - (A0),D0
0x1fec: EXT.L - D0
0x1fee: MOVE.L - D0,D1
0x1ff0: MOVE.W - var_i16_192,D0
0x1ff4: EXT.L - D0
0x1ff6: JSR - "MUL_INT"

0x1ffa: POP.L - D1
0x1ffc: EXG - D0,D1
0x1ffe: SUB.L - D1,D0
0x2000: POP.L - A0
0x2002: MOVE.W - D0,(A0)

0x2004: MOVEQ - 0x0,D0

 */


} // End of namespace Fool
