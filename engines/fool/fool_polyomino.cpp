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

#include "graphics/managed_surface.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

extern ZBasic *g_zbasic;
extern Toolbox *g_toolbox;

// polyomino puzzle
void FoolGame::polyominoRun() {
	// 133:0004
	this->sub_128_271a();
	this->var_i16_c00 = 1;
	for (int i = 0; i <= 0x11; i++) {
		this->arr_i16_1eb8[i] = this->puzzlesReadShort();
	}
	// 133:003c
	this->var_str_384 = this->puzzlesReadString();
	// "the completed puzzle will reveal..."
	this->var_str_384 = g_zbasic->str(209) + this->var_str_384 + g_zbasic->str(210);
	g_zbasic->menu(8, 0xf, 1, this->var_str_384);
	this->var_i16_484 = 0;
	this->var_i16_68c = this->arr_i16_1eb8[8];
	do {
		// 133:009e
		this->var_i16_68a = this->arr_i16_1eb8[10];
		do {
			// 133:00b2
			this->var_i16_484 += 1;
			g_toolbox->SetRect(
				this->arr_rect_1f38[this->var_i16_484],
				this->var_i16_68a,
				this->var_i16_68c,
				this->var_i16_68a + this->arr_i16_1eb8[13],
				this->var_i16_68c + this->arr_i16_1eb8[12]
			);
			// 133:0112
		} while (g_zbasic->incrAndCheck(
			this->var_i16_68a,
			this->arr_i16_1eb8[11],
			this->arr_i16_1eb8[6]
		));
		// 133:0142
	} while (g_zbasic->incrAndCheck(
		this->var_i16_68c,
		this->arr_i16_1eb8[9],
		this->arr_i16_1eb8[7]
	));
	// 133:0172
	this->var_i16_484 = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			// 133:0184
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = 0;
			this->arr_i16_3b38[i*32 + j] = this->var_i16_484;
		}
	}
	// 133:01fa
	for (int i = 0x12; i <= 0x19; i++) {
		this->arr_i16_1eb8[i] = this->puzzlesReadShort();
	}
	// 133:0228
	g_toolbox->SetPort(this->var_i32_f24);
	g_zbasic->text(0xfb, 0x18, 0, kSrcOr);

	for (int j = 1; j <= this->arr_i16_1eb8[0x10]; j++) {
		this->var_i16_103a = this->puzzlesReadShort();
		for (int i = 0; i <= 4; i++) {
			this->arr_i16_47d8[j*8 + i] = this->puzzlesReadShort();
		}
		// 133:0284
		for (int i = 5; i <= 6; i++) {
			this->arr_i16_47d8[j*8 + i] = this->arr_i16_47d8[j*8 + i - 2];
		}
		// 133:02de
		if (this->var_str_c06 != g_zbasic->str(211)) {
			if (j != 1)  {
				this->var_i16_1aa8 = 1;
			}
			for (int i = 3; i <= 4; i++) {
				this->arr_i16_47d8[j*8 + i] = g_zbasic->unk_310(g_zbasic->midStr(this->var_str_c06, i, 2));
				this->var_i16_1aa8 += 2;
			}
		}
		// 133:0356
		for (int i = 0; i <= this->var_i16_103a; i++) {
			this->arr_i16_4338[i] = this->puzzlesReadByte();
		}
		// 133:0388
		this->var_poly_1aac = g_toolbox->OpenPoly();
		g_toolbox->MoveTo(this->arr_i16_4338[1], this->arr_i16_4338[0]);
		for (int i = 2; i <= this->var_i16_103a; i += 2) {
			g_toolbox->LineTo(this->arr_i16_4338[i+1], this->arr_i16_4338[i]);
		}
		g_toolbox->ClosePoly();
		// 133:0408
		this->var_i16_103a = this->puzzlesReadShort();
		if (this->var_i16_103a >= 3) {
			for (int i = 1; i <= this->var_i16_103a; i++) {
				this->arr_i16_3738[i] = this->puzzlesReadByte();
			}
		}
		// 133:044a
		this->arr_i16_1eb8[0x1a] = this->puzzlesReadShort();
		this->arr_i16_1eb8[0x1b] = this->puzzlesReadShort();
		g_toolbox->PenNormal();
		// 133:0476: JSR - "PICTURE_ON"
		PicHandle handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->PenMode(kPatXor);
		if (this->var_i16_103a >= 3) {
			g_toolbox->TextMode(kSrcXor);
			for (int i = 1; i <= this->var_i16_103a; i += 3) {
				g_toolbox->MoveTo(this->arr_i16_3738[i], this->arr_i16_3738[i+1]);
				this->var_str_384 = g_zbasic->chr(this->arr_i16_3738[i+2]);
				g_toolbox->DrawString(this->var_str_384);
			}
		}
		// 133:050a
		g_toolbox->InvertPoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		this->arr_pic_49d8[j*4] = handle;
		g_toolbox->PenNormal();
		// 133:0530: JSR - "PICTURE_ON"
		handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->FillPoly(this->var_poly_1aac, this->arr_pat_58f4[this->arr_i16_1eb8[0x1a]]);
		g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_1eb8[0xd]]);
		g_toolbox->FramePoly(this->var_poly_1aac);
		// 133:0582
		if (this->var_i16_103a >= 3) {
			g_toolbox->TextMode((SourceMode)this->arr_i16_1eb8[0x1b]);
			for (int i = 1; i <= this->var_i16_103a; i += 3) {
				g_toolbox->MoveTo(this->arr_i16_3738[i], this->arr_i16_3738[i+1]);
				this->var_str_384 = g_zbasic->chr(this->arr_i16_3738[i+2]);
				g_toolbox->DrawString(this->var_str_384);
			}
		}
		// 133:061a
		g_toolbox->FramePoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		// 133:0622
		this->arr_pic_49d8[j*4 + 1] = handle;
		g_toolbox->PenNormal();
		// 133:0642: JSR - "PICTURE_ON"
		handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_1eb8[0xf]]);
		g_toolbox->FramePoly(this->var_poly_1aac);
		g_toolbox->PaintPoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		this->arr_pic_49d8[j*4 + 2] = handle;
		g_toolbox->KillPoly(this->var_poly_1aac);
		// 133:0698
	}
	// 133:06b4
	g_toolbox->SetPort(this->var_i32_0);
	this->var_i16_484 = 0;
	for (int j = this->arr_i16_1eb8[0x16]; j <= this->arr_i16_1eb8[0x17]; j++) {
		for (int i = this->arr_i16_1eb8[0x18]; i <= this->arr_i16_1eb8[0x19]; i++) {
			this->var_i16_484++;
			this->arr_i16_4338[this->var_i16_484] = this->puzzlesReadByte();
		}
	}
	// 133:073c
	this->sub_128_bde(1, this->arr_i16_1eb8[0xf], 0, 1, 1, 1);
	this->var_i16_1ab0 = 0;
	for (this->var_i16_7cc = 1; this->var_i16_7cc <= this->arr_i16_1eb8[0x10]; this->var_i16_7cc++) {
		this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
		this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];
		this->sub_133_f04();
	}
	this->var_i16_1ab0 = 1;
	this->sub_133_12d4();
	this->sub_133_12f2();
	this->var_i16_7c6 = 0;
	do {
		do {
			this->sub_128_c6a(-1);
			if (this->var_ev_46.what == 1) {
				this->polyominoOnClick();
			}
			// yield to renderer
			if (this->var_ev_46.what == kNullEvent) {
				g_toolbox->Delay(0);
			}
			// 133:0800
		} while ((this->var_i16_7c6 == 0) && (this->var_i16_d0c == 0));
		// 133:081c
		if (this->var_i16_7c6 == 2) {
			this->sub_133_11cc();
		}
		if (this->var_i16_7c6 == 4) {
			this->polyominoStoreState();
			this->sub_128_3536();
		}
	} while (((this->var_i16_7c6 & 1) == 0) && (this->var_i16_d0c == 0));
	// 133:0864
	if (this->var_i16_d0c != 0) {
		this->sub_133_13e2();
	}
	this->polyominoStoreState();

	//133:0878: JMP - [0x1502]
	// exiting game, delete picture handles
	for (int j = 1; j <= this->arr_i16_1eb8[0x10]; j++) {
		for (int i = 0; i <= 2; i++) {
			g_toolbox->KillPicture(this->arr_pic_49d8[j*4 + i]);
		}
	}
}

void FoolGame::polyominoOnClick() {
	// 133:087c
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) || (this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
		this->sub_128_6186();
		return;
	}
	// 133:0904
	this->var_i16_7cc = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
	if (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] == 0) {
		this->sub_128_6186();
		return;
	}
	// 133:0950
	if (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] <= this->arr_i16_1eb8[0x11]) {
		this->sub_133_10a0();
		return;
	}
	// 133:0990
	this->var_i16_106a = 0;
	this->var_i16_1a96 = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
	this->var_i16_1a98 = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];
	for (int j = this->var_i16_1a98; j <= this->var_i16_1a98 + 3; j++) {
		// 133:09dc
		for (int i = this->var_i16_1a96; i <= this->var_i16_1a96 + 3; i++) {
			if (this->arr_i16_47d8[this->var_i16_7cc*8] & this->bitLUT[this->var_i16_106a]) {
				this->arr_i16_2f38[i*32 + j] = 0;
			}
			// 133:0a36
			this->var_i16_106a++;
		}
	}
	// 133:0a60
	this->var_i16_1ab2 = this->var_i16_1a96 - this->var_i16_68a;
	this->var_i16_1ab4 = this->var_i16_1a98 - this->var_i16_68c;
	this->var_i16_1062 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_1a96*32 + this->var_i16_1a98]].left;
	this->var_i16_1064 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_1a96*32 + this->var_i16_1a98]].top;
	g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4 + 2]);
	// 133:0b5a: JMP - [0xde0]
	while (this->var_ev_46.what != 2) {
		// 133:0d3a
		while ((this->var_i16_1ab6 == this->var_i16_1062) && (this->var_i16_1ab8 == this->var_i16_1064) && (this->var_ev_46.what != 2)) {
			// 133:0b62
			this->sub_128_c6a(4);
			this->var_i16_68a = (this->arr_i16_1eb8[6] / (this->var_ev_46.where.x - this->arr_i16_1eb8[4])) + this->var_i16_1ab2;
			this->var_i16_68c = (this->arr_i16_1eb8[7] / (this->var_ev_46.where.y  - this->arr_i16_1eb8[5])) + this->var_i16_1ab4;
			if (this->var_i16_68a < 1) {
				this->var_i16_68a = 1;
			}
			// 133:0bf8
			if (this->var_i16_68a > (0x12 - this->arr_i16_47d8[this->var_i16_7cc*8 + 1])) {
				this->var_i16_68a = 0x12 - this->arr_i16_47d8[this->var_i16_7cc*8 + 1];
			}
			// 133:0c58
			if (this->var_i16_68c < 1) {
				this->var_i16_68c = 1;
			}
			// 133:0c68
			if (this->var_i16_68c > (0xb - this->arr_i16_47d8[this->var_i16_7cc*8 + 2])) {
				this->var_i16_68c = 0xb - this->arr_i16_47d8[this->var_i16_7cc*8 + 2];
			}
			// 133:0cc8
			this->var_i16_1ab6 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].left;
			this->var_i16_1ab8 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].top;

			// yield to renderer
			if (this->var_ev_46.what == kNullEvent) {
				g_toolbox->Delay(0);
			}
		}
		// 133:0d78
		if (this->var_ev_46.what != 2) {
			g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
			this->var_i16_1062 = this->var_i16_1ab6;
			this->var_i16_1064 = this->var_i16_1ab8;
			g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
		}
		// 133:0de0

	}
	// 133:0dea
	this->var_i16_106a = 0;
	this->var_i16_1a9c = 0;
	for (int j = this->var_i16_68c; j <= this->var_i16_68c + 3; j++) {
		for (int i = this->var_i16_68a; i <= this->var_i16_68a + 3; i++) {
			if (this->arr_i16_2f38[i*32 + j] > 0) {
				this->var_i16_1a9c |= this->bitLUT[this->var_i16_106a];
			}
			// 133:0e44
			this->var_i16_106a++;
		}
	// 133:0e5a
	}
	// 133:0e6c
	if ((this->var_i16_1a9c & this->arr_i16_47d8[this->var_i16_7cc*8]) != 0) {
		this->sub_133_eb2();
	} else {
		this->sub_133_f04();
	}
	// 133:0ea2
	this->sub_133_12d4();
	this->sub_133_12f2();
	this->sub_128_4da(1);
}

void FoolGame::sub_133_eb2() {
	// 133:0eb2
	this->sub_128_50e(0x14, 0x64, 0);
	this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
	this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];

	sub_133_f04();
}

void FoolGame::sub_133_f04() {
	// 133:0f04
	this->var_i16_106a = 0;
	for (int j = this->var_i16_68c; j <= this->var_i16_68c + 3; j++) {
		for (int i = this->var_i16_68a; i <= this->var_i16_68a + 3; i++) {
			this->var_i16_1a96 = this->var_i16_68a;
			if ((this->arr_i16_47d8[this->var_i16_7cc*8 + 1] & this->bitLUT[this->var_i16_106a]) != 0) {
				this->arr_i16_2f38[i*32 + j] = this->var_i16_7cc;
			}
			// 133:0f6c
			this->var_i16_106a++;
		}
	}
	// 133:0f96
	this->arr_i16_47d8[this->var_i16_7cc*8 + 3] = this->var_i16_68a;
	this->arr_i16_47d8[this->var_i16_7cc*8 + 4] = this->var_i16_68c;

	// 133:0fce
	if (this->var_i16_1ab0 != 0) {
		g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
	}
	// 133:1002
	this->var_i16_1062 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].left;
	this->var_i16_1064 = this->arr_rect_1f38[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].top;
	g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4 + 1]);

}

void FoolGame::sub_133_10a0() {
	// 133:10a0
	this->var_i16_484 = this->arr_rect_1f38[this->arr_i16_3b38[this->arr_i16_47d8[this->var_i16_7cc*8 + 3]*32 + this->arr_i16_47d8[this->var_i16_7cc*8 + 4]]].left;
	this->var_i16_7e4 = this->arr_rect_1f38[this->arr_i16_3b38[this->arr_i16_47d8[this->var_i16_7cc*8 + 3]*32 + this->arr_i16_47d8[this->var_i16_7cc*8 + 4]]].top;
	this->sub_128_50e(0x14, 0x64, 0);
	// why do this???
	// 133:1188
	for (int i = 0; i <= 9; i++) {
		g_zbasic->picture(this->var_i16_484, this->var_i16_7e4, this->arr_pic_49d8[this->var_i16_7cc*4]);
	}
	// 133:11c2
	this->sub_133_12d4();
	this->sub_128_6186();
}

void FoolGame::sub_133_11cc() {
	// 133:11cc
	for (int j = 1; j <= this->arr_i16_1eb8[1] + 1; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0] + 1; i++) {
			this->arr_i16_2f38[i*32 + j] = 0;
		}
	}
	// 133:122e
	this->fillRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, this->arr_i16_1eb8[0xf]);
	this->var_i16_1ab0 = 0;
	for (this->var_i16_7cc = 1; this->var_i16_7cc <= this->arr_i16_1eb8[0x10]; this->var_i16_7cc++) {
		this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 5];
		this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 6];
		this->sub_133_f04();
	}
	// 133:12c2
	this->var_i16_1ab0 = 1;
	this->sub_133_12d4();
	this->var_i16_7c6 = 0;
}

void FoolGame::sub_133_12d4() {
	// 133:12d4
	g_toolbox->PenNormal();
	g_toolbox->PenSize(3, 3);
	Common::Rect temp;
	temp.top = this->arr_i16_1eb8[12];
	temp.left = this->arr_i16_1eb8[13];
	temp.bottom = this->arr_i16_1eb8[14];
	temp.right = this->arr_i16_1eb8[15];
	g_toolbox->FrameRect(temp);
	g_toolbox->PenNormal();
}

void FoolGame::sub_133_12f2() {
	// 133:12f2
	this->var_i16_d0c = 1;
	this->var_i16_484 = 0;
	for (int j = this->arr_i16_1eb8[0x16]; j <= this->arr_i16_1eb8[0x17]; j++) {
		for (int i = this->arr_i16_1eb8[0x18]; i <= this->arr_i16_1eb8[0x19]; i++) {
			this->var_i16_484++;
			if (this->arr_i16_2f38[i*32 + j] != this->arr_i16_4338[this->var_i16_484]) {
				// 133:1368
				this->var_i16_d0c = 0;
				j = this->arr_i16_1eb8[0x17];
				i = this->arr_i16_1eb8[0x19];
			}
			// 133:1396
		}
	}
	// 133:13ce
	if (this->var_i16_d0c != 0) {
		this->var_i16_7c6 = 1;
	}
}

void FoolGame::sub_133_13e2() {
	// 133:13e2
	if (this->var_i16_c04 < 0x64) {
		this->var_i16_c04 = 0x64;
	}
	this->sub_128_2664();
	this->sub_128_6186();
	this->sub_128_d34(this->arr_i16_1eb8[0x12], this->arr_i16_1eb8[0x13], this->arr_i16_1eb8[0x14], this->arr_i16_1eb8[0x15], 0x96);
}

void FoolGame::polyominoStoreState() {
	// 133:1452
	this->var_str_c06 = g_zbasic->str(212); // empty
	this->var_i16_68a = 1;
	for (int i = 1; i <= this->arr_i16_1eb8[0x10]; i++) {
		this->var_str_384 = g_zbasic->unk_88(this->arr_i16_47d8[i*8 + 3]);
		this->var_str_9f4 = g_zbasic->unk_88(this->arr_i16_47d8[i*8 + 4]);
		this->var_str_c06 = this->var_str_c06 + this->var_str_384 + this->var_str_9f4;
	}
}



};
