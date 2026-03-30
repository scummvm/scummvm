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

// word search game
void FoolGame::wordSearchRun() {
	// 131:0004
	this->sub_128_271a();
	this->var_str_188e = this->puzzlesReadString();
	for (int i = 0; i <= 0xe; i++) {
		this->arr_i16_1eb8[i] = this->puzzlesReadShort();
	}
	// 131:0042
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
				this->var_i16_68a + this->arr_i16_1eb8[0xd] - 1,
				this->var_i16_68c + this->arr_i16_1eb8[0xc] - 1
			);
		// 131:00d4
		} while (g_zbasic->incrAndCheck(this->var_i16_68a, this->arr_i16_1eb8[0xb], this->arr_i16_1eb8[6]));
	} while (g_zbasic->incrAndCheck(this->var_i16_68c, this->arr_i16_1eb8[9], this->arr_i16_1eb8[7]));
	// 131:0134
	this->var_i16_484 = 0;

	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->arr_i16_2f38[i*32 + j] = 0;
			this->arr_i16_3b38[i*32 + j] = 0;
		}
	}

	// 131:01b6
	// total hidden word count
	this->var_i16_198e = this->puzzlesReadShort();
	if (this->var_str_c06 == g_zbasic->str(191)) { // blank str
		this->var_str_c06 = g_zbasic->space(this->var_i16_198e);
	} else {
		// 131:01f2
		this->arr_i16_4758[0] = g_zbasic->unk_310(g_zbasic->midStr(this->var_str_c06, this->var_i16_198e + 1, 2));
		for (int i = 1; i <= this->arr_i16_4758[0]; i++) {
			this->arr_i16_4758[i] = g_zbasic->unk_310(g_zbasic->midStr(
				this->var_str_c06,
				this->var_i16_198e + 1 + i*2,
				2
			));
		}
		// 131:0282
		this->var_i16_484 = 0;
		this->var_i16_106a = 1;
		this->var_i16_68c = 1;

		for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
			for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
				// 131:029a
				if ((this->arr_i16_4758[this->var_i16_106a] & this->bitLUT[this->var_i16_484]) != 0) {
					this->arr_i16_2f38[i*32 + j] = 1;
				}
				// 131:02e8
				this->var_i16_484++;
				if (this->var_i16_484 == 0x10) {
					this->var_i16_484 = 0;
					this->var_i16_106a++;
				}
				// 131:0300
			}
		}
		// 131:0336

	}
	// 131:0336
	this->var_i16_1990 = 0;
	this->var_i16_68a = 1;
	for (int i = 1; i <= this->var_i16_198e; i++) {
		this->arr_i16_3738[i] =  (int16)strtol(g_zbasic->midStr(this->var_str_c06, i, 1).encode(Common::kMacRoman).c_str(), nullptr, 10);
		if (this->arr_i16_3738[i] != 0) {
			this->var_i16_1990++;
		}
		// 131:038e
	}
	// 131:039e
	this->var_i16_1992 = 1;
	do {
		g_zbasic->indexSet(this->puzzlesReadString(), 1, this->var_i16_1992 + this->var_i16_198e);
		g_zbasic->indexSet(g_zbasic->str(192), 1, this->var_i16_1992); // blank string
		this->var_i16_484 = 1;
		do {
			this->var_str_384 = g_zbasic->midStr(
				g_zbasic->index(1, this->var_i16_1992 + this->var_i16_198e),
				this->var_i16_484,
				1
			);
			// 131:0418
			if (this->var_str_384 != g_zbasic->str(193)) { // string
				g_zbasic->indexSet(g_zbasic->index(1, this->var_i16_1992) + this->var_str_384, 1, this->var_i16_1992);
			}
			// 131:045c
			this->var_i16_484++;
		} while ((int16)(g_zbasic->index(1, this->var_i16_1992 + this->var_i16_198e).size()) >= this->var_i16_484);

		// 131:0480
		this->var_i16_1992++;
	} while (this->var_i16_198e >= this->var_i16_1992);

	// 131:0492
	g_toolbox->PenNormal();
	this->var_str_384 = this->var_str_188e;
	this->wordSearchDrawFooter();
	g_zbasic->text(0xfb, this->arr_i16_1eb8[0xe], 0, kSrcOr);
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	g_toolbox->PenMode(kPatBic);
	g_zbasic->unk_20();
	// 131:04e0
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		this->var_str_1994 = this->puzzlesReadString();
		this->var_i16_1574 = 1 + (j - 1) * this->arr_i16_1eb8[0];
		// 131:051e
		this->arr_rect_5b7c.top = this->arr_rect_1f38[this->var_i16_1574].top - 1;
		this->arr_rect_5b7c.left = 0;
		this->arr_rect_5b7c.bottom = this->arr_rect_1f38[this->var_i16_1574].bottom + 1;
		this->arr_rect_5b7c.right = SCREEN_WIDTH;
		g_toolbox->FillRect(this->arr_rect_5b7c, this->arr_pat_58f4[0]);
		// 131:05b0
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->var_i16_1574 = i + (j - 1) * this->arr_i16_1eb8[0];
			this->var_str_384 = g_zbasic->midStr(this->var_str_1994, i, 1);
			this->arr_i16_3b38[i*32 + j] = g_zbasic->asc(this->var_str_384);

			g_toolbox->MoveTo(
				this->arr_rect_1f38[this->var_i16_1574].left + this->arr_i16_1eb8[2] - 1,
				this->arr_rect_1f38[this->var_i16_1574].top + this->arr_i16_1eb8[3]
			);
			g_toolbox->DrawString(this->var_str_384);
			// 131:069e
			if (this->arr_i16_2f38[i*32 + j] != 0) {
				g_toolbox->PaintRect(this->arr_rect_1f38[this->var_i16_1574]);
			}
			// 131:06dc
		}
		// 131:06f8
	}
	// 131:0714
	g_toolbox->PenNormal();
	if ((this->var_i16_1990 > 0) && (this->var_i16_1990 < this->var_i16_198e)) {
		// only X to go
		this->var_str_384 = g_zbasic->str(194) + Common::U32String::format(" %d", this->var_i16_198e - this->var_i16_1990) + g_zbasic->str(195);
		this->wordSearchDrawFooter();
	}
	// 131:077a
	this->var_str_9f4 = Common::U32String::format(" %d", this->var_i16_198e);
	if (this->var_i16_7e2 != 0x50) {
		// There are X words hidden below
		this->var_str_384 = g_zbasic->str(196) + this->var_str_9f4 + g_zbasic->str(197);
		g_zbasic->menu(8, 3, 1, this->var_str_384);
		g_zbasic->menu(8, 4, 1, g_zbasic->str(198));
	} else {
	// 131:07f8
		this->var_str_384 = g_zbasic->str(199) + this->var_str_9f4 + g_zbasic->str(200);
		g_zbasic->menu(8, 3, 1, this->var_str_384);
		g_zbasic->menu(8, 4, 1, g_zbasic->str(201));
	}
	// 131:084c
	this->var_i16_7c6 = 0;
	while ((this->var_i16_7c6 & 1) == 0) {
		this->sub_128_c6a(-1);
		if (this->var_ev_46.what == 1) {
			this->wordSearchOnClick();
		}
		if (this->var_i16_1990 == this->var_i16_198e) {
			this->wordSearchSuccess();
		}
		if (this->var_i16_7c6 == 4) {
			this->wordSearchStoreState();
			this->sub_128_3536();
		}
		if (this->var_ev_46.what == kNullEvent) {
			g_toolbox->Delay(0);
		}
	}
	this->wordSearchStoreState();
	// 131:08a6

	// JMP 0x1116
	// 131:1116
}

void FoolGame::wordSearchOnClick() {
	// 131:08aa
	this->var_i16_1a94 = 0; // length of word selected
	this->var_str_1070 = g_zbasic->str(202); // blank
	this->var_i16_1a96 = -1;
	this->var_i16_1a98 = -1;

	while ((this->var_ev_46.modifiers & 0x80) == 0) {
		// 131:08d4
		this->sub_128_c6a(0);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (!((this->var_i16_68a == this->var_i16_1a96) && (this->var_i16_68c == this->var_i16_1a98))) {
			// 131:0934
			this->var_i16_1574 = this->var_i16_68a + (this->var_i16_68c - 1)*this->arr_i16_1eb8[0];
			this->var_i16_1a96 = this->var_i16_68a;
			this->var_i16_1a98 = this->var_i16_68c;
			this->var_str_1070 += g_zbasic->chr(this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]);
			g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_1574]);
			this->var_i16_1a94++;
			// 131:09c4
			this->arr_i16_4338[this->var_i16_1a94] = this->var_i16_1574;
			if (this->var_i16_1a94 == 0x64) {
				this->wordSearchBadSelect();
				return;
			}
		}
		// 131:09ec
		g_toolbox->Delay(0);
	}
	debugC(5, kDebugLoading, "wordSearchOnClick: pos (%d, %d)", this->var_i16_1a96, this->var_i16_1a98);
	// 131:09fc
	this->var_i16_1a9a = 0;
	this->var_i16_1a9c = this->arr_i16_4338[2] - this->arr_i16_4338[1];
	this->var_i16_68a = 1;
	for (int i = 1; i <= this->var_i16_1a94 - 1; i++) {
		if (this->arr_i16_4338[this->var_i16_68a + 1] - this->arr_i16_4338[this->var_i16_68a] != this->var_i16_1a9c) {
			this->var_i16_1a9a = 0x29a;
		}
	// 131:0a78
	}
	if (this->var_i16_1a9a == 0x29a) {
		this->wordSearchBadSelect();
		return;
	}
	this->var_i16_1a9a = 0;
	// 131:0aa0
	// check selected string against the remaining word list
	for (int i = 1; i <= this->var_i16_198e; i++) {
		if ((this->var_str_1070 == g_zbasic->index(1, i)) && (this->arr_i16_3738[i] == 0)) {
			// 131:0ae0
			this->var_i16_1a9a = i;
			this->arr_i16_3738[i] = 1;
		}
		// 131:0af8
	}
	if (this->var_i16_1a9a == 0) {
		this->wordSearchBadSelect();
		return;
	}
	// found a word, update the screen
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	g_toolbox->PenMode(kPatBic);
	// for every letter in the word
	for (int i = 1; i <= this->var_i16_1a94; i++) {
		// play a tone
		this->sub_128_50e(0xa + g_zbasic->rndInt(0x3e8), 0x28, 1);
		// 131:0b54
		// remove the inverted square
		g_toolbox->InvertRect(this->arr_rect_1f38[this->arr_i16_4338[i]]);
		// paint the square with white checkerboard
		g_toolbox->PaintRect(this->arr_rect_1f38[this->arr_i16_4338[i]]);
		this->var_i16_484 = ((this->arr_i16_4338[i] - 1) % this->arr_i16_1eb8[0]) + 1;
		// 131:0bce
		this->var_i16_7e4 = ((this->arr_i16_4338[i] - 1) / this->arr_i16_1eb8[0]) + 1;
		// add the square to the selected list
		this->arr_i16_2f38[this->var_i16_484*32 + this->var_i16_7e4] = 1;
	}
	// 131:0c32
	g_toolbox->PenNormal();
	this->var_i16_1990++;
	if (this->var_i16_1990 == this->var_i16_198e) {
		this->var_str_384 = g_zbasic->str(203); // blank
		this->wordSearchDrawFooter();
		return;
	}
	// 131:0c64
	this->var_str_384 = g_zbasic->index(1, this->var_i16_1a9a + this->var_i16_198e) + g_zbasic->str(204) + Common::U32String::format(" %d", this->var_i16_198e - this->var_i16_1990) + g_zbasic->str(205);	// WORD... only X to go
	this->wordSearchDrawFooter();
	this->sub_128_6186();
}

void FoolGame::wordSearchBadSelect() {
	// 131:0cbe
	// play failure noise
	this->sub_128_50e(0x14, 0x64, 0);
	// uninvert all highlighted boxes
	for (int i = 1; i <= this->var_i16_1a94; i++) {
		g_toolbox->InvertRect(this->arr_rect_1f38[this->arr_i16_4338[i]]);
	}
	// 131:0d0a
	this->var_i16_1a96 = -1;
	this->var_i16_1a98 = -1;
	this->var_i16_1a94 = 0;
	this->sub_128_6186();
}

void FoolGame::wordSearchDrawFooter() {
	// 131:0d22
	this->fillRect(0x13b, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	g_zbasic->text(0xfa, 0xc, 0, kSrcBic);
	this->var_i16_7a2 = 0x152;
	this->sub_128_918(this->var_str_384);
}

void FoolGame::wordSearchStoreState() {
	// 131:0d66
	this->var_str_c06 = g_zbasic->str(206); // empty
	for (int i = 1; i <= this->var_i16_198e; i++) {
		this->var_str_384 = Common::U32String::format("%d", this->arr_i16_3738[i]);
		this->var_str_c06 += g_zbasic->rightStr(this->var_str_384, 1);
	}
	// 131:0dd6
	this->var_i16_484 = 0;
	this->var_i16_106a = 1;
	this->arr_i16_4758[0] = 1;
	this->arr_i16_4758[1] = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			if (this->arr_i16_2f38[i*32 + j] != 0) {
				this->arr_i16_4758[this->var_i16_106a] = this->arr_i16_4758[this->var_i16_106a] | this->bitLUT[this->var_i16_484];
			}
			// 131:0e72
			this->var_i16_484++;
			if (this->var_i16_484 == 0x10) {
				this->var_i16_484 = 0;
				this->var_i16_106a++;
				this->arr_i16_4758[this->var_i16_106a] = 0;
				this->arr_i16_4758[0]++;
			}
			// 131:0ebe
		}
	}
	// 131:0ef6
	for (int i = 0; i <= this->arr_i16_4758[0]; i++) {
		this->var_str_c06 += g_zbasic->unk_88(this->arr_i16_4758[i]);
	}
}

void FoolGame::wordSearchSuccess() {
	// 131:0f42
	if (this->var_i16_c04 < 0x64) {
		this->var_i16_c04 = 0x64;
	}
	// 131:0f52
	this->sub_128_69c(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	g_zbasic->text(0xfb, this->arr_i16_1eb8[0xe], 0, kSrcOr);
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			if (this->arr_i16_2f38[i*32 + j] != 0) {
				// 131:0fc8
				this->var_i16_1574 = i + (j - 1)* this->arr_i16_1eb8[0];
				g_toolbox->EraseRect(this->arr_rect_1f38[this->var_i16_1574]);
				g_toolbox->MoveTo(
					this->arr_rect_1f38[this->var_i16_1574].left + this->arr_i16_1eb8[2] - 1,
					this->arr_rect_1f38[this->var_i16_1574].top + this->arr_i16_1eb8[3]
				);
				// 131:107a
				this->var_str_384 = g_zbasic->chr(this->arr_i16_3b38[i*32 + j]);
				g_toolbox->DrawString(this->var_str_384);
			}
			// 131:10b0
		}
	}
	// 131:10e8
	this->fillRect(0x1eb, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1);
	this->sub_128_2664();
	this->sub_128_61c2();
	this->var_i16_7c6 = 1;
}



};
