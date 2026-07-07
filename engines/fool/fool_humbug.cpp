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

#include "graphics/mactoolbox/toolbox.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/zbasic.h"

namespace Fool {

#define OFF(x) (_zstrOffset[kOffsetHumbug] + (x))



// the humbug - "irritating stick" style challenge
void FoolGame::humbugRun() {
	// 142:0004
	if (_activePuzzleStatus < 0x63) {
		humbugPlayTrail();
		if (!_activePuzzleSolved) {
			return;
		}
		humbugSuccess();
		if (_puzzleCompletionStatus[0x34] > 2) {
			_activePuzzleStatus = 0x64;
			return;
		}
		// 142:004e
		_activePuzzleStatus = 0x63;
		zoomRect(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, Graphics::MacToolbox::kPatCopy, 0x19);
		// behold the 2nd key of thoth
		showBehold(0x17, 0, _zbasic->str(OFF(0)));
	}
	// 142:00b2
	if (_activePuzzleStatus == 0x63) {
		humbugShowThoth();
	}
	if (_activePuzzleStatus >= 0x64) {
		humbugPlayTrail();
		if (_activePuzzleSolved) {
			humbugSuccess();
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
	fetchPuzzleData();
	toggleMouseCursor(false);
	_zbasic->text(kFontChicago, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	drawPuzzleButton(_zbasic->str(OFF(5))); // ~
	// eye button
	_zbasic->get(0x6c, 0x127, 0x84, 0x137, arr_bmp_b3ec);
	for (int16 i = 1; i <= 0xa42; i++) {
		_humbugTrail[i].x = puzzlesReadShort();
		_humbugTrail[i].y = puzzlesReadShort();
		_zbasic->put(
			_humbugTrail[i].x,
			_humbugTrail[i].y,
			arr_bmp_b3ec,
			Graphics::MacToolbox::kSrcCopy
		);
		// Simulate slow draw speed
		if ((i % 42) == 0)
			_toolbox->Delay(0);
	}
	// 142:026e
	toggleMouseCursor(true);
	_zbasic->put(
		_humbugTrail[_humbugTrailIndex].x,
		_humbugTrail[_humbugTrailIndex].y,
		arr_bmp_b3ec,
		Graphics::MacToolbox::kSrcCopy
	);
	_humbugTrailIndex = 1;
	_stateFlags = kStateNull;
	_activePuzzleSolved = false;
	var_i16_1ab6 = 0;
	var_i16_1ab8 = 0;

	while (((_stateFlags & kStateReturn) == 0) && !_activePuzzleSolved) {
		// 142:02ee
		getNextEvent(-1);
		if (!((var_i16_1ab6 == _event.where.x) && (var_i16_1ab8 == _event.where.y))) {
			humbugMoveMouse();
		}
		if (_humbugTrailIndex >= 0xa42) {
			_activePuzzleSolved = true;
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
		// 142:0346
	}
}

void FoolGame::humbugMoveMouse() {
	// 142:0370
	int16 trailMax = _humbugTrailIndex + 0x19;
	if (trailMax > 0xa42) {
		trailMax = 0xa42;
	}
	int16 trailMin = _humbugTrailIndex - 0x19;
	if (trailMin < 1) {
		trailMin = 1;
	}
	var_i16_1a9c = 0;
	for (int16 i = trailMax; i >= trailMin; i--) {
		int16 dx = _event.where.x - _humbugTrail[i].x;
		int16 dy = _event.where.y - _humbugTrail[i].y;
		if ((dx >= -5) && (dx <= 0x1e) && (dy >= -5) && (dy <= 0x19)) {
			var_i16_1a9c = i + 0xa;
		}
		// 142:0470
	}
	// 142:0484
	if (var_i16_1a9c >= 0xa42) {
		var_i16_1a9c = 0xa42;
	}
	if (var_i16_1a9c != 0) {
		int16 incr = (_humbugTrailIndex < var_i16_1a9c) ? 1 : -1;
		int16 index = _humbugTrailIndex;
		do {
			_zbasic->put(
				_humbugTrail[index].x,
				_humbugTrail[index].y,
				arr_bmp_b3ec,
				Graphics::MacToolbox::kSrcCopy
			);
			// 142:051c
		} while (_zbasic->incrAndCheck(index, var_i16_1a9c, incr));
		_humbugTrailIndex = var_i16_1a9c;
		var_i16_1ab6 = _event.where.x;
		var_i16_1ab8 = _event.where.y;
	} else {
		// 142:054a
		int16 rewindTo = _humbugTrailIndex - 0xa;
		if (rewindTo < 1) {
			rewindTo = 1;
		}
		for (int16 i = _humbugTrailIndex; i >= rewindTo; i--) {
			_zbasic->put(
				_humbugTrail[i].x,
				_humbugTrail[i].y,
				arr_bmp_b3ec,
				Graphics::MacToolbox::kSrcCopy
			);
			// 142:05c8
		}
		_humbugTrailIndex -= 0xa;
		if (_humbugTrailIndex < 1) {
			_humbugTrailIndex = 1;
		}
	}
	// 142:05f0
}

void FoolGame::humbugSuccess() {
	// 142:05f2
	zoomRect(0x137, 0xc6, 0x147, 0xdd, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1, Graphics::MacToolbox::kNotPatXor, 0x19);
}

void FoolGame::humbugShowThoth() {
	// 142:0630
	_zbasic->menu(8, 0, 1, _zbasic->str(OFF(6))); // the 2nd key of thoth
	_zbasic->menu(8, 1, 1, _zbasic->str(OFF(7))); // return to scroll
	thothKey2nd();
	if (_activePuzzleSolved) {
		// 142:066e
		_toolbox->PenNormal();
		_zbasic->text(kFontLarge, 0x18, 0x19, Graphics::MacToolbox::kSrcBic);
		for (int16 i = 1; i <= 4; i++) {
			Common::Rect temp;
			temp.top = arr_i16_2f38[i*32];
			temp.left = arr_i16_2f38[i*32+1];
			temp.bottom = arr_i16_2f38[i*32+2];
			temp.right = arr_i16_2f38[i*32+3];
			_toolbox->FillRect(temp, _patterns[2]);
			_toolbox->FrameRect(temp);
			Common::U32String idStr = Common::U32String::format(" %d ", i);
			int16 width = _toolbox->StringWidth(idStr);
			int16 xOffset = (arr_i16_2f38[i*32+3] - arr_i16_2f38[i*32+1])/2;
			// 142:0756
			_toolbox->MoveTo(
				arr_i16_2f38[i*32+1] + xOffset - (width / 2),
				arr_i16_2f38[i*32 + 2] - 0x1e
			);
			_toolbox->DrawString(idStr);
		}
		// 142:07ca
		_zbasic->text(kFontChicago, 0xc, 0, Graphics::MacToolbox::kSrcOr);
		waitForMouseUp();
		menuClickMessage();

		int16 tickCounter = 0;
		int16 idCounter = 0;
		while (_event.modifiers & Graphics::MacToolbox::kModMouseButtonUp) {
			// 142:07f8
			getNextEvent(-1); // was: 0
			tickCounter++;
			if (tickCounter == 0x64) {
				tickCounter = 0;
				idCounter++;
				if (idCounter > 4) {
					idCounter = 1;
				}
				Common::Rect temp;
				temp.top = arr_i16_2f38[idCounter*32];
				temp.left = arr_i16_2f38[idCounter*32+1];
				temp.bottom = arr_i16_2f38[idCounter*32+2];
				temp.right = arr_i16_2f38[idCounter*32+3];
				_toolbox->InvertRect(temp);
			}
		}
	}
	// 142:0850
}

} // End of namespace Fool
