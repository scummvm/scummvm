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

#define OFF(x) (_zstrOffset[kOffsetJigsaw] + (x))

extern Toolbox *g_toolbox;

// jigsaw puzzle game
void FoolGame::jigsawRun() {
	// 132:0004
	this->var_i16_c00 = 1;
	this->fetchPuzzleData();
	int16 picID = puzzlesReadShort();
	for (int i = 0; i <= 0xe; i++) {
		this->arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 132:0044
	// puzzle dimensions
	this->_jigsawPieceCount = this->arr_i16_1eb8[0] * this->arr_i16_1eb8[1];
	for (int i = 1; i <= this->_jigsawPieceCount; i++) {
		this->arr_i16_3738[i] = puzzlesReadByte();
	}
	// 132:00a2
	this->arr_i32_192c0[0] = g_toolbox->GetPicture(picID);
	_zbasic->picture(0, 0x14, this->arr_i32_192c0[0]);
	g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
	this->var_i16_1aa0 = 4 + ((this->arr_i16_1eb8[12] + 1)*2) * ((this->arr_i16_1eb8[13]+0x10)/0x10);
	// 132:0136
	this->var_i16_1aa0 = (this->var_i16_1aa0 / 2) + 1;
	this->var_i16_484 = 0;
	this->var_i16_68c = this->arr_i16_1eb8[8];
	do {
		this->var_i16_68a = this->arr_i16_1eb8[10];
		do {
			this->var_i16_484++;
			g_toolbox->SetRect(
				_screenGrid[this->var_i16_484],
				this->var_i16_68a + 1,
				this->var_i16_68c + 1,
				this->var_i16_68a + this->arr_i16_1eb8[13],
				this->var_i16_68c + this->arr_i16_1eb8[12]
			);
			//this->var_i16_1aa2 = 0x2af8 + this->var_i16_1aa0*(this->arr_i16_3738[this->var_i16_484] - 1);
			// 132:0216
			_jigsawPieces[this->arr_i16_3738[this->var_i16_484]] = BitMap(new Graphics::ManagedSurface());
			_zbasic->get(
				_screenGrid[this->var_i16_484].left,
				_screenGrid[this->var_i16_484].top,
				_screenGrid[this->var_i16_484].right,
				_screenGrid[this->var_i16_484].bottom,
				_jigsawPieces[this->arr_i16_3738[this->var_i16_484]]
			);
		} while (_zbasic->incrAndCheck(
			this->var_i16_68a,
			this->arr_i16_1eb8[11],
			this->arr_i16_1eb8[6]
		));
	// 132:02dc
	} while (_zbasic->incrAndCheck(
		this->var_i16_68c,
		this->arr_i16_1eb8[9],
		this->arr_i16_1eb8[7]
	));
	// 132:030c
	this->var_i16_484 = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->var_i16_484++;
			this->arr_i16_2f38[i*32 + j] = this->var_i16_484;
		}
	}
	// 132:0376
	// rearrange picture tiles to match state
	if (!_activePuzzleBuffer.empty()) { // was: str(OFF(0))
		for (int i = 1; i <= this->_jigsawPieceCount; i++) {
			Common::String tileData = _zbasic->midStr(_activePuzzleBuffer, i*2 - 1, 2);
			this->arr_i16_3738[i] = _zbasic->decodeInt(tileData);
			_zbasic->put(
				_screenGrid[i].left,
				_screenGrid[i].top,
				_jigsawPieces[this->arr_i16_3738[i]],
				kSrcCopy
			);
		// 132:0452
		}
	}
	// 132:0464
	_stateFlags = kStateNull;
	_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->InitCursor();
	this->jigsawCheckIfSolved();
	// JMP 0x4d8

	// 132:04d8
	// input loop
	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 132:0484
		// 132:049c
		while ((_stateFlags == kStateNull) && !_activePuzzleSolved) {
			// 132:0488
			this->getNextEvent(-1);
			if (_event.what == kMouseDown) {
				this->jigsawOnClick();
			}
		}
		// 132:04b8
		if (_stateFlags == kStateUndo) {
			this->jigsawCancelSelect();
		}
		if (_stateFlags == kStateSaveGame) {
			this->jigsawStoreState();
			this->saveGame();
		}
	}
	// 132:0500
	if (_activePuzzleSolved) {
		this->jigsawSuccess();
	}
	this->jigsawStoreState();
	// JMP 0x14a6
	// 132:14a6
}

void FoolGame::jigsawOnClick() {
	// 132:0518
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || ((this->var_i16_68a - (this->arr_i16_1eb8[0])) > 0) || (this->var_i16_68c < 1) || ((this->var_i16_68c - (this->arr_i16_1eb8[1])) > 0)) {
		return;
	}
	// 132:059c
	_menuDisabled = true;
	_zbasic->menu(8, 3, 0, Common::U32String());
	g_toolbox->PenNormal();
	g_toolbox->PenSize(3, 3);
	g_toolbox->PenMode(kPatXor);
	// 132:05c4
	this->arr_i16_4758[0] = this->var_i16_68a;
	this->arr_i16_4758[1] = this->var_i16_68c;

	// change select area while mouse button held down
	this->jigsawDragSelect();
	// leave an XOR outline on the page for the selected block
	Common::Rect temp(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
	g_toolbox->FrameRect(temp);
	this->arr_i16_4758[0] -= this->var_i16_68a;
	this->arr_i16_4758[1] -= this->var_i16_68c;

	// 132:0642
	// move select area to new target
	this->jigsawMoveSelected();
	// remove XOR outline
	temp = Common::Rect(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
	g_toolbox->FrameRect(temp);

	this->jigsawDropSelected();
	this->jigsawCheckIfSolved();
	this->waitForMouseUp();
	_zbasic->menu(8, 3, 1, Common::U32String());
	_menuDisabled = false;
}

void FoolGame::jigsawDragSelect() {
	// 132:067a
	// change select area while mouse button held down
	do {
		this->getNextEvent(4);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->var_i16_68a >= this->arr_i16_4758[0]) {
			this->arr_i16_4758[2] = this->arr_i16_4758[0];
			this->arr_i16_4758[3] = this->var_i16_68a;
		} else {
			// 132:06fc
			this->arr_i16_4758[2] = this->var_i16_68a;
			this->arr_i16_4758[3] = this->arr_i16_4758[0];
		}
		// 132:072c
		if (this->var_i16_68c >= this->arr_i16_4758[1]) {
			this->arr_i16_4758[6] = this->arr_i16_4758[1];
			this->arr_i16_4758[7] = this->var_i16_68c;
		} else {
			this->arr_i16_4758[6] = this->var_i16_68c;
			this->arr_i16_4758[7] = this->arr_i16_4758[1];
		}
		// 132:07b0
		this->arr_i16_4758[11] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].top;
		// 132:080e
		this->arr_i16_4758[12] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[2]*32 + this->arr_i16_4758[6]]].left;
		this->arr_i16_4758[13] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].bottom;
		this->arr_i16_4758[14] = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[3]*32 + this->arr_i16_4758[7]]].right;
		Common::Rect temp(this->arr_i16_4758[12], this->arr_i16_4758[11], this->arr_i16_4758[14], this->arr_i16_4758[13]);
		// 132:092e
		g_toolbox->FrameRect(temp);
		this->delay(2);
		g_toolbox->FrameRect(temp);
		this->delay(1);
	} while (_event.what != kMouseUp);
}

void FoolGame::jigsawMoveSelected() {
	// 132:0962
	// move select area to new target
	warning("call: %s", __func__);
	do {
		this->getNextEvent(2);
		this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
		this->sub_128_342(this->var_i16_68a, this->var_i16_68c);
		if (this->arr_i16_4758[0] >= 0) {
			this->arr_i16_4758[4] = this->var_i16_68a;
			this->arr_i16_4758[5] = this->var_i16_68a + this->arr_i16_4758[0];
		} else {
		// 132:09e2
			this->arr_i16_4758[4] = this->var_i16_68a + this->arr_i16_4758[0];
			this->arr_i16_4758[5] = this->var_i16_68a;
		}
		// 132:0a1e
		if (this->arr_i16_4758[1] >= 0) {
			this->arr_i16_4758[8] = this->var_i16_68c;
			this->arr_i16_4758[9] = this->var_i16_68c + this->arr_i16_4758[1];
		} else {
			// 132:0a70
			this->arr_i16_4758[8] = this->var_i16_68c + this->arr_i16_4758[1];
			this->arr_i16_4758[9] = this->var_i16_68c;
		}
		// 132:0aac
		if (this->arr_i16_4758[4] < 1) {
			this->arr_i16_4758[4] = 1;
			this->arr_i16_4758[5] = 1 + ABS(this->arr_i16_4758[0]);
		}
		// 132:0b10
		if (this->arr_i16_4758[8] < 1) {
			this->arr_i16_4758[8] = 1;
			this->arr_i16_4758[9] = 1 + ABS(this->arr_i16_4758[1]);
		}
		// 132:0b74
		if ((this->arr_i16_4758[5] - this->arr_i16_1eb8[0]) > 0) {
			this->arr_i16_4758[4] = this->arr_i16_1eb8[0] - ABS(this->arr_i16_4758[0]);
			this->arr_i16_4758[5] = this->arr_i16_1eb8[0];
		}
		// 132:0c02
		if ((this->arr_i16_4758[9] - this->arr_i16_1eb8[1]) > 0) {
			this->arr_i16_4758[8] = this->arr_i16_1eb8[1] - ABS(this->arr_i16_4758[1]);
			this->arr_i16_4758[9] = this->arr_i16_1eb8[1];
		}
		// 132:0c90
		this->arr_rect_4776.top = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].top;
		this->arr_rect_4776.left = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[4]*32 + this->arr_i16_4758[8]]].left;
		// 132:0d4e
		this->arr_rect_4776.bottom = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].bottom;
		this->arr_rect_4776.right = _screenGrid[this->arr_i16_2f38[this->arr_i16_4758[5]*32 + this->arr_i16_4758[9]]].right;
		// 132:0e0e
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->delay(2);
		g_toolbox->FrameRect(this->arr_rect_4776);
		this->delay(1);
	}
	while (!((_event.what == kMouseDown) && (_event.where.y > 0x14)));
}

void FoolGame::jigsawCancelSelect() {
	// 132:0e5a
	_stateFlags = kStateNull;
	_zbasic->swapInt(this->arr_i16_4758[2], this->arr_i16_4758[4]);
	_zbasic->swapInt(this->arr_i16_4758[3], this->arr_i16_4758[5]);
	_zbasic->swapInt(this->arr_i16_4758[6], this->arr_i16_4758[8]);
	_zbasic->swapInt(this->arr_i16_4758[7], this->arr_i16_4758[9]);
	jigsawDropSelected();
}

void FoolGame::jigsawDropSelected() {
	// 132:0ed8
	// skip the tile pointer to past the end of the list
	this->var_i16_1aa4 = this->_jigsawPieceCount;

	// arr_i16_4758[2]: src_left
	// arr_i16_4758[3]: src_right
	// arr_i16_4758[4]: dest_left
	// arr_i16_4758[5]: dest_right
	// arr_i16_4758[6]: src_top
	// arr_i16_4758[7]: src_bottom
	// arr_i16_4758[8]: dest_top
	// arr_i16_4758[9]: dest_bottom

	// iterate through src tile positions
	for (int j = this->arr_i16_4758[6]; j <=  this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			this->var_i16_1aa4++;
			// buffer 1: make a copy of source tile positions
			this->arr_i16_3738[this->var_i16_1aa4] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			debug(8, "(%d, %d) [src]: %d -> buffer 1", i, j, this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]);
			// 132:0f54
			// buffer 0: zero out source tiles
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = 0;
		}
	}
	// 132:0fbc
	// tile pointer to buffer 2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	this->var_i16_1aa4 = this->_jigsawPieceCount;

	// 132:0fc8
	// iterate through dest tile positions
	for (int j = this->arr_i16_4758[8]; j <= this->arr_i16_4758[9]; j++) {
		for (int i = this->arr_i16_4758[4]; i <= this->arr_i16_4758[5]; i++) {
			this->var_i16_1aa4++;

			// if the dest tile isn't part of the src tile set, copy to buffer 2
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] > 0) {
				this->var_i16_1aa6++;
				debug(8, "(%d, %d) [dest]: %d -> buffer 2", i, j, this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]);
				this->arr_i16_3738[this->var_i16_1aa6] = this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]];
			}
			// 132:1076
			// write src tile from buffer 1 to dest position
			this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] = this->arr_i16_3738[this->var_i16_1aa4];
			debug(8, "(%d, %d) [dest]: %d <- buffer 1", i, j, this->arr_i16_3738[this->var_i16_1aa4]);
			// 132:10ba
			// 0x2af8, this->var_i16_1aa0 * (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] - 1)
			// blit tiles to screen
			_zbasic->put(
				_screenGrid[this->arr_i16_2f38[i*32 + j]].left,
				_screenGrid[this->arr_i16_2f38[i*32 + j]].top,
				_jigsawPieces[this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]],
				kSrcCopy
			);
		}
	}
	// 132:11c8
	// tile pointer to buffer 2
	this->var_i16_1aa6 = this->var_i16_1aa4;
	// iterate through source tile positions
	for (int j = this->arr_i16_4758[6]; j <= this->arr_i16_4758[7]; j++) {
		for (int i = this->arr_i16_4758[2]; i <= this->arr_i16_4758[3]; i++) {
			// if the source tile has been zeroed out, fill it with tile from buffer 2
			if (this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]] == 0) {
				this->var_i16_1aa6++;
				debug(8, "(%d, %d) [src]: %d <- buffer 2", i, j, this->arr_i16_3738[this->var_i16_1aa6]);
				this->arr_i16_3738[this->arr_i16_2f38[i*32+j]] = this->arr_i16_3738[this->var_i16_1aa6];
				_zbasic->put(
					_screenGrid[this->arr_i16_2f38[i*32+j]].left,
					_screenGrid[this->arr_i16_2f38[i*32+j]].top,
					_jigsawPieces[this->arr_i16_3738[this->arr_i16_2f38[i*32 + j]]],
					kSrcCopy
				);
			}
			// 132:134a
		}
	}
}

void FoolGame::jigsawStoreState() {
	// convert jigsaw positions to string
	// 132:1384
	_activePuzzleBuffer.clear(); // was: str(OFF(1))
	for (int i = 1; i <= this->_jigsawPieceCount; i++) {
		_activePuzzleBuffer += _zbasic->encodeInt(this->arr_i16_3738[i]);
	}
}


void FoolGame::jigsawCheckIfSolved() {
	// check if puzzle is solved
	// 132:13ea
	_activePuzzleSolved = true;
	for (int i = 1; i <= this->_jigsawPieceCount; i++) {
		if (this->arr_i16_3738[i] != i) {
			_activePuzzleSolved = false;
			i = this->_jigsawPieceCount;
		}
	// 132:1420
	}
	// 132:1430
	if (_activePuzzleSolved) {
		_stateFlags = kStateReturn;
	}
}

void FoolGame::jigsawSuccess() {
	// strobe puzzle pieces once on victory
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
		for (int j = 0; j <= 1; j++) {
			for (int i = 1; i <= this->_jigsawPieceCount; i++) {
				g_toolbox->InvertRect(_screenGrid[i]);
				this->delay(1);
			}
		}
	}
	// 132:149c
	this->menuClickMessage();
	this->waitForClick();
}

};
