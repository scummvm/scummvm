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

#include "fool/fool_game.h"
#include "fool/zbasic.h"

namespace Fool {



// The Sun's map
void FoolGame::sunMapRun() {
	// 137:0004
	var_i16_c00 = 1;
	fillRect(1, Graphics::MacToolbox::kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	_gridInfo.xCount = 9;
	_gridInfo.yCount = 9;
	_gridInfo.xOrigin = 0x4b;
	_gridInfo.yOrigin = -9;
	_gridInfo.xOuterSpan = 0x23;
	_gridInfo.yOuterSpan = 0x23;
	int16 gridIndex = 0;
	for (int16 j = 0x1a; j <= 0x132; j += 0x23) {
		for (int16 i = 0x6e; i <= 0x186; i += 0x23) {
			gridIndex++;
			_toolbox->SetRect(_screenGrid[gridIndex], i, j, i+0x24, j+0x24);
		}
	}
	// 137:00fa
	gridIndex = 0;
	for (int16 j = 1; j <= 9; j++) {
		for (int16 i = 1; i <= 9; i++) {
			gridIndex++;
			arr_i16_2f38[i*32 + j] = gridIndex;
		}
	}
	// 137:0148
	for (int i = 1; i <= 0x51; i++) {
		int16 puzzleIndex = arr_i16_4c7c[i];
		if ((_puzzleFlags[puzzleIndex] & (kFlagMenuDiamond | kFlagMenuEnabled)) != 0) {
			arr_i32_192c0[i] = _sunMapTilePic[i];
		} else {
			// 137:01b2
			arr_i32_192c0[i] = _sunMapTilePic[0x52];
		}
		// 137:01d8
		if ((_puzzleFlags[puzzleIndex] & 4) != 0) {
			_puzzleFlags[puzzleIndex] ^= 4;
			_puzzleFlags[i] |= kFlagMapTile;
		}
		// 137:0252
		if ((_puzzleFlags[i] & kFlagMapTile) != 0) {
			arr_i32_192c0[i] = _sunMapTilePic[0x53];
		}
		// 137:0294
	}
	// 137:02a2
	fillRect(0x14, 0x67, 0x156, 0x1b1, 2);
	for (int i = 1; i <= 0x51; i++) {
		_zbasic->picture(_screenGrid[i].left, _screenGrid[i].top, arr_i32_192c0[_sunMapTileID[i]]);
	}
	// 137:0336
	_stateFlags = kStateNull;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->InitCursor();
	sunMapCheckIfSolved();
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		while ((_stateFlags == kStateNull) && (!_activePuzzleSolved)) {
			getNextEvent(-1);
			if (_event.what == Graphics::MacToolbox::kMouseDown) {
				sunMapOnClick();
			}
		}
		// 137:038a
		if (_stateFlags == kStateUndo) {
			sunMapUndoMove();
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
	// 137:03ce
	if (!_activePuzzleSolved) {
		return;
	}
	// JMP 0x1618
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
	int16 gridX = 0;
	int16 gridY = 0;
	getGridFromMouse(gridX, gridY);
	if ((gridX < 1) || (gridX > _gridInfo.xCount) ||
		(gridY < 1) || (gridY > _gridInfo.yCount)) {
		return;
	}
	// 137:0466
	_menuDisabled = true;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->PenNormal();
	_toolbox->PenSize(3, 3);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	arr_i16_4758[0] = gridX;
	arr_i16_4758[1] = gridY;
	if (_puzzleFlags[_sunMapTileID[arr_i16_2f38[gridX*32 + gridY]]] & kFlagMapTile) {
		sunMapRevealPiece(gridX, gridY);
		return;
	}
	sunMapDragSelect(gridX, gridY);
	// 137:0506
	_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
	arr_i16_4758[0] -= gridX;
	arr_i16_4758[1] -= gridY;
	sunMapMoveSelected(gridX, gridY);
	_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
	sunMapDropSelected();
	sunMapCheckIfSolved();
	waitForMouseUp();
	_zbasic->menu(8, 3, 1, Common::U32String());
	_menuDisabled = false;
}

void FoolGame::sunMapDragSelect(int16 &gridX, int16 &gridY) {
	// 137:0598
	do {
		getNextEvent(4);
		getGridFromMouse(gridX, gridY);
		clampToGrid(gridX, gridY);
		if (gridX >= arr_i16_4758[0]) {
			arr_i16_4758[2] = arr_i16_4758[0];
			arr_i16_4758[3] = gridX;
		} else {
		// 137:061a
			arr_i16_4758[2] = gridX;
			arr_i16_4758[3] = arr_i16_4758[0];
		}
		// 137:064a
		if (gridY >= arr_i16_4758[1]) {
			arr_i16_4758[6] = arr_i16_4758[1];
			arr_i16_4758[7] = gridY;
		} else {
			// 137:069e
			arr_i16_4758[6] = gridY;
			arr_i16_4758[7] = arr_i16_4758[1];
		}
		// 137:06ce
		arr_i16_4758[11] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].top;
		arr_i16_4758[12] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].left;
		arr_i16_4758[13] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].bottom;
		arr_i16_4758[14] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].right;
		// 137:084c
		_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
		delay(2);
		_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
		delay(1);
	} while (_event.what != Graphics::MacToolbox::kMouseUp);
	// 137:087e
}

void FoolGame::sunMapMoveSelected(int16 &gridX, int16 &gridY) {
	// 137:0880
	do {
		getNextEvent(2);
		getGridFromMouse(gridX, gridY);
		clampToGrid(gridX, gridY);
		if (arr_i16_4758[0] >= 0) {
			arr_i16_4758[4] = gridX;
			arr_i16_4758[5] = gridX + arr_i16_4758[0];
		} else {
			// 137:0900
			arr_i16_4758[4] = gridX + arr_i16_4758[0];
			arr_i16_4758[5] = gridX;
		}
		// 137:093c
		if (arr_i16_4758[1] >= 0) {
			arr_i16_4758[8] = gridY;
			arr_i16_4758[9] = gridY + arr_i16_4758[1];
		} else {
			// 137:098e
			arr_i16_4758[8] = gridY + arr_i16_4758[1];
			arr_i16_4758[9] = gridY;
		}
		// 137:09ca
		if (arr_i16_4758[4] < 1) {
			arr_i16_4758[4] = 1;
			arr_i16_4758[5] = 1 + ABS(arr_i16_4758[0]);
		}
		// 137:0a2e
		if (arr_i16_4758[8] < 1) {
			arr_i16_4758[8] = 1;
			arr_i16_4758[9] = 1 + ABS(arr_i16_4758[1]);
		}
		// 137:0a92
		if (arr_i16_4758[5] > _gridInfo.xCount) {
			arr_i16_4758[4] = _gridInfo.xCount - ABS(arr_i16_4758[0]);
			arr_i16_4758[5] = _gridInfo.xCount;
		}
		// 137:0b20
		if (arr_i16_4758[9] > _gridInfo.yCount) {
			arr_i16_4758[8] = _gridInfo.yCount - ABS(arr_i16_4758[1]);
			arr_i16_4758[9] = _gridInfo.yCount;
		}
		// 137:0bae
		arr_rect_4776.top = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].top;
		arr_rect_4776.left = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].left;
		arr_rect_4776.bottom = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].bottom;
		arr_rect_4776.right = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].right;
		_toolbox->FrameRect(arr_rect_4776);
		delay(2);
		_toolbox->FrameRect(arr_rect_4776);
		delay(1);
	} while (_event.what != Graphics::MacToolbox::kMouseDown);
}

void FoolGame::sunMapUndoMove() {
	// 137:0d60
	_stateFlags = kStateNull;
	_zbasic->swapInt(arr_i16_4758[2], arr_i16_4758[4]);
	_zbasic->swapInt(arr_i16_4758[3], arr_i16_4758[5]);
	_zbasic->swapInt(arr_i16_4758[6], arr_i16_4758[8]);
	_zbasic->swapInt(arr_i16_4758[7], arr_i16_4758[9]);
	sunMapDropSelected();
}

void FoolGame::sunMapDropSelected() {
	// 137:0dde
	var_i16_1aa4 = 0x51;
	for (int j = arr_i16_4758[6]; j <= arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			var_i16_1aa4++;
			arr_i16_3738[var_i16_1aa4] = _sunMapTileID[arr_i16_2f38[i*32 + j]];
			// 137:0e5a
			_sunMapTileID[arr_i16_2f38[i*32 + j]] = 0;
		}
	}

	// 137:0ec2
	var_i16_1aa6 = var_i16_1aa4;
	var_i16_1aa4 = 0x51;
	for (int j = arr_i16_4758[8]; j <= arr_i16_4758[9]; j++) {
		for (int i = arr_i16_4758[4]; i <= arr_i16_4758[5]; i++) {
			var_i16_1aa4++;
			if (_sunMapTileID[arr_i16_2f38[i*32 + j]] > 0) {
				var_i16_1aa6++;
				arr_i16_3738[var_i16_1aa6] = _sunMapTileID[arr_i16_2f38[i*32 + j]];
			}
			// 137:0f7c
			_sunMapTileID[arr_i16_2f38[i*32 + j]] = arr_i16_3738[var_i16_1aa4];
			_zbasic->picture(
				_screenGrid[arr_i16_2f38[i*32 + j]].left,
				_screenGrid[arr_i16_2f38[i*32 + j]].top,
				arr_i32_192c0[_sunMapTileID[arr_i16_2f38[i*32 + j]]]
			);
		}
	}
	var_i16_1aa6 = var_i16_1aa4;
	for (int j = arr_i16_4758[6]; j <= arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			if (_sunMapTileID[arr_i16_2f38[i*32 + j]] == 0) {
				// 137:1114
				var_i16_1aa6++;
				_sunMapTileID[arr_i16_2f38[i*32 + j]] = arr_i16_3738[var_i16_1aa6];
				_zbasic->picture(
					_screenGrid[arr_i16_2f38[i*32 + j]].left,
					_screenGrid[arr_i16_2f38[i*32 + j]].top,
					arr_i32_192c0[_sunMapTileID[arr_i16_2f38[i*32 + j]]]
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
			var_i16_484 = arr_i16_4c7c[i];
			if ((_puzzleFlags[var_i16_484] & kFlagMapTile) || (_puzzleFlags[var_i16_484] == kFlagMenuDisabled)) {
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

void FoolGame::sunMapRevealPiece(int16 &gridX, int16 &gridY) {
	// 137:131a
	_puzzleFlags[_sunMapTileID[arr_i16_2f38[gridX*32 + gridY]]] ^= kFlagMapTile;
	arr_i32_192c0[_sunMapTileID[arr_i16_2f38[gridX*32 + gridY]]] = _sunMapTilePic[_sunMapTileID[arr_i16_2f38[gridX*32 + gridY]]];
	// 137:142c
	// unrolled loop
	arr_i16_4758[0] = _screenGrid[arr_i16_2f38[gridX*32 + gridY]].top;
	arr_i16_4758[1] = _screenGrid[arr_i16_2f38[gridX*32 + gridY]].left;
	arr_i16_4758[2] = _screenGrid[arr_i16_2f38[gridX*32 + gridY]].bottom;
	arr_i16_4758[3] = _screenGrid[arr_i16_2f38[gridX*32 + gridY]].right;
	// 137:1490
	for (int16 i = 0; i <= 0xf; i++) {
		arr_i16_4758[0] += i;
		arr_i16_4758[1] += i;
		arr_i16_4758[2] -= i;
		arr_i16_4758[3] -= i;
		Common::Rect temp;
		temp.top = arr_i16_4758[0];
		temp.left = arr_i16_4758[1];
		temp.bottom = arr_i16_4758[2];
		temp.right = arr_i16_4758[3];
		_toolbox->FrameRect(temp);
		delay(1);
	}
	// 137:1550
	_zbasic->picture(
		_screenGrid[arr_i16_2f38[gridX*32 + gridY]].left,
		_screenGrid[arr_i16_2f38[gridX*32 + gridY]].top,
		arr_i32_192c0[_sunMapTileID[arr_i16_2f38[gridX*32+gridY]]]
	);
	waitForMouseUp();
	sunMapCheckIfSolved();
	_menuDisabled = false;
}


} // End of namespace Fool
