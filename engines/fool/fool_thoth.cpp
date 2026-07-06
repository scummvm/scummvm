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

#define OFF(x) (_zstrOffset[kOffsetThoth] + (x))



// high priestess challenge
void FoolGame::highPriestessRun() {
	// 140:0004
	if (_activePuzzleStatus < 0x64) {
		thothAdjustPuzzleData();
	}
	if ((var_i16_7ce & 2) != 0) {
		_toolbox->SetCursor(_cursors[0x10]);
	}
	if (_activePuzzleStatus == 0) {
		_menuHidesPlayfield = true;
		thoth99Enchantments();
		_menuHidesPlayfield = false;
		if (!_activePuzzleSolved) {
			sub_140_3412();
			return;
		}
		// 140:0056
		_activePuzzleStatus = 1;
	}
	// 140:005c
	if (_activePuzzleStatus == 1) {
		thothKey1st();
		if (!_activePuzzleSolved) {
			sub_140_3412();
			return;
		}
		_activePuzzleStatus = 2;
	}
	// 140:007e
	_toolbox->SetCursor(_cursors[0x10]);
	thothSetupMenu();
	if (_activePuzzleStatus == 2) {
		thothKey2nd();
		if (!_activePuzzleSolved) {
			sub_140_3412();
			return;
		}
		_activePuzzleStatus = 3;
		if (_puzzleCompletionStatus[0x17] == 0x63) {
			// is the humbug marked with the wadjet eye? if so, unmark
			// 140:00ce
			_puzzleCompletionStatus[0x17] = 0x64;
			var_str_384 = _puzzleName[0x17] + _zbasic->str(OFF(0)); // ' '
			_zbasic->menu(0x4, 0x7, 0x1, var_str_384);
		}
	}
	// 140:0116
	if (_activePuzzleStatus == 3) {
		thothKey3rd();
		if (!_activePuzzleSolved) {
			return;
		}
		_activePuzzleStatus = 4;
		if (_puzzleCompletionStatus[0x3f] == 0x63) {
			// is justice marked with the wadjet eye? if so, unmark
			_puzzleCompletionStatus[0x3f] = 0x64;
			var_str_384 = _puzzleName[0x3f] + _zbasic->str(OFF(1)); // ' '
			_zbasic->menu(6, 0xf, 1, var_str_384);
		}
	}
	// 140:019c
	if (_activePuzzleStatus == 4) {
		_hermitPathStage = 6;
		thothKeyLast();
		if (!_activePuzzleSolved) {
			sub_140_3412();
			return;
		}
		_activePuzzleStatus = 0x64;
		_puzzleFlags[0x51] |= 0x5;
		if (_puzzleCompletionStatus[0x48] == 0x63) {
			// is the hermit marked with the wadjet eye? if so, unmark
			_puzzleCompletionStatus[0x48] = 0x64;
			var_str_384 = _puzzleName[0x48] + _zbasic->str(OFF(2)); // ' '
			_zbasic->menu(7, 8, 1, var_str_384);
		}
	}
	// 140:024c
	if (_sunMapRestored == 0) {
		// show the fake puzzle picture
		_toolbox->PenNormal();
		arr_i32_192c0[0] = _toolbox->GetPicture(0x78);
		_zbasic->picture(0, 0, arr_i32_192c0[0]);
		_toolbox->ReleaseResource(arr_i32_192c0[0]);
		waitForClick();
	}
	// 140:02a2
	if (!_screenOversized) {
		_toolbox->SetPort(var_i32_8_thoth);
	} else {
		sub_128_1ef8();
	}
	// 140:02b4
	var_i16_68a = 1;
	for (int16 i = 1; i <= 0x155; i++) {
		_zbasic->get(1, i, SCREEN_WIDTH, i + 1, arr_bmp_fa3c);
		_zbasic->put(_zbasic->rndInt(0x14) - 0xa, i, arr_bmp_fa3c, kSrcCopy);
		if (i % 2 == 0) {
			_toolbox->Delay(0);
		}
	}
	// 140:0328
	if (_sunMapRestored == 0) {
		_toolbox->PenNormal();
		Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		for (int16 i = 0; i <= 0x22; i++) {
			var_i32_692 = _toolbox->TickCount();
			_toolbox->InvertRect(area);
			delayFromMarker(1);
		}
		// 140:037e
		var_i16_484 = 0x96;

		for (int16 i = 0x37; i <= 0x122; i++) {
			if (i < 0x69) {
				var_i16_484 -= 3;
			}
			if (i > 0xf0) {
				var_i16_484 += 3;
			}
			_toolbox->MoveTo(var_i16_484 + 0x5f - _zbasic->rndInt(0x32), i);
			_toolbox->LineTo(0x1a1 - var_i16_484 + _zbasic->rndInt(0x32), i);
			if (i % 2 == 0) {
				_toolbox->Delay(0);
			}
		}
		// 140:03f0
		_zbasic->text(kFontLarge, 0x18, 0x10, kSrcBic);
		drawTextCenter(_zbasic->str(OFF(3)), 0x96); // you cannot claim
		drawTextCenter(_zbasic->str(OFF(4)), 0xb9); // the book of thoth
		drawTextCenter(_zbasic->str(OFF(5)), 0xdc); // so easily
		_toolbox->SetPort(var_i32_0);
		waitForClick();
	}
	// 140:0450
	sub_140_32ac();
	sub_140_3412();
	return;
}

void FoolGame::thoth99Enchantments() {
	// 140:0458
	toggleMouseCursor(false);
	if (_activePuzzleBuffer.empty()) { // was: str(OFF(6))
		arr_i16_1eb8[0] = 0x63;
	} else {
		arr_i16_1eb8[0] = _zbasic->decodeInt(_activePuzzleBuffer);
	}
	// 140:04a2
	if (arr_i16_1eb8[0] < 1) {
		arr_i16_1eb8[0] = 1;
	}
	if (arr_i16_1eb8[0] == 0x63) {
		_zbasic->text(kFontLarge, 0x18, 0x10, kSrcBic);
		drawTextCenter(_zbasic->str(OFF(7)), 0xaf); // you dare to challenge
		drawTextCenter(_zbasic->str(OFF(8)), 0xd2); // the high priestess
		delay(0xb4);
	}
	// 140:0532
	for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
		_screenGrid[i].top = puzzlesReadShort();
		_screenGrid[i].left = puzzlesReadShort();
		_screenGrid[i].bottom = _screenGrid[i].top + 0x28;
		_screenGrid[i].right = _screenGrid[i].left + 0x28;
	}
	// 140:060c
	_zbasic->picture(0, 0x14, var_pic_7c2);
	copyScreen(0, arr_bmp_b3ec);
	for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
		_toolbox->PenSize(5, 5);
		_toolbox->PenMode(kPatXor);
		for (int16 j = 0; j <= 1; j++) {
			_toolbox->MoveTo(0x104, 0xa2);
			_toolbox->LineTo(_screenGrid[i].left + 0x14, _screenGrid[i].top + 0x14);
			_toolbox->InvertRoundRect(_screenGrid[i], 0x14, 0x14);
			_toolbox->Delay(0);
		}
		_toolbox->PenNormal();
		// 140:06cc
		arr_i16_1eb8[2] = i;
		thothDrawEnchantment();
	}
	// 140:06fc
	_toolbox->PenNormal();
	sub_140_e3c();
	arr_i16_4758[4] = 0x14;
	arr_i16_4758[5] = 0;
	arr_i16_4758[6] = SCREEN_HEIGHT;
	arr_i16_4758[7] = SCREEN_WIDTH;
	arr_i16_1eb8[1] = 0;
	_zbasic->unk_20();
	toggleMouseCursor(true);
	_stateFlags = kStateNull;
	_activePuzzleSolved = false;
	var_i16_2326 = 0;
	var_i16_2328 = 0;

	// new value: number of updates before a screen flash
	arr_i16_1eb8[3] = 0;

	uint32 phase2Timer = _toolbox->TickCount();

	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 140:0752
		var_i32_692 = _toolbox->TickCount();
		arr_i16_1eb8[1]++;
		arr_i16_1eb8[3]++;
		if (arr_i16_1eb8[1] >= arr_i16_1eb8[0]) {
			arr_i16_1eb8[1] = 0;
		}
		// flash the screen. normally this happens after running through all the
		// remaining enchantments, so it increases in intensity as you clear them.
		// limit the amount of continuous full-screen flashing to WCAG
		// recommendation of 3/second
		if (arr_i16_1eb8[3] >= MAX(arr_i16_1eb8[0], (int16)10)) {
			arr_i16_1eb8[3] = 0;
			Common::Rect temp;
			temp.top = arr_i16_4758[4];
			temp.left = arr_i16_4758[5];
			temp.bottom = arr_i16_4758[6];
			temp.right = arr_i16_4758[7];
			_toolbox->InvertRect(temp);
		}
		// 140:07c4
		// Phase 1: enchantments 99 to 34 are inverting and don't move
		if (arr_i16_1eb8[0] >= 0x21) {
			if ((arr_i16_1eb8[1] % 0xa) == 0) {
				arr_i16_1eb8[2] = arr_i16_1eb8[0];
			} else {
				// 140:0826
				arr_i16_1eb8[2] = _zbasic->rndInt(arr_i16_1eb8[0]);
			}
			// 140:084a
			thothDrawEnchantment();
		// Phase 2: enchantments 33 to 2 erase themselves and move around
		} else {
			// 140:0852
			arr_i16_1eb8[2] = arr_i16_1eb8[0] - arr_i16_1eb8[1];
			if (arr_i16_1eb8[2] == arr_i16_1eb8[0]) {
				if ((var_i16_2328 == 1) && (_toolbox->TickCount() > (phase2Timer + 0x28))) {
					var_i16_2328 = 0;
				}
				if (var_i16_2328 == 0) {
					phase2Timer = _toolbox->TickCount();
					var_i16_2328 = 1;
					thothMoveEnchantment();
				}
				// 140:090c
			} else {
				// 140:0910
				thothMoveEnchantment();
			}
		}
		do {
			getNextEvent(-1);
			if ((_event.modifiers & kModMouseButtonUp) == 0) {
				sub_140_c4c();
			}
			// 140:0934
			if (var_i16_2326 != 0) {
				thothScrambleScreen();
			}
		} while (_event.what != 0);
		// 140:094a
		delayFromMarker(1);
	}
	// 140:097a
	if (_activePuzzleSolved) {
		sub_140_f84();
		_activePuzzleBuffer.clear(); // was: str(OFF(9))
	} else {
		// 140:09a4
		if (arr_i16_1eb8[0] < 1) {
			arr_i16_1eb8[0] = 1;
		}
		_activePuzzleBuffer = _zbasic->encodeInt(arr_i16_1eb8[0]);
	}
	// 140:09ee
}

void FoolGame::thothMoveEnchantment() {
	// 140:09f0
	_toolbox->FillRoundRect(_screenGrid[arr_i16_1eb8[2]], 0x14, 0x14, _patterns[2]);
	_screenGrid[arr_i16_1eb8[2]].top = _zbasic->rndInt(0x11a) + 0x14;
	_screenGrid[arr_i16_1eb8[2]].left = _zbasic->rndInt(0x1da);
	_screenGrid[arr_i16_1eb8[2]].bottom = _screenGrid[arr_i16_1eb8[2]].top + 0x28;
	_screenGrid[arr_i16_1eb8[2]].right = _screenGrid[arr_i16_1eb8[2]].left + 0x28;
	thothDrawEnchantment();
}

void FoolGame::thothDrawEnchantment() {
	// 140:0b2c
	_toolbox->PenNormal();
	_toolbox->EraseRoundRect(_screenGrid[arr_i16_1eb8[2]], 0x14, 0x14);
	_toolbox->FrameRoundRect(_screenGrid[arr_i16_1eb8[2]], 0x14, 0x14);
	_zbasic->text(kFontChicago, 0xc, 0, kSrcOr);
	Common::U32String label = Common::U32String::format(" %d ", arr_i16_1eb8[2]); // was: str(OFF(10))
	int16 width = _toolbox->StringWidth(label);
	// 140:0bd2
	_toolbox->MoveTo(
		_screenGrid[arr_i16_1eb8[2]].left + 0x14 - (width / 2),
		_screenGrid[arr_i16_1eb8[2]].top + 0x18
	);
	_toolbox->DrawString(label);
}

void FoolGame::sub_140_c4c() {
	// 140:0c4c
	if (_activePuzzleSolved)
		return;
	if (_toolbox->PtInRect(_event.where, _screenGrid[arr_i16_1eb8[0]])) {
		arr_i16_1eb8[0]--;
		if (arr_i16_1eb8[0] <= 0) {
			_activePuzzleSolved = true;
			return;
		}
		var_i16_484 = arr_i16_1eb8[0] + 1;
		_toolbox->FillRoundRect(_screenGrid[var_i16_484], 0x14, 0x14, _patterns[2]);
		zoomRect(
			_screenGrid[var_i16_484].top,
			_screenGrid[var_i16_484].left,
			_screenGrid[var_i16_484].bottom,
			_screenGrid[var_i16_484].right,
			0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatXor, 0xd
		);
		arr_i16_1eb8[1] = 0;
		sub_140_e3c();
		var_i16_2326 = 0;
		var_i16_2328 = 0;
	} else {
		// 140:0dcc
		var_i16_2326 = 1;
	}
}

void FoolGame::thothScrambleScreen() {
	// 140:0dd4
	if (var_i16_2326 < 0) {
		var_i16_2326 = 0;
	}
	_toolbox->InvertRect(thothRandomSquare());
	_toolbox->InvertRect(thothRandomHRect());
	_toolbox->InvertRect(thothRandomVRect());
	Common::Rect temp;
	temp.top = arr_i16_4758[4];
	temp.left = arr_i16_4758[5];
	temp.bottom = arr_i16_4758[6];
	temp.right = arr_i16_4758[7];
	_toolbox->InvertRect(temp);
	var_i16_2326++;
	if (var_i16_2326 >= 4)
		var_i16_2326 = 0;
}

void FoolGame::sub_140_e3c() {
	// 140:0e3c
	if (arr_i16_1eb8[0] == 1) {
		_zbasic->menu(8, 0, 1, _zbasic->str(OFF(11))); // the last enchantment
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(12))); // run for your life
	} else {
		// 140:0e8a
		var_str_384 = Common::U32String::format(" %d", arr_i16_1eb8[0]);
		var_str_9f4 = _zbasic->str(OFF(13)) + var_str_384 + _zbasic->str(OFF(14)); // the X enchantments
		_zbasic->menu(8, 0, 1, var_str_9f4);
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(15))); // run for your life
		_zbasic->menu(8, 2, 1, _zbasic->str(OFF(16))); // -
		_zbasic->menu(8, 3, 1, _zbasic->str(OFF(17))); // or press each button
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(18))); // in descending order
		var_str_9f4 = _zbasic->str(OFF(19)) + var_str_384 + _zbasic->str(OFF(20)); // from X to 1
		_zbasic->menu(8, 5, 1, var_str_9f4);
	}
	// 140:0f82
	return;
}

void FoolGame::sub_140_f84() {
	// 140:0f84
	_toolbox->PenNormal();
	_toolbox->PenSize(5, 5);
	arr_i16_4758[4] = (_screenGrid[1].top / 0x19) + 1;
	arr_i16_4758[5] = (_screenGrid[1].left / 0x19) + 1;
	// 140:0fea
	arr_i16_4758[6] = ((SCREEN_HEIGHT - _screenGrid[1].bottom) / 0x19) + 1;
	arr_i16_4758[7] = ((SCREEN_WIDTH - _screenGrid[1].right) / 0x19) + 1;
	// 140:1066
	// unrolled loop
	Common::Rect area = _screenGrid[1];
	// 140:10aa
	for (int16 i = 0; i <= 0x1a; i++) {
		area.top -= arr_i16_4758[4];
		area.left -= arr_i16_4758[5];
		area.bottom += arr_i16_4758[6];
		area.right += arr_i16_4758[7];
		_toolbox->FrameRect(area);
		delay(1);
	}
	// 140:11ae
	// unrolled loop
	area = _screenGrid[1];
	// 140:11f2
	for (int16 i = 0; i <= 0x1a; i++) {
		area.top -= arr_i16_4758[4];
		area.left -= arr_i16_4758[5];
		area.bottom += arr_i16_4758[6];
		area.right += arr_i16_4758[7];
		_toolbox->InvertRect(area);
		delay(1);
	}
	// 140:12f6
	sub_140_3296();
}

void FoolGame::thothKey1st() {
	// 140:12fc
	fillRect(0, 0, 0x14, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x14, var_pic_7c2);
	if ((var_i16_7ce & 2) != 0) {
		_toolbox->SetCursor(_cursors[0x10]);
	}
	// 140:1346
	_toolbox->PenSize(5, 5);
	_toolbox->PenMode(kPatXor);
	_event.where = Common::Point(0, 0);
	// 140:139a
	while (!(
		(_event.where.x >= 0xff) &&
		(_event.where.y >= 0xa2) &&
		(_event.where.x <= 0x10e) &&
		(_event.where.y <= 0xac)
	)) {
		// 140:1366
		getNextEvent(-1); // was: 0
		_toolbox->MoveTo(0x105, 0xa2);
		_toolbox->LineTo(_event.where.x, _event.where.y);
		delay(2);
		_toolbox->MoveTo(0x105, 0xa2);
		_toolbox->LineTo(_event.where.x, _event.where.y);
	}
	// 140:13dc
	_toolbox->PenNormal();
	if ((var_i16_7ce & 2) == 0) {
		_stateFlags = kStateReturn;
		zoomRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xa2, 0xff, 0xac, 0x10e, 1, kPatXor, 0x19);
		delay(0x3c);
		_activePuzzleSolved = false;
	} else {
		// 140:1446
		showBehold(0, 0, _zbasic->str(OFF(21)));
		thothSetupMenu();
		var_i16_7ce ^= 2;
		var_i16_7b2 = 0xa;
		_activePuzzleSolved = true;
	}
	// 140:1484
}

void FoolGame::thothKey2nd() {
	// 140:1486
	if (_activePuzzle != 0x34) {
		thothAdjustPuzzleData();
	}
	// FIXME: this looks lethal
	// 140:1494: MOVE.L - var_i32_232a,D0
	// 140:1498: ADDI.L - 0x0000018c,D0
	// 140:149e: MOVE.L - D0,var_i32_696
	_puzzleDataBuffer = var_bytes_232a;
	_puzzleDataPtr = var_ptr_232a + 0x18c;
	for (int16 j = 1; j <= 7; j++) {
		for (int16 i = 0; i <= 3; i++) {
			arr_i16_2f38[j*32+i] = puzzlesReadShort();
		}
	}

	_activePuzzleSolved = false;
	var_i16_103a = 0;
	_stateFlags = kStateNull;
	var_i16_232e = 0;
	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 140:1506
		// was: SetRect
		arr_i16_4758[4] = 0x14;
		arr_i16_4758[5] = 0;
		arr_i16_4758[6] = SCREEN_HEIGHT;
		arr_i16_4758[7] = SCREEN_WIDTH;
		Common::Rect playArea(0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		var_i16_232e++;
		if (var_i16_232e < 1) {
			var_i16_232e = 1;
		}
		if (var_i16_232e > 6) {
			var_i16_232e = 1;
		}
		// 140:1546
		switch (var_i16_232e-1) {
		case 0:
			// 140:31b0
			_toolbox->InvertRect(thothRandomSquare());
			break;
		case 1:
			// 140:31c4
			_toolbox->InvertRect(thothRandomHRect());
			break;
		case 2:
			// 140:31d8
			_toolbox->InvertRect(thothRandomVRect());
			_toolbox->InvertRect(playArea);
			break;
		case 3:
			// 140:31fa
			_toolbox->FillRect(thothRandomSquare(), _patterns[2]);
			break;
		case 4:
			// 140:321a
			_toolbox->FillRect(thothRandomHRect(), _patterns[2]);
			break;
		case 5:
			// 140:323a
			_toolbox->FillRect(thothRandomVRect(), _patterns[2]);
			break;
		default:
			warning("thothKey2nd: breaking out of switch statement");
			break;
		}
		if (_activePuzzle != 0x34) {
			Common::Rect temp;
			temp.top = arr_i16_2f38[(var_i16_103a + 1)*32];
			temp.left = arr_i16_2f38[(var_i16_103a + 1)*32+1];
			temp.bottom = arr_i16_2f38[(var_i16_103a + 1)*32+2];
			temp.right = arr_i16_2f38[(var_i16_103a + 1)*32+3];
			_toolbox->FillRect(temp, _patterns[1]);
		}
		// 140:159e
		getNextEvent(-1);
		if (_event.what == 1) {
			sub_140_15fc();
		}
		if (var_i16_103a == 4) {
			_activePuzzleSolved = true;
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}

		// 140:15d0
	}
	// 140:15fa
}

void FoolGame::sub_140_15fc() {
	// 140:15fc
	var_i16_7e4 = 0;
	for (int16 i = 1; i <= 7; i++) {
		Common::Rect temp;
		temp.top = arr_i16_2f38[i*32];
		temp.left = arr_i16_2f38[i*32+1];
		temp.bottom = arr_i16_2f38[i*32+2];
		temp.right = arr_i16_2f38[i*32+3];

		if (_toolbox->PtInRect(_event.where, temp)) {
			var_i16_7e4 = i;
		}
	}
	var_i16_103a++;
	if (var_i16_7e4 != var_i16_103a) {
		var_i16_103a = 0;
		playTone(0xf, 0x64, 0x1);
		for (int16 i = 0; i <= 9; i++) {
			Common::Rect temp;
			temp.top = arr_i16_4758[4];
			temp.left = arr_i16_4758[5];
			temp.bottom = arr_i16_4758[6];
			temp.right = arr_i16_4758[7];
			_toolbox->InvertRect(temp);
		}
	} else {
		// 140:1698
		if (_activePuzzle == 0x34) {
			for (int16 i = 0; i <= 7; i++) {
				Common::Rect temp;
				temp.top = arr_i16_2f38[32*i];
				temp.left = arr_i16_2f38[32*i+1];
				temp.bottom = arr_i16_2f38[32*i+2];
				temp.right = arr_i16_2f38[32*i+3];
				_toolbox->EraseRect(temp);
			}
			// 140:16ce
			for (int16 i = 1; i <= 0xc; i++) {
				for (int16 j = 1; j <= 7; j++) {
					Common::Rect temp;
					temp.top = arr_i16_2f38[32*j];
					temp.left = arr_i16_2f38[32*j+1];
					temp.bottom = arr_i16_2f38[32*j+2];
					temp.right = arr_i16_2f38[32*j+3];
					_toolbox->InvertRect(temp);
				}
				delay(2);
			}
			// 140:1710
			Common::Rect temp;
			temp.top = arr_i16_2f38[32*var_i16_7e4];
			temp.left = arr_i16_2f38[32*var_i16_7e4+1];
			temp.bottom = arr_i16_2f38[32*var_i16_7e4+2];
			temp.right = arr_i16_2f38[32*var_i16_7e4+3];
			for (int16 i = 1; i <= 0x19; i++) {
				_toolbox->InvertRect(temp);

			}
			// 140:173a
			if (var_i16_103a < 4) {
				var_i16_2330 = 2;
				var_i16_1dee = kPatXor;
			} else {
				// 140:1756
				var_i16_2330 = 3;
				var_i16_1dee = kPatCopy;
			}
			// 140:1762
			zoomRect(
				arr_i16_2f38[32*var_i16_7e4],
				arr_i16_2f38[32*var_i16_7e4+1],
				arr_i16_2f38[32*var_i16_7e4+2],
				arr_i16_2f38[32*var_i16_7e4+3],
				0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH,
				var_i16_2330, var_i16_1dee, 0x19
			);
		} else {
			// 140:1804
			for (int16 i = 0; i <= 3; i++) {
				arr_i16_4758[i] = arr_i16_2f38[32*var_i16_103a + i];
			}
			// 140:184e
			for (int16 i = 0; i <= 0x24; i++) {
				arr_i16_4758[0]++;
				arr_i16_4758[1]++;
				arr_i16_4758[2]--;
				arr_i16_4758[3]--;
				Common::Rect temp;
				temp.top = arr_i16_4758[0];
				temp.left = arr_i16_4758[1];
				temp.bottom = arr_i16_4758[2];
				temp.right = arr_i16_4758[3];
				_toolbox->InvertRect(temp);
			}
		}
	}
	// 140:18f8

}

void FoolGame::thothKey3rd() {
	// 140:18fa
	if (_activePuzzle != 0x34) {
		thothAdjustPuzzleData();
	}
	// FIXME: this looks lethal
	// 140:1908: MOVE.L - var_i32_232a,D0
	// 140:190c: ADDI.L - 0x000001c4,D0
	// 140:1912: MOVE.L - D0,var_i32_696
	_puzzleDataBuffer = var_bytes_232a;
	_puzzleDataPtr = var_ptr_232a + 0x1c4;
	for (int16 i = 0; i <= 3; i++) {
		for (int16 j = 0; j <= 3; j++) {
			arr_i16_2f38[i*32 + j] = puzzlesReadShort();
		}
	}
	// 140:195e
	for (int16 i = 0; i <= 3; i++) {
		_screenGrid[i].top = 0x14;
		_screenGrid[i].left = 0;
		_screenGrid[i].bottom = SCREEN_HEIGHT;
		_screenGrid[i].right = SCREEN_WIDTH;
	}
	// 140:19dc
	_screenGrid[0].right = 0xf;
	_screenGrid[1].left = 0x1f1;
	_screenGrid[2].bottom = 0x23;
	_screenGrid[3].top = 0x147;
	arr_i16_4338[1] = 0;
	arr_i16_4338[9] = 2;
	for (int16 i = 0x14; i <= 0x1a; i++) {
		arr_i16_4338[i - 0x12] = i;
		arr_i16_4338[0x1a - i + 0xa] = i;
	}
	// 140:1a9c
	var_i16_103a = 0;
	var_i16_232e = 0;
	_stateFlags = kStateNull;
	_activePuzzleSolved = false;
	// 140:1b98
	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 140:1ab8
		if (_activePuzzle == 0x34) { // in high priestess, no hints
			var_i16_232e++;
			if (var_i16_232e < 1) {
				var_i16_232e = 1;
			}
			if (var_i16_232e > 3) {
				var_i16_232e = 1;
			}
			switch (var_i16_232e-1) {
			case 0:
				// 140:325a
				_toolbox->InvertRect(thothRandomSquare());
				break;
			case 1:
				// 140:326e
				_toolbox->InvertRect(thothRandomHRect());
				break;
			case 2:
				// 140:3282
				_toolbox->InvertRect(thothRandomVRect());
				break;
			default:
				warning("thothKey3rd: broke out of switch");
				break;
			}
		} else { // in justice, animate the grab bar
			// 140:1b08
			var_i16_232e++;
			if (var_i16_232e < 1) {
				var_i16_232e = 1;
			}
			if (var_i16_232e > 0x10) {
				var_i16_232e = 1;
			}
			_toolbox->FillRect(_screenGrid[var_i16_103a], _patterns[arr_i16_4338[var_i16_232e]]);
		}
		// 140:1b66
		getNextEvent(-1);
		if (_event.what == 1) {
			thothKey3rdOnClick();
		}
		// 140:1b7a
		if (var_i16_103a == 4) {
			_activePuzzleSolved = true;
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
	// 140:1bc2
}

void FoolGame::thothKey3rdOnClick() {
	// 140:1bc4
	var_i16_2332 = 1;
	if (_activePuzzle == 0x34) {
		if ((_event.where.x >= 0xf) && (_event.where.y >= 0x2d) && (_event.where.x <= 0x1f1) && (_event.where.y <= 0x147)) {
			var_i16_2332 = 0;
		}
	} else {
		// 140:1c22
		if (_toolbox->PtInRect(_event.where, _screenGrid[var_i16_103a]) == 0) {
			var_i16_2332 = 0;
		}
	}
	// 140:1c58
	if (var_i16_2332 == 0) {
		sub_140_1f68();
		return;
	}
	// 140:1c66
	Common::Rect area(-1, 0x14, SCREEN_WIDTH + 1, SCREEN_HEIGHT + 1);
	if (_event.where.x < 0xf) {
		area.right = 0xf;
		var_i16_1de6 = 0;
	}
	// 140:1cc6
	if (_event.where.x > 0x1f1) {
		area.left = 0x1f1;
		var_i16_1de6 = 1;
	}
	// 140:1ce6
	if (_event.where.y < 0x2d) {
		area.bottom = 0x2d;
		var_i16_1de6 = 2;
	}
	// 140:1d06
	if (_event.where.y > 0x147) {
		area.top = 0x147;
		var_i16_1de6 = 3;
	}
	// 140:1d26
	while ((_event.modifiers & kModMouseButtonUp) == 0) {
		// 140:1d2a
		// was: 0
		getNextEvent(-1);
		if (var_i16_1de6 == 0) {
			area.right = _event.where.x + 0xa;
		}
		if (var_i16_1de6 == 1) {
			area.left = _event.where.x - 0xa;
		}
		if (var_i16_1de6 == 2) {
			area.bottom = _event.where.y + 0xa;
		}
		if (var_i16_1de6 == 3) {
			area.top = _event.where.y - 0xa;
		}
		// 140:1dc6
		if (area.top < 0x14) {
			area.top = 0x14;
		}
		_toolbox->FillRect(area, _patterns[arr_i16_2f38[var_i16_1de6*32+ var_i16_103a]]);
		// 140:1e2e
		if ((area.left <= 0) && (area.top <= 0x14) && (area.right >= SCREEN_WIDTH) && (area.bottom >= SCREEN_HEIGHT)) {
			_event.modifiers = kModMouseButtonUp;
		}
		// 140:1eae
	}
	// 140:1ec4
	if ((area.left <= 0) && (area.top <= 0x14) && (area.right >= SCREEN_WIDTH) && (area.bottom >= SCREEN_HEIGHT)) {
		if (var_i16_1de6 == var_i16_103a) {
			sub_140_205e();
			return;
		}
	}
	// 140:1f62
	sub_140_1f68();
}

void FoolGame::sub_140_1f68() {
	// 140:1f68
	playTone(0xf, 0x64, 1);
	var_i16_103a = 0;
	_toolbox->PenSize(0x14, 0x14);
	_toolbox->PenPat(_patterns[3]);
	Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 0; i <= 0x104; i += 0x14) {
		area.top = 0x14 + i;
		area.left = i;
		area.bottom = SCREEN_HEIGHT - i;
		area.right = SCREEN_WIDTH - i;
		_toolbox->FrameRect(area);
		_toolbox->Delay(0);
	}
	// 140:2056
	_toolbox->PenNormal();
	waitForMouseUp();
}

void FoolGame::sub_140_205e() {
	// 140:205e
	var_i16_103a++;
	waitForMouseUp();
	_toolbox->PenNormal();
}

void FoolGame::thothKeyLast() {
	// 140:206a
	_activePuzzleSolved = false;
	_stateFlags = kStateNull;
	thothKeyLastSetup();
	sub_140_2978();
	copyScreen(1, arr_bmp_b3ec);
	arr_i16_1eb8[1] = -1;
	var_i16_1de6 = arr_i16_3b38[_hermitPathStage];
	warning("thothKeyLast: 1de6: %d, target: %d", var_i16_1de6, (arr_i16_3b38[0x20 + _hermitPathStage] + 1));
	// JMP 0x228c
	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 140:20c2
		do {
			if (_activePuzzle != 0x34) { // we're in the hermit, print the shape
				int hermitPathStartIndex = arr_i16_3b38[_hermitPathStage]; // var_i16_2334
				int hermitPathEndIndex = arr_i16_3b38[_hermitPathStage + 0x20]; // var_i16_2336
				do {
					getNextEvent(-1);
					_toolbox->FillRect(_screenGrid[arr_i16_3738[hermitPathStartIndex]], _patterns[var_i16_2338]);
					hermitPathStartIndex++;
					// we're updating the screen in the event handler, this has no effect
					/*if (_hermitPathStage == 5) {
						if (hermitPathStartIndex % 4 == 0)
							_toolbox->Delay(0);
					} else {
						if (hermitPathStartIndex % 8 == 0)
							_toolbox->Delay(0);
					}*/
				// 140:2168
				} while (!((_event.what == 1) || (hermitPathStartIndex > hermitPathEndIndex) || ((_stateFlags & kStateReturn) != 0)));
				// 140:219e
				if ((_event.what == 0) && ((_stateFlags & kStateReturn) == 0)) {
					if (_hermitPathStage != 5) {
						_toolbox->Delay(0x7d*60/1000);
					} else {
						_toolbox->Delay(0xfa*60/1000);
					}
				}
				// 140:21e8
				copyScreen(1, arr_bmp_b3ec);
			} else { // we're in the high priestess, show nothing but random filled squares
				// 140:21fe
				var_i16_484 = _zbasic->rndInt(0x4e) + 2;
				var_i16_68a = _zbasic->rndInt(0x14) - 1;
				var_i16_68c = _zbasic->rndInt(0xe) - 1;
				// 140:2222
				_toolbox->FillRect(_screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]], _patterns[var_i16_484]);
				getNextEvent(-1);
			}
			if (_event.what == 1) {
				thothKeyLastOnClick();
			}
			if (_stateFlags == kStateSaveGame) {
				saveGame();
			}
			// 140:228c
		} while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved);
	}
}

void FoolGame::thothKeyLastOnClick() {
	// 140:22b8
	while (((_event.modifiers & kModMouseButtonUp) == 0) && (var_i16_1de6 != (arr_i16_3b38[0x20 + _hermitPathStage] + 1))) {
		// 140:22bc
		getNextEvent(-1); // was: 0
		getGridFromMouse(var_i16_68a, var_i16_68c);
		if ((var_i16_68a >= 0) && (var_i16_68c >= 0) && (var_i16_68a <= 0x13) && (var_i16_68c <= 0xd)) {
			// 140:2316
			arr_i16_1eb8[0] = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
			if (arr_i16_1eb8[0] != arr_i16_1eb8[1]) {
				// 140:2372
				if (arr_i16_1eb8[0] != arr_i16_3738[var_i16_1de6]) {
					thothBadSelect();
					return;
				}
				// 140:23aa
				arr_i16_1eb8[1] = arr_i16_1eb8[0];
				playTone(0x32 + (var_i16_1de6 * 5), 0x28, 0);
				_toolbox->FillRect(_screenGrid[arr_i16_1eb8[0]], _patterns[2]);
				var_i16_1de6++;
				warning("thothKeyLastOnClick: 1de6: %d, target: %d", var_i16_1de6, (arr_i16_3b38[0x20 + _hermitPathStage] + 1));
			}
		}
		// 140:2422
	}
	// 140:2472
	if (var_i16_1de6 != (arr_i16_3b38[0x20 + _hermitPathStage] + 1)) {
		thothBadSelect();
	} else {
		hermitNextStage();
	}
}

void FoolGame::thothBadSelect() {
	// 140:24ae
	playTone(0x14, 0x64, 0x0);
	while ((_event.modifiers & kModMouseButtonUp) == 0) {
		// 140:24c4
		getNextEvent(-1);
		var_i16_484 = _zbasic->rndInt(0x50);
		_toolbox->FillRect(_screenGrid[arr_i16_1eb8[0]], _patterns[var_i16_484]);
	}
	// 140:2520
	if (_activePuzzle != 0x34) {
		copyScreen(1, arr_bmp_b3ec);
	} else {
		for (int16 i = var_i16_1de6; i >= arr_i16_3b38[_hermitPathStage]; i--) {
			var_i16_484 = _zbasic->rndInt(0x4e) + 2;
			_toolbox->FillRect(_screenGrid[arr_i16_3738[i]], _patterns[var_i16_484]);
		}
	}
	// 140:25b4
	arr_i16_1eb8[1] = -1;
	var_i16_1de6 = arr_i16_3b38[_hermitPathStage];
}

void FoolGame::hermitNextStage() {
	// 140:25e4
	if (_activePuzzle == 0x34) {
		_activePuzzleSolved = true;
		return;
	}
	// 140:25f6
	if (_hermitPathStage < 5) {
		hermitScreenFlash();
	}
	if (_hermitPathStage == 4) {
		hermitScreenZoom();
	}
	if (_hermitPathStage == 5) {
		hermitScreenBehold();
	}
	if (_hermitPathStage == 6) {
		sub_140_2968();
	}
	if (!_activePuzzleSolved) {
		_hermitPathStage++;
		if (_hermitPathStage == 6) {
			thothKeyLastSetup();
		}
		sub_140_2978();
		copyScreen(1, arr_bmp_b3ec);
	}
	// 140:2660
}

void FoolGame::hermitScreenFlash() {
	// 140:2662
	Common::Rect area(0x37, 0x29, 0x1c9, 0x142);
	// limit the amount of continuous full-screen flashing to WCAG
	// recommendation of 3/second
	for (int16 i = 0; i <= 6; i++) { // was: 0x10
		_toolbox->InvertRect(area);
		delay(10); // was: 1
	}
}

void FoolGame::hermitScreenZoom() {
	// 140:26ca
	toggleMouseCursor(false);
	_toolbox->PenSize(0xa, 0x7);

	for (int16 j = 0; j <= 2; j++) {
		// 140:26e0
		Common::Rect area(0x2d, 0x22, 0x1d3, 0x149);
		for (int16 i = 0; i <= 0x14; i++) {
			area.top += 7;
			area.left += 0xa;
			area.bottom -= 7;
			area.right -= 0xa;
			_toolbox->InvertRect(area);
			delay(1);
		}
		for (int16 i = 0; i <= 0x13; i++) {
			area.top -= 7;
			area.left -= 0xa;
			area.bottom += 7;
			area.right += 0xa;
			_toolbox->InvertRect(area);
			delay(1);
		}
	}
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[1]);
	var_i16_1de6 = 0;
	toggleMouseCursor(true);
}

void FoolGame::hermitScreenBehold() {
	// 140:28bc
	if (_puzzleCompletionStatus[0x68] > 4) {
		waitForMouseUp();
		menuClickMessage();
		flashRect(0x29, 0x37, 0x142, 0x1c9, 0x96);
		_activePuzzleSolved = true;
	} else {
		// 140:290c
		zoomRect(0x14, 0xc8, 0x14, 0x138, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x21);
		// behold the last key of thoth
		showBehold(0x48, 2, _zbasic->str(OFF(22)));
	}
	// 140:2966
}

void FoolGame::sub_140_2968() {
	// 140:2968
	menuClickMessage();
	waitForClick();
	_activePuzzleSolved = true;
}

void FoolGame::sub_140_2978() {
	// 140:2978
	waitForMouseUp();
	if (_hermitPathStage < 6) {
		var_i16_2338 = 1;
	} else {
		var_i16_2338 = 2;
	}
}

void FoolGame::thothKeyLastSetup() {
	// 140:2998
	if (_hermitPathStage != 6) {
		fetchPuzzleData();
		arr_i16_1eb8[4] = 0x37;
		arr_i16_1eb8[5] = 0x29;
		arr_i16_1eb8[6] = 0x14;
		arr_i16_1eb8[7] = 0x14;
		arr_i16_1eb8[8] = 0x29;
		arr_i16_1eb8[9] = 0x12d;
		arr_i16_1eb8[10] = 0x37;
		arr_i16_1eb8[11] = 0x1bd;
		arr_i16_1eb8[12] = 0x15;
		arr_i16_1eb8[13] = 0x15;
		for (int16 i = 0; i <= 0x13; i++) {
			arr_i16_5bbc[i] = puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0xd; i++) {
			arr_i16_5cbc[i] = puzzlesReadShort();
		}
		// 140:2aa4
		for (int16 i = 0; i <= 0xcd; i++) {
			arr_i16_3738[i] = puzzlesReadShort();
			arr_i16_3738[i] = (arr_i16_3738[i]/4) + 1;
		}
		// 140:2b06
		for (int16 i = 1; i <= 6; i++) {
			arr_i16_3b38[i] = puzzlesReadShort();
			arr_i16_3b38[0x20 + i] = puzzlesReadShort();
		}
		// 140:2b54

	} else { // last key of thoth
		// 140:2b58
		if (_activePuzzle != 0x34) { // in the hermit
			thothAdjustPuzzleData();
		}
		// FIXME: some kind of manual bodge for the puzzle data loader,
		// this looks lethal
		// 140:2b66: MOVE.L - var_i32_232a,D0
		// 140:2b6a: ADDI.L - 0x000001e4,D0
		// 140:2b70: MOVE.L - D0,var_i32_696
		_puzzleDataBuffer = var_bytes_232a;
		_puzzleDataPtr = var_ptr_232a + 0x1e4;
		for (int16 i = 0; i <= 0xe; i++) {
			arr_i16_1eb8[i] = puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0x13; i++) {
			arr_i16_5bbc[i] = puzzlesReadShort();
		}
		for (int16 i = 0; i <= 0xd; i++) {
			arr_i16_5cbc[i] = puzzlesReadShort();
		}
		// 140:2bfe
		for (int16 i = 0x9c; i <= 0xcd; i++) {
			arr_i16_3738[i] = puzzlesReadShort();
		}
		arr_i16_3b38[6] = 0x9c;
		arr_i16_3b38[6+0x20] = 0xcd;
	}
	// 140:2c58
	var_i16_484 = 0;
	var_i16_68c = arr_i16_1eb8[8];
	do {
		var_i16_68a = arr_i16_1eb8[0xa];
		do {
			var_i16_484++;
			_toolbox->SetRect(
				_screenGrid[var_i16_484],
				var_i16_68a,
				var_i16_68c,
				var_i16_68a + arr_i16_1eb8[0xd],
				var_i16_68c + arr_i16_1eb8[0xc]
			);
			// 140:2ce4
		} while (_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[0xb], arr_i16_1eb8[6]));
	} while (_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	// 140:2d46

	var_i16_484 = 0;
	for (int16 j = 0; j <= 0xd; j++) {
		for (int16 i = 0; i <= 0x13; i++) {
			var_i16_484++;
			arr_i16_2f38[i*32 + j] = var_i16_484;
		}
	}
	// 140:2d94
	if (_hermitPathStage != 6) {
		_toolbox->PenPat(_patterns[1]);
		for (int16 j = 0; j <= 0x13; j++) {
			for (int16 i = 0; i <= 0xd; i++) {
				_toolbox->EraseRect(_screenGrid[arr_i16_2f38[arr_i16_5bbc[j]*32 + arr_i16_5cbc[i]]]);
				_toolbox->FrameRect(_screenGrid[arr_i16_2f38[arr_i16_5bbc[j]*32 + arr_i16_5cbc[i]]]);
				if (((i + j*0xd) % 6) == 0)
					_toolbox->Delay(0);
			}
		}
	} else {
		// 140:2e7a
		for (int16 k = 0; k <= 0x13; k++) {
			var_i16_68a = k;
			for (int16 j = 0; j <= 0xd; j++) {
				// 140:2e8c
				var_i16_484 = _zbasic->rndInt(0x4e) + 2;
				_toolbox->FillRect(
					_screenGrid[arr_i16_2f38[arr_i16_5bbc[var_i16_68a]*32 + arr_i16_5cbc[j]]],
					_patterns[var_i16_484]
				);
				if (((j + k*0xd) % 6) == 0)
					_toolbox->Delay(0);
				var_i16_68a++;
				if (var_i16_68a > 0x13) {
					var_i16_68a = 0;
				}
				// 140:2f12
			}
		}
	}
	// 140:2f2e
	if ((_hermitPathStage == 6) && (_activePuzzle != 0x34)) {
		_zbasic->menu(8, 0, 1, _zbasic->str(OFF(23))); // the last key of thoth
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(24))); // return to scroll
	}
	copyScreen(0, arr_bmp_b3ec);
}

void FoolGame::thothSetupMenu() {
	// 140:2f92
	_zbasic->menu(8, 0, 1, _zbasic->str(OFF(25))); // the book of thoth
	_zbasic->menu(8, 1, 1, _zbasic->str(OFF(26))); // return to scroll
	_zbasic->menu(8, 2, 0, _zbasic->str(OFF(27))); // -
	_zbasic->menu(8, 3, 1, _zbasic->str(OFF(28))); // be forewarned
	_zbasic->menu(8, 4, 1, _zbasic->str(OFF(29))); // the high priestess has corrupted
	_zbasic->menu(8, 5, 1, _zbasic->str(OFF(30))); // the book of thoth. you may need
	_zbasic->menu(8, 6, 1, _zbasic->str(OFF(31))); // the help of others to unlock its
	_zbasic->menu(8, 7, 1, _zbasic->str(OFF(32))); // secrets.
}

Common::Rect FoolGame::thothRandomSquare() {
	// 140:3050
	int16 x = _zbasic->rndInt(0x19c);
	int16 y = 0x14 + _zbasic->rndInt(0xde);
	return Common::Rect(
		x,
		y,
		x + 0x64,
		y + 0x64
	);
}

Common::Rect FoolGame::thothRandomHRect() {
	// 140:30da
	int16 y = 0x14 + _zbasic->rndInt(0xde);
	return Common::Rect(
		0,
		y,
		SCREEN_WIDTH,
		y + _zbasic->rndInt(0x64)
	);
}

Common::Rect FoolGame::thothRandomVRect() {
	// 140:3148
	int16 x = _zbasic->rndInt(0x19c);
	return Common::Rect(
		x,
		0x14,
		x + _zbasic->rndInt(0x64),
		SCREEN_HEIGHT
	);
}

void FoolGame::sub_140_3296() {
	// 140:3296
	if (!_screenOversized) {
		_toolbox->SetPort(var_i32_8_thoth);
	} else {
		sub_128_1ef8();
	}
	sub_140_32ac();
}

void FoolGame::sub_140_32ac() {
	// 140:33ac
	toggleMouseCursor(false);
	_toolbox->PenNormal();
	for (int16 j = 0; j <= 0x22; j++) {
		for (int16 i = -5; i <= 0x159; i += 0xa) {
			_toolbox->MoveTo(0, _zbasic->rndInt(0xa) + i);
			_toolbox->LineTo(SCREEN_WIDTH, _zbasic->rndInt(0xa) + i);
		}
	}
	// 140:3300
	Common::Rect area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 1; i <= 0xd; i++) {
		var_i32_692 = _toolbox->TickCount();
		_toolbox->InvertRect(area);
		delayFromMarker(1);
	}
	// 140:334a
	_toolbox->FillRect(area, _patterns[2]);
	toggleMouseCursor(true);
	_toolbox->SetPort(var_i32_0);
}

void FoolGame::thothAdjustPuzzleData() {
	// 140:3372
	// change puzzle data pointer to use high priestess data
	var_i16_484 = (_puzzleDataOffsets[0x34] - 1) / 1000;
	var_i16_7e4 = (_puzzleDataOffsets[0x34] - 1) % 1000;
	_zbasic->record(1, var_i16_484, var_i16_7e4);
	//_puzzleDataBuffer = arr_bytes_109dc;
	_puzzleDataBuffer = _zbasic->readFile(1, _puzzleDataOffsets[0x35] - _puzzleDataOffsets[0x34]);
	_puzzleDataPtr = 0;
	var_bytes_232a = _puzzleDataBuffer;
	var_ptr_232a = _puzzleDataPtr;
}

void FoolGame::sub_140_3412() {
	// 140:3412
	_toolbox->PenNormal();
	if ((var_i16_7ce & 2) == 0) {
		_toolbox->InitCursor();
	}
}

} // End of namespace Fool
