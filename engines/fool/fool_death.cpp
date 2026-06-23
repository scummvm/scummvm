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



// Death challenge
void FoolGame::deathRun() {
	// 141:0004
	_zbasic->get(0x6c, 0x127, 0x84, 0x127, this->arr_bmp_c38c);
	this->drawPuzzleButton(Common::U32String("~")); // was: str(334)
	_toolbox->InvertRect(this->arr_rect_1910c);
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	_zbasic->put(0x6c, 0x127, this->arr_bmp_c38c, kSrcCopy);
	this->deathDrawZoom();
	this->fillRect(0x14f, 0, 0x156, 0x7, 0x2);
	this->fillRect(0x14f, 0x1f9, 0x156, 0x200, 2);
	_deathBlackEye.top = 0x1e;
	_deathBlackEye.left = 0x190;
	_deathBlackEye.bottom = 0x2e;
	_deathBlackEye.right = 0x1a8;
	// 141:0122
	_zbasic->get(_deathBlackEye.left, _deathBlackEye.top, _deathBlackEye.right, _deathBlackEye.bottom, this->arr_bmp_bbbc);
	_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
	_event.where.y = 0x14;
	// JMP 0x3c2
	while (true) {
		// 141:01c8
		// JMP 0x232
		while (_event.where.y >= 0x14) {
			// 141:01cc
			this->getNextEvent(-1);
			this->deathMoveBlackEye();
			if (_toolbox->PtInRect(_event.where, _deathBlackEye)) {
				this->deathCaught();
				// 141:09f6
				this->deathDrawWhiteEye();
				return;
			}
			// 141:0200
			if (_stateFlags == kStateUndo) {
				_stateFlags = kStateNull;
			}
			if (_stateFlags == kStateSaveGame) {
				this->saveGame();
			}
			if ((_stateFlags & kStateReturn)) {
				// 141:09f6
				this->deathDrawWhiteEye();
				return;
			}
		}
		// 141:023a
		_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_c38c);
		this->drawPuzzleButton(Common::U32String("~")); // was: str(335))
		_deathWhiteEyeNeedsDraw = true;
		// JMP 0x2be
		while (_event.where.y < 0x14) {
			// cursor is over the top menu, safe
			// 141:0282
			this->deathMoveBlackEye();
			this->getNextEvent(-1);
			if (_stateFlags == kStateUndo) {
				_stateFlags = kStateNull;
			}
			if (_stateFlags == kStateSaveGame) {
				this->saveGame();
			}
		}
		// 141:02c6
		this->getNextEvent(0);
		if (_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
			this->var_i32_692 = _toolbox->TickCount();
			// 141:02fa
			do {
				this->getNextEvent(0);
				if (((_event.modifiers & kModMouseButtonUp) == 0) &&
					_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
					// JMP 0x9a2
					// 141:09a2
					this->deathDrawWhiteEye();
					_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_bbbc, kSrcCopy);
					this->deathDrawZoom();
					if (_activePuzzleStatus < 0x64) {
						_activePuzzleStatus = 0x64;
					}
					this->deathDrawWhiteEye();
					return;
				}
				// 141:0342
			} while (_toolbox->TickCount() <= (this->var_i32_692 + 0x2d));
		}
		// 141:035a
		// mouse moved out from menu, zap the white eye
		_toolbox->PenNormal();
		_toolbox->PenMode(kNotPatXor);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->PenSize(0x17, 0x10);
		for (int i = 0; i <= 5; i++) {
			_toolbox->MoveTo(_deathBlackEye.left, _deathBlackEye.top);
			_toolbox->LineTo(0x6c, 0x127);
			_toolbox->Delay(0);
		}
		// 141:03bc
		this->deathDrawWhiteEye();
		_toolbox->PenNormal();
	}
	// 141:03c2
	this->deathMoveBlackEye();
}

void FoolGame::deathMoveBlackEye() {
	// 141:03ca
	_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
	this->var_i16_68a = _event.where.x - 0xc - _deathBlackEye.left;
	this->var_i16_68c = _event.where.y - 8 - _deathBlackEye.top;
	// 141:0452
	if (this->var_i16_68c < -4) {
		this->var_i16_68c = -4;
	}
	if (this->var_i16_68c > 4) {
		this->var_i16_68c = 4;
	}
	if (this->var_i16_68a < -6) {
		this->var_i16_68a = -6;
	}
	if (this->var_i16_68a > 6) {
		this->var_i16_68a = 6;
	}
	_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:04ce
	Common::Rect newPos; // arr_rect_1ec0
	newPos.top = _deathBlackEye.top + this->var_i16_68c;
	newPos.left = _deathBlackEye.left + this->var_i16_68a;
	newPos.bottom = _deathBlackEye.bottom + this->var_i16_68c;
	newPos.right = _deathBlackEye.right + this->var_i16_68a;

	_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:05a2
	if (newPos.top < 0x14) {
		newPos.top = 0x14;
		newPos.bottom = 0x24;
	}
	// 141:05dc
	if (newPos.left < 0) {
		newPos.left = 0;
		newPos.right = 0x18;
	}
	// 141:060c
	if (newPos.bottom > SCREEN_HEIGHT) {
		newPos.top = 0x146;
		newPos.bottom = 0x156;
	}
	if (newPos.right > SCREEN_WIDTH) {
		newPos.left = 0x1e8;
		newPos.right = 0x200;
	}
	_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:06bc
	if (!((_deathBlackEye.top == newPos.top) &&
		((_deathBlackEye.left == newPos.left)))) {
		// 141:0722
		_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_bbbc, kSrcCopy);
		_deathBlackEye = newPos;
		_zbasic->get(_deathBlackEye.left, _deathBlackEye.top, _deathBlackEye.right, _deathBlackEye.bottom, this->arr_bmp_bbbc);
		_zbasic->put(_deathBlackEye.left, _deathBlackEye.top, this->arr_bmp_b3ec, kSrcCopy);
		this->delay(1);
	}
	// 141:0880
}

void FoolGame::deathDrawWhiteEye() {
	// 141:0882
	if (_deathWhiteEyeNeedsDraw) {
		_zbasic->put(0x6c, 0x127, this->arr_bmp_c38c, kSrcCopy);
	}
	// 141:08b0
	_deathWhiteEyeNeedsDraw = false;
}

void FoolGame::deathCaught() {
	// 141:08b8
	this->deathDrawWhiteEye();
	this->toggleMouseCursor(false);
	this->zoomRect(_deathBlackEye.top, _deathBlackEye.left, _deathBlackEye.bottom, _deathBlackEye.right, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x19);
	this->toggleMouseCursor(true);
}

void FoolGame::deathDrawZoom() {
	// 141:0934
	for (int i = 1; i <= 2; i++) {
		this->zoomRect(0x130, 0x76, 0x130, 0x76, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x1a);
	}
	this->fillRect(1, kNotPatXor, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
}


}
