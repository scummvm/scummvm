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

// jigsaw puzzle game
void FoolGame::jigsawRun() {
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
			this->arr_jigsaw_5dfc[this->arr_i16_3738[this->var_i16_484]] = BitMap(new Graphics::ManagedSurface());
			g_zbasic->get(
				this->arr_rect_1f38[this->var_i16_484].left,
				this->arr_rect_1f38[this->var_i16_484].top,
				this->arr_rect_1f38[this->var_i16_484].right,
				this->arr_rect_1f38[this->var_i16_484].bottom,
				this->arr_jigsaw_5dfc[this->arr_i16_3738[this->var_i16_484]]
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
	if (!this->activePuzzleBuffer.empty()) { // was: str(207)
		for (int i = 1; i <= this->var_i16_1a9e; i++) {
			this->arr_i16_3738[i] = g_zbasic->unk_310(g_zbasic->midStr(this->activePuzzleBuffer, i*2 - 1, 2));
			g_zbasic->put(
				this->arr_rect_1f38[i].left,
				this->arr_rect_1f38[i].top,
				this->arr_jigsaw_5dfc[this->arr_i16_3738[i]],
				kPutCopy
			);
		// 132:0452
		}
	}
	// 132:0464
	this->stateFlags = kStateNull;
	g_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->InitCursor();
	this->jigsawCheckIfSolved();
	// JMP 0x4d8

	// 132:04d8
	// input loop
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 132:0484
		// 132:049c
		while ((this->stateFlags == kStateNull) && (this->var_i16_d0c == 0)) {
			// 132:0488
			this->getNextEvent(-1);
			if (this->var_ev_46.what == kMouseDown) {
				this->jigsawOnClick();
			}
		}
		// 132:04b8
		if (this->stateFlags == kStateUndo) {
			this->jigsawCancelSelect();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->jigsawStoreState();
			this->saveGame();
		}
	}
	// 132:0500
	if (this->var_i16_d0c != 0) {
		this->jigsawSuccess();
	}
	this->jigsawStoreState();
	// JMP 0x14a6
	// 132:14a6
}

void FoolGame::jigsawOnClick() {
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

	// change select area while mouse button held down
	this->jigsawDragSelect();
	// leave an XOR outline on the page for the selected block
	Common::Rect temp(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
	g_toolbox->FrameRect(temp);
	this->arr_i16_4758[0] -= this->var_i16_68a;
	this->arr_i16_4758[1] -= this->var_i16_68c;

	// 132:0642
	// move select area to new target
	this->jigsawMoveSelected();
	// remove XOR outline
	temp = Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
	g_toolbox->FrameRect(temp);

	this->jigsawDropSelected();
	this->jigsawCheckIfSolved();
	this->sub_128_6186();
	g_zbasic->menu(8, 3, 1, Common::U32String());
	this->var_i16_e12 = 0;
}

void FoolGame::jigsawDragSelect() {
	// 132:067a
	// change select area while mouse button held down
	do {
		this->getNextEvent(4);
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
		this->arr_i16_4758[11] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].top;
		// 132:080e
		this->arr_i16_4758[12] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].left;
		this->arr_i16_4758[13] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].bottom;
		this->arr_i16_4758[14] = this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].right;
		Common::Rect temp(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
		// 132:092e
		g_toolbox->FrameRect(temp);
		this->sub_128_3da(2);
		g_toolbox->FrameRect(temp);
		this->sub_128_3da(1);
	} while (this->var_ev_46.what != kMouseUp);
}

void FoolGame::jigsawMoveSelected() {
	// 132:0962
	// move select area to new target
	warning("call: %s", __func__);
	do {
		this->getNextEvent(2);
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
	while (!((this->var_ev_46.what == kMouseDown) && (this->var_ev_46.where.y > 0x14)));
}

void FoolGame::jigsawCancelSelect() {
	// 132:0e5a
	this->stateFlags = kStateNull;
	g_zbasic->swapInt(this->arr_i16_4758[2], this->arr_i16_4758[4]);
	g_zbasic->swapInt(this->arr_i16_4758[3], this->arr_i16_4758[5]);
	g_zbasic->swapInt(this->arr_i16_4758[6], this->arr_i16_4758[8]);
	g_zbasic->swapInt(this->arr_i16_4758[7], this->arr_i16_4758[9]);
	jigsawDropSelected();
}

void FoolGame::jigsawDropSelected() {
	// 132:0ed8
	// skip the tile pointer to past the end of the list
	this->var_i16_1aa4 = this->var_i16_1a9e;

	// arr_i16_4758[2]: src_left
	// arr_i16_4758[3]: src_right
	// arr_i16_4758[4]: dest_left
	// arr_i16_4758[5]: dest_right
	// arr_i16_4758[6]: src_top
	// arr_i16_4758[7]: src_bottom
	// arr_i16_4758[8]: dest_top
	// arr_i16_4758[9]: dest_bottom

	// iterate through src tile positions
	for (int j = this->arr_i16_4758[6]; j <=  this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			this->var_i16_1aa4++;
			// buffer 1: make a copy of source tile positions
			this->arr_i16_3738[this->var_i16_1aa4] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			debug(8, "(%d, %d) [src]: %d -> buffer 1", i, j, this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]);
			// 132:0f54
			// buffer 0: zero out source tiles
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = 0;
		}
	}
	// 132:0fbc
	// tile pointer to buffer 2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	this->var_i16_1aa4 = this->var_i16_1a9e;

	// 132:0fc8
	// iterate through dest tile positions
	for (int j = this->arr_i16_4758[8]; j <= this->arr_i16_4758[9]; j++) {
		for (int i = this->arr_i16_4758[4]; i <= this->arr_i16_4758[5]; i++) {
			this->var_i16_1aa4++;

			// if the dest tile isn't part of the src tile set, copy to buffer 2
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] > 0) {
				this->var_i16_1aa6++;
				debug(8, "(%d, %d) [dest]: %d -> buffer 2", i, j, this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]);
				this->arr_i16_3738[this->var_i16_1aa6] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			}
			// 132:1076
			// write src tile from buffer 1 to dest position
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa4];
			debug(8, "(%d, %d) [dest]: %d <- buffer 1", i, j, this->arr_i16_3738[this->var_i16_1aa4]);
			// 132:10ba
			// 0x2af8, this->var_i16_1aa0 * (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] - 1)
			// blit tiles to screen
			g_zbasic->put(
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].left,
				this->arr_rect_1f38[this->arr_i16_2f38[i*32 + j]].top,
				this->arr_jigsaw_5dfc[this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]],
				kPutCopy
			);
		}
	}
	// 132:11c8
	// tile pointer to buffer 2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	// iterate through source tile positions
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			// if the source tile has been zeroed out, fill it with tile from buffer 2
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] == 0) {
				this->var_i16_1aa6++;
				debug(8, "(%d, %d) [src]: %d <- buffer 2", i, j, this->arr_i16_3738[this->var_i16_1aa6]);
				this->arr_i16_3738[this->arr_i16_2f38[i*32+j]] = this->arr_i16_3738[this->var_i16_1aa6];
				g_zbasic->put(
					this->arr_rect_1f38[this->arr_i16_2f38[i*32+j]].left,
					this->arr_rect_1f38[this->arr_i16_2f38[i*32+j]].top,
					this->arr_jigsaw_5dfc[this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]],
					kPutCopy
				);
			}
			// 132:134a
		}
	}
}

void FoolGame::jigsawStoreState() {
	// convert jigsaw positions to string
	// 132:1384
	this->activePuzzleBuffer = g_zbasic->str(208);
	for (int i = 1; i <= this->var_i16_1a9e; i++) {
		this->var_str_384 = g_zbasic->unk_88(this->arr_i16_3738[i]);
		this->activePuzzleBuffer += this->var_str_384;
	}
}


void FoolGame::jigsawCheckIfSolved() {
	// check if puzzle is solved
	// 132:13ea
	this->var_i16_d0c = 1;
	for (int i = 1; i <= this->var_i16_1a9e; i++) {
		if (this->arr_i16_3738[i] != i) {
			this->var_i16_d0c = 0;
			i = this->var_i16_1a9e;
		}
	// 132:1420
	}
	// 132:1430
	if (this->var_i16_d0c != 0) {
		this->stateFlags = kStateReturn;
	}
}

void FoolGame::jigsawSuccess() {
	// strobe puzzle pieces once on victory
	if (this->var_i16_c04 < 0x64) {
		this->var_i16_c04 = 0x64;
		for (int j = 0; j <= 1; j++) {
			for (int i = 1; i <= this->var_i16_1a9e; i++) {
				g_toolbox->InvertRect(this->arr_rect_1f38[i]);
				this->sub_128_3da(1);
			}
		}
	}
	// 132:149c
	this->sub_128_2664();
	this->sub_128_61c2();
}

};
