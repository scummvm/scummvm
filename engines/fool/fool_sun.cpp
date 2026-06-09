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



// The Sun's map
void FoolGame::sunMapRun() {
	// 137:0004
	this->var_i16_c00 = 1;
	this->fillRect(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	this->arr_i16_1eb8[0] = 9;
	this->arr_i16_1eb8[1] = 9;
	this->arr_i16_1eb8[4] = 0x4b;
	this->arr_i16_1eb8[5] = -9;
	this->arr_i16_1eb8[6] = 0x23;
	this->arr_i16_1eb8[7] = 0x23;
	this->var_i16_484 = 0;
	for (int16 j = 0x1a; j <= 0x132; j += 0x23) {
		for (int16 i = 0x6e; i <= 0x186; i += 0x23) {
			this->var_i16_484++;
			_toolbox->SetRect(_screenGrid[this->var_i16_484], i, j, i+0x24, j+0x24);
		}
	}
	// 137:00fa
	this->var_i16_484 = 0;
	for (int16 j = 1; j <= 9; j++) {
		for (int16 i = 1; i <= 9; i++) {
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = this->var_i16_484;
		}
	}
	// 137:0148
	for (int i = 1; i <= 0x51; i++) {
		this->var_i16_484 = this->arr_i16_4c7c[i];
		if ((_puzzleFlags[this->var_i16_484] & (kFlagMenuDiamond | kFlagMenuEnabled)) != 0) {
			this->arr_i32_192c0[i] = _sunMapTilePic[i];
		} else {
			// 137:01b2
			this->arr_i32_192c0[i] = _sunMapTilePic[0x52];
		}
		// 137:01d8
		if ((_puzzleFlags[this->var_i16_484] & 4) != 0) {
			_puzzleFlags[this->var_i16_484] ^= 4;
			_puzzleFlags[i] |= kFlagMapTile;
		}
		// 137:0252
		if ((_puzzleFlags[i] & kFlagMapTile) != 0) {
			this->arr_i32_192c0[i] = _sunMapTilePic[0x53];
		}
		// 137:0294
	}
	// 137:02a2
	this->fillRect(0x14, 0x67, 0x156, 0x1b1, 2);
	for (int i = 1; i <= 0x51; i++) {
		_zbasic->picture(_screenGrid[i].left, _screenGrid[i].top, this->arr_i32_192c0[_sunMapTileID[i]]);
	}
	// 137:0336
	_stateFlags = kStateNull;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->InitCursor();
	this->sunMapCheckIfSolved();
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		while ((_stateFlags == kStateNull) && (!_activePuzzleSolved)) {
			this->getNextEvent(-1);
			if (_event.what == kMouseDown) {
				this->sunMapOnClick();
			}
		}
		// 137:038a
		if (_stateFlags == kStateUndo) {
			this->sunMapUndoMove();
		}
		if (_stateFlags == kStateSaveGame) {
			this->saveGame();
		}
	}
	// 137:03ce
	if (!_activePuzzleSolved) {
		return;
	}
	// JMP 0x1618
	this->var_i16_68c = 0;
	for (int j = 0; j <= 5; j++) {
		for (int i = 1; i <= 0x51; i++) {
			_toolbox->InvertRect(_screenGrid[i]);
		}
	}
	// 137:1654
	_activePuzzle = 0;
	_activePuzzleStatus = 0x64;
	_sunMapRestored = 1;
	_zbasic->menu(2, 7, 1, Common::U32String());
	for (int i = 1; i <= 0x50; i++) {
		_puzzleFlags[i] = kFlagMenuEnabled;
	}
	for (int j = 3; j <= 7; j++) {
		for (int i = 1; i <= 0x10; i++) {
			_zbasic->menu(j, i, 1, Common::U32String());
		}
	}
}

void FoolGame::sunMapOnClick() {
	// 137:03e2
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) ||
		(this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
		return;
	}
	// 137:0466
	_menuDisabled = true;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->PenNormal();
	_toolbox->PenSize(3, 3);
	_toolbox->PenMode(kPatXor);
	this->arr_i16_4758[0] = this->var_i16_68a;
	this->arr_i16_4758[1] = this->var_i16_68c;
	if (_puzzleFlags[_sunMapTileID[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] & kFlagMapTile) {
		this->sunMapRevealPiece();
		return;
	}
	this->sunMapDragSelect();
	// 137:0506
	_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
	this->arr_i16_4758[0] -= this->var_i16_68a;
	this->arr_i16_4758[1] -= this->var_i16_68c;
	this->sunMapMoveSelected();
	_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
	this->sunMapDropSelected();
	this->sunMapCheckIfSolved();
	this->waitForMouseUp();
	_zbasic->menu(8, 3, 1, Common::U32String());
	_menuDisabled = false;
}

void FoolGame::sunMapDragSelect() {
	// 137:0598
	do {
		this->getNextEvent(4);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->var_i16_68a >= this->arr_i16_4758[0]) {
			this->arr_i16_4758[2] = this->arr_i16_4758[0];
			this->arr_i16_4758[3] = this->var_i16_68a;
		} else {
		// 137:061a
			this->arr_i16_4758[2] = this->var_i16_68a;
			this->arr_i16_4758[3] = this->arr_i16_4758[0];
		}
		// 137:064a
		if (this->var_i16_68c >= this->arr_i16_4758[1]) {
			this->arr_i16_4758[6] = this->arr_i16_4758[1];
			this->arr_i16_4758[7] = this->var_i16_68c;
		} else {
			// 137:069e
			this->arr_i16_4758[6] = this->var_i16_68c;
			this->arr_i16_4758[7] = this->arr_i16_4758[1];
		}
		// 137:06ce
		this->arr_i16_4758[11] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].top;
		this->arr_i16_4758[12] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].left;
		this->arr_i16_4758[13] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].bottom;
		this->arr_i16_4758[14] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].right;
		// 137:084c
		_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
		this->delay(2);
		_toolbox->FrameRect(Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]));
		this->delay(1);
	} while (_event.what != kMouseUp);
	// 137:087e
}

void FoolGame::sunMapMoveSelected() {
	// 137:0880
	do {
		this->getNextEvent(2);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->arr_i16_4758[0] >= 0) {
			this->arr_i16_4758[4] = this->var_i16_68a;
			this->arr_i16_4758[5] = this->var_i16_68a + this->arr_i16_4758[0];
		} else {
			// 137:0900
			this->arr_i16_4758[4] = this->var_i16_68a + this->arr_i16_4758[0];
			this->arr_i16_4758[5] = this->var_i16_68a;
		}
		// 137:093c
		if (this->arr_i16_4758[1] >= 0) {
			this->arr_i16_4758[8] = this->var_i16_68c;
			this->arr_i16_4758[9] = this->var_i16_68c + this->arr_i16_4758[1];
		} else {
			// 137:098e
			this->arr_i16_4758[8] = this->var_i16_68c + this->arr_i16_4758[1];
			this->arr_i16_4758[9] = this->var_i16_68c;
		}
		// 137:09ca
		if (this->arr_i16_4758[4] < 1) {
			this->arr_i16_4758[4] = 1;
			this->arr_i16_4758[5] = 1 + ABS(this->arr_i16_4758[0]);
		}
		// 137:0a2e
		if (this->arr_i16_4758[8] < 1) {
			this->arr_i16_4758[8] = 1;
			this->arr_i16_4758[9] = 1 + ABS(this->arr_i16_4758[1]);
		}
		// 137:0a92
		if (this->arr_i16_4758[5] > this->arr_i16_1eb8[0]) {
			this->arr_i16_4758[4] = this->arr_i16_1eb8[0] - ABS(this->arr_i16_4758[0]);
			this->arr_i16_4758[5] = this->arr_i16_1eb8[0];
		}
		// 137:0b20
		if (this->arr_i16_4758[9] > this->arr_i16_1eb8[1]) {
			this->arr_i16_4758[8] = this->arr_i16_1eb8[1] - ABS(this->arr_i16_4758[1]);
			this->arr_i16_4758[9] = this->arr_i16_1eb8[1];
		}
		// 137:0bae
		this->arr_rect_4776.top = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].top;
		this->arr_rect_4776.left = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].left;
		this->arr_rect_4776.bottom = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].bottom;
		this->arr_rect_4776.right = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].right;
		_toolbox->FrameRect(this->arr_rect_4776);
		this->delay(2);
		_toolbox->FrameRect(this->arr_rect_4776);
		this->delay(1);
	} while (_event.what != kMouseDown);
}

void FoolGame::sunMapUndoMove() {
	// 137:0d60
	_stateFlags = kStateNull;
	_zbasic->swapInt(this->arr_i16_4758[2], this->arr_i16_4758[4]);
	_zbasic->swapInt(this->arr_i16_4758[3], this->arr_i16_4758[5]);
	_zbasic->swapInt(this->arr_i16_4758[6], this->arr_i16_4758[8]);
	_zbasic->swapInt(this->arr_i16_4758[7], this->arr_i16_4758[9]);
	this->sunMapDropSelected();
}

void FoolGame::sunMapDropSelected() {
	// 137:0dde
	this->var_i16_1aa4 = 0x51;
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			this->var_i16_1aa4++;
			this->arr_i16_3738[this->var_i16_1aa4] = _sunMapTileID[this->arr_i16_2f38[i*32 + j]];
			// 137:0e5a
			_sunMapTileID[this->arr_i16_2f38[i*32 + j]] = 0;
		}
	}

	// 137:0ec2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	this->var_i16_1aa4 = 0x51;
	for (int j = this->arr_i16_4758[8]; j <= this->arr_i16_4758[9]; j++) {
		for (int i = this->arr_i16_4758[4]; i <= this->arr_i16_4758[5]; i++) {
			this->var_i16_1aa4++;
			if (_sunMapTileID[this->arr_i16_2f38[i*32 + j]] > 0) {
				this->var_i16_1aa6++;
				this->arr_i16_3738[this->var_i16_1aa6] = _sunMapTileID[this->arr_i16_2f38[i*32 + j]];
			}
			// 137:0f7c
			_sunMapTileID[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa4];
			_zbasic->picture(
				_screenGrid[this->arr_i16_2f38[i*32 + j]].left,
				_screenGrid[this->arr_i16_2f38[i*32 + j]].top,
				this->arr_i32_192c0[_sunMapTileID[this->arr_i16_2f38[i*32 + j]]]
			);
		}
	}
	this->var_i16_1aa6 = this->var_i16_1aa4;
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			if (_sunMapTileID[this->arr_i16_2f38[i*32 + j]] == 0) {
				// 137:1114
				this->var_i16_1aa6++;
				_sunMapTileID[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa6];
				_zbasic->picture(
					_screenGrid[this->arr_i16_2f38[i*32 + j]].left,
					_screenGrid[this->arr_i16_2f38[i*32 + j]].top,
					this->arr_i32_192c0[_sunMapTileID[this->arr_i16_2f38[i*32 + j]]]
				);
			}
			// 137:1214
		}
	}

	// 137:124c
}

void FoolGame::sunMapCheckIfSolved() {
	// 137:124e
	_activePuzzleSolved = true;
	for (int i = 1; i <= 0x51; i++) {
		if (_sunMapTileID[i] != i) {
			_activePuzzleSolved = false;
			i = 0x51;
		}
	}
	// 137:128e
	if (_activePuzzleSolved) {
		for (int i = 1; i <= 0x51; i++) {
			this->var_i16_484 = this->arr_i16_4c7c[i];
			if ((_puzzleFlags[this->var_i16_484] & kFlagMapTile) || (_puzzleFlags[this->var_i16_484] == kFlagMenuDisabled)) {
				_activePuzzleSolved = false;
				i = 0x51;
			}
			// 137:12fa
		}
	}
	// 137:1306
	if (_activePuzzleSolved) {
		_stateFlags = kStateReturn;
	}
}

void FoolGame::sunMapRevealPiece() {
	// 137:131a
	_puzzleFlags[_sunMapTileID[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] ^= kFlagMapTile;
	this->arr_i32_192c0[_sunMapTileID[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]] = _sunMapTilePic[_sunMapTileID[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]]];
	// 137:142c
	// unrolled loop
	this->arr_i16_4758[0] = _screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].top;
	this->arr_i16_4758[1] = _screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].left;
	this->arr_i16_4758[2] = _screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].bottom;
	this->arr_i16_4758[3] = _screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].right;
	// 137:1490
	for (int16 i = 0; i <= 0xf; i++) {
		this->arr_i16_4758[0] += i;
		this->arr_i16_4758[1] += i;
		this->arr_i16_4758[2] -= i;
		this->arr_i16_4758[3] -= i;
		Common::Rect temp;
		temp.top = this->arr_i16_4758[0];
		temp.left = this->arr_i16_4758[1];
		temp.bottom = this->arr_i16_4758[2];
		temp.right = this->arr_i16_4758[3];
		_toolbox->FrameRect(temp);
		this->delay(1);
	}
	// 137:1550
	_zbasic->picture(
		_screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].left,
		_screenGrid[this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c]].top,
		this->arr_i32_192c0[_sunMapTileID[this->arr_i16_2f38[this->var_i16_68a*32+this->var_i16_68c]]]
	);
	this->waitForMouseUp();
	this->sunMapCheckIfSolved();
	_menuDisabled = false;
}


}
