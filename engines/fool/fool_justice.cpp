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

// justice - lights on
void FoolGame::justiceRun() {
	// 142:0852
	if (_activePuzzleStatus < 0x63) {
		this->sub_142_9be();
		if (!_activePuzzleSolved) {
			this->justiceStoreState();
			return;
		}
		// 142:0874
		this->justiceResetGrid();
		if (_puzzleCompletionStatus[0x34] > 3) {
			_activePuzzleStatus = 0x64;
			return;
		} else {
			// 142:08a2
			this->zoomRect(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x1a);
			_activePuzzleStatus = 0x63;
			this->var_str_384 = _zbasic->str(345); // behold the 3rd key of thoth
			this->sub_128_178a(0x3f, 1);
		}
	}
	// 142:0906
	if (_activePuzzleStatus == 0x63) {
		_zbasic->menu(8, 0, 1, _zbasic->str(346)); // the 3rd key of thoth
		_zbasic->menu(8, 1, 1, _zbasic->str(347)); // return to scroll
		this->thothKey3rd();
		if (_activePuzzleSolved) {
			this->zoomRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xa2, 0xff, 0xac, 0x10e, 2, kPatXor, 0x19);
		}
	}
	// 142:098a
	if (_activePuzzleStatus >= 0x64) {
		this->sub_142_9be();
		if (!_activePuzzleSolved) {
			this->justiceStoreState();
			return;
		}
		// 142:09b0
		this->justiceResetGrid();
		_activePuzzleStatus = 0x65;
	}
	// 142:09ba
	return;
}

void FoolGame::sub_142_9be() {
	// 142:09be
	this->sub_128_271a();
	for (int16 i = 1; i <= 0x19; i++) {
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
	}
	// 142:09ea
	if (_puzzleCompletionStatus[0x34] < 4) {
		_zbasic->menu(8, 3, 1, _zbasic->str(348)); // a secret hides here
		_zbasic->menu(8, 4, 1, _zbasic->str(349)); // if twenty-five appear
	} else {
		// 142:0a3a
		_zbasic->menu(8, 3, 1, _zbasic->str(350)); // you will remain here
		_zbasic->menu(8, 4, 1, _zbasic->str(351)); // unless twenty-five appear
	}
	// 142:0a6a
	this->justiceZoom();
	this->var_i16_484 = 0;
	for (int16 i = 0x24; i <= 0x11d; i += 0x32) {
		for (int16 j = 0x8c; j <= 0x185; j += 0x32) {
			this->var_i16_484++;
			g_toolbox->SetRect(
				_screenGrid[this->var_i16_484],
				j,
				i,
				j + 0x32,
				i + 0x32
			);
		}
	}
	// 142:0adc
	this->justiceZoom();
	this->fillRect(0x1a, 0x82, 0x128, 0x190, 2);
	if (_activePuzzleBuffer.empty()) { // was: str(352)
		_activePuzzleBuffer = (_zbasic->space(0xc) + _zbasic->str(353) + _zbasic->space(0xc)).encode(Common::kMacRoman); // 1
		this->var_i16_233e = 1;
	} else {
		// 142:0b4c
		this->var_i16_233e = _zbasic->castInt(_zbasic->leftStr(_activePuzzleBuffer, 1));
		_activePuzzleBuffer = _zbasic->rightStr(_activePuzzleBuffer, 0x19);
	}
	// 142:0b7c
	for (this->var_i16_484 = 1; this->var_i16_484 <= 0x19; this->var_i16_484++) {
		this->var_i16_103a = _zbasic->castInt(_zbasic->midStr(_activePuzzleBuffer, this->var_i16_484, 1));
		if ((this->var_i16_103a & 2) == 0) {
			_zbasic->indexSet(_zbasic->str(354), 1, (this->var_i16_484 + 0x19));
		} else {
			// 142:0be0
			_zbasic->indexSet(_zbasic->str(355), 1, (this->var_i16_484 + 0x19));
		}
		// 142:0c06
		if ((this->var_i16_103a & 1) != 0) {
			this->justiceDrawBlock();
		} else {
			this->justiceRemoveBlock();
		}
	}
	// 142:0c32
	_activePuzzleSolved = false;
	_stateFlags = 0;
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 142:0c42
		while ((_stateFlags == 0) && (!_activePuzzleSolved)) {
			this->getNextEvent(-1);
			if (_event.what == 1) {
				this->justiceOnClick();
			}
		}
		// 142:0c76
		if (_stateFlags == kStateSaveGame) {
			this->justiceStoreState();
			this->saveGame();
		}
		// 142:0c88
	}
}

void FoolGame::justiceOnClick() {
	// 142:0cb2
	this->var_i16_68a = (_event.where.x - 0x5a) / 0x32;
	this->var_i16_68c = (_event.where.y + 0xe) / 0x32;
	if ((this->var_i16_68a < 0) || (this->var_i16_68c < 0) || (this->var_i16_68a > 5) || (this->var_i16_68c > 5)) {
		return;
	}
	// 142:0d26
	this->var_i16_103a = (this->var_i16_68c - 1)*5 + this->var_i16_68a;
	if (this->arr_i16_3738[this->var_i16_103a] == 0) {
		return;
	}
	// 142:0d56
	g_toolbox->InvertRoundRect(_screenGrid[this->var_i16_103a], 0x1e, 0x1e);
	if (_zbasic->index(1, this->var_i16_103a + 0x19) == _zbasic->str(356)) { // \xa5
		_zbasic->indexSet(_zbasic->str(357), 1, this->var_i16_103a + 0x19); // ~
	}
	// 142:0dc6
	if (this->var_i16_103a == 0xd) { // center button
		if (this->var_i16_233e == 0) {
			for (this->var_i16_484 = 1; this->var_i16_484 <= 0x19; this->var_i16_484++) {
				_zbasic->indexSet(_zbasic->str(358), 1, this->var_i16_484 + 0x19); // \xa5
				this->arr_i16_3738[this->var_i16_484] = 0;
				this->justiceRemoveBlock();
			}
			this->var_i16_233e = 1;
			// redraw center tile
			this->var_i16_484 = 0xd;
			this->justiceDrawBlock();
			return;
		}
		// 142:0e3e
		this->var_i16_233e = 0;
	}
	// 142:0e44
	Common::String buffer = _zbasic->indexRaw(1, this->var_i16_103a);
	if (_zbasic->leftStr(buffer, 1) == _zbasic->str(359).encode(Common::kMacRoman)) { // M
		this->var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(buffer, 2, 2));
		this->justiceDrawBlock();
		this->var_i16_2340 = 5;
	} else {
		// 142:0ea8
		this->var_i16_2340 = 2;
	}
	// 142:0eae
	this->var_i16_2342 = _zbasic->decodeInt(_zbasic->midStr(buffer, this->var_i16_2340, 2));
	for (int16 i = 1; i <= this->var_i16_2342; i++) {
		this->var_i16_2340 += 2;
		this->var_i16_484 = _zbasic->decodeInt(_zbasic->midStr(buffer, this->var_i16_2340, 2));
		// 142:0ef0
		if (this->arr_i16_3738[this->var_i16_484] == 0) {
			this->justiceDrawBlock();
		} else {
			this->justiceRemoveBlock();
		}
	}
	// 142:0f20
	_activePuzzleSolved = true;
	for (int16 i = 1; i <= 0x19; i++) {
		if (this->arr_i16_3738[i] == 0) {
			_activePuzzleSolved = false;
		}
	}
	this->sub_128_6186();
}

void FoolGame::justiceZoom() {
	// 142:0f58
	this->zoomRect(0x130, 0x76, 0x130, 0x76, 0x1a, 0x82, 0x128, 0x190, 1, kPatXor, 0x19);
}

void FoolGame::justiceDrawBlock() {
	// 142:0f96
	this->arr_i16_3738[this->var_i16_484] = 1;
	this->sub_128_50e(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	g_toolbox->PenNormal();
	g_toolbox->FillRoundRect(_screenGrid[this->var_i16_484], 0x1e, 0x1e, _patterns[0]);
	g_toolbox->FrameRoundRect(_screenGrid[this->var_i16_484], 0x1e, 0x1e);
	_zbasic->text(0, 0xc, 0, kSrcOr);
	this->var_str_384 = _zbasic->index(1, this->var_i16_484 + 0x19);
	this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_384);
	// 142:1056
	g_toolbox->MoveTo(
		_screenGrid[this->var_i16_484].left + 0x19 - (this->var_i16_7e4/2),
		_screenGrid[this->var_i16_484].top + 0x1e
	);
	g_toolbox->DrawString(this->var_str_384);
}

void FoolGame::justiceRemoveBlock() {
	// 142:10bc
	this->arr_i16_3738[this->var_i16_484] = 0;
	this->sub_128_50e(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
	g_toolbox->FillRect(_screenGrid[this->var_i16_484], _patterns[_zbasic->rndInt(0x4d)+3]);
}

void FoolGame::justiceStoreState() {
	// 142:111e
	this->var_str_384 = Common::U32String::format(" %d", this->var_i16_233e);
	_activePuzzleBuffer = _zbasic->rightStr(this->var_str_384, 1);
	for (int16 i = 1; i <= 0x19; i++) {
		this->var_i16_484 = this->arr_i16_3738[i];
		if (_zbasic->index(1, i + 0x19) == _zbasic->str(360)) { // ~
			this->var_i16_484 |= 2;
			this->var_str_384 = _zbasic->rightStr(Common::U32String::format(" %d", this->var_i16_484), 1);
			_activePuzzleBuffer += this->var_str_384;
		}
	}
	// 142:11fc
}

void FoolGame::justiceResetGrid() {
	// 142:11fe
	this->sub_128_3da(0x28);
	_activePuzzleBuffer.clear();
	for (int16 i = 1; i <= 0x19; i++) {
		this->sub_128_50e(_zbasic->rndInt(0x1f4) + 0x14, 0x28, 1);
		g_toolbox->FillRect(_screenGrid[i], _patterns[2]);
	}
	// 142:126e
	this->zoomRect(0xec, 0x154, 0x11e, 0x186, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x1a);
}

}
