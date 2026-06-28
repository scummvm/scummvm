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

#define OFF(x) (_zstrOffset[kOffsetSentence] + (x))



// sentence join game
void FoolGame::sentenceRun() {
	// 135:0004
	this->fetchPuzzleData();
	this->var_i16_c00 = 1;
	this->arr_i16_3738[0] = 0xbf;
	this->arr_i16_3738[2] = 0xe1;
	this->arr_i16_3738[4] = 0xbf;
	this->arr_i16_3738[5] = 0;
	this->arr_i16_3738[6] = 0xe1;
	this->arr_i16_3738[7] = SCREEN_WIDTH;
	this->arr_i16_3738[8] = 0xf2;
	this->arr_i16_3738[0xa] = SCREEN_HEIGHT;
	this->arr_i16_3738[0xc] = MENU_HEIGHT;
	this->arr_i16_3738[0xd] = 0;
	this->arr_i16_3738[0xe] = SCREEN_HEIGHT;
	this->arr_i16_3738[0xf] = SCREEN_WIDTH;

	// 135:00ce
	this->arr_i32_192c0[0] = _toolbox->GetPicture(puzzlesReadShort());
	this->arr_i16_1eb8[0] = puzzlesReadShort();
	_zbasic->indexRawSet(puzzlesReadString(), 1, 0);
	_sentenceGoal = puzzlesReadString().decode(Common::kMacRoman);
	Common::U32String menuStr = puzzlesReadString().decode(Common::kMacRoman);
	menuStr = _zbasic->str(OFF(0)) + menuStr + _zbasic->str(OFF(1)); // to reveal XXXXX
	// 135:0152
	_zbasic->menu(8, 7, 1, menuStr);
	this->arr_i16_1eb8[1] = 0x1c2 / this->arr_i16_1eb8[0];
	this->arr_i16_1eb8[2] = (SCREEN_WIDTH/2) - ((this->arr_i16_1eb8[0] * this->arr_i16_1eb8[1]) / 2);

	// 135:01e2
	int16 buttonGridX = this->arr_i16_1eb8[2];
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		this->arr_i16_4338[i] = 0;
		this->arr_i16_4338[i + this->arr_i16_1eb8[0]] = puzzlesReadByte();
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		// 135:0254
		_zbasic->indexRawSet(puzzlesReadString(), 1, i + this->arr_i16_1eb8[0]);
		_screenGrid[i].top = 0xf2;
		_screenGrid[i].left = buttonGridX;
		_screenGrid[i].bottom = SCREEN_HEIGHT;
		_screenGrid[i].right = buttonGridX + this->arr_i16_1eb8[1];
		// 135:030c
		buttonGridX += this->arr_i16_1eb8[1];
	}
	// 135:0348
	_zbasic->picture(0, MENU_HEIGHT, this->arr_i32_192c0[0]);
	_toolbox->ReleaseResource(this->arr_i32_192c0[0]);
	Common::Rect temp(this->arr_i16_3738[5], this->arr_i16_3738[4], this->arr_i16_3738[7], this->arr_i16_3738[6]);
	_toolbox->FillRect(temp, _patterns[2]);
	this->arr_i16_1eb8[4] = 0;
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		// 135:03a4
		if (_activePuzzleBuffer.empty()) { // was: str(OFF(2))
			this->arr_i16_4338[i] = 1;
		} else {
			// 135:03d4
			this->arr_i16_4338[i] = _zbasic->castInt(_zbasic->midStr(_activePuzzleBuffer, i, 1));

		}
		// 135:0408
		if (this->arr_i16_4338[i] == 1) {
			this->playTone(_zbasic->rndInt(1000) + 0x19, 0x28, true);
			this->sentenceDrawButton(i);
		}
		// 135:044a
	}
	// 135:0466
	if (_activePuzzleBuffer.empty()) { // was: str(OFF(3))
		_sentenceBuffer = _zbasic->index(1, 0);
	} else {
		// 135:049a
		this->var_i16_1372 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, this->arr_i16_1eb8[0]+1, 2));
		_sentenceBuffer = _zbasic->midStr(_activePuzzleBuffer, this->arr_i16_1eb8[0]+3, this->var_i16_1372).decode(Common::kMacRoman);
	}
	// 135:04f2
	this->sentenceDrawBuffer();
	_stateFlags = kStateNull;
	_activePuzzleSolved = _sentenceBuffer == _sentenceGoal;
	// 135:0520
	do {
		this->getNextEvent(-1);
		if (_event.what == kMouseDown) {
			this->sentenceOnClick();
		}
		// 135:0538
		if (_sentenceBuffer == _sentenceGoal) {
			_activePuzzleSolved = true;
		}
		// 135:0552
		if (_stateFlags == kStateUndo) {
			sentenceUndo();
			_stateFlags = kStateNull;
		}
		// 135:0566
		if (_stateFlags == kStateSaveGame) {
			sentenceStoreState();
			this->saveGame();
		}
		// 135:0578
	} while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved));
	// 135:05a0
	if (_activePuzzleSolved) {
		sentenceSuccess();
	}
	sentenceStoreState();
	// JMP 0xd9c
}

void FoolGame::sentenceOnClick() {
	// 135:05b6
	this->var_i16_1abc = 0;
	if ((_event.where.x < this->arr_i16_1eb8[2]) || (_event.where.y < 0xf2))
		return;

	// determine which button was clicked on
	int16 index = ((_event.where.x - this->arr_i16_1eb8[2]) / this->arr_i16_1eb8[1]) + 1;
	// 135:0632
	if ((index > this->arr_i16_1eb8[0]) || (this->arr_i16_4338[index] == 0))
		return;
	// 135:0676
	this->arr_i16_4338[index] = 0;
	_toolbox->FillOval(_screenGrid[index], _patterns[2]);
	// 135:06aa
	switch (this->arr_i16_4338[index + this->arr_i16_1eb8[0]]-1) {
	case 0:
		sentenceAddLeft(index);
		break;
	case 1:
		sentenceAddRight(index);
		break;
	case 2:
		sentenceAddLeftRight(index);
		break;
	case 3:
		sentenceReplace(index);
		break;
	case 4:
		sentenceReverse();
		break;
	default:
		warning("sentenceOnClick: breaking out of switch");
		break;
	}
	this->sentenceDrawBuffer();
	this->waitForMouseUp();
	if (_sentenceBuffer == _sentenceGoal) {
		return;
	}
	// 135:0714
	bool buttonsRemaining = false;
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		if (this->arr_i16_4338[i] != 0) {
			buttonsRemaining = true;
		}
	}
	// 135:0758
	if (buttonsRemaining) {
		return;
	}
	_zbasic->text(kFontChicago, 0xc, 0, kSrcBic);
	drawTextCenter(_zbasic->str(OFF(4)), 0x124); // click mouse to reset puzzle

	this->waitForMouseUp();
	this->var_i16_1abc = 1;
	_stateFlags = kNullEvent;
	// 135:07a0
	while ((_event.modifiers & kModMouseButtonUp) && (_stateFlags == kNullEvent)) {
		this->getNextEvent(-1);
	}
	// 135:07c8
	if (_stateFlags == kNullEvent) {
		sentenceUndo();
	}
}

void FoolGame::sentenceAddLeft(int16 index) {
	// 135:07d6
	_sentenceBuffer = _zbasic->index(1, index) + _sentenceBuffer;
}

void FoolGame::sentenceAddRight(int16 index) {
	// 135:07fa
	_sentenceBuffer += _zbasic->index(1, index);
}

void FoolGame::sentenceAddLeftRight(int16 index) {
	// 135:0813
	_sentenceBuffer = _zbasic->index(1, index)
		+ _sentenceBuffer
		+ _zbasic->index(1, index + this->arr_i16_1eb8[0]);
}

void FoolGame::sentenceReplace(int16 index) {
	// 135:086a
	this->var_i16_9f2 = _zbasic->index(1, index).size();
	while (true) {
		// 135:0884
		int16 length = _sentenceBuffer.size();
		this->var_i16_1abe = _zbasic->instr(1, _sentenceBuffer, _zbasic->index(1, index));
		if (this->var_i16_1abe == 0)
			return;
		// 135:08bc
		if ((length - this->var_i16_1abe + 1 - this->var_i16_9f2) < 0)
			return;
		_sentenceBuffer = _zbasic->leftStr(_sentenceBuffer, this->var_i16_1abe - 1)
			+ _zbasic->index(1, index + this->arr_i16_1eb8[0])
			+ _zbasic->rightStr(_sentenceBuffer, length - this->var_i16_1abe + 1 - this->var_i16_9f2);
	}
	// should never reach here
}

void FoolGame::sentenceReverse() {
	// 135:094c
	Common::U32String buffer; // was: str(OFF(5))
	for (int16 i = (int16)_sentenceBuffer.size(); i >= 1; i--) {
		buffer += _zbasic->midStr(_sentenceBuffer, i, 1);
	}
	_sentenceBuffer = buffer;
}

void FoolGame::sentenceUndo() {
	// 135:09ba
	this->arr_i16_1eb8[4] = 0;
	Common::Rect temp(this->arr_i16_3738[5], this->arr_i16_3738[4], this->arr_i16_3738[7], this->arr_i16_3738[6]);
	_toolbox->FillRect(temp, _patterns[2]);
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		this->arr_i16_4338[i] = 1;
		this->sentenceDrawButton(i);
	}
	_sentenceBuffer = _zbasic->index(1, 0);
	this->sentenceDrawBuffer();
}

void FoolGame::sentenceDrawButton(int16 gridIndex) {
	// 135:0a34
	_zbasic->text(kFontChicago, 0xc, 0, kSrcOr);
	Common::U32String buf = Common::U32String::format("%d", gridIndex) + _zbasic->str(OFF(6)); // ' '
	int16 width = _toolbox->StringWidth(buf);
	_toolbox->FillOval(_screenGrid[gridIndex], _patterns[0]);
	_toolbox->FrameOval(_screenGrid[gridIndex]);
	_toolbox->MoveTo(_screenGrid[gridIndex].left + (this->arr_i16_1eb8[1] / 2) - (width / 2), 0x129);
	_toolbox->DrawString(buf);
}

void FoolGame::sentenceDrawBuffer() {
	// 135:0b16
	_zbasic->text(kFontLarge, 0x18, 0, kSrcOr);
	int16 width = _toolbox->StringWidth(_sentenceBuffer);
	if (width < this->arr_i16_1eb8[4]) {
		width = this->arr_i16_1eb8[4];
	} else {
		this->arr_i16_1eb8[4] = width;
	}
	// 135:0b80
	this->arr_i16_3738[1] = 0xf6 - (width / 2);
	this->arr_i16_3738[3] = 0x109 + (width / 2);
	Common::Rect temp(this->arr_i16_3738[1], this->arr_i16_3738[0], this->arr_i16_3738[3], this->arr_i16_3738[2]);
	_toolbox->FillRect(temp, _patterns[0]);
	_toolbox->MoveTo(0x100 - (width / 2), 0xd9);
	_toolbox->DrawString(_sentenceBuffer);
}

void FoolGame::sentenceStoreState() {
	// 135:0c1c
	_activePuzzleBuffer.clear(); // was: str(OFF(7))
	if (this->var_i16_1abc != 0) {
		this->var_i16_1abc = 0;
		return;
	}
	// 135:0c44
	for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
		Common::U32String buf = Common::U32String::format("%d", this->arr_i16_4338[i]);
		buf = _zbasic->rightStr(buf, 1);
		_activePuzzleBuffer += buf;
	}
	// 135:0cba
	Common::String temp = _sentenceBuffer.encode(Common::kMacRoman);
	this->var_i16_1372 = (int16)temp.size();
	_activePuzzleBuffer += _zbasic->encodeInt(this->var_i16_1372) + temp;
}

void FoolGame::sentenceSuccess() {
	// 135:0cee
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
		for (int16 i = 1; i <= this->arr_i16_1eb8[0]; i++) {
			this->playTone(_zbasic->rndInt(1000) + 0x19, 0x28, false);
		}
	}
	// 135:0d40
	this->menuClickMessage();
	this->waitForMouseUp();
	this->flashRect(this->arr_i16_3738[0], this->arr_i16_3738[1], this->arr_i16_3738[2], this->arr_i16_3738[3], 0x64);
}


}
