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

#include "fool/detection.h"
#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

;
extern Toolbox *g_toolbox;

// sentence join game
void FoolGame::sentenceRun() {
	// 135:0004
	this->sub_128_271a();
	this->var_i16_c00 = 1;
	this->arr_i16_3738[0] = 0xbf;
	this->arr_i16_3738[2] = 0xe1;
	this->arr_i16_3738[4] = 0xbf;
	this->arr_i16_3738[5] = 0;
	this->arr_i16_3738[6] = 0xe1;
	this->arr_i16_3738[7] = 0x200;
	this->arr_i16_3738[8] = 0xf2;
	this->arr_i16_3738[0xa] = 0x156;
	this->arr_i16_3738[0xc] = 0x14;
	this->arr_i16_3738[0xd] = 0;
	this->arr_i16_3738[0xe] = 0x156;
	this->arr_i16_3738[0xf] = 0x200;

	// 135:00ce
	this->var_i16_1066 = puzzlesReadShort();
	this->arr_i32_192c0[0] = g_toolbox->GetPicture(this->var_i16_1066);
	this->arr_i16_1eb8[0] = puzzlesReadShort();
	_zbasic->indexRawSet(puzzlesReadString(), 1, 0);
	this->var_str_1272 = puzzlesReadString().decode(Common::kMacRoman);
	this->var_str_384 = puzzlesReadString().decode(Common::kMacRoman);
	this->var_str_384 = _zbasic->str(215) + this->var_str_384 + _zbasic->str(216); // to reveal XXXXX
	// 135:0152
	_zbasic->menu(8, 7, 1, this->var_str_384);
	this->arr_i16_1eb8[1] = 0x1c2 / this->arr_i16_1eb8[0];
	this->arr_i16_1eb8[2] = 0x100 - ((this->arr_i16_1eb8[0] * this->arr_i16_1eb8[1]) / 2);

	// 135:01e2
	this->var_i16_7e4 = this->arr_i16_1eb8[2];
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		this->arr_i16_4338[i] = 0;
		this->arr_i16_4338[i + this->arr_i16_1eb8[0]] = puzzlesReadByte();
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		// 135:0254
		_zbasic->indexRawSet(puzzlesReadString(), 1, i + this->arr_i16_1eb8[0]);
		_screenGrid[i].top = 0xf2;
		_screenGrid[i].left = this->var_i16_7e4;
		_screenGrid[i].bottom = 0x156;
		_screenGrid[i].right = this->var_i16_7e4 + this->arr_i16_1eb8[1];
		// 135:030c
		this->var_i16_7e4 += this->arr_i16_1eb8[1];
	}
	// 135:0348
	_zbasic->picture(0, 0x14, this->arr_i32_192c0[0]);
	g_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
	Common::Rect temp(this->arr_i16_3738[5], this->arr_i16_3738[4], this->arr_i16_3738[7], this->arr_i16_3738[6]);
	g_toolbox->FillRect(temp, _patterns[2]);
	this->arr_i16_1eb8[4] = 0;
	for (this->var_i16_68a = 1; this->var_i16_68a <= this->arr_i16_1eb8[0]; this->var_i16_68a++) {
		// 135:03a4
		if (_activePuzzleBuffer.empty()) { // was: str(217)
			this->arr_i16_4338[this->var_i16_68a] = 1;
		} else {
			// 135:03d4
			this->arr_i16_4338[this->var_i16_68a] = _zbasic->castInt(_zbasic->midStr(_activePuzzleBuffer, this->var_i16_68a, 1));

		}
		// 135:0408
		if (this->arr_i16_4338[this->var_i16_68a] == 1) {
			this->playTone(_zbasic->rndInt(1000) + 0x19, 0x28, 1);
			this->sub_135_a34();
		}
		// 135:044a
	}
	// 135:0466
	if (_activePuzzleBuffer.empty()) { // was: str(218)
		this->var_str_1070 = _zbasic->index(1, 0);
	} else {
		// 135:049a
		this->var_i16_1372 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, this->arr_i16_1eb8[0]+1, 2));
		this->var_str_1070 = _zbasic->midStr(_activePuzzleBuffer, this->arr_i16_1eb8[0]+3, this->var_i16_1372);
	}
	// 135:04f2
	this->sub_135_b16();
	_stateFlags = kStateNull;
	if (this->var_str_1070 == this->var_str_1272) {
		_activePuzzleSolved = true;
	} else {
		_activePuzzleSolved = false;
	}
	// 135:0520
	do {
		this->getNextEvent(-1);
		if (_event.what == kMouseDown) {
			this->sub_135_5b6();
		}
		// 135:0538
		if (this->var_str_1070 == this->var_str_1272) {
			_activePuzzleSolved = true;
		}
		// 135:0552
		if (_stateFlags == kStateUndo) {
			this->sub_135_9ba();
			_stateFlags = kStateNull;
		}
		// 135:0566
		if (_stateFlags == kStateSaveGame) {
			this->sub_135_c1c();
			this->saveGame();
		}
		// 135:0578
	} while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved));
	// 135:05a0
	if (_activePuzzleSolved) {
		this->sub_135_cee();
	}
	this->sub_135_c1c();
	// JMP 0xd9c
}

void FoolGame::sub_135_5b6() {
	// 135:05b6
	this->var_i16_1abc = 0;
	if ((_event.where.x < this->arr_i16_1eb8[2]) || (_event.where.y < 0xf2))
		return;
	// determine which button was clicked on
	this->var_i16_103a = ((_event.where.x - this->arr_i16_1eb8[2]) / this->arr_i16_1eb8[1]) + 1;
	// 135:0632
	if ((this->var_i16_103a > this->arr_i16_1eb8[0]) || (this->arr_i16_4338[this->var_i16_103a] == 0))
		return;
	// 135:0676
	this->arr_i16_4338[this->var_i16_103a] = 0;
	g_toolbox->FillOval(_screenGrid[this->var_i16_103a], _patterns[2]);
	// 135:06aa
	switch (this->arr_i16_4338[this->var_i16_103a + this->arr_i16_1eb8[0]]-1) {
	case 0:
		this->sentenceAddLeft();
		break;
	case 1:
		this->sentenceAddRight();
		break;
	case 2:
		this->sub_135_81e();
		break;
	case 3:
		this->sub_135_86a();
		break;
	case 4:
		this->sub_135_94c();
		break;
	default:
		warning("sub_135_5b6: breaking out of switch");
		break;
	}
	this->sub_135_b16();
	this->sub_128_6186();
	if (this->var_str_1070 == this->var_str_1272) {
		return;
	}
	// 135:0714
	this->var_i16_484 = 0;
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		if (this->arr_i16_4338[i] != 0) {
			this->var_i16_484 = 1;
		}
	}
	// 135:0758
	if (this->var_i16_484 != 0) {
		return;
	}
	_zbasic->text(0, 0xc, 0, kSrcBic);
	this->var_i16_7a2 = 0x124;
	this->sub_128_918(_zbasic->str(219)); // click mouse to reset puzzle

	this->sub_128_6186();
	this->var_i16_1abc = 1;
	_stateFlags = kNullEvent;
	// 135:07a0
	while ((_event.modifiers & kModMouseButtonUp) && (_stateFlags == kNullEvent)) {
		this->getNextEvent(-1);
	}
	// 135:07c8
	if (_stateFlags == kNullEvent) {
		this->sub_135_9ba();
	}
}

void FoolGame::sentenceAddLeft() {
	// 135:07d6
	this->var_str_1070 = _zbasic->index(1, this->var_i16_103a) + this->var_str_1070;
}

void FoolGame::sentenceAddRight() {
	// 135:07fa
	this->var_str_1070 += _zbasic->index(1, this->var_i16_103a);
}

void FoolGame::sub_135_81e() {
	// 135:0813
	this->var_str_1070 = _zbasic->index(1, this->var_i16_103a)
		+ this->var_str_1070
		+ _zbasic->index(1, this->var_i16_103a + this->arr_i16_1eb8[0]);
}

void FoolGame::sub_135_86a() {
	// 135:086a
	this->var_i16_9f2 = _zbasic->index(1, this->var_i16_103a).size();
	while (true) {
		// 135:0884
		this->var_i16_484 = this->var_str_1070.size();
		this->var_i16_1abe = _zbasic->instr(1, this->var_str_1070, _zbasic->index(1, this->var_i16_103a));
		if (this->var_i16_1abe == 0)
			return;
		// 135:08bc
		if ((this->var_i16_484 - this->var_i16_1abe + 1 - this->var_i16_9f2) < 0)
			return;
		this->var_str_1070 = _zbasic->leftStr(this->var_str_1070, this->var_i16_1abe - 1)
			+ _zbasic->index(1, this->var_i16_103a + this->arr_i16_1eb8[0])
			+ _zbasic->rightStr(this->var_str_1070, this->var_i16_484 - this->var_i16_1abe + 1 - this->var_i16_9f2);
	}
	// should never reach here
	this->sub_135_94c();
}

void FoolGame::sub_135_94c() {
	// 135:094c
	this->var_i16_484 = this->var_str_1070.size();
	this->var_str_384.clear(); // was: str(220)
	for (int16 i = this->var_i16_484; i >= 1; i--) {
		this->var_str_384 += _zbasic->midStr(this->var_str_1070, i, 1);
	}
	this->var_str_1070 = this->var_str_384;
}

void FoolGame::sub_135_9ba() {
	// 135:09ba
	this->arr_i16_1eb8[4] = 0;
	Common::Rect temp(this->arr_i16_3738[5], this->arr_i16_3738[4], this->arr_i16_3738[7], this->arr_i16_3738[6]);
	g_toolbox->FillRect(temp, _patterns[2]);
	for (this->var_i16_68a = 1; this->var_i16_68a <= this->arr_i16_1eb8[0]; this->var_i16_68a++) {
		this->arr_i16_4338[this->var_i16_68a] = 1;
		this->sub_135_a34();
	}
	this->var_str_1070 = _zbasic->index(1, 0);
	this->sub_135_b16();
}

void FoolGame::sub_135_a34() {
	// 135:0a34
	_zbasic->text(0, 0xc, 0, kSrcOr);
	this->var_str_384 = Common::U32String::format("%d", this->var_i16_68a) + _zbasic->str(221);
	this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_384);
	g_toolbox->FillOval(_screenGrid[this->var_i16_68a], _patterns[0]);
	g_toolbox->FrameOval(_screenGrid[this->var_i16_68a]);
	g_toolbox->MoveTo(_screenGrid[this->var_i16_68a].left + (this->arr_i16_1eb8[1] / 2) - (this->var_i16_7e4 / 2), 0x129);
	g_toolbox->DrawString(this->var_str_384);
}

void FoolGame::sub_135_b16() {
	// 135:0b16
	_zbasic->text(0xfe, 0x18, 0, kSrcOr);
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_1070);
	if (this->var_i16_484 < this->arr_i16_1eb8[4]) {
		this->var_i16_484 = this->arr_i16_1eb8[4];
	} else {
		this->arr_i16_1eb8[4] = this->var_i16_484;
	}
	// 135:0b80
	this->arr_i16_3738[1] = 0xf6 - (this->var_i16_484 / 2);
	this->arr_i16_3738[3] = 0x109 + (this->var_i16_484 / 2);
	Common::Rect temp(this->arr_i16_3738[1], this->arr_i16_3738[0], this->arr_i16_3738[3], this->arr_i16_3738[2]);
	g_toolbox->FillRect(temp, _patterns[0]);
	g_toolbox->MoveTo(0x100 - (this->var_i16_484 / 2), 0xd9);
	g_toolbox->DrawString(this->var_str_1070);
}

void FoolGame::sub_135_c1c() {
	// 135:0c1c
	_activePuzzleBuffer.clear(); // was: str(222)
	if (this->var_i16_1abc != 0) {
		this->var_i16_1abc = 0;
		return;
	}
	// 135:0c44
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		this->var_str_384 = Common::U32String::format("%d", this->arr_i16_4338[i]);
		this->var_str_384 = _zbasic->rightStr(this->var_str_384, 1);
		_activePuzzleBuffer += this->var_str_384;
	}
	// 135:0cba
	Common::String temp = this->var_str_1070.encode(Common::kMacRoman);
	this->var_i16_1372 = (int16)temp.size();
	_activePuzzleBuffer += _zbasic->encodeInt(this->var_i16_1372) + temp;
}

void FoolGame::sub_135_cee() {
	// 135:0cee
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
		this->var_i16_68a = 1;
		for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->playTone(_zbasic->rndInt(1000) + 0x19, 0x28, 0);
		}
	}
	// 135:0d40
	this->sub_128_2664();
	this->sub_128_6186();
	this->sub_128_d34(this->arr_i16_3738[0], this->arr_i16_3738[1], this->arr_i16_3738[2], this->arr_i16_3738[3], 0x64);
}


}
