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

#define OFF(x) (_zstrOffset[kOffsetReveal] + (x))

extern Toolbox *g_toolbox;

// mask reveal puzzle
void FoolGame::revealRun() {
	// 134:0004
	this->fetchPuzzleData();
	this->var_i16_c00 = 1;
	this->arr_i16_1eb8[15] = puzzlesReadShort();
	this->arr_i16_1eb8[16] = puzzlesReadShort();
	this->arr_i16_1eb8[21] = puzzlesReadShort();
	debugC(5, kDebugLoading, "%d, %d, %04x", this->arr_i16_1eb8[15], this->arr_i16_1eb8[16], this->arr_i16_1eb8[21]);
	this->var_str_384 = puzzlesReadString();
	this->var_str_384 = _zbasic->str(OFF(0)) + this->var_str_384 + _zbasic->str(OFF(1));
	_zbasic->menu(8, 6, 1, this->var_str_384);
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		this->arr_i16_3738[i] = puzzlesReadByte();
	}
	// 134:00ce
	// rack up polygon information
	g_toolbox->SetPort(this->var_i32_f24);
	this->arr_i16_1eb8[20] = 0;

	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		this->var_i16_103a = puzzlesReadShort();
		_revealPoly[i] = g_toolbox->OpenPoly();
		// 134:010e
		for (int j = 5; j <= this->var_i16_103a - 1; j += 2) {
			this->var_i16_484 = puzzlesReadShort();
			this->var_i16_7e4 = puzzlesReadShort();
			if (j == 5) {
				g_toolbox->MoveTo(this->var_i16_7e4, this->var_i16_484 + 0xf);
			} else {
				g_toolbox->LineTo(this->var_i16_7e4, this->var_i16_484 + 0xf);
			}
			// 134:015c
		}
		g_toolbox->ClosePoly();
		if (i <= this->arr_i16_1eb8[16]) {
			this->arr_i16_1eb8[20] |= _bitLUT[i - 1];
		}
		// 134:01ce
	}
	// 134:01ea
	g_toolbox->SetPort(this->var_i32_0);

	this->arr_i16_1eb8[17] = (0x1c2 / this->arr_i16_1eb8[15]);
	this->arr_i16_1eb8[18] = 0x100 - ((this->arr_i16_1eb8[15] * this->arr_i16_1eb8[17]) / 2);
	this->var_str_1272 = puzzlesReadString();
	this->zoomRect(0xb4, 0xff, 0xb6, 0x101, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
	this->zoomRect(0x9a, 0xff, 0x9c, 0x101, 0x37, 0x13, 0xff, 0x1ee, 0, kPatCopy, 0x19);
	// 134:02f0
	this->arr_i16_1eb8[19] = _zbasic->decodeInt(_activePuzzleBuffer);
	this->var_i16_1aba = this->arr_i16_1eb8[18];
	_zbasic->text(_fontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	for (this->var_i16_7be = 1; this->var_i16_7be <= this->arr_i16_1eb8[15]; this->var_i16_7be++) {
		// play a random tone and draw each of the letter buttons in sequence
		this->playTone(
			_zbasic->rndInt(0x3e8) + 0x19,
			0x28,
			1
		);
		// 134:0352
		_screenGrid[this->var_i16_7be].top = 0x10f;
		_screenGrid[this->var_i16_7be].left = this->var_i16_1aba;
		_screenGrid[this->var_i16_7be].bottom = 0x137;
		_screenGrid[this->var_i16_7be].right = this->var_i16_1aba + this->arr_i16_1eb8[17];

		this->var_i16_1aba += this->arr_i16_1eb8[17];
		// 134:03fc
		g_toolbox->FillRoundRect(_screenGrid[this->var_i16_7be], 0x19, 0x19, _patterns[0]);
		g_toolbox->FrameRoundRect(_screenGrid[this->var_i16_7be], 0x19, 0x19);
		// 134:0448
		this->var_str_384 = _zbasic->midStr(this->var_str_1272, this->var_i16_7be, 1);
		this->var_i16_7ba = g_toolbox->StringWidth(this->var_str_384);
		g_toolbox->MoveTo(
			_screenGrid[this->var_i16_7be].left + (this->arr_i16_1eb8[17] / 2) - (this->var_i16_7ba / 2),
			0x127
		);
		g_toolbox->DrawString(this->var_str_384);
		// 134:04d6
		if (this->arr_i16_1eb8[19] & _bitLUT[this->arr_i16_3738[this->var_i16_7be] - 1]) {
			this->revealSelectButton();
		}
	}
	// 134:0538
	_stateFlags = kStateNull;
	if (this->arr_i16_1eb8[20] == this->arr_i16_1eb8[19]) {
		_activePuzzleSolved = true;
	} else {
		// 134:0572
		_activePuzzleSolved = false;
	}
	g_toolbox->PenNormal();
	// 134:057a
	// 134:057a: JMP - [0x61e]
	// main event loop
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 134:057e
		while ((_stateFlags == 0) && (!_activePuzzleSolved)) {
			// 134:0582
			this->getNextEvent(-1);
			if (_event.what == kMouseDown) {
				this->revealOnClick();
			}
			if (this->arr_i16_1eb8[20] == this->arr_i16_1eb8[19]) {
				_activePuzzleSolved = true;
			}
		}
		// 134:05e2
		if (_stateFlags == kStateUndo) {
			this->revealReset();
		}
		if (_stateFlags == kStateSaveGame) {
			_activePuzzleBuffer = _zbasic->encodeInt(this->arr_i16_1eb8[19]);
			this->saveGame();
		}
		// 134:061e
	}
	// 134:0648
	if (_activePuzzleSolved) {
		this->revealSuccess();
	}
	_activePuzzleBuffer = _zbasic->encodeInt(this->arr_i16_1eb8[19]);
	// 134:0678: JMP - [0x8ae]
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		g_toolbox->KillPoly(_revealPoly[i]);
	}
}

void FoolGame::revealOnClick() {
	// 134:067c
	this->var_i16_7be = 0;
	for (int j = 1; j <= this->arr_i16_1eb8[15]; j++) {
		if (g_toolbox->PtInRect(_event.where, _screenGrid[j])) {
			this->var_i16_7be = j;
			j = this->arr_i16_1eb8[15];
		}
	}
	// 134:06d0
	if (this->var_i16_7be == 0) {
		return;
	}
	// 134:06f4
	this->revealSelectButton();
	this->arr_i16_1eb8[19] ^= _bitLUT[this->arr_i16_3738[this->var_i16_7be]-1];
	debugC(5, kDebugLoading, "reveal: %d, %04x, %04x", this->var_i16_7be, this->arr_i16_1eb8[19], this->arr_i16_1eb8[20]);
	this->waitForMouseUp();
}

void FoolGame::revealSelectButton() {
	// 134:074a
	// invert the button
	g_toolbox->InvertRoundRect(_screenGrid[this->var_i16_7be], 0x19, 0x19);
	g_toolbox->PenPat(_patterns[this->arr_i16_1eb8[21]]);
	// fill the polygon
	g_toolbox->PenMode(kPatXor);
	g_toolbox->PaintPoly(_revealPoly[this->arr_i16_3738[this->var_i16_7be]]);
	g_toolbox->PenNormal();
}

void FoolGame::revealReset() {
	// 134:07bc
	this->fillRect(0x37, 0x13, 0xff, 0x1ee, 0);
	for (int i = 1; i <= this->arr_i16_1eb8[15]; i++) {
		if (this->arr_i16_1eb8[19] & _bitLUT[this->arr_i16_3738[i]-1]) {
			g_toolbox->InvertRoundRect(_screenGrid[i], 0x19, 0x19);
		}
	}
	// 134:085c
	this->arr_i16_1eb8[19] = 0;
	_stateFlags = kStateNull;
}

void FoolGame::revealSuccess() {
	// 134:0872
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	this->menuClickMessage();
	this->waitForMouseUp();
	this->flashRect(0x37, 0x13, 0xff, 0x1ee, 0xc8);
}

}
