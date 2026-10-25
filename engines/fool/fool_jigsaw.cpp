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

#include "graphics/mactoolbox/toolbox.h"
#include "graphics/managed_surface.h"

#include "fool/fool_game.h"
#include "fool/zbasic.h"

namespace Fool {

#define OFF(x) (_zstrOffset[kOffsetJigsaw] + (x))



// jigsaw puzzle game
void FoolGame::jigsawRun() {
	// 132:0004
	var_i16_c00 = 1;
	fetchPuzzleData();
	int16 picID = puzzlesReadShort();
	_gridInfo = puzzlesReadGridInfo();
	// 132:0044
	// puzzle dimensions
	_jigsawPieceCount = _gridInfo.xCount * _gridInfo.yCount;
	for (int i = 1; i <= _jigsawPieceCount; i++) {
		arr_i16_3738[i] = puzzlesReadByte();
	}
	// 132:00a2
	arr_i32_192c0[0] = _toolbox->GetPicture(picID);
	_zbasic->picture(0, 0x14, arr_i32_192c0[0]);
	_toolbox->ReleaseResource(arr_i32_192c0[0]);
	var_i16_1aa0 = 4 + ((_gridInfo.xInnerSpan + 1)*2) * ((_gridInfo.yInnerSpan+0x10)/0x10);
	// 132:0136
	var_i16_1aa0 = (var_i16_1aa0 / 2) + 1;
	int16 gridIndex = 0;
	int16 gridX = 0;
	int16 gridY = 0;
	gridY = _gridInfo.yStartPos;
	do {
		gridX = _gridInfo.xStartPos;
		do {
			gridIndex++;
			_toolbox->SetRect(
				_screenGrid[gridIndex],
				gridX + 1,
				gridY + 1,
				gridX + _gridInfo.xInnerSpan,
				gridY + _gridInfo.yInnerSpan
			);
			// 132:0216
			_jigsawPieces[arr_i16_3738[gridIndex]] = BitMap(new Graphics::ManagedSurface());
			_zbasic->get(
				_screenGrid[gridIndex].left,
				_screenGrid[gridIndex].top,
				_screenGrid[gridIndex].right,
				_screenGrid[gridIndex].bottom,
				_jigsawPieces[arr_i16_3738[gridIndex]]
			);
		} while (_zbasic->incrAndCheck(
			gridX,
			_gridInfo.xEndPos,
			_gridInfo.xOuterSpan
		));
	// 132:02dc
	} while (_zbasic->incrAndCheck(
		gridY,
		_gridInfo.yEndPos,
		_gridInfo.yOuterSpan
	));
	// 132:030c
	gridIndex = 0;
	for (int j = 1; j <= _gridInfo.yCount; j++) {
		for (int i = 1; i <= _gridInfo.xCount; i++) {
			gridIndex++;
			arr_i16_2f38[i*32 + j] = gridIndex;
		}
	}
	// 132:0376
	// rearrange picture tiles to match state
	if (!_activePuzzleBuffer.empty()) { // was: str(OFF(0))
		for (int i = 1; i <= _jigsawPieceCount; i++) {
			Common::String tileData = _zbasic->midStr(_activePuzzleBuffer, i*2 - 1, 2);
			arr_i16_3738[i] = _zbasic->decodeInt(tileData);
			_zbasic->put(
				_screenGrid[i].left,
				_screenGrid[i].top,
				_jigsawPieces[arr_i16_3738[i]],
				Graphics::MacToolbox::kSrcCopy
			);
		// 132:0452
		}
	}
	// 132:0464
	_stateFlags = kStateNull;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->InitCursor();
	jigsawCheckIfSolved();
	// JMP 0x4d8

	// 132:04d8
	// input loop
	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 132:0484
		// 132:049c
		while ((_stateFlags == kStateNull) && !_activePuzzleSolved) {
			// 132:0488
			getNextEvent(-1);
			if (_event.what == Graphics::MacToolbox::kMouseDown) {
				jigsawOnClick();
			}
		}
		// 132:04b8
		if (_stateFlags == kStateUndo) {
			jigsawCancelSelect();
		}
		if (_stateFlags == kStateSaveGame) {
			jigsawStoreState();
			saveGame();
		}
	}
	// 132:0500
	if (_activePuzzleSolved) {
		jigsawSuccess();
	}
	jigsawStoreState();
	// JMP 0x14a6
	// 132:14a6
}

void FoolGame::jigsawOnClick() {
	// 132:0518
	int16 gridX = 0;
	int16 gridY = 0;
	getGridFromMouse(gridX, gridY);
	if ((gridX < 1) || ((gridX - (_gridInfo.xCount)) > 0) || (gridY < 1) || ((gridY - (_gridInfo.yCount)) > 0)) {
		return;
	}
	// 132:059c
	_menuDisabled = true;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->PenNormal();
	_toolbox->PenSize(3, 3);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	// 132:05c4
	arr_i16_4758[0] = gridX;
	arr_i16_4758[1] = gridY;

	// change select area while mouse button held down
	jigsawDragSelect(gridX, gridY);
	// leave an XOR outline on the page for the selected block
	Common::Rect temp(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]);
	_toolbox->FrameRect(temp);
	arr_i16_4758[0] -= gridX;
	arr_i16_4758[1] -= gridY;

	// 132:0642
	// move select area to new target
	jigsawMoveSelected(gridX, gridY);
	// remove XOR outline
	temp = Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]);
	_toolbox->FrameRect(temp);

	jigsawDropSelected();
	jigsawCheckIfSolved();
	waitForMouseUp();
	_zbasic->menu(8, 3, 1, Common::U32String());
	_menuDisabled = false;
}

void FoolGame::jigsawDragSelect(int16 &gridX, int16 &gridY) {
	// 132:067a
	// change select area while mouse button held down
	do {
		getNextEvent(4);
		getGridFromMouse(gridX, gridY);
		clampToGrid(gridX, gridY);
		if (gridX >= arr_i16_4758[0]) {
			arr_i16_4758[2] = arr_i16_4758[0];
			arr_i16_4758[3] = gridX;
		} else {
			// 132:06fc
			arr_i16_4758[2] = gridX;
			arr_i16_4758[3] = arr_i16_4758[0];
		}
		// 132:072c
		if (gridY >= arr_i16_4758[1]) {
			arr_i16_4758[6] = arr_i16_4758[1];
			arr_i16_4758[7] = gridY;
		} else {
			arr_i16_4758[6] = gridY;
			arr_i16_4758[7] = arr_i16_4758[1];
		}
		// 132:07b0
		arr_i16_4758[11] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].top;
		// 132:080e
		arr_i16_4758[12] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].left;
		arr_i16_4758[13] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].bottom;
		arr_i16_4758[14] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].right;
		Common::Rect temp(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]);
		// 132:092e
		_toolbox->FrameRect(temp);
		delay(2);
		_toolbox->FrameRect(temp);
		delay(1);
	} while (_event.what != Graphics::MacToolbox::kMouseUp);
}

void FoolGame::jigsawMoveSelected(int16 &gridX, int16 &gridY) {
	// 132:0962
	// move select area to new target
	do {
		getNextEvent(2);
		getGridFromMouse(gridX, gridY);
		clampToGrid(gridX, gridY);
		if (arr_i16_4758[0] >= 0) {
			arr_i16_4758[4] = gridX;
			arr_i16_4758[5] = gridX + arr_i16_4758[0];
		} else {
		// 132:09e2
			arr_i16_4758[4] = gridX + arr_i16_4758[0];
			arr_i16_4758[5] = gridX;
		}
		// 132:0a1e
		if (arr_i16_4758[1] >= 0) {
			arr_i16_4758[8] = gridY;
			arr_i16_4758[9] = gridY + arr_i16_4758[1];
		} else {
			// 132:0a70
			arr_i16_4758[8] = gridY + arr_i16_4758[1];
			arr_i16_4758[9] = gridY;
		}
		// 132:0aac
		if (arr_i16_4758[4] < 1) {
			arr_i16_4758[4] = 1;
			arr_i16_4758[5] = 1 + ABS(arr_i16_4758[0]);
		}
		// 132:0b10
		if (arr_i16_4758[8] < 1) {
			arr_i16_4758[8] = 1;
			arr_i16_4758[9] = 1 + ABS(arr_i16_4758[1]);
		}
		// 132:0b74
		if ((arr_i16_4758[5] - _gridInfo.xCount) > 0) {
			arr_i16_4758[4] = _gridInfo.xCount - ABS(arr_i16_4758[0]);
			arr_i16_4758[5] = _gridInfo.xCount;
		}
		// 132:0c02
		if ((arr_i16_4758[9] - _gridInfo.yCount) > 0) {
			arr_i16_4758[8] = _gridInfo.yCount - ABS(arr_i16_4758[1]);
			arr_i16_4758[9] = _gridInfo.yCount;
		}
		// 132:0c90
		arr_rect_4776.top = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].top;
		arr_rect_4776.left = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].left;
		// 132:0d4e
		arr_rect_4776.bottom = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].bottom;
		arr_rect_4776.right = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].right;
		// 132:0e0e
		_toolbox->FrameRect(arr_rect_4776);
		delay(2);
		_toolbox->FrameRect(arr_rect_4776);
		delay(1);
	}
	while (!((_event.what == Graphics::MacToolbox::kMouseDown) && (_event.where.y > 0x14)));
}

void FoolGame::jigsawCancelSelect() {
	// 132:0e5a
	_stateFlags = kStateNull;
	_zbasic->swapInt(arr_i16_4758[2], arr_i16_4758[4]);
	_zbasic->swapInt(arr_i16_4758[3], arr_i16_4758[5]);
	_zbasic->swapInt(arr_i16_4758[6], arr_i16_4758[8]);
	_zbasic->swapInt(arr_i16_4758[7], arr_i16_4758[9]);
	jigsawDropSelected();
}

void FoolGame::jigsawDropSelected() {
	// 132:0ed8
	// skip the tile pointer to past the end of the list
	var_i16_1aa4 = _jigsawPieceCount;

	// arr_i16_4758[2]: src_left
	// arr_i16_4758[3]: src_right
	// arr_i16_4758[4]: dest_left
	// arr_i16_4758[5]: dest_right
	// arr_i16_4758[6]: src_top
	// arr_i16_4758[7]: src_bottom
	// arr_i16_4758[8]: dest_top
	// arr_i16_4758[9]: dest_bottom

	// iterate through src tile positions
	for (int j = arr_i16_4758[6]; j <=  arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			var_i16_1aa4++;
			// buffer 1: make a copy of source tile positions
			arr_i16_3738[var_i16_1aa4] = arr_i16_3738[arr_i16_2f38[i*32 + j]];
			debug(8, "(%d, %d) [src]: %d -> buffer 1", i, j, arr_i16_3738[arr_i16_2f38[i*32 + j]]);
			// 132:0f54
			// buffer 0: zero out source tiles
			arr_i16_3738[arr_i16_2f38[i*32 + j]] = 0;
		}
	}
	// 132:0fbc
	// tile pointer to buffer 2
	var_i16_1aa6 = var_i16_1aa4;
	var_i16_1aa4 = _jigsawPieceCount;

	// 132:0fc8
	// iterate through dest tile positions
	for (int j = arr_i16_4758[8]; j <= arr_i16_4758[9]; j++) {
		for (int i = arr_i16_4758[4]; i <= arr_i16_4758[5]; i++) {
			var_i16_1aa4++;

			// if the dest tile isn't part of the src tile set, copy to buffer 2
			if (arr_i16_3738[arr_i16_2f38[i*32 + j]] > 0) {
				var_i16_1aa6++;
				debug(8, "(%d, %d) [dest]: %d -> buffer 2", i, j, arr_i16_3738[arr_i16_2f38[i*32 + j]]);
				arr_i16_3738[var_i16_1aa6] = arr_i16_3738[arr_i16_2f38[i*32 + j]];
			}
			// 132:1076
			// write src tile from buffer 1 to dest position
			arr_i16_3738[arr_i16_2f38[i*32 + j]] = arr_i16_3738[var_i16_1aa4];
			debug(8, "(%d, %d) [dest]: %d <- buffer 1", i, j, arr_i16_3738[var_i16_1aa4]);
			// 132:10ba
			// 0x2af8, var_i16_1aa0 * (arr_i16_3738[arr_i16_2f38[i*32 + j]] - 1)
			// blit tiles to screen
			_zbasic->put(
				_screenGrid[arr_i16_2f38[i*32 + j]].left,
				_screenGrid[arr_i16_2f38[i*32 + j]].top,
				_jigsawPieces[arr_i16_3738[arr_i16_2f38[i*32 + j]]],
				Graphics::MacToolbox::kSrcCopy
			);
		}
	}
	// 132:11c8
	// tile pointer to buffer 2
	var_i16_1aa6 = var_i16_1aa4;
	// iterate through source tile positions
	for (int j = arr_i16_4758[6]; j <= arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			// if the source tile has been zeroed out, fill it with tile from buffer 2
			if (arr_i16_3738[arr_i16_2f38[i*32 + j]] == 0) {
				var_i16_1aa6++;
				debug(8, "(%d, %d) [src]: %d <- buffer 2", i, j, arr_i16_3738[var_i16_1aa6]);
				arr_i16_3738[arr_i16_2f38[i*32+j]] = arr_i16_3738[var_i16_1aa6];
				_zbasic->put(
					_screenGrid[arr_i16_2f38[i*32+j]].left,
					_screenGrid[arr_i16_2f38[i*32+j]].top,
					_jigsawPieces[arr_i16_3738[arr_i16_2f38[i*32 + j]]],
					Graphics::MacToolbox::kSrcCopy
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
	for (int i = 1; i <= _jigsawPieceCount; i++) {
		_activePuzzleBuffer += _zbasic->encodeInt(arr_i16_3738[i]);
	}
}


void FoolGame::jigsawCheckIfSolved() {
	// check if puzzle is solved
	// 132:13ea
	_activePuzzleSolved = true;
	for (int i = 1; i <= _jigsawPieceCount; i++) {
		if (arr_i16_3738[i] != i) {
			_activePuzzleSolved = false;
			i = _jigsawPieceCount;
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
			for (int i = 1; i <= _jigsawPieceCount; i++) {
				_toolbox->InvertRect(_screenGrid[i]);
				delay(1);
			}
		}
	}
	// 132:149c
	menuClickMessage();
	waitForClick();
}

} // End of namespace Fool
