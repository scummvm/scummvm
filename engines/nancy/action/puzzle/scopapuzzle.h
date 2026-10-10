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

#ifndef NANCY_ACTION_SCOPAPUZZLE_H
#define NANCY_ACTION_SCOPAPUZZLE_H

#include "graphics/font.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Scopa is the Italian card game Nancy plays against Enrico in Nancy18. Each
// player holds three cards and four lie on the table. A card either takes table
// cards that add up to its value or is left on the table, and taking the last
// cards of the table is a "scopa". Once the deck runs out the round is scored:
// most cards, most coins, the seven of coins, the best "primes" and one point
// per scopa.
//
// The record plays one round and then ends, with an event flag telling whether
// somebody has won the match. The points are kept under the puzzle's name for
// the next round.
//
// The cards lie in numbered piles, and get from one to another through a queue
// of moves. A card doesn't travel: it fades out where it was and in where it
// goes, and a pile without places of its own leaves the card where it is.
class ScopaPuzzle : public PuzzleRecord {
public:
	ScopaPuzzle() : PuzzleRecord(7) {}
	virtual ~ScopaPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ScopaPuzzle"; }

	static const uint kNumPlayers = 2;

	enum { kHuman = 0, kOpponent = 1 };

	// A player's piles follow each other: hand, taken, trick
	enum Pile {
		kHumanHand		= 0,
		kHumanTaken		= 1,
		kHumanTrick		= 2,	// taken since the last deal
		kOpponentHand	= 3,
		kOpponentTaken	= 4,
		kOpponentTrick	= 5,
		kDeck			= 6,
		kTable			= 7,
		kLaidOut		= 8,	// a player's cards, shown for the scoring
		kPrimes			= 9,	// the best card of every suit, of both players
		kNumPiles		= 10,
		kPrimeValues	= 10,	// only places, for the numbers next to those cards
		kNumSlotLists	= 11
	};

	enum CardMode {
		kFaceUp			= 0,
		kFaceDown		= 1,
		kHidden			= 2,
		kHighlighted	= 3
	};

	enum Phase {
		kDeal,
		kDealing,
		kPlaying,
		kTally,
		kScorePlayers,
		kScorePrimes
	};

	// The cards a line of the scoring is about
	enum Highlight {
		kAllCards,
		kCoinCards,
		kSevenOfCoinsCard
	};

	// Why a play isn't allowed
	enum PlayError {
		kPlayAllowed,
		kWrongSum,			// the cards don't add up to the card played
		kHasToTake,			// a card that can take something may not be left on the table
		kHasToTakeOneCard	// a single card of the same value comes before several
	};

	// The lines a player has, in the order of the data
	enum PlayerSound {
		kSoundScopa,
		kSoundLastCardsLong,
		kSoundLastCards,
		kSoundMostCards,
		kSoundMostCoins,
		kSoundSevenOfCoins,
		kSoundBestPrimes,
		kSoundNoPoints,
		kSoundWin,
		kNumPlayerSounds
	};

	// Something that shows up and goes away over a set time
	struct Fader {
		uint32 fadeInTime = 0;
		uint32 fadeOutTime = 0;
		bool isShown = false;
		bool isFading = false;
		float alpha = 0.0f;
		uint32 lastTime = 0;

		void show(bool instantly);
		void hide(bool instantly);
		float update();
		bool isVisible() { return update() > 0.0f; }
	};

	struct CardState {
		CardMode mode = kHidden;
		Common::Rect rect;
		Fader fader;
	};

	// The state a card had before stays on screen until it has faded away
	struct Card {
		CardState current;
		CardState previous;

		bool isFading();
	};

	static const int32 kWaitForCard = -1;

	struct Move {
		int srcPile = 0;
		uint srcIndex = 0;
		int32 delay = 0;	// until the next move, in milliseconds
		int dstPile = 0;
		CardMode mode = kFaceUp;
	};

	// What holds up the game. A player can get points once it is over.
	struct Wait {
		enum Type { kCard, kTime, kSound };

		Type type = kTime;
		int card = -1;
		uint32 endTime = 0;
		SoundDescription sound;
		int player = -1;
		int32 points = 0;
	};

	struct Player {
		// -- File data --
		uint16 cursor = 0;
		int16 winFlag = -1;
		int16 nextStarterFlag = -1;	// set after a round the other player started
		Common::Rect pileSrc;
		Common::Rect pileDest;
		Common::Rect pointsRect;
		int32 fontID = 0;
		int32 colorID = 0;
		RandomSoundBlock sounds[kNumPlayerSounds];

		// -- Runtime --
		int32 points = 0;
		int32 numScopas = 0;
		Fader pileFader;

		// What the cards taken in the round come to
		int32 numCards = 0;
		int32 numCoins = 0;
		int32 primes = 0;
		Common::Array<int> bestPrimeCards;	// by suit, -1 for none
		bool hasScoreLine = false;
		bool scoresMostCards = false;
		bool scoresMostCoins = false;
		bool scoresSevenOfCoins = false;
		bool scoresPrimes = false;
	};

	struct Play {
		int card = -1;		// from the hand
		uint handIndex = 0;
		Common::Array<uint> taken;	// places on the table, in order
		int worth = 0;
	};

	static void readPlayer(Common::SeekableReadStream &stream, Player &player);
	static void readSlots(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &slots);

	static int getHandPile(uint playerID) { return playerID * 3; }
	static int getTakenPile(uint playerID) { return playerID * 3 + 1; }
	static int getTrickPile(uint playerID) { return playerID * 3 + 2; }
	static bool hasFixedPlaces(int pile);

	uint getOtherPlayer(uint playerID) const { return (playerID + 1) % kNumPlayers; }
	uint countCards(int pile) const;

	// Card moves and what the game waits for
	void setCardState(int cardID, CardMode mode, const Common::Rect &rect);
	void setCardMode(int cardID, CardMode mode);
	int doMove(const Move &move);
	void queueMove(int srcPile, uint srcIndex, int32 delay, int dstPile, CardMode mode);
	bool runMoves();
	bool updateWaits();
	void waitForTime(uint32 time, int player = -1, int32 points = 0);
	void waitForSound(const RandomSoundBlock &sound, int player = -1, int32 points = 0);
	void startDealSound();
	void stopDealSound();

	// The game
	void runPhase();
	void dealCards();
	void startTurn();
	void switchPlayer();
	PlayError checkPlay(const Play &play) const;
	void findPlays(Common::Array<Play> &plays) const;
	void makePlay(const Play &play);
	void afterPlay();
	void moveTricksToTaken();
	void takeLastCards();
	void showPilePictures(bool show);
	void updateButtons();

	// The scoring
	void tallyPlayer(Player &player, int takenPile);
	void tallyRound();
	void layOutTakenCards(uint playerID);
	void highlightLaidOutCards(Highlight highlight);
	void movePrimes(uint playerID);
	bool scorePlayerStep();
	bool scorePrimesStep();
	void finishRound();

	void findSelectedCards(int pile, Common::Array<uint> &indices) const;
	int getHoveredCard(int pile, const Common::Point &pos) const;
	void clickCard(int pile, uint index);
	void tryPlay(bool sayWhyNotToDiscard);

	void blendPixel(int x, int y, byte a, byte r, byte g, byte b);
	void blitFaded(const Graphics::ManagedSurface &image, const Common::Rect &src, const Common::Point &dest, uint32 argb, float alpha);
	void drawCardState(int cardID, CardState &state);
	void drawText(const Common::String &text, const Common::Rect &rect, uint fontID, uint colorID, Graphics::TextAlign align);
	void redraw();

	// -- File data --
	Common::String _saveName;
	int32 _pointsToWin = 0;
	int16 _roundOverFlag = -1;
	int16 _redealFlag = -1;
	int32 _primesFontID = 0;
	int32 _primesColorID = 0;
	Player _players[kNumPlayers];
	Common::Rect _takeSrc;
	Common::Rect _takeDest;
	Common::Rect _discardSrc;
	Common::Rect _discardDest;
	int32 _thinkTime = 0;	// in milliseconds
	int32 _dealTime = 0;
	int32 _layOutTime = 0;
	RandomSoundBlock _dealSound;
	RandomSoundBlock _discardSound;
	RandomSoundBlock _takeSound;
	RandomSoundBlock _wrongSumSound;
	RandomSoundBlock _hasToTakeSound;
	RandomSoundBlock _hasToTakeOneCardSound;
	RandomSoundBlock _noLastScopaSound;
	RandomSoundBlock _primesSound;
	RandomSoundBlock _primesTieSound;
	RandomSoundBlock _scopasSound;
	Common::Array<Common::Rect> _slots[kNumSlotLists];
	Common::Path _cardsImageName;
	int32 _fadeInTime = 0;
	int32 _fadeOutTime = 0;
	uint32 _highlightColor = 0;	// ARGB
	Common::Rect _cardBackSrc;
	Common::Array<Common::Rect> _cardSrcs;

	// -- Runtime --
	Graphics::ManagedSurface _cardsImage;
	Common::Array<Card> _cards;
	Common::Array<int> _piles[kNumPiles];	// card ids, -1 for a place that got empty
	Common::Array<Move> _moves;
	Common::Array<Wait> _waits;

	Phase _phase = kDeal;
	bool _isBusy = true;
	uint _starter = kHuman;
	uint _currentPlayer = kHuman;
	int _lastTaker = -1;
	bool _needsRedeal = false;
	bool _hasCheckedScopa = false;
	bool _hasSaidLastCardsLong = false;
	bool _hasSaidNoLastScopa = false;
	bool _showsPrimes = false;
	bool _showsPrimeTotals = false;

	bool _showsTake = false;
	bool _showsDiscard = false;
	Fader _takeFader;
	Fader _discardFader;

	SoundDescription _playingDealSound;
	SoundDescription _errorSound;
	bool _hasChanged = true;
	bool _isFading = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SCOPAPUZZLE_H
