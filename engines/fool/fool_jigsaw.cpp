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

#include "common/ustr.h"
#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"
#include "graphics/managed_surface.h"

namespace Fool {

extern ZBasic *g_zbasic;
extern Toolbox *g_toolbox;

// jigsaw puzzle game
void FoolGame::sub_132_004() {
	// 132:0004
	this->var_i16_c00 = 1;
	this->sub_128_271a();
	this->var_i16_1066 = this->puzzlesReadShort();
	for (int i = 0; i <= 0xe; i++) {
		this->arr_i16_1eb8[i] = this->puzzlesReadShort();
	}
	// 132:0044
	// puzzle dimensions
	this->var_i16_1a9e = this->arr_i16_1eb8[0] * this->arr_i16_1eb8[1];
	for (int i = 1; i <= this->var_i16_1a9e; i++) {
		this->arr_i16_3738[i] = this->puzzlesReadByte();
	}
	// 132:00a2
	this->arr_i32_192c0[0] = g_toolbox->GetPicture(this->var_i16_1066);
	g_zbasic->picture(0, 0x14, this->arr_i32_192c0[0]);
	g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
	this->var_i16_1aa0 = 4 + ((this->arr_i16_1eb8[12] + 1)*2) * ((this->arr_i16_1eb8[13]+0x10)/0x10);
	// 132:0136
	this->var_i16_1aa0 = (this->var_i16_1aa0 / 2) + 1;
	this->var_i16_484 = 0;
	this->var_i16_68c = this->arr_i16_1eb8[8];
	do {
		this->var_i16_68a = this->arr_i16_1eb8[10];
		do {
			this->var_i16_484++;
			g_toolbox->SetRect(
				this->arr_rect_1f38[this->var_i16_484],
				this->var_i16_68a + 1,
				this->var_i16_68c + 1,
				this->var_i16_68a + this->arr_i16_1eb8[13],
				this->var_i16_68c + this->arr_i16_1eb8[12]
			);
			//this->var_i16_1aa2 = 0x2af8 + this->var_i16_1aa0*(this->arr_i16_3738[this->var_i16_484] - 1);
			// 132:0216
			this->arr_jigsaw_5dfc[this->var_i16_484] = BitMap(new Graphics::ManagedSurface());
			g_zbasic->get(
				this->arr_rect_1f38[this->var_i16_484].left,
				this->arr_rect_1f38[this->var_i16_484].top,
				this->arr_rect_1f38[this->var_i16_484].right,
				this->arr_rect_1f38[this->var_i16_484].bottom,
				this->arr_jigsaw_5dfc[this->var_i16_484]
			);
		} while (g_zbasic->incrAndCheck(
			this->var_i16_68a,
			this->arr_i16_1eb8[11],
			this->arr_i16_1eb8[6]
		));
	// 132:02dc
	} while (g_zbasic->incrAndCheck(
		this->var_i16_68c,
		this->arr_i16_1eb8[9],
		this->arr_i16_1eb8[7]
	));
	// 132:030c
	this->var_i16_484 = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = this->var_i16_484;
		}
	}
	// 132:0376
	// rearrange picture tiles to match state
	if (this->var_str_c06 != g_zbasic->str(207)) { // empty
		for (int i = 1; i <= this->var_i16_1a9e; i++) {
			this->arr_i16_3738[i] = g_zbasic->unk_310(g_zbasic->midStr(this->var_str_c06, i*2 - 1, 2));
			g_zbasic->put(
				this->arr_rect_1f38[i].left,
				this->arr_rect_1f38[i].top,
				this->arr_jigsaw_5dfc[i],
				kPutCopy
			);
		// 132:0452
		}
	}
	// 132:0464
	this->var_i16_7c6 = 0;
	g_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->InitCursor();
	this->sub_132_13ea();
	// JMP 0x4d8

	// 132:04d8
	// input loop
	while (((this->var_i16_7c6 & 1) == 0) && (this->var_i16_d0c == 0)) {
		// 132:0484
		// 132:049c
		while ((this->var_i16_7c6 == 0) && (this->var_i16_d0c == 0)) {
			// 132:0488
			this->sub_128_c6a(-1);
			if (this->var_ev_46.what == kMouseDown) {
				this->sub_132_518();
			}
			g_toolbox->Delay(0);
		}
		// 132:04b8
		if (this->var_i16_7c6 == 2) {
			this->sub_132_e5a();
		}
		if (this->var_i16_7c6 == 4) {
			this->sub_132_1384();
			this->sub_128_3536();
		}
	}
	// 132:0500
	if (this->var_i16_d0c != 0) {
		this->sub_132_1444();
	}
	this->sub_132_1384();
	// JMP 0x14a6
	// 132:14a6
}

void FoolGame::sub_132_518() {
	// 132:0518
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || ((this->var_i16_68a - (this->arr_i16_1eb8[0])) > 0) || (this->var_i16_68c < 1) || ((this->var_i16_68c - (this->arr_i16_1eb8[1])) > 0)) {
		return;
	}
	// 132:059c
	this->var_i16_e12 = 1;
	g_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->PenNormal();
	g_toolbox->PenSize(3, 3);
	g_toolbox->PenMode(kPatXor);
	// 132:05c4
	this->arr_i16_4758[0] = this->var_i16_68a;
	this->arr_i16_4758[1] = this->var_i16_68c;
	this->sub_132_67a();
	g_toolbox->FrameRect(this->arr_rect_476e);
	this->arr_i16_4758[0] -= this->var_i16_68a;
	this->arr_i16_4758[1] -= this->var_i16_68c;
	// 132:0642
	this->sub_132_962();
	g_toolbox->FrameRect(this->arr_rect_476e);
	this->sub_132_ed8();
	this->sub_132_13ea();
	this->sub_128_6186();
	g_zbasic->menu(8, 3, 1, Common::U32String());
	this->var_i16_e12 = 0;
}

void FoolGame::sub_132_67a() {
	do {
		this->sub_128_c6a(4);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->var_i16_68a >= this->arr_i16_4758[0]) {
			this->arr_i16_4758[2] = this->arr_i16_4758[0];
			this->arr_i16_4758[3] = this->var_i16_68a;
		} else {
			// 132:06fc
			this->arr_i16_4758[2] = this->var_i16_68a;
			this->arr_i16_4758[3] = this->arr_i16_4758[0];
		}
		// 132:072c
		if (this->var_i16_68c >= this->arr_i16_4758[1]) {
			this->arr_i16_4758[6] = this->arr_i16_4758[1];
			this->arr_i16_4758[7] = this->var_i16_68c;
		} else {
			this->arr_i16_4758[6] = this->var_i16_68c;
			this->arr_i16_4758[7] = this->arr_i16_4758[1];
		}
		// 132:07b0
		this->arr_rect_476e.top = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].top;
		// 132:080e
		this->arr_rect_476e.left = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].left;
		this->arr_rect_476e.bottom = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].bottom;
		this->arr_rect_476e.right = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].right;
		// 132:092e
		g_toolbox->FrameRect(this->arr_rect_476e);
		this->sub_128_3da(2);
		g_toolbox->FrameRect(this->arr_rect_476e);
		this->sub_128_3da(1);
	} while (this->var_ev_46.what != kMouseUp);
}

void FoolGame::sub_132_962() {
	do {
		this->sub_128_c6a(2);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->arr_i16_4758[0] >= 0) {
			this->arr_i16_4758[4] = this->var_i16_68a;
			this->arr_i16_4758[5] = this->var_i16_68a + this->arr_i16_4758[0];
		} else {
		// 132:09e2
			this->arr_i16_4758[4] = this->var_i16_68a + this->arr_i16_4758[0];
			this->arr_i16_4758[5] = this->var_i16_68a;
		}
		// 132:0a1e
		if (this->arr_i16_4758[1] >= 0) {
			this->arr_i16_4758[8] = this->var_i16_68c;
			this->arr_i16_4758[9] = this->var_i16_68c + this->arr_i16_4758[1];
		} else {
			// 132:0a70
			this->arr_i16_4758[8] = this->var_i16_68c + this->arr_i16_4758[1];
			this->arr_i16_4758[9] = this->var_i16_68c;
		}
		// 132:0aac
		if (this->arr_i16_4758[4] < 1) {
			this->arr_i16_4758[4] = 1;
			this->arr_i16_4758[5] = 1 + ABS(this->arr_i16_4758[0]);
		}
		// 132:0b10
		if (this->arr_i16_4758[8] < 1) {
			this->arr_i16_4758[8] = 1;
			this->arr_i16_4758[9] = 1 + ABS(this->arr_i16_4758[1]);
		}
		// 132:0b74
		if ((this->arr_i16_4758[5] - this->arr_i16_1eb8[0]) > 0) {
			this->arr_i16_4758[4] = this->arr_i16_1eb8[0] - ABS(this->arr_i16_4758[0]);
			this->arr_i16_4758[5] = this->arr_i16_1eb8[0];
		}
		// 132:0c02
		if ((this->arr_i16_4758[9] - this->arr_i16_1eb8[1]) > 0) {
			this->arr_i16_4758[8] = this->arr_i16_1eb8[1] - ABS(this->arr_i16_4758[1]);
			this->arr_i16_4758[9] = this->arr_i16_1eb8[1];
		}
		// 132:0c90
		this->arr_rect_4776.top = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].top;
		this->arr_rect_4776.left = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].left;
		// 132:0d4e
		this->arr_rect_4776.bottom = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].bottom;
		this->arr_rect_4776.right = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].right;
		// 132:0e0e
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->sub_128_3da(2);
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->sub_128_3da(1);
	}
	while ((this->var_ev_46.what == kMouseDown) && (this->var_ev_46.where.y > 0x14));
}

void FoolGame::sub_132_e5a() {
	// 132:0e5a
	this->var_i16_7c6 = 0;
	g_zbasic->swapInt(this->arr_i16_4758[2], this->arr_i16_4758[4]);
	g_zbasic->swapInt(this->arr_i16_4758[3], this->arr_i16_4758[5]);
	g_zbasic->swapInt(this->arr_i16_4758[6], this->arr_i16_4758[8]);
	g_zbasic->swapInt(this->arr_i16_4758[7], this->arr_i16_4758[9]);
	sub_132_ed8();
}

void FoolGame::sub_132_ed8() {
	// 132:0ed8
	this->var_i16_1aa4 = this->var_i16_1a9e;
	this->var_i16_68c = this->arr_i16_4758[6];
	for (int j = this->arr_i16_4758[6]; j <=  this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			this->var_i16_1aa4++;
			this->arr_i16_3738[this->var_i16_1aa4] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			// 132:0f54
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = 0;
		}
	}
	// 132:0fbc
	this->var_i16_1aa6 = this->var_i16_1aa4;
	this->var_i16_1aa4 = this->var_i16_1a9e;
	// 132:0fc8
	for (int j = this->arr_i16_4758[8]; j <= this->arr_i16_4758[9]; j++) {
		for (int i = this->arr_i16_4758[4]; i <= this->arr_i16_4758[5]; i++) {
			this->var_i16_1aa4++;
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] > 0) {
				this->var_i16_1aa6++;
				this->arr_i16_3738[this->var_i16_1aa6] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			}
			// 132:1076
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa4];
			// 132:10ba
			// 0x2af8, this->var_i16_1aa0 * (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] - 1)
			g_zbasic->put(
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].left,
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].top,
				this->arr_jigsaw_5dfc[this->arr_i16_2f38[i*32 + j]],
				kPutCopy
			);
		}
	}
	// 132:11c8
	this->var_i16_1aa6 = this->var_i16_1aa4;
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] == 0) {
				this->var_i16_1aa6++;
				this->arr_i16_3738[this->arr_i16_2f38[i*32+j]] = this->arr_i16_3738[this->var_i16_1aa6];
				g_zbasic->put(
					this->arr_rect_1f38[this->arr_i16_2f38[i*32+j]].left,
					this->arr_rect_1f38[this->arr_i16_2f38[i*32+j]].top,
					this->arr_jigsaw_5dfc[this->arr_i16_2f38[i*32 + j]],
					kPutCopy
				);
			}
			// 132:134a
		}
	}
}

void FoolGame::sub_132_1384() {
	// 132:1384
	this->var_str_c06 = g_zbasic->str(208);
	for (int i = 1; i <= this->var_i16_1a9e; i++) {
		this->var_str_384 = g_zbasic->unk_88(this->arr_i16_3738[i]);
		this->var_str_c06 += this->var_str_384;
	}
}


void FoolGame::sub_132_13ea() {
	// 132:13ea
	this->var_i16_d0c = 1;
	for (int i = 1; i <= this->var_i16_1a9e; i++) {
		if (this->arr_i16_3738[i] != i) {
			this->var_i16_d0c = 0;
			this->var_i16_68a = this->var_i16_1a9e;
		}
	// 132:1420
	}
	// 132:1430
	if (this->var_i16_d0c != 0) {
		this->var_i16_7c6 = 1;
	}
}

void FoolGame::sub_132_1444() {
	warning("STUB: %s", __func__);
}

};
