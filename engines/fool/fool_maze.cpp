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

#define OFF(x) (_zstrOffset[kOffsetMaze] + (x))



void FoolGame::mazeRun() {
	// 136:0004
	fetchPuzzleData();
	toggleMouseCursor(false);
	for (int16 i = 0; i <= 0x17; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
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
			// 136:00ca
		} while (_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[11], arr_i16_1eb8[6]));
	} while (_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	if (arr_i16_1eb8[20] != 0) {
		var_i32_1ac0 = _toolbox->GetPicture(arr_i16_1eb8[20]);
		_zbasic->picture(arr_i16_1eb8[18], arr_i16_1eb8[19], var_i32_1ac0);
	}
	// 136:018c
	for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		_mazeInvItemCount[i] = 0;
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
	playArea.top = _screenGrid[1].top + 3;
	playArea.left = _screenGrid[1].left + 3;
	playArea.bottom = _screenGrid[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].bottom - 3;
	playArea.right = _screenGrid[arr_i16_1eb8[0] * arr_i16_1eb8[1] - 1].right - 3;
	_toolbox->FillRect(playArea, _patterns[2]);
	_toolbox->PenNormal();
	_toolbox->PenSize(5, 5);
	_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
	// 136:042c
	var_i16_1ac4 = arr_i16_1eb8[17];
	var_i16_1574 = 0;
	var_i16_68c = 1;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_1574++;
			if ((arr_i16_3738[var_i16_1574] & 0x10) != 0) {
				mazeAddWallLeft();
			}
			if ((arr_i16_3738[var_i16_1574] & 0x20) != 0) {
				mazeAddWallTop();
			}
			if ((arr_i16_3738[var_i16_1574] & 0x40) != 0) {
				mazeAddWallRight();
			}
			// 136:04b6
			if ((arr_i16_3738[var_i16_1574] & 0x80) != 0) {
				mazeAddWallBottom();
			}
		}
		if ((arr_i16_3738[var_i16_1574 - 1] & 4) == 0) {
			Common::Rect temp;
			temp.top = _screenGrid[var_i16_1574].top + 3;
			temp.left = _screenGrid[var_i16_1574].left - 3;
			temp.bottom = _screenGrid[var_i16_1574].bottom - 3;
			temp.right = SCREEN_WIDTH;
			_toolbox->FillRect(temp, _patterns[2]);
		}
		// 136:05d0
	}
	// 136:05ec
	if (_activePuzzleBuffer.empty()) { // was: str(OFF(0))
		for (int16 j = 1; j <= arr_i16_1eb8[23]; j++) {
			var_str_1ac8 = _zbasic->indexRaw(1, j);
			var_i16_1bc8 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ac8, 3, 2));
			if (var_i16_1bc8 != 666) {
				for (int16 i = 1; i <= var_i16_1bc8; i++) {
					sub_136_2582();
				}
			}
			// 136:0664
		}
		// 136:067e
		_mazePlayerX = arr_i16_1eb8[15];
		_mazePlayerY = arr_i16_1eb8[16];
		var_i16_1bd0 = 0;
	} else {
		// 136:06b0
		mazeLoadState();
	}
	// 136:06b4
	var_str_1578.clear(); // was str(OFF(1))
	mazeSetupMenu();
	sub_136_ade();
	_stateFlags = kStateNull;
	toggleMouseCursor(true);
	if (_mazePlayerX == 0x19) {
		_activePuzzleSolved = true;
	} else {
		// 136:06f0
		_activePuzzleSolved = false;
	}
	// 136:06f6
	// JMP 0x75c
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 136:06fa
		while ((_stateFlags == kStateNull) && (!_activePuzzleSolved)) {
			// 136:06fe
			getNextEvent(-1);
			if ((_event.modifiers & kModMouseButtonUp) == 0) {
				mazeOnClick();
			}
			if (_mazePlayerX == 0x19) {
				_activePuzzleSolved = true;
			}
		}
		// 136:074a
		if (_stateFlags == kStateSaveGame) {
			mazeStoreState();
			saveGame();
		}
		// 136:075c
	}
	// 136:0786
	if (_activePuzzleSolved != 0) {
		sub_136_3a70();
	}
	mazeStoreState();
}

void FoolGame::mazeOnClick() {
	// 136:079e
	getGridFromMouse(var_i16_68a, var_i16_68c);
	if ((var_i16_68a == _mazePlayerX) && (var_i16_68c == _mazePlayerY)) {
		return;
	}
	// 136:07e2
	if (_mazePlayerY > var_i16_68c) {
		_mazePlayerDirection = 2;
	}
	if (_mazePlayerX > var_i16_68a) {
		_mazePlayerDirection = 1;
	}
	if (_mazePlayerY < var_i16_68c) {
		_mazePlayerDirection = 4;
	}
	if (_mazePlayerX < var_i16_68a) {
		_mazePlayerDirection = 3;
	}
	if (var_i16_1bd4 != _mazePlayerDirection) {
		if ((_mazePlayerDirection == 1) && ((arr_i16_3738[var_i16_1574] & 1) == 1)) {
			mazeFlashWall();
			return;
		}
		// 136:088a
		if ((_mazePlayerDirection == 2) && ((arr_i16_3738[var_i16_1574] & 2) == 2)) {
			mazeFlashWall();
			return;
		}
		if ((_mazePlayerDirection == 3) && ((arr_i16_3738[var_i16_1574] & 4) == 4)) {
			mazeFlashWall();
			return;
		}
		if ((_mazePlayerDirection == 4) && ((arr_i16_3738[var_i16_1574] & 8) == 8)) {
			mazeFlashWall();
			return;
		}
		// 136:0944
		if (var_i16_1bd6 == _mazePlayerDirection) {
			return;
		}
	}
	// 136:0956
	if (_mazePlayerDirection == 1)
		_mazePlayerX--;
	if (_mazePlayerDirection == 2)
		_mazePlayerY--;
	if (_mazePlayerDirection == 3)
		_mazePlayerX++;
	if (_mazePlayerDirection == 4)
		_mazePlayerY++;
	mazeMovePlayer();
}

void FoolGame::mazeFlashWall() {
	// 136:0994
	_toolbox->PenNormal();
	_toolbox->PenSize(5, 5);
	if (arr_i16_1eb8[21] == 0) {
		_toolbox->PenMode(kPatXor);
		switch (_mazePlayerDirection-1) {
		case 0:
			mazeAddWallLeft();
			break;
		case 1:
			mazeAddWallTop();
			break;
		case 2:
			mazeAddWallRight();
			break;
		case 3:
			mazeAddWallBottom();
			break;
		default:
			warning("mazeFlashWall: breaking out of switch");
			break;
		}
		_toolbox->Delay(0);
	} else {
		// 136:09de
		_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
	}
	// 136:0a00
	switch (_mazePlayerDirection-1) {
	case 0:
		mazeAddWallLeft();
		break;
	case 1:
		mazeAddWallTop();
		break;
	case 2:
		mazeAddWallRight();
		break;
	case 3:
		mazeAddWallBottom();
		break;
	default:
		warning("mazeFlashWall: breaking out of switch");
		break;
	}
	_toolbox->Delay(0);
}

void FoolGame::mazeMovePlayer() {
	// 136:0a22
	if (var_i16_1bd6 != 0) {
		_toolbox->PenNormal();
		_toolbox->PenPat(_patterns[2]);
		_toolbox->PenSize(5, 5);
		switch (var_i16_1bd6 - 1) {
		case 0:
			mazeDrawWallLeft();
			break;
		case 1:
			mazeDrawWallTop();
			break;
		case 2:
			mazeDrawWallRight();
			break;
		case 3:
			mazeDrawWallBottom();
			break;
		default:
			warning("mazeMovePlayer: breaking out of switch");
			break;
		}
	}
	// 136:0a66
	if (var_i16_1bd4 != 0) {
		_toolbox->PenNormal();
		_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
		_toolbox->PenSize(5, 5);
		switch (var_i16_1bd4 - 1) {
		case 0:
			mazeDrawWallLeft();
			break;
		case 1:
			mazeDrawWallTop();
			break;
		case 2:
			mazeDrawWallRight();
			break;
		case 3:
			mazeDrawWallBottom();
			break;
		default:
			warning("mazeMovePlayer: breaking out of switch");
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
		_toolbox->InvertOval(temp); // erase current spot
		mazeDrawPlayer();
		var_i16_1bda = var_i16_1574;
		mazeHotspot();
	} while (var_i16_1bda != var_i16_1574);
}


void FoolGame::sub_136_ade() {
	// 136:0ade
	// this was done by a GOTO into the previous function, so we copy most of it here.
	mazeDrawPlayer();
	var_i16_1bda = var_i16_1574;
	mazeHotspot();
	while (var_i16_1bda != var_i16_1574) {
		var_i16_1bd6 = 0;
		var_i16_1bd4 = 0;
		var_i16_1bd8 = 0;
		Common::Rect temp;
		temp.top = arr_i16_1eb8[24];
		temp.left = arr_i16_1eb8[25];
		temp.bottom = arr_i16_1eb8[26];
		temp.right = arr_i16_1eb8[27];
		_toolbox->InvertOval(temp);
		mazeDrawPlayer();
		var_i16_1bda = var_i16_1574;
		mazeHotspot();
	};
}

void FoolGame::mazeHotspot() {
	// 136:0b00
	if (arr_i16_1eb8[22] || var_i16_1bdc) {
		fillRect(0x136, 0, SCREEN_HEIGHT, SCREEN_WIDTH, var_i16_1ac4);
		var_i16_1bdc = 0;
		var_str_1578.clear(); // was: str(OFF(2))
	}
	// 136:0b5e
	if ((arr_i16_3738[var_i16_1574] & 0x1000) == 0) { // cell doesn't have hotspot flag
		var_i16_1bd0 = var_i16_1574;
		return;
	}
	var_str_1ac8 = _zbasic->indexRaw(1, arr_i16_3b38[_mazePlayerX*32 + _mazePlayerY]);
	debugC(5, kDebugLoading, "mazeHotspot: processing hotspot at (%d, %d)", _mazePlayerX, _mazePlayerY);
	if (debugChannelSet(5, kDebugLoading)) {
		Common::hexdump((const byte *)var_str_1ac8.c_str(), var_str_1ac8.size());
	}
	var_str_1bde = _zbasic->strRaw(OFF(3)); // CILTDABRPXMOZSFQGWEJKV
	// 136:0bd0
	var_i16_1cde = 1;
	_zbasic->unk_20();
	while (var_i16_1cde > 0) {
		// 136:0bde
		var_i16_1cde = _zbasic->instr(var_i16_1cde, var_str_1ac8, _zbasic->strRaw(OFF(4))); // \xa5\xa5
		if (var_i16_1cde > 0) {
			// get command ID
			for (int16 i = 1; i <= (int16)var_str_1bde.size(); i++) {
				if (_zbasic->midStr(var_str_1ac8, var_i16_1cde + 2, 1) == _zbasic->midStr(var_str_1bde, i, 1)) {
					var_i16_1a9a = i;
				}
				// 136:0c4c
			}

			// 136:0c5e
			var_i16_1ce0 = _zbasic->instr(var_i16_1cde, var_str_1ac8, _zbasic->strRaw(OFF(5))); // <<
			var_str_1ce2 = _zbasic->midStr(var_str_1ac8, var_i16_1cde + 3, var_i16_1ce0 - var_i16_1cde - 3);
			if (_zbasic->midStr(var_str_1ac8, var_i16_1ce0 + 2, 2) == _zbasic->strRaw(OFF(6))) { // ||
				// 136:0cdc
				var_i16_1cde = 0;
			} else {
				var_i16_1cde = var_i16_1ce0;
			}
			// 136:0cec
			switch (var_i16_1a9a - 1) {
			case 0:	// C
				mazePrintMessage();
				break;
			case 1: // I
				mazePickUpItem();
				break;
			case 2: // L
				mazeUseItem();
				break;
			case 3: // T
				mazeWanderingWinds();
				break;
			case 4: // D
				mazeDrawLetter();
				break;
			case 5: // A
				sub_136_137c();
				break;
			case 6: // B
				sub_136_163c();
				break;
			case 7: // R
				sub_136_1650();
				break;
			case 8: // P
				mazeHiddenDoorOpen();
				break;
			case 9: // X
				mazeHiddenDoorShut();
				break;
			case 10: // M
				sub_136_1806();
				break;
			case 11: // O
				mazeSetTrigger();
				break;
			case 12: // Z
				mazeClearTrigger();
				break;
			case 13: // S
				mazePickUpTone();
				break;
			case 14: // F
				mazeFireDemon();
				break;
			case 15: // Q
				sub_136_19d2();
				break;
			case 16: // G
				mazeNoisySprite();
				break;
			case 17: // W
				mazeDelay();
				break;
			case 18: // E
				mazeYeetObject();
				break;
			case 19: // J
				mazeThornsGetScroll();
				break;
			case 20: // K
				sub_136_21fa();
				break;
			case 21: // V
				sub_136_2200();
				break;
			default:
				warning("mazeHotspot: breaking out of switch statement");
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
	if (!var_str_1ce2.empty()) { // was: str(OFF(7))
		_zbasic->text(kFontFool, 0xc, 0, kSrcBic);
		int16 width = _toolbox->StringWidth(var_str_1ce2);
		fillRect(0x13b, 0xf1 - (width / 2), 0x14f, 0x10f + (width / 2), 2);
		drawTextCenter(var_str_1ce2.decode(Common::kMacRoman), 0x149);
	}
	// 136:0e3a
	var_str_1578 = var_str_1ce2.decode(Common::kMacRoman);
}

void FoolGame::mazePickUpItem() {
	// 136:0e4c
	warning(__func__);
	int16 itemID = _zbasic->decodeInt(var_str_1ce2);
	_mazeInvItemCount[itemID]++;
	if (arr_i16_3738[var_i16_1574] & 0x1000) {
		arr_i16_3738[var_i16_1574] ^= 0x1000;
	}
	// 136:0ed2
	mazeSetupMenu();
}

void FoolGame::mazeUseItem() {
	warning(__func__);
	// 136:0ed8
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	int16 itemID = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 3, 2));
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		_mazeInvItemCount[itemID]--;
	}
	if (_mazeInvItemCount[itemID] < 0) {
		_mazeInvItemCount[itemID] = 0;
	}
	mazeSetupMenu();
}

void FoolGame::mazeWanderingWinds() {
	// 136:0f74
	if (_zbasic->leftStr(var_str_1ce2, 1) == _zbasic->strRaw(OFF(8))) { // L
		var_i16_1de6 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_1de4 = _zbasic->rndInt(var_i16_1de6);
		var_i16_1de8 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, var_i16_1de4 * 2 + 2, 2));
		int16 offset = var_i16_1de6 * 2 + 4;
		mazeLoadTone(offset);
	}
	// 136:0ffc
	if (_zbasic->leftStr(var_str_1ce2, 1) == _zbasic->strRaw(OFF(9))) { // A
		var_i16_68a = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_68c = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 4, 2));
		var_i16_1a96 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 6, 2));
		var_i16_1a98 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 8, 2));
		var_i16_68a += _zbasic->rndInt(var_i16_1a96 + 1) - 1;
		var_i16_68c += _zbasic->rndInt(var_i16_1a98 + 1) - 1;
		// 136:10bc
		var_i16_1de8 = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
		mazeLoadTone(0xa);
	}
	// 136:10ea
	Common::Rect temp;
	temp.top = arr_i16_1eb8[24];
	temp.left = arr_i16_1eb8[25];
	temp.bottom = arr_i16_1eb8[26];
	temp.right = arr_i16_1eb8[27];
	_toolbox->InvertOval(temp);
	var_i16_1dea = var_i16_1574;
	var_i16_1dec = 2;
	mazeMovementTrail();
	// 136:1108
	_mazePlayerX = ((var_i16_1de8 - 1) % arr_i16_1eb8[0]) + 1;
	_mazePlayerY = ((var_i16_1de8 - 1) / arr_i16_1eb8[0]) + 1;
	mazeDrawPlayer();
}

void FoolGame::mazeDrawLetter() {
	warning(__func__);
	// 136:115a
	if (_zbasic->leftStr(var_str_1ce2, 1) == _zbasic->strRaw(OFF(10))) { // P
		var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_106e = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 4, 2));
		if (var_i16_1dea == var_i16_1574) {
			var_src_1dee = kSrcOr;
		} else {
			var_src_1dee = kSrcBic;
		}
		// 136:11ce
		arr_i16_4338[var_i16_1dea] = var_i16_106e;
	}
	// 136:11e0
	if (_zbasic->leftStr(var_str_1ce2, 1) == _zbasic->strRaw(OFF(11))) { // E
		var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
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
	if (_zbasic->leftStr(var_str_1ce2, 1) == _zbasic->strRaw(OFF(12))) { // C
		var_i16_1dea = var_i16_1574;
		var_i16_106e = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
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
	_zbasic->text(kFontPuzzle, 0xc, 0, var_src_1dee);
	_toolbox->MoveTo(
		_screenGrid[var_i16_1dea].left + arr_i16_1eb8[2],
		_screenGrid[var_i16_1dea].top + arr_i16_1eb8[3]
	);
	var_str_384 = _zbasic->chr(var_i16_106e);
	_toolbox->DrawString(var_str_384);
}

void FoolGame::sub_136_137c() {
	Common::String op = _zbasic->leftStr(var_str_1ce2, 1);
	warning("%s: op %s", __func__, op.c_str());
	// 136:137c
	var_i16_1df0 = 0;
	if (op == _zbasic->strRaw(OFF(13))) { // I
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		if (_mazeInvItemCount[var_i16_484] == 0) {
			var_i16_1df0 = 1;
		}
	}
	// 136:13d8
	if (op == _zbasic->strRaw(OFF(14))) { // M
		var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		var_i16_1de2 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 4, 2));
		if (_mazeInvItemCount[var_i16_1de2] < var_i16_1de4) {
			var_i16_1df0 = 1;
		}
	}
	// 136:144e
	if (op == _zbasic->strRaw(OFF(15))) { // B
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		if ((arr_i16_3738[var_i16_484] & 0x1000) == 0) {
			var_i16_1df0 = 1;
		}
	}
	// 136:14b2
	if (op == _zbasic->strRaw(OFF(16))) { // L
		// 136:14d6
		var_i16_7e4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		if (arr_i16_4338[var_i16_484] != var_i16_7e4) {
			var_i16_1df0 = 1;
		}
	}
	// 136:1528
	if (op == _zbasic->strRaw(OFF(17))) { // S
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		for (int16 i = 4; i <= (2*var_i16_484 + 2); i += 2) {
			var_i16_7e4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, i, 2));
			if (arr_i16_4338[var_i16_7e4] == 0) {
				var_i16_1df0 = 1;
			}
		}
	}
	// 136:15c0
	if (op == _zbasic->strRaw(OFF(18))) { // P
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 2, 2));
		if (var_i16_1bd0 != var_i16_484) {
			var_i16_1df0 = 1;
		}
	}
	// 136:1612
	if (var_i16_1df0 != 0) {
		var_i16_1cde = _zbasic->instr(1, var_str_1ac8, _zbasic->strRaw(OFF(19))); // ||
	}
	// 136:163a
	return;
}

void FoolGame::sub_136_163c() {
	warning(__func__);
	// 136:163c
	var_i16_1bd8 = var_i16_1574;
	var_i16_1bd6 = _zbasic->decodeInt(var_str_1ce2);
}

void FoolGame::sub_136_1650() {
	warning(__func__);
	// 136:1650
	int16 index = _zbasic->decodeInt(var_str_1ce2);
	Common::String data = _zbasic->indexRaw(1, index);
	var_i16_103a = _zbasic->decodeInt(_zbasic->leftStr(data, 2));
	var_i16_1df2 = 0;
	for (int16 i = 1; i <= var_i16_103a; i++) {
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(data, i*2 + 3, 2));
		if ((arr_i16_3738[var_i16_484] & 0x1000) == 0) {
			var_i16_1df2++;
			arr_i16_5bbc[var_i16_1df2] = var_i16_484;
		}
		// 136:16f4
	}
	// 136:1704
	var_i16_484 = arr_i16_5bbc[_zbasic->rndInt(var_i16_1df2)];
	arr_i16_3738[var_i16_484] |= 0x1000;
}

void FoolGame::mazeHiddenDoorOpen() {
	// 136:1756
	var_i16_1bd8 = var_i16_1574;
	var_i16_1bd4 = _zbasic->decodeInt(var_str_1ce2);
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[2]);
	_toolbox->PenSize(5, 5);
	switch (var_i16_1bd4-1) {
	case 0:
		mazeDrawWallLeft();
		break;
	case 1:
		mazeDrawWallTop();
		break;
	case 2:
		mazeDrawWallRight();
		break;
	case 3:
		mazeDrawWallBottom();
		break;
	default:
		warning("mazeHiddenDoorOpen: broke out of switch statement");
		break;
	}
}

void FoolGame::mazeHiddenDoorShut() {
	// 136:17a4
	var_i16_1bd8 = var_i16_1574;
	var_i16_1bd6 = _zbasic->decodeInt(var_str_1ce2);
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
	_toolbox->PenSize(5, 5);
	switch (var_i16_1bd6-1) {
	case 0:
		mazeDrawWallLeft();
		break;
	case 1:
		mazeDrawWallTop();
		break;
	case 2:
		mazeDrawWallRight();
		break;
	case 3:
		mazeDrawWallBottom();
		break;
	default:
		warning("mazeHiddenDoorShut: broke out of switch statement");
		break;
	}
}

void FoolGame::sub_136_1806() {
	warning(__func__);
	// 136:1806
	sub_136_2582();
	if ((arr_i16_3738[var_i16_1574] & 0x1000) != 0) {
		arr_i16_3738[var_i16_1574] ^= 0x1000;
	}
	// 136:1858
}

void FoolGame::mazeSetTrigger() {
	warning(__func__);
	// 136:185a
	var_i16_484 = _zbasic->decodeInt(var_str_1ce2);
	arr_i16_3738[var_i16_484] |= 0x1000;
}

void FoolGame::mazeClearTrigger() {
	warning(__func__);
	// 136:1898
	var_i16_484 = _zbasic->decodeInt(var_str_1ce2);
	if (arr_i16_3738[var_i16_484] & 0x1000) {
		arr_i16_3738[var_i16_484] ^= 0x1000;
	}
}

void FoolGame::mazePickUpTone() {
	warning(__func__);
	// 136:18f4
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	mazeLoadTone(3);
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		mazePlayTone();
	}
}

void FoolGame::mazeFireDemon() {
	warning(__func__);
	// 136:1932
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	mazeLoadTone(3);
	arr_i16_4758[0] = 0x14;
	arr_i16_4758[1] = 0;
	arr_i16_4758[2] = SCREEN_HEIGHT;
	arr_i16_4758[3] = SCREEN_WIDTH;
	Common::Rect temp;
	temp.top = arr_i16_4758[0];
	temp.left = arr_i16_4758[1];
	temp.bottom = arr_i16_4758[2];
	temp.right = arr_i16_4758[3];
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		_toolbox->InvertRect(temp);
		_toolbox->Delay(0);
		if (mazeToneFreq > 0) {
			mazePlayTone();
		}
		_toolbox->InvertRect(temp);
		_toolbox->Delay(0);
	}
}

void FoolGame::sub_136_19d2() {
	warning(__func__);
	// 136:19d2
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);
	_toolbox->PenSize(6, 6);
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_1de4 = var_i16_1de4 * 2 - 1;
	var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 3, 2));
	var_i16_1dec = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 5, 2));
	var_i16_1de8 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 7, 2));
	var_i16_1df6 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 9, 2));
	mazeLoadTone(0xb);
	// 136:1a7a
	var_i16_1df8 = _screenGrid[var_i16_1dea].left + ((_screenGrid[var_i16_1dea].right - _screenGrid[var_i16_1dea].left) / 2) - 3;
	var_i16_1dfa = _screenGrid[var_i16_1dea].top + ((_screenGrid[var_i16_1dea].bottom - _screenGrid[var_i16_1dea].top) / 2) - 3;
	// 136:1b56
	var_i16_1dfc = _screenGrid[var_i16_1de8].left + ((_screenGrid[var_i16_1de8].right - _screenGrid[var_i16_1de8].left) / 2) - 3;
	var_i16_1dfe = _screenGrid[var_i16_1de8].top + ((_screenGrid[var_i16_1de8].bottom - _screenGrid[var_i16_1de8].top) / 2) - 3;
	// 136:1c32
	for (int16 i = 0; i <= var_i16_1de4; i++) {
		if (var_i16_1dec == 1) {
			_toolbox->InvertRect(_screenGrid[var_i16_1dea]);
		}
		// 136:1c5a
		if (var_i16_1dec == 2) {
			_toolbox->InvertOval(_screenGrid[var_i16_1dea]);
		}
		_toolbox->MoveTo(var_i16_1df8, var_i16_1dfa);
		_toolbox->LineTo(var_i16_1dfc, var_i16_1dfe);
		if (var_i16_1df6 == 1) {
			_toolbox->InvertRect(_screenGrid[var_i16_1de8]);
		}
		// 136:1cb2
		if (var_i16_1df6 == 2) {
			_toolbox->InvertOval(_screenGrid[var_i16_1de8]);

		}
		if (mazeToneFreq > 0) {
			mazePlayTone();
		}
		_toolbox->Delay(0);
	}
}

void FoolGame::mazeNoisySprite() {
	warning(__func__);
	// 136:1cf4
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 3, 2));
	var_i16_1dec = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 5, 2));
	mazeLoadTone(7);
	for (int16 i = 1; i <= var_i16_1de4; i++) {
		// 136:1d4c
		if (mazeToneFreq > 0) {
			mazePlayTone();
		}
		if (var_i16_1dec == 1) {
			_toolbox->InvertRect(_screenGrid[var_i16_1dea]);
			_toolbox->Delay(0);
			_toolbox->InvertRect(_screenGrid[var_i16_1dea]);
			_toolbox->Delay(0);
		} else {
			// 136:1d98
			_toolbox->InvertOval(_screenGrid[var_i16_1dea]);
			_toolbox->Delay(0);
			_toolbox->InvertOval(_screenGrid[var_i16_1dea]);
			_toolbox->Delay(0);
		}
		// 136:1dc8
	}
}

void FoolGame::mazeDelay() {
	warning(__func__);
	// 136:1ddc
	var_i16_484 = _zbasic->decodeInt(var_str_1ce2);
	_toolbox->Delay(var_i16_484*60/1000);
}

void FoolGame::mazeYeetObject() {
	warning(__func__);
	// 136:1df4
	var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_1de8 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 3, 2));
	var_i16_1dec = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 5, 2));
	mazeLoadTone(7);
	mazeMovementTrail();
}

void FoolGame::mazeThornsGetScroll() {
	// 136:1e4c
	var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 1, 2));
	var_i16_1dec = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 3, 2));
	var_i16_1e00 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, 5, 2));
	mazeLoadTone(7);
	_toolbox->PenNormal();
	_toolbox->PenSize(4, 4);
	_toolbox->PenMode(kPatXor);
	// 136:1eb0
	if (var_i16_1e00 == 1) {
		arr_i16_4758[0] = 0x14;
		arr_i16_4758[1] = 0;
		arr_i16_4758[2] = SCREEN_HEIGHT;
		arr_i16_4758[3] = SCREEN_WIDTH;
		arr_i16_4758[4 + 0] = _screenGrid[var_i16_1dea].top;
		arr_i16_4758[4 + 1] = _screenGrid[var_i16_1dea].left;
		arr_i16_4758[4 + 2] = _screenGrid[var_i16_1dea].bottom;
		arr_i16_4758[4 + 3] = _screenGrid[var_i16_1dea].right;
	} else {
		// 136:1f4a
		arr_i16_4758[0] = _screenGrid[var_i16_1dea].top + 1;
		arr_i16_4758[1] = _screenGrid[var_i16_1dea].left + 1;
		arr_i16_4758[2] = _screenGrid[var_i16_1dea].bottom + 1;
		arr_i16_4758[3] = _screenGrid[var_i16_1dea].right + 1;
		// 136:1f96
		arr_i16_4758[4] = 0x14;
		arr_i16_4758[5] = 0;
		arr_i16_4758[6] = SCREEN_HEIGHT;
		arr_i16_4758[7] = SCREEN_HEIGHT;
	}
	// 136:1fd4
	for (int16 i = 0; i <= 3; i++) {
		arr_bcd_5dbc[4 + i] = (arr_i16_4758[i] - arr_i16_4758[4+i])/25.0;
	}
	// 136:2050
	for (int16 k = 0; k <= 1; k++) {
		// 136:2056
		for (int16 i = 0; i <= 3; i++) {
			arr_bcd_5dbc[i] = arr_i16_4758[4 + i];
		}
		for (int16 j = 1; j < 0x19; j++) {
			if (k == 1) {
				mazeToneFreqStep *= -1;
			}
			// 136:20bc
			for (int16 i = 0; i <= 3; i++) {
				var_i16_9f2 = 0;
				if (var_i16_1e00 == 1) {
					arr_i16_4758[i] = (int)arr_bcd_5dbc[i];
				}
				// 136:2100
				arr_bcd_5dbc[i] += arr_bcd_5dbc[i + 4];
				if (var_i16_1e00 == 2) {
					arr_i16_4758[i] = (int)arr_bcd_5dbc[i];
				}
				// 136:2192
			}
			// 136:21a0
			Common::Rect temp;
			temp.top = arr_i16_4758[0];
			temp.left = arr_i16_4758[1];
			temp.bottom = arr_i16_4758[2];
			temp.right = arr_i16_4758[3];
			if (var_i16_1dec == 1) {
				_toolbox->FrameRect(temp);
			}
			// 136:21b8
			if (var_i16_1dec == 2) {
				_toolbox->FrameOval(temp);
			}
			// 136:21d0
			if (mazeToneFreq > 0) {
				mazePlayTone();
			}
			_toolbox->Delay(0);
		}
	}
}

void FoolGame::sub_136_21fa() {
	warning(__func__);
	// 136:21fa
	waitForMouseUp();
}

void FoolGame::sub_136_2200() {
	warning(__func__);
	// 136:2200
	var_i16_1bdc = 1;
}

void FoolGame::mazeMovementTrail() {
	// 136:2208
	// unrolled loop
	arr_bcd_5dbc[4 + 0] = (_screenGrid[var_i16_1de8].top - _screenGrid[var_i16_1dea].top) / 25.0f;
	arr_bcd_5dbc[4 + 1] = (_screenGrid[var_i16_1de8].left - _screenGrid[var_i16_1dea].left) / 25.0f;
	arr_bcd_5dbc[4 + 2] = (_screenGrid[var_i16_1de8].bottom - _screenGrid[var_i16_1dea].bottom) / 25.0f;
	arr_bcd_5dbc[4 + 3] = (_screenGrid[var_i16_1de8].right - _screenGrid[var_i16_1dea].right) / 25.0f;
	// 136:229a
	for (int16 k = 0; k <= 1; k++) {
		// 136:22a0
		arr_bcd_5dbc[0] = _screenGrid[var_i16_1dea].top + 4.0f;
		arr_bcd_5dbc[1] = _screenGrid[var_i16_1dea].left + 4.0f;
		arr_bcd_5dbc[2] = _screenGrid[var_i16_1dea].bottom - 4.0f;
		arr_bcd_5dbc[3] = _screenGrid[var_i16_1dea].right - 4.0f;
		for (int16 j = 1; j <= 0x19; j++) {
			var_i32_692 = _toolbox->TickCount();
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
				_toolbox->InvertRect(temp);
			}
			// 136:2472
			if (var_i16_1dec == 2) {
				_toolbox->InvertOval(temp);
			}
			delayFromMarker(0);
		}
	}
}

void FoolGame::mazeLoadTone(int16 offset) {
	warning(__func__);
	// 136:24ae
	mazeToneFreq = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, offset, 2));
	if (mazeToneFreq > 0) {
		mazeToneFreqStep = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, offset + 2, 2));
		var_i16_1372 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, offset + 4, 2))*2;
		var_i16_1e04 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ce2, offset + 6, 2));
	}
	// 136:2536
}

void FoolGame::mazePlayTone() {
	// 136:2538
	playTone(mazeToneFreq, var_i16_1372, false);
	if ((mazeToneFreq > 0xe) && (mazeToneFreq < 0x7d00)) {
		mazeToneFreq += mazeToneFreqStep;
	}
}


void FoolGame::sub_136_2582() {
	warning(__func__);
	// 136:2582
	var_i16_1df2 = 0;
	var_i16_1e06 = _zbasic->decodeInt(_zbasic->leftStr(var_str_1ac8, 2));
	for (int16 i = 1; i <= var_i16_1e06; i++) {
		var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(var_str_1ac8, i*2 + 3, 2));
		if ((arr_i16_3738[var_i16_484] & 0x1000) == 0) {
			var_i16_1df2++;
			arr_i16_5bbc[var_i16_1df2] = var_i16_484;
		}
		// 136:2602
	}
	var_i16_484 = arr_i16_5bbc[_zbasic->rndInt(var_i16_1df2)];
	arr_i16_3738[var_i16_484] |= 0x1000;
}

void FoolGame::mazeDrawPlayer() {
	// 136:2664
	var_i16_1574 = arr_i16_2f38[_mazePlayerX*32 + _mazePlayerY];
	arr_i16_1eb8[24] = _screenGrid[var_i16_1574].top + 4;
	arr_i16_1eb8[25] = _screenGrid[var_i16_1574].left + 4;
	arr_i16_1eb8[26] = _screenGrid[var_i16_1574].bottom - 4;
	arr_i16_1eb8[27] = _screenGrid[var_i16_1574].right - 4;
	// 136:273e
	Common::Rect temp;
	temp.top = arr_i16_1eb8[24];
	temp.left = arr_i16_1eb8[25];
	temp.bottom = arr_i16_1eb8[26];
	temp.right = arr_i16_1eb8[27];
	_toolbox->InvertOval(temp); // draw new player pos
}

void FoolGame::mazeSetupMenu() {
	MenuHandle menu = _toolbox->GetMHandle(8);
	_toolbox->DeleteMenu(8);
	_toolbox->DisposeMenu(menu);

	// 136:274e
	_zbasic->menu(8, 0, 1, _puzzleName[_activePuzzle]);
	if (_activePuzzle < 0x50) {
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(20))); // return to scroll
	} else {
		// 136:279a
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(21))); // return to map
	}
	// 136:27b2
	_zbasic->menu(8, 2, 0, _zbasic->str(OFF(22))); // -
	_zbasic->menu(8, 3, 1, _zbasic->str(OFF(23))); // to escape from the maze
	_zbasic->menu(8, 4, 1, _zbasic->str(OFF(24))); // use the mouse to maneuver
	_zbasic->menu(8, 5, 1, _zbasic->str(OFF(25))); // the white dot to the exit
	if (_activePuzzle == 0x57) {
		// 136:281c
		int16 menuItem = 7;
		for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {

			if ((_mazeInvItemCount[i] > 0) &&
				(_zbasic->instr(1, _zbasic->index(1, i), _zbasic->str(OFF(26))) > 0)) {
				if (menuItem == 7) {
					_zbasic->menu(8, 6, 0, _zbasic->str(OFF(27))); // -
					_zbasic->menu(8, 7, 1, _zbasic->str(OFF(28))); // you have acquired
				}
				// 136:28b4
				var_i16_1dea = _zbasic->instr(1, _zbasic->index(1, i), _zbasic->str(OFF(29))); // ++
				var_i16_1de8 = _zbasic->instr(1, _zbasic->index(1, i), _zbasic->str(OFF(30))); // \xa5\xa5
				Common::U32String invItemName = _zbasic->midStr(_zbasic->index(1, i), var_i16_1dea + 2, var_i16_1de8 - var_i16_1dea - 2); // var_str_1e08
				if (_zbasic->leftStr(invItemName, 1) == _zbasic->str(OFF(31))) { // +
					// 136:2964
					var_i16_484 = invItemName.size();
					invItemName = _zbasic->rightStr(invItemName, var_i16_484 - 1);
				}
				// 136:298e
				Common::U32String menuStr = Common::U32String::format(" %d", _mazeInvItemCount[i]);
				menuStr = _zbasic->space(4 - menuStr.size()) + menuStr + _zbasic->str(OFF(32)) + invItemName; // ' '
				if (_mazeInvItemCount[i] > 1) {
					menuStr += _zbasic->str(OFF(33)); // s
				}
				// 136:2a2a
				menuStr += _zbasic->str(OFF(34)); // '  '
				menuItem++;
				_zbasic->menu(8, menuItem, 1, menuStr);
			}
			// 136:2a5e
		}
	}
	// 136:2a7a
	return;
}

void FoolGame::mazeAddWallLeft() {
	warning(__func__);
	// 136:2a7c
	arr_i16_3738[var_i16_1574] |= 0x100;
	_toolbox->MoveTo(
		_screenGrid[var_i16_1574].left - 2,
		_screenGrid[var_i16_1574].top - 2
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1574].left - 2,
		_screenGrid[var_i16_1574].bottom - 3
	);
}

void FoolGame::mazeAddWallTop() {
	warning(__func__);
	// 136:2b30
	arr_i16_3738[var_i16_1574] |= 0x200;
	_toolbox->MoveTo(
		_screenGrid[var_i16_1574].left - 2,
		_screenGrid[var_i16_1574].top - 2
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1574].right - 3,
		_screenGrid[var_i16_1574].top - 2
	);
}

void FoolGame::mazeAddWallRight() {
	warning(__func__);
	// 136:2be2
	arr_i16_3738[var_i16_1574] |= 0x400;
	_toolbox->MoveTo(
		_screenGrid[var_i16_1574].right - 3,
		_screenGrid[var_i16_1574].top - 2
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1574].right - 3,
		_screenGrid[var_i16_1574].bottom - 3
	);
}

void FoolGame::mazeAddWallBottom() {
	warning(__func__);
	// 136:2c96
	arr_i16_3738[var_i16_1574] |= 0x800;
	_toolbox->MoveTo(
		_screenGrid[var_i16_1574].left - 2,
		_screenGrid[var_i16_1574].bottom - 3
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1574].right - 3,
		_screenGrid[var_i16_1574].bottom - 3
	);
}

void FoolGame::mazeDrawWallLeft() {
	warning(__func__);
	// 136:2d4c
	_toolbox->MoveTo(
		_screenGrid[var_i16_1bd8].left - 2,
		_screenGrid[var_i16_1bd8].top + 3
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1bd8].left - 2,
		_screenGrid[var_i16_1bd8].bottom - 8
	);
}

void FoolGame::mazeDrawWallTop() {
	warning(__func__);
	// 136:2dd0
	_toolbox->MoveTo(
		_screenGrid[var_i16_1bd8].left + 3,
		_screenGrid[var_i16_1bd8].top - 2
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1bd8].right - 8,
		_screenGrid[var_i16_1bd8].top - 2
	);
}

void FoolGame::mazeDrawWallRight() {
	warning(__func__);
	// 136:2e52
	_toolbox->MoveTo(
		_screenGrid[var_i16_1bd8].right - 3,
		_screenGrid[var_i16_1bd8].top + 3
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1bd8].right - 3,
		_screenGrid[var_i16_1bd8].bottom - 8
	);
}

void FoolGame::mazeDrawWallBottom() {
	warning(__func__);
	// 136:2ed6
	_toolbox->MoveTo(
		_screenGrid[var_i16_1bd8].left + 3,
		_screenGrid[var_i16_1bd8].bottom - 3
	);
	_toolbox->LineTo(
		_screenGrid[var_i16_1bd8].right - 8,
		_screenGrid[var_i16_1bd8].bottom - 3
	);
}

void FoolGame::mazeStoreState() {
	// 136:2f5c
	// for the endgame maze, there's specific code to not save the puzzle state
	if (!((_activePuzzle != 0x57) && (!_activePuzzleSolved))) {
		_activePuzzleBuffer.clear(); // was: str(OFF(35))
		return;
	}
	_activePuzzleBuffer = _zbasic->encodeInt(_mazePlayerX) +
		_zbasic->encodeInt(_mazePlayerY) +
		_zbasic->encodeInt(var_i16_1bd0) +
		_zbasic->encodeInt(var_i16_1bd6) +
		_zbasic->encodeInt(var_i16_1bd4) +
		_zbasic->encodeInt(var_i16_1bd8);
	// 136:300a
	for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {
		_activePuzzleBuffer += _zbasic->encodeInt(_mazeInvItemCount[i]);
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
				buffer += _zbasic->encodeInt(arr_i16_2f38[i*32 + j]) + _zbasic->encodeInt(arr_i16_4338[var_i16_484]);
			}
			// 136:30f0
		}
	}
	// 136:3128
	_activePuzzleBuffer += _zbasic->encodeInt(var_i16_1de6) + buffer;

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
	debugC(5, kDebugLoading, "mazeStoreState: added mask %04x", var_i16_1f08);
	_activePuzzleBuffer += _zbasic->encodeInt(var_i16_1f08);
}

void FoolGame::sub_136_3408() {
	warning(__func__);
	// 136:3408
	var_i16_1f08 |= _bitLUT[var_i16_1de6];
}

void FoolGame::sub_136_342a() {
	warning(__func__);
	// 136:342a
	var_i16_1de6++;
	if (var_i16_1de6 > 0xf) {
		debugC(5, kDebugLoading, "sub_136_342a: added mask %04x", var_i16_1f08);
		_activePuzzleBuffer += _zbasic->encodeInt(var_i16_1f08);
		var_i16_1f08 = 0;
		var_i16_1de6 = 0;
	}
	// 136:3464
}

void FoolGame::mazeLoadState() {
	// 136:3466
	_mazePlayerX = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 1, 2));
	_mazePlayerY = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 3, 2));
	var_i16_1bd0 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 5, 2));
	var_i16_1bd6 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 7, 2));
	var_i16_1bd4 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 9, 2));
	var_i16_1bd8 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 0xb, 2));
	if (var_i16_1bd6 > 0) {
		// 136:3500
		_toolbox->PenNormal();
		_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
		_toolbox->PenSize(5, 5);
		switch (var_i16_1bd6 - 1) {
		case 0:
			mazeDrawWallLeft();
			break;
		case 1:
			mazeDrawWallTop();
			break;
		case 2:
			mazeDrawWallRight();
			break;
		case 3:
			mazeDrawWallBottom();
			break;
		default:
			warning("mazeLoadState: breaking out of switch");
			break;
		}
	}
	// 136:354e
	if (var_i16_1bd4 > 0) {
		_toolbox->PenNormal();
		_toolbox->PenPat(_patterns[2]);
		_toolbox->PenSize(5, 5);
		switch (var_i16_1bd4 - 1) {
		case 0:
			mazeDrawWallLeft();
			break;
		case 1:
			mazeDrawWallTop();
			break;
		case 2:
			mazeDrawWallRight();
			break;
		case 3:
			mazeDrawWallBottom();
			break;
		default:
			warning("mazeLoadState: breaking out of switch");
			break;
		}
	}
	// 136:3592
	var_i16_1f0a = 0xd;
	for (int16 i = 1; i <= arr_i16_1eb8[23]; i++) {
		_mazeInvItemCount[i] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
		var_i16_1f0a += 2;
	}
	// 136:35ec
	var_i16_1de4 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
	var_i16_1f0a += 2;
	if (var_i16_1de4 > 0) {
		_zbasic->text(kFontPuzzle, 0xc, 0, kSrcBic);
		for (int16 i = 1; i <= var_i16_1de4; i++) {
			var_i16_1dea = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
			var_i16_1f0a += 2;
			var_i16_106e = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
			var_i16_1f0a += 2;
			// 136:366e
			_toolbox->MoveTo(
				_screenGrid[var_i16_1dea].left + arr_i16_1eb8[2],
				_screenGrid[var_i16_1dea].top + arr_i16_1eb8[3]
			);
			_zbasic->unk_8();
			// the one place that used ZBasic's print API,
			// easier to cut out the middleman and use DrawString
			_toolbox->DrawString(_zbasic->chr(var_i16_106e));
			//_zbasic->print(_zbasic->chr(var_i16_106e));
			//_zbasic->printNewline();
			arr_i16_4338[var_i16_1dea] = var_i16_106e;
		}
	}
	// 136:3710
	// draw walls that are added by the visibility mask
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[arr_i16_1eb8[17]]);
	_toolbox->PenSize(5, 5);
	var_i16_1574 = 0;
	var_i16_1de6 = 0;
	var_i16_1f08 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
	// 136:3766
	debugC(5, kDebugLoading, "mazeLoadState: mask[%d] %04x", var_i16_1f0a, var_i16_1f08);
	var_i16_1f0a += 2;
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_1574++;
			if (arr_i16_1eb8[21] != 0) {
				if (((arr_i16_3738[var_i16_1574] & 1) == 1) && ((arr_i16_3738[var_i16_1574] & 0x10) == 0)) {
					if ((var_i16_1f08 & _bitLUT[var_i16_1de6]) != 0) {
						mazeAddWallLeft();
					}
					sub_136_3a30();
				}
				// 136:380c
				if (((arr_i16_3738[var_i16_1574] & 2) == 2) && ((arr_i16_3738[var_i16_1574] & 0x20) == 0)) {
					if ((var_i16_1f08 & _bitLUT[var_i16_1de6]) != 0) {
						mazeAddWallTop();
					}
					sub_136_3a30();
				}
				// 136:3886
				if (((arr_i16_3738[var_i16_1574] & 4) == 4) && ((arr_i16_3738[var_i16_1574] & 0x40) == 0)) {
					if ((var_i16_1f08 & _bitLUT[var_i16_1de6]) != 0) {
						mazeAddWallRight();
					}
					sub_136_3a30();
				}
				// 136:3900
				if (((arr_i16_3738[var_i16_1574] & 8) == 8) && ((arr_i16_3738[var_i16_1574] & 0x80) == 0)) {
					if ((var_i16_1f08 & _bitLUT[var_i16_1de6]) != 0) {
						mazeAddWallBottom();
					}
					sub_136_3a30();
				}
			}
			// 136:397c
			if (arr_i16_3b38[i*32 + j] > 0) {
				if ((var_i16_1f08 & _bitLUT[var_i16_1de6]) != 0) {
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
	warning(__func__);
	// 136:3a30
	var_i16_1de6++;
	if (var_i16_1de6 > 0xf) {
		var_i16_1de6 = 0;
	}
	if (var_i16_1de6 == 0) {
		var_i16_1f08 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1f0a, 2));
		debugC(5, kDebugLoading, "sub_136_3a30: mask[%d] %04x", var_i16_1f0a, var_i16_1f08);
		var_i16_1f0a += 2;
	}
	// 136:3a6e
}

void FoolGame::sub_136_3a70() {
	warning(__func__);
	// 136:3a70
	if (_activePuzzle == 0x57) {
		_activePuzzleStatus = 0x64;
		menuClickMessage();
		waitForClick();
	} else {
		// 136:3a8e
		if (_activePuzzleStatus < 0x64) {
			_activePuzzleStatus = 0x64;
		}
		var_i16_484 = 1 + (arr_i16_1eb8[24] - 0x14) / 0x23;
		var_i16_7e4 = 1 + (arr_i16_1eb8[25]) / 0x23;
		var_i16_9f2 = 1 + (SCREEN_HEIGHT - arr_i16_1eb8[26]) / 0x23;
		_toolbox->PenNormal();
		// 136:3b0e
		for (int16 j = 0; j <= 1; j++) {
			if (j == 1) {
				_toolbox->PenMode(kNotPatOr);
				_toolbox->PenPat(_patterns[1]);
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
					_toolbox->InvertRect(temp);
				} else {
					// 136:3c48
					_toolbox->PaintRect(temp);
				}
				// 136:3c56
				_toolbox->Delay(0);
			}
			// 136:3c64
		}
	}
	// 136:3c72
}

}
