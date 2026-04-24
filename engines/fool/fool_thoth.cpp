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
void FoolGame::highPriestessRun() {
	// 140:0004
	if (this->var_i16_c04 < 0x64) {
		this->thothAdjustPuzzleData();
	}
	if ((this->var_i16_7ce & 2) != 0) {
		g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
	}
	if (this->var_i16_c04 == 0) {
		this->var_i16_e14 = 1;
		this->thoth99Enchantments();
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
		this->thothKey1st();
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
		this->thothKey2nd();
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		this->var_i16_c04 = 3;
		if (this->puzzleCompletionStatus[0x17] == 0x63) {
			// is the humbug marked with the wadjet eye? if so, unmark
			// 140:00ce
			this->puzzleCompletionStatus[0x17] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x17] + g_zbasic->str(301); // ' '
			g_zbasic->menu(0x4, 0x7, 0x1, this->var_str_384);
		}
	}
	// 140:0116
	if (this->var_i16_c04 == 3) {
		this->thothKey3rd();
		if (this->var_i16_d0c == 0) {
			return;
		}
		this->var_i16_c04 = 4;
		if (this->puzzleCompletionStatus[0x3f] == 0x63) {
			// is justice marked with the wadjet eye? if so, unmark
			this->puzzleCompletionStatus[0x3f] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x3f] + g_zbasic->str(302); // ' '
			g_zbasic->menu(6, 0xf, 1, this->var_str_384);
		}
	}
	// 140:019c
	if (this->var_i16_c04 == 4) {
		this->hermitPathStage = 6;
		this->thothKeyLast();
		if (this->var_i16_d0c == 0) {
			this->sub_140_3412();
			return;
		}
		this->arr_i16_1d24[0x51] |= 0x5;
		if (this->puzzleCompletionStatus[0x48] == 0x63) {
			// is the hermit marked with the wadjet eye? if so, unmark
			this->puzzleCompletionStatus[0x48] = 0x64;
			this->var_str_384 = this->arr_str_195e8[0x48] + g_zbasic->str(303);
			g_zbasic->menu(7, 8, 1, this->var_str_384);
		}
	}
	// 140:024c
	if (this->var_i16_7d2 == 0) {
		// show the fake puzzle picture
		g_toolbox->PenNormal();
		this->arr_i32_192c0[0] = g_toolbox->GetPicture(0x78);
		g_zbasic->picture(0, 0, this->arr_i32_192c0[0]);
		g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
		this->sub_128_61c2();
	}
	// 140:02a2
	if (this->var_i16_37a == 0) {
		g_toolbox->SetPort(this->var_i32_8_thoth);
	} else {
		this->sub_128_1ef8();
	}
	// 140:02b4
	this->var_i16_68a = 1;
	for (int16 i = 1; i <= 0x155; i++) {
		g_zbasic->get(1, i, SCREEN_WIDTH, i + 1, this->arr_bmp_fa3c);
		g_zbasic->put(g_zbasic->rndInt(0x14) - 0xa, i, this->arr_bmp_fa3c, kPutCopy);
		g_toolbox->Delay(0);
	}
	// 140:0328
	if (this->var_i16_7d2 == 0) {
		g_toolbox->PenNormal();
		Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		for (int16 i = 0; i <= 0x22; i++) {
			this->var_i32_692 = g_toolbox->TickCount();
			g_toolbox->InvertRect(area);
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
			if (i % 2 == 0) {
				g_toolbox->Delay(0);
			}
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

void FoolGame::thoth99Enchantments() {
	// 140:0458
	this->sub_128_4da(0);
	if (this->activePuzzleBuffer.empty()) { // was: str(307)
		this->arr_i16_1eb8[0] = 0x63;
	} else {
		this->arr_i16_1eb8[0] = g_zbasic->decodeInt(this->activePuzzleBuffer);
	}
	// 140:04a2
	if (this->arr_i16_1eb8[0] < 1) {
		this->arr_i16_1eb8[0] = 1;
	}
	if (this->arr_i16_1eb8[0] == 0x63) {
		g_zbasic->text(0xfe, 0x18, 0x10, kSrcBic);
		this->var_i16_7a2 = 0xaf;
		this->sub_128_918(g_zbasic->str(308)); // you dare to challenge
		this->var_i16_7a2 = 0xd2;
		this->sub_128_918(g_zbasic->str(309)); // the high priestess
		this->sub_128_3da(0xb4);
	}
	// 140:0532
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		this->arr_rect_1f38[i].top = puzzlesReadShort();
		this->arr_rect_1f38[i].left = puzzlesReadShort();
		this->arr_rect_1f38[i].bottom = this->arr_rect_1f38[i].top + 0x28;
		this->arr_rect_1f38[i].right = this->arr_rect_1f38[i].left + 0x28;
	}
	// 140:060c
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
	this->copyScreen(0, this->arr_bmp_b3ec);
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		g_toolbox->PenSize(5, 5);
		g_toolbox->PenMode(kPatXor);
		for (int16 j = 0; j <= 1; j++) {
			g_toolbox->MoveTo(0x104, 0xa2);
			g_toolbox->LineTo(this->arr_rect_1f38[i].left + 0x14, this->arr_rect_1f38[i].top + 0x14);
			g_toolbox->InvertRoundRect(this->arr_rect_1f38[i], 0x14, 0x14);
			g_toolbox->Delay(0);
		}
		g_toolbox->PenNormal();
		// 140:06cc
		this->arr_i16_1eb8[2] = i;
		this->thothDrawEnchantment();
	}
	// 140:06fc
	g_toolbox->PenNormal();
	this->sub_140_e3c();
	this->arr_i16_4758[4] = 0x14;
	this->arr_i16_4758[5] = 0;
	this->arr_i16_4758[6] = SCREEN_HEIGHT;
	this->arr_i16_4758[7] = SCREEN_WIDTH;
	this->arr_i16_1eb8[1] = 0;
	g_zbasic->unk_20();
	this->sub_128_4da(1);
	this->stateFlags = kStateNull;
	this->var_i16_d0c = 0;
	this->var_i16_2326 = 0;
	this->var_i16_2328 = 0;

	// new value: number of updates before a screen flash
	this->arr_i16_1eb8[3] = 0;

	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 140:0752
		this->var_i32_692 = g_toolbox->TickCount();
		this->arr_i16_1eb8[1]++;
		this->arr_i16_1eb8[3]++;
		if (this->arr_i16_1eb8[1] >= this->arr_i16_1eb8[0]) {
			this->arr_i16_1eb8[1] = 0;
		}
		// flash the screen. normally this happens after running through all the
		// remaining enchantments, so it increases in intensity as you clear them.
		// limit the amount of continuous full-screen flashing to WCAG
		// recommendation of 3/second
		if (this->arr_i16_1eb8[3] >= MAX(this->arr_i16_1eb8[0], (int16)10)) {
			this->arr_i16_1eb8[3] = 0;
			Common::Rect temp;
			temp.top = this->arr_i16_4758[4];
			temp.left = this->arr_i16_4758[5];
			temp.bottom = this->arr_i16_4758[6];
			temp.right = this->arr_i16_4758[7];
			g_toolbox->InvertRect(temp);
		}
		// 140:07c4
		// Phase 1: enchantments 99 to 34 are inverting and don't move
		if (this->arr_i16_1eb8[0] >= 0x21) {
			if ((this->arr_i16_1eb8[1] % 0xa) == 0) {
				this->arr_i16_1eb8[2] = this->arr_i16_1eb8[0];
			} else {
				// 140:0826
				this->arr_i16_1eb8[2] = g_zbasic->rndInt(this->arr_i16_1eb8[0]);
			}
			// 140:084a
			this->thothDrawEnchantment();
		// Phase 2: enchantments 33 to 2 erase themselves and move around
		} else {
			// 140:0852
			this->arr_i16_1eb8[2] = this->arr_i16_1eb8[0] - this->arr_i16_1eb8[1];
			if (this->arr_i16_1eb8[2] == this->arr_i16_1eb8[0]) {
				if ((this->var_i16_2328 == 1) && (g_toolbox->TickCount() > (this->var_i32_68e + 0x28))) {
					this->var_i16_2328 = 0;
				}
				if (this->var_i16_2328 == 0) {
					this->var_i32_68e = g_toolbox->TickCount();
					this->var_i16_2328 = 1;
					this->thothMoveEnchantment();
				}
				// 140:090c
			} else {
				// 140:0910
				this->thothMoveEnchantment();
			}
		}
		do {
			this->getNextEvent(-1);
			if ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) {
				this->sub_140_c4c();
			}
			// 140:0934
			if (this->var_i16_2326 != 0) {
				this->thothScrambleScreen();
			}
		} while (this->var_ev_46.what != 0);
		// 140:094a
		this->sub_128_406(1);
	}
	// 140:097a
	if (this->var_i16_d0c != 0) {
		this->sub_140_f84();
		this->activePuzzleBuffer.clear(); // was: 310
	} else {
		// 140:09a4
		if (this->arr_i16_1eb8[0] < 1) {
			this->arr_i16_1eb8[0] = 1;
		}
		this->activePuzzleBuffer = g_zbasic->encodeInt(this->arr_i16_1eb8[0]);
	}
	// 140:09ee
}

void FoolGame::thothMoveEnchantment() {
	// 140:09f0
	g_toolbox->FillRoundRect(this->arr_rect_1f38[this->arr_i16_1eb8[2]], 0x14, 0x14, this->arr_pat_58f4[2]);
	this->arr_rect_1f38[this->arr_i16_1eb8[2]].top = g_zbasic->rndInt(0x11a) + 0x14;
	this->arr_rect_1f38[this->arr_i16_1eb8[2]].left = g_zbasic->rndInt(0x1da);
	this->arr_rect_1f38[this->arr_i16_1eb8[2]].bottom = this->arr_rect_1f38[this->arr_i16_1eb8[2]].top + 0x28;
	this->arr_rect_1f38[this->arr_i16_1eb8[2]].right = this->arr_rect_1f38[this->arr_i16_1eb8[2]].left + 0x28;
	this->thothDrawEnchantment();
}

void FoolGame::thothDrawEnchantment() {
	// 140:0b2c
	g_toolbox->PenNormal();
	g_toolbox->EraseRoundRect(this->arr_rect_1f38[this->arr_i16_1eb8[2]], 0x14, 0x14);
	g_toolbox->FrameRoundRect(this->arr_rect_1f38[this->arr_i16_1eb8[2]], 0x14, 0x14);
	g_zbasic->text(0, 0xc, 0, kSrcOr);
	this->var_str_384 = Common::U32String::format(" %d ", this->arr_i16_1eb8[2]); // was: str(311)
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_384);
	// 140:0bd2
	g_toolbox->MoveTo(
		this->arr_rect_1f38[this->arr_i16_1eb8[2]].left + 0x14 - (this->var_i16_484 / 2),
		this->arr_rect_1f38[this->arr_i16_1eb8[2]].top + 0x18
	);
	g_toolbox->DrawString(this->var_str_384);
}

void FoolGame::sub_140_c4c() {
	// 140:0c4c
	if (this->var_i16_d0c != 0)
		return;
	if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1f38[this->arr_i16_1eb8[0]])) {
		this->arr_i16_1eb8[0]--;
		if (this->arr_i16_1eb8[0] <= 0) {
			this->var_i16_d0c = 1;
			return;
		}
		this->var_i16_484 = this->arr_i16_1eb8[0] + 1;
		g_toolbox->FillRoundRect(this->arr_rect_1f38[this->var_i16_484], 0x14, 0x14, this->arr_pat_58f4[2]);
		this->sub_128_962(
			this->arr_rect_1f38[this->var_i16_484].top,
			this->arr_rect_1f38[this->var_i16_484].left,
			this->arr_rect_1f38[this->var_i16_484].bottom,
			this->arr_rect_1f38[this->var_i16_484].right,
			0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatXor, 0xd
		);
		this->arr_i16_1eb8[1] = 0;
		this->sub_140_e3c();
		this->var_i16_2326 = 0;
		this->var_i16_2328 = 0;
	} else {
		// 140:0dcc
		this->var_i16_2326 = 1;
	}
}

void FoolGame::thothScrambleScreen() {
	// 140:0dd4
	if (this->var_i16_2326 < 0) {
		this->var_i16_2326 = 0;
	}
	g_toolbox->InvertRect(this->thothRandomSquare());
	g_toolbox->InvertRect(this->thothRandomHRect());
	g_toolbox->InvertRect(this->thothRandomVRect());
	Common::Rect temp;
	temp.top = this->arr_i16_4758[4];
	temp.left = this->arr_i16_4758[5];
	temp.bottom = this->arr_i16_4758[6];
	temp.right = this->arr_i16_4758[7];
	g_toolbox->InvertRect(temp);
	this->var_i16_2326++;
	if (this->var_i16_2326 >= 4)
		this->var_i16_2326 = 0;
}

void FoolGame::sub_140_e3c() {
	// 140:0e3c
	if (this->arr_i16_1eb8[0] == 1) {
		g_zbasic->menu(8, 0, 1, g_zbasic->str(312)); // the last enchantment
		g_zbasic->menu(8, 1, 1, g_zbasic->str(313)); // run for your life
	} else {
		// 140:0e8a
		this->var_str_384 = Common::U32String::format(" %d", this->arr_i16_1eb8[0]);
		this->var_str_9f4 = g_zbasic->str(314) + this->var_str_384 + g_zbasic->str(315); // the X enchantments
		g_zbasic->menu(8, 0, 1, this->var_str_9f4);
		g_zbasic->menu(8, 1, 1, g_zbasic->str(316)); // run for your life
		g_zbasic->menu(8, 2, 1, g_zbasic->str(317)); // -
		g_zbasic->menu(8, 3, 1, g_zbasic->str(318)); // or press each button
		g_zbasic->menu(8, 4, 1, g_zbasic->str(319)); // in descending order
		this->var_str_9f4 = g_zbasic->str(320) + this->var_str_384 + g_zbasic->str(321); // from X to 1
		g_zbasic->menu(8, 5, 1, this->var_str_9f4);
	}
	// 140:0f82
	return;
}

void FoolGame::sub_140_f84() {
	// 140:0f84
	g_toolbox->PenNormal();
	g_toolbox->PenSize(5, 5);
	this->arr_i16_4758[4] = (this->arr_rect_1f38[1].top / 0x19) + 1;
	this->arr_i16_4758[5] = (this->arr_rect_1f38[1].left / 0x19) + 1;
	// 140:0fea
	this->arr_i16_4758[6] = ((SCREEN_HEIGHT - this->arr_rect_1f38[1].bottom) / 0x19) + 1;
	this->arr_i16_4758[7] = ((SCREEN_WIDTH - this->arr_rect_1f38[1].right) / 0x19) + 1;
	// 140:1066
	// unrolled loop
	Common::Rect area = this->arr_rect_1f38[1];
	// 140:10aa
	for (int16 i = 0; i <= 0x1a; i++) {
		area.top -= this->arr_i16_4758[4];
		area.left -= this->arr_i16_4758[5];
		area.bottom += this->arr_i16_4758[6];
		area.right += this->arr_i16_4758[7];
		g_toolbox->FrameRect(area);
		this->sub_128_3da(1);
	}
	// 140:11ae
	// unrolled loop
	area = this->arr_rect_1f38[1];
	// 140:11f2
	for (int16 i = 0; i <= 0x1a; i++) {
		area.top -= this->arr_i16_4758[4];
		area.left -= this->arr_i16_4758[5];
		area.bottom += this->arr_i16_4758[6];
		area.right += this->arr_i16_4758[7];
		g_toolbox->InvertRect(area);
		this->sub_128_3da(1);
	}
	// 140:12f6
	this->sub_140_3296();
}

void FoolGame::thothKey1st() {
	// 140:12fc
	this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 2);
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
	if ((this->var_i16_7ce & 2) != 0) {
		g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
	}
	// 140:1346
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenMode(kPatXor);
	this->var_ev_46.where = Common::Point(0, 0);
	// 140:139a
	while (!(
		(this->var_ev_46.where.x >= 0xff) &&
		(this->var_ev_46.where.y >= 0xa2) &&
		(this->var_ev_46.where.x <= 0x10e) &&
		(this->var_ev_46.where.y <= 0xac)
	)) {
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

void FoolGame::thothKey2nd() {
	// 140:1486
	if (this->activePuzzle != 0x34) {
		this->thothAdjustPuzzleData();
	}
	// FIXME: this looks lethal
	// 140:1494: MOVE.L - var_i32_232a,D0
	// 140:1498: ADDI.L - 0x0000018c,D0
	// 140:149e: MOVE.L - D0,var_i32_696
	this->var_bytes_696 = this->var_bytes_232a;
	this->var_ptr_696 = this->var_ptr_232a + 0x18c;
	for (int16 j = 1; j <= 7; j++) {
		for (int16 i = 0; i <= 3; i++) {
			this->arr_i16_2f38[j*32+i] = this->puzzlesReadShort();
		}
	}

	this->var_i16_d0c = 0;
	this->var_i16_103a = 0;
	this->stateFlags = kStateNull;
	this->var_i16_232e = 0;
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 140:1506
		// was: SetRect
		this->arr_i16_4758[4] = 0x14;
		this->arr_i16_4758[5] = 0;
		this->arr_i16_4758[6] = SCREEN_HEIGHT;
		this->arr_i16_4758[7] = SCREEN_WIDTH;
		Common::Rect playArea(0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		this->var_i16_232e++;
		if (this->var_i16_232e < 1) {
			this->var_i16_232e = 1;
		}
		if (this->var_i16_232e > 6) {
			this->var_i16_232e = 1;
		}
		// 140:1546
		switch (this->var_i16_232e-1) {
		case 0:
			// 140:31b0
			g_toolbox->InvertRect(this->thothRandomSquare());
			break;
		case 1:
			// 140:31c4
			g_toolbox->InvertRect(this->thothRandomHRect());
			break;
		case 2:
			// 140:31d8
			g_toolbox->InvertRect(this->thothRandomVRect());
			g_toolbox->InvertRect(playArea);
			break;
		case 3:
			// 140:31fa
			g_toolbox->FillRect(this->thothRandomSquare(), this->arr_pat_58f4[2]);
			break;
		case 4:
			// 140:321a
			g_toolbox->FillRect(this->thothRandomHRect(), this->arr_pat_58f4[2]);
			break;
		case 5:
			// 140:323a
			g_toolbox->FillRect(this->thothRandomVRect(), this->arr_pat_58f4[2]);
			break;
		default:
			warning("thothKey2nd: breaking out of switch statement");
			break;
		}
		if (this->activePuzzle != 0x34) {
			Common::Rect temp;
			temp.top = this->arr_i16_2f38[(this->var_i16_103a + 1)*32];
			temp.left = this->arr_i16_2f38[(this->var_i16_103a + 1)*32+1];
			temp.bottom = this->arr_i16_2f38[(this->var_i16_103a + 1)*32+2];
			temp.right = this->arr_i16_2f38[(this->var_i16_103a + 1)*32+3];
			g_toolbox->FillRect(temp, this->arr_pat_58f4[1]);
		}
		// 140:159e
		this->getNextEvent(-1);
		if (this->var_ev_46.what == 1) {
			this->sub_140_15fc();
		}
		if (this->var_i16_103a == 4) {
			this->var_i16_d0c = 1;
		}
		if (this->stateFlags == kStateSaveGame) {
			this->saveGame();
		}

		// 140:15d0
	}
	// 140:15fa
}

void FoolGame::sub_140_15fc() {
	// 140:15fc
	this->var_i16_7e4 = 0;
	for (int16 i = 1; i <= 7; i++) {
		Common::Rect temp;
		temp.top = this->arr_i16_2f38[i*32];
		temp.left = this->arr_i16_2f38[i*32+1];
		temp.bottom = this->arr_i16_2f38[i*32+2];
		temp.right = this->arr_i16_2f38[i*32+3];

		if (g_toolbox->PtInRect(this->var_ev_46.where, temp)) {
			this->var_i16_7e4 = i;
		}
	}
	this->var_i16_103a++;
	if (this->var_i16_7e4 != this->var_i16_103a) {
		this->var_i16_103a = 0;
		this->sub_128_50e(0xf, 0x64, 0x1);
		for (int16 i = 0; i <= 9; i++) {
			Common::Rect temp;
			temp.top = this->arr_i16_4758[4];
			temp.left = this->arr_i16_4758[5];
			temp.bottom = this->arr_i16_4758[6];
			temp.right = this->arr_i16_4758[7];
			g_toolbox->InvertRect(temp);
		}
	} else {
		// 140:1698
		if (this->activePuzzle == 0x34) {
			for (int16 i = 0; i <= 7; i++) {
				Common::Rect temp;
				temp.top = this->arr_i16_2f38[32*i];
				temp.left = this->arr_i16_2f38[32*i+1];
				temp.bottom = this->arr_i16_2f38[32*i+2];
				temp.right = this->arr_i16_2f38[32*i+3];
				g_toolbox->EraseRect(temp);
			}
			// 140:16ce
			for (int16 i = 1; i <= 0xc; i++) {
				for (int16 j = 1; j <= 7; j++) {
					Common::Rect temp;
					temp.top = this->arr_i16_2f38[32*j];
					temp.left = this->arr_i16_2f38[32*j+1];
					temp.bottom = this->arr_i16_2f38[32*j+2];
					temp.right = this->arr_i16_2f38[32*j+3];
					g_toolbox->InvertRect(temp);
				}
				this->sub_128_3da(2);
			}
			// 140:1710
			Common::Rect temp;
			temp.top = this->arr_i16_2f38[32*this->var_i16_7e4];
			temp.left = this->arr_i16_2f38[32*this->var_i16_7e4+1];
			temp.bottom = this->arr_i16_2f38[32*this->var_i16_7e4+2];
			temp.right = this->arr_i16_2f38[32*this->var_i16_7e4+3];
			for (int16 i = 1; i <= 0x19; i++) {
				g_toolbox->InvertRect(temp);

			}
			// 140:173a
			if (this->var_i16_103a < 4) {
				this->var_i16_2330 = 2;
				this->var_i16_1dee = kPatXor;
			} else {
				// 140:1756
				this->var_i16_2330 = 3;
				this->var_i16_1dee = kPatCopy;
			}
			// 140:1762
			this->sub_128_962(
				this->arr_i16_2f38[32*this->var_i16_7e4],
				this->arr_i16_2f38[32*this->var_i16_7e4+1],
				this->arr_i16_2f38[32*this->var_i16_7e4+2],
				this->arr_i16_2f38[32*this->var_i16_7e4+3],
				0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH,
				this->var_i16_2330, this->var_i16_1dee, 0x19
			);
		} else {
			// 140:1804
			for (int16 i = 0; i <= 3; i++) {
				this->arr_i16_4758[i] = this->arr_i16_2f38[32*this->var_i16_103a + i];
			}
			// 140:184e
			for (int16 i = 0; i <= 0x24; i++) {
				this->arr_i16_4758[0]++;
				this->arr_i16_4758[1]++;
				this->arr_i16_4758[2]--;
				this->arr_i16_4758[3]--;
				Common::Rect temp;
				temp.top = this->arr_i16_4758[0];
				temp.left = this->arr_i16_4758[1];
				temp.bottom = this->arr_i16_4758[2];
				temp.right = this->arr_i16_4758[3];
				g_toolbox->InvertRect(temp);
			}
		}
	}
	// 140:18f8

}

void FoolGame::thothKey3rd() {
	// 140:18fa
	if (this->activePuzzle != 0x34) {
		this->thothAdjustPuzzleData();
	}
	// FIXME: this looks lethal
	// 140:1908: MOVE.L - var_i32_232a,D0
	// 140:190c: ADDI.L - 0x000001c4,D0
	// 140:1912: MOVE.L - D0,var_i32_696
	this->var_bytes_696 = this->var_bytes_232a;
	this->var_ptr_696 = this->var_ptr_232a + 0x1c4;
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
		if (this->activePuzzle == 0x34) { // in high priestess, no hints
			this->var_i16_232e++;
			if (this->var_i16_232e < 1) {
				this->var_i16_232e = 1;
			}
			if (this->var_i16_232e > 3) {
				this->var_i16_232e = 1;
			}
			switch (this->var_i16_232e-1) {
			case 0:
				// 140:325a
				g_toolbox->InvertRect(this->thothRandomSquare());
				break;
			case 1:
				// 140:326e
				g_toolbox->InvertRect(this->thothRandomHRect());
				break;
			case 2:
				// 140:3282
				g_toolbox->InvertRect(this->thothRandomVRect());
				break;
			default:
				warning("thothKey3rd: broke out of switch");
				break;
			}
		} else { // in justice, animate the grab bar
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
			this->thothKey3rdOnClick();
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

void FoolGame::thothKey3rdOnClick() {
	// 140:1bc4
	this->var_i16_2332 = 1;
	if (this->activePuzzle == 0x34) {
		if ((this->var_ev_46.where.x >= 0xf) && (this->var_ev_46.where.y >= 0x2d) && (this->var_ev_46.where.x <= 0x1f1) && (this->var_ev_46.where.y <= 0x147)) {
			this->var_i16_2332 = 0;
		}
	} else {
		// 140:1c22
		if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1f38[this->var_i16_103a]) == 0) {
			this->var_i16_2332 = 0;
		}
	}
	// 140:1c58
	if (this->var_i16_2332 == 0) {
		this->sub_140_1f68();
		return;
	}
	// 140:1c66
	Common::Rect area(-1, 0x14, SCREEN_WIDTH + 1, SCREEN_HEIGHT + 1);
	if (this->var_ev_46.where.x < 0xf) {
		area.right = 0xf;
		this->var_i16_1de6 = 0;
	}
	// 140:1cc6
	if (this->var_ev_46.where.x > 0x1f1) {
		area.left = 0x1f1;
		this->var_i16_1de6 = 1;
	}
	// 140:1ce6
	if (this->var_ev_46.where.y < 0x2d) {
		area.bottom = 0x2d;
		this->var_i16_1de6 = 2;
	}
	// 140:1d06
	if (this->var_ev_46.where.y > 0x147) {
		area.top = 0x147;
		this->var_i16_1de6 = 3;
	}
	// 140:1d26
	while ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) {
		// 140:1d2a
		// was: 0
		this->getNextEvent(-1);
		if (this->var_i16_1de6 == 0) {
			area.right = this->var_ev_46.where.x + 0xa;
		}
		if (this->var_i16_1de6 == 1) {
			area.left = this->var_ev_46.where.x - 0xa;
		}
		if (this->var_i16_1de6 == 2) {
			area.bottom = this->var_ev_46.where.y + 0xa;
		}
		if (this->var_i16_1de6 == 3) {
			area.top = this->var_ev_46.where.y - 0xa;
		}
		// 140:1dc6
		if (area.top < 0x14) {
			area.top = 0x14;
		}
		g_toolbox->FillRect(area, this->arr_pat_58f4[this->arr_i16_2f38[this->var_i16_1de6*32+ this->var_i16_103a]]);
		// 140:1e2e
		if ((area.left <= 0) && (area.top <= 0x14) && (area.right >= SCREEN_WIDTH) && (area.bottom >= SCREEN_HEIGHT)) {
			this->var_ev_46.modifiers = kModMouseButtonUp;
		}
		// 140:1eae
	}
	// 140:1ec4
	if ((area.left <= 0) && (area.top <= 0x14) && (area.right >= SCREEN_WIDTH) && (area.bottom >= SCREEN_HEIGHT)) {
		if (this->var_i16_1de6 == this->var_i16_103a) {
			this->sub_140_205e();
			return;
		}
	}
	// 140:1f62
	this->sub_140_1f68();
}

void FoolGame::sub_140_1f68() {
	// 140:1f68
	this->sub_128_50e(0xf, 0x64, 1);
	this->var_i16_103a = 0;
	g_toolbox->PenSize(0x14, 0x14);
	g_toolbox->PenPat(this->arr_pat_58f4[3]);
	Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 0; i <= 0x104; i += 0x14) {
		area.top = 0x14 + i;
		area.left = i;
		area.bottom = SCREEN_HEIGHT - i;
		area.right = SCREEN_WIDTH - i;
		g_toolbox->FrameRect(area);
		g_toolbox->Delay(0);
	}
	// 140:2056
	g_toolbox->PenNormal();
	this->sub_128_6186();
}

void FoolGame::sub_140_205e() {
	// 140:205e
	this->var_i16_103a++;
	this->sub_128_6186();
	g_toolbox->PenNormal();
}

void FoolGame::thothKeyLast() {
	// 140:206a
	this->var_i16_d0c = 0;
	this->stateFlags = kStateNull;
	this->thothKeyLastSetup();
	this->sub_140_2978();
	this->copyScreen(1, this->arr_bmp_b3ec);
	this->arr_i16_1eb8[1] = -1;
	this->var_i16_1de6 = this->arr_i16_3b38[this->hermitPathStage];
	warning("thothKeyLast: 1de6: %d, target: %d", this->var_i16_1de6, (this->arr_i16_3b38[0x20 + this->hermitPathStage] + 1));
	// JMP 0x228c
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 140:20c2
		do {
			if (this->activePuzzle != 0x34) { // we're in the hermit, print the shape
				this->hermitPathStartIndex = this->arr_i16_3b38[this->hermitPathStage];
				this->hermitPathEndIndex = this->arr_i16_3b38[this->hermitPathStage + 0x20];
				do {
					this->getNextEvent(-1);
					g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_3738[this->hermitPathStartIndex]], this->arr_pat_58f4[this->var_i16_2338]);
					this->hermitPathStartIndex++;
					// we're updating the screen in the event handler, this has no effect
					/*if (this->hermitPathStage == 5) {
						if (this->hermitPathStartIndex % 4 == 0)
							g_toolbox->Delay(0);
					} else {
						if (this->hermitPathStartIndex % 8 == 0)
							g_toolbox->Delay(0);
					}*/
				// 140:2168
				} while (!((this->var_ev_46.what == 1) || (this->hermitPathStartIndex > this->hermitPathEndIndex) || ((this->stateFlags & kStateReturn) != 0)));
				// 140:219e
				if ((this->var_ev_46.what == 0) && ((this->stateFlags & kStateReturn) == 0)) {
					if (this->hermitPathStage != 5) {
						g_toolbox->Delay(0x7d*60/1000);
					} else {
						g_toolbox->Delay(0xfa*60/1000);
					}
				}
				// 140:21e8
				this->copyScreen(1, this->arr_bmp_b3ec);
			} else { // we're in the high priestess, show nothing but random filled squares
				// 140:21fe
				this->var_i16_484 = g_zbasic->rndInt(0x4e) + 2;
				this->var_i16_68a = g_zbasic->rndInt(0x14) - 1;
				this->var_i16_68c = g_zbasic->rndInt(0xe) - 1;
				// 140:2222
				g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]], this->arr_pat_58f4[this->var_i16_484]);
				this->getNextEvent(-1);
			}
			if (this->var_ev_46.what == 1) {
				this->thothKeyLastOnClick();
			}
			if (this->stateFlags == kStateSaveGame) {
				this->saveGame();
			}
			// 140:228c
		} while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0));
	}
}

void FoolGame::thothKeyLastOnClick() {
	// 140:22b8
	while (((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) && (this->var_i16_1de6 != (this->arr_i16_3b38[0x20 + this->hermitPathStage] + 1))) {
		// 140:22bc
		this->getNextEvent(-1); // was: 0
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		if ((this->var_i16_68a >= 0) && (this->var_i16_68c >= 0) && (this->var_i16_68a <= 0x13) && (this->var_i16_68c <= 0xd)) {
			// 140:2316
			this->arr_i16_1eb8[0] = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
			if (this->arr_i16_1eb8[0] != this->arr_i16_1eb8[1]) {
				// 140:2372
				if (this->arr_i16_1eb8[0] != this->arr_i16_3738[this->var_i16_1de6]) {
					this->thothBadSelect();
					return;
				}
				// 140:23aa
				this->arr_i16_1eb8[1] = this->arr_i16_1eb8[0];
				this->sub_128_50e(0x32 + (this->var_i16_1de6 * 5), 0x28, 0);
				g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_1eb8[0]], this->arr_pat_58f4[2]);
				this->var_i16_1de6++;
				warning("thothKeyLastOnClick: 1de6: %d, target: %d", this->var_i16_1de6, (this->arr_i16_3b38[0x20 + this->hermitPathStage] + 1));
			}
		}
		// 140:2422
	}
	// 140:2472
	if (this->var_i16_1de6 != (this->arr_i16_3b38[0x20 + this->hermitPathStage] + 1)) {
		this->thothBadSelect();
	} else {
		this->hermitNextStage();
	}
}

void FoolGame::thothBadSelect() {
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
		for (int16 i = this->var_i16_1de6; i >= this->arr_i16_3b38[this->hermitPathStage]; i--) {
			this->var_i16_484 = g_zbasic->rndInt(0x4e) + 2;
			g_toolbox->FillRect(this->arr_rect_1f38[this->arr_i16_3738[i]], this->arr_pat_58f4[this->var_i16_484]);
		}
	}
	// 140:25b4
	this->arr_i16_1eb8[1] = -1;
	this->var_i16_1de6 = this->arr_i16_3b38[this->hermitPathStage];
}

void FoolGame::hermitNextStage() {
	// 140:25e4
	if (this->activePuzzle == 0x34) {
		this->var_i16_d0c = 1;
		return;
	}
	// 140:25f6
	if (this->hermitPathStage < 5) {
		this->hermitScreenFlash();
	}
	if (this->hermitPathStage == 4) {
		this->hermitScreenZoom();
	}
	if (this->hermitPathStage == 5) {
		this->hermitScreenBehold();
	}
	if (this->hermitPathStage == 6) {
		this->sub_140_2968();
	}
	if (this->var_i16_d0c == 0) {
		this->hermitPathStage++;
		if (this->hermitPathStage == 6) {
			this->thothKeyLastSetup();
		}
		this->sub_140_2978();
		this->copyScreen(1, this->arr_bmp_b3ec);
	}
	// 140:2660
}

void FoolGame::hermitScreenFlash() {
	// 140:2662
	Common::Rect area(0x37, 0x29, 0x1c9, 0x142);
	for (int16 i = 0; i <= 0x10; i++) {
		g_toolbox->InvertRect(area);
		this->sub_128_3da(1);
	}
}

void FoolGame::hermitScreenZoom() {
	// 140:26ca
	this->sub_128_4da(0);
	g_toolbox->PenSize(0xa, 0x7);

	for (int16 j = 0; j <= 2; j++) {
		// 140:26e0
		Common::Rect area(0x2d, 0x22, 0x1d3, 0x149);
		for (int16 i = 0; i <= 0x14; i++) {
			area.top += 7;
			area.left += 0xa;
			area.bottom -= 7;
			area.right -= 0xa;
			g_toolbox->InvertRect(area);
			this->sub_128_3da(1);
		}
		for (int16 i = 0; i <= 0x13; i++) {
			area.top -= 7;
			area.left -= 0xa;
			area.bottom += 7;
			area.right += 0xa;
			g_toolbox->InvertRect(area);
			this->sub_128_3da(1);
		}
	}
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	this->var_i16_1de6 = 0;
	this->sub_128_4da(1);
}

void FoolGame::hermitScreenBehold() {
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
	if (this->hermitPathStage < 6) {
		this->var_i16_2338 = 1;
	} else {
		this->var_i16_2338 = 2;
	}
}

void FoolGame::thothKeyLastSetup() {
	// 140:2998
	if (this->hermitPathStage != 6) {
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

	} else { // last key of thoth
		// 140:2b58
		if (this->activePuzzle != 0x34) { // in the hermit
			this->thothAdjustPuzzleData();
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
	if (this->hermitPathStage != 6) {
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
	if ((this->hermitPathStage == 6) && (this->activePuzzle != 0x34)) {
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

Common::Rect FoolGame::thothRandomSquare() {
	// 140:3050
	int16 x = g_zbasic->rndInt(0x19c);
	int16 y = 0x14 + g_zbasic->rndInt(0xde);
	return Common::Rect(
		x,
		y,
		x + 0x64,
		y + 0x64
	);
}

Common::Rect FoolGame::thothRandomHRect() {
	// 140:30da
	int16 y = 0x14 + g_zbasic->rndInt(0xde);
	return Common::Rect(
		0,
		y,
		SCREEN_WIDTH,
		y + g_zbasic->rndInt(0x64)
	);
}

Common::Rect FoolGame::thothRandomVRect() {
	// 140:3148
	int16 x = g_zbasic->rndInt(0x19c);
	return Common::Rect(
		x,
		0x14,
		x + g_zbasic->rndInt(0x64),
		SCREEN_HEIGHT
	);
}

void FoolGame::sub_140_3296() {
	// 140:3296
	if (this->var_i16_37a == 0) {
		g_toolbox->SetPort(this->var_i32_8_thoth);
	} else {
		this->sub_128_1ef8();
	}
	this->sub_140_32ac();
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
	Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 1; i <= 0xd; i++) {
		this->var_i32_692 = g_toolbox->TickCount();
		g_toolbox->InvertRect(area);
		this->sub_128_406(1);
	}
	// 140:334a
	g_toolbox->FillRect(area, this->arr_pat_58f4[2]);
	this->sub_128_4da(1);
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::thothAdjustPuzzleData() {
	// 140:3372
	// change puzzle data pointer to use high priestess data
	this->var_i16_484 = (this->puzzleDataOffsets[0x34] - 1) / 1000;
	this->var_i16_7e4 = (this->puzzleDataOffsets[0x34] - 1) % 1000;
	g_zbasic->record(1, this->var_i16_484, this->var_i16_7e4);
	//this->var_bytes_696 = this->arr_bytes_109dc;
	this->var_bytes_696 = g_zbasic->readFile(1, this->puzzleDataOffsets[0x35] - this->puzzleDataOffsets[0x34]);
	this->var_ptr_696 = 0;
	this->var_bytes_232a = this->var_bytes_696;
	this->var_ptr_232a = this->var_ptr_696;
}

void FoolGame::sub_140_3412() {
	// 140:3412
	g_toolbox->PenNormal();
	if ((this->var_i16_7ce & 2) == 0) {
		g_toolbox->InitCursor();
	}
}

}
