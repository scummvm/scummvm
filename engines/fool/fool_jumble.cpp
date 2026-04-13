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
#include <climits>

namespace Fool {

extern ZBasic *g_zbasic;
extern Toolbox *g_toolbox;

// word jumble/substitute puzzle
void FoolGame::jumbleRun() {
	// 130:0004
	this->var_i16_d0c = 0;
	this->sub_128_271a();
	this->jumbleSubPuzzleCount = this->puzzlesReadShort();
	debugC(5, kDebugLoading, "FoolGame::jumbleRun: racking %d subpuzzles", this->jumbleSubPuzzleCount);

	for (this->jumbleCurrentSubPuzzle = 1; this->jumbleCurrentSubPuzzle <= this->jumbleSubPuzzleCount; this->jumbleCurrentSubPuzzle++) {
		this->var_i16_105a = this->puzzlesReadShort();
		debugC(5, kDebugLoading, "FoolGame::jumbleRun: subpuzzle %d", this->jumbleCurrentSubPuzzle);
		// load grid information block
		// 0x0: grid width (squares)
		// 0x1: grid height (squares)
		// 0x2:
		// 0x3:
		// 0x4:
		// 0x5:
		// 0x6: grid square outer width (px)
		// 0x7: grid square outer height (px)
		// 0x8: grid top bound (px)
		// 0x9: grid bottom bound (px)
		// 0xa: grid left bound (px)
		// 0xb: grid right bound (px)
		// 0xc: grid square inner height (px)
		// 0xd: grid square inner width (px)
		if ((this->var_i16_105a & 0x1) == 1) {
			for (int i = 0; i <= 0xe; i++) {
				this->arr_i16_1eb8[i] = this->puzzlesReadShort();
			}
		}
		// 130:0066
		this->var_i16_484 = 0;
		this->var_i16_68c = this->arr_i16_1eb8[8];
		do {
			this->var_i16_68a = this->arr_i16_1eb8[10];
			do {
				this->var_i16_484++;
				g_toolbox->SetRect(
					this->arr_rect_1f38[this->var_i16_484],
					this->var_i16_68a,
					this->var_i16_68c,
					this->var_i16_68a + this->arr_i16_1eb8[13],
					this->var_i16_68c + this->arr_i16_1eb8[12]
				);
				// 130:00f6
			} while (g_zbasic->incrAndCheck(this->var_i16_68a, this->arr_i16_1eb8[11], this->arr_i16_1eb8[6]));
		// 130:0126
		} while (g_zbasic->incrAndCheck(this->var_i16_68c, this->arr_i16_1eb8[9], this->arr_i16_1eb8[7]));
		// 130:0156
		if ((this->var_i16_105a & 0x2) == 2) {
			this->var_i16_105c = this->puzzlesReadShort();
			this->var_i16_105e = this->puzzlesReadShort();
			this->var_i16_1060 = this->puzzlesReadShort();
			this->var_i16_1062 = this->puzzlesReadShort();
			this->var_i16_1064 = this->puzzlesReadShort();
			this->var_i16_1066 = this->puzzlesReadShort();
			if (this->var_i16_1066 > 0) {
				this->arr_i32_192c0[0] = g_toolbox->GetPicture(this->var_i16_1066);
				g_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_i32_192c0[0]);
				g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
			}
		}
		// 130:01f0
		this->jumbleGameType = this->puzzlesReadShort();
		if (this->jumbleGameType == 6) {
			for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
				for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
					this->arr_i16_2f38[i*32 + j] = this->puzzlesReadByte();
					this->arr_i16_3b38[i*32 + j] = 0;
				}
			}
		} else {
		// 130:028a
			for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
				for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
					this->arr_i16_2f38[i*32 + j] = 0;
					this->arr_i16_3b38[i*32 + j] = 0;
				}
			}
		}
		// 130:0306
		this->var_i16_103a = this->puzzlesReadShort();
		if (this->var_i16_103a > 0) {
			g_toolbox->PenNormal();
			g_toolbox->PenPat(this->arr_pat_58f4[this->var_i16_1060]);
			this->var_i16_484 = 0;
			for (int j = 1; j <= this->var_i16_103a; j++) {
				this->var_i16_106a = this->puzzlesReadShort();
				if (this->var_i16_106a == 0) {
					this->var_i16_484 += 0x10;
				} else {
				// 130:035a
					for (int i = 0; i <= 0xf; i++) {
						this->var_i16_484++;
						if (this->var_i16_106a & this->bitLUT[i]) {
							g_toolbox->PaintRect(this->arr_rect_1f38[this->var_i16_484]);
						}
					}
					// 130:039e

				}
				// 103:03aa
			}
		}
		// 130:03ba
		if (this->var_i16_105e == 2) {
			this->var_i16_106c = kSrcBic;
		} else {
			this->var_i16_106c = kSrcOr;
		}
		// 130:03d4
		if (this->jumbleGameType == 4) {
			this->fillRect(0x63, 0xae, 0x108, 0x153, 2);
		}
		// 130:03fe
		//if (this->var_i16_c04 <= this->jumbleCurrentSubPuzzle)
		this->var_i16_106e = 0;
		this->var_str_1070 = g_zbasic->str(168); // empty
		this->var_str_1170 = g_zbasic->str(169); // empty
		this->var_i16_1270 = this->puzzlesReadShort();
		for (this->var_i16_103a = 1; this->var_i16_103a <= this->var_i16_1270; this->var_i16_103a++) {
			this->var_str_1272 = this->puzzlesReadString();
			this->var_i16_1372 = this->var_str_1272.size();
			this->var_str_1170 += this->var_str_1272;
			// 130:047e
			if (this->var_i16_c04 > this->jumbleCurrentSubPuzzle) {
				this->var_str_1070 += this->var_str_1272;
			} else {
				// 130:04ac
				if ((this->var_i16_c04 == this->jumbleCurrentSubPuzzle) && !this->activePuzzleBuffer.empty()) { // was: str(170)
					// 130:04dc
					this->var_str_1070 = this->activePuzzleBuffer;
				} else {
					// 130:04f0
					if ((this->jumbleGameType == 1) || (this->jumbleGameType == 5)) {
						// 130:0514
						this->var_str_384 = this->var_str_1272;
						this->sub_130_25d8();
					}
					// 130:0528
					if (this->jumbleGameType == 2) {
						this->var_str_384 = this->var_str_1272;
						this->sub_130_2790();
					}
					// 130:0548
					if ((this->jumbleGameType == 3) || (this->jumbleGameType == 6)) {
						this->var_str_9f4 = g_zbasic->space(this->var_i16_1372);
					}
					// 130:0580
					if (this->jumbleGameType == 4) {
						this->var_str_384 = g_zbasic->leftStr(this->var_str_1272, 4);
						this->sub_130_25d8();
						this->var_str_1374 = this->var_str_9f4;
						this->var_str_384 = g_zbasic->rightStr(this->var_str_1272, 4);
						this->sub_130_25d8();
						this->var_str_9f4 = this->var_str_9f4 + this->var_str_1474 + this->var_str_1374 + g_zbasic->midStr(this->var_str_1272, 5, 1) + this->var_str_1474;
					}
					// 130:0610
					this->var_str_1070 += this->var_str_9f4;
				}
			}
			// 130:0628
			for (this->var_i16_484 = 1; this->var_i16_484 <= this->var_i16_1372; this->var_i16_484++) {
				this->var_i16_106e++;
				this->arr_i16_3738[this->var_i16_106e] = this->puzzlesReadShort();
				this->var_i16_1574 = ABS(this->arr_i16_3738[this->var_i16_106e]);
				this->sub_130_20fe();
				this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] = this->var_i16_103a;
				this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c] = this->var_i16_106e;
				this->sub_130_1c6c();
				this->var_str_d12 = g_zbasic->midStr(this->var_str_1070, this->var_i16_106e, 1);
				// 130:06d4
				if ((this->activePuzzle <= 0x50) || (this->activePuzzle == 0x55)) {
					this->sub_130_1dcc();
				} else {
					// 130:0700
					this->sub_130_1e76();
				}
			}
			// 130:0716
			g_zbasic->indexSet(this->puzzlesReadString(), 1, this->var_i16_103a);
		}
		// 130:073e
		this->var_i16_1576 = this->var_str_1170.size();
		if ((this->activePuzzle == 0x51) || (this->activePuzzle == 0x54)) {
			// 130:076e
			g_toolbox->PenNormal();
			g_toolbox->PenPat(this->arr_pat_58f4[1]);
			g_toolbox->PenSize(3, 3);
			if (this->activePuzzle == 0x51) {
				// 130:0794
				g_toolbox->MoveTo(0xf3, 0x6f);
				g_toolbox->LineTo(0xf3, 0xe4);
				g_toolbox->MoveTo(0x9b, 0xa9);
				g_toolbox->LineTo(0x14c, 0xa9);
			}
			// 130:07bc
			if (this->activePuzzle == 0x54) {
				g_toolbox->MoveTo(0xe9, 0x81);
				g_toolbox->LineTo(0x17d, 0x81);
				g_toolbox->MoveTo(0x124, 0x47);
				g_toolbox->LineTo(0x124, 0xda);
			}
			// 130:07f0
			g_toolbox->PenNormal();
		}
		// 130:07f2
		this->var_i16_103a = this->puzzlesReadShort();
		if (this->var_i16_103a > 0) {
			for (this->var_i16_68a = 1; this->var_i16_68a <= this->var_i16_103a; this->var_i16_68a++) {
				this->var_i16_484 = this->puzzlesReadShort();
				this->var_i16_7e4 = this->puzzlesReadShort();
				this->var_str_1578 = this->puzzlesReadString();
				g_zbasic->text(0xfa, 0xc, 0, kSrcOr);
				if (this->var_i16_7e4 > 0) {
					g_toolbox->MoveTo(this->var_i16_484, this->var_i16_7e4);
					g_toolbox->DrawString(this->var_str_1578);
				} else {
					this->sub_130_2226();
				}
			}
		}
		// 130:086a
		this->var_i16_1678 = 0;
		this->var_str_1578 = g_zbasic->str(171);
		if (this->jumbleCurrentSubPuzzle == this->jumbleSubPuzzleCount) {
			// all puzzles done, load the success animation list
			this->arr_i16_4338[0] = this->puzzlesReadShort();
			if (this->arr_i16_4338[0] > 0) {
				// 130:08be
				for (int i = 1; i <= this->arr_i16_4338[0]; i++) {
					this->arr_i16_4338[i] = this->puzzlesReadShort();
				}
			}
		}
		// 130:08fa
		if (this->var_i16_c04 <= this->jumbleCurrentSubPuzzle) {
			switch (this->jumbleGameType-1) {
			case 0:
				this->sub_130_d2e();
				break;
			case 1:
				this->sub_130_d90();
				break;
			case 2:
				this->sub_130_1004();
				break;
			case 3:
			case 4:
				this->sub_130_10a6();
				break;
			case 5:
				this->sub_130_1282();
				break;
			default:
				break;
			}
			if ((this->stateFlags & kStateReturn) != 0) {
				this->sub_130_c66();
				this->jumbleCurrentSubPuzzle = this->jumbleSubPuzzleCount;
			} else {
				// 130:0956
				if (this->activePuzzle <= 0x50) {
					if ((this->var_i16_1678 > 0) && (this->var_str_1578 != g_zbasic->str(172))) {
						// 130:098c
						this->var_str_1578 = g_zbasic->str(173);
						this->sub_130_2226();
					}
					// 130:09a4
					for (int i = 1; i <= 3; i++) {
						this->sub_128_50e(g_zbasic->rndInt(0x1f4) + 0x19, 0x42, 0);
					}
					for (int j = 0; j <= 1; j++) {
						for (int i = 1; i <= this->var_i16_1576; i++) {
							g_toolbox->Delay((0xc - (this->var_i16_1576 / 0xa))*60/1000);
							g_toolbox->InvertRect(this->arr_rect_1f38[ABS(this->arr_i16_3738[i])]);
						}
					}
					if (this->jumbleCurrentSubPuzzle == this->jumbleSubPuzzleCount) {
						this->var_i16_d0c = 1;
					}
				}
			}
		}
		// 130:0a5c
	}
	if ((this->stateFlags & kStateReturn) != 0) {
		// JMP 0x2962
		return;
	}
	// if we're here, the puzzle is solved
	g_toolbox->PenNormal();
	this->sub_128_2664();
	this->sub_128_6186();
	this->sub_128_61ec();
	// 130:0a90
	if (this->activePuzzle != 0x55) {
		// 130:0b68
		while ((this->var_ev_46.modifiers & kModMouseButtonUp) != 0) {
			// 130:0aa0
			if (this->arr_i16_4338[0] > 0) {
				// strobe the squares in the victory list
				if (this->arr_i16_4338[0] == 1) {
					// originally was 0x64, lock to 200ms
					this->var_i16_7e4 = 0xc8;
				} else {
					// originally was 0xd, lock to 2 frames
					this->var_i16_7e4 = 0x22;
				}
				this->var_i16_68a = 0;
				do {
					int ticks = (int)g_toolbox->TickCount();
					this->var_i16_68a++;
					g_toolbox->InvertRect(this->arr_rect_1f38[this->arr_i16_4338[this->var_i16_68a]]);
					// mask was originally 0
					this->sub_128_c6a(-1);
					int delta = MAX(0, this->var_i16_7e4*60/1000 - ((int)g_toolbox->TickCount() - ticks));
					g_toolbox->Delay(delta);
				} while (!((this->var_i16_68a == this->arr_i16_4338[0]) || ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0)));
			} else {
			// 130:0b62
				// mask was originally 0
				this->sub_128_c6a(-1);
			}
		}
	// 130:0b7a
	} else {
		// 130:0b7e
		this->fillRect(0x131, 0, 0x156, 0x10e, 0x47);
		this->var_i16_d0c = 0;
		this->var_i16_bfc = 0;
		do {
			for (int i = 1; i <= this->arr_i16_4338[0]; i++) {
				g_toolbox->Delay(0xd*60/1000);
				g_toolbox->InvertRect(this->arr_rect_1f38[this->arr_i16_4338[i]]);
				this->sub_128_c6a(0);
				if ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) {
					this->var_i16_d0c = 1;
				}
			}
			// 130:0c14
			if (this->var_i16_bfc == 0) {
				this->var_i16_bfc = 1;
			} else {
				this->var_i16_bfc = 0;
			}
		} while (!((this->var_i16_d0c == 1) && (this->var_i16_bfc == 1)));
	}
	// 130:0c4e
	this->sub_130_c66();
}

void FoolGame::sub_130_c56() {
	// 130:0c56
	this->sub_130_c66();
	this->saveGame();
	this->stateFlags = kStateNull;
}

void FoolGame::sub_130_c66() {
	// 130:0c66
	if (this->var_i16_d0c != 0) {
		this->var_i16_c04 = 0x64;
		this->activePuzzleBuffer.clear(); // was: str(174)
		this->var_i16_d0c = 0;
	} else {
		// 130:0c98
		if (this->var_i16_c04 < 0x64) {
			this->var_i16_c04 = this->jumbleCurrentSubPuzzle;
			if ((this->jumbleGameType == 2) && (this->var_str_1578 != g_zbasic->str(175))) {
				this->activePuzzleBuffer = this->var_str_1070 + g_zbasic->leftStr(this->var_str_1578, this->var_i16_167a-2);
			} else {
				// 130:0cfe
				this->activePuzzleBuffer = this->var_str_1070;
			}
			// 130:0d0e
		} else {
			// 130:0d12
			this->var_i16_c04 = 0x65;
			this->activePuzzleBuffer.clear(); // was: str(176)
		}
	}
	// 130:0d2c
	return;
}

void FoolGame::sub_130_d2e() {
	// 130:0d2e
	this->sub_130_1426();
	this->sub_128_61ec();
	this->stateFlags = kStateNull;
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_str_1070 != this->var_str_1170)) {
		this->sub_128_c6a(-1);
		if (this->var_ev_46.what == 1) {
			this->sub_130_1476();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->sub_130_c56();
		}
	}
}

void FoolGame::sub_130_d90() {
	// 130:0d90
	// substitution cipher
	if (g_zbasic->index(1, 1) != g_zbasic->str(177)) {
		if (this->var_str_1070.size() == this->var_str_1170.size()) {
			this->var_i16_1372 = g_zbasic->index(1, 1).size();
			this->var_str_1578 = g_zbasic->space(this->var_i16_1372);
			for (int i = 1; i<= this->var_i16_1372; i++) {
				this->var_str_167c = g_zbasic->midStr(g_zbasic->index(1, 1), i, 1);
				this->var_i16_7e4 = g_zbasic->asc(g_zbasic->ucase(this->var_str_167c)) - 0x40;
				if ((this->var_i16_7e4 >= 1) && (this->var_i16_7e4 <= 0x1a)) {
					this->var_str_d12 = g_zbasic->chr(this->arr_i16_5cbc[this->var_i16_7e4] + 0x40);
				} else {
					// 130:0e88
					this->var_str_d12 = this->var_str_167c;
				}
				// 130:0e98
				g_zbasic->midStrSet(this->var_str_1578, i, 1, this->var_str_d12);
			}

		} else {
			// 130:0ece
			this->var_i16_484 = this->var_str_1070.size();
			this->var_i16_7e4 = this->var_str_1170.size();
			this->var_str_1578 = g_zbasic->rightStr(this->var_str_1070, this->var_i16_484 - this->var_i16_7e4);
			this->var_str_1070 = g_zbasic->leftStr(this->var_str_1070, this->var_i16_7e4);

		}
		// 130:0f24
		this->var_str_1578 += g_zbasic->str(178) + g_zbasic->index(1, 1); // ABC = XYZ
		this->var_i16_167a = g_zbasic->instr(1, this->var_str_1578, g_zbasic->str(179));
		this->sub_130_2226();
	} else {
		// 130:0f72
		this->var_str_1578 = g_zbasic->str(180);
	}
	// 130:0f86
	this->var_i16_106e = 1;
	this->sub_130_1c1a();
	this->sub_128_61ec();
	this->stateFlags = kStateNull;
	this->keyLastPressed = 0;
	// 130:0fa0
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_str_1070 != this->var_str_1170)) {
		// 130:0fa4
		this->sub_128_c6a(-1);
		if (this->keyLastPressed != 0) {
			this->sub_130_15da();
		}
		if (this->var_ev_46.what == 1) {
			this->sub_130_1476();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->sub_130_c56();
		}
	}
	// 130:0ffe
	this->sub_130_1c52();
}

void FoolGame::sub_130_1004() {
	// 130:1004
	this->var_i16_106e = g_zbasic->instr(1, this->var_str_1070, g_zbasic->str(181));
	if (this->var_i16_106e == 0) {
		this->var_i16_106e = 1;
	}
	this->sub_130_20d2();
	this->sub_128_61ec();
	this->stateFlags = kStateNull;
	this->keyLastPressed = 0;
	// 130:1042
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_str_1070 != this->var_str_1170)) {
		this->sub_128_c6a(-1);
		if (this->keyLastPressed != 0) {
			this->sub_130_15da();
		}
		if (this->var_ev_46.what == 1) {
			this->sub_130_1476();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->sub_130_c56();
		}
	}
	this->sub_130_1c52();
}

void FoolGame::sub_130_10a6() {
	// 130:10a6
	this->sub_130_1426();
	if (this->jumbleGameType != 5) {
		this->var_str_177c = g_zbasic->midStr(this->var_str_1170, 1, 1) + g_zbasic->midStr(this->var_str_1170, 4, 1) + g_zbasic->midStr(this->var_str_1170, 7, 1);
		this->var_str_177c += g_zbasic->midStr(this->var_str_1170, 2, 1) + g_zbasic->midStr(this->var_str_1170, 5, 1) + g_zbasic->midStr(this->var_str_1170, 8, 1);
		this->var_str_177c += g_zbasic->midStr(this->var_str_1170, 3, 1) + g_zbasic->midStr(this->var_str_1170, 6, 1) + g_zbasic->midStr(this->var_str_1170, 9, 1);
	} else {
		// 130:1196
		this->var_str_177c = g_zbasic->midStr(this->var_str_1170, 3, 2) + g_zbasic->midStr(this->var_str_1170, 1, 2) + g_zbasic->midStr(this->var_str_1170, 5, 1);
		this->var_str_177c += g_zbasic->midStr(this->var_str_1170, 8, 2) + g_zbasic->midStr(this->var_str_1170, 6, 2);
	}
	// 130:1212
	this->sub_128_61ec();
	this->stateFlags = kStateNull;
	// 130:121c
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_str_1070 != this->var_str_1170) && (this->var_str_1070 != this->var_str_177c)) {
		this->sub_128_c6a(-1);
		if (this->var_ev_46.what == 1) {
			this->sub_130_1476();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->sub_130_c56();
		}
	}
}

void FoolGame::sub_130_1282() {
	// 130:1282
	this->var_i16_106e = g_zbasic->instr(1, this->var_str_1070, g_zbasic->str(182));
	if (this->var_i16_106e == 0) {
		this->var_i16_106e = 1;
	}
	this->sub_130_20d2();
	this->sub_128_61ec();
	this->var_i16_7b2 = 0;
	this->stateFlags = kStateNull;
	this->keyLastPressed = 0;
	g_toolbox->InitCursor();
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_str_1070 != this->var_str_1170)) {
		// 130:12cc
		this->sub_128_c6a(-1);
		if (this->keyLastPressed != 0) {
			this->sub_130_15da();
		}
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		if (this->var_ev_46.what == 1) {
			this->sub_130_1476();
		}
		if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) || (this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
			this->var_i16_187c = 0;
		} else {
			this->var_i16_187c = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
		}
		// 130:139e
		if (this->var_i16_187c != this->var_i16_7b2) {
			if (this->var_i16_187c < 0x11) {
				g_toolbox->InitCursor();
			} else {
				g_toolbox->SetCursor(this->arr_curs_4d88[this->var_i16_187c]);
			}
			// 130:13dc
			this->var_i16_7b2 = this->var_i16_187c;
		}
		// 130:13e2
		this->var_i16_7b2 = this->var_i16_187c;
		if (this->stateFlags == kStateSaveGame) {
			this->sub_130_c56();
		}
	}
	// 130:141e
	this->sub_130_1c52();
	g_toolbox->InitCursor();
}

void FoolGame::sub_130_1426() {
	// 130:1426
	this->var_i16_187e = 0;
	this->var_i16_1880 = 0;
	if (g_zbasic->index(1, 1) != g_zbasic->str(183)) {
		this->var_i16_1678 = 1;
		this->var_str_1578 = g_zbasic->index(1, 1);
		this->sub_130_2226();
	}
	// 130:1474
}

void FoolGame::sub_130_1476() {
	// 130:1476
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) || (this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
		// 130:14fa
		if (this->var_ev_46.where.x > 0x14) {
			this->sub_128_50e(0xf, 0x42, 0x1);
		}
		return;
	}
	// 130:1518
	if (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] < 1) {
		this->sub_128_50e(0xf, 0x42, 0x1);
		return;
	}
	// 130:1558
	if ((this->jumbleGameType == 6) && (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] >= 0x11)) {
		this->sub_128_50e(0xf, 0x42, 0x1);
		return;
	}
	// 130:15b0
	switch (this->jumbleGameType - 1) {
	case 0:
		this->sub_130_172c();
		break;
	case 1:
	case 2:
	case 6:
		this->sub_130_20aa();
		break;
	case 3:
	case 4:
		this->sub_130_172c();
		break;
	default:
		warning("sub_130_1476: unhandled case %d", this->jumbleGameType-1);
		break;
	}
}

void FoolGame::sub_130_15da() {
	// 130:15da
	if ((this->keyLastPressed == 3) || (this->keyLastPressed == 0xd)) {
		this->sub_130_1c52();
		this->sub_130_2078();
		this->sub_130_20d2();
		this->keyLastPressed = 0;
		return;
	}
	if (this->keyLastPressed == 8) {
		this->sub_130_1c52();
		this->sub_130_2094();
		this->sub_130_20d2();
		this->keyLastPressed = 0;
		return;
	}
	// 130:1632
	if ((this->keyLastPressed >= 0x61) && (this->keyLastPressed <= 0x7a)) {
		this->keyLastPressed -= 0x20;
	}
	if (this->jumbleGameType != 3) {
		if ((this->keyLastPressed < 0x41) || (this->keyLastPressed > 0x5a)) {
			return;
		}
	} else {
		// 130:1690
		this->var_i16_1882 = 0;
		if ((this->keyLastPressed < 0x41) || (this->keyLastPressed > 0x5a)) {
			this->var_i16_1882 = 1;
		}
		if (this->keyLastPressed == 0x20) {
			this->var_i16_1882 = 0;
		}
		if (this->var_i16_1882 != 0)
			return;
	}
	// 130:16dc
	this->var_str_d12 = g_zbasic->chr(this->keyLastPressed);
	// 130:16f2
	this->keyLastPressed = 0;
	if (this->jumbleGameType != 2) {
		// 130:1706
		if (!((this->jumbleGameType == 3) || (this->jumbleGameType == 6))) {
			this->sub_130_172c();
		} else {
			this->sub_130_201a();
		}
	} else {
		this->sub_130_1a16();
	}
}

void FoolGame::sub_130_172c() {
	// 130:172c
	if ((this->jumbleGameType == 4) && (this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c] == 5)) {
		this->sub_130_19da();
		return;
	}
	// 130:1776
	if (!((this->var_i16_1678 == this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]) && (this->var_i16_187e > 0))) {
		// 130:17c4
		this->sub_130_19ac();
		this->var_i16_187e = ABS(this->arr_i16_3738[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]]);
		this->var_i16_1884 = this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c];
		g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_187e]);
		this->sub_130_2178();
		return;
	}
	// 130:1844
	this->var_i16_1880 = ABS(this->arr_i16_3738[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]]);
	if (this->var_i16_187e == this->var_i16_1880) {
		this->sub_130_19ac();
		return;
	}
	// 130:189a
	g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_1880]);
	this->var_i16_1886 = this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c];
	this->var_str_1374 = g_zbasic->midStr(this->var_str_1070, this->var_i16_1884, 1);
	this->var_str_1474 = g_zbasic->midStr(this->var_str_1070, this->var_i16_1886, 1);
	g_zbasic->midStrSet(this->var_str_1070, this->var_i16_1884, 1, this->var_str_1474);
	g_zbasic->midStrSet(this->var_str_1070, this->var_i16_1886, 1, this->var_str_1374);
	this->var_i16_1574 = this->var_i16_187e;
	this->var_i16_106e = this->var_i16_1884;
	this->var_str_d12 = this->var_str_1474;
	this->sub_130_1c6c();
	this->sub_130_1dcc();
	this->var_i16_1574 = this->var_i16_1880;
	this->var_i16_106e = this->var_i16_1886;
	this->var_str_d12 = this->var_str_1374;
	this->sub_130_1c6c();
	this->sub_130_1dcc();
	this->var_i16_187e = 0;
	this->var_i16_1880 = 0;
}

void FoolGame::sub_130_19ac() {
	if (this->var_i16_187e > 0) {
		g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_187e]);
	}
	this->var_i16_187e = 0;
	this->var_i16_1880 = 0;
}

void FoolGame::sub_130_19da() {
	this->sub_130_19ac();
	this->sub_128_50e(0x19, 0x64, 0);
	for (int i = 0; i <= 0x19; i++) {
		g_toolbox->InvertRect(this->arr_rect_1f38[9]);
		this->sub_128_3da(1);
	}
}

void FoolGame::sub_130_1a16() {
	// 130:1a16
	this->var_str_1374 = this->var_str_d12;
	this->var_str_1474 = g_zbasic->midStr(this->var_str_1070, this->var_i16_106e, 1);
	if (this->var_str_1374 == this->var_str_1474)
		return;
	// 130:1a5c
	this->sub_130_1c52();
	for (int16 i = 1; i <= this->var_i16_1576; i++) {
		// 130:1a66
		this->var_str_384 = g_zbasic->midStr(this->var_str_1070, i, 1);
		if ((this->var_str_384 == this->var_str_1374) || (this->var_str_384 == this->var_str_1474)) {
			if (this->var_str_384 == this->var_str_1374) {
				this->var_str_d12 = this->var_str_1474;
			} else {
				// 130:1ad8
				this->var_str_d12 = this->var_str_1374;
			}
			// 130:1ae8
			g_zbasic->midStrSet(this->var_str_1070, i, 1, this->var_str_d12);
			this->var_i16_1574 = ABS(this->arr_i16_3738[i]);
			this->sub_130_1c6c();
			this->sub_130_1dcc();
		}
		// 130:1b2e
	}
	// 130:1b40
	if (this->var_str_1578 != g_zbasic->str(184)) { // blank
		this->var_i16_68a = 1;
		for (int i = 1; i <= this->var_i16_167a - 2; i++) {
			this->var_str_d12 = g_zbasic->midStr(this->var_str_1578, this->var_i16_68a, 1);
			if ((this->var_str_d12 == this->var_str_1374) || (this->var_str_d12 == this->var_str_1474)) {
				// 130:1ba8
				if (this->var_str_d12 == this->var_str_1374) {
					this->var_str_d12 = this->var_str_1474;
				} else {
					// 130:1bd2
					this->var_str_d12 = this->var_str_1374;
				}
				// 130:1be2
				g_zbasic->midStrSet(this->var_str_1578, this->var_i16_68a, 1, this->var_str_d12);
			}
			// 130:1c02
		}
		sub_130_2226();
	}
	// 130:1c1a
	this->sub_130_1c1a();
}

void FoolGame::sub_130_1c1a() {
	// 130:1c1a
	this->var_i16_1574 = ABS(this->arr_i16_3738[this->var_i16_106e]);
	g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_1574]);
}

void FoolGame::sub_130_1c52() {
	// 130:1c52
	g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_1574]);
}

void FoolGame::sub_130_1c6c() {
	g_toolbox->PenNormal();
	g_toolbox->FillRect(this->arr_rect_1f38[this->var_i16_1574], this->arr_pat_58f4[this->var_i16_105e]);
	g_toolbox->PenPat(this->arr_pat_58f4[this->var_i16_105c]);
	g_toolbox->FrameRect(this->arr_rect_1f38[this->var_i16_1574]);
	if (this->arr_i16_3738[this->var_i16_106e] >= 0) {
		return;
	}
	// 130:1ce2
	if ((this->activePuzzle > 0x50) && (this->activePuzzle < 0x55)) {
		return;
	}
	this->arr_i16_4758[0] = this->arr_rect_1f38[this->var_i16_1574].top + 2;
	this->arr_i16_4758[1] = this->arr_rect_1f38[this->var_i16_1574].left + 2;
	this->arr_i16_4758[2] = this->arr_rect_1f38[this->var_i16_1574].bottom - 2;
	this->arr_i16_4758[3] = this->arr_rect_1f38[this->var_i16_1574].right - 2;
	Common::Rect temp;
	temp.top = this->arr_i16_4758[0];
	temp.left = this->arr_i16_4758[1];
	temp.bottom = this->arr_i16_4758[2];
	temp.right = this->arr_i16_4758[3];
	g_toolbox->FrameRect(temp);
}

void FoolGame::sub_130_1dcc() {
	// 130:1dcc
	g_zbasic->text(0xfb, this->arr_i16_1eb8[0xe], 0, this->var_i16_106c);
	g_toolbox->MoveTo(
		this->arr_rect_1f38[this->var_i16_1574].left + this->arr_i16_1eb8[2],
		this->arr_rect_1f38[this->var_i16_1574].top + this->arr_i16_1eb8[3]
	);
	g_toolbox->DrawString(this->var_str_d12);
}

void FoolGame::sub_130_1e5c() {
	// 130:1e5c
	g_toolbox->PenNormal();
	g_toolbox->InvertRect(this->arr_rect_1f38[this->var_i16_1574]);
	this->sub_130_1e76();
}

void FoolGame::sub_130_1e76() {
	// 130:1e76
	this->arr_i16_4758[0] = this->arr_rect_1f38[this->var_i16_1574].top + 3;
	this->arr_i16_4758[1] = this->arr_rect_1f38[this->var_i16_1574].left + 3;
	this->arr_i16_4758[2] = this->arr_rect_1f38[this->var_i16_1574].bottom - 3;
	this->arr_i16_4758[3] = this->arr_rect_1f38[this->var_i16_1574].right - 3;
	Common::Rect temp;
	temp.top = this->arr_i16_4758[0];
	temp.left = this->arr_i16_4758[1];
	temp.bottom = this->arr_i16_4758[2];
	temp.right = this->arr_i16_4758[3];
	g_toolbox->FillRect(temp, this->arr_pat_58f4[this->var_i16_105e]);
	g_zbasic->text(0xfb, this->arr_i16_1eb8[0xe], 0, this->var_i16_106c);
	// 130:1f70
	g_toolbox->MoveTo(
		this->arr_rect_1f38[this->var_i16_1574].left + this->arr_i16_1eb8[2],
		this->arr_rect_1f38[this->var_i16_1574].top + this->arr_i16_1eb8[3]
	);
	g_toolbox->DrawString(this->var_str_d12);
	if (this->arr_i16_3738[this->var_i16_106e] < 0)
		return;
	// 130:1ff4
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	g_toolbox->PenMode(kPatBic);
	g_toolbox->PaintRect(temp);
}

void FoolGame::sub_130_201a() {
	// 130:201a
	g_zbasic->midStrSet(this->var_str_1070, this->var_i16_106e, 1, this->var_str_d12);
	if ((this->activePuzzle > 0x50) && (this->activePuzzle < 0x55)) {
		this->sub_130_1e5c();
	} else {
		// 130:2066
		this->sub_130_1c6c();
		this->sub_130_1dcc();
	}
	// 130:206e
	this->sub_130_2078();
	this->sub_130_20d2();
}

void FoolGame::sub_130_2078() {
	// 130:2078
	this->var_i16_106e++;
	if (this->var_i16_106e > this->var_i16_1576) {
		this->var_i16_106e = 1;
	}
}

void FoolGame::sub_130_2094() {
	// 130:2094
	this->var_i16_106e--;
	if (this->var_i16_106e < 1) {
		this->var_i16_106e = this->var_i16_1576;
	}
}

void FoolGame::sub_130_20aa() {
	// 130:20aa
	this->sub_130_1c52();
	this->var_i16_106e = this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c];
	this->sub_130_20d2();
}


void FoolGame::sub_130_20d2() {
	// 130:20d2
	this->var_i16_1574 = ABS(this->arr_i16_3738[this->var_i16_106e]);
	this->sub_130_1c52();
	this->sub_130_20fe();
	this->sub_130_2178();
}

void FoolGame::sub_130_20fe() {
	// 130:20fe
	this->var_i16_1888 = ABS(this->arr_i16_3738[this->var_i16_106e]) - 1;
	this->var_i16_68a = (this->var_i16_1888 % this->arr_i16_1eb8[0]) + 1;
	this->var_i16_68c = (this->var_i16_1888 / this->arr_i16_1eb8[0]) + 1;
}

void FoolGame::sub_130_2178() {
	// 130:2178
	if ((this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] == this->var_i16_1678) || (this->jumbleGameType == 2))
		return;

	this->var_i16_1678 = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
	if (g_zbasic->index(1, this->var_i16_1678) != g_zbasic->str(185)) { // empty
		this->var_str_1578 = g_zbasic->index(1, this->var_i16_1678);
		this->sub_130_2226();
	}
	// 130:2224
}

void FoolGame::sub_130_2226() {
	// 130:2226
	if (this->activePuzzle == 0x14) {
		this->sub_130_23cc();
		return;
	}
	if (this->activePuzzle == 0x3c) {
		this->sub_130_22ee();
		return;
	}
	if (this->activePuzzle == 0x35) {
		this->sub_130_24aa();
		return;
	}
	if (this->activePuzzle == 0x55) {
		this->sub_130_2548();
		return;
	}
	// 130:2266
	this->fillRect(0x124, 0x64, 0x139, 0x1ef, 0);
	if (this->var_str_1578 == g_zbasic->str(186)) // blank
		return;
	// 130:22a0
	g_zbasic->text(0xfa, 0xc, 0, kSrcOr);
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_1578);
	g_toolbox->MoveTo(0x127 - (this->var_i16_484 / 2), 0x133);
	g_toolbox->DrawString(this->var_str_1578);
}

void FoolGame::sub_130_22ee() {
	// 130:22ee
	this->fillRect(0x131, 0x84, 0x156, 0x17e, this->var_i16_1060);
	if (this->var_str_1578 == g_zbasic->str(187)) // blank
		return;
	g_zbasic->text(0xfa, 0xc, 0, kSrcBic);
	this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_1578);
	this->fillRect(0x13c, 0xf6 - (this->var_i16_7ba / 2), 0x150, 0x10a + (this->var_i16_7ba / 2), 2);
	g_toolbox->MoveTo(0x100 - (this->var_i16_7ba / 2), 0x14a);
	g_toolbox->DrawString(this->var_str_1578);
}

void FoolGame::sub_130_23cc() {
	// 130:23cc
	this->fillRect(0x12c, 0x8c, 0x156, 0x186, this->var_i16_1060);
	if (this->var_str_1578 == g_zbasic->str(188))
		return;
	g_zbasic->text(0xfa, 0xc, 0, kSrcBic);
	this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_1578);
	this->fillRect(0x130, 0x100 - (this->var_i16_7ba / 2), 0x144, 0x114 + (this->var_i16_7ba / 2), 2);
	g_toolbox->MoveTo(0x10a - (this->var_i16_7ba / 2), 0x13e);
	g_toolbox->DrawString(this->var_str_1578);
}

void FoolGame::sub_130_24aa() {
	// 130:24aa
	this->fillRect(0x125, 0x20, 0x136, 0x15e, 2);
	if (this->var_str_1578 == g_zbasic->str(189)) // blank
		return;
	g_zbasic->text(0xfa, 0xc, 0, kSrcBic);
	this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_1578);
	g_toolbox->MoveTo(0xc4 - (this->var_i16_7ba / 2), 0x131);
	g_toolbox->DrawString(this->var_str_1578);
}

void FoolGame::sub_130_2548() {
	// 130:2548
	g_zbasic->text(0xfa, 0xc, 0, kSrcOr);
	this->fillRect(0x131, 0, 0x156, 0x10e, 0x47);
	if (this->var_str_1578 == g_zbasic->str(190)) // blank
		return;

	this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_1578);
	this->fillRect(0x13e, 0xc, 0x14e, 0x16 + this->var_i16_7ba, 0);
	g_toolbox->MoveTo(0x16, 0x14a);
	g_toolbox->DrawString(this->var_str_1578);
}

void FoolGame::sub_130_25d8() {
	// 130:25d8
	g_zbasic->unk_20();
	this->var_i16_188a = this->var_str_384.size();
	this->var_str_9f4 = g_zbasic->space(this->var_i16_188a);
	for (int16 i = 1; i <= this->var_i16_188a; i++) {
		this->arr_i16_5bbc[i] = i;
	}
	this->var_i16_188c = 0;
	for (int16 i = this->var_i16_188a; i >= 1; i--) {
		do {
			this->var_i16_7e4 = g_zbasic->rndInt(i);
			// 130:2640
			if (i == 1) {
				// 130:266e
				if (this->arr_i16_5bbc[this->var_i16_7e4] == i) {
					this->var_i16_188c = 1;
				}
				break;
			}
			// 130:264c
		} while (this->arr_i16_5bbc[this->var_i16_7e4] == i);
		// 130:2690
		this->arr_i16_5cbc[i] = this->arr_i16_5bbc[this->var_i16_7e4];
		for (int16 j = this->var_i16_7e4; j <= i; j++) {
			this->arr_i16_5bbc[j] = this->arr_i16_5bbc[j+1];
		}
		// 130:26f2
	}
	// 130:2712
	if (this->var_i16_188c != 0) {
		g_zbasic->swapInt(this->arr_i16_5cbc[1], this->arr_i16_5cbc[2]);
	}
	// 130:273c
	for (int16 i = 1; i <= this->var_i16_188a; i++) {
		g_zbasic->midStrSet(this->var_str_9f4, i, 1, g_zbasic->midStr(this->var_str_384, this->arr_i16_5cbc[i], 1));
	}
}

void FoolGame::sub_130_2790() {
	g_zbasic->unk_20();
	for (int16 i = 1; i <= 0x1a; i++) {
		this->arr_i16_5bbc[i] = i;
	}
	this->var_i16_188c = 0;
	for (int16 i = 0x1a; i > 0; i--) {
		// 130:27c4
		do {
			this->var_i16_7e4 = g_zbasic->rndInt(i);
			// 130:27d2
			if (i == 1) {
				// 130:2800
				if (this->arr_i16_5bbc[this->var_i16_7e4] == i) {
					// 130:281c
					this->var_i16_188c = 1;
				}
				break;
			}
			// 130:27de
		} while (this->arr_i16_5bbc[this->var_i16_7e4] == i);
		// 130:2822
		this->arr_i16_5cbc[i] = this->arr_i16_5bbc[this->var_i16_7e4];
		for (int16 j = this->var_i16_7e4; j <= i; j++) {
			this->arr_i16_5bbc[j] = this->arr_i16_5bbc[j+1];
		}
	}
	// 130:28a4
	if (this->var_i16_188c != 0) {
		g_zbasic->swapInt(this->arr_i16_5cbc[1], this->arr_i16_5cbc[2]);
	}
	this->var_i16_188a = this->var_str_384.size();
	this->var_str_9f4 = g_zbasic->space(this->var_i16_188a);
	for (int16 i = 1; i <= this->var_i16_188a; i++) {
		this->var_i16_7e4 = g_zbasic->asc(g_zbasic->midStr(this->var_str_384, i, 1)) - 0x40;
		g_zbasic->midStrSet(this->var_str_9f4, i, 1, g_zbasic->chr(this->arr_i16_5cbc[this->var_i16_7e4]+0x40));
	}
}


}
