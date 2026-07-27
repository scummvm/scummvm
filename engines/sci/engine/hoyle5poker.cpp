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

#include "common/algorithm.h"
#include "sci/engine/features.h"
#include "sci/engine/hoyle5poker.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"

namespace Sci {

#ifdef ENABLE_SCI32

//#define DEBUG_POKER_LOGIC

// The logic for the poker game in Hoyle Classic Games (Hoyle 5) is hardcoded
// in PENGIN16.DLL, which is then loaded and invoked via the kWinDLL kernel call.
// Note that the first player is the left one.
//
// The DLL is passed a single array of 16-bit words. The kernel entry point
// (SCIDLLENTRY) dispatches on the operation code and calls one of four handlers:
//   op 1 -> FUN_1000_2016 (betting AI)
//   op 2 -> FUN_1000_17e4 (determine winner(s))
//   op 3 -> FUN_1000_1dc0 (discard AI)
//   op 4 -> FUN_1000_632e (classify the current player's hand)
// Internally the DLL builds a linked list of cards for each hand and evaluates
// it (FUN_1000_44c2). There are no wild cards, so the highest possible hand is a
// royal flush, which the DLL reports with its own bit (0x100).

enum Hoyle5PokerSuits {
	kSuitSpades = 0,
	kSuitClubs = 1,
	kSuitDiamonds = 2,
	kSuitHearts = 3
};

enum Hoyle5Operations {
	kCheckPlayerAction = 1, // FUN_1000_2016
	kCheckWinner = 2,       // FUN_1000_17e4
	kCheckDiscard = 3,      // FUN_1000_1dc0 (PokerHand::think)
	kCheckHand = 4          // FUN_1000_632e (PokerHand::whatAmI)
};

enum Hoyle5PlayerActions {
	kPlayerActionCheck = -2,
	kPlayerActionFold = -1,
	kPlayerActionCall = 0,
	kPlayerActionRaise = 1
};

enum Hoyle5DiscardActions {
	kDiscardActionKeep = 0,
	kDiscardActionDiscard = 1
};

// Hand types, as returned by PokerHand::whatAmI (FUN_1000_44c2). Higher values
// are stronger hands, which is what lets the winner check compare them directly.
enum Hoyle5HandType {
	kHandTypeRoyalFlush = 1 << 8,    // 256, ace-high straight flush
	kHandTypeStraightFlush = 1 << 7, // 128, straight flush
	kHandTypeFourOfAKind = 1 << 6,   //  64, four of a kind
	kHandTypeFullHouse = 1 << 5,     //  32, full house
	kHandTypeFlush = 1 << 4,         //  16, flush
	kHandTypeStraight = 1 << 3,      //   8, straight
	kHandTypeThreeOfAKind = 1 << 2,  //   4, three of a kind
	kHandTypeTwoPairs = 1 << 1,      //   2, two pairs
	kHandTypeOnePair = 1 << 0,       //   1, one pair
	kHandTypeHighCard = 0            //   0, high card
};

enum Hoyle5PokerData {
	kOperation = 0,
	kTotalChips = 1,
	kCurrentPot = 2,
	kCurrentBet = 3,
	kTotalChipsPlayer1 = 4,
	kTotalChipsPlayer2 = 5,
	kTotalChipsPlayer3 = 6,
	kTotalChipsPlayer4 = 7,
	kStatusPlayer1 = 8,
	kStatusPlayer2 = 9,
	kStatusPlayer3 = 10,
	kStatusPlayer4 = 11,
	kTotalBetPlayer1 = 12,
	kTotalBetPlayer2 = 13,
	kTotalBetPlayer3 = 14,
	kTotalBetPlayer4 = 15,
	kAmountToCall = 16,  // chips the current player must add to stay in
	kCurrentPlayer = 17, // hand number
	kCurrentStage = 18,  // Stage 1: Card changes, 2: Betting
	kCard0 = 19,
	kSuit0 = 20,
	kCard1 = 21,
	kSuit1 = 22,
	kCard2 = 23,
	kSuit2 = 24,
	kCard3 = 25,
	kSuit3 = 26,
	kCard4 = 27,
	kSuit4 = 28,
	// 19 - 28: current player's cards (number + suit)
	// 29 - 38: next clockwise player's cards (number + suit)
	// 39 - 48: next clockwise player's cards (number + suit)
	// 49 - 58: next clockwise player's cards (number + suit)
	kUnkVar = 59, // set by FUN_1000_0df8 to global 906
	// ---- Return values - start ---------------------------
	kPlayerAction = 60,   // flag, checked by FUN_1000_2016
	kWhatAmIResult = 61,  // hand type bitmask (see Hoyle5HandType)
	kWinningPlayers = 62, // bitmask, winning players (0000 - 1111 binary)
	kDiscardCard0 = 63,   // flag, checked by PokerHand::think
	kDiscardCard1 = 64,   // flag, checked by PokerHand::think
	kDiscardCard2 = 65,   // flag, checked by PokerHand::think
	kDiscardCard3 = 66,   // flag, checked by PokerHand::think
	kDiscardCard4 = 67,   // flag, checked by PokerHand::think
	// ---- Return values - end -----------------------------
	kConfidencePlayer1 = 68, // 0x8c array; clamped 0 - 3 by the DLL entry point
	// 69 - 71: confidence for the other players
	kAggressionPlayer1 = 72, // 0x94 array; adjusted by chip stack in the entry point
	// 73 - 75: aggression for the other players
	// 77 is a random number (0 - 32767)
	kLastRaise1 = 78,
	kLastRaise2 = 79,
	kLastRaise3 = 80,
	kLastRaise4 = 81,
	kLastSaw1 = 82,
	kLastSaw2 = 83,
	kLastSaw3 = 84,
	kLastSaw4 = 85,
	kTookCard1 = 86,
	kTookCard2 = 87,
	kTookCard3 = 88,
	kTookCard4 = 89
	// 90 is a number
};

// A single card, with its rank normalized so that aces are always high (14).
struct PokerCard {
	int rank; // 2 - 14
	int suit; // see Hoyle5PokerSuits
};

// The classified result of a five card hand. The tie-break ranks list the
// significant ranks in descending priority order, which mirrors how the DLL
// reorders its card list before comparing two hands of the same type:
// - four of a kind: quad rank, then kicker
// - full house:     trip rank, then pair rank
// - three of a kind:trip rank, then the two kickers (descending)
// - two pairs:      high pair, low pair, then kicker
// - one pair:       pair rank, then the three kickers (descending)
// - straights:      the straight's high card (a wheel counts as five high)
// - flush/highcard: all five ranks (descending)
struct PokerHandInfo {
	int handType;
	int tieBreak[5];
};

#ifdef DEBUG_POKER_LOGIC
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
	else if (card == 14 || card == 1)
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

void debugInputData(SciArray* data) {
	debug("Player %d's turn", data->getAsInt16(kCurrentPlayer));

	debug("Pot: %d, bet: %d", data->getAsInt16(kCurrentPot), data->getAsInt16(kCurrentBet));

	debug("Chips: %d %d %d %d - %d in total",
		data->getAsInt16(kTotalChipsPlayer1),
		data->getAsInt16(kTotalChipsPlayer2),
		data->getAsInt16(kTotalChipsPlayer3),
		data->getAsInt16(kTotalChipsPlayer4),
		data->getAsInt16(kTotalChips)
	);

	debug("Player status: %d %d %d %d",
		data->getAsInt16(kStatusPlayer1),
		data->getAsInt16(kStatusPlayer2),
		data->getAsInt16(kStatusPlayer3),
		data->getAsInt16(kStatusPlayer4)
	);

	for (int player = 0; player < 4; player++) {
		debug("Player %d cards:", player);
		for (int i = 19 + player * 10; i < 29 + player * 10; i += 2) {
			if (data->getAsInt16(i) > 0)
				debug("- %s", getCardDescription(data->getAsInt16(i), data->getAsInt16(i + 1)).c_str());
		}
	}

	for (int i = 0; i < data->size(); i++) {
		if (i >= kTotalChipsPlayer1 && i <= kTotalChipsPlayer4)
			continue;
		if (i >= 8 && i <= 11)
			continue;
		if (i >= 19 && i <= 58)
			continue;

		if (data->getAsInt16(i) != 0)
			debug("%d: %d", i, data->getAsInt16(i));
	}
}

#endif

// Aces are stored as either 1 or 14 depending on the caller, but are always the
// highest valued card in the DLL's evaluator.
static int getCardValue(int card) {
	return card == 1 ? 14 : card;
}

// Reads a player's five cards from the shared array into a normalized form.
static void readPlayerCards(SciArray *data, int player, PokerCard cards[5]) {
	for (int i = 0; i < 5; i++) {
		cards[i].rank = getCardValue(data->getAsInt16(kCard0 + 10 * player + i * 2));
		cards[i].suit = data->getAsInt16(kSuit0 + 10 * player + i * 2);
	}
}

static bool pokerCardRankGreater(const PokerCard &a, const PokerCard &b) {
	return a.rank > b.rank;
}

// A distinct rank together with how many cards share it, used to build the
// tie-break ordering (sorted by count, then rank, both descending).
struct RankGroup {
	int rank;
	int count;
};

static bool rankGroupGreater(const RankGroup &a, const RankGroup &b) {
	if (a.count != b.count)
		return a.count > b.count;
	return a.rank > b.rank;
}

// Classifies a five card hand, mirroring PokerHand::whatAmI (FUN_1000_44c2).
static PokerHandInfo classifyHand(const PokerCard cardsIn[5]) {
	PokerHandInfo info;
	info.handType = kHandTypeHighCard;
	for (int i = 0; i < 5; i++)
		info.tieBreak[i] = 0;

	PokerCard cards[5];
	for (int i = 0; i < 5; i++)
		cards[i] = cardsIn[i];

	// Sort the cards in descending rank order.
	Common::sort(cards, cards + 5, pokerCardRankGreater);

	// A flush is five cards of the same suit.
	bool isFlush = true;
	for (int i = 1; i < 5; i++) {
		if (cards[i].suit != cards[0].suit)
			isFlush = false;
	}

	// A straight requires five distinct, consecutive ranks. The DLL also treats
	// A-2-3-4-5 (the "wheel") as a five-high straight.
	bool distinct = true;
	for (int i = 0; i < 4; i++) {
		if (cards[i].rank == cards[i + 1].rank)
			distinct = false;
	}

	bool isStraight = false;
	int straightHigh = 0;
	if (distinct) {
		if (cards[0].rank - cards[4].rank == 4) {
			isStraight = true;
			straightHigh = cards[0].rank;
		} else if (cards[0].rank == 14 && cards[1].rank == 5 &&
		           cards[2].rank == 4 && cards[3].rank == 3 && cards[4].rank == 2) {
			isStraight = true;
			straightHigh = 5;
		}
	}

	// Group the cards by rank, sorted by group size then rank (both descending).
	RankGroup groups[5];
	int groupCount = 0;
	for (int i = 0; i < 5; i++) {
		int j;
		for (j = 0; j < groupCount; j++) {
			if (groups[j].rank == cards[i].rank) {
				groups[j].count++;
				break;
			}
		}
		if (j == groupCount) {
			groups[groupCount].rank = cards[i].rank;
			groups[groupCount].count = 1;
			groupCount++;
		}
	}
	Common::sort(groups, groups + groupCount, rankGroupGreater);

	int largestGroup = groups[0].count;
	bool hasFullHouse = (groupCount == 2 && largestGroup == 3);

	if (isStraight && isFlush) {
		// An ace-high straight flush is a royal flush.
		info.handType = (straightHigh == 14) ? kHandTypeRoyalFlush : kHandTypeStraightFlush;
	} else if (largestGroup == 4) {
		info.handType = kHandTypeFourOfAKind;
	} else if (hasFullHouse) {
		info.handType = kHandTypeFullHouse;
	} else if (isFlush) {
		info.handType = kHandTypeFlush;
	} else if (isStraight) {
		info.handType = kHandTypeStraight;
	} else if (largestGroup == 3) {
		info.handType = kHandTypeThreeOfAKind;
	} else if (groupCount == 3 && largestGroup == 2) {
		info.handType = kHandTypeTwoPairs;
	} else if (largestGroup == 2) {
		info.handType = kHandTypeOnePair;
	} else {
		info.handType = kHandTypeHighCard;
	}

	// Fill in the tie-break ranks.
	if (isStraight && !hasFullHouse && largestGroup < 3) {
		// Straights (and straight flushes) only differ by their high card.
		int high = straightHigh;
		for (int i = 0; i < 5; i++)
			info.tieBreak[i] = high - i;
	} else {
		for (int i = 0; i < groupCount; i++)
			info.tieBreak[i] = groups[i].rank;
	}

	return info;
}

// Compares two classified hands. Returns 1 if a is stronger, -1 if b is
// stronger and 0 if they tie (a split pot). This is the showdown comparison
// used by the DLL (FUN_1000_530a / FUN_1000_5c56).
static int compareHands(const PokerHandInfo &a, const PokerHandInfo &b) {
	if (a.handType != b.handType)
		return a.handType > b.handType ? 1 : -1;

	for (int i = 0; i < 5; i++) {
		if (a.tieBreak[i] != b.tieBreak[i])
			return a.tieBreak[i] > b.tieBreak[i] ? 1 : -1;
	}

	return 0;
}

// Classifies a player's hand and returns its type bitmask.
static int checkHand(SciArray *data, int player = 0) {
	PokerCard cards[5];
	readPlayerCards(data, player, cards);
	return classifyHand(cards).handType;
}

// Determines the winning player(s) at showdown, mirroring FUN_1000_17e4.
// Returns a bitmask of the winners (more than one bit is set on a split pot) and
// stores the winning hand type in the shared array. Folded players (whose status
// is -1) are ignored.
static int getWinners(SciArray *data) {
	int winners = 0;
	PokerHandInfo best;
	best.handType = -1;

	for (int player = 0; player < 4; player++) {
		if (data->getAsInt16(kStatusPlayer1 + player) == -1)
			continue; // folded

		PokerCard cards[5];
		readPlayerCards(data, player, cards);
		PokerHandInfo info = classifyHand(cards);

		if (winners == 0) {
			best = info;
			winners = 1 << player;
		} else {
			int cmp = compareHands(info, best);
			if (cmp > 0) {
				best = info;
				winners = 1 << player;
			} else if (cmp == 0) {
				winners |= 1 << player;
			}
		}
	}

	if (best.handType >= 0)
		data->setFromInt16(kWhatAmIResult, best.handType);

	return winners;
}

// Returns the current player's confidence value (the 0x8c array), which the DLL
// entry point clamps to the 0 - 3 range. It gates whether the AI chases a
// gutshot straight draw.
static int getConfidence(SciArray *data, int player) {
	int value = data->getAsInt16(kConfidencePlayer1 + player);
	return CLIP(value, 0, 3);
}

// Detects a four-card flush draw. If four of the five cards share a suit, the
// index of the odd (off-suit) card is returned so it can be discarded, mirroring
// FUN_1000_5062. Returns -1 if there is no flush draw.
static int findFlushDrawDiscard(const PokerCard cards[5]) {
	int suitCounts[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < 5; i++)
		suitCounts[cards[i].suit]++;

	for (int suit = 0; suit < 4; suit++) {
		if (suitCounts[suit] == 4) {
			for (int i = 0; i < 5; i++) {
				if (cards[i].suit != suit)
					return i;
			}
		}
	}

	return -1;
}

// Detects a four-card straight draw, mirroring FUN_1000_511c. If four of the
// cards can be completed into a straight, the index of the odd card is returned
// (so it can be discarded) and openEnded is set to distinguish an open-ended
// draw (four consecutive ranks) from a gutshot (an inside gap). Returns -1 if
// there is no straight draw. Aces are considered both high and low.
static int findStraightDrawDiscard(const PokerCard cards[5], bool &openEnded) {
	openEnded = false;

	// Try dropping each card and testing whether the remaining four form a
	// straight draw, preferring an open-ended draw over a gutshot.
	int gutshotDiscard = -1;

	for (int drop = 0; drop < 5; drop++) {
		for (int aceLow = 0; aceLow < 2; aceLow++) {
			int ranks[4];
			int n = 0;
			for (int i = 0; i < 5; i++) {
				if (i == drop)
					continue;
				int rank = cards[i].rank;
				if (aceLow && rank == 14)
					rank = 1;
				ranks[n++] = rank;
			}

			// The four ranks must be distinct.
			bool ok = true;
			int lowest = ranks[0], highest = ranks[0];
			for (int i = 0; i < 4; i++) {
				for (int j = i + 1; j < 4; j++) {
					if (ranks[i] == ranks[j])
						ok = false;
				}
				lowest = MIN(lowest, ranks[i]);
				highest = MAX(highest, ranks[i]);
			}
			if (!ok)
				continue;

			int span = highest - lowest;
			if (span == 3) {
				// Four consecutive ranks: an open-ended draw.
				openEnded = true;
				return drop;
			}
			if (span == 4 && gutshotDiscard == -1) {
				// Four ranks with a single inside gap: a gutshot.
				gutshotDiscard = drop;
			}
		}
	}

	return gutshotDiscard;
}

// Implements the discard AI (op 3, FUN_1000_1dc0 / PokerHand::think). It decides
// which of the current player's cards to keep and which to discard, then writes
// a keep/discard flag for each card into the shared array.
static void handleDiscard(SciArray *data) {
	int player = data->getAsInt16(kCurrentPlayer);

	// The acting player's cards are always in the first block (indices 19-28);
	// kCurrentPlayer only indexes the per-player state arrays (mood etc.).
	PokerCard cards[5];
	readPlayerCards(data, 0, cards);

	PokerHandInfo info = classifyHand(cards);

	// Count how many cards share each rank, so complete groups can be kept.
	int rankCounts[5];
	for (int i = 0; i < 5; i++) {
		rankCounts[i] = 0;
		for (int j = 0; j < 5; j++) {
			if (cards[j].rank == cards[i].rank)
				rankCounts[i]++;
		}
	}

	bool discard[5] = { false, false, false, false, false };

	switch (info.handType) {
	case kHandTypeRoyalFlush:
	case kHandTypeStraightFlush:
	case kHandTypeFullHouse:
	case kHandTypeFlush:
	case kHandTypeStraight:
		// A made hand of five cards: keep everything.
		break;

	case kHandTypeFourOfAKind:
	case kHandTypeThreeOfAKind:
	case kHandTypeTwoPairs:
	case kHandTypeOnePair:
		// Keep the paired cards, discard the odd ones out. This keeps four
		// cards for quads/two pairs, three for trips and two for a pair, which
		// matches the keep counts the DLL derives in FUN_1000_56bc.
		for (int i = 0; i < 5; i++) {
			if (rankCounts[i] == 1)
				discard[i] = true;
		}
		break;

	case kHandTypeHighCard: {
		int flushDrawDiscard = findFlushDrawDiscard(cards);
		if (flushDrawDiscard >= 0) {
			// Chase the flush by discarding the single off-suit card.
			discard[flushDrawDiscard] = true;
			break;
		}

		bool openEnded = false;
		int straightDrawDiscard = findStraightDrawDiscard(cards, openEnded);
		// An open-ended draw is always chased; a gutshot only when the player is
		// confident (its clamped confidence value has reached the maximum).
		if (straightDrawDiscard >= 0 && (openEnded || getConfidence(data, player) >= 3)) {
			discard[straightDrawDiscard] = true;
			break;
		}

		// No draw worth chasing: keep the highest one or two cards. The DLL
		// keeps a single card when the top card is a king or ace, and otherwise
		// occasionally does so as well.
		int highestRank = 0;
		for (int i = 0; i < 5; i++)
			highestRank = MAX(highestRank, cards[i].rank);

		bool keepOne = (highestRank >= 13) || (g_sci->getRNG().getRandomNumber(32767) > 0x6000);
		int keepCount = keepOne ? 1 : 2;

		// Discard everything but the highest keepCount cards.
		for (int i = 0; i < 5; i++)
			discard[i] = true;
		for (int kept = 0; kept < keepCount; kept++) {
			int bestIndex = -1;
			for (int i = 0; i < 5; i++) {
				if (discard[i] && (bestIndex == -1 || cards[i].rank > cards[bestIndex].rank))
					bestIndex = i;
			}
			if (bestIndex >= 0)
				discard[bestIndex] = false;
		}
		break;
	}

	default:
		break;
	}

	for (int i = 0; i < 5; i++)
		data->setFromInt16(kDiscardCard0 + i, discard[i] ? kDiscardActionDiscard : kDiscardActionKeep);
}

// Returns a rough win expectation (0 - 100) for a classified hand, used by the
// simplified betting AI below.
static int getHandStrength(const PokerHandInfo &info) {
	switch (info.handType) {
	case kHandTypeRoyalFlush:    return 100;
	case kHandTypeStraightFlush: return 99;
	case kHandTypeFourOfAKind:   return 96;
	case kHandTypeFullHouse:     return 92;
	case kHandTypeFlush:         return 82;
	case kHandTypeStraight:      return 72;
	case kHandTypeThreeOfAKind:  return 62;
	case kHandTypeTwoPairs:      return 48;
	case kHandTypeOnePair:
		// A high pair is worth noticeably more than a low one.
		return 26 + (info.tieBreak[0] - 2);
	default:
		// High card: scale by the highest card held.
		return info.tieBreak[0] - 2;
	}
}

// Implements the betting AI (op 1, FUN_1000_2016). The original DLL runs a
// recursive expected-value search over the possible betting sequences
// (FUN_1000_269c). AI folds when it cannot afford to continue with a weak
// hand, and otherwise weighs the hand's strength against the pot odds,
// adjusted by the player's aggression, to decide whether to check, call or
// raise.
static void handlePlayerAction(SciArray *data) {
	int player = data->getAsInt16(kCurrentPlayer);
	int chips = data->getAsInt16(kTotalChipsPlayer1 + player);
	int amountToCall = data->getAsInt16(kAmountToCall);
	int pot = data->getAsInt16(kCurrentPot);

	// As with the discard logic, the acting player's cards are in the first
	// block; kCurrentPlayer indexes the chip/bet/aggression arrays.
	PokerCard cards[5];
	readPlayerCards(data, 0, cards);
	PokerHandInfo info = classifyHand(cards);
	int strength = getHandStrength(info);

	// Confidence and aggression are derived by the DLL entry point from the
	// player's chip stack (a short stack plays cautiously, a big stack pushes).
	int aggression = data->getAsInt16(kAggressionPlayer1 + player);
	aggression = CLIP(aggression, 0, 3);

	// If the player cannot cover the call, only a strong hand keeps going.
	if (amountToCall > 0 && chips < amountToCall) {
		data->setFromInt16(kPlayerAction, strength >= 70 ? kPlayerActionCall : kPlayerActionFold);
		return;
	}

	// Pot odds: the fraction of the (post-call) pot the call would cost.
	int potOdds = 0;
	if (amountToCall > 0 && pot + amountToCall > 0)
		potOdds = (amountToCall * 100) / (pot + amountToCall);

	// A higher aggression raises the effective strength (and adds occasional
	// bluffs); the threshold to raise scales down with it.
	int effectiveStrength = strength + aggression * 6;
	int raiseThreshold = 78 - aggression * 6;

	if (amountToCall == 0) {
		// Betting is free: raise with a good hand (or a bluff), otherwise check.
		bool bluff = (int)g_sci->getRNG().getRandomNumber(99) < aggression * 5;
		if (effectiveStrength >= raiseThreshold || bluff)
			data->setFromInt16(kPlayerAction, kPlayerActionRaise);
		else
			data->setFromInt16(kPlayerAction, kPlayerActionCheck);
		return;
	}

	if (effectiveStrength < potOdds) {
		// The pot is not offering the right price. Fold, but bluff occasionally.
		bool bluff = (int)g_sci->getRNG().getRandomNumber(99) < aggression * 4;
		data->setFromInt16(kPlayerAction, bluff ? kPlayerActionCall : kPlayerActionFold);
		return;
	}

	if (effectiveStrength >= raiseThreshold)
		data->setFromInt16(kPlayerAction, kPlayerActionRaise);
	else
		data->setFromInt16(kPlayerAction, kPlayerActionCall);
}

reg_t hoyle5PokerEngine(SciArray *data) {
	int16 operation = data->getAsInt16(kOperation);

#ifdef DEBUG_POKER_LOGIC
	debug("*** Before running operation %d", operation);
	debugInputData(data);
#endif

	switch (operation) {
	case kCheckPlayerAction:
		handlePlayerAction(data);
		break;
	case kCheckWinner:
		data->setFromInt16(kWinningPlayers, getWinners(data));
		break;
	case kCheckDiscard:
		handleDiscard(data);
		break;
	case kCheckHand:
		data->setFromInt16(kWhatAmIResult, checkHand(data));
		break;
	default:
		error("Unknown Poker logic operation: %d", operation);
		break;
	}

#ifdef DEBUG_POKER_LOGIC
	debug("*** After running operation %d", operation);
	debugInputData(data);
#endif

	return TRUE_REG;
}

#endif

}
