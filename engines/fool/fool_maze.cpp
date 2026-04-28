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

void FoolGame::mazeRun() {
	// 136:0004
	sub_128_271a();
	sub_128_4da(0);
	for (int16 i = 0; i <= 0x17; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	var_i16_484 = 0;
	do {
		var_i16_68c = arr_i16_1eb8[8];
		do {
			var_i16_68a = arr_i16_1eb8[10];
			var_i16_484++;
			g_toolbox->SetRect(
				arr_rect_1f38[var_i16_484],
				var_i16_68a,
				var_i16_68c,
				arr_i16_1eb8[13],
				arr_i16_1eb8[12]
			);
			// 136:00ca
		} while (g_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[11], arr_i16_1eb8[6]));
	} while (g_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	if (this->arr_i16_1eb8[20] != 0) {
		var_i32_1ac0 = g_toolbox->GetPicture(arr_i16_1eb8[20]);
		g_zbasic->picture(arr_i16_1eb8[18], arr_i16_1eb8[19], var_i32_1ac0);
	}
	// 136:018c
	for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {
		g_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		arr_i16_5cbc[i] = 0;
	}
	// 136:01d2
	var_i16_484 = 0;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_484++;
			arr_i16_3738[var_i16_484] = puzzlesReadByte();
			arr_i16_2f38[i*32 + j] = var_i16_484;
			arr_i16_4338[var_i16_484] = 0;
		}
	}
	// 136:026a
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			arr_i16_3b38[i*32 + j] = puzzlesReadByte();
		}
	}
	// 136:02ce
	sub_128_bde(1, arr_i16_1eb8[17], 0, 1, 0, 1);

	Common::Rect playArea;
	playArea.top = arr_rect_1f38[1].top + 3;
	playArea.left = arr_rect_1f38[0].top + 3;
	playArea.bottom = arr_rect_1f38[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].bottom - 3;
	playArea.right = arr_rect_1f38[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].right - 3;
	g_toolbox->FillRect(playArea, arr_pat_58f4[2]);
	g_toolbox->PenNormal();
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
	// 136:042c
	var_i16_1ac4 = arr_i16_1eb8[1];
	var_i16_1574 = 0;
	var_i16_68c = 1;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_1574++;
			if ((arr_i16_3738[var_i16_1574] & 0x10) != 0) {
				sub_136_2a7c();
			}
			if ((arr_i16_3738[var_i16_1574] & 0x20) != 0) {
				sub_136_2b30();
			}
			if ((arr_i16_3738[var_i16_1574] & 0x40) != 0) {
				sub_136_2be2();
			}
			// 136:04b6
			if ((arr_i16_3738[var_i16_1574] & 0x80) != 0) {
				sub_136_2c96();
			}
		}
		if ((arr_i16_3738[var_i16_1574 - 1] & 4) == 0) {
			Common::Rect temp;
			temp.top = arr_rect_1f38[var_i16_1574].top + 3;
			temp.left = arr_rect_1f38[var_i16_1574].left - 3;
			temp.bottom = arr_rect_1f38[var_i16_1574].bottom - 3;
			temp.right = SCREEN_WIDTH;
			g_toolbox->FillRect(temp, arr_pat_58f4[2]);
		}
		// 136:05d0
	}
	// 136:05ec
	if (activePuzzleBuffer.empty()) { // was: str(223)
		for (int16 j = 1; j <= arr_i16_1eb8[23]; j++) {
			var_str_1ac8 = g_zbasic->indexRaw(1, j);
			var_i16_1bc8 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ac8, 3, 2));
			if (var_i16_1bc8 != 0x29a) {
				for (int16 i = 1; i <= var_i16_1bc8; i++) {
					sub_136_2582();
				}
			}
			// 136:0664
		}
		// 136:067e
		var_i16_1bcc = arr_i16_1eb8[15];
		var_i16_1bce = arr_i16_1eb8[16];
		var_i16_1bd0 = 0;
	} else {
		// 136:06b0
		sub_136_3466();
	}
	// 136:06b4
	var_str_1578.clear(); // was str(224)
	sub_136_274e();
	sub_136_ade();
	stateFlags = 0;
	sub_128_4da(1);
	if (var_i16_1bcc == 0x19) {
		var_i16_d0c = 1;
	} else {
		// 136:06f0
		var_i16_d0c = 0;
	}
	// 136:06f6
	// JMP 0x75c
	while (((stateFlags & kStateReturn) == 0) && (var_i16_d0c == 0)) {
		// 136:06fa
		while ((stateFlags == kStateNull) && (var_i16_d0c == 0)) {
			// 136:06fe
			getNextEvent(-1);
			if ((var_ev_46.modifiers & kModMouseButtonUp) == 0) {
				sub_136_79e();
			}
		}
		// 136:074a
		if (stateFlags == kStateSaveGame) {
			sub_136_2f5c();
			saveGame();
		}
		// 136:075c
	}
	// 136:0786
	if (var_i16_d0c != 0) {
		sub_136_3a70();
	}
	sub_136_2f5c();
}

void FoolGame::sub_136_79e() {
	// 136:079e
	sub_128_2be(var_i16_68a, var_i16_68c);
	if ((var_i16_68a == var_i16_1bcc) && (var_i16_68c == var_i16_1bce)) {
		return;
	}
	// 136:07e2
	if (var_i16_1bce > var_i16_68c) {
		var_i16_1bd2 = 2;
	}
	if (var_i16_1bcc > var_i16_68a) {
		var_i16_1bd2 = 1;
	}
	if (var_i16_1bce < var_i16_68c) {
		var_i16_1bd2 = 4;
	}
	if (var_i16_1bcc < var_i16_68a) {
		var_i16_1bd2 = 3;
	}
	if (var_i16_1bd4 != var_i16_1bd2) {
		if ((var_i16_1bd2 == 1) && ((arr_i16_3738[var_i16_1574] & 1) == 1)) {
			sub_136_994();
			return;
		}
		// 136:088a
		if ((var_i16_1bd2 == 2) && ((arr_i16_3738[var_i16_1574] & 2) == 2)) {
			sub_136_994();
			return;
		}
		if ((var_i16_1bd2 == 3) && ((arr_i16_3738[var_i16_1574] & 4) == 4)) {
			sub_136_994();
			return;
		}
		if ((var_i16_1bd2 == 4) && ((arr_i16_3738[var_i16_1574] & 8) == 8)) {
			sub_136_994();
			return;
		}
		// 136:0944
		if (var_i16_1bd6 == var_i16_1bd2) {
			return;
		}
	}
	// 136:0956
	if (var_i16_1bd2 == 1)
		var_i16_1bcc--;
	if (var_i16_1bd2 == 2)
		var_i16_1bce--;
	if (var_i16_1bd2 == 3)
		var_i16_1bcc++;
	if (var_i16_1bd2 == 4)
		var_i16_1bce++;
	sub_136_a22();
}

void FoolGame::sub_136_994() {
	// 136:0994
	g_toolbox->PenNormal();
	g_toolbox->PenSize(5, 5);
	if (arr_i16_1eb8[21] == 0) {
		g_toolbox->PenMode(kPatXor);
		switch (var_i16_1bd2-1) {
		case 0:
			sub_136_2a7c();
			break;
		case 1:
			sub_136_2b30();
			break;
		case 2:
			sub_136_2be2();
			break;
		case 3:
			sub_136_2c96();
			break;
		default:
			warning("sub_136_994: breaking out of switch");
			break;
		}
	} else {
		// 136:09de
		g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
	}
	// 136:0a00
	switch (var_i16_1bd2-1) {
	case 0:
		sub_136_2a7c();
		break;
	case 1:
		sub_136_2b30();
		break;
	case 2:
		sub_136_2be2();
		break;
	case 3:
		sub_136_2c96();
		break;
	default:
		warning("sub_136_994: breaking out of switch");
		break;
	}
}

void FoolGame::sub_136_a22() {
	// 136:0a22
	if (var_i16_1bd6 != 0) {
		g_toolbox->PenNormal();
		g_toolbox->PenPat(arr_pat_58f4[2]);
		g_toolbox->PenSize(5, 5);
		switch (var_i16_1bd6 - 1) {
		case 0:
			sub_136_2d4c();
			break;
		case 1:
			sub_136_2dd0();
			break;
		case 2:
			sub_136_2e52();
			break;
		case 3:
			sub_136_2ed6();
			break;
		default:
			warning("sub_136_a22: breaking out of switch");
			break;
		}
	}
	// 136:0a66
	if (var_i16_1bd4 != 0) {
		g_toolbox->PenNormal();
		g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
		g_toolbox->PenSize(5, 5);
		switch (var_i16_1bd6 - 1) {
		case 0:
			sub_136_2d4c();
			break;
		case 1:
			sub_136_2dd0();
			break;
		case 2:
			sub_136_2e52();
			break;
		case 3:
			sub_136_2ed6();
			break;
		default:
			warning("sub_136_a22: breaking out of switch");
			break;
		}
	}
	// 136:0abe
	do {
		var_i16_1bd6 = 0;
		var_i16_1bd4 = 0;
		var_i16_1bd8 = 0;
		Common::Rect temp;
		temp.top = arr_i16_1eb8[12];
		temp.left = arr_i16_1eb8[13];
		temp.bottom = arr_i16_1eb8[14];
		temp.right = arr_i16_1eb8[15];
		g_toolbox->InvertOval(temp);
		var_i16_1bda = var_i16_1574;
		sub_136_b00();
	} while (var_i16_1bd8 != var_i16_1574);
}


void FoolGame::sub_136_ade() {
	// 136:0ade

	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_b00() {
	// 136:0b00
	if (arr_i16_1eb8[11] | var_i16_1bdc) {
		fillRect(0x136, 0, SCREEN_HEIGHT, SCREEN_WIDTH, var_i16_1ac4);
		var_i16_1bdc = 0;
		var_str_1578.clear(); // was: str(225)
	}
	// 136:0b5e
	if ((arr_i16_3738[var_i16_1574] & 0x1000) == 0) {
		var_i16_1bd0 = var_i16_1574;
		return;
	}
	var_str_1ac8 = g_zbasic->index(1, arr_i16_3b38[var_i16_1bcc*32 + var_i16_1bce]);
	var_str_1bde = g_zbasic->str(226); // CILTDABRPXMOZSFQGWEJKV

	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_2582() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_274e() {
	// 136:274e
	g_zbasic->menu(8, 0, 1, arr_str_195e8[activePuzzle]);
	if (activePuzzle < 0x50) {
		g_zbasic->menu(8, 1, 1, g_zbasic->str(243)); // return to scroll
	} else {
		// 136:279a
		g_zbasic->menu(8, 1, 1, g_zbasic->str(244)); // return to map
	}
	// 136:27b2
	g_zbasic->menu(8, 2, 0, g_zbasic->str(245)); // -
	g_zbasic->menu(8, 3, 1, g_zbasic->str(246)); // to escape from the maze
	g_zbasic->menu(8, 4, 1, g_zbasic->str(247)); // use the mouse to maneuver
	g_zbasic->menu(8, 5, 1, g_zbasic->str(248)); // the white dot to the exit
	if (activePuzzle == 0x57) {
		// 136:281c
		var_i16_103a = 7;
		var_i16_68a = 1;
		for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {

			if ((arr_i16_5cbc[var_i16_68a] > 0) &&
				(g_zbasic->instr(1, g_zbasic->index(1, var_i16_68a), g_zbasic->str(249)) > 0)) {
				if (var_i16_103a == 7) {
					g_zbasic->menu(8, 6, 0, g_zbasic->str(250)); // -
					g_zbasic->menu(8, 7, 0, g_zbasic->str(251)); // you have acquired
				}
				// 136:28b4
				var_i16_1dea = g_zbasic->instr(1, g_zbasic->index(1, var_i16_68a), g_zbasic->str(252)); // ++
				var_i16_1de8 = g_zbasic->instr(1, g_zbasic->index(1, var_i16_68a), g_zbasic->str(253)); // \xa5\xa5
				var_str_1e08 = g_zbasic->midStr(g_zbasic->index(1, var_i16_68a), var_i16_1dea + 2, var_i16_1de8 - var_i16_1dea - 2);
				if (g_zbasic->leftStr(var_str_1e08, 1) == g_zbasic->str(254)) { // +
					// 136:2964
					var_i16_484 = var_str_1e08.size();
					var_str_1e08 = g_zbasic->rightStr(var_str_1e08, var_i16_484 - 1);
				}
				// 136:298e
				var_str_384 = Common::U32String::format(" %d", arr_i16_5cbc[var_i16_68a]);
				var_str_384 = g_zbasic->space(4 - var_str_384.size()) + var_str_384 + g_zbasic->str(255) + var_str_1e08; // ' '
				if (arr_i16_5cbc[var_i16_68a] > 1) {
					var_str_384 += g_zbasic->str(256); // s
				}
				// 136:2a2a
				var_str_384 += g_zbasic->str(257); // '  '
				var_i16_103a++;
				g_zbasic->menu(8, var_i16_103a, 1, var_str_384);
			}
			// 136:2a5e
		}
	}
	// 136:2a7a
	return;
}

void FoolGame::sub_136_2a7c() {
	// 136:2a7c
	arr_i16_3738[var_i16_1574] |= 0x100;
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1574].left - 2,
		arr_rect_1f38[var_i16_1574].top - 2
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1574].left - 2,
		arr_rect_1f38[var_i16_1574].bottom - 3
	);
}

void FoolGame::sub_136_2b30() {
	// 136:2b30
	arr_i16_3738[var_i16_1574] |= 0x200;
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1574].left - 2,
		arr_rect_1f38[var_i16_1574].top - 2
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1574].right - 3,
		arr_rect_1f38[var_i16_1574].top - 2
	);
}

void FoolGame::sub_136_2be2() {
	// 136:2be2
	arr_i16_3738[var_i16_1574] |= 0x400;
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1574].right - 3,
		arr_rect_1f38[var_i16_1574].top - 2
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1574].right - 3,
		arr_rect_1f38[var_i16_1574].bottom - 3
	);
}

void FoolGame::sub_136_2c96() {
	// 136:2c96
	arr_i16_3738[var_i16_1574] |= 0x800;
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1574].left - 2,
		arr_rect_1f38[var_i16_1574].bottom - 3
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1574].right - 3,
		arr_rect_1f38[var_i16_1574].bottom - 3
	);
}

void FoolGame::sub_136_2d4c() {
	// 136:2d4c
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1bd8].left - 2,
		arr_rect_1f38[var_i16_1bd8].top + 3
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1bd8].left - 2,
		arr_rect_1f38[var_i16_1bd8].bottom - 8
	);
}

void FoolGame::sub_136_2dd0() {
	// 136:2dd0
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1bd8].left + 3,
		arr_rect_1f38[var_i16_1bd8].top - 2
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1bd8].right - 8,
		arr_rect_1f38[var_i16_1bd8].top - 2
	);
}

void FoolGame::sub_136_2e52() {
	// 136:2e52
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1bd8].right - 3,
		arr_rect_1f38[var_i16_1bd8].top + 3
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1bd8].right - 3,
		arr_rect_1f38[var_i16_1bd8].bottom - 8
	);
}

void FoolGame::sub_136_2ed6() {
	// 136:2ed6
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1bd8].left + 3,
		arr_rect_1f38[var_i16_1bd8].bottom - 3
	);
	g_toolbox->LineTo(
		arr_rect_1f38[var_i16_1bd8].right - 8,
		arr_rect_1f38[var_i16_1bd8].bottom - 3
	);
}

void FoolGame::sub_136_2f5c() {
	// 136:2f5c
	if ((activePuzzle != 0x57) && (var_i16_d0c == 0)) {
		activePuzzleBuffer.clear(); // was: str(258)
		return;
	}
	activePuzzleBuffer = g_zbasic->encodeInt(var_i16_1bcc) +
		g_zbasic->encodeInt(var_i16_1bce) +
		g_zbasic->encodeInt(var_i16_1bd0) +
		g_zbasic->encodeInt(var_i16_1bd6) +
		g_zbasic->encodeInt(var_i16_1bd4) +
		g_zbasic->encodeInt(var_i16_1bd8);
	// 136:300a
	for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {
		activePuzzleBuffer += g_zbasic->encodeInt(arr_i16_5cbc[i]);
	}
	// 136:3054
	var_i16_484 = 0;
	Common::String buffer;
	var_i16_1de6 = 0;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_484++;
			if (arr_i16_4338[var_i16_484] > 0) {
				var_i16_1de6++;
				buffer += g_zbasic->encodeInt(arr_i16_2f38[i*32 + j]) + g_zbasic->encodeInt(arr_i16_4338[var_i16_484]);
			}
			// 136:30f0
		}
	}
	// 136:3128
	activePuzzleBuffer += g_zbasic->encodeInt(var_i16_1de6) + buffer;

	var_i16_484 = 0;
	var_i16_1de6 = 0;
	var_i16_1f08 = 0;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_484++;
			if (arr_i16_1eb8[21] != 0) {
				// 136:3188
				if (((arr_i16_3738[var_i16_484] & 1) == 1) && ((arr_i16_3738[var_i16_484] & 0x10) == 0)) {
					if ((arr_i16_3738[var_i16_484] & 0x100) != 0) {
						sub_136_3408();
					}
					sub_136_342a();
				}
				// 136:31fe
				if (((arr_i16_3738[var_i16_484] & 2) == 2) && ((arr_i16_3738[var_i16_484] & 0x20) == 0)) {
					if ((arr_i16_3738[var_i16_484] & 0x200) != 0) {
						sub_136_3408();
					}
					sub_136_342a();
				}
				// 136:3274
				if (((arr_i16_3738[var_i16_484] & 4) == 4) && ((arr_i16_3738[var_i16_484] & 0x40) == 0)) {
					if ((arr_i16_3738[var_i16_484] & 0x400) != 0) {
						sub_136_3408();
					}
					sub_136_342a();
				}
				// 136:32ea
				if (((arr_i16_3738[var_i16_484] & 8) == 8) && ((arr_i16_3738[var_i16_484] & 0x80) == 0)) {
					if ((arr_i16_3738[var_i16_484] & 0x800) != 0) {
						sub_136_3408();
					}
					sub_136_342a();
				}
			}
			// 136:3362
			if (arr_i16_3b38[i*32 + j] > 0) {
				if ((arr_i16_3738[var_i16_484] & 0x1000) != 0) {
					sub_136_3408();
				}
				sub_136_342a();
			}
			// 136:33b0
		}
	}
	// 136:33e8
	activePuzzleBuffer += g_zbasic->encodeInt(var_i16_1f08);
}

void FoolGame::sub_136_3408() {
	// 136:3408
	var_i16_1f08 |= bitLUT[var_i16_1de6];
}

void FoolGame::sub_136_342a() {
	// 136:342a
	var_i16_1de6++;
	if (var_i16_1de6 > 0xf) {
		activePuzzleBuffer += g_zbasic->encodeInt(var_i16_1f08);
		var_i16_1f08 = 0;
		var_i16_1de6 = 0;
	}
	// 136:3464
}

void FoolGame::sub_136_3466() {
	// 136:3466
	var_i16_1bcc = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 1, 2));
	var_i16_1bce = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 3, 2));
	var_i16_1bd0 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 5, 2));
	var_i16_1bd6 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 7, 2));
	var_i16_1bd4 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 9, 2));
	var_i16_1bd8 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, 0xb, 2));
	if (var_i16_1bd6 > 0) {
		// 136:3500
		g_toolbox->PenNormal();
		g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
		g_toolbox->PenSize(5, 5);
		switch (var_i16_1bd6 - 1) {
		case 0:
			sub_136_2d4c();
			break;
		case 1:
			sub_136_2dd0();
			break;
		case 2:
			sub_136_2e52();
			break;
		case 3:
			sub_136_2ed6();
			break;
		default:
			warning("sub_136_3466: breaking out of switch");
			break;
		}
	}
	// 136:354e
	if (var_i16_1bd4 > 0) {
		g_toolbox->PenNormal();
		g_toolbox->PenPat(arr_pat_58f4[2]);
		g_toolbox->PenSize(5, 5);
		switch (var_i16_1bd4 - 1) {
		case 0:
			sub_136_2d4c();
			break;
		case 1:
			sub_136_2dd0();
			break;
		case 2:
			sub_136_2e52();
			break;
		case 3:
			sub_136_2ed6();
			break;
		default:
			warning("sub_136_3466: breaking out of switch");
			break;
		}
	}
	// 136:3592
	var_i16_1f0a = 0xd;
	var_i16_68a = 1;
	arr_i16_5cbc[var_i16_68a] = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, var_i16_1f0a, 2));
	var_i16_1f0a += 2;
	if (var_i16_1de4 > 0) {
		g_zbasic->text(0xfb, 0xc, 0, kSrcBic);
		for (int16 i = 1; i <= var_i16_1de4; i++) {
			var_i16_1dea = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, var_i16_1f0a, 2));
			var_i16_1f0a += 2;
			var_i16_106e = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, var_i16_1f0a, 2));
			var_i16_1f0a += 2;
			// 136:366e
			g_toolbox->MoveTo(
				arr_rect_1f38[var_i16_1dea].left + arr_i16_1eb8[2],
				arr_rect_1f38[var_i16_1dea].top + arr_i16_1eb8[3]
			);
			g_zbasic->unk_8();
			// the one place that used ZBasic's print API,
			// easier to cut out the middleman and use DrawString
			g_toolbox->DrawString(g_zbasic->chr(var_i16_106e));
			//g_zbasic->print(g_zbasic->chr(var_i16_106e));
			//g_zbasic->printNewline();
			arr_i16_4338[var_i16_1dea] = var_i16_106e;
		}
	}
	// 136:3710
	g_toolbox->PenNormal();
	g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[16]]);
	g_toolbox->PenSize(5, 5);
	var_i16_1574 = 0;
	var_i16_1de6 = 0;
	var_i16_1f08 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, var_i16_1f0a, 2));
	// 136:3766
	var_i16_1f0a += 2;
	var_i16_68c = 1;
	var_i16_68a = 1;

	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_3a70() {
	// 136:3a70
	if (activePuzzle == 0x57) {
		var_i16_c04 = 0x64;
		sub_128_2664();
		sub_128_61c2();
	} else {
		// 136:3a8e
		if (var_i16_c04 < 0x64) {
			var_i16_c04 = 0x64;
		}
		var_i16_484 = 1 + (arr_i16_1eb8[24] - 0x14) / 0x23;
		var_i16_7e4 = 1 + (arr_i16_1eb8[25]) / 0x23;
		var_i16_9f2 = 1 + (SCREEN_HEIGHT - arr_i16_1eb8[26]) / 0x23;
		g_toolbox->PenNormal();
		// 136:3b0e
		for (int16 j = 0; j <= 1; j++) {
			if (j == 1) {
				g_toolbox->PenMode(kNotPatOr);
				g_toolbox->PenPat(arr_pat_58f4[1]);
			}
			// 136:3b32
			for (int16 i = 4; i <= 7; i++) {
				arr_i16_4758[i] = arr_i16_1eb8[i + 0x14];
			}
			// 136:3b7a
			for (int16 i = 0; i <= 0x23; i++) {
				arr_i16_4758[4] -= var_i16_484;
				arr_i16_4758[5] -= var_i16_7e4;
				arr_i16_4758[6] += var_i16_9f2;
				arr_i16_4758[7] = SCREEN_WIDTH;
				Common::Rect temp;
				temp.top = arr_i16_4758[4];
				temp.left = arr_i16_4758[5];
				temp.bottom = arr_i16_4758[6];
				temp.right = arr_i16_4758[7];
				if (arr_i16_4758[4] < 0x14) {
					arr_i16_4758[4] = 0x14;
				}

				if (var_i16_68c == 0) {
					g_toolbox->InvertRect(temp);
				} else {
					// 136:3c48
					g_toolbox->PaintRect(temp);
				}
				// 136:3c56
			}
			// 136:3c64
		}
	}
	// 136:3c72
}

}
