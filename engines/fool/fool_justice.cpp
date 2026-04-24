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

// justice - lights on
void FoolGame::justiceRun() {
	// 142:0852
	if (this->var_i16_c04 < 0x63) {
		this->sub_142_9be();
		if (this->var_i16_d0c == 0) {
			this->justiceStoreState();
			return;
		}
		// 142:0874
		this->justiceResetGrid();
		if (this->puzzleCompletionStatus[0x34] > 3) {
			this->var_i16_c04 = 0x64;
			return;
		} else {
			// 142:08a2
			this->sub_128_962(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x1a);
			this->var_i16_c04 = 0x63;
			this->var_str_384 = g_zbasic->str(345); // behold the 3rd key of thoth
			this->sub_128_178a(0x3f, 1);
		}
	}
	// 142:0906
	if (this->var_i16_c04 == 0x63) {
		g_zbasic->menu(8, 0, 1, g_zbasic->str(346)); // the 3rd key of thoth
		g_zbasic->menu(8, 1, 1, g_zbasic->str(347)); // return to scroll
		this->thothKey3rd();
		if (this->var_i16_d0c == 1) {
			this->sub_128_962(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xa2, 0xff, 0xac, 0x10e, 2, kPatXor, 0x19);
		}
	}
	// 142:098a
	if (this->var_i16_c04 >= 0x64) {
		this->sub_142_9be();
		if (this->var_i16_d0c == 0) {
			this->justiceStoreState();
			return;
		}
		// 142:09b0
		this->justiceResetGrid();
		this->var_i16_c04 = 0x65;
	}
	// 142:09ba
	return;
}

void FoolGame::sub_142_9be() {
	// 142:09be
	this->sub_128_271a();
	for (int16 i = 1; i <= 0x19; i++) {
		g_zbasic->indexRawSet(this->puzzlesReadString(), 1, i);
	}
	// 142:09ea
	if (this->puzzleCompletionStatus[0x34] < 4) {
		g_zbasic->menu(8, 3, 1, g_zbasic->str(348)); // a secret hides here
		g_zbasic->menu(8, 4, 1, g_zbasic->str(349)); // if twenty-five appear
	} else {
		// 142:0a3a
		g_zbasic->menu(8, 3, 1, g_zbasic->str(350)); // you will remain here
		g_zbasic->menu(8, 4, 1, g_zbasic->str(351)); // unless twenty-five appear
	}
	// 142:0a6a
	this->justiceZoom();
	this->var_i16_484 = 0;
	for (int16 i = 0x24; i <= 0x11d; i += 0x32) {
		for (int16 j = 0x8c; j <= 0x185; j += 0x32) {
			this->var_i16_484++;
			g_toolbox->SetRect(
				this->arr_rect_1f38[this->var_i16_484],
				j,
				i,
				j + 0x32,
				i + 0x32
			);
		}
	}
	// 142:0adc
	this->justiceZoom();
	this->fillRect(0x1a, 0x82, 0x128, 0x190, 2);
	if (this->activePuzzleBuffer.empty()) { // was: str(352)
		this->activePuzzleBuffer = (g_zbasic->space(0xc) + g_zbasic->str(353) + g_zbasic->space(0xc)).encode(Common::kMacRoman); // 1
		this->var_i16_233e = 1;
	} else {
		// 142:0b4c
		this->var_i16_233e = g_zbasic->castInt(g_zbasic->leftStr(this->activePuzzleBuffer, 1));
		this->activePuzzleBuffer = g_zbasic->rightStr(this->activePuzzleBuffer, 0x19);
	}
	// 142:0b7c
	for (this->var_i16_484 = 1; this->var_i16_484 <= 0x19; this->var_i16_484++) {
		this->var_i16_103a = g_zbasic->castInt(g_zbasic->midStr(this->activePuzzleBuffer, this->var_i16_484, 1));
		if ((this->var_i16_103a & 2) == 0) {
			g_zbasic->indexSet(g_zbasic->str(354), 1, (this->var_i16_484 + 0x19));
		} else {
			// 142:0be0
			g_zbasic->indexSet(g_zbasic->str(355), 1, (this->var_i16_484 + 0x19));
		}
		// 142:0c06
		if ((this->var_i16_103a & 1) != 0) {
			this->justiceDrawBlock();
		} else {
			this->justiceRemoveBlock();
		}
	}
	// 142:0c32
	this->var_i16_d0c = 0;
	this->stateFlags = 0;
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		// 142:0c42
		while ((this->stateFlags == 0) && (this->var_i16_d0c == 0)) {
			this->getNextEvent(-1);
			if (this->var_ev_46.what == 1) {
				this->justiceOnClick();
			}
		}
		// 142:0c76
		if (this->stateFlags == kStateSaveGame) {
			this->justiceStoreState();
			this->saveGame();
		}
		// 142:0c88
	}
}

void FoolGame::justiceOnClick() {
	// 142:0cb2
	this->var_i16_68a = (this->var_ev_46.where.x - 0x5a) / 0x32;
	this->var_i16_68c = (this->var_ev_46.where.y + 0xe) / 0x32;
	if ((this->var_i16_68a < 0) || (this->var_i16_68c < 0) || (this->var_i16_68a > 5) || (this->var_i16_68c > 5)) {
		return;
	}
	// 142:0d26
	this->var_i16_103a = (this->var_i16_68c - 1)*5 + this->var_i16_68a;
	if (this->arr_i16_3738[this->var_i16_103a] == 0) {
		return;
	}
	// 142:0d56
	g_toolbox->InvertRoundRect(this->arr_rect_1f38[this->var_i16_103a], 0x1e, 0x1e);
	if (g_zbasic->index(1, this->var_i16_103a + 0x19) == g_zbasic->str(356)) { // \xa5
		g_zbasic->indexSet(g_zbasic->str(357), 1, this->var_i16_103a + 0x19); // ~
	}
	// 142:0dc6
	if (this->var_i16_103a == 0xd) { // center button
		if (this->var_i16_233e == 0) {
			for (this->var_i16_484 = 1; this->var_i16_484 <= 0x19; this->var_i16_484++) {
				g_zbasic->indexSet(g_zbasic->str(358), 1, this->var_i16_484 + 0x19); // \xa5
				this->arr_i16_3738[this->var_i16_484] = 0;
				this->justiceRemoveBlock();
			}
			this->var_i16_233e = 1;
			// redraw center tile
			this->var_i16_484 = 0xd;
			this->justiceDrawBlock();
			return;
		}
		// 142:0e3e
		this->var_i16_233e = 0;
	}
	// 142:0e44
	Common::String buffer = g_zbasic->indexRaw(1, this->var_i16_103a);
	if (g_zbasic->leftStr(buffer, 1) == g_zbasic->str(359).encode(Common::kMacRoman)) { // M
		this->var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(buffer, 2, 2));
		this->justiceDrawBlock();
		this->var_i16_2340 = 5;
	} else {
		// 142:0ea8
		this->var_i16_2340 = 2;
	}
	// 142:0eae
	this->var_i16_2342 = g_zbasic->decodeInt(g_zbasic->midStr(buffer, this->var_i16_2340, 2));
	for (int16 i = 1; i <= this->var_i16_2342; i++) {
		this->var_i16_2340 += 2;
		this->var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(buffer, this->var_i16_2340, 2));
		// 142:0ef0
		if (this->arr_i16_3738[this->var_i16_484] == 0) {
			this->justiceDrawBlock();
		} else {
			this->justiceRemoveBlock();
		}
	}
	// 142:0f20
	this->var_i16_d0c = 1;
	for (int16 i = 1; i <= 0x19; i++) {
		if (this->arr_i16_3738[i] == 0) {
			this->var_i16_d0c = 0;
		}
	}
	this->sub_128_6186();
}

void FoolGame::justiceZoom() {
	// 142:0f58
	this->sub_128_962(0x130, 0x76, 0x130, 0x76, 0x1a, 0x82, 0x128, 0x190, 1, kPatXor, 0x19);
}

void FoolGame::justiceDrawBlock() {
	// 142:0f96
	this->arr_i16_3738[this->var_i16_484] = 1;
	this->sub_128_50e(g_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	g_toolbox->PenNormal();
	g_toolbox->FillRoundRect(this->arr_rect_1f38[this->var_i16_484], 0x1e, 0x1e, this->arr_pat_58f4[0]);
	g_toolbox->FrameRoundRect(this->arr_rect_1f38[this->var_i16_484], 0x1e, 0x1e);
	g_zbasic->text(0, 0xc, 0, kSrcOr);
	this->var_str_384 = g_zbasic->index(1, this->var_i16_484 + 0x19);
	this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_384);
	// 142:1056
	g_toolbox->MoveTo(
		this->arr_rect_1f38[this->var_i16_484].left + 0x19 - (this->var_i16_7e4/2),
		this->arr_rect_1f38[this->var_i16_484].top + 0x1e
	);
	g_toolbox->DrawString(this->var_str_384);
}

void FoolGame::justiceRemoveBlock() {
	// 142:10bc
	this->arr_i16_3738[this->var_i16_484] = 0;
	this->sub_128_50e(g_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	g_toolbox->FillRect(this->arr_rect_1f38[this->var_i16_484], this->arr_pat_58f4[g_zbasic->rndInt(0x4d)+3]);
}

void FoolGame::justiceStoreState() {
	// 142:111e
	this->var_str_384 = Common::U32String::format(" %d", this->var_i16_233e);
	this->activePuzzleBuffer = g_zbasic->rightStr(this->var_str_384, 1);
	for (int16 i = 1; i <= 0x19; i++) {
		this->var_i16_484 = this->arr_i16_3738[i];
		if (g_zbasic->index(1, i + 0x19) == g_zbasic->str(360)) { // ~
			this->var_i16_484 |= 2;
			this->var_str_384 = g_zbasic->rightStr(Common::U32String::format(" %d", this->var_i16_484), 1);
			this->activePuzzleBuffer += this->var_str_384;
		}
	}
	// 142:11fc
}

void FoolGame::justiceResetGrid() {
	// 142:11fe
	this->sub_128_3da(0x28);
	this->activePuzzleBuffer.clear();
	for (int16 i = 1; i <= 0x19; i++) {
		this->sub_128_50e(g_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
		g_toolbox->FillRect(this->arr_rect_1f38[i], this->arr_pat_58f4[2]);
	}
	// 142:126e
	this->sub_128_962(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x1a);
}

}
