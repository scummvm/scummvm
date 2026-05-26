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

#define OFF(x) (_zstrOffset[kOffsetCards] + (x))

extern Toolbox *g_toolbox;

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
	this->sub_128_271a();
	this->var_i16_c00 = 1;
	// set up card bounding boxes
	g_toolbox->SetRect(_screenGrid[1], 0x78, 0x19, 0xd1, 0xae);
	g_toolbox->SetRect(_screenGrid[2], 0xd3, 0x19, 0x12c, 0xae);
	g_toolbox->SetRect(_screenGrid[3], 0x12e, 0x19, 0x187, 0xae);
	g_toolbox->SetRect(_screenGrid[4],1, 0x2a, 0x5a, 0xbf);
	g_toolbox->SetRect(_screenGrid[5],0x1a6, 0x2a, 0x1ff, 0xbf);
	g_toolbox->SetRect(_screenGrid[6], 0x1, 0xc0, 0x5a, 0x155);
	g_toolbox->SetRect(_screenGrid[7],0x1a6, 0xc0, 0x1ff, 0x155);
	g_toolbox->SetRect(_screenGrid[8],0x5b, 0xc0, 0xb4, 0x155);
	g_toolbox->SetRect(_screenGrid[9],0x14c, 0xc0, 0x1a5, 0x155);

	// 139:011c
	for (int i = 0; i <= 0x25; i++) {
		_zbasic->indexSet(puzzlesReadString(), 1, i);
	}

	this->var_i16_484 = 0x5e;
	for (int i = 0x17; i >= 0; i--) {
		this->var_i16_484++;
		this->arr_i32_192c0[i] = g_toolbox->GetPicture(this->var_i16_484);
	}
	// 139:0184
	this->sub_128_bde(1, 1, 0, 1, 1, 1);
	this->fillRect(0xb4, 0xb5, 0x156, 0x14b, 2);
	// illustration of the card game
	_zbasic->picture(0xc3, 0xf3, this->arr_i32_192c0[0x17]);
	if (_activePuzzleBuffer.empty()) {
		// 139:01fa
		_activePuzzleBuffer = _zbasic->encodeInt(0) + _zbasic->encodeInt(0) + _zbasic->encodeInt(0);
		// card IDs
		for (int16 i = 0; i <= 0x15; i++) {
			this->arr_i16_5cbc[i] = i;
		}
		// 139:0244
		for (int i = 0; i <= 0x15; i++) {
			this->var_i16_484 = _zbasic->rndInt(0x16) - 1;
			this->var_i16_7e4 = _zbasic->rndInt(0x16) - 1;
			_zbasic->swapInt(this->arr_i16_5cbc[this->var_i16_484], this->arr_i16_5cbc[this->var_i16_7e4]);
		}
		// 139:029c
		for (int i = 0; i <= 0x15; i++) {
			_activePuzzleBuffer += _zbasic->encodeInt(this->arr_i16_5cbc[i]);
		}
	}
	// 139:02d8
	// player scores
	this->arr_i16_1eb8[5] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 1, 2));
	this->arr_i16_1eb8[6] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 3, 2));
	// 139:0320
	this->var_i16_2010 = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, 5, 2));
	if ((this->arr_i16_1eb8[5] > CARDS_MAX_SCORE) || (this->arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
		this->arr_i16_1eb8[5] = 0;
		this->arr_i16_1eb8[6] = 0;
		this->var_i16_2010 = 0;
	}
	// 139:039e

	for (int i = 0; i <= 0x15; i++) {
		this->arr_i16_5cbc[i] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, i*2 + 7, 2));
	}
	// 139:03ea
	this->arr_i16_5cbc[0x16] = 0x16;
	this->arr_rect_4338.top = 0xdb;
	this->arr_rect_4338.left = 0xe2;
	this->arr_rect_4338.bottom = 0xee;
	this->arr_rect_4338.right = 0x11e;
	this->var_i16_2012 = 0;
	_stateFlags = 0;
	this->cardsDrawScores();
	if (this->var_i16_2010 == 0) {
		this->cardsShuffleDeck();
	}
	// 139:0456
	this->cardsDrawTable();
	while ((_stateFlags & kStateReturn) == 0) {
		// 139:045e
		this->var_i16_2014 = 0;
		_stateFlags = 0;
		while (_stateFlags == 0) {
			// 139:046e
			this->getNextEvent(-1);
			if ((_event.modifiers & kModMouseButtonUp) == 0) {
				this->cardsOnClick();
			}
			if (this->var_i16_2014 != 0) {
				this->sub_139_50e();
			}
		}
		// 139:04a4
		if (_stateFlags == kStateUndo) {
			this->cardsReset();
		}
		if (_stateFlags == kStateSaveGame) {
			this->cardsStoreState();
			this->saveGame();
		}
		// 139:04c4
	}
	// 139:04d6
	this->cardsStoreState();
	// JMP 1d7e
	// 139:1d7e
	for (int i = 0; i <= 0x17; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_192c0[i]);
	}
}

void FoolGame::cardsReset() {
	this->arr_i16_1eb8[5] = 0;
	this->arr_i16_1eb8[6] = 0;
	this->var_i16_2012 = 0;
	this->cardsDrawScores();
	this->cardsShuffleDeck();
	this->cardsDrawTable();
}

void FoolGame::sub_139_50e() {
	this->var_i16_2014 = 0;
	this->cardsShuffleDeck();
	this->cardsDrawTable();
}

void FoolGame::cardsShuffleDeck() {
	// 139:051e
	_zbasic->unk_20();
	for (int j = 0; j <= 2; j++) {
		for (int i = 0; i <= 0x15; i++) {
			this->var_i16_484 = _zbasic->rndInt(0x16)-1;
			_zbasic->swapInt(this->arr_i16_5cbc[this->var_i16_484], this->arr_i16_5cbc[i]);
		}
	}
}

void FoolGame::cardsDrawTable() {
	this->sub_139_17fc();
	this->drawTarotCard(5, 0, 0);
	this->drawTarotCard(4, 1, 0);
	this->drawTarotCard(7, 2, 0);
	this->drawTarotCard(6, 0x16, 0);
	this->drawTarotCard(1, 4, 0);
	this->drawTarotCard(2, 5, 0);
	this->drawTarotCard(3, 6, 0);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_i16_7a2 = 0xc3;
	// 139:061a
	this->sub_128_918(_zbasic->str(OFF(0))); // Select a card above
	this->var_i16_7a2 = 0xd4;
	this->sub_128_918(_zbasic->str(OFF(1))); // or
	g_toolbox->FillRoundRect(this->arr_rect_4338, 0xa, 0xa, _patterns[0]);
	g_toolbox->FrameRoundRect(this->arr_rect_4338, 0xa, 0xa);
	_zbasic->text(_fontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_7a2 = 0xe9;
	this->sub_128_918(_zbasic->str(OFF(2))); // Yield
	this->var_i16_2016 = 1;
	this->var_i16_2010 = 1;
	this->sub_128_6186();
}

void FoolGame::cardsOnClick() {
	// 139:06b0
	this->var_i16_1e00 = 0;
	if (!((_event.where.x >= 0x78) && (_event.where.y >= 0x19) && (_event.where.x <= 0x187) && (_event.where.y <= 0xae))) {
		// 139:06fc
		// didn't click in the three cards area
		if (g_toolbox->PtInRect(_event.where, this->arr_rect_4338)) {
			g_toolbox->InvertRoundRect(this->arr_rect_4338, 0xa, 0xa);
			this->sub_128_6186();
			g_toolbox->InvertRoundRect(this->arr_rect_4338, 0xa, 0xa);
			if (!g_toolbox->PtInRect(_event.where, this->arr_rect_4338))
				return;
			// 139:0778
			// player clicked the yield button (bad choice)
			this->playTone(0x19, 0x64, 0x0);
			this->var_i16_2012 = -CARDS_YIELD_PENALTY;
			this->cardsDrawScores();
			this->var_i16_2014 = 1;
		} else {
			// 139:079e
			this->playTone(0xf, 0x64, 0x1);
			for (int j = 0; j <= 1; j++) {
				for (int16 i = 0; i <= 0x3; i++) {
					this->drawTarotCard(i, 0, 3);
				}
			}

		}
		// 139:07e6
	} else {
		// 139:07ea
		// player picked a card
		this->fillRect(0xb4, 0xb5, 0xf3, 0x14b, 2);
		if (_event.where.x < 0xd2) {
			// 139:0818
			this->var_i16_1e00 = 4;
			this->drawTarotCard(1, 0, 1);
		}
		// 139:082e
		if ((_event.where.x >= 0xd2) && (_event.where.x <= 0x12d)) {
			this->var_i16_1e00 = 5;
			this->drawTarotCard(2, 0, 1);
		}
		// 139:0868
		if (_event.where.x > 0x12d) {
			this->var_i16_1e00 = 6;
			this->drawTarotCard(3, 0, 1);
		}
		// 139:088a
		this->drawTarotCard(9, this->var_i16_1e00, 0);
		_zbasic->indexSet(
			_zbasic->chr(this->arr_i16_5cbc[0]+0x41) + _zbasic->chr(this->arr_i16_5cbc[2]+0x41) + _zbasic->chr(this->arr_i16_5cbc[this->var_i16_1e00]+0x41),
			1,
			0x26
		);
		this->sub_139_19da();
		this->sub_139_191c();
		this->sub_139_a22();
		if (this->var_i16_2018 == 0) {
			// 139:0920
			this->cardsRevealHands();
			this->var_i16_7a2 = 0xc3;

		} else {
			// 139:092e
			this->cardsOpponentYields();
			this->var_i16_7a2 = 0xd7;
		}
		// 139:0938
		this->var_i16_2010 = 0;
		if (this->arr_i16_1eb8[5] > CARDS_MAX_SCORE) {
			this->sub_139_1d3e();
			return;
		}
		// 139:095e
		this->sub_128_61ec();
		// original code has the font ID as 1... maybe there's a fallback?
		// this was intended to be Small
		_zbasic->text(kFontSmall, 9, Graphics::kMacFontRegular, kSrcBic);
		this->sub_128_918(_zbasic->str(OFF(3))); // (click mouse to continue)
		this->var_i16_2014 = 0;
		_stateFlags = 0;
		while ((_stateFlags == 0) && (this->var_i16_2014 == 0)) {
			this->getNextEvent(-1);
			if (_event.what == kMouseUp) {
				this->var_i16_2014 = 1;
			}
			if (_stateFlags == kStateSaveGame) {
				this->cardsStoreState();
				this->saveGame();
			}
		}
		// 139:09d8
		if (((_stateFlags & kStateReturn) == 0) && (this->arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
			_stateFlags = kStateUndo;
			this->var_i16_2014 = 0;
		}
	}
	// 139:0a20
	return;
}

void FoolGame::sub_139_a22() {
	// 139:0a22
	this->var_str_201a = _zbasic->chr(this->arr_i16_5cbc[0] + 0x41) + _zbasic->chr(this->arr_i16_5cbc[this->var_i16_1e00] + 0x41);
	this->var_i16_211a = this->var_i16_1e00;
	_zbasic->indexSet(_zbasic->chr(this->arr_i16_5cbc[1] + 0x41) + _zbasic->chr(this->arr_i16_5cbc[3] + 0x41), 1, 0x27);
	this->var_str_211c = _zbasic->index(1, 0x27);
	this->var_i16_2016 = 0x2;
	this->var_i16_221c = 0;
	this->var_i16_221e = 0;
	for (int16 i = 4; i <= 6; i++) {
		// 139:0ae8
		if (i  != this->var_i16_211a) {
			_zbasic->indexSet(this->var_str_211c + _zbasic->chr(this->arr_i16_5cbc[i] + 0x41), 1, 0x27);
			this->sub_139_19da();
			if (this->arr_i16_1eb8[2] >= this->var_i16_221c) {
				// 139:0b52
				this->var_i16_221c = this->arr_i16_1eb8[2];
				this->var_i16_1e00 = i;
			} else {
				// 139:0b70
				if (this->arr_i16_1eb8[2] == this->var_i16_221c) {
					this->sub_139_191c();
					if (this->arr_i16_1eb8[4] > this->var_i16_221e) {
						this->var_i16_221e = this->arr_i16_1eb8[4];
						this->var_i16_1e00 = i;
					}
				}
			}
		}
		// 139:0bc4
	}
	_zbasic->indexSet(this->var_str_211c + _zbasic->chr(this->arr_i16_5cbc[this->var_i16_1e00] + 0x41), 1, 0x27);
	// 139:0c0a
	this->arr_i16_1eb8[2] = this->var_i16_221c;
	this->sub_139_191c();
	this->var_i16_221c = 0;
	// cycle backwards through the scoring combinations
	for (int16 j = 8; j >= 1; j--) {
		this->var_i16_7a8 = 0;
		// count the number of combination cards in the hand
		for (int16 i = 1; i <= (int16)_zbasic->index(1, j + 0x15).size(); i++) {
			if (_zbasic->instr(
					1,
					this->var_str_201a,
					_zbasic->midStr(_zbasic->index(1, j + 0x15), i, 1))
				) {
				this->var_i16_7a8++;
			}
			// 139:0c72
		}
		// 139:0c90
		if (this->var_i16_7a8 == 2) {
			this->var_i16_221c = j;
			j = 1;
		}
		// 139:0ca6
	}
	// 139:0cb6
	this->var_i16_2018 = 0;
	if (this->var_i16_221c > this->arr_i16_1eb8[2]) {
		this->var_i16_2018 = 1;
	}
	// 139:0ce2
	if (this->var_i16_221c == this->arr_i16_1eb8[2]) {
		for (int16 i = 1; i <= (int16)this->var_str_201a.size(); i++) {
			this->var_i16_484 = 0x15 - (_zbasic->asc(_zbasic->midStr(this->var_str_201a, i, 1)) - 0x41);
			if (this->var_i16_484 > this->arr_i16_1eb8[4]) {
				this->var_i16_2018 = 1;
			}
			// 139:0d5c
		}
	}
	// 139:0d6e
	if ((this->var_i16_2018 == 1) && ((this->var_i16_221c + 1)*9 <= CARDS_YIELD_PENALTY)) {
		this->var_i16_2018 = 0;
	}

	// 139:0da6
}

void FoolGame::cardsOpponentYields() {
	// 139:0da8
	// ol' mate gives up
	this->playTone(0x19, 0x64, 0x1);
	this->var_i16_2012 = -CARDS_YIELD_PENALTY;
	this->cardsDrawScores();
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_i16_7a2 = 0xc4;
	this->sub_128_918(_zbasic->str(OFF(4))); // The Old Man yields
}

void FoolGame::cardsRevealHands() {
	// 139:0dee
	// remove the three cards from the top row
	this->drawTarotCard(1, 0, 1);
	this->drawTarotCard(2, 0, 1);
	this->drawTarotCard(3, 0, 1);

	// show ol' mate's draw from the pile
	this->drawTarotCard(8, this->var_i16_1e00, 0);

	_zbasic->indexSet(_zbasic->index(1, 0x27) + _zbasic->chr(this->arr_i16_5cbc[this->var_i16_1e00] + 0x41), 1, 0x27);
	// 139:0e70
	for (int16 i = 1; i <= 2; i++) {
		_zbasic->indexSet(Common::U32String(), 1, i + 0x27); // was: str(OFF(5))
		if ((this->arr_i16_1eb8[i] > 0) && (this->arr_i16_1eb8[i] < 9)) {
			// 139:0ed6
			_zbasic->indexSet(
				_zbasic->index(1, 0x1d + this->arr_i16_1eb8[i]) + _zbasic->str(OFF(6)), // X pair
				1,
				i + 0x27
			);
		}
		// 139:0f1e
		if (this->arr_i16_1eb8[i] > 8) {
			_zbasic->indexSet(
				_zbasic->index(1, 0x1d + this->arr_i16_1eb8[i] - 8) + _zbasic->str(OFF(7)), // X triplet
				1,
				0x27 + i
			);
		}
		// 139:0f84
		_zbasic->indexSet(
			_zbasic->index(1,0x15 - this->arr_i16_1eb8[i + 2]),
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
	this->var_i16_2016 = 0;
	if (this->arr_i16_1eb8[1] > this->arr_i16_1eb8[2]) {
		this->var_i16_2016 = 1; // fool wins
	}
	// 139:105e
	if (this->arr_i16_1eb8[1] < this->arr_i16_1eb8[2]) {
		this->var_i16_2016 = 2; // ol' mate wins
	}
	if (this->var_i16_2016 == 0) {
		if (this->arr_i16_1eb8[3] > this->arr_i16_1eb8[4]) {
			this->var_i16_2016 = 1; // fool wins
		}
		// 139:10c8
		if (this->arr_i16_1eb8[3] < this->arr_i16_1eb8[4]) {
			this->var_i16_2016 = 2; // ol' mate wins
		}
		// 139:10f8
		if (this->arr_i16_1eb8[1] != 0) {
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
	this->drawTarotCard(6, 3, 0);
	// calculate number of points gained by the winner
	this->var_i16_2012 = 9*(this->arr_i16_1eb8[this->var_i16_2016] + 1);

	this->cardsDrawScores();
	if (this->var_i16_2016 == 1) {
		this->var_str_384 = _zbasic->str(OFF(11)); // The fool
	} else {
		// 139:11ce
		this->var_str_384 = _zbasic->str(OFF(12)); // The Old Man
	}
	// 139:11e2
	if ((this->arr_i16_1eb8[5] > CARDS_MAX_SCORE) || (this->arr_i16_1eb8[6] > CARDS_MAX_SCORE)) {
		this->var_str_9f4 = _zbasic->str(OFF(13)); // the game!
	} else {
		// 139:123e
		this->var_str_9f4 = Common::U32String::format(" %d", this->var_i16_2012) + _zbasic->str(OFF(14)); // X points
	}
	// 139:1264
	this->var_str_384 += _zbasic->str(OFF(15)) + this->var_str_9f4; // X wins Y
	// winning hand
	this->var_str_167c = _zbasic->index(1, 0x28);
	// losing hand
	this->var_str_2222 = _zbasic->index(1, 0x29);
	if (this->var_i16_2016 == 2) {
		_zbasic->swapStr(this->var_str_167c, this->var_str_2222);
	}
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_384);
	this->var_i16_7e4 = g_toolbox->StringWidth(this->var_str_167c);
	this->var_i16_9f2 = g_toolbox->StringWidth(this->var_str_2222);
	this->var_i16_2322 = 0;
	if (this->var_i16_484 > this->var_i16_2322) {
		this->var_i16_2322 = this->var_i16_484;
	}
	// 139:1318
	if (this->var_i16_7e4 > this->var_i16_2322) {
		this->var_i16_2322 = this->var_i16_7e4;
	}
	if (this->var_i16_9f2 > this->var_i16_2322) {
		this->var_i16_2322 = this->var_i16_9f2;
	}
	// 139:1344
	this->var_i16_2322 = (this->var_i16_2322 / 2) + 0x14;
	this->fillRect(0x2e, 0x100 - this->var_i16_2322, 0x9a, 0x100 + this->var_i16_2322, 2);
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_i16_7a2 = 0x46;
	this->sub_128_918(this->var_str_384);
	this->var_i16_7a2 = 0x64;
	this->sub_128_918(this->var_str_167c);
	this->var_i16_7a2 = 0x78;
	this->sub_128_918(_zbasic->str(OFF(16))); // beats
	this->var_i16_7a2 = 0x8c;
	this->sub_128_918(this->var_str_2222);
	if (this->arr_i16_1eb8[1] != 0) {
		this->var_i16_68a = this->arr_i16_1eb8[1];
		if (this->var_i16_68a > 8) {
			this->var_i16_68a -= 8;
		}
		// 139:1420
		if (_zbasic->instr(
			1,
			_zbasic->index(1, 0x15 + this->var_i16_68a),
			_zbasic->midStr(_zbasic->index(1, 0x26), 1, 1)
		) == 0) {
			this->drawTarotCard(5, 0, 2);
		}
		// 139:1470
		if (_zbasic->instr(
				1,
				_zbasic->index(1, 0x15 + this->var_i16_68a),
				_zbasic->midStr(_zbasic->index(1, 0x26), 2, 1)
			) == 0) {
			this->drawTarotCard(7, 0, 2);
		}
		// 139:14c0
		if (_zbasic->instr(
				1,
				_zbasic->index(1, 0x15 + this->var_i16_68a),
				_zbasic->midStr(_zbasic->index(1, 0x26), 3, 1)
			) == 0) {
			this->drawTarotCard(9, 0, 2);
		}
	} else {
		// 139:1514
		this->var_str_384 = _zbasic->chr((0x15 - this->arr_i16_1eb8[3]) + 0x41);
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 1, 1) != this->var_str_384) {
			this->drawTarotCard(5, 0, 2);
		}
		// 139:157e
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 2, 1) != this->var_str_384) {
			this->drawTarotCard(7, 0, 2);
		}
		if (_zbasic->midStr(_zbasic->index(1, 0x26), 3, 1) != this->var_str_384) {
			this->drawTarotCard(9, 0, 2);
		}
	}
	// 139:15f2
	if (this->arr_i16_1eb8[2] != 0) {
		this->var_i16_68a = this->arr_i16_1eb8[2];
		if (this->var_i16_68a > 8) {
			this->var_i16_68a -= 8;
		}
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + this->var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 1, 1)) == 0) {
			this->drawTarotCard(4, 0, 2);
		}
		// 139:1678
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + this->var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 2, 1)) == 0) {
			this->drawTarotCard(6, 0, 2);
		}
		// 139:16c8
		if (_zbasic->instr(1, _zbasic->index(1, 0x15 + this->var_i16_68a), _zbasic->midStr(_zbasic->index(1, 0x27), 3, 1)) == 0) {
			this->drawTarotCard(8, 0, 2);
		}
	} else {
		// 139:171c
		this->var_str_384 = _zbasic->chr((0x15 - this->arr_i16_1eb8[4]) + 0x41);
		if (this->var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 1, 1)) {
			this->drawTarotCard(4, 0, 2);
		}
		// 139:1786
		if (this->var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 2, 1)) {
			this->drawTarotCard(6, 0, 2);
		}
		if (this->var_str_384 != _zbasic->midStr(_zbasic->index(1, 0x27), 3, 1)) {
			this->drawTarotCard(8, 0, 2);
		}
	}
	// 139:17fa
}

void FoolGame::sub_139_17fc() {
	// 139:17fc
	this->fillRect(0xb4, 0xb5, 0xf3, 0x14b, 2);
	this->fillRect(0x14, 0x5f, 0xaf, 0x1a4, 1);
	this->fillRect(0x2a, 0, 0x156, 0xb4, 1);
	this->fillRect(0x2a, 0x14c, 0x156, 0x200, 1);
	// 139:1878
	for (int i = 1; i <= 2; i++) {
		this->arr_i16_1eb8[i + 2] = 0;
		this->arr_i16_1eb8[i] = 0;
		_zbasic->indexSet(Common::U32String(), 1, 0x25 + i); // was: str(OFF(17))
		_zbasic->indexSet(Common::U32String(), 1, 0x27 + i); // was: str(OFF(18))
		_zbasic->indexSet(Common::U32String(), 1, 0x29 + i); // was: str(OFF(19))
	}
}

void FoolGame::sub_139_191c() {
	// 139:191c
	this->arr_i16_1eb8[this->var_i16_2016 + 2] = 0;
	for (int i = 1; i <= (int)_zbasic->index(1, this->var_i16_2016 + 0x25).size(); i++) {
		this->var_i16_484 = 0x15 - (_zbasic->asc(_zbasic->midStr(_zbasic->index(1, 0x25 + this->var_i16_2016), i, 1)) - 0x41);
		// 139:1974
		if (this->var_i16_484 > this->arr_i16_1eb8[2 + this->var_i16_2016]) {
			this->arr_i16_1eb8[2 + this->var_i16_2016] = this->var_i16_484;
		}
		// 139:19b8
	}
}

void FoolGame::sub_139_19da() {
	// 139:19da
	this->arr_i16_1eb8[this->var_i16_2016] = 0;
	for (int k = 1; k <= 2; k++) {
		// cycle backward through the scoring combinations
		for (int j = 8; j >= 1; j--) {
			this->var_i16_7a8 = 0;
			for (int i = 1; i <= (int)_zbasic->index(1, j + 0x15).size(); i++) {
				if (_zbasic->instr(
					1,
					_zbasic->index(1, this->var_i16_2016 + 0x25),
					_zbasic->midStr(_zbasic->index(1, 0x15 + j), i, 1)
				) != 0) {
					this->var_i16_7a8++;
				}
				// 139:1a4c
			}
			// 139:1a6a
			if (((k == 2) && (this->var_i16_7a8 == 2)) ||
				((k == 1) && (this->var_i16_7a8 == 3)))	{
				// 139:1ad6
				this->arr_i16_1eb8[this->var_i16_2016] = (this->var_i16_7a8 - 2)*8 + j;
				j = 1;
				k = 2;
			}
		}
		// 139:1ac6
	}
}

void FoolGame::cardsDrawScores() {
	// 139:1b12
	_zbasic->text(kFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->arr_i16_1eb8[this->var_i16_2016 + 4] += this->var_i16_2012;
	if (this->arr_i16_1eb8[5] < CARDS_MIN_SCORE) {
		this->arr_i16_1eb8[5] = CARDS_MIN_SCORE;
	}
	// 139:1b88
	if (this->arr_i16_1eb8[6] < CARDS_MIN_SCORE) {
		this->arr_i16_1eb8[6] = CARDS_MIN_SCORE;
	}
	// 139:1bb2
	this->fillRect(0x14, 0, 0x29, 0x5a, 2);
	// Old Man
	this->var_str_384 = _zbasic->str(OFF(20)) + Common::U32String::format(" %d", this->arr_i16_1eb8[6]);
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_384);
	g_toolbox->MoveTo(0x2d - (this->var_i16_484 / 2), 0x23);
	g_toolbox->DrawString(this->var_str_384);

	// 139:1c30
	this->fillRect(0x14, 0x1a6, 0x29, 0x200, 2);
	this->var_str_384 = _zbasic->str(OFF(21)) + Common::U32String::format(" %d", this->arr_i16_1eb8[5]);
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_384);
	g_toolbox->MoveTo(0x1d3 - (this->var_i16_484 / 2), 0x23);
	g_toolbox->DrawString(this->var_str_384);
}

void FoolGame::cardsStoreState() {
	// 139:1cba
	_activePuzzleBuffer = _zbasic->encodeInt(this->arr_i16_1eb8[5]) +
		_zbasic->encodeInt(this->arr_i16_1eb8[6]) +
		_zbasic->encodeInt(this->var_i16_2010);
	// 139:1d00
	for (int i = 0; i <= 0x15; i++) {
		_activePuzzleBuffer += _zbasic->encodeInt(this->arr_i16_5cbc[i]);
	}
}

void FoolGame::sub_139_1d3e() {
	// 139:1d3e
	_stateFlags = kFlagMenuEnabled;
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	this->sub_128_6186();
	this->sub_128_2664();
	this->sub_128_d34(0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0xc8);
}


};
