/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sci/engine/features.h"
#include "sci/engine/hoyle5poker.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"

namespace Sci {

#ifdef ENABLE_SCI32

// The logic for the poker game in Hoyle Classic Games (Hoyle 5) is hardcoded
// in PENGIN16.DLL, which is then loaded and invoked via the kWinDLL kernel call.
// Note that the first player is the left one.

enum Hoyle5PokerSuits {
	kSuitSpades = 0,
	kSuitClubs = 1,
	kSuitDiamonds = 2,
	kSuitHearts = 3
};

enum Hoyle5PokerData {
	kTotalChips = 1,
	kCurrentPot = 2,
	kCurrentBet = 3,
	kChipsPlayer1 = 4,
	kChipsPlayer2 = 5,
	kChipsPlayer3 = 6,
	kChipsPlayer4 = 7,
	kStatusPlayer1 = 8,
	kStatusPlayer2 = 9,
	kStatusPlayer3 = 10,
	kStatusPlayer4 = 11,
	// 12 - 16 seem to be unused?
	kCurrentPlayer = 17,
	kCurrentStage = 18	// Stage 1: Card changes, 2: Betting
	// 19 - 28: current player's cards (number + suit)
	// 29 - 38: next clockwise player's cards (number + suit)
	// 39 - 48: next clockwise player's cards (number + suit)
	// 49 - 58: next clockwise player's cards (number + suit)
	// 59 - 67 seem to be unused?
	// 77 seems to be a bit array?
};

#if 0
Common::String getCardDescription(int16 card, int16 suit) {
	Common::String result;

	if (card >= 2 && card <= 10)
		result += Common::String::format("%d", card);
	else if (card == 11)
		result = "Jack";
	else if (card == 12)
		result = "Queen";
	else if (card == 13)
		result = "King";
	else if (card == 14)
		result = "Ace";
	else
		result = "Unknown";

	switch (suit) {
	case kSuitSpades:
		return result + " of spades";
	case kSuitClubs:
		return result + " of clubs";
	case kSuitDiamonds:
		return result + " of diamonds";
	case kSuitHearts:
		return result + " of hearts";
	default:
		return result + " of unknown";
	}
}

void printPlayerCards(int player, SciArray *data) {
	debug("Player %d cards:", player);
	for (int i = 19 + player * 10; i < 29 + player * 10; i += 2) {
		if (data->getAsInt16(i) > 0)
			debug("- %s", getCardDescription(data->getAsInt16(i), data->getAsInt16(i + 1)).c_str());
	}
}
#endif

reg_t hoyle5PokerEngine(SciArray *data) {
#if 0
	debug("Player %d's turn", data->getAsInt16(kCurrentPlayer));

	debug("Pot: %d, bet: %d", data->getAsInt16(kCurrentPot), data->getAsInt16(kCurrentBet));

	debug("Chips: %d %d %d %d - %d in total",
		data->getAsInt16(kChipsPlayer1),
		data->getAsInt16(kChipsPlayer2),
		data->getAsInt16(kChipsPlayer3),
		data->getAsInt16(kChipsPlayer4),
		data->getAsInt16(kTotalChips)
	);

	debug("Player status: %d %d %d %d",
		data->getAsInt16(kStatusPlayer1),
		data->getAsInt16(kStatusPlayer2),
		data->getAsInt16(kStatusPlayer3),
		data->getAsInt16(kStatusPlayer4)
	);

	for (int i = 0; i < 4; i++)
		printPlayerCards(i, data);

	for (int i = 0; i < data->size(); i++) {
		if (i >= kChipsPlayer1 && i <= kChipsPlayer4)
			continue;
		if (i >= 8 && i <= 11)
			continue;
		if (i >= 19 && i <= 58)
			continue;

		if (data->getAsInt16(i) != 0)
			debug("%d: %d", i, data->getAsInt16(i));
	}
#endif

	warning("The Poker game logic has not been implemented yet");
	return NULL_REG;	// Returning 0 is a DLL invocation error for the game scripts
}
#endif

}
