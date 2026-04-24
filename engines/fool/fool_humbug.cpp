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

// the humbug - "irritating stick" style challenge
void FoolGame::humbugRun() {
	// 142:0004
	if (this->var_i16_c04 < 0x63) {
		this->humbugTrail();
		if (this->var_i16_d0c == 0) {
			return;
		}
		this->sub_142_5f2();
		if (this->puzzleCompletionStatus[0x34] > 2) {
			this->var_i16_c04 = 0x64;
			return;
		}
		// 142:004e
		this->var_i16_c04 = 0x63;
		this->sub_128_962(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
		this->var_str_384 = g_zbasic->str(336); // behold the 2nd key of thoth
		this->sub_128_178a(0x17, 0);
	}
	// 142:00b2
	if (this->var_i16_c04 == 0x63) {
		this->sub_142_630();
	}
	if (this->var_i16_c04 >= 0x64) {
		this->humbugTrail();
		if (this->var_i16_d0c != 0) {
			this->sub_142_5f2();
		}
	}
	// 142:00e2
	return;
}

void FoolGame::humbugTrail() {
	// 142:00e6
	if (this->puzzleCompletionStatus[0x34] < 3) {
		g_zbasic->menu(8, 3, 1, g_zbasic->str(337)); // A secret is undone
		g_zbasic->menu(8, 4, 1, g_zbasic->str(338)); // if the two become one.
	} else {
		// 142:0136
		g_zbasic->menu(8, 3, 1, g_zbasic->str(339)); // A puzzle is undone
		g_zbasic->menu(8, 4, 1, g_zbasic->str(340)); // if the two become one.
	}
	// 142:0166
	this->sub_128_271a();
	this->sub_128_4da(0);
	g_zbasic->text(0, 0xc, 0, kSrcOr);
	this->sub_128_55c(g_zbasic->str(341)); // ~
	// eye button
	g_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	for (int16 i = 1; i <= 0xa42; i++) {
		this->arr_i16_9894[i] = this->puzzlesReadShort();
		this->arr_i16_ac1c[i] = this->puzzlesReadShort();
		g_zbasic->put(
			this->arr_i16_9894[i],
			this->arr_i16_ac1c[i],
			this->arr_bmp_b3ec,
			kPutCopy
		);
		// Simulate slow draw speed
		if ((i % 42) == 0)
			g_toolbox->Delay(0);
	}
	// 142:026e
	this->sub_128_4da(1);
	g_zbasic->put(
		this->arr_i16_9894[this->var_i16_233c],
		this->arr_i16_ac1c[this->var_i16_233c],
		this->arr_bmp_b3ec,
		kPutCopy
	);
	this->var_i16_233c = 1;
	this->stateFlags = kStateNull;
	this->var_i16_d0c = 0;
	this->var_i16_1ab6 = 0;
	this->var_i16_1ab8 = 0;

	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 142:02ee
		this->getNextEvent(-1);
		if (!((this->var_i16_1ab6 == this->var_ev_46.where.x) && (this->var_i16_1ab8 == this->var_ev_46.where.y))) {
			this->sub_142_370();
		}
		if (this->var_i16_233c >= 0xa42) {
			this->var_i16_d0c = 1;
		}
		if (this->stateFlags == kStateSaveGame) {
			this->saveGame();
		}
		// 142:0346
	}
}

void FoolGame::sub_142_370() {
	// 142:0370
	this->var_i16_7e4 = this->var_i16_233c + 0x19;
	if (this->var_i16_7e4 > 0xa42) {
		this->var_i16_7e4 = 0xa42;
	}
	this->var_i16_9f2 = this->var_i16_233c - 0x19;
	if (this->var_i16_9f2 < 1) {
		this->var_i16_9f2 = 1;
	}
	this->var_i16_1a9c = 0;
	for (int16 i = this->var_i16_7e4; i >= this->var_i16_9f2; i--) {
		this->var_i16_1a96 = this->var_ev_46.where.x - this->arr_i16_9894[i];
		this->var_i16_1a98 = this->var_ev_46.where.y - this->arr_i16_ac1c[i];
		if ((this->var_i16_1a96 >= -5) && (this->var_i16_1a96 <= 0x1e) && (this->var_i16_1a98 >= -5) && (this->var_i16_1a98 <= 0x19)) {
			this->var_i16_1a9c = i + 0xa;
		}
		// 142:0470
	}
	// 142:0484
	if (this->var_i16_1a9c >= 0xa42) {
		this->var_i16_1a9c = 0xa42;
	}
	if (this->var_i16_1a9c != 0) {
		if (this->var_i16_233c < this->var_i16_1a9c) {
			this->var_i16_484 = 1;
		} else {
			this->var_i16_484 = -1;
		}
		this->var_i16_7a8 = this->var_i16_233c;
		do {
			g_zbasic->put(
				this->arr_i16_9894[this->var_i16_7a8],
				this->arr_i16_ac1c[this->var_i16_7a8],
				this->arr_bmp_b3ec,
				kPutCopy
			);
			// 142:051c
		} while (g_zbasic->incrAndCheck(this->var_i16_7a8, this->var_i16_1a9c, this->var_i16_484));
		this->var_i16_233c = this->var_i16_1a9c;
		this->var_i16_1ab6 = this->var_ev_46.where.x;
		this->var_i16_1ab8 = this->var_ev_46.where.y;
	} else {
		// 142:054a
		this->var_i16_7e4 = this->var_i16_233c - 0xa;
		if (this->var_i16_7e4 < 1) {
			this->var_i16_7e4 = 1;
		}
		for (int16 i = this->var_i16_233c; i >= this->var_i16_7e4; i--) {
			g_zbasic->put(
				this->arr_i16_9894[i],
				this->arr_i16_ac1c[i],
				this->arr_bmp_b3ec,
				kPutCopy
			);
			// 142:05c8
		}
		this->var_i16_233c -= 0xa;
		if (this->var_i16_233c < 1) {
			this->var_i16_233c = 1;
		}
	}
	// 142:05f0
}

void FoolGame::sub_142_5f2() {
	// 142:05f2
	this->sub_128_962(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x19);
}

void FoolGame::sub_142_630() {
	// 142:0630
	g_zbasic->menu(8, 0, 1, g_zbasic->str(342)); // the 2nd key of thoth
	g_zbasic->menu(8, 1, 1, g_zbasic->str(343)); // return to scroll
	this->thothKey2nd();
	if (this->var_i16_d0c == 1) {
		// 142:066e
		g_toolbox->PenNormal();
		g_zbasic->text(0xfe, 0x18, 0x19, kSrcBic);
		for (int16 i = 1; i <= 4; i++) {
			Common::Rect temp;
			temp.top = this->arr_i16_2f38[i*32];
			temp.left = this->arr_i16_2f38[i*32+1];
			temp.bottom = this->arr_i16_2f38[i*32+2];
			temp.right = this->arr_i16_2f38[i*32+3];
			g_toolbox->FillRect(temp, this->arr_pat_58f4[2]);
			g_toolbox->FrameRect(temp);
			this->var_str_384 = Common::U32String::format(" %d ", i);
			this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_384);
			this->var_i16_9f2 = (this->arr_i16_2f38[i*32+3] - this->arr_i16_2f38[i*32+1])/2;
			// 142:0756
			g_toolbox->MoveTo(
				this->arr_i16_2f38[i*32+1] + this->var_i16_9f2 - (this->var_i16_7e4 / 2),
				this->arr_i16_2f38[i*32 + 2] - 0x1e
			);
			g_toolbox->DrawString(this->var_str_384);
		}
		// 142:07ca
		g_zbasic->text(0, 0xc, 0, kSrcOr);
		this->sub_128_6186();
		this->sub_128_2664();
		this->var_i16_484 = 0;
		this->var_i16_7e4 = 0;
		this->var_i16_9f2 = 0;
		while (this->var_ev_46.modifiers & kModMouseButtonUp) {
			// 142:07f8
			this->getNextEvent(-1); // was: 0
			this->var_i16_7e4++;
			if (this->var_i16_7e4 == 0x64) {
				this->var_i16_7e4 = 0;
				this->var_i16_9f2++;
				if (this->var_i16_9f2 > 4) {
					this->var_i16_9f2 = 1;
				}
				Common::Rect temp;
				temp.top = this->arr_i16_2f38[this->var_i16_9f2*32];
				temp.left = this->arr_i16_2f38[this->var_i16_9f2*32+1];
				temp.bottom = this->arr_i16_2f38[this->var_i16_9f2*32+2];
				temp.right = this->arr_i16_2f38[this->var_i16_9f2*32+3];
				g_toolbox->InvertRect(temp);
			}
		}
	}
	// 142:0850
}

}
