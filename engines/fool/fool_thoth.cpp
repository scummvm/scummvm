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

// high priestess challenge
void FoolGame::sub_140_004() {
	// 140:0004
	if (this->var_i16_c04 < 4) {
		this->sub_140_3372();
	}
	if ((this->var_i16_7ce & 2) != 0) {
		g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
	}
	if (this->var_i16_c04 == 0) {
		this->var_i16_e14 = 1;
		this->sub_140_458();
		this->var_i16_e14 = 0;
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		// 140:0056
		this->var_i16_c04 = 1;
	}
	// 140:005c
	if (this->var_i16_c04 == 1) {
		this->sub_140_12fc();
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		this->var_i16_c04 = 2;
	}
	// 140:007e
	g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
	this->sub_140_2f92();
	if (this->var_i16_c04 == 2) {
		this->sub_140_1486();
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		this->var_i16_c04 = 3;
		if (this->puzzleCompletionStatus[0x17] == 0x63) {
			// 140:00ce
			this->puzzleCompletionStatus[0x17] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x17] + g_zbasic->str(301); // ' '
			g_zbasic->menu(0x4, 0x7, 0x1, this->var_str_384);
		}
	}
	// 140:0116
	if (this->var_i16_c04 == 3) {
		this->sub_140_18fa();
		if (this->puzzleCompletionStatus[0x3f] == 0x63) {
			this->puzzleCompletionStatus[0x3f] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x3f] + g_zbasic->str(302); // ' '
			g_zbasic->menu(6, 0xf, 1, this->var_str_384);
		}
	}
	// 140:019c
	if (this->var_i16_c04 == 4) {
		this->var_i16_2324 = 6;
		this->sub_140_206a();
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		this->arr_i16_1d24[0x51] |= 0x5;
		if (this->puzzleCompletionStatus[0x48] == 0x63) {
			this->puzzleCompletionStatus[0x48] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x48] + g_zbasic->str(303);
			g_zbasic->menu(7, 8, 1, this->var_str_384);
		}
	}
	// 140:024c
	if (this->var_i16_7d2 == 0) {
		g_toolbox->PenNormal();
		this->arr_i32_192c0[0] = g_toolbox->GetPicture(0x78);
		g_zbasic->picture(0, 0, this->arr_i32_192c0[0]);
		g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
		this->sub_128_61c2();
	}
	// 140:02a2
	if (this->var_i16_37a == 0) {
		g_toolbox->SetPort(this->var_i32_8);
	} else {
		this->sub_128_1ef8();
	}
	// 140:02b4
	this->var_i16_68a = 1;
	for (int16 i = 1; i <= 0x155; i++) {
		g_zbasic->get(1, i, SCREEN_WIDTH, i + 1, this->arr_bmp_fa3c);
		g_zbasic->put(g_zbasic->rndInt(0x14) - 0xa, i, this->arr_bmp_fa3c, kPutCopy);
	}
	// 140:0328
	if (this->var_i16_7d2 == 0) {
		g_toolbox->PenNormal();
		this->arr_i16_4758[0] = 0;
		this->arr_i16_4758[1] = 0;
		this->arr_i16_4758[2] = SCREEN_HEIGHT;
		this->arr_i16_4758[3] = SCREEN_WIDTH;

		for (int16 i = 0; i <= 0x22; i++) {
			this->var_i32_692 = g_toolbox->TickCount();
			g_toolbox->InvertRect(Common::Rect(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]));
			this->sub_128_406(1);
		}
		// 140:037e
		this->var_i16_484 = 0x96;

		for (int16 i = 0x37; i <= 0x122; i++) {
			if (i < 0x69) {
				this->var_i16_484 -= 3;
			}
			if (i > 0xf0) {
				this->var_i16_484 += 3;
			}
			g_toolbox->MoveTo(this->var_i16_484 + 0x5f - g_zbasic->rndInt(0x32), i);
			g_toolbox->LineTo(0x1a1 - this->var_i16_484 + g_zbasic->rndInt(0x32), i);
		}
		// 140:03f0
		g_zbasic->text(0xfe, 0x18, 0x10, kSrcBic);
		this->var_i16_7a2 = 0x96;
		this->sub_128_918(g_zbasic->str(304)); // you cannot claim
		this->var_i16_7a2 = 0xb9;
		this->sub_128_918(g_zbasic->str(305)); // the book of thoth
		this->var_i16_7a2 = 0xdc;
		this->sub_128_918(g_zbasic->str(306)); // so easily
		g_toolbox->SetPort(this->var_i32_0);
		this->sub_128_61c2();
	}
	// 140:0450
	this->sub_140_32ac();
	this->sub_140_3412();
	return;
}

void FoolGame::sub_140_458() {
	// 140:0458
	warning("STUB: %s", __func__);
}

void FoolGame::sub_140_12fc() {
	// 140:12fc
	this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 2);
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
	if ((this->var_i16_7ce & 2) != 0) {
		g_toolbox->SetCursor(this->arr_curs_4d88[0x22]);
	}
	// 140:1346
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenMode(kPatXor);
	this->var_ev_46.where = Common::Point(0, 0);
	// 140:139a
	while (!((this->var_ev_46.where.x >= 0xff) && (this->var_ev_46.where.y >= 0xa2) && (this->var_ev_46.where.x <= 0x10e) && (this->var_ev_46.where.y <= 0xac))) {
		// 140:1366
		this->getNextEvent(-1); // was: 0
		g_toolbox->MoveTo(0x105, 0xa2);
		g_toolbox->LineTo(this->var_ev_46.where.x, this->var_ev_46.where.y);
		this->sub_128_3da(2);
		g_toolbox->MoveTo(0x105, 0xa2);
		g_toolbox->LineTo(this->var_ev_46.where.x, this->var_ev_46.where.y);
	}
	// 140:13dc
	g_toolbox->PenNormal();
	if ((this->var_i16_7ce & 2) == 0) {
		this->stateFlags = kStateReturn;
		this->sub_128_962(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xa2, 0xff, 0xac, 0x10e, 1, kPatXor, 0x19);
		this->sub_128_3da(0x3c);
		this->var_i16_d0c = 0;
	} else {
		// 140:1446
		this->var_str_384 = g_zbasic->str(322); // behold ye the book of thoth
		this->sub_128_178a(0, 0);
		this->sub_140_2f92();
		this->var_i16_7ce ^= 2;
		this->var_i16_7b2 = 0xa;
		this->var_i16_d0c = 1;
	}
	// 140:1484
}

void FoolGame::sub_140_1486() {
	// 140:1486
	warning("STUB: %s", __func__);
}

void FoolGame::sub_140_18fa() {
	// 140:18fa
	if (this->activePuzzle != 0x34) {
		this->sub_140_3372();
	}
	// STUB: rig the data pointer
	// 140:1908: MOVE.L - var_i32_232a,D0
	// 140:190c: ADDI.L - 0x000001c4,D0
	// 140:1912: MOVE.L - D0,var_i32_696
	for (int16 i = 0; i <= 3; i++) {
		for (int16 j = 0; j <= 3; j++) {
			this->arr_i16_2f38[i*32 + j] = puzzlesReadShort();
		}
	}
	// 140:195e
	for (int16 i = 0; i <= 3; i++) {
		this->arr_rect_1f38[i].top = 0x14;
		this->arr_rect_1f38[i].left = 0;
		this->arr_rect_1f38[i].bottom = SCREEN_HEIGHT;
		this->arr_rect_1f38[i].right = SCREEN_WIDTH;
	}
	// 140:19dc
	this->arr_rect_1f38[0].right = 0xf;
	this->arr_rect_1f38[1].left = 0x1f1;
	this->arr_rect_1f38[2].bottom = 0x23;
	this->arr_rect_1f38[3].top = 0x147;
	this->arr_i16_4338[1] = 0;
	this->arr_i16_4338[9] = 2;
	for (int16 i = 0x14; i <= 0x1a; i++) {
		this->arr_i16_4338[i - 0x12] = i;
		this->arr_i16_4338[0x1a - i + 0xa] = i;
	}
	// 140:1a9c
	this->var_i16_103a = 0;
	this->var_i16_232e = 0;
	this->stateFlags = kStateNull;
	this->var_i16_d0c = 0;
	// 140:1b98
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 140:1ab8
		if (this->activePuzzle == 0x34) {
			this->var_i16_232e++;
			if (this->var_i16_232e < 1) {
				this->var_i16_232e = 1;
			}
			if (this->var_i16_232e > 3) {
				this->var_i16_232e = 1;
			}
			switch (this->var_i16_232e-1) {
			case 0:
				this->sub_140_325a();
				break;
			case 1:
				this->sub_140_326e();
				break;
			case 2:
				this->sub_140_3282();
				break;
			default:
				warning("sub_140_18fa: broke out of switch");
				break;
			}
		} else {
			// 140:1b08
			this->var_i16_232e++;
			if (this->var_i16_232e < 1) {
				this->var_i16_232e = 1;
			}
			if (this->var_i16_232e > 0x10) {
				this->var_i16_232e = 1;
			}
			g_toolbox->FillRect(this->arr_rect_1f38[this->var_i16_103a], this->arr_pat_58f4[this->arr_i16_4338[this->var_i16_232e]]);
		}
		// 140:1b66
		this->getNextEvent(-1);
		if (this->var_ev_46.what == 1) {
			this->sub_140_1bc4();
		}
		// 140:1b7a
		if (this->var_i16_103a == 4) {
			this->var_i16_d0c = 1;
		}
		if (this->stateFlags == kStateSaveGame) {
			this->saveGame();
		}
	}
	// 140:1bc2
}

void FoolGame::sub_140_1bc4() {
	// 140:1bc4
	this->var_i16_2332 = 1;
	if (this->activePuzzle == 0x34) {
		if ((this->var_ev_46.where.x >= 0xf) && (this->var_ev_46.where.y >= 0x2d) && (this->var_ev_46.where.x <= 0x1f1) && (this->var_ev_46.where.x <= 0x147)) {
			this->var_i16_2332 = 0;
		}
	} else {
		// 140:1c22

	}
	// 140:1c58
	warning("STUB: %s", __func__);
}

void FoolGame::sub_140_206a() {
	// 140:206a
	this->var_i16_d0c = 0;
	this->stateFlags = kStateNull;
	this->sub_140_2998();
	this->sub_140_2978();
	this->copyScreen(1, this->arr_bmp_b3ec);
	this->arr_i16_1eb8[1] = -1;
	this->var_i16_2324 = 0;
	this->var_i16_1de6 = this->arr_i16_3b38[this->var_i16_2324];
	// JMP 0x228c
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 140:20c2
		do {
			if (this->activePuzzle == 0x34) {
				this->var_i16_2334 = this->arr_i16_3b38[this->var_i16_2324];
				this->var_i16_2336 = this->arr_i16_3b38[this->var_i16_2324 + 0x20];
				do {
					this->getNextEvent(-1);
					g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_3738[this->var_i16_2334]], this->arr_pat_58f4[this->var_i16_2338]);
					this->var_i16_2334++;
					if (this->var_i16_2324 != 5) {
						g_toolbox->Delay(4);
					} else {
						g_toolbox->Delay(2);
					}
				// 140:2168
				} while ((this->var_ev_46.what == 1) || (this->var_i16_2334 > this->var_i16_2336) || ((this->stateFlags & kStateReturn) != 0));
				// 140:219e
				if ((this->var_ev_46.what == 0) && ((this->stateFlags & kStateReturn) == 0)) {
					if (this->var_i16_2324 != 5) {
						g_toolbox->Delay(0x7d);
					} else {
						g_toolbox->Delay(0xfa);
					}
				}
				// 140:21e8
				this->copyScreen(1, this->arr_bmp_b3ec);
			}
			// 140:21fe
			this->var_i16_484 = g_zbasic->rndInt(0x4e) + 2;
			this->var_i16_68a = g_zbasic->rndInt(0x14) - 1;
			this->var_i16_68c = g_zbasic->rndInt(0xe) - 1;
			// 140:2222
			g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]], this->arr_pat_58f4[this->var_i16_484]);
			this->getNextEvent(-1);
			if (this->var_ev_46.what == 1) {
				this->sub_140_22b8();
			}
			if (this->stateFlags == kStateSaveGame) {
				this->saveGame();
			}
			// 140:228c
		} while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0));
	}
}

void FoolGame::sub_140_22b8() {
	// 140:22b8
	while (((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) && (this->var_i16_1de6 != (this->arr_i16_3b38[0x20 + this->var_i16_2324] + 1))) {
		// 140:22bc
		this->getNextEvent(-1); // was: 0
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		if ((this->var_i16_68a >= 0) && (this->var_i16_68c >= 0) && (this->var_i16_68a <= 0x13) && (this->var_i16_68c <= 0xd)) {
			// 140:2316
			this->arr_i16_1eb8[0] = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
			if (this->arr_i16_1eb8[0] != this->arr_i16_1eb8[1]) {
				// 140:2372
				if (this->arr_i16_1eb8[0] != this->arr_i16_3738[this->var_i16_1de6]) {
					this->sub_140_24ae();
					return;
				}
				// 140:23aa
				this->arr_i16_1eb8[1] = this->arr_i16_1eb8[0];
				this->sub_128_50e(0x32 + (this->var_i16_1de6 * 5), 0x28, 0);
				g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_1eb8[0]], this->arr_pat_58f4[2]);
				this->var_i16_1de6++;
			}
		}
		// 140:2422
	}
	// 140:2472
	if (this->var_i16_1de6 != (this->arr_i16_3b38[0x20 + this->var_i16_2324] + 1)) {
		this->sub_140_24ae();
	} else {
		this->sub_140_25e4();
	}
}

void FoolGame::sub_140_24ae() {
	// 140:24ae
	this->sub_128_50e(0x14, 0x64, 0x0);
	while ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) {
		// 140:24c4
		this->getNextEvent(-1);
		this->var_i16_484 = g_zbasic->rndInt(0x50);
		g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_1eb8[0]], this->arr_pat_58f4[this->var_i16_484]);
	}
	// 140:2520
	if (this->activePuzzle != 0x34) {
		this->copyScreen(1, this->arr_bmp_b3ec);
	} else {
		for (int16 i = this->var_i16_1de6; i >= this->arr_i16_3b38[this->var_i16_2324]; i--) {
			this->var_i16_484 = g_zbasic->rndInt(0x4e) + 2;
			g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_3738[i]], this->arr_pat_58f4[this->var_i16_484]);
		}
	}
	// 140:25b4
	this->arr_i16_1eb8[1] = -1;
	this->var_i16_1de6 = this->arr_i16_3b38[this->var_i16_2324];
}

void FoolGame::sub_140_25e4() {
	// 140:25e4
	if (this->activePuzzle == 0x34) {
		this->var_i16_d0c = 1;
		return;
	}
	// 140:25f6
	if (this->var_i16_2324 < 5) {
		this->sub_140_2662();
	}
	if (this->var_i16_2324 == 4) {
		this->sub_140_26ca();
	}
	if (this->var_i16_2324 == 5) {
		this->sub_140_28bc();
	}
	if (this->var_i16_2324 == 6) {
		this->sub_140_2968();
	}
	if (this->var_i16_d0c == 0) {
		this->var_i16_2324++;
		if (this->var_i16_2324 == 6) {
			this->sub_140_2998();
		}
		this->sub_140_2978();
		this->copyScreen(1, this->arr_bmp_b3ec);
	}
	// 140:2660
}

void FoolGame::sub_140_2662() {
	// 140:2662
	this->arr_i16_4758[0] = 0x29;
	this->arr_i16_4758[1] = 0x37;
	this->arr_i16_4758[2] = 0x142;
	this->arr_i16_4758[3] = 0x1c9;
	Common::Rect temp(0x37, 0x29, 0x1c9, 0x142);
	for (int16 i = 0; i <= 0x10; i++) {
		g_toolbox->InvertRect(temp);
		this->sub_128_3da(1);
	}
}

void FoolGame::sub_140_26ca() {
	// 140:26ca
	this->sub_128_4da(0);
	g_toolbox->PenSize(0xa, 0x7);

	for (int16 j = 0; j <= 2; j++) {
		// 140:26e0
		this->arr_i16_4758[1] = 0x2d;
		this->arr_i16_4758[0] = 0x22;
		this->arr_i16_4758[3] = 0x1d3;
		this->arr_i16_4758[2] = 0x149;
		for (int16 i = 0; i <= 0x14; i++) {
			this->arr_i16_4758[0] += 7;
			this->arr_i16_4758[1] += 0xa;
			this->arr_i16_4758[2] -= 7;
			this->arr_i16_4758[3] -= 0xa;
			Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
			g_toolbox->InvertRect(temp);
			this->sub_128_3da(1);
		}
		for (int16 i = 0; i <= 0x13; i++) {
			this->arr_i16_4758[0] -= 7;
			this->arr_i16_4758[1] -= 0xa;
			this->arr_i16_4758[2] += 7;
			this->arr_i16_4758[3] += 0xa;
			Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
			g_toolbox->InvertRect(temp);
			this->sub_128_3da(1);
		}
	}
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	this->sub_128_4da(1);
}

void FoolGame::sub_140_28bc() {
	// 140:28bc
	if (this->puzzleCompletionStatus[0x68] > 4) {
		this->sub_128_6186();
		this->sub_128_2664();
		this->sub_128_d34(0x29, 0x37, 0x142, 0x1c9, 0x96);
		this->var_i16_d0c = 1;
	} else {
		// 140:290c
		this->sub_128_962(0x14, 0xc8, 0x14, 0x138, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x21);
		this->var_str_384 = g_zbasic->str(323); // behold the last key of thoth
		this->sub_128_178a(0x48, 2);
	}
	// 140:2966
}

void FoolGame::sub_140_2968() {
	// 140:2968
	this->sub_128_2664();
	this->sub_128_61c2();
	this->var_i16_d0c = 1;
}

void FoolGame::sub_140_2978() {
	// 140:2978
	this->sub_128_6186();
	if (this->var_i16_2324 < 6) {
		this->var_i16_2338 = 1;
	} else {
		this->var_i16_2338 = 2;
	}
}

void FoolGame::sub_140_2998() {
	// 140:2998
	if (this->var_i16_2324 != 6) {
		this->sub_128_271a();
		this->arr_i16_1eb8[4] = 0x37;
		this->arr_i16_1eb8[5] = 0x29;
		this->arr_i16_1eb8[6] = 0x14;
		this->arr_i16_1eb8[7] = 0x14;
		this->arr_i16_1eb8[8] = 0x29;
		this->arr_i16_1eb8[9] = 0x12d;
		this->arr_i16_1eb8[10] = 0x37;
		this->arr_i16_1eb8[11] = 0x1bd;
		this->arr_i16_1eb8[12] = 0x15;
		this->arr_i16_1eb8[13] = 0x15;
		for (int16 i = 0; i <= 0x13; i++) {
			this->arr_i16_5bbc[i] = this->puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0xd; i++) {
			this->arr_i16_5cbc[i] = this->puzzlesReadShort();
		}
		// 140:2aa4
		for (int16 i = 0; i <= 0xcd; i++) {
			this->arr_i16_3738[i] = this->puzzlesReadShort();
			this->arr_i16_3738[i] = (this->arr_i16_3738[i]/4) + 1;
		}
		// 140:2b06
		for (int16 i = 1; i <= 6; i++) {
			this->arr_i16_3b38[i] = this->puzzlesReadShort();
			this->arr_i16_3b38[0x20 + i] = this->puzzlesReadShort();
		}
		// 140:2b54

	} else {
		// 140:2b58
		if (this->activePuzzle != 0x34) {
			this->sub_140_3372();
		}
		// FIXME: some kind of manual bodge for the puzzle data loader,
		// this looks lethal
		// 140:2b66: MOVE.L - var_i32_232a,D0
		// 140:2b6a: ADDI.L - 0x000001e4,D0
		// 140:2b70: MOVE.L - D0,var_i32_696
		this->var_bytes_696 = this->var_bytes_232a;
		this->var_ptr_696 = this->var_ptr_232a + 0x1e4;
		for (int16 i = 0; i <= 0xe; i++) {
			this->arr_i16_1eb8[i] = this->puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0x13; i++) {
			this->arr_i16_5bbc[i] = this->puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0xd; i++) {
			this->arr_i16_5cbc[i] = this->puzzlesReadShort();
		}
		// 140:2bfe
		for (int16 i = 0x9c; i <= 0xcd; i++) {
			this->arr_i16_3738[i] = this->puzzlesReadShort();
		}
		this->arr_i16_3b38[6] = 0x9c;
		this->arr_i16_3b38[6+0x20] = 0xcd;
	}
	// 140:2c58
	this->var_i16_484 = 0;
	this->var_i16_68c = this->arr_i16_1eb8[8];
	do {
		this->var_i16_68a = this->arr_i16_1eb8[0xa];
		do {
			this->var_i16_484++;
			g_toolbox->SetRect(
				this->arr_rect_1f38[this->var_i16_484],
				this->var_i16_68a,
				this->var_i16_68c,
				this->var_i16_68a + this->arr_i16_1eb8[0xd],
				this->var_i16_68c + this->arr_i16_1eb8[0xc]
			);
			// 140:2ce4
		} while (g_zbasic->incrAndCheck(this->var_i16_68a, this->arr_i16_1eb8[0xb], this->arr_i16_1eb8[6]));
	} while (g_zbasic->incrAndCheck(this->var_i16_68c, this->arr_i16_1eb8[9], this->arr_i16_1eb8[7]));
	// 140:2d46

	this->var_i16_484 = 0;
	for (int16 j = 0; j <= 0xd; j++) {
		for (int16 i = 0; i <= 0x13; i++) {
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = this->var_i16_484;
		}
	}
	// 140:2d94
	if (this->var_i16_2324 != 6) {
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		for (int16 j = 0; j <= 0x13; j++) {
			for (int16 i = 0; i <= 0xd; i++) {
				g_toolbox->EraseRect(this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_5bbc[j]*32 + this->arr_i16_5cbc[i]]]);
				g_toolbox->FrameRect(this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_5bbc[j]*32 + this->arr_i16_5cbc[i]]]);
			}
		}
	} else {
		// 140:2e7a
		for (int16 k = 0; k <= 0x13; k++) {
			this->var_i16_68a = k;
			for (int16 j = 0; j <= 0xd; j++) {
				// 140:2e8c
				this->var_i16_484 = g_zbasic->rndInt(0x4e) + 2;
				g_toolbox->FillRect(
					this->arr_rect_1f38[this->arr_i16_2f38[this->arr_i16_5bbc[this->var_i16_68a]*32 + this->arr_i16_5cbc[j]]],
					this->arr_pat_58f4[this->var_i16_484]
				);
				this->var_i16_68a++;
				if (this->var_i16_68a > 0x13) {
					this->var_i16_68a = 0;
				}
				// 140:2f12
			}
		}
	}
	// 140:2f2e
	if ((this->var_i16_2324 == 6) && (this->activePuzzle != 0x34)) {
		g_zbasic->menu(8, 0, 1, g_zbasic->str(324)); // the last key of thoth
		g_zbasic->menu(8, 1, 1, g_zbasic->str(325)); // return to scroll
	}
	this->copyScreen(0, this->arr_bmp_b3ec);
}

void FoolGame::sub_140_2f92() {
	// 140:2f92
	g_zbasic->menu(8, 0, 1, g_zbasic->str(326)); // the book of thoth
	g_zbasic->menu(8, 1, 1, g_zbasic->str(327)); // return to scroll
	g_zbasic->menu(8, 2, 0, g_zbasic->str(328)); // -
	g_zbasic->menu(8, 3, 1, g_zbasic->str(329)); // be forewarned
	g_zbasic->menu(8, 4, 1, g_zbasic->str(330)); // the high priestess has corrupted
	g_zbasic->menu(8, 5, 1, g_zbasic->str(331)); // the book of thoth. you may need
	g_zbasic->menu(8, 6, 1, g_zbasic->str(332)); // the help of others to unlock its
	g_zbasic->menu(8, 7, 1, g_zbasic->str(333)); // secrets.
}

void FoolGame::sub_140_3050() {
	// 140:3050
	this->arr_i16_4758[0] = 0x14 + g_zbasic->rndInt(0xde);
	this->arr_i16_4758[1] = g_zbasic->rndInt(0x19c);
	this->arr_i16_4758[2] = this->arr_i16_4758[0] + 0x64;
	this->arr_i16_4758[3] = this->arr_i16_4758[1] + 0x64;
}

void FoolGame::sub_140_30da() {
	// 140:30da
	this->arr_i16_4758[0] = 0x14 + g_zbasic->rndInt(0xde);
	this->arr_i16_4758[1] = 0;
	this->arr_i16_4758[2] = this->arr_i16_4758[0] + g_zbasic->rndInt(0x64);
	this->arr_i16_4758[3] = SCREEN_WIDTH;
}

void FoolGame::sub_140_3148() {
	// 140:3148
	this->arr_i16_4758[0] = 0x14;
	this->arr_i16_4758[1] = g_zbasic->rndInt(0x19c);
	this->arr_i16_4758[2] = SCREEN_HEIGHT;
	this->arr_i16_4758[3] = this->arr_i16_4758[1] + g_zbasic->rndInt(0x64);
}

void FoolGame::sub_140_325a() {
	// 140:325a
	this->sub_140_3050();
	Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
	g_toolbox->InvertRect(temp);
}

void FoolGame::sub_140_326e() {
	// 140:326e
	this->sub_140_30da();
	Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
	g_toolbox->InvertRect(temp);
}

void FoolGame::sub_140_3282() {
	// 140:3282
	this->sub_140_3148();
	Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
	g_toolbox->InvertRect(temp);
}

void FoolGame::sub_140_32ac() {
	// 140:33ac
	this->sub_128_4da(0);
	g_toolbox->PenNormal();
	for (int16 j = 0; j <= 0x22; j++) {
		for (int16 i = -5; i <= 0x159; i += 0xa) {
			g_toolbox->MoveTo(0, g_zbasic->rndInt(0xa) + i);
			g_toolbox->LineTo(SCREEN_WIDTH, g_zbasic->rndInt(0xa) + i);
		}
	}
	// 140:3300
	this->arr_i16_4758[0] = 0;
	this->arr_i16_4758[1] = 0;
	this->arr_i16_4758[2] = SCREEN_HEIGHT;
	this->arr_i16_4758[3] = SCREEN_WIDTH;
	Common::Rect temp(this->arr_i16_4758[1], this->arr_i16_4758[0], this->arr_i16_4758[3], this->arr_i16_4758[2]);
	for (int16 i = 1; i <= 0xd; i++) {
		this->var_i32_692 = g_toolbox->TickCount();
		g_toolbox->InvertRect(temp);
		this->sub_128_406(1);
	}
	// 140:334a
	g_toolbox->FillRect(temp, this->arr_pat_58f4[2]);
	this->sub_128_4da(1);
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_140_3372() {
	this->var_i16_484 = (this->puzzleDataOffsets[0x34] - 1) / 1000;
	this->var_i16_7e4 = (this->puzzleDataOffsets[0x34] - 1) % 1000;
	g_zbasic->record(1, this->var_i16_484, this->var_i16_7e4);
	this->var_bytes_696 = this->arr_bytes_109dc;
	this->var_bytes_232a = this->var_bytes_696;
	this->var_ptr_232a = this->var_ptr_696;
	this->var_bytes_696 = g_zbasic->readFile(1, this->puzzleDataOffsets[0x35] - this->puzzleDataOffsets[0x34]);
	this->var_ptr_696 = 0;
}

void FoolGame::sub_140_3412() {
	// 140:3412
	g_toolbox->PenNormal();
	if ((this->var_i16_7ce & 2) == 0) {
		g_toolbox->InitCursor();
	}
}

}
