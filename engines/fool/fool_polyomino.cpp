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

#define OFF(x) (_zstrOffset[kOffsetPolyomino] + (x))



// polyomino puzzle
void FoolGame::polyominoRun() {
	// 133:0004
	fetchPuzzleData();
	var_i16_c00 = 1;
	for (int i = 0; i <= 0x11; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 133:003c
	Common::U32String hint = puzzlesReadString().decode(Common::kMacRoman);
	// "the completed puzzle will reveal..."
	hint = _zbasic->str(OFF(0)) + hint + _zbasic->str(OFF(1));
	_zbasic->menu(8, 0xf, 1, hint);
	int16 gridIndex = 0;
	int16 gridY = arr_i16_1eb8[8];
	do {
		// 133:009e
		int16 gridX = arr_i16_1eb8[10];
		do {
			// 133:00b2
			gridIndex++;
			_toolbox->SetRect(
				_screenGrid[gridIndex],
				gridX,
				gridY,
				gridX + arr_i16_1eb8[13],
				gridY + arr_i16_1eb8[12]
			);
			// 133:0112
		} while (_zbasic->incrAndCheck(
			gridX,
			arr_i16_1eb8[11],
			arr_i16_1eb8[6]
		));
		// 133:0142
	} while (_zbasic->incrAndCheck(
		gridY,
		arr_i16_1eb8[9],
		arr_i16_1eb8[7]
	));
	// 133:0172
	var_i16_484 = 0;
	for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
			// 133:0184
			var_i16_484++;
			// collision map: 2D grid where element is occupying polyomino ID
			arr_i16_2f38[i*32 + j] = 0;

			arr_i16_3b38[i*32 + j] = var_i16_484;
		}
	}
	// 133:01fa
	for (int i = 0x12; i <= 0x19; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 133:0228
	_toolbox->SetPort(var_i32_f24);
	_zbasic->text(kFontPuzzle, 0x18, Graphics::kMacFontRegular, kSrcOr);

	// for each polyomino
	for (int j = 1; j <= arr_i16_1eb8[0x10]; j++) {
		// read number of coordinate parts
		var_i16_103a = puzzlesReadShort();
		for (int i = 0; i <= 4; i++) {
			arr_i16_47d8[j*8 + i] = puzzlesReadShort();
		}
		// 133:0284
		for (int i = 5; i <= 6; i++) {
			arr_i16_47d8[j*8 + i] = arr_i16_47d8[j*8 + i - 2];
		}
		if (debugLevelSet(5)) {
			debug(5,"FoolGame::polyominoRun: adding polyomino %d:", j);
			for (int i = 0; i < 4; i++) {
				uint16 data = (arr_i16_47d8[j*8] >> (i*4)) & 0xf;
				debug(5, "%s%s%s%s",
					data & 0x1 ? "#" : " ",
					data & 0x2 ? "#" : " ",
					data & 0x4 ? "#" : " ",
					data & 0x8 ? "#" : " "
				);
			}
			debug(5, "%d, %d, start position %d, %d",
				arr_i16_47d8[j*8 + 1],
				arr_i16_47d8[j*8 + 2],
				arr_i16_47d8[j*8 + 3],
				arr_i16_47d8[j*8 + 4]
			);
		}
		// 133:02de
		if (!_activePuzzleBuffer.empty()) { // was: str(OFF(2))
			if (j == 1)  {
				var_i16_1aa8 = 1;
			}
			for (int i = 3; i <= 4; i++) {
				arr_i16_47d8[j*8 + i] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_1aa8, 2));
				var_i16_1aa8 += 2;
			}
		}
		// 133:0356
		// read coordinates. these are unsigned bytes in pixel units
		for (int i = 0; i <= var_i16_103a; i++) {
			arr_i16_4338[i] = puzzlesReadByte();
		}

		// 133:0388
		// create polyomino polygons from coordinates
		PolyHandle poly = _toolbox->OpenPoly(); // var_poly_1aac
		_toolbox->MoveTo(arr_i16_4338[1], arr_i16_4338[0]);
		for (int i = 2; i <= var_i16_103a; i += 2) {
			_toolbox->LineTo(arr_i16_4338[i+1], arr_i16_4338[i]);
		}
		_toolbox->ClosePoly();

		// 133:0408
		// load in letter placement information
		var_i16_103a = puzzlesReadShort();
		if (var_i16_103a >= 3) {
			for (int i = 1; i <= var_i16_103a; i++) {
				arr_i16_3738[i] = puzzlesReadByte();
			}
		}
		// 133:044a
		arr_i16_1eb8[0x1a] = puzzlesReadShort(); // polygon fill pattern index
		arr_i16_1eb8[0x1b] = puzzlesReadShort(); // text source mode
		_toolbox->PenNormal();

		// 133:0476: JSR - "PICTURE_ON"
		// picture handle 0: XOR text + polyomino
		PicHandle handle = _toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		_toolbox->PenMode(kPatXor);
		if (var_i16_103a >= 3) {
			_toolbox->TextMode(kSrcXor);
			for (int i = 1; i <= var_i16_103a; i += 3) {
				_toolbox->MoveTo(arr_i16_3738[i], arr_i16_3738[i+1]);
				var_str_384 = _zbasic->chr(arr_i16_3738[i+2]);
				_toolbox->DrawString(var_str_384);
			}
		}
		// 133:050a
		_toolbox->InvertPoly(poly);
		_toolbox->ClosePicture();
		_polyominoPics[j*4] = handle;
		_toolbox->PenNormal();

		// 133:0530: JSR - "PICTURE_ON"
		// picture handle 1:
		handle = _toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		_toolbox->FillPoly(poly, _patterns[arr_i16_1eb8[0x1a]]);
		_toolbox->PenPat(_patterns[arr_i16_1eb8[0xf]]);
		_toolbox->FramePoly(poly);
		// 133:0582
		if (var_i16_103a >= 3) {
			_toolbox->TextMode((SourceMode)arr_i16_1eb8[0x1b]);
			for (int i = 1; i <= var_i16_103a; i += 3) {
				_toolbox->MoveTo(arr_i16_3738[i], arr_i16_3738[i+1]);
				var_str_384 = _zbasic->chr(arr_i16_3738[i+2]);
				_toolbox->DrawString(var_str_384);
			}
		}
		// 133:061a
		_toolbox->FramePoly(poly);
		_toolbox->ClosePicture();
		// 133:0622
		_polyominoPics[j*4 + 1] = handle;
		_toolbox->PenNormal();

		// 133:0642: JSR - "PICTURE_ON"
		// picture handle 2: draw the shape without letters
		handle = _toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		_toolbox->PenPat(_patterns[arr_i16_1eb8[0xf]]);
		_toolbox->FramePoly(poly);
		_toolbox->PaintPoly(poly);
		_toolbox->ClosePicture();
		_polyominoPics[j*4 + 2] = handle;

		// clear polygon data
		_toolbox->KillPoly(poly);
		// 133:0698
	}
	// 133:06b4
	_toolbox->SetPort(var_i32_0);
	var_i16_484 = 0;
	for (int j = arr_i16_1eb8[0x16]; j <= arr_i16_1eb8[0x17]; j++) {
		for (int i = arr_i16_1eb8[0x18]; i <= arr_i16_1eb8[0x19]; i++) {
			var_i16_484++;
			arr_i16_4338[var_i16_484] = puzzlesReadByte();
		}
	}
	// 133:073c
	sub_128_bde(1, arr_i16_1eb8[0xf], 0, 1, 1, 1);
	var_i16_1ab0 = 0;
	for (var_i16_7cc = 1; var_i16_7cc <= arr_i16_1eb8[0x10]; var_i16_7cc++) {
		polyominoMove(
			arr_i16_47d8[var_i16_7cc*8 + 3],
			arr_i16_47d8[var_i16_7cc*8 + 4]
		);
	}
	var_i16_1ab0 = 1;
	polyominoDrawFrame();
	polyominoCheckIfSolved();
	_stateFlags = kStateNull;
	do {
		do {
			getNextEvent(-1);
			if (_event.what == 1) {
				polyominoOnClick();
			}
			// 133:0800
		} while ((_stateFlags == kStateNull) && (!_activePuzzleSolved));
		// 133:081c
		if (_stateFlags == kStateUndo) {
			polyominoReset();
		}
		if (_stateFlags == kStateSaveGame) {
			polyominoStoreState();
			saveGame();
		}
	} while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved));
	// 133:0864
	if (_activePuzzleSolved) {
		polyominoSuccess();
	}
	polyominoStoreState();

	//133:0878: JMP - [0x1502]
	// exiting game, delete picture handles
	for (int j = 1; j <= arr_i16_1eb8[0x10]; j++) {
		for (int i = 0; i <= 2; i++) {
			_toolbox->KillPicture(_polyominoPics[j*4 + i]);
		}
	}
}

void FoolGame::polyominoOnClick() {
	// 133:087c
	getGridFromMouse(var_i16_68a, var_i16_68c);
	debug(5, "FoolGame::polyominoOnClick: grid pos: (%d, %d)", var_i16_68a, var_i16_68c);
	if ((var_i16_68a < 1) || (var_i16_68a > arr_i16_1eb8[0]) || (var_i16_68c < 1) || (var_i16_68c > arr_i16_1eb8[1])) {
		waitForMouseUp();
		return;
	}
	// 133:0904
	// get polyomino from collision map
	var_i16_7cc = arr_i16_2f38[var_i16_68a*32 + var_i16_68c];
	debug(5, "FoolGame::polyominoOnClick: selected polyomino: %d", var_i16_7cc);
	// if no polyomino, ignore
	if (arr_i16_2f38[var_i16_68a*32 + var_i16_68c] == 0) {
		waitForMouseUp();
		return;
	}
	// 133:0950
	// if polyomino is part of the fixed list, act as if it were an invalid move
	if (arr_i16_2f38[var_i16_68a*32 + var_i16_68c] <= arr_i16_1eb8[0x11]) {
		polyominoOnClickFixed();
		return;
	}
	// 133:0990
	var_i16_106a = 0;
	int16 polyGridPosX = arr_i16_47d8[var_i16_7cc*8 + 3];
	int16 polyGridPosY = arr_i16_47d8[var_i16_7cc*8 + 4];
	debug(5, "FoolGame::polyominoOnClick: polyGridPosX: %d, polyGridPosY: %d", polyGridPosX, polyGridPosY);

	for (int j = polyGridPosY; j <= polyGridPosY + 3; j++) {
		// 133:09dc
		for (int i = polyGridPosX; i <= polyGridPosX + 3; i++) {
			if (arr_i16_47d8[var_i16_7cc*8] & _bitLUT[var_i16_106a]) {
				// erase grabbed polyomino from collision map
				arr_i16_2f38[i*32 + j] = 0;
			}
			// 133:0a36
			var_i16_106a++;
		}
	}
	// 133:0a60
	var_i16_1ab2 = polyGridPosX - var_i16_68a;
	var_i16_1ab4 = polyGridPosY - var_i16_68c;
	_polyominoPosX = _screenGrid[arr_i16_3b38[polyGridPosX*32 + polyGridPosY]].left;
	_polyominoPosY = _screenGrid[arr_i16_3b38[polyGridPosX*32 + polyGridPosY]].top;
	debug(5, "FoolGame::polyominoOnClick: Unblit solid and blit ghost (%d, %d)", _polyominoPosX, _polyominoPosY);
	_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4 + 2]);
	// 133:0b1c
	polyominoDrawFrame();
	toggleMouseCursor(false);
	var_i16_1ab6 = _polyominoPosX;
	var_i16_1ab8 = _polyominoPosY;
	_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4]);

	// 133:0b5a: JMP - [0xde0]
	while (_event.what != 2) {
		// 133:0b5e
		// 133:0d3a
		while ((var_i16_1ab6 == _polyominoPosX) && (var_i16_1ab8 == _polyominoPosY) && (_event.what != 2)) {
			// 133:0b62
			getNextEvent(4);

			var_i16_68a = var_i16_1ab2 + ((_event.where.x - arr_i16_1eb8[4]) / arr_i16_1eb8[6]);
			var_i16_68c = var_i16_1ab4 + ((_event.where.y - arr_i16_1eb8[5]) / arr_i16_1eb8[7]);

			if (var_i16_68a < 1) {
				var_i16_68a = 1;
			}
			// 133:0bf8
			if (var_i16_68a > (0x12 - arr_i16_47d8[var_i16_7cc*8 + 1])) {
				var_i16_68a = 0x12 - arr_i16_47d8[var_i16_7cc*8 + 1];
			}
			// 133:0c58
			if (var_i16_68c < 1) {
				var_i16_68c = 1;
			}
			// 133:0c68
			if (var_i16_68c > (0xb - arr_i16_47d8[var_i16_7cc*8 + 2])) {
				var_i16_68c = 0xb - arr_i16_47d8[var_i16_7cc*8 + 2];
			}
			// 133:0cc8
			var_i16_1ab6 = _screenGrid[arr_i16_3b38[var_i16_68a*32 + var_i16_68c]].left;
			var_i16_1ab8 = _screenGrid[arr_i16_3b38[var_i16_68a*32 + var_i16_68c]].top;
			//debug(5, "FoolGame::polyominoOnClick: Move to (%d, %d) -> (%d, %d)", var_i16_68a, var_i16_68c, var_i16_1ab6, var_i16_1ab8);

		}
		// 133:0d78
		if (_event.what != 2) {
			debug(5, "FoolGame::polyominoOnClick: Unblit ghost (%d, %d)", _polyominoPosX, _polyominoPosY);
			// XOR out original position
			_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4]);
			// drag to cursor position
			_polyominoPosX = var_i16_1ab6;
			_polyominoPosY = var_i16_1ab8;
			// XOR out new position
			debug(5, "FoolGame::polyominoOnClick: Blit ghost (%d, %d)", _polyominoPosX, _polyominoPosY);
			_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4]);
		}
		// 133:0de0

	}
	// 133:0dea
	var_i16_106a = 0;
	var_i16_1a9c = 0;
	// store polyomino as bitfield.
	for (int j = var_i16_68c; j <= var_i16_68c + 3; j++) {
		for (int i = var_i16_68a; i <= var_i16_68a + 3; i++) {
			if (arr_i16_2f38[i*32 + j] > 0) {
				var_i16_1a9c |= _bitLUT[var_i16_106a];
			}
			// 133:0e44
			var_i16_106a++;
		}
	// 133:0e5a
	}
	// 133:0e6c
	if ((var_i16_1a9c & arr_i16_47d8[var_i16_7cc*8]) != 0) {
		polyominoCancelMove();
	} else {
		polyominoMove(var_i16_68a, var_i16_68c);
	}
	// 133:0ea2
	polyominoDrawFrame();
	polyominoCheckIfSolved();
	toggleMouseCursor(true);
}

void FoolGame::polyominoCancelMove() {
	// 133:0eb2
	playTone(0x14, 0x64, 0);

	polyominoMove(
		arr_i16_47d8[var_i16_7cc*8 + 3],
		arr_i16_47d8[var_i16_7cc*8 + 4]
	);
}

void FoolGame::polyominoMove(int16 x, int16 y) {
	// 133:0f04
	var_i16_106a = 0;
	// polyominos are max 4x4
	// add the polyomino to the collision map
	for (int j = y; j <= y + 3; j++) {
		for (int i = x; i <= x + 3; i++) {
			if ((arr_i16_47d8[var_i16_7cc*8] & _bitLUT[var_i16_106a]) != 0) {
				arr_i16_2f38[i*32 + j] = var_i16_7cc;
			}
			// 133:0f6c
			var_i16_106a++;
		}
	}
	// 133:0f96
	// set the polyomino position
	arr_i16_47d8[var_i16_7cc*8 + 3] = x;
	arr_i16_47d8[var_i16_7cc*8 + 4] = y;

	// 133:0fce
	if (var_i16_1ab0 != 0) {
		debug(5, "FoolGame::polyominoMove: Blit ghost (%d, %d)", _polyominoPosX, _polyominoPosY);
		_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4]);
	}
	// 133:1002
	_polyominoPosX = _screenGrid[arr_i16_3b38[x*32 + y]].left;
	_polyominoPosY = _screenGrid[arr_i16_3b38[x*32 + y]].top;
	debug(5, "FoolGame::polyominoMove: Blit solid (%d, %d)", _polyominoPosX, _polyominoPosY);
	_zbasic->picture(_polyominoPosX, _polyominoPosY, _polyominoPics[var_i16_7cc*4 + 1]);

}

void FoolGame::polyominoOnClickFixed() {
	// 133:10a0
	int16 posX = _screenGrid[arr_i16_3b38[arr_i16_47d8[var_i16_7cc*8 + 3]*32 + arr_i16_47d8[var_i16_7cc*8 + 4]]].left;
	int16 posY = _screenGrid[arr_i16_3b38[arr_i16_47d8[var_i16_7cc*8 + 3]*32 + arr_i16_47d8[var_i16_7cc*8 + 4]]].top;
	// play bass tone
	playTone(0x14, 0x64, 0);
	// why do this???
	// best guess: flashing the shape a few times before knocking it back
	// 133:1188
	debug(5, "FoolGame::polyominoOnClickFixed: Draw pic 0 (%d, %d)", posX, posY);
	for (int i = 0; i <= 9; i++) {
		_zbasic->picture(posX, posY, _polyominoPics[var_i16_7cc*4]);
		_toolbox->Delay(0);
	}
	// 133:11c2
	polyominoDrawFrame();
	waitForMouseUp();
}

void FoolGame::polyominoReset() {
	// 133:11cc
	// zero out collision grid
	for (int j = 1; j <= arr_i16_1eb8[1] + 1; j++) {
		for (int i = 1; i <= arr_i16_1eb8[0] + 1; i++) {
			arr_i16_2f38[i*32 + j] = 0;
		}
	}
	// 133:122e
	fillRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, arr_i16_1eb8[0xf]);
	var_i16_1ab0 = 0;
	for (var_i16_7cc = 1; var_i16_7cc <= arr_i16_1eb8[0x10]; var_i16_7cc++) {
		polyominoMove(
			arr_i16_47d8[var_i16_7cc*8 + 5],
			arr_i16_47d8[var_i16_7cc*8 + 6]
		);
	}
	// 133:12c2
	var_i16_1ab0 = 1;
	polyominoDrawFrame();
	_stateFlags = kStateNull;
}

void FoolGame::polyominoDrawFrame() {
	// 133:12d4
	_toolbox->PenNormal();
	_toolbox->PenSize(3, 3);
	Common::Rect temp;
	temp.top = arr_i16_1eb8[0x12];
	temp.left = arr_i16_1eb8[0x13];
	temp.bottom = arr_i16_1eb8[0x14];
	temp.right = arr_i16_1eb8[0x15];
	_toolbox->FrameRect(temp);
	_toolbox->PenNormal();
}

void FoolGame::polyominoCheckIfSolved() {
	// 133:12f2
	_activePuzzleSolved = true;
	int16 index = 0;
	for (int j = arr_i16_1eb8[0x16]; j <= arr_i16_1eb8[0x17]; j++) {
		for (int i = arr_i16_1eb8[0x18]; i <= arr_i16_1eb8[0x19]; i++) {
			index++;
			if (arr_i16_2f38[i*32 + j] != arr_i16_4338[index]) {
				// 133:1368
				_activePuzzleSolved = false;
				j = arr_i16_1eb8[0x17];
				i = arr_i16_1eb8[0x19];
			}
			// 133:1396
		}
	}
	// 133:13ce
	if (_activePuzzleSolved) {
		_stateFlags = kStateReturn;
	}
}

void FoolGame::polyominoSuccess() {
	// 133:13e2
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	menuClickMessage();
	waitForMouseUp();
	flashRect(arr_i16_1eb8[0x12], arr_i16_1eb8[0x13], arr_i16_1eb8[0x14], arr_i16_1eb8[0x15], 0x96);
}

void FoolGame::polyominoStoreState() {
	// 133:1452
	_activePuzzleBuffer.clear(); // was: str(OFF(3))
	for (int i = 1; i <= arr_i16_1eb8[0x10]; i++) {
		Common::String val1 = _zbasic->encodeInt(arr_i16_47d8[i*8 + 3]);
		Common::String val2 = _zbasic->encodeInt(arr_i16_47d8[i*8 + 4]);
		_activePuzzleBuffer += val1 + val2;
	}
}


} // End of namespace Fool
