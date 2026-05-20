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

#define OFF(x) (_zstrOffset[kOffsetHumbug] + (x))

extern Toolbox *g_toolbox;

// the humbug - "irritating stick" style challenge
void FoolGame::humbugRun() {
	// 142:0004
	if (_activePuzzleStatus < 0x63) {
		this->humbugTrail();
		if (!_activePuzzleSolved) {
			return;
		}
		this->sub_142_5f2();
		if (_puzzleCompletionStatus[0x34] > 2) {
			_activePuzzleStatus = 0x64;
			return;
		}
		// 142:004e
		_activePuzzleStatus = 0x63;
		this->zoomRect(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
		this->var_str_384 = _zbasic->str(OFF(0)); // behold the 2nd key of thoth
		this->sub_128_178a(0x17, 0);
	}
	// 142:00b2
	if (_activePuzzleStatus == 0x63) {
		this->sub_142_630();
	}
	if (_activePuzzleStatus >= 0x64) {
		this->humbugTrail();
		if (_activePuzzleSolved) {
			this->sub_142_5f2();
		}
	}
	// 142:00e2
	return;
}

void FoolGame::humbugTrail() {
	// 142:00e6
	if (_puzzleCompletionStatus[0x34] < 3) {
		_zbasic->menu(8, 3, 1, _zbasic->str(OFF(1))); // A secret is undone
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(2))); // if the two become one.
	} else {
		// 142:0136
		_zbasic->menu(8, 3, 1, _zbasic->str(OFF(3))); // A puzzle is undone
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(4))); // if the two become one.
	}
	// 142:0166
	this->sub_128_271a();
	this->sub_128_4da(0);
	_zbasic->text(_fontChicago, 0xc, 0, kSrcOr);
	this->sub_128_55c(_zbasic->str(OFF(5))); // ~
	// eye button
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	for (int16 i = 1; i <= 0xa42; i++) {
		this->arr_i16_9894[i] = puzzlesReadShort();
		this->arr_i16_ac1c[i] = puzzlesReadShort();
		_zbasic->put(
			this->arr_i16_9894[i],
			this->arr_i16_ac1c[i],
			this->arr_bmp_b3ec,
			kSrcCopy
		);
		// Simulate slow draw speed
		if ((i % 42) == 0)
			g_toolbox->Delay(0);
	}
	// 142:026e
	this->sub_128_4da(1);
	_zbasic->put(
		this->arr_i16_9894[this->var_i16_233c],
		this->arr_i16_ac1c[this->var_i16_233c],
		this->arr_bmp_b3ec,
		kSrcCopy
	);
	this->var_i16_233c = 1;
	_stateFlags = kStateNull;
	_activePuzzleSolved = false;
	this->var_i16_1ab6 = 0;
	this->var_i16_1ab8 = 0;

	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 142:02ee
		this->getNextEvent(-1);
		if (!((this->var_i16_1ab6 == _event.where.x) && (this->var_i16_1ab8 == _event.where.y))) {
			this->sub_142_370();
		}
		if (this->var_i16_233c >= 0xa42) {
			_activePuzzleSolved = true;
		}
		if (_stateFlags == kStateSaveGame) {
			this->saveGame();
		}
		// 142:0346
	}
}

void FoolGame::sub_142_370() {
	// 142:0370
	this->var_i16_7e4 = this->var_i16_233c + 0x19;
	if (this->var_i16_7e4 > 0xa42) {
		this->var_i16_7e4 = 0xa42;
	}
	this->var_i16_9f2 = this->var_i16_233c - 0x19;
	if (this->var_i16_9f2 < 1) {
		this->var_i16_9f2 = 1;
	}
	this->var_i16_1a9c = 0;
	for (int16 i = this->var_i16_7e4; i >= this->var_i16_9f2; i--) {
		this->var_i16_1a96 = _event.where.x - this->arr_i16_9894[i];
		this->var_i16_1a98 = _event.where.y - this->arr_i16_ac1c[i];
		if ((this->var_i16_1a96 >= -5) && (this->var_i16_1a96 <= 0x1e) && (this->var_i16_1a98 >= -5) && (this->var_i16_1a98 <= 0x19)) {
			this->var_i16_1a9c = i + 0xa;
		}
		// 142:0470
	}
	// 142:0484
	if (this->var_i16_1a9c >= 0xa42) {
		this->var_i16_1a9c = 0xa42;
	}
	if (this->var_i16_1a9c != 0) {
		if (this->var_i16_233c < this->var_i16_1a9c) {
			this->var_i16_484 = 1;
		} else {
			this->var_i16_484 = -1;
		}
		this->var_i16_7a8 = this->var_i16_233c;
		do {
			_zbasic->put(
				this->arr_i16_9894[this->var_i16_7a8],
				this->arr_i16_ac1c[this->var_i16_7a8],
				this->arr_bmp_b3ec,
				kSrcCopy
			);
			// 142:051c
		} while (_zbasic->incrAndCheck(this->var_i16_7a8, this->var_i16_1a9c, this->var_i16_484));
		this->var_i16_233c = this->var_i16_1a9c;
		this->var_i16_1ab6 = _event.where.x;
		this->var_i16_1ab8 = _event.where.y;
	} else {
		// 142:054a
		this->var_i16_7e4 = this->var_i16_233c - 0xa;
		if (this->var_i16_7e4 < 1) {
			this->var_i16_7e4 = 1;
		}
		for (int16 i = this->var_i16_233c; i >= this->var_i16_7e4; i--) {
			_zbasic->put(
				this->arr_i16_9894[i],
				this->arr_i16_ac1c[i],
				this->arr_bmp_b3ec,
				kSrcCopy
			);
			// 142:05c8
		}
		this->var_i16_233c -= 0xa;
		if (this->var_i16_233c < 1) {
			this->var_i16_233c = 1;
		}
	}
	// 142:05f0
}

void FoolGame::sub_142_5f2() {
	// 142:05f2
	this->zoomRect(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x19);
}

void FoolGame::sub_142_630() {
	// 142:0630
	_zbasic->menu(8, 0, 1, _zbasic->str(OFF(6))); // the 2nd key of thoth
	_zbasic->menu(8, 1, 1, _zbasic->str(OFF(7))); // return to scroll
	this->thothKey2nd();
	if (_activePuzzleSolved) {
		// 142:066e
		g_toolbox->PenNormal();
		_zbasic->text(kFontLarge, 0x18, 0x19, kSrcBic);
		for (int16 i = 1; i <= 4; i++) {
			Common::Rect temp;
			temp.top = this->arr_i16_2f38[i*32];
			temp.left = this->arr_i16_2f38[i*32+1];
			temp.bottom = this->arr_i16_2f38[i*32+2];
			temp.right = this->arr_i16_2f38[i*32+3];
			g_toolbox->FillRect(temp, _patterns[2]);
			g_toolbox->FrameRect(temp);
			this->var_str_384 = Common::U32String::format(" %d ", i);
			this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_384);
			this->var_i16_9f2 = (this->arr_i16_2f38[i*32+3] - this->arr_i16_2f38[i*32+1])/2;
			// 142:0756
			g_toolbox->MoveTo(
				this->arr_i16_2f38[i*32+1] + this->var_i16_9f2 - (this->var_i16_7e4 / 2),
				this->arr_i16_2f38[i*32 + 2] - 0x1e
			);
			g_toolbox->DrawString(this->var_str_384);
		}
		// 142:07ca
		_zbasic->text(_fontChicago, 0xc, 0, kSrcOr);
		this->sub_128_6186();
		this->sub_128_2664();
		this->var_i16_484 = 0;
		this->var_i16_7e4 = 0;
		this->var_i16_9f2 = 0;
		while (_event.modifiers & kModMouseButtonUp) {
			// 142:07f8
			this->getNextEvent(-1); // was: 0
			this->var_i16_7e4++;
			if (this->var_i16_7e4 == 0x64) {
				this->var_i16_7e4 = 0;
				this->var_i16_9f2++;
				if (this->var_i16_9f2 > 4) {
					this->var_i16_9f2 = 1;
				}
				Common::Rect temp;
				temp.top = this->arr_i16_2f38[this->var_i16_9f2*32];
				temp.left = this->arr_i16_2f38[this->var_i16_9f2*32+1];
				temp.bottom = this->arr_i16_2f38[this->var_i16_9f2*32+2];
				temp.right = this->arr_i16_2f38[this->var_i16_9f2*32+3];
				g_toolbox->InvertRect(temp);
			}
		}
	}
	// 142:0850
}

}
