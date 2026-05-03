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
	var_i16_68c = arr_i16_1eb8[8];
	do {
		var_i16_68a = arr_i16_1eb8[10];
		do {
			var_i16_484++;
			g_toolbox->SetRect(
				arr_rect_1f38[var_i16_484],
				var_i16_68a,
				var_i16_68c,
				var_i16_68a + arr_i16_1eb8[13],
				var_i16_68c + arr_i16_1eb8[12]
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
	playArea.left = arr_rect_1f38[1].left + 3;
	playArea.bottom = arr_rect_1f38[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].bottom - 3;
	playArea.right = arr_rect_1f38[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].right - 3;
	g_toolbox->FillRect(playArea, arr_pat_58f4[2]);
	g_toolbox->PenNormal();
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
	// 136:042c
	var_i16_1ac4 = arr_i16_1eb8[17];
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
			if (var_i16_1bc8 != 666) {
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
	stateFlags = kStateNull;
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
				mazeOnClick();
			}
			if (var_i16_1bcc == 0x19) {
				var_i16_d0c = 1;
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

void FoolGame::mazeOnClick() {
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
		temp.top = arr_i16_1eb8[24];
		temp.left = arr_i16_1eb8[25];
		temp.bottom = arr_i16_1eb8[26];
		temp.right = arr_i16_1eb8[27];
		g_toolbox->InvertOval(temp); // erase current spot
		mazeDrawPlayer();
		var_i16_1bda = var_i16_1574;
		sub_136_b00();
	} while (var_i16_1bda != var_i16_1574);
}


void FoolGame::sub_136_ade() {
	// 136:0ade
	// this was done by a GOTO into the previous function, so we copy most of it here.
	mazeDrawPlayer();
	var_i16_1bda = var_i16_1574;
	sub_136_b00();
	while (var_i16_1bda != var_i16_1574) {
		var_i16_1bd6 = 0;
		var_i16_1bd4 = 0;
		var_i16_1bd8 = 0;
		Common::Rect temp;
		temp.top = arr_i16_1eb8[24];
		temp.left = arr_i16_1eb8[25];
		temp.bottom = arr_i16_1eb8[26];
		temp.right = arr_i16_1eb8[27];
		g_toolbox->InvertOval(temp);
		mazeDrawPlayer();
		var_i16_1bda = var_i16_1574;
		sub_136_b00();
	};
}

void FoolGame::sub_136_b00() {
	warning(__func__);
	// 136:0b00
	if (arr_i16_1eb8[22] | var_i16_1bdc) {
		fillRect(0x136, 0, SCREEN_HEIGHT, SCREEN_WIDTH, var_i16_1ac4);
		var_i16_1bdc = 0;
		var_str_1578.clear(); // was: str(225)
	}
	// 136:0b5e
	if ((arr_i16_3738[var_i16_1574] & 0x1000) == 0) {
		var_i16_1bd0 = var_i16_1574;
		return;
	}
	var_str_1ac8 = g_zbasic->indexRaw(1, arr_i16_3b38[var_i16_1bcc*32 + var_i16_1bce]);
	var_str_1bde = g_zbasic->strRaw(226); // CILTDABRPXMOZSFQGWEJKV
	// 136:0bd0
	var_i16_1cde = 1;
	g_zbasic->unk_20();
	while (var_i16_1cde > 0) {
		// 136:0bde
		var_i16_1cde = g_zbasic->instr(var_i16_1cde, var_str_1ac8, g_zbasic->strRaw(227)); // \xa5\xa5
		if (var_i16_1cde > 0) {
			var_i16_484 = 1;
			for (int16 i = 1; i <= (int16)var_str_1bde.size(); i++) {
				if (g_zbasic->midStr(var_str_1ac8, var_i16_1cde + 2, 1) == g_zbasic->midStr(var_str_1bde, i, 1)) {
					var_i16_1a9a = i;
				}
				// 136:0c4c
			}
			// 136:0c5e
			var_i16_1ce0 = g_zbasic->instr(var_i16_1cde, var_str_1ac8, g_zbasic->strRaw(228)); // <<
			var_str_1ce2 = g_zbasic->midStr(var_str_1ac8, var_i16_1cde + 3, var_i16_1ce0 - var_i16_1cde - 3);
			if (g_zbasic->midStr(var_str_1ac8, var_i16_1ce0 + 2, 2) == g_zbasic->strRaw(229)) {
				// 136:0cdc
				var_i16_1cde = 0;
			} else {
				var_i16_1cde = var_i16_1ce0;
			}
			// 136:0cec
			switch (var_i16_1a9a - 1) {
			case 0:
				mazePrintMessage();
				break;
			case 1:
				sub_136_e4c();
				break;
			case 2:
				sub_136_ed8();
				break;
			case 3:
				mazeWanderingWinds();
				break;
			case 4:
				sub_136_115a();
				break;
			case 5:
				sub_136_137c();
				break;
			case 6:
				sub_136_163c();
				break;
			case 7:
				sub_136_1650();
				break;
			case 8:
				mazeHiddenDoorOpen();
				break;
			case 9:
				mazeHiddenDoorShut();
				break;
			case 10:
				sub_136_1806();
				break;
			case 11:
				sub_136_185a();
				break;
			case 12:
				sub_136_1898();
				break;
			case 13:
				sub_136_18f4();
				break;
			case 14:
				sub_136_1932();
				break;
			case 15:
				sub_136_19d2();
				break;
			case 16:
				sub_136_1cf4();
				break;
			case 17:
				sub_136_1ddc();
				break;
			case 18:
				sub_136_1df4();
				break;
			case 19:
				sub_136_1e4c();
				break;
			case 20:
				sub_136_21fa();
				break;
			case 21:
				sub_136_2200();
				break;
			default:
				warning("sub_136_b00: breaking out of switch statement");
				break;
			}
		}
		// 136:0d54
	}
	// 136:0d5c
	var_i16_1bd0 = var_i16_1574;
}

void FoolGame::mazePrintMessage() {
	// 136:0d64
	if (var_str_1ce2.decode(Common::kMacRoman) == var_str_1578) {
		return;
	}
	fillRect(0x136, 0, SCREEN_HEIGHT, SCREEN_WIDTH, var_i16_1ac4);
	if (!var_str_1ce2.empty()) { // was: str(230)
		g_zbasic->text(0xfa, 0xc, 0, kSrcBic);
		var_i16_484 = g_toolbox->StringWidth(var_str_1ce2);
		fillRect(0x13b, 0xf1 - (var_i16_484 / 2), 0x14f, 0x10f + (var_i16_484 / 2), 2);
		var_i16_7a2 = 0x149;
		sub_128_918(var_str_1ce2.decode(Common::kMacRoman));
	}
	// 136:0e3a
	var_str_1578 = var_str_1ce2.decode(Common::kMacRoman);
}

void FoolGame::sub_136_e4c() {
	// 136:0e4c
	warning(__func__);
	var_i16_1de2 = g_zbasic->decodeInt(var_str_1ce2);
	arr_i16_5cbc[var_i16_1de2]++;
	if (arr_i16_3738[var_i16_1574] & 0x1000) {
		arr_i16_3738[var_i16_1574] ^= 0x1000;
	}
	// 136:0ed2
	sub_136_274e();
}

void FoolGame::sub_136_ed8() {
	warning(__func__);
	// 136:0ed8
	var_i16_1de4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_1de2 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 3, 2));
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		arr_i16_5cbc[var_i16_1de2]--;
	}
	if (arr_i16_5cbc[var_i16_1de2] < 0) {
		arr_i16_5cbc[var_i16_1de2] = 0;
	}
	sub_136_274e();
}

void FoolGame::mazeWanderingWinds() {
	// 136:0f74
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(231)) { // L
		var_i16_1de6 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_1de4 = g_zbasic->rndInt(var_i16_1de6);
		var_i16_1de8 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_1de4 * 2 + 2, 2));
		var_i16_484 = var_i16_1de6 * 2 + 4;
		sub_136_24ae();
	}
	// 136:0ffc
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(232)) { // A
		var_i16_68a = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_68c = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 4, 2));
		var_i16_1a96 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 6, 2));
		var_i16_1a98 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 8, 2));
		var_i16_68a += g_zbasic->rndInt(var_i16_1a96 + 1) - 1;
		var_i16_68c += g_zbasic->rndInt(var_i16_1a98 + 1) - 1;
		// 136:10bc
		var_i16_1de8 = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
		var_i16_484 = 0xa;
		sub_136_24ae();
	}
	// 136:10ea
	Common::Rect temp;
	temp.top = arr_i16_1eb8[24];
	temp.left = arr_i16_1eb8[25];
	temp.bottom = arr_i16_1eb8[26];
	temp.right = arr_i16_1eb8[27];
	g_toolbox->InvertOval(temp);
	var_i16_1dea = var_i16_1574;
	var_i16_1dec = 2;
	mazeMovementTrail();
	// 136:1108
	var_i16_1bcc = ((var_i16_1de8 - 1) % arr_i16_1eb8[0]) + 1;
	var_i16_1bce = ((var_i16_1de8 - 1) / arr_i16_1eb8[0]) + 1;
	mazeDrawPlayer();
}

void FoolGame::sub_136_115a() {
	warning(__func__);
	// 136:115a
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(233)) { // P
		var_i16_1dea = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_106e = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 4, 2));
		if (var_i16_1dea == var_i16_1574) {
			var_src_1dee = kSrcOr;
		} else {
			var_src_1dee = kSrcBic;
		}
		// 136:11ce
		arr_i16_4338[var_i16_1dea] = var_i16_106e;
	}
	// 136:11e0
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(234)) { // E
		var_i16_1dea = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if (var_i16_1dea < 0) {
			var_i16_1dea = var_i16_1574;
		}
		var_i16_106e = arr_i16_4338[var_i16_1dea];
		arr_i16_4338[var_i16_1dea] = 0;
		if (var_i16_1dea == var_i16_1574) {
			var_src_1dee = kSrcBic;
		} else {
			// 136:126a
			var_src_1dee = kSrcOr;
		}
	}
	// 136:1270
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(235)) { // C
		var_i16_1dea = var_i16_1574;
		var_i16_106e = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if (var_i16_1dea == var_i16_1574) {
			var_src_1dee = kSrcOr;
		} else {
			// 136:12cc
			var_src_1dee = kSrcBic;
		}
		// 136:12d2
		arr_i16_4338[var_i16_1dea] = var_i16_106e;
	}
	// 136:12e4
	g_zbasic->text(0xfb, 0xc, 0, var_src_1dee);
	g_toolbox->MoveTo(
		arr_rect_1f38[var_i16_1dea].left + arr_i16_1eb8[2],
		arr_rect_1f38[var_i16_1dea].top + arr_i16_1eb8[3]
	);
	var_str_384 = g_zbasic->chr(var_i16_106e);
	g_toolbox->DrawString(var_str_384);
}

void FoolGame::sub_136_137c() {
	warning(__func__);
	// 136:137c
	var_i16_1df0 = 0;
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(236)) { // I
		var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if (arr_i16_5cbc[var_i16_484] == 0) {
			var_i16_1df0 = 1;
		}
	}
	// 136:13d8
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(237)) { // M
		var_i16_1de4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_1de2 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 4, 2));
		if (arr_i16_5cbc[var_i16_1de2] < var_i16_1de4) {
			var_i16_1df0 = 1;
		}
	}
	// 136:144e
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(238)) { // B
		var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if ((arr_i16_3738[var_i16_484] & 0x1000) == 0) {
			var_i16_1df0 = 1;
		}
	}
	// 136:14b2
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(239)) { // L
		// 136:14d6
		var_i16_7e4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if (arr_i16_4338[var_i16_484] != var_i16_7e4) {
			var_i16_1df0 = 1;
		}
	}
	// 136:1528
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(240)) { // S
		var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_68a = 4;
		for (int16 i = 4; i <= (2*var_i16_484 + 2); i += 2) {
			var_i16_7e4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_68a, 2));
			if (arr_i16_4338[var_i16_7e4] == 0) {
				var_i16_1df0 = 1;
			}
		}
	}
	// 136:15c0
	if (g_zbasic->leftStr(var_str_1ce2, 1) == g_zbasic->strRaw(241)) { // P
		var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 2, 2));
		if (var_i16_1bd0 != var_i16_484) {
			var_i16_1df0 = 1;
		}
	}
	// 136:1612
	if (var_i16_1df0 != 0) {
		var_i16_1cde = g_zbasic->instr(1, var_str_1ac8, g_zbasic->strRaw(242));
	}
	// 136:163a
	return;
}

void FoolGame::sub_136_163c() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1650() {
	warning("STUB: %s", __func__);
}

void FoolGame::mazeHiddenDoorOpen() {
	// 136:1756
	var_i16_1bd8 = var_i16_1574;
	var_i16_1bd4 = g_zbasic->decodeInt(var_str_1ce2);
	g_toolbox->PenNormal();
	g_toolbox->PenPat(arr_pat_58f4[2]);
	g_toolbox->PenSize(5, 5);
	switch (var_i16_1bd4-1) {
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
		warning("mazeHiddenDoorOpen: broke out of switch statement");
		break;
	}
}

void FoolGame::mazeHiddenDoorShut() {
	// 136:17a4
	var_i16_1bd8 = var_i16_1574;
	var_i16_1bd6 = g_zbasic->decodeInt(var_str_1ce2);
	g_toolbox->PenNormal();
	g_toolbox->PenPat(arr_pat_58f4[arr_i16_1eb8[17]]);
	g_toolbox->PenSize(5, 5);
	switch (var_i16_1bd6-1) {
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
		warning("mazeHiddenDoorShut: broke out of switch statement");
		break;
	}
}

void FoolGame::sub_136_1806() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_185a() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1898() {
	warning(__func__);
	// 136:1898
	var_i16_484 = g_zbasic->decodeInt(var_str_1ce2);
	if (arr_i16_3738[var_i16_484] & 0x1000) {
		arr_i16_3738[var_i16_484] ^= 0x1000;
	}
}

void FoolGame::sub_136_18f4() {
	warning(__func__);
	// 136:18f4
	var_i16_1de4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_484 = 3;
	sub_136_24ae();
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		sub_136_2538();
	}
}

void FoolGame::sub_136_1932() {
	warning(__func__);
	// 136:1932
	var_i16_1de4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_484 = 3;
	sub_136_24ae();
	arr_i16_4758[0] = 0x14;
	arr_i16_4758[1] = 0;
	arr_i16_4758[2] = SCREEN_HEIGHT;
	arr_i16_4758[3] = SCREEN_WIDTH;
	for (var_i16_68a = 1; var_i16_68a <= var_i16_1de4; var_i16_68a++) {
		Common::Rect temp;
		temp.top = arr_i16_4758[0];
		temp.left = arr_i16_4758[1];
		temp.bottom = arr_i16_4758[2];
		temp.right = arr_i16_4758[3];
		g_toolbox->InvertRect(temp);
		if (var_i16_1df4 > 0) {
			sub_136_2538();
		}
		temp.top = arr_i16_4758[0];
		temp.left = arr_i16_4758[1];
		temp.bottom = arr_i16_4758[2];
		temp.right = arr_i16_4758[3];
		g_toolbox->InvertRect(temp);
	}
}

void FoolGame::sub_136_19d2() {
	// 136:19d2
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1cf4() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1ddc() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1df4() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_1e4c() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_136_21fa() {
	// 136:21fa
	sub_128_6186();
}

void FoolGame::sub_136_2200() {
	// 136:2200
	var_i16_1bdc = 1;
}

void FoolGame::mazeMovementTrail() {
	// 136:2208
	// unrolled loop
	arr_bcd_5dbc[4 + 0] = (arr_rect_1f38[var_i16_1de8].top - arr_rect_1f38[var_i16_1dea].top) / 25.0f;
	arr_bcd_5dbc[4 + 1] = (arr_rect_1f38[var_i16_1de8].left - arr_rect_1f38[var_i16_1dea].left) / 25.0f;
	arr_bcd_5dbc[4 + 2] = (arr_rect_1f38[var_i16_1de8].bottom - arr_rect_1f38[var_i16_1dea].bottom) / 25.0f;
	arr_bcd_5dbc[4 + 3] = (arr_rect_1f38[var_i16_1de8].right - arr_rect_1f38[var_i16_1dea].right) / 25.0f;
	// 136:229a
	for (int16 k = 0; k <= 1; k++) {
		// 136:22a0
		arr_bcd_5dbc[0] = arr_rect_1f38[var_i16_1dea].top + 4.0f;
		arr_bcd_5dbc[1] = arr_rect_1f38[var_i16_1dea].left + 4.0f;
		arr_bcd_5dbc[2] = arr_rect_1f38[var_i16_1dea].bottom - 4.0f;
		arr_bcd_5dbc[3] = arr_rect_1f38[var_i16_1dea].right - 4.0f;
		for (int16 j = 1; j <= 0x19; j++) {
			var_i32_692 = g_toolbox->TickCount();
			for (int16 i = 0; i <= 3; i++) {
				// 136:23c4
				arr_bcd_5dbc[i] += arr_bcd_5dbc[i + 4];
				arr_i16_4758[i] = (int16)arr_bcd_5dbc[i];
			}
			// 136:245a
			Common::Rect temp;
			temp.top = arr_i16_4758[0];
			temp.left = arr_i16_4758[1];
			temp.bottom = arr_i16_4758[2];
			temp.right = arr_i16_4758[3];

			if (var_i16_1dec == 1) {
				g_toolbox->InvertRect(temp);
			}
			// 136:2472
			if (var_i16_1dec == 2) {
				g_toolbox->InvertOval(temp);
			}
			sub_128_406(0);
		}
	}
}

void FoolGame::sub_136_24ae() {
	warning(__func__);
	// 136:24ae
	var_i16_1df4 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_484, 2));
	if (var_i16_1df4 > 0) {
		var_i16_1e02 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_484 + 2, 2));
		var_i16_1372 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_484 + 4, 2))*2;
		var_i16_1e04 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ce2, var_i16_484 + 6, 2));
	}
	// 136:2536
}

void FoolGame::sub_136_2538() {
	warning(__func__);
	// 136:2538
	sub_128_50e(var_i16_1df4, var_i16_1372, 0);
	if ((var_i16_1df4 > 0xe) && (var_i16_1df4 < 0x7d00)) {
		var_i16_1df4 += var_i16_1e02;
	}
}


void FoolGame::sub_136_2582() {
	warning(__func__);
	// 136:2582
	var_i16_1df2 = 0;
	var_i16_1e06 = g_zbasic->decodeInt(g_zbasic->leftStr(var_str_1ac8, 2));
	for (int16 i = 1; i <= var_i16_1e06; i++) {
		var_i16_484 = g_zbasic->decodeInt(g_zbasic->midStr(var_str_1ac8, i*2 + 3, 2));
		if ((arr_i16_3738[var_i16_484] & 0x1000) == 0) {
			var_i16_1df2++;
			arr_i16_5bbc[var_i16_1df2] = var_i16_484;
		}
		// 136:2602
	}
	var_i16_484 = arr_i16_5bbc[g_zbasic->rndInt(var_i16_1df2)];
	arr_i16_3738[var_i16_484] |= 0x1000;
}

void FoolGame::mazeDrawPlayer() {
	warning(__func__);
	// 136:2664
	var_i16_1574 = arr_i16_2f38[var_i16_1bcc*32 + var_i16_1bce];
	arr_i16_1eb8[24] = arr_rect_1f38[var_i16_1574].top + 4;
	arr_i16_1eb8[25] = arr_rect_1f38[var_i16_1574].left + 4;
	arr_i16_1eb8[26] = arr_rect_1f38[var_i16_1574].bottom - 4;
	arr_i16_1eb8[27] = arr_rect_1f38[var_i16_1574].right - 4;
	// 136:273e
	Common::Rect temp;
	temp.top = arr_i16_1eb8[24];
	temp.left = arr_i16_1eb8[25];
	temp.bottom = arr_i16_1eb8[26];
	temp.right = arr_i16_1eb8[27];
	g_toolbox->InvertOval(temp); // draw new player pos
}

void FoolGame::sub_136_274e() {
	warning(__func__);
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
	for (var_i16_68c = 1; var_i16_68c <= arr_i16_1eb8[1]; var_i16_68c++) {
		for (var_i16_68a = 1; var_i16_68a <= arr_i16_1eb8[0]; var_i16_68a++) {
			var_i16_1574++;
			if (arr_i16_1eb8[21] != 0) {
				if (((arr_i16_3738[var_i16_1574] & 1) == 1) && ((arr_i16_3738[var_i16_1574] & 0x10) == 0)) {
					if ((var_i16_1f08 & bitLUT[var_i16_1de6]) != 0) {
						sub_136_2a7c();
					}
					sub_136_3a30();
				}
				// 136:380c
				if (((arr_i16_3738[var_i16_1574] & 2) == 2) && ((arr_i16_3738[var_i16_1574] & 0x20) == 0)) {
					if ((var_i16_1f08 & bitLUT[var_i16_1de6]) != 0) {
						sub_136_2b30();
					}
					sub_136_3a30();
				}
				// 136:3886
				if (((arr_i16_3738[var_i16_1574] & 4) == 0) && ((arr_i16_3738[var_i16_1574] & 0x40) == 0)) {
					if ((var_i16_1f08 & bitLUT[var_i16_1de6]) != 0) {
						sub_136_2be2();
					}
					sub_136_3a30();
				}
				// 136:3900
				if (((arr_i16_3738[var_i16_1574] & 8) == 8) && ((arr_i16_3738[var_i16_1574] & 0x80) == 0)) {
					if ((var_i16_1f08 & bitLUT[var_i16_1de6]) != 0) {
						sub_136_2c96();
					}
					sub_136_3a30();
				}
			}
			// 136:397c
			if (arr_i16_3b38[var_i16_68a*32 + var_i16_68c] > 0) {
				if ((var_i16_1f08 & bitLUT[var_i16_1de6]) != 0) {
					arr_i16_3738[var_i16_1574] |= 0x1000;
				}
				// 136:39f2
				sub_136_3a30();
			}
			// 136:39f6
		}
	}
}

void FoolGame::sub_136_3a30() {
	// 136:3a30
	var_i16_1de6++;
	if (var_i16_1de6 > 0xf) {
		var_i16_1de6 = 0;
	}
	if (var_i16_1de6 == 0) {
		var_i16_1f08 = g_zbasic->decodeInt(g_zbasic->midStr(activePuzzleBuffer, var_i16_1f0a, 2));
		var_i16_1f0a += 2;
	}
	// 136:3a6e
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

				if (j == 0) {
					g_toolbox->InvertRect(temp);
				} else {
					// 136:3c48
					g_toolbox->PaintRect(temp);
				}
				// 136:3c56
				g_toolbox->Delay(0);
			}
			// 136:3c64
		}
	}
	// 136:3c72
}

}
