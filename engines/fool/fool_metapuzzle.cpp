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

;
extern Toolbox *g_toolbox;

// sun's map metapuzzle
void FoolGame::metapuzzleRun() {
	// 138:0004
	if (_sunMapRestored == 1) {
		_sunMapRestored = 666;
		var_i32_7c8 = _zbasic->mem(-1);
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
				_zbasic->picture(i, j, _sunMapTilePic[var_i16_484]);
			}
		}
	}
	// 138:00f6
	g_toolbox->PenNormal();
	g_toolbox->InitCursor();
	copyScreen(0, arr_bmp_b3ec);
	if (_activePuzzle == 0) {
		metapuzzleSetupMenu();
	}
	// 138:0116
	_stateFlags = kStateNull;
	_keyLastPressed = 0;
	while ((_stateFlags & kStateReturn) == 0) {
		// 138:0126
		getNextEvent(-1);
		if (_event.what == 1) {
			metapuzzleOnClick();
		}
		if ((_event.modifiers & (kModLOptionKeyDown | kModMouseButtonUp)) == (kModLOptionKeyDown | kModMouseButtonUp)) {
			metapuzzleOnOption();
		}
		if ((_event.modifiers & (kModLShiftKeyDown | kModMouseButtonUp)) == (kModLShiftKeyDown | kModMouseButtonUp)) {
			metapuzzleOnShift();
		}
		if (_activePuzzle != 0) {
			sub_138_21e();
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
		if (_keyLastPressed == 0x20) {
			setStateBits(kStateReturn);
		}
	}
	// JMP 0xe7c
	// 138:0e7c
	var_i16_c00 = 0;
	if ((_stateFlags & kStateMetapuzzleComplete) == 0) {
		var_menu_bf8 = g_toolbox->GetMHandle(8);
		g_toolbox->DeleteMenu(8);
		g_toolbox->DisposeMenu(var_menu_bf8);
		g_toolbox->DrawMenuBar();
		copyScreen(1, arr_bmp_5dfc);
		if ((_stateFlags & (kStateNewGame | kStateOpenGame | kStateQuit)) == 0) {
			if ((var_i16_7ce & 1) != 0) {
				var_i16_7ce ^= 1;
			}
			// 138:0ef8
			_activePuzzle = 0;
			if ((_stateFlags & kStateChapterSelect) == 0) {
				storyRenderPage();
			}
		}
		// 138:0f16
		sub_128_61ec();
	}
	// 138:0f1a
}

void FoolGame::metapuzzleOnClick() {
	// 138:01b4
	var_i16_7e4 = 0;
	_activePuzzle = 0;
	for (int16 i = 1; i <= 0xc; i++) {
		Common::Rect temp;
		temp.top = arr_i16_4d20[i*4];
		temp.left = arr_i16_4d20[i*4+1];
		temp.bottom = arr_i16_4d20[i*4+2];
		temp.right = arr_i16_4d20[i*4+3];
		if (g_toolbox->PtInRect(_event.where, temp) != 0) {
			var_i16_7e4 = i;
		}
		// 138:01f8
	}
	if (var_i16_7e4 > 0) {
		_activePuzzle = var_i16_7e4 + 0x50;
	}
}

void FoolGame::sub_138_21e() {
	// 138:021e
	int16 offset = _activePuzzle - 0x50;
	Common::Rect temp;
	temp.top = arr_i16_4d20[offset*4];
	temp.left = arr_i16_4d20[offset*4+1];
	temp.bottom = arr_i16_4d20[offset*4+2];
	temp.right = arr_i16_4d20[offset*4+3];
	g_toolbox->InvertRect(temp);
	puzzleLoadContext();
	puzzleSetupMenu();
	sub_128_26f6();
	sub_128_61ec();
	if (_activePuzzle == 0x55) {
		fillRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x47);
	}
	// 138:0272
	if ((_activePuzzle >= 0x51) && (_activePuzzle <= 0x55)) {
		jumbleRun();
	}
	if (_activePuzzle == 0x56) {
		straightPathRun();
	}
	if (_activePuzzle == 0x57) {
		zoomRect(arr_i16_4d20[28], arr_i16_4d20[0x1c+1], arr_i16_4d20[0x1c+2], arr_i16_4d20[0x1c+3], 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kPatXor, 0x1a);
		mazeRun();
	}
	// 138:0334
	if ((_activePuzzle >= 0x58) && (_activePuzzle <= 0x5b)) {
		metapuzzleSecretCode();
	}
	if (_activePuzzle == 0x5c) {
		metapuzzleWheel();
	}
	puzzleSaveContext();
	// any progress is good progress
	autoSaveGame();
	if ((_activePuzzle == 0x55) && (_puzzleCompletionStatus[0x55] == 0x64)) {
		_stateFlags = kStateMetapuzzleComplete | kStateQuit | kStateReturn;
	}
	if (_stateFlags <= 1) {
		_stateFlags = kStateNull;
		_activePuzzle = 0;
		var_i16_c00 = 0;
		g_toolbox->PenNormal();
		copyScreen(1, arr_bmp_b3ec);
		metapuzzleSetupMenu();
	}
	// 138:03da
	sub_128_61ec();
}

void FoolGame::metapuzzleSetupMenu() {
	// 138:03e0
	_zbasic->menu(8, 0, 1, _zbasic->str(260)); // the sun's map
	_zbasic->menu(8, 1, 1, _zbasic->str(261)); // return to scroll
	_zbasic->menu(8, 2, 0, _zbasic->str(262)); // -
	var_i16_484 = 2;
	for (int16 i = _puzzleMenuInstructions[0x30]; i <= _puzzleMenuInstructions[0x30+1]; i++) {
		var_i16_484++;
		var_str_384 = _zbasic->index(0, i) + _zbasic->str(263); // '  '
		_zbasic->menu(8, var_i16_484, 1, var_str_384);
	}
}

void FoolGame::metapuzzleWheel() {
	// 138:049e
	for (int16 i = 1; i <= 2; i++) {
		zoomRect(arr_i16_4d20[0x30], arr_i16_4d20[0x30 + 1], arr_i16_4d20[0x30 + 2], arr_i16_4d20[0x30 + 3], 0x6e, 7, 0x100, 0x5d, 0x1, kPatXor, 0x1a);
	}
	_zbasic->picture(6, 0x6d, _metapuzzleWheelPic);
	sub_128_2664();
	sub_128_61c2();
}

void FoolGame::metapuzzleSecretCode() {
	// 138:0550
	var_i16_c00 = 1;
	for (int16 i = 1; i <= 2; i++) {
		var_i16_68c = _activePuzzle - 0x50;
		zoomRect(arr_i16_4d20[var_i16_68c*4], arr_i16_4d20[var_i16_68c*4+1], arr_i16_4d20[var_i16_68c*4+2], arr_i16_4d20[var_i16_68c*4+3], 0x122, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kPatXor, 0x1a);
	}
	if (_activePuzzle == 0x58) {
		var_str_1170 = _zbasic->str(264); // treasure name
		_metapuzzleSecretCodeSpaceOffset = 8;
		_metapuzzleSecretCodeCipher = _zbasic->str(265); // CQHXBMLGPTWIFRYASUVJZNKDOE
	}
	// 138:0650
	if (_activePuzzle == 0x59) {
		var_str_1170 = _zbasic->str(266); // treasure name
		_metapuzzleSecretCodeSpaceOffset = 0;
		_metapuzzleSecretCodeCipher = _zbasic->str(267); // IEGADHFBCPLNRKQOJMZWTYVUSX
	}
	// 138:068a
	if (_activePuzzle == 0x5a) {
		var_str_1170 = _zbasic->str(268); // treasure name
		_metapuzzleSecretCodeSpaceOffset = 5;
		_metapuzzleSecretCodeCipher = _zbasic->str(269); // ISDXMRCLHQYWBVPKGOUZTFNJAE
	}
	// 138:06c4
	if (_activePuzzle == 0x5b) {
		var_str_1170 = _zbasic->str(270); // treasure name
		_metapuzzleSecretCodeSpaceOffset = 9;
		_metapuzzleSecretCodeCipher = _zbasic->str(271); // ZYXWVUTSRQPONMLKJIHGFEDCBA
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
	g_toolbox->FillRect(temp, _patterns[2]);
	if (_activePuzzleBuffer.empty()) { // was: str(272)
		metapuzzleSecretCodeReset();
	} else {
		// 138:0778
		var_str_1272 = _activePuzzleBuffer;
		_metapuzzleSecretCodeCount = 0;
		var_i16_68a = 1;
		for (int16 i = 1; i <= (int16)var_str_1272.size(); i++) {
			if (_zbasic->midStr(var_str_1272, i, 1) != _zbasic->str(273)) { // ' '
				_metapuzzleSecretCodeCount++;
			}
			// 138:07c2
			sub_138_9c4();
		}
	}
	// 138:07d8
	while ((_stateFlags & kStateReturn) == 0) {
		// 138:07dc
		_stateFlags = kStateNull;
		_keyLastPressed = 0;
		while (_stateFlags == kStateNull) {
			// 138:07ec
			getNextEvent(-1);
			if (_keyLastPressed > 0) {
				metapuzzleSecretCodeDrawText();
				_keyLastPressed = 0;
			}
			if (var_str_1272 == var_str_1170) {
				sub_138_a90();
			}
			// 138:081c
		}
		// 138:0822
		if (_stateFlags == kStateUndo) {
			metapuzzleSecretCodeReset();
		}
		_activePuzzleBuffer = var_str_1272;
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
}

void FoolGame::metapuzzleSecretCodeDrawText() {
	// 138:0864
	if ((_keyLastPressed == 3) || (_keyLastPressed == 0xd)) {
		sub_138_a06();
		return;
	}
	if ((_keyLastPressed >= 0x61) && (_keyLastPressed <= 0x7a)) {
		_keyLastPressed -= 0x20;
	}
	if (!((_keyLastPressed >= 0x41) && (_keyLastPressed <= 0x5a))) {
		return;
	}
	_zbasic->text(0xfe, 0x18, 0x19, kSrcBic);
	_metapuzzleSecretCodeCount++;
	if (_metapuzzleSecretCodeSpaceOffset == _metapuzzleSecretCodeCount) {
		if (_zbasic->leftStr(var_str_1170, _metapuzzleSecretCodeSpaceOffset - 1) == var_str_1272) {
			var_str_1272 += _zbasic->str(274); // ' '
		}
	}
	// 138:0946
	var_str_d12 = _zbasic->midStr(_metapuzzleSecretCodeCipher, _keyLastPressed - 0x40, 1);
	var_str_1272 += var_str_d12;
	var_i16_484 = g_toolbox->StringWidth(var_str_1272);
	var_i16_7e4 = g_toolbox->StringWidth(var_str_1170);
	if (var_i16_484 > var_i16_7e4) {
		sub_138_a06();
	} else {
		sub_138_9c4();
	}
}

void FoolGame::sub_138_9c4() {
	// 138:09c4
	_zbasic->text(0xfe, 0x18, 0x19, kSrcBic);
	Common::Rect temp;
	temp.top = arr_i16_1eb8[0];
	temp.left = arr_i16_1eb8[1];
	temp.bottom = arr_i16_1eb8[2];
	temp.right = arr_i16_1eb8[3];
	g_toolbox->FillRect(temp, _patterns[2]);
	var_i16_7a2 = 0x146;
	sub_128_918(var_str_1272);
}

void FoolGame::sub_138_a06() {
	// 138:0a06
	playTone(0x19, 0x64, 1);
	if (_metapuzzleSecretCodeCount == 0)
		return;
	metapuzzleSecretCodeReset();
}

void FoolGame::metapuzzleSecretCodeReset() {
	// 138:0a22
	Common::Rect temp;
	temp.top = arr_i16_1eb8[0];
	temp.left = arr_i16_1eb8[1];
	temp.bottom = arr_i16_1eb8[2];
	temp.right = arr_i16_1eb8[3];
	g_toolbox->FillRect(temp, _patterns[2]);
	_zbasic->text(0xfa, 0xc, 0, kSrcBic);
	var_i16_7a2 = 0x140;
	var_str_384 = _zbasic->str(275); // if you know which is which, enter the letters you wish to switch
	sub_128_918(var_str_384);
	var_str_1272.clear(); // was: str(276)
	_metapuzzleSecretCodeCount = 0;
}

void FoolGame::sub_138_a90() {
	// 138:0a90
	_stateFlags = kStateReturn;
	_activePuzzleBuffer = var_str_1272;
	sub_128_2664();
	sub_128_6186();
	sub_128_d34(arr_i16_1eb8[0], arr_i16_1eb8[1], arr_i16_1eb8[2], arr_i16_1eb8[3], 0xc8);
}

void FoolGame::metapuzzleOnOption() {
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

void FoolGame::metapuzzleOnShift() {
	// 138:0b6a
	if (!((_event.where.x >= 0x6e) && (_event.where.y >= 0x1a) && (_event.where.x <= 0x1a8) && (_event.where.y <= SCREEN_HEIGHT))) {
		return;
	}
	var_i16_200e = 0;
	while ((_event.modifiers & kModLShiftKeyDown) != 0) {
		getNextEvent(-1); // was: 0
		if ((_event.where.x >= 0x6e) && (_event.where.y >= 0x1a) && (_event.where.x <= 0x1a8) && (_event.where.y <= SCREEN_HEIGHT)) {
			// 138:0c04
			var_i16_68a = (_event.where.x - 0x6e)/0x23;
			var_i16_68c = (_event.where.y - 0x1a)/0x23;
			var_i16_1888 = var_i16_68c * 9 + var_i16_68a + 1;
			_selectedMenuChapter = arr_i16_4c7c[var_i16_1888];
			if (var_i16_200e != _selectedMenuChapter) {
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
				fillRect(0, 7, 0x13, _windowWidth - 7, 0);
				if (_selectedMenuChapter != 0x51) {
					var_str_384 = _puzzleName[_selectedMenuChapter];
				} else {
					// 138:0d8a
					var_str_384 = _zbasic->str(277); // the book of thoth
				}
				// 138:0d9e
				_zbasic->text(0, 0xc, 0, kSrcOr);
				var_i16_30 = g_toolbox->StringWidth(var_str_384);
				g_toolbox->MoveTo((_windowWidth / 2) - (var_i16_30 / 2), 0xf);
				g_toolbox->DrawString(var_str_384);
				g_toolbox->_defaultMenu->setOverlayDirty(true);
				g_toolbox->SetPort(var_i32_0);
				var_i16_200e = _selectedMenuChapter;
			}
			// 138:0dfc
			if ((_event.modifiers & kModMouseButtonUp) == 0) {
				if (_selectedMenuChapter != 0x51) {
					setStateBits(kStateChapterSelect | kStateReturn);
					return;
				} else {
					_activePuzzle = 0x55;
					return;
				}
			}
			// 138:0e34
		} else {
			// 138:0e38
			_event.modifiers = 0;
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

}
