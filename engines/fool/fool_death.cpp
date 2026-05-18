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

extern Toolbox *g_toolbox;

// Death challenge
void FoolGame::deathRun() {
	// 141:0004
	_zbasic->get(0x6c, 0x127, 0x84, 0x127, this->arr_bmp_c38c);
	this->sub_128_55c(_zbasic->str(334));
	g_toolbox->InvertRect(this->arr_rect_1910c);
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	_zbasic->put(0x6c, 0x127, this->arr_bmp_c38c, kSrcCopy);
	this->deathDrawZoom();
	this->fillRect(0x14f, 0, 0x156, 0x7, 0x2);
	this->fillRect(0x14f, 0x1f9, 0x156, 0x200, 2);
	this->arr_rect_1eb8.top = 0x1e;
	this->arr_rect_1eb8.left = 0x190;
	this->arr_rect_1eb8.bottom = 0x2e;
	this->arr_rect_1eb8.right = 0x1a8;
	// 141:0122
	_zbasic->get(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_rect_1eb8.right, this->arr_rect_1eb8.bottom, this->arr_bmp_bbbc);
	_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
	_event.where.y = 0x14;
	// JMP 0x3c2
	while (true) {
		// 141:01c8
		// JMP 0x232
		while (_event.where.y >= 0x14) {
			// 141:01cc
			this->getNextEvent(-1);
			this->deathMoveBlackEye();
			if (g_toolbox->PtInRect(_event.where, this->arr_rect_1eb8)) {
				this->deathCaught();
				// 141:09f6
				this->deathDrawWhiteEye();
				return;
			}
			// 141:0200
			if (_stateFlags == 2) {
				_stateFlags = 0;
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
		this->sub_128_55c(_zbasic->str(335)); // "~"
		this->var_i16_233a = 1;
		// JMP 0x2be
		while (_event.where.y < 0x14) {
			// cursor is over the top menu, safe
			// 141:0282
			this->deathMoveBlackEye();
			this->getNextEvent(-1);
			if (_stateFlags == 2) {
				_stateFlags = 0;
			}
			if (_stateFlags == kStateSaveGame) {
				this->saveGame();
			}
		}
		// 141:02c6
		this->getNextEvent(0);
		if (g_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
			this->var_i32_692 = g_toolbox->TickCount();
			// 141:02fa
			do {
				this->getNextEvent(0);
				if (((_event.modifiers & kModMouseButtonUp) == 0) &&
					g_toolbox->PtInRect(_event.where, this->arr_rect_1910c)) {
					// JMP 0x9a2
					// 141:09a2
					this->deathDrawWhiteEye();
					_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_bbbc, kSrcCopy);
					this->deathDrawZoom();
					if (_activePuzzleStatus < 0x64) {
						_activePuzzleStatus = 0x64;
					}
					this->deathDrawWhiteEye();
					return;
				}
				// 141:0342
			} while (g_toolbox->TickCount() <= (this->var_i32_692 + 0x2d));
		}
		// 141:035a
		// mouse moved out from menu, zap the white eye
		g_toolbox->PenNormal();
		g_toolbox->PenMode(kNotPatXor);
		g_toolbox->PenPat(_patterns[1]);
		g_toolbox->PenSize(0x17, 0x10);
		for (int i = 0; i <= 5; i++) {
			g_toolbox->MoveTo(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top);
			g_toolbox->LineTo(0x6c, 0x127);
			g_toolbox->Delay(0);
		}
		// 141:03bc
		this->deathDrawWhiteEye();
		g_toolbox->PenNormal();
	}
	// 141:03c2
	this->deathMoveBlackEye();
}

void FoolGame::deathMoveBlackEye() {
	// 141:03ca
	_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
	this->var_i16_68a = _event.where.x - 0xc - this->arr_rect_1eb8.left;
	this->var_i16_68c = _event.where.y - 8 - this->arr_rect_1eb8.top;
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
	_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:04ce
	this->arr_rect_1ec0.top = this->arr_rect_1eb8.top + this->var_i16_68c;
	this->arr_rect_1ec0.left = this->arr_rect_1eb8.left + this->var_i16_68a;
	this->arr_rect_1ec0.bottom = this->arr_rect_1eb8.bottom + this->var_i16_68c;
	this->arr_rect_1ec0.right = this->arr_rect_1eb8.right + this->var_i16_68a;

	_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:05a2
	if (this->arr_rect_1ec0.top < 0x14) {
		this->arr_rect_1ec0.top = 0x14;
		this->arr_rect_1ec0.bottom = 0x24;
	}
	// 141:05dc
	if (this->arr_rect_1ec0.left < 0) {
		this->arr_rect_1ec0.left = 0;
		this->arr_rect_1ec0.right = 0x18;
	}
	// 141:060c
	if (this->arr_rect_1ec0.bottom > SCREEN_HEIGHT) {
		this->arr_rect_1ec0.top = 0x146;
		this->arr_rect_1ec0.bottom = 0x156;
	}
	if (this->arr_rect_1ec0.right > SCREEN_WIDTH) {
		this->arr_rect_1ec0.left = 0x1e8;
		this->arr_rect_1ec0.right = 0x200;
	}
	_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
	// 141:06bc
	if (!((this->arr_rect_1eb8.top == this->arr_rect_1ec0.top) &&
		((this->arr_rect_1eb8.left == this->arr_rect_1ec0.left)))) {
		// 141:0722
		_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_bbbc, kSrcCopy);
		this->arr_rect_1eb8 = this->arr_rect_1ec0;
		_zbasic->get(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_rect_1eb8.right, this->arr_rect_1eb8.bottom, this->arr_bmp_bbbc);
		_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kSrcCopy);
		this->sub_128_3da(1);
	}
	// 141:0880
}

void FoolGame::deathDrawWhiteEye() {
	// 141:0882
	if (this->var_i16_233a != 0) {
		_zbasic->put(0x6c, 0x127, this->arr_bmp_c38c, kSrcCopy);
	}
	// 141:08b0
	this->var_i16_233a = 0;
}

void FoolGame::deathCaught() {
	// 141:08b8
	this->deathDrawWhiteEye();
	this->sub_128_4da(0);
	this->zoomRect(this->arr_rect_1eb8.top, this->arr_rect_1eb8.left, this->arr_rect_1eb8.bottom, this->arr_rect_1eb8.right, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x19);
	this->sub_128_4da(1);
}

void FoolGame::deathDrawZoom() {
	// 141:0934
	for (int i = 1; i <= 2; i++) {
		this->zoomRect(0x130, 0x76, 0x130, 0x76, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, kNotPatXor, 0x1a);
	}
	this->sub_128_69c(1, kNotPatXor, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
}


}
