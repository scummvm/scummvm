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

#define OFF(x) (_zstrOffset[kOffsetCards] + (x))



// v1.1 and v2.0 have the winning score set to 700.
// v3.0 revises this to be 666, which I can't dispute is a much cooler score.
#define CARDS_MAX_SCORE 666
#define CARDS_MIN_SCORE (-999)
#define CARDS_YIELD_PENALTY 27

// wheel of fortune card game
void FoolGame::cardsRun() {
	// missing zeroed state
	var_i16_2016 = 0;

	// 139:0004
	fetchPuzzleData();
	var_i16_c00 = 1;
	// set up card bounding boxes
	_toolbox->SetRect(_screenGrid[1], 0x78, 0x19, 0xd1, 0xae);
	_toolbox->SetRect(_screenGrid[2], 0xd3, 0x19, 0x12c, 0xae);
	_toolbox->SetRect(_screenGrid[3], 0x12e, 0x19, 0x187, 0xae);
	_toolbox->SetRect(_screenGrid[4],1, 0x2a, 0x5a, 0xbf);
	_toolbox->SetRect(_screenGrid[5],0x1a6, 0x2a, 0x1ff, 0xbf);
	_toolbox->SetRect(_screenGrid[6], 0x1, 0xc0, 0x5a, 0x155);
	_toolbox->SetRect(_screenGrid[7],0x1a6, 0xc0, 0x1ff, 0x155);
	_toolbox->SetRect(_screenGrid[8],0x5b, 0xc0, 0xb4, 0x155);
	_toolbox->SetRect(_screenGrid[9],0x14c, 0xc0, 0x1a5, 0x155);

	// 139:011c
	for (int i = 0; i <= 0x25; i++) {
		_zbasic->indexSet(puzzlesReadString(), 1, i);
	}

	var_i16_484 = 0x5e;
	for (int i = 0x17; i >= 0; i--) {
		var_i16_484++;
		arr_i32_192c0[i] = _toolbox->GetPicture(var_i16_484);
	}
	// 139:0184
	sub_128_bde(1, 1, 0, 1, 1, 1);
	fillRect(0xb4, 0xb5, 0x156, 0x14b, 2);
	// illustration of the card game
	_zbasic->picture(0xc3, 0xf3, arr_i32_192c0[0x17]);
	if (_activePuzzleBuffer.empty()) {
		// 139:01fa
		_activePuzzleBuffer = _zbasic->encodeInt(0) + _zbasic->encodeInt(0) + _zbasic->encodeInt(0);
		// card IDs
		for (int16 i = 0; i <= 0x15; i++) {
			arr_i16_5cbc[i] = i;
		}
		// 139:0244
		for (int i = 0; i <= 0x15; i++) {
			var_i16_484 = _zbasic->rndInt(0x16) - 1;
			var_i16_7e4 = _zbasic->rndInt(0x16) - 1;
			_zbasic->swapInt(arr_i16_5cbc[var_i16_484], arr_i16_5cbc[var_i16_7e4]);
		}
		// 139:029c
		for (int i = 0; i <= 0x15; i++) {
			_activePuzzleBuffer += _zbasic->encodeInt(arr_i16_5cbc[i]);
		}
	}
	// 139:02d8
	// player scores
	arr_i16_1eb8[5] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 1, 2));
	arr_i16_1eb8[6] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 3, 2));
	// 139:0320
	var_i16_2010 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 5, 2));
	if ((arr_i16_1eb8[5] > CARDS_MAX_SCORE) || (arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
		arr_i16_1eb8[5] = 0;
		arr_i16_1eb8[6] = 0;
		var_i16_2010 = 0;
	}
	// 139:039e

	for (int i = 0; i <= 0x15; i++) {
		arr_i16_5cbc[i] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, i*2 + 7, 2));
	}
	// 139:03ea
	arr_i16_5cbc[0x16] = 0x16;
	arr_rect_4338.top = 0xdb;
	arr_rect_4338.left = 0xe2;
	arr_rect_4338.bottom = 0xee;
	arr_rect_4338.right = 0x11e;
	var_i16_2012 = 0;
	_stateFlags = 0;
	cardsDrawScores();
	if (var_i16_2010 == 0) {
		cardsShuffleDeck();
	}
	// 139:0456
	cardsDrawTable();
	while ((_stateFlags & kStateReturn) == 0) {
		// 139:045e
		var_i16_2014 = 0;
		_stateFlags = 0;
		while (_stateFlags == 0) {
			// 139:046e
			getNextEvent(-1);
			if ((_event.modifiers & Graphics::MacToolbox::kModMouseButtonUp) == 0) {
				cardsOnClick();
			}
			if (var_i16_2014 != 0) {
				sub_139_50e();
			}
		}
		// 139:04a4
		if (_stateFlags == kStateUndo) {
			cardsReset();
		}
		if (_stateFlags == kStateSaveGame) {
			cardsStoreState();
			saveGame();
		}
		// 139:04c4
	}
	// 139:04d6
	cardsStoreState();
	// JMP 1d7e
	// 139:1d7e
	for (int i = 0; i <= 0x17; i++) {
		_toolbox->ReleaseResource(arr_i32_192c0[i]);
	}
}

void FoolGame::cardsReset() {
	arr_i16_1eb8[5] = 0;
	arr_i16_1eb8[6] = 0;
	var_i16_2012 = 0;
	cardsDrawScores();
	cardsShuffleDeck();
	cardsDrawTable();
}

void FoolGame::sub_139_50e() {
	var_i16_2014 = 0;
	cardsShuffleDeck();
	cardsDrawTable();
}

void FoolGame::cardsShuffleDeck() {
	// 139:051e
	_zbasic->unk_20();
	for (int j = 0; j <= 2; j++) {
		for (int i = 0; i <= 0x15; i++) {
			var_i16_484 = _zbasic->rndInt(0x16)-1;
			_zbasic->swapInt(arr_i16_5cbc[var_i16_484], arr_i16_5cbc[i]);
		}
	}
}

void FoolGame::cardsDrawTable() {
	sub_139_17fc();
	drawTarotCard(5, 0, 0);
	drawTarotCard(4, 1, 0);
	drawTarotCard(7, 2, 0);
	drawTarotCard(6, 0x16, 0);
	drawTarotCard(1, 4, 0);
	drawTarotCard(2, 5, 0);
	drawTarotCard(3, 6, 0);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	// 139:061a
	drawTextCenter(_zbasic->str(OFF(0)), 0xc3); // Select a card above
	drawTextCenter(_zbasic->str(OFF(1)), 0xd4); // or
	_toolbox->FillRoundRect(arr_rect_4338, 0xa, 0xa, _patterns[0]);
	_toolbox->FrameRoundRect(arr_rect_4338, 0xa, 0xa);
	_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);
	drawTextCenter(_zbasic->str(OFF(2)), 0xe9); // Yield
	var_i16_2016 = 1;
	var_i16_2010 = 1;
	waitForMouseUp();
}

void FoolGame::cardsOnClick() {
	// 139:06b0
	var_i16_1e00 = 0;
	if (!((_event.where.x >= 0x78) && (_event.where.y >= 0x19) && (_event.where.x <= 0x187) && (_event.where.y <= 0xae))) {
		// 139:06fc
		// didn't click in the three cards area
		if (_toolbox->PtInRect(_event.where, arr_rect_4338)) {
			_toolbox->InvertRoundRect(arr_rect_4338, 0xa, 0xa);
			waitForMouseUp();
			_toolbox->InvertRoundRect(arr_rect_4338, 0xa, 0xa);
			if (!_toolbox->PtInRect(_event.where, arr_rect_4338))
				return;
			// 139:0778
			// player clicked the yield button (bad choice)
			playTone(0x19, 0x64, 0x0);
			var_i16_2012 = -CARDS_YIELD_PENALTY;
			cardsDrawScores();
			var_i16_2014 = 1;
		} else {
			// 139:079e
			playTone(0xf, 0x64, 0x1);
			for (int j = 0; j <= 1; j++) {
				for (int16 i = 0; i <= 0x3; i++) {
					drawTarotCard(i, 0, 3);
				}
			}

		}
		// 139:07e6
	} else {
		// 139:07ea
		// player picked a card
		fillRect(0xb4, 0xb5, 0xf3, 0x14b, 2);
		if (_event.where.x < 0xd2) {
			// 139:0818
			var_i16_1e00 = 4;
			drawTarotCard(1, 0, 1);
		}
		// 139:082e
		if ((_event.where.x >= 0xd2) && (_event.where.x <= 0x12d)) {
			var_i16_1e00 = 5;
			drawTarotCard(2, 0, 1);
		}
		// 139:0868
		if (_event.where.x > 0x12d) {
			var_i16_1e00 = 6;
			drawTarotCard(3, 0, 1);
		}
		// 139:088a
		drawTarotCard(9, var_i16_1e00, 0);
		_zbasic->indexSet(
			_zbasic->chr(arr_i16_5cbc[0]+0x41) + _zbasic->chr(arr_i16_5cbc[2]+0x41) + _zbasic->chr(arr_i16_5cbc[var_i16_1e00]+0x41),
			1,
			0x26
		);
		sub_139_19da();
		sub_139_191c();
		sub_139_a22();
		int16 strY = 0;
		if (var_i16_2018 == 0) {
			// 139:0920
			cardsRevealHands();
			strY = 0xc3;

		} else {
			// 139:092e
			cardsOpponentYields();
			strY = 0xd7;
		}
		// 139:0938
		var_i16_2010 = 0;
		if (arr_i16_1eb8[5] > CARDS_MAX_SCORE) {
			sub_139_1d3e();
			return;
		}
		// 139:095e
		sub_128_61ec();
		// original code has the font ID as 1... maybe there's a fallback?
		// this was intended to be Small
		_zbasic->text(kFontSmall, 9, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
		drawTextCenter(_zbasic->str(OFF(3)), strY); // (click mouse to continue)
		var_i16_2014 = 0;
		_stateFlags = 0;
		while ((_stateFlags == 0) && (var_i16_2014 == 0)) {
			getNextEvent(-1);
			if (_event.what == Graphics::MacToolbox::kMouseUp) {
				var_i16_2014 = 1;
			}
			if (_stateFlags == kStateSaveGame) {
				cardsStoreState();
				saveGame();
			}
		}
		// 139:09d8
		if (((_stateFlags & kStateReturn) == 0) && (arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
			_stateFlags = kStateUndo;
			var_i16_2014 = 0;
		}
	}
	// 139:0a20
	return;
}

void FoolGame::sub_139_a22() {
	// 139:0a22
	var_str_201a = _zbasic->chr(arr_i16_5cbc[0] + 0x41) + _zbasic->chr(arr_i16_5cbc[var_i16_1e00] + 0x41);
	var_i16_211a = var_i16_1e00;
	_zbasic->indexSet(_zbasic->chr(arr_i16_5cbc[1] + 0x41) + _zbasic->chr(arr_i16_5cbc[3] + 0x41), 1, 0x27);
	var_str_211c = _zbasic->index(1, 0x27);
	var_i16_2016 = 0x2;
	var_i16_221c = 0;
	var_i16_221e = 0;
	for (int16 i = 4; i <= 6; i++) {
		// 139:0ae8
		if (i  != var_i16_211a) {
			_zbasic->indexSet(var_str_211c + _zbasic->chr(arr_i16_5cbc[i] + 0x41), 1, 0x27);
			sub_139_19da();
			if (arr_i16_1eb8[2] >= var_i16_221c) {
				// 139:0b52
				var_i16_221c = arr_i16_1eb8[2];
				var_i16_1e00 = i;
			} else {
				// 139:0b70
				if (arr_i16_1eb8[2] == var_i16_221c) {
					sub_139_191c();
					if (arr_i16_1eb8[4] > var_i16_221e) {
						var_i16_221e = arr_i16_1eb8[4];
						var_i16_1e00 = i;
					}
				}
			}
		}
		// 139:0bc4
	}
	_zbasic->indexSet(var_str_211c + _zbasic->chr(arr_i16_5cbc[var_i16_1e00] + 0x41), 1, 0x27);
	// 139:0c0a
	arr_i16_1eb8[2] = var_i16_221c;
	sub_139_191c();
	var_i16_221c = 0;
	// cycle backwards through the scoring combinations
	for (int16 j = 8; j >= 1; j--) {
		var_i16_7a8 = 0;
		// count the number of combination cards in the hand
		for (int16 i = 1; i <= (int16)_zbasic->index(1, j + 0x15).size(); i++) {
			if (_zbasic->instr(
					1,
					var_str_201a,
					_zbasic->midStr(_zbasic->index(1, j + 0x15), i, 1))
				) {
				var_i16_7a8++;
			}
			// 139:0c72
		}
		// 139:0c90
		if (var_i16_7a8 == 2) {
			var_i16_221c = j;
			j = 1;
		}
		// 139:0ca6
	}
	// 139:0cb6
	var_i16_2018 = 0;
	if (var_i16_221c > arr_i16_1eb8[2]) {
		var_i16_2018 = 1;
	}
	// 139:0ce2
	if (var_i16_221c == arr_i16_1eb8[2]) {
		for (int16 i = 1; i <= (int16)var_str_201a.size(); i++) {
			var_i16_484 = 0x15 - (_zbasic->asc(_zbasic->midStr(var_str_201a, i, 1)) - 0x41);
			if (var_i16_484 > arr_i16_1eb8[4]) {
				var_i16_2018 = 1;
			}
			// 139:0d5c
		}
	}
	// 139:0d6e
	if ((var_i16_2018 == 1) && ((var_i16_221c + 1)*9 <= CARDS_YIELD_PENALTY)) {
		var_i16_2018 = 0;
	}

	// 139:0da6
}

void FoolGame::cardsOpponentYields() {
	// 139:0da8
	// ol' mate gives up
	playTone(0x19, 0x64, 0x1);
	var_i16_2012 = -CARDS_YIELD_PENALTY;
	cardsDrawScores();
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	drawTextCenter(_zbasic->str(OFF(4)), 0xc4); // The Old Man yields
}

void FoolGame::cardsRevealHands() {
	// 139:0dee
	// remove the three cards from the top row
	drawTarotCard(1, 0, 1);
	drawTarotCard(2, 0, 1);
	drawTarotCard(3, 0, 1);

	// show ol' mate's draw from the pile
	drawTarotCard(8, var_i16_1e00, 0);

	_zbasic->indexSet(_zbasic->index(1, 0x27) + _zbasic->chr(arr_i16_5cbc[var_i16_1e00] + 0x41), 1, 0x27);
	// 139:0e70
	for (int16 i = 1; i <= 2; i++) {
		_zbasic->indexSet(Common::U32String(), 1, i + 0x27); // was: str(OFF(5))
		if ((arr_i16_1eb8[i] > 0) && (arr_i16_1eb8[i] < 9)) {
			// 139:0ed6
			_zbasic->indexSet(
				_zbasic->index(1, 0x1d + arr_i16_1eb8[i]) + _zbasic->str(OFF(6)), // X pair
				1,
				i + 0x27
			);
		}
		// 139:0f1e
		if (arr_i16_1eb8[i] > 8) {
			_zbasic->indexSet(
				_zbasic->index(1, 0x1d + arr_i16_1eb8[i] - 8) + _zbasic->str(OFF(7)), // X triplet
				1,
				0x27 + i
			);
		}
		// 139:0f84
		_zbasic->indexSet(
			_zbasic->index(1,0x15 - arr_i16_1eb8[i + 2]),
			1,
			i + 0x29
		);
		// 139:0fca
		if (_zbasic->index(1, i + 0x27).empty()) { // was: str(OFF(8))
			// 139:0ff0
			_zbasic->indexSet(
				_zbasic->index(1, 0x29 + i),
				1,
				i + 0x27
			);

		}
		// 139:101a
	}
	// 139:1028
	// determine winner
	var_i16_2016 = 0;
	if (arr_i16_1eb8[1] > arr_i16_1eb8[2]) {
		var_i16_2016 = 1; // fool wins
	}
	// 139:105e
	if (arr_i16_1eb8[1] < arr_i16_1eb8[2]) {
		var_i16_2016 = 2; // ol' mate wins
	}
	if (var_i16_2016 == 0) {
		if (arr_i16_1eb8[3] > arr_i16_1eb8[4]) {
			var_i16_2016 = 1; // fool wins
		}
		// 139:10c8
		if (arr_i16_1eb8[3] < arr_i16_1eb8[4]) {
			var_i16_2016 = 2; // ol' mate wins
		}
		// 139:10f8
		if (arr_i16_1eb8[1] != 0) {
			for (int i = 1; i <= 2; i++) {
				_zbasic->indexSet(
					_zbasic->index(1, 0x27 + i) + _zbasic->str(OFF(9)) + _zbasic->index(1, 0x29 + i) + _zbasic->str(OFF(10)),
					1,
					0x27 + i
				);
			}
		}
	}
	// 139:1174
	drawTarotCard(6, 3, 0);
	// calculate number of points gained by the winner
	var_i16_2012 = 9*(arr_i16_1eb8[var_i16_2016] + 1);

	cardsDrawScores();
	if (var_i16_2016 == 1) {
		var_str_384 = _zbasic->str(OFF(11)); // The fool
	} else {
		// 139:11ce
		var_str_384 = _zbasic->str(OFF(12)); // The Old Man
	}
	// 139:11e2
	if ((arr_i16_1eb8[5] > CARDS_MAX_SCORE) || (arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
		var_str_9f4 = _zbasic->str(OFF(13)); // the game!
	} else {
		// 139:123e
		var_str_9f4 = Common::U32String::format(" %d", var_i16_2012) + _zbasic->str(OFF(14)); // X points
	}
	// 139:1264
	var_str_384 += _zbasic->str(OFF(15)) + var_str_9f4; // X wins Y
	// winning hand
	var_str_167c = _zbasic->index(1, 0x28);
	// losing hand
	var_str_2222 = _zbasic->index(1, 0x29);
	if (var_i16_2016 == 2) {
		_zbasic->swapStr(var_str_167c, var_str_2222);
	}
	var_i16_484 = _toolbox->StringWidth(var_str_384);
	var_i16_7e4 = _toolbox->StringWidth(var_str_167c);
	var_i16_9f2 = _toolbox->StringWidth(var_str_2222);
	var_i16_2322 = 0;
	if (var_i16_484 > var_i16_2322) {
		var_i16_2322 = var_i16_484;
	}
	// 139:1318
	if (var_i16_7e4 > var_i16_2322) {
		var_i16_2322 = var_i16_7e4;
	}
	if (var_i16_9f2 > var_i16_2322) {
		var_i16_2322 = var_i16_9f2;
	}
	// 139:1344
	var_i16_2322 = (var_i16_2322 / 2) + 0x14;
	fillRect(0x2e, 0x100 - var_i16_2322, 0x9a, 0x100 + var_i16_2322, 2);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	drawTextCenter(var_str_384, 0x46);
	drawTextCenter(var_str_167c, 0x64);
	drawTextCenter(_zbasic->str(OFF(16)), 0x78); // beats
	drawTextCenter(var_str_2222, 0x8c);
	if (arr_i16_1eb8[1] != 0) {
		var_i16_68a = arr_i16_1eb8[1];
		if (var_i16_68a > 8) {
			var_i16_68a -= 8;
		}
		// 139:1420
		if (_zbasic->instr(
			1,
			_zbasic->index(1, 0x15 + var_i16_68a),
			_zbasic->midStr(_zbasic->index(1, 0x26), 1, 1)
		) == 0) {
			drawTarotCard(5, 0, 2);
		}
		// 139:1470
		if (_zbasic->instr(
				1,
				_zbasic->index(1, 0x15 + var_i16_68a),
				_zbasic->midStr(_zbasic->index(1, 0x26), 2, 1)
			) == 0) {
			drawTarotCard(7, 0, 2);
		}
		// 139:14c0
		if (_zbasic->instr(
				1,
				_zbasic->index(1, 0x15 + var_i16_68a),
				_zbasic->midStr(_zbasic->index(1, 0x26), 3, 1)
			) == 0) {
			drawTarotCard(9, 0, 2);
		}
	} else {
		// 139:1514
		var_str_384 = _zbasic->chr((0x15 - arr_i16_1eb8[3]) + 0x41);
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 1, 1) != var_str_384) {
			drawTarotCard(5, 0, 2);
		}
		// 139:157e
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 2, 1) != var_str_384) {
			drawTarotCard(7, 0, 2);
		}
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 3, 1) != var_str_384) {
			drawTarotCard(9, 0, 2);
		}
	}
	// 139:15f2
	if (arr_i16_1eb8[2] != 0) {
		var_i16_68a = arr_i16_1eb8[2];
		if (var_i16_68a > 8) {
			var_i16_68a -= 8;
		}
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 1, 1)) == 0) {
			drawTarotCard(4, 0, 2);
		}
		// 139:1678
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 2, 1)) == 0) {
			drawTarotCard(6, 0, 2);
		}
		// 139:16c8
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 3, 1)) == 0) {
			drawTarotCard(8, 0, 2);
		}
	} else {
		// 139:171c
		var_str_384 = _zbasic->chr((0x15 - arr_i16_1eb8[4]) + 0x41);
		if (var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 1, 1)) {
			drawTarotCard(4, 0, 2);
		}
		// 139:1786
		if (var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 2, 1)) {
			drawTarotCard(6, 0, 2);
		}
		if (var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 3, 1)) {
			drawTarotCard(8, 0, 2);
		}
	}
	// 139:17fa
}

void FoolGame::sub_139_17fc() {
	// 139:17fc
	fillRect(0xb4, 0xb5, 0xf3, 0x14b, 2);
	fillRect(0x14, 0x5f, 0xaf, 0x1a4, 1);
	fillRect(0x2a, 0, 0x156, 0xb4, 1);
	fillRect(0x2a, 0x14c, 0x156, 0x200, 1);
	// 139:1878
	for (int i = 1; i <= 2; i++) {
		arr_i16_1eb8[i + 2] = 0;
		arr_i16_1eb8[i] = 0;
		_zbasic->indexSet(Common::U32String(), 1, 0x25 + i); // was: str(OFF(17))
		_zbasic->indexSet(Common::U32String(), 1, 0x27 + i); // was: str(OFF(18))
		_zbasic->indexSet(Common::U32String(), 1, 0x29 + i); // was: str(OFF(19))
	}
}

void FoolGame::sub_139_191c() {
	// 139:191c
	arr_i16_1eb8[var_i16_2016 + 2] = 0;
	for (int i = 1; i <= (int)_zbasic->index(1, var_i16_2016 + 0x25).size(); i++) {
		var_i16_484 = 0x15 - (_zbasic->asc(_zbasic->midStr(_zbasic->index(1, 0x25 + var_i16_2016), i, 1)) - 0x41);
		// 139:1974
		if (var_i16_484 > arr_i16_1eb8[2 + var_i16_2016]) {
			arr_i16_1eb8[2 + var_i16_2016] = var_i16_484;
		}
		// 139:19b8
	}
}

void FoolGame::sub_139_19da() {
	// 139:19da
	arr_i16_1eb8[var_i16_2016] = 0;
	for (int k = 1; k <= 2; k++) {
		// cycle backward through the scoring combinations
		for (int j = 8; j >= 1; j--) {
			var_i16_7a8 = 0;
			for (int i = 1; i <= (int)_zbasic->index(1, j + 0x15).size(); i++) {
				if (_zbasic->instr(
					1,
					_zbasic->index(1, var_i16_2016 + 0x25),
					_zbasic->midStr(_zbasic->index(1, 0x15 + j), i, 1)
				) != 0) {
					var_i16_7a8++;
				}
				// 139:1a4c
			}
			// 139:1a6a
			if (((k == 2) && (var_i16_7a8 == 2)) ||
				((k == 1) && (var_i16_7a8 == 3)))	{
				// 139:1ad6
				arr_i16_1eb8[var_i16_2016] = (var_i16_7a8 - 2)*8 + j;
				j = 1;
				k = 2;
			}
		}
		// 139:1ac6
	}
}

void FoolGame::cardsDrawScores() {
	// 139:1b12
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	arr_i16_1eb8[var_i16_2016 + 4] += var_i16_2012;
	if (arr_i16_1eb8[5] < CARDS_MIN_SCORE) {
		arr_i16_1eb8[5] = CARDS_MIN_SCORE;
	}
	// 139:1b88
	if (arr_i16_1eb8[6] < CARDS_MIN_SCORE) {
		arr_i16_1eb8[6] = CARDS_MIN_SCORE;
	}
	// 139:1bb2
	fillRect(0x14, 0, 0x29, 0x5a, 2);
	// Old Man
	var_str_384 = _zbasic->str(OFF(20)) + Common::U32String::format(" %d", arr_i16_1eb8[6]);
	var_i16_484 = _toolbox->StringWidth(var_str_384);
	_toolbox->MoveTo(0x2d - (var_i16_484 / 2), 0x23);
	_toolbox->DrawString(var_str_384);

	// 139:1c30
	fillRect(0x14, 0x1a6, 0x29, 0x200, 2);
	var_str_384 = _zbasic->str(OFF(21)) + Common::U32String::format(" %d", arr_i16_1eb8[5]);
	var_i16_484 = _toolbox->StringWidth(var_str_384);
	_toolbox->MoveTo(0x1d3 - (var_i16_484 / 2), 0x23);
	_toolbox->DrawString(var_str_384);
}

void FoolGame::cardsStoreState() {
	// 139:1cba
	_activePuzzleBuffer = _zbasic->encodeInt(arr_i16_1eb8[5]) +
		_zbasic->encodeInt(arr_i16_1eb8[6]) +
		_zbasic->encodeInt(var_i16_2010);
	// 139:1d00
	for (int i = 0; i <= 0x15; i++) {
		_activePuzzleBuffer += _zbasic->encodeInt(arr_i16_5cbc[i]);
	}
}

void FoolGame::sub_139_1d3e() {
	// 139:1d3e
	_stateFlags = kFlagMenuEnabled;
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	waitForMouseUp();
	menuClickMessage();
	flashRect(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xc8);
}

} // End of namespace Fool
