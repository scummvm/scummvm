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



// the humbug - "irritating stick" style challenge
void FoolGame::humbugRun() {
	// 142:0004
	if (_activePuzzleStatus < 0x63) {
		this->humbugPlayTrail();
		if (!_activePuzzleSolved) {
			return;
		}
		this->humbugSuccess();
		if (_puzzleCompletionStatus[0x34] > 2) {
			_activePuzzleStatus = 0x64;
			return;
		}
		// 142:004e
		_activePuzzleStatus = 0x63;
		this->zoomRect(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
		// behold the 2nd key of thoth
		this->showBehold(0x17, 0, _zbasic->str(OFF(0)));
	}
	// 142:00b2
	if (_activePuzzleStatus == 0x63) {
		this->sub_142_630();
	}
	if (_activePuzzleStatus >= 0x64) {
		this->humbugPlayTrail();
		if (_activePuzzleSolved) {
			this->humbugSuccess();
		}
	}
	// 142:00e2
	return;
}

void FoolGame::humbugPlayTrail() {
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
	this->fetchPuzzleData();
	this->toggleMouseCursor(false);
	_zbasic->text(_fontChicago, 0xc, 0, kSrcOr);
	this->drawPuzzleButton(_zbasic->str(OFF(5))); // ~
	// eye button
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	for (int16 i = 1; i <= 0xa42; i++) {
		_humbugTrail[i].x = puzzlesReadShort();
		_humbugTrail[i].y = puzzlesReadShort();
		_zbasic->put(
			_humbugTrail[i].x,
			_humbugTrail[i].y,
			this->arr_bmp_b3ec,
			kSrcCopy
		);
		// Simulate slow draw speed
		if ((i % 42) == 0)
			_toolbox->Delay(0);
	}
	// 142:026e
	this->toggleMouseCursor(true);
	_zbasic->put(
		_humbugTrail[this->_humbugTrailIndex].x,
		_humbugTrail[this->_humbugTrailIndex].y,
		this->arr_bmp_b3ec,
		kSrcCopy
	);
	this->_humbugTrailIndex = 1;
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
		if (this->_humbugTrailIndex >= 0xa42) {
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
	int16 trailMax = this->_humbugTrailIndex + 0x19;
	if (trailMax > 0xa42) {
		trailMax = 0xa42;
	}
	int16 trailMin = this->_humbugTrailIndex - 0x19;
	if (trailMin < 1) {
		trailMin = 1;
	}
	this->var_i16_1a9c = 0;
	for (int16 i = trailMax; i >= trailMin; i--) {
		int16 dx = _event.where.x - _humbugTrail[i].x;
		int16 dy = _event.where.y - _humbugTrail[i].y;
		if ((dx >= -5) && (dx <= 0x1e) && (dy >= -5) && (dy <= 0x19)) {
			this->var_i16_1a9c = i + 0xa;
		}
		// 142:0470
	}
	// 142:0484
	if (this->var_i16_1a9c >= 0xa42) {
		this->var_i16_1a9c = 0xa42;
	}
	if (this->var_i16_1a9c != 0) {
		int16 incr = (this->_humbugTrailIndex < this->var_i16_1a9c) ? 1 : -1;
		int16 index = this->_humbugTrailIndex;
		do {
			_zbasic->put(
				_humbugTrail[index].x,
				_humbugTrail[index].y,
				this->arr_bmp_b3ec,
				kSrcCopy
			);
			// 142:051c
		} while (_zbasic->incrAndCheck(index, this->var_i16_1a9c, incr));
		this->_humbugTrailIndex = this->var_i16_1a9c;
		this->var_i16_1ab6 = _event.where.x;
		this->var_i16_1ab8 = _event.where.y;
	} else {
		// 142:054a
		int16 rewindTo = this->_humbugTrailIndex - 0xa;
		if (rewindTo < 1) {
			rewindTo = 1;
		}
		for (int16 i = this->_humbugTrailIndex; i >= rewindTo; i--) {
			_zbasic->put(
				_humbugTrail[i].x,
				_humbugTrail[i].y,
				this->arr_bmp_b3ec,
				kSrcCopy
			);
			// 142:05c8
		}
		this->_humbugTrailIndex -= 0xa;
		if (this->_humbugTrailIndex < 1) {
			this->_humbugTrailIndex = 1;
		}
	}
	// 142:05f0
}

void FoolGame::humbugSuccess() {
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
		_toolbox->PenNormal();
		_zbasic->text(kFontLarge, 0x18, 0x19, kSrcBic);
		for (int16 i = 1; i <= 4; i++) {
			Common::Rect temp;
			temp.top = this->arr_i16_2f38[i*32];
			temp.left = this->arr_i16_2f38[i*32+1];
			temp.bottom = this->arr_i16_2f38[i*32+2];
			temp.right = this->arr_i16_2f38[i*32+3];
			_toolbox->FillRect(temp, _patterns[2]);
			_toolbox->FrameRect(temp);
			Common::U32String idStr = Common::U32String::format(" %d ", i);
			int16 width = _toolbox->StringWidth(idStr);
			int16 xOffset = (this->arr_i16_2f38[i*32+3] - this->arr_i16_2f38[i*32+1])/2;
			// 142:0756
			_toolbox->MoveTo(
				this->arr_i16_2f38[i*32+1] + xOffset - (width / 2),
				this->arr_i16_2f38[i*32 + 2] - 0x1e
			);
			_toolbox->DrawString(idStr);
		}
		// 142:07ca
		_zbasic->text(_fontChicago, 0xc, 0, kSrcOr);
		this->waitForMouseUp();
		this->menuClickMessage();

		int16 tickCounter = 0;
		int16 idCounter = 0;
		while (_event.modifiers & kModMouseButtonUp) {
			// 142:07f8
			this->getNextEvent(-1); // was: 0
			tickCounter++;
			if (tickCounter == 0x64) {
				tickCounter = 0;
				idCounter++;
				if (idCounter > 4) {
					idCounter = 1;
				}
				Common::Rect temp;
				temp.top = this->arr_i16_2f38[idCounter*32];
				temp.left = this->arr_i16_2f38[idCounter*32+1];
				temp.bottom = this->arr_i16_2f38[idCounter*32+2];
				temp.right = this->arr_i16_2f38[idCounter*32+3];
				_toolbox->InvertRect(temp);
			}
		}
	}
	// 142:0850
}

}
