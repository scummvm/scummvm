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



// word jumble/substitute puzzle
void FoolGame::jumbleRun() {
	// 130:0004
	_activePuzzleSolved = false;
	fetchPuzzleData();
	_jumbleSubPuzzleCount = puzzlesReadShort();
	debugC(5, kDebugLoading, "FoolGame::jumbleRun: racking %d subpuzzles", _jumbleSubPuzzleCount);

	for (_jumbleCurrentSubPuzzle = 1; _jumbleCurrentSubPuzzle <= _jumbleSubPuzzleCount; _jumbleCurrentSubPuzzle++) {
		var_i16_105a = puzzlesReadShort();
		debugC(5, kDebugLoading, "FoolGame::jumbleRun: subpuzzle %d", _jumbleCurrentSubPuzzle);
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
		if ((var_i16_105a & 0x1) == 1) {
			for (int i = 0; i <= 0xe; i++) {
				arr_i16_1eb8[i] = puzzlesReadShort();
			}
		}
		// 130:0066
		var_i16_484 = 0;
		var_i16_68c = arr_i16_1eb8[8];
		do {
			var_i16_68a = arr_i16_1eb8[10];
			do {
				var_i16_484++;
				_toolbox->SetRect(
					_screenGrid[var_i16_484],
					var_i16_68a,
					var_i16_68c,
					var_i16_68a + arr_i16_1eb8[13],
					var_i16_68c + arr_i16_1eb8[12]
				);
				// 130:00f6
			} while (_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[11], arr_i16_1eb8[6]));
		// 130:0126
		} while (_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
		// 130:0156
		if ((var_i16_105a & 0x2) == 2) {
			var_i16_105c = puzzlesReadShort();
			var_i16_105e = puzzlesReadShort();
			var_i16_1060 = puzzlesReadShort();
			int16 picX = puzzlesReadShort();
			int16 picY = puzzlesReadShort();
			int16 picID = puzzlesReadShort();
			if (picID > 0) {
				arr_i32_192c0[0] = _toolbox->GetPicture(picID);
				_zbasic->picture(picX, picY, arr_i32_192c0[0]);
				_toolbox->ReleaseResource(arr_i32_192c0[0]);
			}
		}
		// 130:01f0
		_jumbleGameType = puzzlesReadShort();
		if (_jumbleGameType == 6) {
			for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
				for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
					arr_i16_2f38[i*32 + j] = puzzlesReadByte();
					arr_i16_3b38[i*32 + j] = 0;
				}
			}
		} else {
		// 130:028a
			for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
				for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
					arr_i16_2f38[i*32 + j] = 0;
					arr_i16_3b38[i*32 + j] = 0;
				}
			}
		}
		// 130:0306
		var_i16_103a = puzzlesReadShort();
		if (var_i16_103a > 0) {
			_toolbox->PenNormal();
			_toolbox->PenPat(_patterns[var_i16_1060]);
			var_i16_484 = 0;
			for (int j = 1; j <= var_i16_103a; j++) {
				var_i16_106a = puzzlesReadShort();
				if (var_i16_106a == 0) {
					var_i16_484 += 0x10;
				} else {
				// 130:035a
					for (int i = 0; i <= 0xf; i++) {
						var_i16_484++;
						if (var_i16_106a & _bitLUT[i]) {
							_toolbox->PaintRect(_screenGrid[var_i16_484]);
						}
					}
					// 130:039e

				}
				// 103:03aa
			}
		}
		// 130:03ba
		if (var_i16_105e == 2) {
			var_i16_106c = kSrcBic;
		} else {
			var_i16_106c = kSrcOr;
		}
		// 130:03d4
		if (_jumbleGameType == 4) {
			fillRect(0x63, 0xae, 0x108, 0x153, 2);
		}
		// 130:03fe
		//if (_activePuzzleStatus <= _jumbleCurrentSubPuzzle)
		_jumblePosition = 0;
		var_str_1070.clear(); // was: str(168)
		var_str_1170.clear(); // was: str(169)
		var_i16_1270 = puzzlesReadShort();
		for (var_i16_103a = 1; var_i16_103a <= var_i16_1270; var_i16_103a++) {
			Common::U32String data = puzzlesReadString().decode(Common::kMacRoman);
			var_i16_1372 = data.size();
			var_str_1170 += data;
			// 130:047e
			if (_activePuzzleStatus > _jumbleCurrentSubPuzzle) {
				var_str_1070 += data;
			} else {
				// 130:04ac
				if ((_activePuzzleStatus == _jumbleCurrentSubPuzzle) && !_activePuzzleBuffer.empty()) { // was: str(170)
					// 130:04dc
					var_str_1070 = _activePuzzleBuffer;
				} else {
					// 130:04f0
					if ((_jumbleGameType == 1) || (_jumbleGameType == 5)) {
						// 130:0514
						var_str_384 = data;
						sub_130_25d8();
					}
					// 130:0528
					if (_jumbleGameType == 2) {
						var_str_384 = data;
						sub_130_2790();
					}
					// 130:0548
					if ((_jumbleGameType == 3) || (_jumbleGameType == 6)) {
						var_str_9f4 = _zbasic->space(var_i16_1372);
					}
					// 130:0580
					// word square
					if (_jumbleGameType == 4) {
						var_str_384 = _zbasic->leftStr(data, 4);
						sub_130_25d8();
						var_str_1374 = var_str_9f4;
						var_str_384 = _zbasic->rightStr(data, 4);
						sub_130_25d8();
						var_str_1474 = var_str_9f4;
						var_str_9f4 = var_str_1374 + _zbasic->midStr(data, 5, 1) + var_str_1474;
					}
					// 130:0610
					var_str_1070 += var_str_9f4;
				}
			}
			// 130:0628
			for (var_i16_484 = 1; var_i16_484 <= var_i16_1372; var_i16_484++) {
				_jumblePosition++;
				arr_i16_3738[_jumblePosition] = puzzlesReadShort();
				_jumbleSelected = ABS(arr_i16_3738[_jumblePosition]);
				sub_130_20fe();
				arr_i16_2f38[var_i16_68a*32 + var_i16_68c] = var_i16_103a;
				arr_i16_3b38[var_i16_68a*32 + var_i16_68c] = _jumblePosition;
				sub_130_1c6c();
				var_str_d12 = _zbasic->midStr(var_str_1070, _jumblePosition, 1);
				// 130:06d4
				if ((_activePuzzle <= 0x50) || (_activePuzzle == 0x55)) {
					jumbleDrawLetter();
				} else {
					// 130:0700
					sub_130_1e76();
				}
			}
			// 130:0716
			_zbasic->indexRawSet(puzzlesReadString(), 1, var_i16_103a);
		}
		// 130:073e
		var_i16_1576 = var_str_1170.size();
		if ((_activePuzzle == 0x51) || (_activePuzzle == 0x54)) {
			// 130:076e
			_toolbox->PenNormal();
			_toolbox->PenPat(_patterns[1]);
			_toolbox->PenSize(3, 3);
			if (_activePuzzle == 0x51) {
				// 130:0794
				_toolbox->MoveTo(0xf3, 0x6f);
				_toolbox->LineTo(0xf3, 0xe4);
				_toolbox->MoveTo(0x9b, 0xa9);
				_toolbox->LineTo(0x14c, 0xa9);
			}
			// 130:07bc
			if (_activePuzzle == 0x54) {
				_toolbox->MoveTo(0xe9, 0x81);
				_toolbox->LineTo(0x17d, 0x81);
				_toolbox->MoveTo(0x124, 0x47);
				_toolbox->LineTo(0x124, 0xda);
			}
			// 130:07f0
			_toolbox->PenNormal();
		}
		// 130:07f2
		var_i16_103a = puzzlesReadShort();
		if (var_i16_103a > 0) {
			for (var_i16_68a = 1; var_i16_68a <= var_i16_103a; var_i16_68a++) {
				var_i16_484 = puzzlesReadShort();
				var_i16_7e4 = puzzlesReadShort();
				var_str_1578 = puzzlesReadString().decode(Common::kMacRoman);
				_zbasic->text(kFontFool, 0xc, 0, kSrcOr);
				if (var_i16_7e4 > 0) {
					_toolbox->MoveTo(var_i16_484, var_i16_7e4);
					_toolbox->DrawString(var_str_1578);
				} else {
					sub_130_2226();
				}
			}
		}
		// 130:086a
		var_i16_1678 = 0;
		var_str_1578.clear(); // was: str(171);
		if (_jumbleCurrentSubPuzzle == _jumbleSubPuzzleCount) {
			// all puzzles done, load the success animation list
			arr_i16_4338[0] = puzzlesReadShort();
			if (arr_i16_4338[0] > 0) {
				// 130:08be
				for (int i = 1; i <= arr_i16_4338[0]; i++) {
					arr_i16_4338[i] = puzzlesReadShort();
				}
			}
		}
		// 130:08fa
		if (_activePuzzleStatus <= _jumbleCurrentSubPuzzle) {
			switch (_jumbleGameType-1) {
			case 0:
				sub_130_d2e();
				break;
			case 1:
				jumbleRunSubstitution();
				break;
			case 2:
				sub_130_1004();
				break;
			case 3:
			case 4:	// word square
				jumbleRunWordSquare();
				break;
			case 5: // hidden message
				jumbleRunHiddenMessage();
				break;
			default:
				break;
			}
			if ((_stateFlags & kStateReturn) != 0) {
				jumbleStoreState();
				_jumbleCurrentSubPuzzle = _jumbleSubPuzzleCount;
			} else {
				// 130:0956
				if (_activePuzzle <= 0x50) {
					if ((var_i16_1678 > 0) && (!var_str_1578.empty())) { // was: str(172)
						// 130:098c
						var_str_1578.clear(); // was: str(173)
						sub_130_2226();
					}
					// 130:09a4
					for (int i = 1; i <= 3; i++) {
						playTone(_zbasic->rndInt(0x1f4) + 0x19, 0x42, 0);
					}
					for (int j = 0; j <= 1; j++) {
						for (int i = 1; i <= var_i16_1576; i++) {
							_toolbox->Delay((0xc - (var_i16_1576 / 0xa))*60/1000);
							_toolbox->InvertRect(_screenGrid[ABS(arr_i16_3738[i])]);
						}
					}
					if (_jumbleCurrentSubPuzzle == _jumbleSubPuzzleCount) {
						_activePuzzleSolved = true;
					}
				}
			}
		}
		// 130:0a5c
	}
	if ((_stateFlags & kStateReturn) != 0) {
		// JMP 0x2962
		return;
	}
	// if we're here, the puzzle is solved
	_toolbox->PenNormal();
	menuClickMessage();
	waitForMouseUp();
	sub_128_61ec();
	// 130:0a90
	if (_activePuzzle != 0x55) {
		// 130:0b68
		while ((_event.modifiers & kModMouseButtonUp) != 0) {
			// 130:0aa0
			if (arr_i16_4338[0] > 0) {
				// strobe the squares in the victory list
				if (arr_i16_4338[0] == 1) {
					// originally was 0x64, lock to 200ms
					var_i16_7e4 = 0xc8;
				} else {
					// originally was 0xd, lock to 2 frames
					var_i16_7e4 = 0x22;
				}
				var_i16_68a = 0;
				do {
					int ticks = (int)_toolbox->TickCount();
					var_i16_68a++;
					_toolbox->InvertRect(_screenGrid[arr_i16_4338[var_i16_68a]]);
					// mask was originally 0
					getNextEvent(-1);
					int delta = MAX(0, var_i16_7e4*60/1000 - ((int)_toolbox->TickCount() - ticks));
					_toolbox->Delay(delta);
				} while (!((var_i16_68a == arr_i16_4338[0]) || ((_event.modifiers & kModMouseButtonUp) == 0)));
			} else {
			// 130:0b62
				// mask was originally 0
				getNextEvent(-1);
			}
		}
	// 130:0b7a
	} else {
		// 130:0b7e
		fillRect(0x131, 0, 0x156, 0x10e, 0x47);
		_activePuzzleSolved = false;
		var_i16_bfc = 0;
		do {
			for (int i = 1; i <= arr_i16_4338[0]; i++) {
				_toolbox->Delay(0xd*60/1000);
				_toolbox->InvertRect(_screenGrid[arr_i16_4338[i]]);
				getNextEvent(0);
				if ((_event.modifiers & kModMouseButtonUp) == 0) {
					_activePuzzleSolved = true;
				}
			}
			// 130:0c14
			if (var_i16_bfc == 0) {
				var_i16_bfc = 1;
			} else {
				var_i16_bfc = 0;
			}
		} while (!((_activePuzzleSolved) && (var_i16_bfc == 1)));
	}
	// 130:0c4e
	jumbleStoreState();
}

void FoolGame::sub_130_c56() {
	warning(__func__);
	// 130:0c56
	jumbleStoreState();
	saveGame();
	_stateFlags = kStateNull;
}

void FoolGame::jumbleStoreState() {
	// 130:0c66
	if (_activePuzzleSolved) {
		_activePuzzleStatus = 0x64;
		_activePuzzleBuffer.clear(); // was: str(174)
		_activePuzzleSolved = false;
	} else {
		// 130:0c98
		if (_activePuzzleStatus < 0x64) {
			_activePuzzleStatus = _jumbleCurrentSubPuzzle;
			if ((_jumbleGameType == 2) && (!var_str_1578.empty())) { // was: str(175)
				_activePuzzleBuffer = var_str_1070 + _zbasic->leftStr(var_str_1578, var_i16_167a-2);
			} else {
				// 130:0cfe
				_activePuzzleBuffer = var_str_1070;
			}
			// 130:0d0e
		} else {
			// 130:0d12
			_activePuzzleStatus = 0x65;
			_activePuzzleBuffer.clear(); // was: str(176)
		}
	}
	// 130:0d2c
	return;
}

void FoolGame::sub_130_d2e() {
	warning(__func__);
	// 130:0d2e
	sub_130_1426();
	sub_128_61ec();
	_stateFlags = kStateNull;
	while (((_stateFlags & kStateReturn) == 0) && (var_str_1070 != var_str_1170)) {
		getNextEvent(-1);
		if (_event.what == kMouseDown) {
			jumbleOnClick();
		}
		if (_stateFlags == kStateSaveGame) {
			sub_130_c56();
		}
	}
}

void FoolGame::jumbleRunSubstitution() {
	warning(__func__);
	// 130:0d90
	// substitution cipher
	if (!_zbasic->index(1, 1).empty()) { // was: str(177)
		if (var_str_1070.size() == var_str_1170.size()) {
			var_i16_1372 = _zbasic->index(1, 1).size();
			var_str_1578 = _zbasic->space(var_i16_1372);
			for (int i = 1; i<= var_i16_1372; i++) {
				var_str_167c = _zbasic->midStr(_zbasic->index(1, 1), i, 1);
				var_i16_7e4 = _zbasic->asc(_zbasic->ucase(var_str_167c)) - 0x40;
				if ((var_i16_7e4 >= 1) && (var_i16_7e4 <= 0x1a)) {
					var_str_d12 = _zbasic->chr(arr_i16_5cbc[var_i16_7e4] + 0x40);
				} else {
					// 130:0e88
					var_str_d12 = var_str_167c;
				}
				// 130:0e98
				_zbasic->midStrSet(var_str_1578, i, 1, var_str_d12);
			}

		} else {
			// 130:0ece
			var_i16_484 = var_str_1070.size();
			var_i16_7e4 = var_str_1170.size();
			var_str_1578 = _zbasic->rightStr(var_str_1070, var_i16_484 - var_i16_7e4);
			var_str_1070 = _zbasic->leftStr(var_str_1070, var_i16_7e4);

		}
		// 130:0f24
		var_str_1578 += Common::U32String(" = ") + _zbasic->index(1, 1); // ABC = XYZ
		var_i16_167a = _zbasic->instr(1, var_str_1578, Common::U32String("=")); // was: str(179)
		sub_130_2226();
	} else {
		// 130:0f72
		var_str_1578.clear(); // was: str(180);
	}
	// 130:0f86
	_jumblePosition = 1;
	sub_130_1c1a();
	sub_128_61ec();
	_stateFlags = kStateNull;
	_keyLastPressed = 0;
	// 130:0fa0
	while (((_stateFlags & kStateReturn) == 0) && (var_str_1070 != var_str_1170)) {
		// 130:0fa4
		getNextEvent(-1);
		if (_keyLastPressed != 0) {
			jumbleOnKey();
		}
		if (_event.what == kMouseDown) {
			jumbleOnClick();
		}
		if (_stateFlags == kStateSaveGame) {
			sub_130_c56();
		}
	}
	// 130:0ffe
	jumbleSelectSquare();
}

void FoolGame::sub_130_1004() {
	warning(__func__);
	// 130:1004
	_jumblePosition = _zbasic->instr(1, var_str_1070, Common::U32String(" ")); // was: str(181)
	if (_jumblePosition == 0) {
		_jumblePosition = 1;
	}
	sub_130_20d2();
	sub_128_61ec();
	_stateFlags = kStateNull;
	_keyLastPressed = 0;
	// 130:1042
	while (((_stateFlags & kStateReturn) == 0) && (var_str_1070 != var_str_1170)) {
		getNextEvent(-1);
		if (_keyLastPressed != 0) {
			jumbleOnKey();
		}
		if (_event.what == kMouseDown) {
			jumbleOnClick();
		}
		if (_stateFlags == kStateSaveGame) {
			sub_130_c56();
		}
	}
	jumbleSelectSquare();
}

void FoolGame::jumbleRunWordSquare() {
	warning(__func__);
	// 130:10a6
	// word square
	sub_130_1426();
	if (_jumbleGameType != 5) {
		var_str_177c = _zbasic->midStr(var_str_1170, 1, 1) + _zbasic->midStr(var_str_1170, 4, 1) + _zbasic->midStr(var_str_1170, 7, 1);
		var_str_177c += _zbasic->midStr(var_str_1170, 2, 1) + _zbasic->midStr(var_str_1170, 5, 1) + _zbasic->midStr(var_str_1170, 8, 1);
		var_str_177c += _zbasic->midStr(var_str_1170, 3, 1) + _zbasic->midStr(var_str_1170, 6, 1) + _zbasic->midStr(var_str_1170, 9, 1);
	} else {
		// 130:1196
		var_str_177c = _zbasic->midStr(var_str_1170, 3, 2) + _zbasic->midStr(var_str_1170, 1, 2) + _zbasic->midStr(var_str_1170, 5, 1);
		var_str_177c += _zbasic->midStr(var_str_1170, 8, 2) + _zbasic->midStr(var_str_1170, 6, 2);
	}
	// 130:1212
	sub_128_61ec();
	_stateFlags = kStateNull;
	// 130:121c
	while (((_stateFlags & kStateReturn) == 0) && (var_str_1070 != var_str_1170) && (var_str_1070 != var_str_177c)) {
		getNextEvent(-1);
		if (_event.what == kMouseDown) {
			jumbleOnClick();
		}
		if (_stateFlags == kStateSaveGame) {
			sub_130_c56();
		}
	}
}

void FoolGame::jumbleRunHiddenMessage() {
	warning(__func__);
	// 130:1282
	// hidden message, change the mouse cursor
	_jumblePosition = _zbasic->instr(1, var_str_1070, Common::U32String(" ")); // was: str(182)
	if (_jumblePosition == 0) {
		_jumblePosition = 1;
	}
	sub_130_20d2();
	sub_128_61ec();
	var_i16_7b2 = 0;
	_stateFlags = kStateNull;
	_keyLastPressed = 0;
	_toolbox->InitCursor();
	while (((_stateFlags & kStateReturn) == 0) && (var_str_1070 != var_str_1170)) {
		// 130:12cc
		getNextEvent(-1);
		if (_keyLastPressed != 0) {
			jumbleOnKey();
		}
		getGridFromMouse(var_i16_68a, var_i16_68c);
		if (_event.what == kMouseDown) {
			jumbleOnClick();
		}
		if ((var_i16_68a < 1) || (var_i16_68a > arr_i16_1eb8[0]) || (var_i16_68c < 1) || (var_i16_68c > arr_i16_1eb8[1])) {
			var_i16_187c = 0;
		} else {
			var_i16_187c = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
		}
		// 130:139e
		if (var_i16_187c != var_i16_7b2) {
			if (var_i16_187c < 0x11) {
				_toolbox->InitCursor();
			} else {
				_toolbox->SetCursor(_cursors[var_i16_187c]);
			}
			// 130:13dc
			var_i16_7b2 = var_i16_187c;
		}
		// 130:13e2
		var_i16_7b2 = var_i16_187c;
		if (_stateFlags == kStateSaveGame) {
			sub_130_c56();
		}
	}
	// 130:141e
	jumbleSelectSquare();
	_toolbox->InitCursor();
}

void FoolGame::sub_130_1426() {
	warning(__func__);
	// 130:1426
	var_i16_187e = 0;
	var_i16_1880 = 0;
	if (!_zbasic->index(1, 1).empty()) { // was: str(183)
		var_i16_1678 = 1;
		var_str_1578 = _zbasic->index(1, 1);
		sub_130_2226();
	}
	// 130:1474
}

void FoolGame::jumbleOnClick() {
	warning(__func__);
	// 130:1476
	getGridFromMouse(var_i16_68a, var_i16_68c);
	if ((var_i16_68a < 1) || (var_i16_68a > arr_i16_1eb8[0]) || (var_i16_68c < 1) || (var_i16_68c > arr_i16_1eb8[1])) {
		// 130:14fa
		if (_event.where.x > 0x14) {
			playTone(0xf, 0x42, 0x1);
		}
		return;
	}
	// 130:1518
	if (arr_i16_2f38[var_i16_68a*32 + var_i16_68c] < 1) {
		playTone(0xf, 0x42, 0x1);
		return;
	}
	// 130:1558
	if ((_jumbleGameType == 6) && (arr_i16_2f38[var_i16_68a*32 + var_i16_68c] >= 0x11)) {
		playTone(0xf, 0x42, 0x1);
		return;
	}
	// 130:15b0
	switch (_jumbleGameType - 1) {
	case 0:
		sub_130_172c();
		break;
	case 1:	// the knight of wands
	case 2:
	case 6:
		sub_130_20aa();
		break;
	case 3:
	case 4:	// word square
		sub_130_172c();
		break;
	default:
		warning("jumbleOnClick: unhandled case %d", _jumbleGameType-1);
		break;
	}
}

void FoolGame::jumbleOnKey() {
	warning(__func__);
	// 130:15da
	if ((_keyLastPressed == 3) || (_keyLastPressed == 0xd)) {
		jumbleSelectSquare();
		jumbleNextPosition();
		sub_130_20d2();
		_keyLastPressed = 0;
		return;
	}
	if (_keyLastPressed == 8) {
		jumbleSelectSquare();
		jumblePreviousPosition();
		sub_130_20d2();
		_keyLastPressed = 0;
		return;
	}
	// 130:1632
	if ((_keyLastPressed >= 0x61) && (_keyLastPressed <= 0x7a)) {
		_keyLastPressed -= 0x20;
	}
	if (_jumbleGameType != 3) {
		if ((_keyLastPressed < 0x41) || (_keyLastPressed > 0x5a)) {
			return;
		}
	} else {
		// 130:1690
		var_i16_1882 = 0;
		if ((_keyLastPressed < 0x41) || (_keyLastPressed > 0x5a)) {
			var_i16_1882 = 1;
		}
		if (_keyLastPressed == 0x20) {
			var_i16_1882 = 0;
		}
		if (var_i16_1882 != 0)
			return;
	}
	// 130:16dc
	var_str_d12 = _zbasic->chr(_keyLastPressed);
	// 130:16f2
	_keyLastPressed = 0;
	if (_jumbleGameType != 2) {
		// 130:1706
		if (!((_jumbleGameType == 3) || (_jumbleGameType == 6))) {
			sub_130_172c();
		} else {
			sub_130_201a();
		}
	} else {
		sub_130_1a16();
	}
}

void FoolGame::sub_130_172c() {
	warning(__func__);
	// 130:172c
	// word square, disallow clicking the centre tile
	if ((_jumbleGameType == 4) && (arr_i16_3b38[var_i16_68a*32 + var_i16_68c] == 5)) {
		jumbleClickFixedSquare();
		return;
	}
	// 130:1776
	if (!((var_i16_1678 == arr_i16_2f38[var_i16_68a*32 + var_i16_68c]) && (var_i16_187e > 0))) {
		// 130:17c4
		sub_130_19ac();
		var_i16_187e = ABS(arr_i16_3738[arr_i16_3b38[var_i16_68a*32 + var_i16_68c]]);
		var_i16_1884 = arr_i16_3b38[var_i16_68a*32 + var_i16_68c];
		_toolbox->InvertRect(_screenGrid[var_i16_187e]);
		sub_130_2178();
		return;
	}
	// 130:1844
	var_i16_1880 = ABS(arr_i16_3738[arr_i16_3b38[var_i16_68a*32 + var_i16_68c]]);
	if (var_i16_187e == var_i16_1880) {
		sub_130_19ac();
		return;
	}
	// 130:189a
	_toolbox->InvertRect(_screenGrid[var_i16_1880]);
	var_i16_1886 = arr_i16_3b38[var_i16_68a*32 + var_i16_68c];
	var_str_1374 = _zbasic->midStr(var_str_1070, var_i16_1884, 1);
	var_str_1474 = _zbasic->midStr(var_str_1070, var_i16_1886, 1);
	_zbasic->midStrSet(var_str_1070, var_i16_1884, 1, var_str_1474);
	_zbasic->midStrSet(var_str_1070, var_i16_1886, 1, var_str_1374);
	_jumbleSelected = var_i16_187e;
	_jumblePosition = var_i16_1884;
	var_str_d12 = var_str_1474;
	sub_130_1c6c();
	jumbleDrawLetter();
	_jumbleSelected = var_i16_1880;
	_jumblePosition = var_i16_1886;
	var_str_d12 = var_str_1374;
	sub_130_1c6c();
	jumbleDrawLetter();
	var_i16_187e = 0;
	var_i16_1880 = 0;
}

void FoolGame::sub_130_19ac() {
	warning(__func__);
	// 130:19ac
	if (var_i16_187e > 0) {
		_toolbox->InvertRect(_screenGrid[var_i16_187e]);
	}
	var_i16_187e = 0;
	var_i16_1880 = 0;
}

void FoolGame::jumbleClickFixedSquare() {
	warning(__func__);
	// 130:19da
	// fixed tile
	sub_130_19ac();
	playTone(0x19, 0x64, 0);
	for (int i = 0; i <= 0x19; i++) {
		_toolbox->InvertRect(_screenGrid[13]);
		delay(1);
	}
}

void FoolGame::sub_130_1a16() {
	warning(__func__);
	// 130:1a16
	var_str_1374 = var_str_d12;
	var_str_1474 = _zbasic->midStr(var_str_1070, _jumblePosition, 1);
	if (var_str_1374 == var_str_1474)
		return;
	// 130:1a5c
	jumbleSelectSquare();
	for (int16 i = 1; i <= var_i16_1576; i++) {
		// 130:1a66
		var_str_384 = _zbasic->midStr(var_str_1070, i, 1);
		if ((var_str_384 == var_str_1374) || (var_str_384 == var_str_1474)) {
			if (var_str_384 == var_str_1374) {
				var_str_d12 = var_str_1474;
			} else {
				// 130:1ad8
				var_str_d12 = var_str_1374;
			}
			// 130:1ae8
			_zbasic->midStrSet(var_str_1070, i, 1, var_str_d12);
			_jumbleSelected = ABS(arr_i16_3738[i]);
			sub_130_1c6c();
			jumbleDrawLetter();
		}
		// 130:1b2e
	}
	// 130:1b40
	if (!var_str_1578.empty()) { // was: str(184)
		for (int16 i = 1; i <= var_i16_167a - 2; i++) {
			var_str_d12 = _zbasic->midStr(var_str_1578, i, 1);
			if ((var_str_d12 == var_str_1374) || (var_str_d12 == var_str_1474)) {
				// 130:1ba8
				if (var_str_d12 == var_str_1374) {
					var_str_d12 = var_str_1474;
				} else {
					// 130:1bd2
					var_str_d12 = var_str_1374;
				}
				// 130:1be2
				_zbasic->midStrSet(var_str_1578, i, 1, var_str_d12);
			}
			// 130:1c02
		}
		sub_130_2226();
	}
	// 130:1c1a
	sub_130_1c1a();
}

void FoolGame::sub_130_1c1a() {
	warning(__func__);
	// 130:1c1a
	_jumbleSelected = ABS(arr_i16_3738[_jumblePosition]);
	_toolbox->InvertRect(_screenGrid[_jumbleSelected]);
}

void FoolGame::jumbleSelectSquare() {
	warning(__func__);
	// 130:1c52
	_toolbox->InvertRect(_screenGrid[_jumbleSelected]);
}

void FoolGame::sub_130_1c6c() {
	warning(__func__);
	_toolbox->PenNormal();
	_toolbox->FillRect(_screenGrid[_jumbleSelected], _patterns[var_i16_105e]);
	_toolbox->PenPat(_patterns[var_i16_105c]);
	_toolbox->FrameRect(_screenGrid[_jumbleSelected]);
	if (arr_i16_3738[_jumblePosition] >= 0) {
		return;
	}
	// 130:1ce2
	if ((_activePuzzle > 0x50) && (_activePuzzle < 0x55)) {
		return;
	}
	arr_i16_4758[0] = _screenGrid[_jumbleSelected].top + 2;
	arr_i16_4758[1] = _screenGrid[_jumbleSelected].left + 2;
	arr_i16_4758[2] = _screenGrid[_jumbleSelected].bottom - 2;
	arr_i16_4758[3] = _screenGrid[_jumbleSelected].right - 2;
	Common::Rect temp;
	temp.top = arr_i16_4758[0];
	temp.left = arr_i16_4758[1];
	temp.bottom = arr_i16_4758[2];
	temp.right = arr_i16_4758[3];
	_toolbox->FrameRect(temp);
}

void FoolGame::jumbleDrawLetter() {
	warning(__func__);
	// 130:1dcc
	_zbasic->text(kFontPuzzle, arr_i16_1eb8[0xe], 0, var_i16_106c);
	_toolbox->MoveTo(
		_screenGrid[_jumbleSelected].left + arr_i16_1eb8[2],
		_screenGrid[_jumbleSelected].top + arr_i16_1eb8[3]
	);
	_toolbox->DrawString(var_str_d12);
}

void FoolGame::sub_130_1e5c() {
	warning(__func__);
	// 130:1e5c
	_toolbox->PenNormal();
	_toolbox->InvertRect(_screenGrid[_jumbleSelected]);
	sub_130_1e76();
}

void FoolGame::sub_130_1e76() {
	warning(__func__);
	// 130:1e76
	arr_i16_4758[0] = _screenGrid[_jumbleSelected].top + 3;
	arr_i16_4758[1] = _screenGrid[_jumbleSelected].left + 3;
	arr_i16_4758[2] = _screenGrid[_jumbleSelected].bottom - 3;
	arr_i16_4758[3] = _screenGrid[_jumbleSelected].right - 3;
	Common::Rect temp;
	temp.top = arr_i16_4758[0];
	temp.left = arr_i16_4758[1];
	temp.bottom = arr_i16_4758[2];
	temp.right = arr_i16_4758[3];
	_toolbox->FillRect(temp, _patterns[var_i16_105e]);
	_zbasic->text(kFontPuzzle, arr_i16_1eb8[0xe], 0, var_i16_106c);
	// 130:1f70
	_toolbox->MoveTo(
		_screenGrid[_jumbleSelected].left + arr_i16_1eb8[2],
		_screenGrid[_jumbleSelected].top + arr_i16_1eb8[3]
	);
	_toolbox->DrawString(var_str_d12);
	if (arr_i16_3738[_jumblePosition] < 0)
		return;
	// 130:1ff4
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenMode(kPatBic);
	_toolbox->PaintRect(temp);
}

void FoolGame::sub_130_201a() {
	warning(__func__);
	// 130:201a
	_zbasic->midStrSet(var_str_1070, _jumblePosition, 1, var_str_d12);
	if ((_activePuzzle > 0x50) && (_activePuzzle < 0x55)) {
		sub_130_1e5c();
	} else {
		// 130:2066
		sub_130_1c6c();
		jumbleDrawLetter();
	}
	// 130:206e
	jumbleNextPosition();
	sub_130_20d2();
}

void FoolGame::jumbleNextPosition() {
	warning(__func__);
	// 130:2078
	_jumblePosition++;
	if (_jumblePosition > var_i16_1576) {
		_jumblePosition = 1;
	}
}

void FoolGame::jumblePreviousPosition() {
	warning(__func__);
	// 130:2094
	_jumblePosition--;
	if (_jumblePosition < 1) {
		_jumblePosition = var_i16_1576;
	}
}

void FoolGame::sub_130_20aa() {
	warning(__func__);
	// 130:20aa
	jumbleSelectSquare();
	_jumblePosition = arr_i16_3b38[var_i16_68a*32 + var_i16_68c];
	sub_130_20d2();
}


void FoolGame::sub_130_20d2() {
	warning(__func__);
	// 130:20d2
	_jumbleSelected = ABS(arr_i16_3738[_jumblePosition]);
	jumbleSelectSquare();
	sub_130_20fe();
	sub_130_2178();
}

void FoolGame::sub_130_20fe() {
	warning(__func__);
	// 130:20fe
	var_i16_1888 = ABS(arr_i16_3738[_jumblePosition]) - 1;
	var_i16_68a = (var_i16_1888 % arr_i16_1eb8[0]) + 1;
	var_i16_68c = (var_i16_1888 / arr_i16_1eb8[0]) + 1;
}

void FoolGame::sub_130_2178() {
	warning(__func__);
	// 130:2178
	if ((arr_i16_2f38[var_i16_68a*32 + var_i16_68c] == var_i16_1678) || (_jumbleGameType == 2))
		return;

	var_i16_1678 = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
	if (!_zbasic->index(1, var_i16_1678).empty()) { // was: str(185)
		var_str_1578 = _zbasic->index(1, var_i16_1678);
		sub_130_2226();
	}
	// 130:2224
}

void FoolGame::sub_130_2226() {
	warning(__func__);
	// 130:2226
	if (_activePuzzle == 0x14) { // the blacksmith
		sub_130_23cc();
		return;
	}
	if (_activePuzzle == 0x3c) { // the boat
		sub_130_22ee();
		return;
	}
	if (_activePuzzle == 0x35) { // the sentry
		sub_130_24aa();
		return;
	}
	if (_activePuzzle == 0x55) { // final puzzle
		sub_130_2548();
		return;
	}
	// 130:2266
	fillRect(0x124, 0x64, 0x139, 0x1ef, 0);
	if (var_str_1578.empty()) // was: str(186)
		return;
	// 130:22a0
	_zbasic->text(kFontFool, 0xc, 0, kSrcOr);
	var_i16_484 = _toolbox->StringWidth(var_str_1578);
	_toolbox->MoveTo(0x127 - (var_i16_484 / 2), 0x133);
	_toolbox->DrawString(var_str_1578);
}

void FoolGame::sub_130_22ee() {
	warning(__func__);
	// 130:22ee
	fillRect(0x131, 0x84, 0x156, 0x17e, var_i16_1060);
	if (var_str_1578.empty()) // was: str(187)
		return;
	_zbasic->text(kFontFool, 0xc, 0, kSrcBic);
	var_i16_7ba = _toolbox->StringWidth(var_str_1578);
	fillRect(0x13c, 0xf6 - (var_i16_7ba / 2), 0x150, 0x10a + (var_i16_7ba / 2), 2);
	_toolbox->MoveTo(0x100 - (var_i16_7ba / 2), 0x14a);
	_toolbox->DrawString(var_str_1578);
}

void FoolGame::sub_130_23cc() {
	warning(__func__);
	// 130:23cc
	fillRect(0x12c, 0x8c, 0x156, 0x186, var_i16_1060);
	if (var_str_1578.empty()) // was: str(188)
		return;
	_zbasic->text(kFontFool, 0xc, 0, kSrcBic);
	var_i16_7ba = _toolbox->StringWidth(var_str_1578);
	fillRect(0x130, 0x100 - (var_i16_7ba / 2), 0x144, 0x114 + (var_i16_7ba / 2), 2);
	_toolbox->MoveTo(0x10a - (var_i16_7ba / 2), 0x13e);
	_toolbox->DrawString(var_str_1578);
}

void FoolGame::sub_130_24aa() {
	warning(__func__);
	// 130:24aa
	fillRect(0x125, 0x20, 0x136, 0x15e, 2);
	if (var_str_1578.empty()) // was: str(189)
		return;
	_zbasic->text(kFontFool, 0xc, 0, kSrcBic);
	var_i16_7ba = _toolbox->StringWidth(var_str_1578);
	_toolbox->MoveTo(0xc4 - (var_i16_7ba / 2), 0x131);
	_toolbox->DrawString(var_str_1578);
}

void FoolGame::sub_130_2548() {
	warning(__func__);
	// 130:2548
	_zbasic->text(kFontFool, 0xc, 0, kSrcOr);
	fillRect(0x131, 0, 0x156, 0x10e, 0x47);
	if (var_str_1578.empty()) // was: str(190)
		return;

	var_i16_7ba = _toolbox->StringWidth(var_str_1578);
	fillRect(0x13e, 0xc, 0x14e, 0x16 + var_i16_7ba, 0);
	_toolbox->MoveTo(0x16, 0x14a);
	_toolbox->DrawString(var_str_1578);
}

void FoolGame::sub_130_25d8() {
	warning(__func__);
	// 130:25d8
	_zbasic->unk_20();
	var_i16_188a = var_str_384.size();
	var_str_9f4 = _zbasic->space(var_i16_188a);
	for (int16 i = 1; i <= var_i16_188a; i++) {
		arr_i16_5bbc[i] = i;
	}
	var_i16_188c = 0;
	for (int16 i = var_i16_188a; i >= 1; i--) {
		do {
			var_i16_7e4 = _zbasic->rndInt(i);
			// 130:2640
			if (i == 1) {
				// 130:266e
				if (arr_i16_5bbc[var_i16_7e4] == i) {
					var_i16_188c = 1;
				}
				break;
			}
			// 130:264c
		} while (arr_i16_5bbc[var_i16_7e4] == i);
		// 130:2690
		arr_i16_5cbc[i] = arr_i16_5bbc[var_i16_7e4];
		for (int16 j = var_i16_7e4; j <= i; j++) {
			arr_i16_5bbc[j] = arr_i16_5bbc[j+1];
		}
		// 130:26f2
	}
	// 130:2712
	if (var_i16_188c != 0) {
		_zbasic->swapInt(arr_i16_5cbc[1], arr_i16_5cbc[2]);
	}
	// 130:273c
	for (int16 i = 1; i <= var_i16_188a; i++) {
		_zbasic->midStrSet(var_str_9f4, i, 1, _zbasic->midStr(var_str_384, arr_i16_5cbc[i], 1));
	}
}

void FoolGame::sub_130_2790() {
	warning(__func__);
	_zbasic->unk_20();
	for (int16 i = 1; i <= 0x1a; i++) {
		arr_i16_5bbc[i] = i;
	}
	var_i16_188c = 0;
	for (int16 i = 0x1a; i > 0; i--) {
		// 130:27c4
		do {
			var_i16_7e4 = _zbasic->rndInt(i);
			// 130:27d2
			if (i == 1) {
				// 130:2800
				if (arr_i16_5bbc[var_i16_7e4] == i) {
					// 130:281c
					var_i16_188c = 1;
				}
				break;
			}
			// 130:27de
		} while (arr_i16_5bbc[var_i16_7e4] == i);
		// 130:2822
		arr_i16_5cbc[i] = arr_i16_5bbc[var_i16_7e4];
		for (int16 j = var_i16_7e4; j <= i; j++) {
			arr_i16_5bbc[j] = arr_i16_5bbc[j+1];
		}
	}
	// 130:28a4
	if (var_i16_188c != 0) {
		_zbasic->swapInt(arr_i16_5cbc[1], arr_i16_5cbc[2]);
	}
	var_i16_188a = var_str_384.size();
	var_str_9f4 = _zbasic->space(var_i16_188a);
	for (int16 i = 1; i <= var_i16_188a; i++) {
		var_i16_7e4 = _zbasic->asc(_zbasic->midStr(var_str_384, i, 1)) - 0x40;
		_zbasic->midStrSet(var_str_9f4, i, 1, _zbasic->chr(arr_i16_5cbc[var_i16_7e4]+0x40));
	}
}


}
