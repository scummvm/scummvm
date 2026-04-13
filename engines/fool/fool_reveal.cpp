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

// mask reveal puzzle
void FoolGame::revealRun() {
	// 134:0004
	this->sub_128_271a();
	this->var_i16_c00 = 1;
	this->arr_i16_1eb8[15] = this->puzzlesReadShort();
	this->arr_i16_1eb8[16] = this->puzzlesReadShort();
	this->arr_i16_1eb8[21] = this->puzzlesReadShort();
	debugC(5, kDebugLoading, "%d, %d, %04x", this->arr_i16_1eb8[15], this->arr_i16_1eb8[16], this->arr_i16_1eb8[21]);
	this->var_str_384 = this->puzzlesReadString();
	this->var_str_384 = g_zbasic->str(213) + this->var_str_384 + g_zbasic->str(214);
	g_zbasic->menu(8, 6, 1, this->var_str_384);
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		this->arr_i16_3738[i] = this->puzzlesReadByte();
	}
	// 134:00ce
	// rack up polygon information
	g_toolbox->SetPort(this->var_i32_f24);
	this->arr_i16_1eb8[20] = 0;

	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		this->var_i16_103a = this->puzzlesReadShort();
		this->arr_poly_192c0[i] = g_toolbox->OpenPoly();
		// 134:010e
		for (int j = 5; j <= this->var_i16_103a - 1; j += 2) {
			this->var_i16_484 = this->puzzlesReadShort();
			this->var_i16_7e4 = this->puzzlesReadShort();
			if (j == 5) {
				g_toolbox->MoveTo(this->var_i16_7e4, this->var_i16_484 + 0xf);
			} else {
				g_toolbox->LineTo(this->var_i16_7e4, this->var_i16_484 + 0xf);
			}
			// 134:015c
		}
		g_toolbox->ClosePoly();
		if (i <= this->arr_i16_1eb8[16]) {
			this->arr_i16_1eb8[20] |= this->bitLUT[i - 1];
		}
		// 134:01ce
	}
	// 134:01ea
	g_toolbox->SetPort(this->var_i32_0);

	this->arr_i16_1eb8[17] = (0x1c2 / this->arr_i16_1eb8[15]);
	this->arr_i16_1eb8[18] = 0x100 - ((this->arr_i16_1eb8[15] * this->arr_i16_1eb8[17]) / 2);
	this->var_str_1272 = this->puzzlesReadString();
	this->sub_128_962(0xb4, 0xff, 0xb6, 0x101, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
	this->sub_128_962(0x9a, 0xff, 0x9c, 0x101, 0x37, 0x13, 0xff, 0x1ee, 0, kPatCopy, 0x19);
	// 134:02f0
	this->arr_i16_1eb8[19] = g_zbasic->unk_310(this->activePuzzleBuffer);
	this->var_i16_1aba = this->arr_i16_1eb8[18];
	g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	for (this->var_i16_7be = 1; this->var_i16_7be <= this->arr_i16_1eb8[15]; this->var_i16_7be++) {
		// play a random tone and draw each of the letter buttons in sequence
		this->sub_128_50e(
			g_zbasic->rndInt(0x3e8) + 0x19,
			0x28,
			1
		);
		// 134:0352
		this->arr_rect_1f38[this->var_i16_7be].top = 0x10f;
		this->arr_rect_1f38[this->var_i16_7be].left = this->var_i16_1aba;
		this->arr_rect_1f38[this->var_i16_7be].bottom = 0x137;
		this->arr_rect_1f38[this->var_i16_7be].right = this->var_i16_1aba + this->arr_i16_1eb8[17];

		this->var_i16_1aba += this->arr_i16_1eb8[17];
		// 134:03fc
		g_toolbox->FillRoundRect(this->arr_rect_1f38[this->var_i16_7be], 0x19, 0x19, this->arr_pat_58f4[0]);
		g_toolbox->FrameRoundRect(this->arr_rect_1f38[this->var_i16_7be], 0x19, 0x19);
		// 134:0448
		this->var_str_384 = g_zbasic->midStr(this->var_str_1272, this->var_i16_7be, 1);
		this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_384);
		g_toolbox->MoveTo(
			this->arr_rect_1f38[this->var_i16_7be].left + (this->arr_i16_1eb8[17] / 2) - (this->var_i16_7ba / 2),
			0x127
		);
		g_toolbox->DrawString(this->var_str_384);
		// 134:04d6
		if (this->arr_i16_1eb8[19] & this->bitLUT[this->arr_i16_3738[this->var_i16_7be] - 1]) {
			this->revealSelectButton();
		}
	}
	// 134:0538
	this->stateFlags = kStateNull;
	if (this->arr_i16_1eb8[20] == this->arr_i16_1eb8[19]) {
		this->var_i16_d0c = 1;
	} else {
		// 134:0572
		this->var_i16_d0c = 0;
	}
	g_toolbox->PenNormal();
	// 134:057a
	// 134:057a: JMP - [0x61e]
	// main event loop
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 134:057e
		while ((this->stateFlags == 0) && (this->var_i16_d0c == 0)) {
			// 134:0582
			this->sub_128_c6a(-1);
			if (this->var_ev_46.what == kMouseDown) {
				this->revealOnClick();
			}
			if (this->arr_i16_1eb8[20] == this->arr_i16_1eb8[19]) {
				this->var_i16_d0c = 1;
			}
			// render screen
			if (this->var_ev_46.what == kNullEvent) {
				g_toolbox->Delay(0);
			}
		}
		// 134:05e2
		if (this->stateFlags == kStateUndo) {
			this->revealReset();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->activePuzzleBuffer = g_zbasic->unk_88(this->arr_i16_1eb8[19]);
			this->saveGame();
		}
		// 134:061e
	}
	// 134:0648
	if (this->var_i16_d0c != 0) {
		this->revealSuccess();
	}
	this->activePuzzleBuffer = g_zbasic->unk_88(this->arr_i16_1eb8[19]);
	// 134:0678: JMP - [0x8ae]
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		g_toolbox->KillPoly(this->arr_poly_192c0[i]);
	}
}

void FoolGame::revealOnClick() {
	// 134:067c
	this->var_i16_7be = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[15]; j++) {
		if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1f38[j])) {
			this->var_i16_7be = j;
			j = this->arr_i16_1eb8[15];
		}
	}
	// 134:06d0
	if (this->var_i16_7be == 0) {
		return;
	}
	// 134:06f4
	this->revealSelectButton();
	this->arr_i16_1eb8[19] ^= this->bitLUT[this->arr_i16_3738[this->var_i16_7be]-1];
	debugC(5, kDebugLoading, "reveal: %d, %04x, %04x", this->var_i16_7be, this->arr_i16_1eb8[19], this->arr_i16_1eb8[20]);
	this->sub_128_6186();
}

void FoolGame::revealSelectButton() {
	// 134:074a
	// invert the button
	g_toolbox->InvertRoundRect(this->arr_rect_1f38[this->var_i16_7be], 0x19, 0x19);
	g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_1eb8[21]]);
	// fill the polygon
	g_toolbox->PenMode(kPatXor);
	g_toolbox->PaintPoly(this->arr_poly_192c0[this->arr_i16_3738[this->var_i16_7be]]);
	g_toolbox->PenNormal();
}

void FoolGame::revealReset() {
	// 134:07bc
	this->fillRect(0x37, 0x13, 0xff, 0x1ee, 0);
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		if (this->arr_i16_1eb8[19] & this->bitLUT[this->arr_i16_3738[i]-1]) {
			g_toolbox->InvertRoundRect(this->arr_rect_1f38[i], 0x19, 0x19);
		}
	}
	// 134:085c
	this->arr_i16_1eb8[19] = 0;
	this->stateFlags = kStateNull;
}

void FoolGame::revealSuccess() {
	// 134:0872
	if (this->var_i16_c04 < 0x64) {
		this->var_i16_c04 = 0x64;
	}
	this->sub_128_2664();
	this->sub_128_6186();
	this->sub_128_d34(0x37, 0x13, 0xff, 0x1ee, 0xc8);
}

}
