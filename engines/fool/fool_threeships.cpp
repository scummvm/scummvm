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
	waitForMouseUp();
	drawPuzzleButton(_zbasic->str(OFF(0))); // "?"
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, arr_bmp_b3ec);

	fillRect(0x127, 0x6c, 0x137, 0x84, 0);

	_zbasic->get(0x6c, 0x127, 0x84, 0x137, arr_bmp_bbbc);
	// 138:51f0
	var_i16_d08 = _zbasic->rndInt(5) + 5;
	var_i16_d0a = _zbasic->rndInt(5) - 0xa;
	for (int16 i = 0; i <= 5; i++) {
		_zbasic->put(arr_rect_1910c.left, arr_rect_1910c.top, arr_bmp_bbbc, kSrcCopy);
		_toolbox->OffsetRect(arr_rect_1910c, var_i16_d08, var_i16_d0a);
		sub_128_5a6c();
	}
	// 128:5274
	arr_rect_19114.top = arr_rect_1910c.top - 0xa;
	arr_rect_19114.left = arr_rect_1910c.left - 0xa;
	arr_rect_19114.bottom = arr_rect_1910c.bottom + 0xa;
	arr_rect_19114.right = arr_rect_1910c.right + 0xa;
	// 128:530c
	_activePuzzleSolved = false;
	_keyLastPressed = 0;
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		getNextEvent(-1);
		if (_toolbox->PtInRect(_event.where, arr_rect_19114) != 0) {
			sub_128_55ac();
		}
		// 128:5348
		var_i16_d0e = _event.where.y;
		var_i16_d10 = _event.where.x;
		if (_keyLastPressed > 0) {
			sub_128_57a2();
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
	// 128:5396
	if (_activePuzzleSolved) {
		waitForMouseUp();
		playTone(0x14, 0x64, 0);
		_zbasic->put(arr_rect_1910c.left, arr_rect_1910c.top, arr_bmp_bbbc, kSrcCopy);
		_event.where.x += 5;
		_event.where.y -= 5;
		if (_activePuzzleStatus < 0x64) {
			_toolbox->SetCursor(_cursors[0x10]);
		}
		for (int16 i = 1; i <= 1; i++) {
			// 128:541c
			arr_i16_4758[0] = _event.where.y;
			arr_i16_4758[1] = _event.where.x;
			arr_i16_4758[2] = _event.where.y;
			arr_i16_4758[3] = _event.where.x;
			for (int16 j = 1; j <= 0x19; j++) {
				// 128:5462
				arr_i16_4758[0]--;
				arr_i16_4758[1]--;
				arr_i16_4758[2]++;
				arr_i16_4758[3]++;
				Common::Rect temp;
				temp.top = arr_i16_4758[0];
				temp.left = arr_i16_4758[1];
				temp.bottom = arr_i16_4758[2];
				temp.right = arr_i16_4758[3];
				_toolbox->InvertOval(temp);
			}
		}
		// 128:5514
		delay(0x3c);
		if (_activePuzzleStatus < 0x64) {
			// 128:5526
			waitForMouseUp();
			_activePuzzleStatus = 0x64;
			var_i16_7ce |= 2;
			zoomRect(
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
			showBehold(0, 0, _zbasic->str(OFF(1)));
		}
		// 128:55aa
	}
	// 128:55aa
	return;
}

void FoolGame::sub_128_55ac() {
	// 128:55ac
	if ((var_i16_d10 == _event.where.x) && (var_i16_d0e == _event.where.y)) {
		return;
	}
	var_i16_d08 = _zbasic->rndInt(5) + 5;
	var_i16_d0a = _zbasic->rndInt(5) + 5;
	if (_zbasic->maybe()) {
		var_i16_d08 *= -1;
	}
	// 128:560a
	if (_zbasic->maybe()) {
		var_i16_d0a *= -1;
	}
	// 128:5620
	if (arr_rect_1910c.top < 0x50) {
		var_i16_d0a = 0xa;
	}
	// 128:5640
	if (arr_rect_1910c.left < 0x3c) {
		var_i16_d08 = 0xa;
	}
	if (arr_rect_1910c.bottom > 0x104) {
		var_i16_d0a = -0xa;
	}
	if (arr_rect_1910c.right > 0x1a4) {
		var_i16_d08 = -0xa;
	}
	// 128:56a0
	for (var_i16_68a = 0; var_i16_68a <= 0x5; var_i16_68a++) {
		_zbasic->put(arr_rect_1910c.left, arr_rect_1910c.top, arr_bmp_bbbc, kSrcCopy);
		_toolbox->OffsetRect(arr_rect_1910c, var_i16_d08, var_i16_d0a);
		sub_128_5a6c();
	}
	// 128:5708
	arr_rect_19114.top = arr_rect_1910c.top - 0xa;
	arr_rect_19114.left = arr_rect_1910c.left - 0xa;
	arr_rect_19114.bottom = arr_rect_1910c.bottom + 0xa;
	arr_rect_19114.right = arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_57a2() {
	// 128:57a2
	var_str_d12 = _zbasic->ucase(_zbasic->chr(_keyLastPressed));
	if (_zbasic->instr(1, _zbasic->str(OFF(2)), var_str_d12) > 0) { // NESW
		// 128:57e0
		_zbasic->put(arr_rect_1910c.left, arr_rect_1910c.top, arr_bmp_bbbc, kSrcCopy);
		var_i16_d08 = 0;
		var_i16_d0a = 0;
		if (var_str_d12 == _zbasic->str(OFF(3))) { // N
			var_i16_d0a = -0x10;
		}
		if (var_str_d12 == _zbasic->str(OFF(4))) { // S
			var_i16_d0a = 0x10;
		}
		if (var_str_d12 == _zbasic->str(OFF(5))) { // W
			var_i16_d08 = -0x18;
		}
		if (var_str_d12 == _zbasic->str(OFF(6))) { // E
			var_i16_d08 = 0x18;
		}
		_toolbox->OffsetRect(arr_rect_1910c, var_i16_d08, var_i16_d0a);
		var_i16_d08 = 0;
		var_i16_d0a = 0;
		// 128:58c2
		if (arr_rect_1910c.top < 0x14) {
			arr_rect_1910c.top = 0x146;
			arr_rect_1910c.bottom = SCREEN_HEIGHT;
		}
		if (arr_rect_1910c.left < 0) {
			arr_rect_1910c.left = 0x1e8;
			arr_rect_1910c.right = SCREEN_WIDTH;
		}
		// 128:592e
		if (arr_rect_1910c.bottom > SCREEN_HEIGHT) {
			arr_rect_1910c.top = 0x14;
			arr_rect_1910c.bottom = 0x24;
		}
		if (arr_rect_1910c.right > SCREEN_WIDTH) {
			arr_rect_1910c.left = 0;
			arr_rect_1910c.right = 0x18;
		}
		sub_128_5a6c();
		if (_toolbox->PtInRect(_event.where, arr_rect_1910c)) {
			_activePuzzleSolved = true;
		}
	}
	// 128:59cc
	_keyLastPressed = 0;
	arr_rect_19114.top = arr_rect_1910c.top - 0xa;
	arr_rect_19114.left = arr_rect_1910c.left - 0xa;
	arr_rect_19114.bottom = arr_rect_1910c.bottom + 0xa;
	arr_rect_19114.right = arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_5a6c() {
	// 128:5a6c
	var_i32_692 = _toolbox->TickCount();
	_zbasic->get(arr_rect_1910c.left, arr_rect_1910c.top, arr_rect_1910c.right, arr_rect_1910c.bottom, arr_bmp_bbbc);
	//warning("sub_128_5a6c: bbbc surface");
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);
	//arr_bmp_bbbc->rawSurface().debugPrint(5, 0, 0,0, 0, -1, 160, fakePal);
	_toolbox->EraseRoundRect(arr_rect_1910c, 8, 7);
	_zbasic->put(arr_rect_1910c.left, arr_rect_1910c.top, arr_bmp_b3ec, kSrcXor);
	delayFromMarker(0);
}

} // End of namespace Fool
