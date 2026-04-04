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

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

extern ZBasic *g_zbasic;
extern Toolbox *g_toolbox;

// The Sun's map
void FoolGame::sunMapRun() {
	// 137:0004
	this->var_i16_c00 = 1;
	this->sub_128_69c(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	this->arr_i16_1eb8[0] = 9;
	this->arr_i16_1eb8[1] = 9;
	this->arr_i16_1eb8[4] = 0x4b;
	this->arr_i16_1eb8[5] = -9;
	this->arr_i16_1eb8[6] = 0x23;
	this->arr_i16_1eb8[7] = 0x23;
	this->var_i16_484 = 0;
	for (int16 j = 0x1a; j <= 0x132; j += 0x23) {
		for (int16 i = 0x6e; i <= 0x186; i += 0x23) {
			this->var_i16_484++;
			g_toolbox->SetRect(this->arr_rect_1f38[this->var_i16_484], i, j, i+0x24, j+0x24);
		}
	}
	// 137:00fa
	this->var_i16_484 = 0;
	for (int16 j = 1; j <= 9; j++) {
		for (int16 i = 1; i <= 9; i++) {
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = this->var_i16_484;
		}
	}
	// 137:0148
	for (int i = 1; i <= 0x51; i++) {
		this->var_i16_484 = this->arr_i16_4c7c[i];
		if ((this->arr_i16_1d24[this->var_i16_484] & 3) != 0) {
			this->arr_i32_192c0[i] = this->arr_i32_1912c[i];
		} else {
			// 137:01b2
			this->arr_i32_192c0[i] = this->arr_i32_1912c[0x52];
		}
		// 137:01d8
		if ((this->arr_i16_1d24[this->var_i16_484] & 4) != 0) {
			this->arr_i16_1d24[this->var_i16_484] ^= 4;
			this->arr_i16_1d24[i] |= 8;
		}
		// 137:0252
		if ((this->arr_i16_1d24[i] & 8) != 0) {
			this->arr_i32_192c0[i] = this->arr_i32_1912c[0x53];
		}
		// 137:0294
	}
	// 137:02a2
	this->fillRect(0x14, 0x67, 0x156, 0x1b1, 2);
	for (int i = 1; i <= 0x51; i++) {
		g_zbasic->picture(this->arr_rect_1f38[i].left, this->arr_rect_1f38[i].top, this->arr_i32_192c0[this->arr_i16_4bd8[i]]);
	}
	// 137:0336
	this->var_i16_7c6 = 0;
	g_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->InitCursor();
	this->sunMapCheckIfSolved();
	while (((this->var_i16_7c6 & 1) == 0) && (this->var_i16_d0c == 0)) {
		while ((this->var_i16_7c6 == 0) && (this->var_i16_d0c == 0)) {
			this->sub_128_c6a(-1);
			if (this->var_ev_46.what == kMouseDown) {
				this->sunMapOnClick();
			}
			if (this->var_ev_46.what == kNullEvent)
				g_toolbox->Delay(0);
		}
		// 137:038a
		if (this->var_i16_7c6 == 2) {
			this->sunMapUndoMove();
		}
		if (this->var_i16_7c6 == 4) {
			this->sub_128_3536();
		}
	}
	// 137:03ce
	if (this->var_i16_d0c == 0) {
		return;
	}
	// JMP 0x1618
	this->var_i16_68c = 0;
	for (int j = 0; j <= 5; j++) {
		for (int i = 1; i <= 0x51; i++) {
			g_toolbox->InvertRect(this->arr_rect_1f38[i]);
		}
	}
	// 137:1654
	this->var_i16_7e2 = 0;
	this->var_i16_c04 = 0x64;
	this->var_i16_7d2 = 1;
	g_zbasic->menu(2, 7, 1, Common::U32String());
	for (int i = 1; i <= 0x50; i++) {
		this->arr_i16_1d24[i] = 1;
	}
	for (int j = 3; j <= 7; j++) {
		for (int i = 1; i <= 0x10; i++) {
			g_zbasic->menu(j, i, 1, Common::U32String());
		}
	}
}

void FoolGame::sunMapOnClick() {
	// 137:03e2
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) ||
		(this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
		return;
	}
	// 137:0466
	this->var_i16_e12 = 1;
	g_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->PenNormal();
	g_toolbox->PenSize(3, 3);
	g_toolbox->PenMode(kPatXor);
	this->arr_i16_4758[0] = this->var_i16_68a;
	this->arr_i16_4758[1] = this->var_i16_68c;
	if (this->arr_i16_1d24[this->arr_i16_4bd8[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] & 8) {
		this->sunMapRevealPiece();
		return;
	}
	this->sunMapDragSelect();
	// 137:0506
	g_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
	this->arr_i16_4758[0] -= this->var_i16_68a;
	this->arr_i16_4758[1] -= this->var_i16_68c;
	this->sunMapMoveSelected();
	g_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
	this->sunMapDropSelected();
	this->sunMapCheckIfSolved();
	this->sub_128_6186();
	g_zbasic->menu(8, 3, 1, Common::U32String());
	this->var_i16_e12 = 0;
}

void FoolGame::sunMapDragSelect() {
	// 137:0598
	do {
		this->sub_128_c6a(4);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->var_i16_68a >= this->arr_i16_4758[0]) {
			this->arr_i16_4758[2] = this->arr_i16_4758[0];
			this->arr_i16_4758[3] = this->var_i16_68a;
		} else {
		// 137:061a
			this->arr_i16_4758[2] = this->var_i16_68a;
			this->arr_i16_4758[3] = this->arr_i16_4758[0];
		}
		// 137:064a
		if (this->var_i16_68c >= this->arr_i16_4758[1]) {
			this->arr_i16_4758[6] = this->arr_i16_4758[1];
			this->arr_i16_4758[7] = this->var_i16_68c;
		} else {
			// 137:069e
			this->arr_i16_4758[6] = this->var_i16_68c;
			this->arr_i16_4758[7] = this->arr_i16_4758[1];
		}
		// 137:06ce
		this->arr_i16_4758[11] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].top;
		this->arr_i16_4758[12] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].left;
		this->arr_i16_4758[13] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].bottom;
		this->arr_i16_4758[14] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].right;
		// 137:084c
		g_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
		this->sub_128_3da(2);
		g_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
		this->sub_128_3da(1);
	} while (this->var_ev_46.what != kMouseUp);
	// 137:087e
}

void FoolGame::sunMapMoveSelected() {
	// 137:0880
	do {
		this->sub_128_c6a(2);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->arr_i16_4758[0] >= 0) {
			this->arr_i16_4758[4] = this->var_i16_68a;
			this->arr_i16_4758[5] = this->var_i16_68a + this->arr_i16_4758[0];
		} else {
			// 137:0900
			this->arr_i16_4758[4] = this->var_i16_68a + this->arr_i16_4758[0];
			this->arr_i16_4758[5] = this->var_i16_68a;
		}
		// 137:093c
		if (this->arr_i16_4758[1] >= 0) {
			this->arr_i16_4758[8] = this->var_i16_68c;
			this->arr_i16_4758[9] = this->var_i16_68c + this->arr_i16_4758[1];
		} else {
			// 137:098e
			this->arr_i16_4758[8] = this->var_i16_68c + this->arr_i16_4758[1];
			this->arr_i16_4758[9] = this->var_i16_68c;
		}
		// 137:09ca
		if (this->arr_i16_4758[4] < 1) {
			this->arr_i16_4758[4] = 1;
			this->arr_i16_4758[5] = 1 + ABS(this->arr_i16_4758[0]);
		}
		// 137:0a2e
		if (this->arr_i16_4758[8] < 1) {
			this->arr_i16_4758[8] = 1;
			this->arr_i16_4758[9] = 1 + ABS(this->arr_i16_4758[1]);
		}
		// 137:0a92
		if (this->arr_i16_4758[5] > this->arr_i16_1eb8[0]) {
			this->arr_i16_4758[4] = this->arr_i16_1eb8[0] - ABS(this->arr_i16_4758[0]);
			this->arr_i16_4758[5] = this->arr_i16_1eb8[0];
		}
		// 137:0b20
		if (this->arr_i16_4758[9] > this->arr_i16_1eb8[1]) {
			this->arr_i16_4758[8] = this->arr_i16_1eb8[1] - ABS(this->arr_i16_4758[1]);
			this->arr_i16_4758[9] = this->arr_i16_1eb8[1];
		}
		// 137:0bae
		this->arr_rect_4776.top = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].top;
		this->arr_rect_4776.left = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].left;
		this->arr_rect_4776.bottom = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].bottom;
		this->arr_rect_4776.right = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].right;
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->sub_128_3da(2);
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->sub_128_3da(1);
	} while (this->var_ev_46.what != kMouseDown);
}

void FoolGame::sunMapUndoMove() {
	// 137:0d60
	this->var_i16_7c6 = 0;
	g_zbasic->swapInt(this->arr_i16_4758[2], this->arr_i16_4758[4]);
	g_zbasic->swapInt(this->arr_i16_4758[3], this->arr_i16_4758[5]);
	g_zbasic->swapInt(this->arr_i16_4758[6], this->arr_i16_4758[8]);
	g_zbasic->swapInt(this->arr_i16_4758[7], this->arr_i16_4758[9]);
	this->sunMapDropSelected();
}

void FoolGame::sunMapDropSelected() {
	// 137:0dde
	this->var_i16_1aa4 = 0x51;
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			this->var_i16_1aa4++;
			this->arr_i16_3738[this->var_i16_1aa4] = this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]];
			// 137:0e5a
			this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]] = 0;
		}
	}

	// 137:0ec2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	this->var_i16_1aa4 = 0x51;
	for (int j = this->arr_i16_4758[8]; j <= this->arr_i16_4758[9]; j++) {
		for (int i = this->arr_i16_4758[4]; i <= this->arr_i16_4758[5]; i++) {
			this->var_i16_1aa4++;
			if (this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]] > 0) {
				this->var_i16_1aa6++;
				this->arr_i16_3738[this->var_i16_1aa6] = this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]];
			}
			// 137:0f7c
			this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa4];
			g_zbasic->picture(
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].left,
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].top,
				this->arr_i32_192c0[this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]]]
			);
		}
	}
	this->var_i16_1aa6 = this->var_i16_1aa4;
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			if (this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]] == 0) {
				// 137:1114
				this->var_i16_1aa6++;
				this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa6];
				g_zbasic->picture(
					this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].left,
					this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].top,
					this->arr_i32_192c0[this->arr_i16_4bd8[this->arr_i16_2f38[i*32 + j]]]
				);
			}
			// 137:1214
		}
	}

	// 137:124c
}

void FoolGame::sunMapCheckIfSolved() {
	// 137:124e
	this->var_i16_d0c = 1;
	for (int i = 1; i <= 0x51; i++) {
		if (this->arr_i16_4bd8[i] != i) {
			this->var_i16_d0c = 0;
			i = 0x51;
		}
	}
	// 137:128e
	if (this->var_i16_d0c == 1) {
		for (int i = 1; i <= 0x51; i++) {
			this->var_i16_484 = this->arr_i16_4c7c[i];
			if ((this->arr_i16_1d24[this->var_i16_484] & 8) || (this->arr_i16_1d24[this->var_i16_484] == 0)) {
				this->var_i16_d0c = 0;
				i = 0x51;
			}
			// 137:12fa
		}
	}
	// 137:1306
	if (this->var_i16_d0c != 0) {
		this->var_i16_7c6 = 1;
	}
}

void FoolGame::sunMapRevealPiece() {
	// 137:131a
	this->arr_i16_1d24[this->arr_i16_4bd8[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] ^= 8;
	this->arr_i32_192c0[this->arr_i16_4bd8[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] = this->arr_i32_1912c[this->arr_i16_4bd8[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]];
	// 137:142c
	// unrolled loop
	this->arr_i16_4758[0] = this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].top;
	this->arr_i16_4758[1] = this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].left;
	this->arr_i16_4758[2] = this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].bottom;
	this->arr_i16_4758[3] = this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].right;
	// 137:1490
	for (int16 i = 0; i <= 0xf; i++) {
		this->arr_i16_4758[0] += i;
		this->arr_i16_4758[1] += i;
		this->arr_i16_4758[2] -= i;
		this->arr_i16_4758[3] -= i;
		Common::Rect temp;
		temp.top = this->arr_i16_4758[0];
		temp.left = this->arr_i16_4758[1];
		temp.bottom = this->arr_i16_4758[2];
		temp.right = this->arr_i16_4758[3];
		g_toolbox->FrameRect(temp);
		this->sub_128_3da(1);
	}
	// 137:1550
	g_zbasic->picture(
		this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].left,
		this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].top,
		this->arr_i32_192c0[this->arr_i16_4bd8[this->arr_i16_2f38[this->var_i16_68a*32+this->var_i16_68c]]]
	);
	this->sub_128_6186();
	this->sunMapCheckIfSolved();
	this->var_i16_e12 = 0;
}


}
