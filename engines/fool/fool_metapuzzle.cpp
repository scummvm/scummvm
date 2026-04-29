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

// sun's map metapuzzle
void FoolGame::sub_138_004() {
	// 138:0004
	if (sunMapRestored == 1) {
		sunMapRestored = 666;
		var_i32_7c8 = g_zbasic->mem(-1);
	} else {
		// 138:0024
		var_i16_7ce |= 1;
		copyScreen(0, arr_bmp_5dfc);
		fillRect(0x127, 0x69, 0x138, 0x190, 0);
		sub_128_69c(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
		fillRect(0x14, 0x67, SCREEN_HEIGHT, 0x1b1, 2);
		var_i16_484 = 0;
		for (int16 j = 0x1a; j <= 0x132; j += 0x23) {
			for (int16 i = 0x6e; i <= 0x186; i += 0x23) {
				var_i16_484++;
				g_zbasic->picture(i, j, arr_i32_1912c[var_i16_484]);
			}
		}
	}
	// 138:00f6
	g_toolbox->PenNormal();
	g_toolbox->InitCursor();
	copyScreen(0, arr_bmp_b3ec);
	if (activePuzzle == 0) {
		sub_138_3e0();
	}
	// 138:0116
	stateFlags = 0;
	keyLastPressed = 0;
	while ((stateFlags & kStateReturn) == 0) {
		// 138:0126
		getNextEvent(-1);
		if (var_ev_46.what == 1) {
			sub_138_1b4();
		}
		if ((var_ev_46.modifiers & (kModLOptionKeyDown | kModMouseButtonUp)) == 0) {
			sub_138_b06();
		}
		if ((var_ev_46.modifiers & (kModLShiftKeyDown | kModMouseButtonUp)) == 0) {
			sub_138_b6a();
		}
		if (activePuzzle != 0) {
			sub_138_21e();
		}
		if (stateFlags == 4) {
			saveGame();
		}
		if (keyLastPressed == 0x20) {
			setStateBits(kStateReturn);
		}
	}
	// JMP 0xe7c
	// 138:0e7c
	var_i16_c00 = 0;
	if ((stateFlags & 0x800) == 0) {
		var_menu_bf8 = g_toolbox->GetMHandle(8);
		g_toolbox->DeleteMenu(8);
		g_toolbox->DisposeMenu(var_menu_bf8);
		g_toolbox->DrawMenuBar();
		copyScreen(1, arr_bmp_5dfc);
		if ((stateFlags & 0x38) == 0) {
			if ((var_i16_7ce & 1) != 0) {
				var_i16_7ce ^= 1;
			}
			// 138:0ef8
			activePuzzle = 0;
			if ((stateFlags & 0x40) == 0) {
				storyRenderPage();
			}
		}
		// 138:0f16
		sub_128_61ec();
	}
	// 138:0f1a
}

void FoolGame::sub_138_1b4() {
	// 138:01b4
	var_i16_7e4 = 0;
	activePuzzle = 0;
	for (int16 i = 1; i <= 0xc; i++) {
		Common::Rect temp;
		temp.top = arr_i16_4d20[i*4];
		temp.left = arr_i16_4d20[i*4+1];
		temp.bottom = arr_i16_4d20[i*4+2];
		temp.right = arr_i16_4d20[i*4+3];
		if (g_toolbox->PtInRect(var_ev_46.where, temp) != 0) {
			var_i16_7e4 = i;
		}
		// 138:01f8
	}
	if (var_i16_7e4 > 0) {
		activePuzzle = var_i16_7e4 + 0x50;
	}
}

void FoolGame::sub_138_21e() {
	// 138:021e
	int16 offset = activePuzzle - 0x50;
	Common::Rect temp;
	temp.top = arr_i16_4d20[offset*4];
	temp.left = arr_i16_4d20[offset*4+1];
	temp.bottom = arr_i16_4d20[offset*4+2];
	temp.right = arr_i16_4d20[offset*4+3];
	g_toolbox->InvertRect(temp);
	puzzleLoadContext();
	sub_128_3fb6();
	sub_128_26f6();
	sub_128_61ec();
	if (activePuzzle == 0x55) {
		fillRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x47);
	}
	// 138:0272
	if ((activePuzzle >= 0x51) && (activePuzzle <= 0x55)) {
		jumbleRun();
	}
	if (activePuzzle == 0x56) {
		sub_143_004();
	}
	if (activePuzzle == 0x57) {
		sub_128_962(arr_i16_4d20[28], arr_i16_4d20[0x1c+1], arr_i16_4d20[0x1c+2], arr_i16_4d20[0x1c+3], 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kPatXor, 0x1a);
		mazeRun();
	}
	// 138:0334
	if ((activePuzzle >= 0x58) && (activePuzzle <= 0x5b)) {
		sub_138_550();
	}
	if (activePuzzle == 0x5c) {
		sub_138_49e();
	}
	puzzleSaveContext();
	if ((activePuzzle == 0x55) && (puzzleCompletionStatus[0x55] == 0x64)) {
		stateFlags = 0x821;
	}
	if (stateFlags <= 1) {
		stateFlags = 0;
		activePuzzle = 0;
		var_i16_c00 = 0;
		g_toolbox->PenNormal();
		copyScreen(1, arr_bmp_b3ec);
	}
	// 138:03da
	sub_128_61ec();
}

void FoolGame::sub_138_3e0() {
	// 138:03e0
	g_zbasic->menu(8, 0, 1, g_zbasic->str(260)); // the sun's map
	g_zbasic->menu(8, 1, 1, g_zbasic->str(261)); // return to scroll
	g_zbasic->menu(8, 2, 0, g_zbasic->str(262)); // -
	var_i16_484 = 2;
	for (int16 i = arr_i16_1b10[0x30]; i <= arr_i16_1b10[0x30+1]; i++) {
		var_i16_484++;
		var_str_384 = g_zbasic->index(0, i) + g_zbasic->str(263); // '  '
		g_zbasic->menu(8, var_i16_484, 1, var_str_384);
	}
}

void FoolGame::sub_138_49e() {
	// 138:049e
	for (int16 i = 1; i <= 2; i++) {
		sub_128_962(arr_i16_4d20[0x30], arr_i16_4d20[0x30 + 1], arr_i16_4d20[0x30 + 2], arr_i16_4d20[0x30 + 3], 0x6e, 7, 0x100, 0x5d, 0x1, kPatXor, 0x1a);
	}
	g_zbasic->picture(6, 0x6d, var_pic_1032);
	sub_128_2664();
	sub_128_61c2();
}

void FoolGame::sub_138_550() {
	// 138:0550
	var_i16_c00 = 1;
	for (int16 i = 1; i <= 2; i++) {
		var_i16_68c = activePuzzle - 0x50;
		sub_128_962(arr_i16_4d20[var_i16_68c*4], arr_i16_4d20[var_i16_68c*4+1], arr_i16_4d20[var_i16_68c*4+2], arr_i16_4d20[var_i16_68c*4+3], 0x122, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kPatXor, 0x1a);
	}
	if (activePuzzle == 0x58) {
		var_str_1170 = g_zbasic->str(264); // treasure name
		var_i16_1aa2 = 8;
		var_str_1f0c = g_zbasic->str(265); // CQHXBMLGPTWIFRYASUVJZNKDOE
	}
	// 138:0650
	if (activePuzzle == 0x59) {
		var_str_1170 = g_zbasic->str(266); // treasure name
		var_i16_1aa2 = 0;
		var_str_1f0c = g_zbasic->str(267); // IEGADHFBCPLNRKQOJMZWTYVUSX
	}
	// 138:068a
	if (activePuzzle == 0x5a) {
		var_str_1170 = g_zbasic->str(268); // treasure name
		var_i16_1aa2 = 5;
		var_str_1f0c = g_zbasic->str(269); // ISDXMRCLHQYWBVPKGOUZTFNJAE
	}
	// 138:06c4
	if (activePuzzle == 0x5b) {
		var_str_1170 = g_zbasic->str(270); // treasure name
		var_i16_1aa2 = 5;
		var_str_1f0c = g_zbasic->str(271); // ZYXWVUTSRQPONMLKJIHGFEDCBA
	}
	// 138:06fe
	arr_i16_1eb8[0] = 0x122;
	arr_i16_1eb8[1] = 0;
	arr_i16_1eb8[2] = SCREEN_HEIGHT;
	arr_i16_1eb8[3] = SCREEN_WIDTH;
	Common::Rect temp;
	temp.top = arr_i16_1eb8[0];
	temp.left = arr_i16_1eb8[1];
	temp.bottom = arr_i16_1eb8[2];
	temp.right = arr_i16_1eb8[3];
	g_toolbox->FillRect(temp, arr_pat_58f4[2]);
	if (activePuzzleBuffer.empty()) { // was: str(272)
		sub_138_a22();
	} else {
		// 138:0778
		var_str_1272 = activePuzzleBuffer;
		var_i16_200c = 0;
		var_i16_68a = 1;
		for (int16 i = 1; i <= (int16)var_str_1272.size(); i++) {
			if (g_zbasic->midStr(var_str_1272, i, 1) != g_zbasic->str(273)) {
				var_i16_200c++;
			}
			// 138:07c2
			sub_138_9c4();
		}
	}
	// 138:07d8
	while ((stateFlags & kStateReturn) == 0) {
		// 138:07dc
		stateFlags = kStateNull;
		keyLastPressed = 0;
		while (stateFlags == kStateNull) {
			// 138:07ec
			getNextEvent(-1);
			if (keyLastPressed > 0) {
				sub_138_864();
				keyLastPressed = 0;
			}
			if (var_str_1272 == var_str_1170) {
				sub_138_a90();
			}
			// 138:081c
		}
		// 138:0822
		if (stateFlags == kStateUndo) {
			sub_138_a22();
		}
		activePuzzleBuffer = var_str_1272;
		if (stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
}

void FoolGame::sub_138_864() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_138_9c4() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_138_a22() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_138_a90() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_138_b06() {
	// 138:0b06
	var_i16_bfc++;
	if ((var_i16_bfc < 0) || (var_i16_bfc > 0xc)) {
		var_i16_bfc = 1;
	}
	Common::Rect temp;
	temp.top = arr_i16_4d20[var_i16_bfc*4];
	temp.left = arr_i16_4d20[var_i16_bfc*4+1];
	temp.bottom = arr_i16_4d20[var_i16_bfc*4+2];
	temp.right = arr_i16_4d20[var_i16_bfc*4+3];
	g_toolbox->InvertRect(temp);
	sub_128_3da(3);
	g_toolbox->InvertRect(temp);
}

void FoolGame::sub_138_b6a() {
	// 138:0b6a
	if (!((var_ev_46.where.x >= 0x6e) && (var_ev_46.where.y >= 0x1a) && (var_ev_46.where.x <= 0x1a8) && (var_ev_46.where.y <= SCREEN_HEIGHT))) {
		return;
	}
	var_i16_200e = 0;
	while ((var_ev_46.modifiers & kModLShiftKeyDown) == 0) {
		getNextEvent(-1); // was: 0
		if ((var_ev_46.where.x >= 0x6e) && (var_ev_46.where.y >= 0x1a) && (var_ev_46.where.x <= 0x1a8) && (var_ev_46.where.y <= SCREEN_HEIGHT)) {
			// 138:0c04
			var_i16_68a = (var_ev_46.where.x - 0x6e)/0x23;
			var_i16_68c = (var_ev_46.where.y - 0x1a)/0x23;
			var_i16_1888 = var_i16_68c * 9 + var_i16_68a + 1;
			selectedMenuChapter = arr_i16_4c7c[var_i16_1888];
			if (var_i16_200e != selectedMenuChapter) {
				if (var_i16_200e > 0) {
					Common::Rect temp;
					temp.top = arr_i16_1eb8[0];
					temp.left = arr_i16_1eb8[1];
					temp.bottom = arr_i16_1eb8[2];
					temp.right = arr_i16_1eb8[3];
					g_toolbox->InvertRect(temp);
				}
				// 138:0c8c
				arr_i16_1eb8[0] = 0x1a + var_i16_68c * 0x23;
				arr_i16_1eb8[1] = 0x6e + var_i16_68a * 0x23;
				arr_i16_1eb8[2] = 0x3d + var_i16_68c * 0x23;
				arr_i16_1eb8[3] = 0x91 + var_i16_68a * 0x23;
				Common::Rect temp;
				temp.top = arr_i16_1eb8[0];
				temp.left = arr_i16_1eb8[1];
				temp.bottom = arr_i16_1eb8[2];
				temp.right = arr_i16_1eb8[3];
				g_toolbox->InvertRect(temp);

				// 138:0d36
				g_toolbox->SetPort(var_i32_8);
				fillRect(0, 7, 0x13, var_i16_5a - 7, 0);
				if (selectedMenuChapter != 0x51) {
					var_str_384 = arr_str_195e8[selectedMenuChapter];
				} else {
					// 138:0d8a
					var_str_384 = g_zbasic->str(277); // the book of thoth
				}
				// 138:0d9e
				g_zbasic->text(0, 0xc, 0, kSrcOr);
				var_i16_30 = g_toolbox->StringWidth(var_str_384);
				g_toolbox->MoveTo((var_i16_5a / 2) - (var_i16_30 / 2), 0xf);
				g_toolbox->DrawString(var_str_384);
				g_toolbox->SetPort(var_i32_0);
				var_i16_200e = selectedMenuChapter;
			}
			// 138:0dfc
			if ((var_ev_46.modifiers & kModMouseButtonUp) == 0) {
				if (selectedMenuChapter != 0x51) {
					setStateBits(0x41);
					return;
				} else {
					activePuzzle = 0x55;
					return;
				}
			}
			// 138:0e34
		} else {
			// 138:0e38
			var_ev_46.modifiers = 0;
		}
		// 138:0e3e
	}
	// 138:0e50
	if (var_i16_200e > 0) {
		g_toolbox->DrawMenuBar();
		Common::Rect temp;
		temp.top = arr_i16_1eb8[0];
		temp.left = arr_i16_1eb8[1];
		temp.bottom = arr_i16_1eb8[2];
		temp.right = arr_i16_1eb8[3];
		g_toolbox->InvertRect(temp);
		var_i16_200e = 0;
	}
	// 138:0e70
	g_toolbox->SetPort(var_i32_0);
	sub_128_61ec();
}

void FoolGame::sub_143_004() {
	warning("STUB: %s", __func__);
}


}
