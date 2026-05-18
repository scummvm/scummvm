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

;
extern Toolbox *g_toolbox;

// polyomino puzzle
void FoolGame::polyominoRun() {
	// 133:0004
	this->sub_128_271a();
	this->var_i16_c00 = 1;
	for (int i = 0; i <= 0x11; i++) {
		this->arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 133:003c
	this->var_str_384 = puzzlesReadString().decode(Common::kMacRoman);
	// "the completed puzzle will reveal..."
	this->var_str_384 = _zbasic->str(209) + this->var_str_384 + _zbasic->str(210);
	_zbasic->menu(8, 0xf, 1, this->var_str_384);
	this->var_i16_484 = 0;
	this->var_i16_68c = this->arr_i16_1eb8[8];
	do {
		// 133:009e
		this->var_i16_68a = this->arr_i16_1eb8[10];
		do {
			// 133:00b2
			this->var_i16_484 += 1;
			g_toolbox->SetRect(
				_screenGrid[this->var_i16_484],
				this->var_i16_68a,
				this->var_i16_68c,
				this->var_i16_68a + this->arr_i16_1eb8[13],
				this->var_i16_68c + this->arr_i16_1eb8[12]
			);
			// 133:0112
		} while (_zbasic->incrAndCheck(
			this->var_i16_68a,
			this->arr_i16_1eb8[11],
			this->arr_i16_1eb8[6]
		));
		// 133:0142
	} while (_zbasic->incrAndCheck(
		this->var_i16_68c,
		this->arr_i16_1eb8[9],
		this->arr_i16_1eb8[7]
	));
	// 133:0172
	this->var_i16_484 = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			// 133:0184
			this->var_i16_484++;
			// collision map: 2D grid where element is occupying polyomino ID
			this->arr_i16_2f38[i*32 + j] = 0;

			this->arr_i16_3b38[i*32 + j] = this->var_i16_484;
		}
	}
	// 133:01fa
	for (int i = 0x12; i <= 0x19; i++) {
		this->arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 133:0228
	g_toolbox->SetPort(this->var_i32_f24);
	_zbasic->text(0xfb, 0x18, Graphics::kMacFontRegular, kSrcOr);

	// for each polyomino
	for (int j = 1; j <= this->arr_i16_1eb8[0x10]; j++) {
		// read number of coordinate parts
		this->var_i16_103a = puzzlesReadShort();
		for (int i = 0; i <= 4; i++) {
			this->arr_i16_47d8[j*8 + i] = puzzlesReadShort();
		}
		// 133:0284
		for (int i = 5; i <= 6; i++) {
			this->arr_i16_47d8[j*8 + i] = this->arr_i16_47d8[j*8 + i - 2];
		}
		debug(5,"FoolGame::polyominoRun: adding polyomino %d:", j);
		for (int i = 0; i < 4; i++) {
			uint16 data =(this->arr_i16_47d8[j*8] >> (i*4)) & 0xf;
			debug(5, "%s%s%s%s",
				data & 0x1 ? "#" : " ",
				data & 0x2 ? "#" : " ",
				data & 0x4 ? "#" : " ",
				data & 0x8 ? "#" : " "
			);
		}
		debug(5, "%d, %d, start position %d, %d",
				this->arr_i16_47d8[j*8 + 1],
				this->arr_i16_47d8[j*8 + 2],
				this->arr_i16_47d8[j*8 + 3],
				this->arr_i16_47d8[j*8 + 4]
				);
		// 133:02de
		if (!_activePuzzleBuffer.empty()) { // was: str(211)
			if (j == 1)  {
				this->var_i16_1aa8 = 1;
			}
			for (int i = 3; i <= 4; i++) {
				this->arr_i16_47d8[j*8 + i] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, this->var_i16_1aa8, 2));
				this->var_i16_1aa8 += 2;
			}
		}
		// 133:0356
		// read coordinates. these are unsigned bytes in pixel units
		for (int i = 0; i <= this->var_i16_103a; i++) {
			this->arr_i16_4338[i] = puzzlesReadByte();
		}

		// 133:0388
		// create polyomino polygons from coordinates
		this->var_poly_1aac = g_toolbox->OpenPoly();
		g_toolbox->MoveTo(this->arr_i16_4338[1], this->arr_i16_4338[0]);
		for (int i = 2; i <= this->var_i16_103a; i += 2) {
			g_toolbox->LineTo(this->arr_i16_4338[i+1], this->arr_i16_4338[i]);
		}
		g_toolbox->ClosePoly();

		// 133:0408
		// load in letter placement information
		this->var_i16_103a = puzzlesReadShort();
		if (this->var_i16_103a >= 3) {
			for (int i = 1; i <= this->var_i16_103a; i++) {
				this->arr_i16_3738[i] = puzzlesReadByte();
			}
		}
		// 133:044a
		this->arr_i16_1eb8[0x1a] = puzzlesReadShort(); // polygon fill pattern index
		this->arr_i16_1eb8[0x1b] = puzzlesReadShort(); // text source mode
		g_toolbox->PenNormal();

		// 133:0476: JSR - "PICTURE_ON"
		// picture handle 0: XOR text + polyomino
		PicHandle handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->PenMode(kPatXor);
		if (this->var_i16_103a >= 3) {
			g_toolbox->TextMode(kSrcXor);
			for (int i = 1; i <= this->var_i16_103a; i += 3) {
				g_toolbox->MoveTo(this->arr_i16_3738[i], this->arr_i16_3738[i+1]);
				this->var_str_384 = _zbasic->chr(this->arr_i16_3738[i+2]);
				g_toolbox->DrawString(this->var_str_384);
			}
		}
		// 133:050a
		g_toolbox->InvertPoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		this->arr_pic_49d8[j*4] = handle;
		g_toolbox->PenNormal();

		// 133:0530: JSR - "PICTURE_ON"
		// picture handle 1:
		handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->FillPoly(this->var_poly_1aac, _patterns[this->arr_i16_1eb8[0x1a]]);
		g_toolbox->PenPat(_patterns[this->arr_i16_1eb8[0xf]]);
		g_toolbox->FramePoly(this->var_poly_1aac);
		// 133:0582
		if (this->var_i16_103a >= 3) {
			g_toolbox->TextMode((SourceMode)this->arr_i16_1eb8[0x1b]);
			for (int i = 1; i <= this->var_i16_103a; i += 3) {
				g_toolbox->MoveTo(this->arr_i16_3738[i], this->arr_i16_3738[i+1]);
				this->var_str_384 = _zbasic->chr(this->arr_i16_3738[i+2]);
				g_toolbox->DrawString(this->var_str_384);
			}
		}
		// 133:061a
		g_toolbox->FramePoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		// 133:0622
		this->arr_pic_49d8[j*4 + 1] = handle;
		g_toolbox->PenNormal();

		// 133:0642: JSR - "PICTURE_ON"
		// picture handle 2: draw the shape without letters
		handle = g_toolbox->OpenPicture(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
		g_toolbox->PenPat(_patterns[this->arr_i16_1eb8[0xf]]);
		g_toolbox->FramePoly(this->var_poly_1aac);
		g_toolbox->PaintPoly(this->var_poly_1aac);
		g_toolbox->ClosePicture();
		this->arr_pic_49d8[j*4 + 2] = handle;

		// clear polygon data
		g_toolbox->KillPoly(this->var_poly_1aac);
		// 133:0698
	}
	// 133:06b4
	g_toolbox->SetPort(this->var_i32_0);
	this->var_i16_484 = 0;
	for (int j = this->arr_i16_1eb8[0x16]; j <= this->arr_i16_1eb8[0x17]; j++) {
		for (int i = this->arr_i16_1eb8[0x18]; i <= this->arr_i16_1eb8[0x19]; i++) {
			this->var_i16_484++;
			this->arr_i16_4338[this->var_i16_484] = puzzlesReadByte();
		}
	}
	// 133:073c
	this->sub_128_bde(1, this->arr_i16_1eb8[0xf], 0, 1, 1, 1);
	this->var_i16_1ab0 = 0;
	for (this->var_i16_7cc = 1; this->var_i16_7cc <= this->arr_i16_1eb8[0x10]; this->var_i16_7cc++) {
		this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
		this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];
		this->polyominoMove();
	}
	this->var_i16_1ab0 = 1;
	this->polyominoDrawFrame();
	this->polyominoCheckIfSolved();
	_stateFlags = kStateNull;
	do {
		do {
			this->getNextEvent(-1);
			if (_event.what == 1) {
				this->polyominoOnClick();
			}
			// 133:0800
		} while ((_stateFlags == kStateNull) && (!_activePuzzleSolved));
		// 133:081c
		if (_stateFlags == kStateUndo) {
			this->polyominoReset();
		}
		if (_stateFlags == kStateSaveGame) {
			this->polyominoStoreState();
			this->saveGame();
		}
	} while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved));
	// 133:0864
	if (_activePuzzleSolved) {
		this->polyominoSuccess();
	}
	this->polyominoStoreState();

	//133:0878: JMP - [0x1502]
	// exiting game, delete picture handles
	for (int j = 1; j <= this->arr_i16_1eb8[0x10]; j++) {
		for (int i = 0; i <= 2; i++) {
			g_toolbox->KillPicture(this->arr_pic_49d8[j*4 + i]);
		}
	}
}

void FoolGame::polyominoOnClick() {
	// 133:087c
	this->sub_128_2be(this->var_i16_68a, this->var_i16_68c);
	debug(5, "FoolGame::polyominoOnClick: grid pos: (%d, %d)", this->var_i16_68a, this->var_i16_68c);
	if ((this->var_i16_68a < 1) || (this->var_i16_68a > this->arr_i16_1eb8[0]) || (this->var_i16_68c < 1) || (this->var_i16_68c > this->arr_i16_1eb8[1])) {
		this->sub_128_6186();
		return;
	}
	// 133:0904
	// get polyomino from collision map
	this->var_i16_7cc = this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c];
	debug(5, "FoolGame::polyominoOnClick: selected polyomino: %d", this->var_i16_7cc);
	// if no polyomino, ignore
	if (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] == 0) {
		this->sub_128_6186();
		return;
	}
	// 133:0950
	// if polyomino is part of the fixed list, act as if it were an invalid move
	if (this->arr_i16_2f38[this->var_i16_68a*32 + this->var_i16_68c] <= this->arr_i16_1eb8[0x11]) {
		this->polyominoOnClickFixed();
		return;
	}
	// 133:0990
	this->var_i16_106a = 0;
	this->var_i16_1a96 = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
	this->var_i16_1a98 = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];
	debug(5, "FoolGame::polyominoOnClick: 1a96: %d, 1a98: %d", this->var_i16_1a96, this->var_i16_1a98);

	for (int j = this->var_i16_1a98; j <= this->var_i16_1a98 + 3; j++) {
		// 133:09dc
		for (int i = this->var_i16_1a96; i <= this->var_i16_1a96 + 3; i++) {
			if (this->arr_i16_47d8[this->var_i16_7cc*8] & _bitLUT[this->var_i16_106a]) {
				// erase grabbed polyomino from collision map
				this->arr_i16_2f38[i*32 + j] = 0;
			}
			// 133:0a36
			this->var_i16_106a++;
		}
	}
	// 133:0a60
	this->var_i16_1ab2 = this->var_i16_1a96 - this->var_i16_68a;
	this->var_i16_1ab4 = this->var_i16_1a98 - this->var_i16_68c;
	this->var_i16_1062 = _screenGrid[this->arr_i16_3b38[this->var_i16_1a96*32 + this->var_i16_1a98]].left;
	this->var_i16_1064 = _screenGrid[this->arr_i16_3b38[this->var_i16_1a96*32 + this->var_i16_1a98]].top;
	debug(5, "FoolGame::polyominoOnClick: Draw pic 2 (%d, %d)", this->var_i16_1062, this->var_i16_1064);
	_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4 + 2]);
	// 133:0b1c
	this->polyominoDrawFrame();
	this->sub_128_4da(0);
	this->var_i16_1ab6 = this->var_i16_1062;
	this->var_i16_1ab8 = this->var_i16_1064;
	_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);

	// 133:0b5a: JMP - [0xde0]
	while (_event.what != 2) {
		// 133:0b5e
		// 133:0d3a
		while ((this->var_i16_1ab6 == this->var_i16_1062) && (this->var_i16_1ab8 == this->var_i16_1064) && (_event.what != 2)) {
			// 133:0b62
			this->getNextEvent(4);

			this->var_i16_68a = this->var_i16_1ab2 + ((_event.where.x - this->arr_i16_1eb8[4]) / this->arr_i16_1eb8[6]);
			this->var_i16_68c = this->var_i16_1ab4 + ((_event.where.y - this->arr_i16_1eb8[5]) / this->arr_i16_1eb8[7]);

			if (this->var_i16_68a < 1) {
				this->var_i16_68a = 1;
			}
			// 133:0bf8
			if (this->var_i16_68a > (0x12 - this->arr_i16_47d8[this->var_i16_7cc*8 + 1])) {
				this->var_i16_68a = 0x12 - this->arr_i16_47d8[this->var_i16_7cc*8 + 1];
			}
			// 133:0c58
			if (this->var_i16_68c < 1) {
				this->var_i16_68c = 1;
			}
			// 133:0c68
			if (this->var_i16_68c > (0xb - this->arr_i16_47d8[this->var_i16_7cc*8 + 2])) {
				this->var_i16_68c = 0xb - this->arr_i16_47d8[this->var_i16_7cc*8 + 2];
			}
			// 133:0cc8
			this->var_i16_1ab6 = _screenGrid[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].left;
			this->var_i16_1ab8 = _screenGrid[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].top;
			//debug(5, "FoolGame::polyominoOnClick: Move to (%d, %d) -> (%d, %d)", this->var_i16_68a, this->var_i16_68c, this->var_i16_1ab6, this->var_i16_1ab8);

		}
		// 133:0d78
		if (_event.what != 2) {
			debug(5, "FoolGame::polyominoOnClick: Draw pic 0 (%d, %d)", this->var_i16_1062, this->var_i16_1064);
			// XOR out original position
			_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
			// drag to cursor position
			this->var_i16_1062 = this->var_i16_1ab6;
			this->var_i16_1064 = this->var_i16_1ab8;
			// XOR out new position
			debug(5, "FoolGame::polyominoOnClick: Draw pic 0 (%d, %d)", this->var_i16_1062, this->var_i16_1064);
			_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
		}
		// 133:0de0

	}
	// 133:0dea
	this->var_i16_106a = 0;
	this->var_i16_1a9c = 0;
	// store polyomino as bitfield.
	for (int j = this->var_i16_68c; j <= this->var_i16_68c + 3; j++) {
		for (int i = this->var_i16_68a; i <= this->var_i16_68a + 3; i++) {
			if (this->arr_i16_2f38[i*32 + j] > 0) {
				this->var_i16_1a9c |= _bitLUT[this->var_i16_106a];
			}
			// 133:0e44
			this->var_i16_106a++;
		}
	// 133:0e5a
	}
	// 133:0e6c
	if ((this->var_i16_1a9c & this->arr_i16_47d8[this->var_i16_7cc*8]) != 0) {
		this->polyominoCancelMove();
	} else {
		this->polyominoMove();
	}
	// 133:0ea2
	this->polyominoDrawFrame();
	this->polyominoCheckIfSolved();
	this->sub_128_4da(1);
}

void FoolGame::polyominoCancelMove() {
	// 133:0eb2
	this->sub_128_50e(0x14, 0x64, 0);
	this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 3];
	this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 4];

	polyominoMove();
}

void FoolGame::polyominoMove() {
	// 133:0f04
	this->var_i16_106a = 0;
	// polyominos are max 4x4
	// add the polyomino to the collision map
	for (int j = this->var_i16_68c; j <= this->var_i16_68c + 3; j++) {
		for (int i = this->var_i16_68a; i <= this->var_i16_68a + 3; i++) {
			if ((this->arr_i16_47d8[this->var_i16_7cc*8] & _bitLUT[this->var_i16_106a]) != 0) {
				this->arr_i16_2f38[i*32 + j] = this->var_i16_7cc;
			}
			// 133:0f6c
			this->var_i16_106a++;
		}
	}
	// 133:0f96
	// set the polyomino position
	this->arr_i16_47d8[this->var_i16_7cc*8 + 3] = this->var_i16_68a;
	this->arr_i16_47d8[this->var_i16_7cc*8 + 4] = this->var_i16_68c;

	// 133:0fce
	if (this->var_i16_1ab0 != 0) {
		debug(5, "FoolGame::polyominoMove: Draw pic 0 (%d, %d)", this->var_i16_1062, this->var_i16_1064);
		_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4]);
	}
	// 133:1002
	this->var_i16_1062 = _screenGrid[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].left;
	this->var_i16_1064 = _screenGrid[this->arr_i16_3b38[this->var_i16_68a*32 + this->var_i16_68c]].top;
	debug(5, "FoolGame::polyominoMove: Draw pic 1 (%d, %d)", this->var_i16_1062, this->var_i16_1064);
	_zbasic->picture(this->var_i16_1062, this->var_i16_1064, this->arr_pic_49d8[this->var_i16_7cc*4 + 1]);

}

void FoolGame::polyominoOnClickFixed() {
	// 133:10a0
	this->var_i16_484 = _screenGrid[this->arr_i16_3b38[this->arr_i16_47d8[this->var_i16_7cc*8 + 3]*32 + this->arr_i16_47d8[this->var_i16_7cc*8 + 4]]].left;
	this->var_i16_7e4 = _screenGrid[this->arr_i16_3b38[this->arr_i16_47d8[this->var_i16_7cc*8 + 3]*32 + this->arr_i16_47d8[this->var_i16_7cc*8 + 4]]].top;
	// play bass tone
	this->sub_128_50e(0x14, 0x64, 0);
	// why do this???
	// best guess: flashing the shape a few times before knocking it back
	// 133:1188
	debug(5, "FoolGame::polyominoOnClickFixed: Draw pic 0 (%d, %d)", this->var_i16_484, this->var_i16_7e4);
	for (int i = 0; i <= 9; i++) {
		_zbasic->picture(this->var_i16_484, this->var_i16_7e4, this->arr_pic_49d8[this->var_i16_7cc*4]);
		g_toolbox->Delay(0);
	}
	// 133:11c2
	this->polyominoDrawFrame();
	this->sub_128_6186();
}

void FoolGame::polyominoReset() {
	// 133:11cc
	// zero out collision grid
	for (int j = 1; j <= this->arr_i16_1eb8[1] + 1; j++) {
		for (int i = 1; i <= this->arr_i16_1eb8[0] + 1; i++) {
			this->arr_i16_2f38[i*32 + j] = 0;
		}
	}
	// 133:122e
	this->fillRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, this->arr_i16_1eb8[0xf]);
	this->var_i16_1ab0 = 0;
	for (this->var_i16_7cc = 1; this->var_i16_7cc <= this->arr_i16_1eb8[0x10]; this->var_i16_7cc++) {
		this->var_i16_68a = this->arr_i16_47d8[this->var_i16_7cc*8 + 5];
		this->var_i16_68c = this->arr_i16_47d8[this->var_i16_7cc*8 + 6];
		this->polyominoMove();
	}
	// 133:12c2
	this->var_i16_1ab0 = 1;
	this->polyominoDrawFrame();
	_stateFlags = kStateNull;
}

void FoolGame::polyominoDrawFrame() {
	// 133:12d4
	g_toolbox->PenNormal();
	g_toolbox->PenSize(3, 3);
	Common::Rect temp;
	temp.top = this->arr_i16_1eb8[0x12];
	temp.left = this->arr_i16_1eb8[0x13];
	temp.bottom = this->arr_i16_1eb8[0x14];
	temp.right = this->arr_i16_1eb8[0x15];
	g_toolbox->FrameRect(temp);
	g_toolbox->PenNormal();
}

void FoolGame::polyominoCheckIfSolved() {
	// 133:12f2
	_activePuzzleSolved = true;
	this->var_i16_484 = 0;
	for (int j = this->arr_i16_1eb8[0x16]; j <= this->arr_i16_1eb8[0x17]; j++) {
		for (int i = this->arr_i16_1eb8[0x18]; i <= this->arr_i16_1eb8[0x19]; i++) {
			this->var_i16_484++;
			if (this->arr_i16_2f38[i*32 + j] != this->arr_i16_4338[this->var_i16_484]) {
				// 133:1368
				_activePuzzleSolved = false;
				j = this->arr_i16_1eb8[0x17];
				i = this->arr_i16_1eb8[0x19];
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
	this->sub_128_2664();
	this->sub_128_6186();
	this->sub_128_d34(this->arr_i16_1eb8[0x12], this->arr_i16_1eb8[0x13], this->arr_i16_1eb8[0x14], this->arr_i16_1eb8[0x15], 0x96);
}

void FoolGame::polyominoStoreState() {
	// 133:1452
	_activePuzzleBuffer = _zbasic->str(212); // empty
	this->var_i16_68a = 1;
	for (int i = 1; i <= this->arr_i16_1eb8[0x10]; i++) {
		Common::String val1 = _zbasic->encodeInt(this->arr_i16_47d8[i*8 + 3]);
		Common::String val2 = _zbasic->encodeInt(this->arr_i16_47d8[i*8 + 4]);
		_activePuzzleBuffer += val1 + val2;
	}
}



};
