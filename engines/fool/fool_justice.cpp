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

#define OFF(x) (_zstrOffset[kOffsetJustice] + (x))



// justice - lights on
void FoolGame::justiceRun() {
	// 142:0852
	if (_activePuzzleStatus < 0x63) {
		justiceSetup();
		if (!_activePuzzleSolved) {
			justiceStoreState();
			return;
		}
		// 142:0874
		justiceResetGrid();
		if (_puzzleCompletionStatus[0x34] > 3) {
			_activePuzzleStatus = 0x64;
			return;
		} else {
			// 142:08a2
			zoomRect(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x1a);
			_activePuzzleStatus = 0x63;
			// behold the 3rd key of thoth
			showBehold(0x3f, 1, _zbasic->str(OFF(0)));
		}
	}
	// 142:0906
	if (_activePuzzleStatus == 0x63) {
		_zbasic->menu(8, 0, 1, _zbasic->str(OFF(1))); // the 3rd key of thoth
		_zbasic->menu(8, 1, 1, _zbasic->str(OFF(2))); // return to scroll
		thothKey3rd();
		if (_activePuzzleSolved) {
			zoomRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xa2, 0xff, 0xac, 0x10e, 2, kPatXor, 0x19);
		}
	}
	// 142:098a
	if (_activePuzzleStatus >= 0x64) {
		justiceSetup();
		if (!_activePuzzleSolved) {
			justiceStoreState();
			return;
		}
		// 142:09b0
		justiceResetGrid();
		_activePuzzleStatus = 0x65;
	}
	// 142:09ba
	return;
}

void FoolGame::justiceSetup() {
	// 142:09be
	fetchPuzzleData();
	for (int16 i = 1; i <= 0x19; i++) {
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
	}
	// 142:09ea
	if (_puzzleCompletionStatus[0x34] < 4) {
		_zbasic->menu(8, 3, 1, _zbasic->str(OFF(3))); // a secret hides here
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(4))); // if twenty-five appear
	} else {
		// 142:0a3a
		_zbasic->menu(8, 3, 1, _zbasic->str(OFF(5))); // you will remain here
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(6))); // unless twenty-five appear
	}
	// 142:0a6a
	justiceZoom();
	int16 gridIndex = 0;
	for (int16 i = 0x24; i <= 0x11d; i += 0x32) {
		for (int16 j = 0x8c; j <= 0x185; j += 0x32) {
			gridIndex++;
			_toolbox->SetRect(
				_screenGrid[gridIndex],
				j,
				i,
				j + 0x32,
				i + 0x32
			);
		}
	}
	// 142:0adc
	justiceZoom();
	fillRect(0x1a, 0x82, 0x128, 0x190, 2);
	if (_activePuzzleBuffer.empty()) { // was: str(OFF(7))
		_activePuzzleBuffer = (_zbasic->space(0xc) + _zbasic->str(OFF(8)) + _zbasic->space(0xc)).encode(Common::kMacRoman); // 1
		_justiceFirstMove = 1;
	} else {
		// 142:0b4c
		_justiceFirstMove = _zbasic->castInt(_zbasic->leftStr(_activePuzzleBuffer, 1));
		_activePuzzleBuffer = _zbasic->rightStr(_activePuzzleBuffer, 0x19);
	}
	// 142:0b7c
	for (int16 i = 1; i <= 0x19; i++) {
		int16 drawData = _zbasic->castInt(_zbasic->midStr(_activePuzzleBuffer, i, 1));
		if ((drawData & 2) == 0) {
			_zbasic->indexRawSet(_zbasic->strRaw(OFF(9)), 1, (i + 0x19)); // bullet
		} else {
			// 142:0be0
			_zbasic->indexRawSet(_zbasic->strRaw(OFF(10)), 1, (i + 0x19)); // wadjet
		}
		// 142:0c06
		if ((drawData & 1) != 0) {
			justiceDrawBlock(i);
		} else {
			justiceRemoveBlock(i);
		}
	}
	// 142:0c32
	_activePuzzleSolved = false;
	_stateFlags = 0;
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 142:0c42
		while ((_stateFlags == 0) && (!_activePuzzleSolved)) {
			getNextEvent(-1);
			if (_event.what == 1) {
				justiceOnClick();
			}
		}
		// 142:0c76
		if (_stateFlags == kStateSaveGame) {
			justiceStoreState();
			saveGame();
		}
		// 142:0c88
	}
}

void FoolGame::justiceOnClick() {
	// 142:0cb2
	int16 gridX = (_event.where.x - 0x5a) / 0x32;
	int16 gridY = (_event.where.y + 0xe) / 0x32;
	if ((gridX < 0) || (gridY < 0) || (gridX > 5) || (gridY > 5)) {
		return;
	}
	// 142:0d26
	int16 gridIndex = (gridY - 1)*5 + gridX;
	if (arr_i16_3738[gridIndex] == 0) {
		return;
	}
	// 142:0d56
	_toolbox->InvertRoundRect(_screenGrid[gridIndex], 0x1e, 0x1e);
	if (_zbasic->index(1, gridIndex + 0x19) == _zbasic->str(OFF(11))) { // bullet
		_zbasic->indexSet(_zbasic->str(OFF(12)), 1, gridIndex + 0x19); // wadjet
	}
	// 142:0dc6
	if (gridIndex == 0xd) { // center button
		if (_justiceFirstMove == 0) {
			for (int16 i = 1; i <= 0x19; i++) {
				_zbasic->indexSet(_zbasic->str(OFF(13)), 1, i + 0x19); // bullet
				arr_i16_3738[i] = 0;
				justiceRemoveBlock(i);
			}
			_justiceFirstMove = 1;
			// redraw center tile
			justiceDrawBlock(0xd);
			return;
		}
		// 142:0e3e
		_justiceFirstMove = 0;
	}
	// 142:0e44
	Common::String buffer = _zbasic->indexRaw(1, gridIndex);
	int16 offset = 2;
	if (_zbasic->leftStr(buffer, 1) == _zbasic->str(OFF(14)).encode(Common::kMacRoman)) { // M
		gridIndex = _zbasic->decodeInt(_zbasic->midStr(buffer, 2, 2));
		justiceDrawBlock(gridIndex);
		offset = 5;
	}
	// 142:0eae
	int16 blockCount = _zbasic->decodeInt(_zbasic->midStr(buffer, offset, 2));
	for (int16 i = 1; i <= blockCount; i++) {
		offset += 2;
		gridIndex = _zbasic->decodeInt(_zbasic->midStr(buffer, offset, 2));
		// 142:0ef0
		if (arr_i16_3738[gridIndex] == 0) {
			justiceDrawBlock(gridIndex);
		} else {
			justiceRemoveBlock(gridIndex);
		}
	}
	// 142:0f20
	_activePuzzleSolved = true;
	for (int16 i = 1; i <= 0x19; i++) {
		if (arr_i16_3738[i] == 0) {
			_activePuzzleSolved = false;
		}
	}
	waitForMouseUp();
}

void FoolGame::justiceZoom() {
	// 142:0f58
	zoomRect(0x130, 0x76, 0x130, 0x76, 0x1a, 0x82, 0x128, 0x190, 1, kPatXor, 0x19);
}

void FoolGame::justiceDrawBlock(int16 gridIndex) {
	// 142:0f96
	arr_i16_3738[gridIndex] = 1;
	playTone(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	_toolbox->PenNormal();
	_toolbox->FillRoundRect(_screenGrid[gridIndex], 0x1e, 0x1e, _patterns[0]);
	_toolbox->FrameRoundRect(_screenGrid[gridIndex], 0x1e, 0x1e);
	_zbasic->text(kFontChicago, 0xc, 0, kSrcOr);
	Common::U32String label = _zbasic->index(1, gridIndex + 0x19);
	int16 width = _toolbox->StringWidth(label);
	// 142:1056
	_toolbox->MoveTo(
		_screenGrid[gridIndex].left + 0x19 - (width/2),
		_screenGrid[gridIndex].top + 0x1e
	);
	_toolbox->DrawString(label);
}

void FoolGame::justiceRemoveBlock(int16 gridIndex) {
	// 142:10bc
	arr_i16_3738[gridIndex] = 0;
	playTone(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	_toolbox->FillRect(_screenGrid[gridIndex], _patterns[_zbasic->rndInt(0x4d)+3]);
}

void FoolGame::justiceStoreState() {
	// 142:111e
	_activePuzzleBuffer = Common::String::format("%d", _justiceFirstMove);
	for (int16 i = 1; i <= 0x19; i++) {
		int16 data = arr_i16_3738[i];
		if (_zbasic->index(1, i + 0x19) == _zbasic->str(OFF(15))) { // wadjet
			data |= 2;
		}
		_activePuzzleBuffer += Common::String::format("%d", data);
	}
	// 142:11fc
}

void FoolGame::justiceResetGrid() {
	// 142:11fe
	delay(0x28);
	_activePuzzleBuffer.clear();
	for (int16 i = 1; i <= 0x19; i++) {
		playTone(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
		_toolbox->FillRect(_screenGrid[i], _patterns[2]);
	}
	// 142:126e
	zoomRect(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x1a);
}

}
