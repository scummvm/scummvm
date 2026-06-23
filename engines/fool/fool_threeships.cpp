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

#define OFF(x) (_zstrOffset[kOffsetThreeShips] + (x))



// three freakin' ships
void FoolGame::shipsRun() {
	// 128:5140
	fillRect(0x127, 0x6c, 0x137, 0x84, 0);
	this->waitForMouseUp();
	this->drawPuzzleButton(_zbasic->str(OFF(0))); // "?"
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);

	fillRect(0x127, 0x6c, 0x137, 0x84, 0);

	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_bbbc);
	// 138:51f0
	this->var_i16_d08 = _zbasic->rndInt(5) + 5;
	this->var_i16_d0a = _zbasic->rndInt(5) - 0xa;
	for (int16 i = 0; i <= 5; i++) {
		_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kSrcCopy);
		_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->sub_128_5a6c();
	}
	// 128:5274
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;
	// 128:530c
	_activePuzzleSolved = false;
	_keyLastPressed = 0;
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		this->getNextEvent(-1);
		if (_toolbox->PtInRect(_event.where, this->arr_rect_19114) != 0) {
			this->sub_128_55ac();
		}
		// 128:5348
		this->var_i16_d0e = _event.where.y;
		this->var_i16_d10 = _event.where.x;
		if (_keyLastPressed > 0) {
			this->sub_128_57a2();
		}
		if (_stateFlags == kStateSaveGame) {
			this->saveGame();
		}
	}
	// 128:5396
	if (_activePuzzleSolved) {
		this->waitForMouseUp();
		this->playTone(0x14, 0x64, 0);
		_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kSrcCopy);
		_event.where.x += 5;
		_event.where.y -= 5;
		if (_activePuzzleStatus < 0x64) {
			_toolbox->SetCursor(_cursors[0x10]);
		}
		for (int16 i = 1; i <= 1; i++) {
			// 128:541c
			this->arr_i16_4758[0] = _event.where.y;
			this->arr_i16_4758[1] = _event.where.x;
			this->arr_i16_4758[2] = _event.where.y;
			this->arr_i16_4758[3] = _event.where.x;
			for (int16 j = 1; j <= 0x19; j++) {
				// 128:5462
				this->arr_i16_4758[0]--;
				this->arr_i16_4758[1]--;
				this->arr_i16_4758[2]++;
				this->arr_i16_4758[3]++;
				Common::Rect temp;
				temp.top = this->arr_i16_4758[0];
				temp.left = this->arr_i16_4758[1];
				temp.bottom = this->arr_i16_4758[2];
				temp.right = this->arr_i16_4758[3];
				_toolbox->InvertOval(temp);
			}
		}
		// 128:5514
		this->delay(0x3c);
		if (_activePuzzleStatus < 0x64) {
			// 128:5526
			this->waitForMouseUp();
			_activePuzzleStatus = 0x64;
			this->var_i16_7ce |= 2;
			this->zoomRect(
				_event.where.y - 5,
				_event.where.x - 5,
				_event.where.y + 5,
				_event.where.x + 5,
				0x14,
				0,
				SCREEN_HEIGHT,
				SCREEN_WIDTH,
				2,
				kPatCopy,
				0x42
			);
			// behold the 1st key of thoth
			this->showBehold(0, 0, _zbasic->str(OFF(1)));
		}
		// 128:55aa
	}
	// 128:55aa
	return;
}

void FoolGame::sub_128_55ac() {
	// 128:55ac
	if ((this->var_i16_d10 == _event.where.x) && (this->var_i16_d0e == _event.where.y)) {
		return;
	}
	this->var_i16_d08 = _zbasic->rndInt(5) + 5;
	this->var_i16_d0a = _zbasic->rndInt(5) + 5;
	if (_zbasic->maybe()) {
		this->var_i16_d08 *= -1;
	}
	// 128:560a
	if (_zbasic->maybe()) {
		this->var_i16_d0a *= -1;
	}
	// 128:5620
	if (this->arr_rect_1910c.top < 0x50) {
		this->var_i16_d0a = 0xa;
	}
	// 128:5640
	if (this->arr_rect_1910c.left < 0x3c) {
		this->var_i16_d08 = 0xa;
	}
	if (this->arr_rect_1910c.bottom > 0x104) {
		this->var_i16_d0a = -0xa;
	}
	if (this->arr_rect_1910c.right > 0x1a4) {
		this->var_i16_d08 = -0xa;
	}
	// 128:56a0
	for (this->var_i16_68a = 0; this->var_i16_68a <= 0x5; this->var_i16_68a++) {
		_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kSrcCopy);
		_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->sub_128_5a6c();
	}
	// 128:5708
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_57a2() {
	// 128:57a2
	this->var_str_d12 = _zbasic->ucase(_zbasic->chr(_keyLastPressed));
	if (_zbasic->instr(1, _zbasic->str(OFF(2)), this->var_str_d12) > 0) { // NESW
		// 128:57e0
		_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kSrcCopy);
		this->var_i16_d08 = 0;
		this->var_i16_d0a = 0;
		if (this->var_str_d12 == _zbasic->str(OFF(3))) { // N
			this->var_i16_d0a = -0x10;
		}
		if (this->var_str_d12 == _zbasic->str(OFF(4))) { // S
			this->var_i16_d0a = 0x10;
		}
		if (this->var_str_d12 == _zbasic->str(OFF(5))) { // W
			this->var_i16_d08 = -0x18;
		}
		if (this->var_str_d12 == _zbasic->str(OFF(6))) { // E
			this->var_i16_d08 = 0x18;
		}
		_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->var_i16_d08 = 0;
		this->var_i16_d0a = 0;
		// 128:58c2
		if (this->arr_rect_1910c.top < 0x14) {
			this->arr_rect_1910c.top = 0x146;
			this->arr_rect_1910c.bottom = SCREEN_HEIGHT;
		}
		if (this->arr_rect_1910c.left < 0) {
			this->arr_rect_1910c.left = 0x1e8;
			this->arr_rect_1910c.right = SCREEN_WIDTH;
		}
		// 128:592e
		if (this->arr_rect_1910c.bottom > SCREEN_HEIGHT) {
			this->arr_rect_1910c.top = 0x14;
			this->arr_rect_1910c.bottom = 0x24;
		}
		if (this->arr_rect_1910c.right > SCREEN_WIDTH) {
			this->arr_rect_1910c.left = 0;
			this->arr_rect_1910c.right = 0x18;
		}
		this->sub_128_5a6c();
		if (_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
			_activePuzzleSolved = true;
		}
	}
	// 128:59cc
	_keyLastPressed = 0;
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_5a6c() {
	// 128:5a6c
	this->var_i32_692 = _toolbox->TickCount();
	_zbasic->get(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_rect_1910c.right, this->arr_rect_1910c.bottom, this->arr_bmp_bbbc);
	//warning("sub_128_5a6c: bbbc surface");
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);
	//this->arr_bmp_bbbc->rawSurface().debugPrint(5, 0, 0,0, 0, -1, 160, fakePal);
	_toolbox->EraseRoundRect(this->arr_rect_1910c, 8, 7);
	_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_b3ec, kSrcXor);
	this->delayFromMarker(0);
}

}
