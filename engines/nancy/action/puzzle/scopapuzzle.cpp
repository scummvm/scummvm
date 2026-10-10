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
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/scopapuzzle.h"

namespace Nancy {
namespace Action {

// What gets kept of a match between two rounds
enum {
	kSavedHumanPoints		= 0,
	kSavedOpponentPoints	= 1,
	kSavedHumanStarted		= 2,
	kSavedSaidLastCardsLong	= 3,
	kSavedSaidNoLastScopa	= 4,
	kNumSavedValues			= 5
};

static const uint kNumSuits = 4;
static const uint kNumValues = 10;
static const uint kNumCards = kNumSuits * kNumValues;
static const uint kHandSize = 3;
static const uint kNumTableCards = 4;

static const int kCoins = 0;	// the suit that counts for points
static const int kKing = 10;
static const int kSevenOfCoins = 6;

// Three kings among the first cards on the table make a round unplayable
static const uint kMaxKingsOnTable = 2;

static const uint16 kWaitCursor = 23;

static const uint32 kNoTint = 0xFFFFFFFF;
static const uint32 kPrimesLineColor = 0xFFC8C83C;

// The suits, in the order their best cards get shown in
static const int kPrimeSuitOrder[kNumSuits] = { 0, 1, 3, 2 };

static int getCardValue(int card) {
	return card % kNumValues + 1;
}

static int getCardSuit(int card) {
	return card / kNumValues;
}

// What a card counts for when the best one of each suit gets added up
static int getPrimeValue(int card) {
	static const int kPrimeValues[kNumValues] = { 16, 12, 13, 14, 15, 18, 21, 10, 10, 10 };
	return kPrimeValues[getCardValue(card) - 1];
}

// How much the opponent wants a card
static int getCardWorth(int card) {
	static const int kWorths[kNumValues] = { 12, 4, 5, 6, 7, 23, 34, 3, 2, 1 };
	return kWorths[getCardValue(card) - 1] + (getCardSuit(card) == kCoins ? 10 : 0);
}

// The opponent goes for the play with the most valuable card in it, then for
// the one that takes the most cards
struct ScopaPlayOrder {
	template<typename T>
	bool operator()(const T &a, const T &b) const {
		if (a.taken.empty() != b.taken.empty()) {
			return !a.taken.empty();
		}

		if (a.worth != b.worth) {
			return a.worth > b.worth;
		}

		if (a.taken.size() != b.taken.size()) {
			return a.taken.size() > b.taken.size();
		}

		if (a.handIndex != b.handIndex) {
			return a.handIndex > b.handIndex;
		}

		for (uint i = 0; i < a.taken.size(); ++i) {
			if (a.taken[i] != b.taken[i]) {
				return a.taken[i] < b.taken[i];
			}
		}

		return false;
	}
};

// The order a player's cards get laid out in: by suit, the most wanted first
struct ScopaCardOrder {
	bool operator()(int a, int b) const {
		if (getCardSuit(a) != getCardSuit(b)) {
			return getCardSuit(a) < getCardSuit(b);
		}

		return getCardWorth(a) > getCardWorth(b);
	}
};

void ScopaPuzzle::Fader::show(bool instantly) {
	isShown = true;

	if (fadeInTime == 0 || instantly) {
		isFading = false;
		alpha = 1.0f;
	} else {
		isFading = true;
		lastTime = g_nancy->getTotalPlayTime();
	}
}

void ScopaPuzzle::Fader::hide(bool instantly) {
	isShown = false;

	if (fadeOutTime == 0 || instantly) {
		isFading = false;
		alpha = 0.0f;
	} else {
		isFading = true;
		lastTime = g_nancy->getTotalPlayTime();
	}
}

float ScopaPuzzle::Fader::update() {
	if (!isFading) {
		return alpha;
	}

	const uint32 now = g_nancy->getTotalPlayTime();
	const float elapsed = now - lastTime;
	lastTime = now;

	if (isShown) {
		alpha += elapsed / fadeInTime;
		if (alpha >= 1.0f) {
			alpha = 1.0f;
			isFading = false;
		}
	} else {
		alpha -= elapsed / fadeOutTime;
		if (alpha <= 0.0f) {
			alpha = 0.0f;
			isFading = false;
		}
	}

	return alpha;
}

bool ScopaPuzzle::Card::isFading() {
	current.fader.update();
	previous.fader.update();
	return current.fader.isFading || previous.fader.isFading;
}

void ScopaPuzzle::readPlayer(Common::SeekableReadStream &stream, Player &player) {
	player.cursor = stream.readUint16LE();
	player.winFlag = stream.readSint16LE();
	player.nextStarterFlag = stream.readSint16LE();
	readExclusiveRect(stream, player.pileSrc);
	readExclusiveRect(stream, player.pileDest);
	readExclusiveRect(stream, player.pointsRect);
	player.fontID = stream.readSint32LE();
	player.colorID = stream.readSint32LE();

	for (uint i = 0; i < kNumPlayerSounds; ++i) {
		player.sounds[i].readData(stream);
	}
}

void ScopaPuzzle::readSlots(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &slots) {
	slots.resize(stream.readUint16LE());
	for (Common::Rect &slot : slots) {
		readExclusiveRect(stream, slot);
	}
}

void ScopaPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _saveName);
	_pointsToWin = stream.readSint32LE();
	_roundOverFlag = stream.readSint16LE();
	_redealFlag = stream.readSint16LE();
	readFilename(stream, _imageName);
	_primesFontID = stream.readSint32LE();
	_primesColorID = stream.readSint32LE();

	for (uint i = 0; i < kNumPlayers; ++i) {
		readPlayer(stream, _players[i]);
	}

	readExclusiveRect(stream, _takeSrc);
	readExclusiveRect(stream, _takeDest);
	readExclusiveRect(stream, _discardSrc);
	readExclusiveRect(stream, _discardDest);

	_thinkTime = stream.readSint32LE();
	_dealTime = stream.readSint32LE();
	_layOutTime = stream.readSint32LE();

	_dealSound.readData(stream);
	_discardSound.readData(stream);
	_takeSound.readData(stream);
	_wrongSumSound.readData(stream);
	_hasToTakeSound.readData(stream);
	_hasToTakeOneCardSound.readData(stream);
	_noLastScopaSound.readData(stream);
	_primesSound.readData(stream);
	_primesTieSound.readData(stream);
	_scopasSound.readData(stream);

	readSlots(stream, _slots[kHumanHand]);
	readSlots(stream, _slots[kOpponentHand]);
	readSlots(stream, _slots[kTable]);
	readSlots(stream, _slots[kLaidOut]);
	readSlots(stream, _slots[kPrimes]);
	readSlots(stream, _slots[kPrimeValues]);

	readFilename(stream, _cardsImageName);
	_fadeInTime = stream.readSint32LE();
	_fadeOutTime = stream.readSint32LE();
	_highlightColor = stream.readUint32BE();
	readExclusiveRect(stream, _cardBackSrc);
	readSlots(stream, _cardSrcs);

	// Hands for testing, two 40 character lines each
	stream.skip((stream.readUint16LE() / 2) * 80);
}

void ScopaPuzzle::init() {
	// The cards fade, so the surface has an alpha channel instead of a transparent color
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.clear(0);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	g_nancy->_resource->loadImage(_cardsImageName, _cardsImage);

	// The player who didn't start the last round starts this one
	const auto *data = (const SavedStateData *)NancySceneState.getPuzzleData(SavedStateData::getTag());
	assert(data);
	if (!_saveName.empty() && data->states.contains(_saveName) && data->states[_saveName].size() == kNumSavedValues) {
		const Common::Array<int32> &values = data->states[_saveName];
		_players[kHuman].points = values[kSavedHumanPoints];
		_players[kOpponent].points = values[kSavedOpponentPoints];
		_starter = values[kSavedHumanStarted] ? kOpponent : kHuman;
		_hasSaidLastCardsLong = values[kSavedSaidLastCardsLong];
		_hasSaidNoLastScopa = values[kSavedSaidNoLastScopa];
	}

	_cards.clear();
	_cards.resize(kNumCards);
	for (Card &card : _cards) {
		card.current.fader.fadeInTime = card.previous.fader.fadeInTime = _fadeInTime;
		card.current.fader.fadeOutTime = card.previous.fader.fadeOutTime = _fadeOutTime;
	}

	_takeFader.fadeInTime = _discardFader.fadeInTime = _fadeInTime;
	_takeFader.fadeOutTime = _discardFader.fadeOutTime = _fadeOutTime;

	Common::Array<int> &deck = _piles[kDeck];
	deck.resize(kNumCards);
	for (uint i = 0; i < kNumCards; ++i) {
		deck[i] = i;
	}

	for (uint i = kNumCards - 1; i > 0; --i) {
		SWAP(deck[i], deck[g_nancy->_randomSource->getRandomNumber(i)]);
	}

	_piles[kLaidOut].resize(kNumCards, -1);

	for (Player &player : _players) {
		player.bestPrimeCards.resize(kNumSuits, -1);
	}

	_currentPlayer = _starter;
	_phase = kDeal;
	_isBusy = true;
}

// A pile of this kind keeps the place of a card that left it
bool ScopaPuzzle::hasFixedPlaces(int pile) {
	return pile == kHumanHand || pile == kOpponentHand || pile == kTable || pile == kLaidOut;
}

uint ScopaPuzzle::countCards(int pile) const {
	uint count = 0;
	for (int card : _piles[pile]) {
		if (card != -1) {
			++count;
		}
	}

	return count;
}

// What the card showed until now fades out, unless the new picture just goes over it
void ScopaPuzzle::setCardState(int cardID, CardMode mode, const Common::Rect &rect) {
	Card &card = _cards[cardID];
	if (card.current.mode == mode && card.current.rect == rect) {
		return;
	}

	card.previous.mode = card.current.mode;
	card.previous.rect = card.current.rect;
	card.previous.fader.show(true);
	if (card.current.rect != rect || mode == kHidden) {
		card.previous.fader.hide(false);
	}

	card.current.fader.hide(true);
	card.current.fader.show(false);
	card.current.mode = mode;
	card.current.rect = rect;

	_hasChanged = true;
}

void ScopaPuzzle::setCardMode(int cardID, CardMode mode) {
	setCardState(cardID, mode, Common::Rect(_cards[cardID].current.rect));
}

// Returns the card that got moved, -1 when there is none in that place
int ScopaPuzzle::doMove(const Move &move) {
	Common::Array<int> &src = _piles[move.srcPile];
	if (move.srcIndex >= src.size() || src[move.srcIndex] == -1) {
		return -1;
	}

	const int cardID = src[move.srcIndex];
	if (hasFixedPlaces(move.srcPile)) {
		src[move.srcIndex] = -1;
	} else {
		src.remove_at(move.srcIndex);
	}

	Common::Array<int> &dst = _piles[move.dstPile];
	uint index = dst.size();

	if (move.dstPile == kLaidOut) {
		// A row for every suit
		index = getCardSuit(cardID) * kNumValues;
		for (int other : dst) {
			if (other != -1 && getCardSuit(other) == getCardSuit(cardID)) {
				++index;
			}
		}
	} else if (hasFixedPlaces(move.dstPile)) {
		for (uint i = 0; i < dst.size(); ++i) {
			if (dst[i] == -1) {
				index = i;
				break;
			}
		}
	}

	if (index < dst.size()) {
		dst[index] = cardID;
	} else {
		dst.push_back(cardID);
	}

	const Common::Array<Common::Rect> &slots = _slots[move.dstPile];
	if (index < slots.size()) {
		setCardState(cardID, move.mode, slots[index]);
	} else {
		setCardMode(cardID, move.mode);
	}

	return cardID;
}

void ScopaPuzzle::queueMove(int srcPile, uint srcIndex, int32 delay, int dstPile, CardMode mode) {
	Move move;
	move.srcPile = srcPile;
	move.srcIndex = srcIndex;
	move.delay = delay;
	move.dstPile = dstPile;
	move.mode = mode;
	_moves.push_back(move);
}

// The next move only starts once there is nothing left to wait for.
// Returns whether a card got moved.
bool ScopaPuzzle::runMoves() {
	bool hasMoved = false;

	while (!_moves.empty() && updateWaits()) {
		const Move move = _moves.front();
		_moves.remove_at(0);
		const int cardID = doMove(move);

		if (move.delay != kWaitForCard) {
			waitForTime(move.delay);
		} else if (cardID != -1) {
			Wait wait;
			wait.type = Wait::kCard;
			wait.card = cardID;
			_waits.push_back(wait);
		}

		hasMoved = true;
	}

	return hasMoved;
}

// Returns whether everything the game waited for is over
bool ScopaPuzzle::updateWaits() {
	while (!_waits.empty()) {
		const Wait &wait = _waits.front();

		bool isOver = true;
		switch (wait.type) {
		case Wait::kCard:
			isOver = !_cards[wait.card].isFading();
			break;
		case Wait::kTime:
			isOver = g_nancy->getTotalPlayTime() >= wait.endTime;
			break;
		case Wait::kSound:
			isOver = !g_nancy->_sound->isSoundPlaying(wait.sound);
			break;
		}

		if (!isOver) {
			return false;
		}

		if (wait.player != -1) {
			_players[wait.player].points += wait.points;
			_hasChanged = true;
		}

		_waits.remove_at(0);
	}

	return true;
}

void ScopaPuzzle::waitForTime(uint32 time, int player, int32 points) {
	Wait wait;
	wait.type = Wait::kTime;
	wait.endTime = g_nancy->getTotalPlayTime() + time;
	wait.player = player;
	wait.points = points;
	_waits.push_back(wait);
}

void ScopaPuzzle::waitForSound(const RandomSoundBlock &sound, int player, int32 points) {
	Wait wait;
	wait.type = Wait::kSound;
	wait.sound = playSoundBlock(sound);
	wait.player = player;
	wait.points = points;
	_waits.push_back(wait);
}

// The sound of cards being dealt goes on until the last one is out
void ScopaPuzzle::startDealSound() {
	if (_dealSound.names.empty()) {
		return;
	}

	_playingDealSound.name = _dealSound.names[g_nancy->_randomSource->getRandomNumber(_dealSound.names.size() - 1)];
	_playingDealSound.channelID = _dealSound.channel;
	_playingDealSound.numLoops = _dealSound.numLoops;
	_playingDealSound.volume = _dealSound.volume;

	g_nancy->_sound->loadSound(_playingDealSound);
	g_nancy->_sound->playSound(_playingDealSound);
}

void ScopaPuzzle::stopDealSound() {
	g_nancy->_sound->stopSound(_playingDealSound);
	_playingDealSound = SoundDescription();
}

// Three cards each, one at a time, and four more on the table when the round begins
void ScopaPuzzle::dealCards() {
	const uint other = getOtherPlayer(_starter);

	for (uint i = 0; i < kHandSize; ++i) {
		queueMove(kDeck, 0, _dealTime, getHandPile(_starter), _starter == kHuman ? kFaceUp : kFaceDown);
		queueMove(kDeck, 0, _dealTime, getHandPile(other), other == kHuman ? kFaceUp : kFaceDown);
	}

	if (countCards(kTable) == 0 && countCards(kHumanTaken) == 0 && countCards(kOpponentTaken) == 0) {
		const Common::Array<int> &deck = _piles[kDeck];
		const uint firstTableCard = kHandSize * kNumPlayers;
		uint numKings = 0;

		for (uint i = firstTableCard; i < firstTableCard + kNumTableCards && i < deck.size(); ++i) {
			if (getCardValue(deck[i]) == kKing) {
				++numKings;
			}
		}

		if (numKings > kMaxKingsOnTable) {
			_needsRedeal = true;
		}

		for (uint i = 0; i < kNumTableCards; ++i) {
			queueMove(kDeck, 0, _dealTime, kTable, kFaceUp);
		}
	}

	_isBusy = true;
}

ScopaPuzzle::PlayError ScopaPuzzle::checkPlay(const Play &play) const {
	const Common::Array<int> &table = _piles[kTable];
	const int value = getCardValue(play.card);

	// The cards on the table that could be taken with this one
	Common::Array<int> values;
	for (int card : table) {
		if (card != -1) {
			values.push_back(getCardValue(card));
		}
	}

	if (play.taken.empty()) {
		for (uint mask = 1; mask < (1U << values.size()); ++mask) {
			int sum = 0;
			for (uint i = 0; i < values.size(); ++i) {
				if (mask & (1 << i)) {
					sum += values[i];
				}
			}

			if (sum == value) {
				return kHasToTake;
			}
		}

		return kPlayAllowed;
	}

	int sum = 0;
	for (uint index : play.taken) {
		sum += getCardValue(table[index]);
	}

	if (sum != value) {
		return kWrongSum;
	}

	if (play.taken.size() > 1) {
		for (int tableValue : values) {
			if (tableValue == value) {
				return kHasToTakeOneCard;
			}
		}
	}

	return kPlayAllowed;
}

// Every play the rules allow the current player
void ScopaPuzzle::findPlays(Common::Array<Play> &plays) const {
	const Common::Array<int> &table = _piles[kTable];
	const Common::Array<int> &hand = _piles[getHandPile(_currentPlayer)];

	Common::Array<uint> places;
	for (uint i = 0; i < table.size(); ++i) {
		if (table[i] != -1) {
			places.push_back(i);
		}
	}

	for (uint handIndex = 0; handIndex < hand.size(); ++handIndex) {
		if (hand[handIndex] == -1) {
			continue;
		}

		for (uint mask = 0; mask < (1U << places.size()); ++mask) {
			Play play;
			play.card = hand[handIndex];
			play.handIndex = handIndex;
			play.worth = getCardWorth(play.card);

			for (uint i = 0; i < places.size(); ++i) {
				if (mask & (1 << i)) {
					play.taken.push_back(places[i]);
					play.worth = MAX(play.worth, getCardWorth(table[places[i]]));
				}
			}

			if (checkPlay(play) == kPlayAllowed) {
				plays.push_back(play);
			}
		}
	}
}

// The cards a player takes go out of sight, a card that takes nothing goes to the table
void ScopaPuzzle::makePlay(const Play &play) {
	int dstPile = kTable;
	CardMode mode = kFaceUp;

	if (play.taken.empty()) {
		playSoundBlock(_discardSound);
	} else {
		_lastTaker = _currentPlayer;
		dstPile = getTrickPile(_currentPlayer);
		mode = kHidden;
		playSoundBlock(_takeSound);
	}

	queueMove(getHandPile(_currentPlayer), play.handIndex, 0, dstPile, mode);
	for (uint index : play.taken) {
		queueMove(kTable, index, 0, dstPile, mode);
	}

	_moves.back().delay = kWaitForCard;
	_isBusy = true;
}

// The opponent shows what it is going to play, and then takes its time
void ScopaPuzzle::startTurn() {
	if (_currentPlayer != kOpponent) {
		return;
	}

	Common::Array<Play> plays;
	findPlays(plays);
	if (plays.empty()) {
		return;
	}

	// With nothing to take, the card that is wanted the least gets left on the table
	Common::sort(plays.begin(), plays.end(), ScopaPlayOrder());
	const Play play = plays[0].taken.empty() ? plays.back() : plays[0];

	setCardMode(play.card, kHighlighted);
	for (uint index : play.taken) {
		setCardMode(_piles[kTable][index], kHighlighted);
	}

	waitForTime(_thinkTime);
	makePlay(play);
}

void ScopaPuzzle::switchPlayer() {
	_currentPlayer = getOtherPlayer(_currentPlayer);
}

// A player with cards has a pile of them lying next to the table
void ScopaPuzzle::showPilePictures(bool show) {
	for (uint i = 0; i < kNumPlayers; ++i) {
		Fader &fader = _players[i].pileFader;

		if (!show) {
			fader.hide(false);
		} else if (countCards(getTakenPile(i)) > 0 || (countCards(getTrickPile(i)) > 0 && !fader.isVisible())) {
			fader.show(false);
		}
	}

	_hasChanged = true;
}

// One of the two buttons is there while a card of the hand is picked,
// depending on whether cards of the table are, too
void ScopaPuzzle::updateButtons() {
	Common::Array<uint> handCards, tableCards;
	findSelectedCards(getHandPile(_currentPlayer), handCards);
	findSelectedCards(kTable, tableCards);

	const bool showsDiscard = !handCards.empty() && tableCards.empty();
	if (showsDiscard != _showsDiscard) {
		_showsDiscard = showsDiscard;
		if (showsDiscard) {
			_discardFader.show(false);
		} else {
			_discardFader.hide(false);
		}
	}

	const bool showsTake = !handCards.empty() && !tableCards.empty();
	if (showsTake != _showsTake) {
		_showsTake = showsTake;
		if (showsTake) {
			_takeFader.show(false);
		} else {
			_takeFader.hide(false);
		}
	}

	_hasChanged = true;
}

void ScopaPuzzle::moveTricksToTaken() {
	for (uint i = 0; i < kNumPlayers; ++i) {
		for (uint j = 0; j < _piles[getTrickPile(i)].size(); ++j) {
			queueMove(getTrickPile(i), 0, 0, getTakenPile(i), kHidden);
		}
	}
}

// What is left on the table goes to whoever took cards last
void ScopaPuzzle::takeLastCards() {
	if (_lastTaker == -1) {
		return;
	}

	const Common::Array<int> &table = _piles[kTable];
	bool hasCardsLeft = false;

	for (uint i = 0; i < table.size(); ++i) {
		if (table[i] != -1) {
			queueMove(kTable, i, 0, getTakenPile(_lastTaker), kHidden);
			hasCardsLeft = true;
		}
	}

	if (hasCardsLeft) {
		_moves.back().delay = kWaitForCard;
		waitForSound(_players[_lastTaker].sounds[_hasSaidLastCardsLong ? kSoundLastCards : kSoundLastCardsLong]);
		_hasSaidLastCardsLong = true;
	}
}

// Clearing the table is a scopa, but not with the last card of the round
void ScopaPuzzle::afterPlay() {
	const bool isDeckEmpty = countCards(kDeck) == 0;
	const bool isLastHandEmpty = countCards(getHandPile(getOtherPlayer(_starter))) == 0;

	if (!_hasCheckedScopa && countCards(kTable) == 0) {
		if (!isDeckEmpty || !isLastHandEmpty) {
			++_players[_currentPlayer].numScopas;
			waitForSound(_players[_currentPlayer].sounds[kSoundScopa]);
		} else if (!_hasSaidNoLastScopa) {
			waitForSound(_noLastScopaSound);
			_hasSaidNoLastScopa = true;
		}

		_hasCheckedScopa = true;
		_isBusy = true;
		return;
	}

	_hasCheckedScopa = false;

	if (!isLastHandEmpty) {
		showPilePictures(true);
		switchPlayer();
		startTurn();
		return;
	}

	moveTricksToTaken();

	if (!isDeckEmpty) {
		_phase = kDeal;
	} else {
		takeLastCards();
		_phase = kTally;
	}

	_isBusy = true;
}

void ScopaPuzzle::tallyPlayer(Player &player, int takenPile) {
	Common::Array<int> bestPrimes;
	bestPrimes.resize(kNumSuits, 0);

	for (int card : _piles[takenPile]) {
		const int suit = getCardSuit(card);
		++player.numCards;

		if (suit == kCoins) {
			++player.numCoins;
			if (card == kSevenOfCoins) {
				player.scoresSevenOfCoins = true;
			}
		}

		if (getPrimeValue(card) > bestPrimes[suit]) {
			bestPrimes[suit] = getPrimeValue(card);
			player.bestPrimeCards[suit] = card;
		}
	}

	for (uint suit = 0; suit < kNumSuits; ++suit) {
		player.primes += bestPrimes[suit];
	}
}

void ScopaPuzzle::tallyRound() {
	Player &human = _players[kHuman];
	Player &opponent = _players[kOpponent];

	tallyPlayer(human, kHumanTaken);
	tallyPlayer(opponent, kOpponentTaken);

	if (human.numCards > opponent.numCards) {
		human.scoresMostCards = human.hasScoreLine = true;
	} else if (opponent.numCards > human.numCards) {
		opponent.scoresMostCards = opponent.hasScoreLine = true;
	}

	if (human.numCoins > opponent.numCoins) {
		human.scoresMostCoins = human.hasScoreLine = true;
	} else if (opponent.numCoins > human.numCoins) {
		opponent.scoresMostCoins = opponent.hasScoreLine = true;
	}

	if (human.scoresSevenOfCoins) {
		human.hasScoreLine = true;
	} else {
		opponent.hasScoreLine = true;
	}

	if (human.primes > opponent.primes) {
		human.scoresPrimes = true;
	} else if (opponent.primes > human.primes) {
		opponent.scoresPrimes = true;
	}
}

// For the scoring a player's cards get laid out in the place of the game
void ScopaPuzzle::layOutTakenCards(uint playerID) {
	Common::Array<int> &taken = _piles[getTakenPile(playerID)];
	Common::sort(taken.begin(), taken.end(), ScopaCardOrder());

	for (uint i = 0; i < taken.size(); ++i) {
		queueMove(getTakenPile(playerID), 0, _layOutTime, kLaidOut, kFaceUp);
	}

	startDealSound();
}

void ScopaPuzzle::highlightLaidOutCards(Highlight highlight) {
	for (int card : _piles[kLaidOut]) {
		if (card == -1) {
			continue;
		}

		bool isHighlighted = true;
		if (highlight == kCoinCards) {
			isHighlighted = getCardSuit(card) == kCoins;
		} else if (highlight == kSevenOfCoinsCard) {
			isHighlighted = card == kSevenOfCoins;
		}

		setCardMode(card, isHighlighted ? kHighlighted : kFaceUp);
	}
}

// The best card of every suit is put aside, out of sight for now, and the rest goes away
void ScopaPuzzle::movePrimes(uint playerID) {
	Common::Array<int> &laidOut = _piles[kLaidOut];

	for (uint i = 0; i < kNumSuits; ++i) {
		const int card = _players[playerID].bestPrimeCards[kPrimeSuitOrder[i]];

		for (uint j = 0; j < laidOut.size(); ++j) {
			if (card != -1 && laidOut[j] == card) {
				Move move;
				move.srcPile = kLaidOut;
				move.srcIndex = j;
				move.dstPile = kPrimes;
				move.mode = kHidden;
				doMove(move);
				break;
			}
		}
	}

	for (uint i = 0; i < laidOut.size(); ++i) {
		if (laidOut[i] != -1) {
			Card &card = _cards[laidOut[i]];
			card.current.mode = kHidden;
			card.current.fader.hide(true);
			card.previous.fader.hide(true);
			laidOut[i] = -1;
		}
	}

	_hasChanged = true;
}

// A line for every point the cards of the player are worth, with the cards the
// point is for picked out. Returns true once both players are through.
bool ScopaPuzzle::scorePlayerStep() {
	stopDealSound();

	Player &player = _players[_currentPlayer];
	const RandomSoundBlock *sound = nullptr;
	int32 points = 1;

	if (player.scoresMostCards) {
		highlightLaidOutCards(kAllCards);
		sound = &player.sounds[kSoundMostCards];
		player.scoresMostCards = false;
	} else if (player.scoresMostCoins) {
		highlightLaidOutCards(kCoinCards);
		sound = &player.sounds[kSoundMostCoins];
		player.scoresMostCoins = false;
	} else if (player.scoresSevenOfCoins) {
		highlightLaidOutCards(kSevenOfCoinsCard);
		sound = &player.sounds[kSoundSevenOfCoins];
		player.scoresSevenOfCoins = false;
	} else if (!player.hasScoreLine) {
		sound = &player.sounds[kSoundNoPoints];
		points = 0;
		player.hasScoreLine = true;
	}

	if (sound) {
		waitForSound(*sound, _currentPlayer, points);
		return false;
	}

	movePrimes(_currentPlayer);
	if (_currentPlayer != kHuman) {
		return true;
	}

	switchPlayer();
	layOutTakenCards(_currentPlayer);
	return false;
}

// The cards put aside are shown with what they count for, then the sums, and
// last come the scopas. Returns true once there is nothing left to score.
bool ScopaPuzzle::scorePrimesStep() {
	if (!_showsPrimes) {
		_showsPrimes = true;

		for (int card : _piles[kPrimes]) {
			setCardMode(card, kFaceUp);
		}

		waitForSound(_primesSound);
		_hasChanged = true;
		return false;
	}

	if (!_showsPrimeTotals) {
		_showsPrimeTotals = true;

		if (_players[kHuman].scoresPrimes) {
			waitForSound(_players[kHuman].sounds[kSoundBestPrimes], kHuman, 1);
		} else if (_players[kOpponent].scoresPrimes) {
			waitForSound(_players[kOpponent].sounds[kSoundBestPrimes], kOpponent, 1);
		} else {
			waitForSound(_primesTieSound);
		}

		_hasChanged = true;
		return false;
	}

	if (_players[kHuman].numScopas < 1 && _players[kOpponent].numScopas < 1) {
		return true;
	}

	waitForSound(_scopasSound, kHuman, _players[kHuman].numScopas);
	waitForTime(0, kOpponent, _players[kOpponent].numScopas);
	_players[kHuman].numScopas = 0;
	_players[kOpponent].numScopas = 0;
	return false;
}

// The match is won with enough points, and more of them than the other player has
void ScopaPuzzle::finishRound() {
	const int32 humanPoints = _players[kHuman].points;
	const int32 opponentPoints = _players[kOpponent].points;

	int16 flag = _roundOverFlag;
	if (humanPoints >= _pointsToWin && humanPoints > opponentPoints) {
		flag = _players[kHuman].winFlag;
	} else if (opponentPoints >= _pointsToWin && opponentPoints > humanPoints) {
		flag = _players[kOpponent].winFlag;
	}

	NancySceneState.setEventFlag(flag, g_nancy->_true);
	NancySceneState.setEventFlag(_players[getOtherPlayer(_starter)].nextStarterFlag, g_nancy->_true);

	if (!_saveName.empty()) {
		auto *data = (SavedStateData *)NancySceneState.getPuzzleData(SavedStateData::getTag());
		Common::Array<int32> &values = data->states[_saveName];
		values.resize(kNumSavedValues);
		values[kSavedHumanPoints] = humanPoints;
		values[kSavedOpponentPoints] = opponentPoints;
		values[kSavedHumanStarted] = _starter == kHuman;
		values[kSavedSaidLastCardsLong] = _hasSaidLastCardsLong;
		values[kSavedSaidNoLastScopa] = _hasSaidNoLastScopa;
	}
}

// What comes next once the cards have stopped and the lines are said
void ScopaPuzzle::runPhase() {
	switch (_phase) {
	case kDeal:
		dealCards();
		startDealSound();
		_hasCheckedScopa = false;
		_phase = kDealing;
		break;
	case kDealing:
		if (!_moves.empty()) {
			break;
		}

		stopDealSound();
		_currentPlayer = _starter;
		_phase = kPlaying;

		if (_needsRedeal) {
			NancySceneState.setEventFlag(_redealFlag, g_nancy->_true);
			_state = kActionTrigger;
		} else {
			startTurn();
		}

		break;
	case kPlaying:
		afterPlay();
		break;
	case kTally:
		tallyRound();
		_currentPlayer = kHuman;
		layOutTakenCards(kHuman);
		showPilePictures(false);
		_phase = kScorePlayers;
		_isBusy = true;
		break;
	case kScorePlayers:
		if (scorePlayerStep()) {
			_phase = kScorePrimes;
		}

		_isBusy = true;
		break;
	case kScorePrimes:
		if (scorePrimesStep()) {
			finishRound();
			_state = kActionTrigger;
		} else {
			_isBusy = true;
		}

		break;
	}
}

void ScopaPuzzle::blendPixel(int x, int y, byte a, byte r, byte g, byte b) {
	if (a == 0 || x < 0 || y < 0 || x >= _drawSurface.w || y >= _drawSurface.h) {
		return;
	}

	uint32 *dst = (uint32 *)_drawSurface.getBasePtr(x, y);
	byte dstA, dstR, dstG, dstB;
	_drawSurface.format.colorToARGB(*dst, dstA, dstR, dstG, dstB);

	const uint under = dstA * (255 - a) / 255;
	const uint outA = a + under;
	*dst = _drawSurface.format.ARGBToColor(outA,
		(r * a + dstR * under) / outA,
		(g * a + dstG * under) / outA,
		(b * a + dstB * under) / outA);
}

// Draws a part of an image, with its colors and its opacity scaled
void ScopaPuzzle::blitFaded(const Graphics::ManagedSurface &image, const Common::Rect &src, const Common::Point &dest, uint32 argb, float alpha) {
	if (image.empty() || alpha <= 0.0f) {
		return;
	}

	byte tintA, tintR, tintG, tintB;
	kARGBFormat.colorToARGB(argb, tintA, tintR, tintG, tintB);
	const uint scaledA = (uint)(tintA * MIN(alpha, 1.0f));

	Common::Rect clipped = src;
	clipped.clip(Common::Rect(image.w, image.h));

	for (int y = clipped.top; y < clipped.bottom; ++y) {
		for (int x = clipped.left; x < clipped.right; ++x) {
			byte a, r, g, b;
			image.format.colorToARGB(image.getPixel(x, y), a, r, g, b);
			if (a == 0) {
				continue;
			}

			blendPixel(dest.x + x - src.left, dest.y + y - src.top, a * scaledA / 255,
				r * tintR / 255, g * tintG / 255, b * tintB / 255);
		}
	}
}

void ScopaPuzzle::drawCardState(int cardID, CardState &state) {
	const float alpha = state.fader.update();
	const Common::Point dest(state.rect.left, state.rect.top);

	switch (state.mode) {
	case kFaceUp:
		blitFaded(_cardsImage, _cardSrcs[cardID], dest, kNoTint, alpha);
		break;
	case kFaceDown:
		blitFaded(_cardsImage, _cardBackSrc, dest, kNoTint, alpha);
		break;
	case kHighlighted:
		blitFaded(_cardsImage, _cardSrcs[cardID], dest, _highlightColor, alpha);
		break;
	case kHidden:
		break;
	}
}

// The text is drawn in white on black first, which tells how much of every pixel it covers
void ScopaPuzzle::drawText(const Common::String &text, const Common::Rect &rect, uint fontID, uint colorID, Graphics::TextAlign align) {
	const Font *font = g_nancy->_graphics->getFont(fontID);
	const Graphics::Font *systemFont = font ? font->getSystemFont() : nullptr;
	if (!systemFont || rect.isEmpty()) {
		return;
	}

	Graphics::ManagedSurface mask(rect.width(), rect.height(), _drawSurface.format);
	mask.clear(mask.format.ARGBToColor(0xFF, 0, 0, 0));
	systemFont->drawString(&mask, text, 0, (rect.height() - systemFont->getFontHeight()) / 2, rect.width(),
		mask.format.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF), align);

	byte a, r, g, b;
	_drawSurface.format.colorToARGB(g_nancy->_graphics->getFontColor(colorID, _drawSurface.format), a, r, g, b);

	for (int y = 0; y < mask.h; ++y) {
		for (int x = 0; x < mask.w; ++x) {
			byte coverA, coverR, coverG, coverB;
			mask.format.colorToARGB(mask.getPixel(x, y), coverA, coverR, coverG, coverB);
			blendPixel(rect.left + x, rect.top + y, coverG, r, g, b);
		}
	}
}

void ScopaPuzzle::redraw() {
	_drawSurface.clear(0);
	_isFading = false;

	// The cards a player took make a pile next to them
	for (Player &player : _players) {
		blitFaded(_image, player.pileSrc, Common::Point(player.pileDest.left, player.pileDest.top), kNoTint, player.pileFader.update());
		_isFading = _isFading || player.pileFader.isFading;
	}

	for (const Player &player : _players) {
		drawText(Common::String::format("%d", player.points), player.pointsRect, player.fontID, player.colorID, Graphics::kTextAlignRight);
	}

	for (uint pile = 0; pile < kNumPiles; ++pile) {
		for (int cardID : _piles[pile]) {
			if (cardID == -1) {
				continue;
			}

			Card &card = _cards[cardID];
			drawCardState(cardID, card.previous);
			drawCardState(cardID, card.current);
			_isFading = _isFading || card.current.fader.isFading || card.previous.fader.isFading;
		}
	}

	blitFaded(_image, _takeSrc, Common::Point(_takeDest.left, _takeDest.top), kNoTint, _takeFader.update());
	blitFaded(_image, _discardSrc, Common::Point(_discardDest.left, _discardDest.top), kNoTint, _discardFader.update());
	_isFading = _isFading || _takeFader.isFading || _discardFader.isFading;

	if (_showsPrimes) {
		// Next to every card that got put aside, what it counts for
		const Common::Array<Common::Rect> &slots = _slots[kPrimeValues];
		const Common::Array<int> &primes = _piles[kPrimes];

		for (uint i = 0; i < primes.size() && i < slots.size(); ++i) {
			drawText(Common::String::format("%d", getPrimeValue(primes[i])), slots[i], _primesFontID, _primesColorID, Graphics::kTextAlignCenter);
		}

		// The sums go under a line, in the two places that come after
		if (_showsPrimeTotals && primes.size() + 1 < slots.size()) {
			const Common::Rect &humanRect = slots[primes.size()];
			const Common::Rect &opponentRect = slots[primes.size() + 1];

			_drawSurface.fillRect(Common::Rect(opponentRect.left, humanRect.top, humanRect.right, humanRect.top + 1),
				argbToColor(_drawSurface.format, kPrimesLineColor));

			drawText(Common::String::format("%d", _players[kHuman].primes), humanRect, _primesFontID, _primesColorID, Graphics::kTextAlignCenter);
			drawText(Common::String::format("%d", _players[kOpponent].primes), opponentRect, _primesFontID, _primesColorID, Graphics::kTextAlignCenter);
		}
	}

	_hasChanged = false;
	_needsRedraw = true;
}

void ScopaPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		_isBusy = runMoves() || _isBusy;

		if (_isBusy && updateWaits()) {
			_isBusy = false;
			runPhase();
		}

		if (_hasChanged || _isFading) {
			redraw();
		}

		break;
	case kActionTrigger:
		_isDone = true;
		break;
	}
}

void ScopaPuzzle::findSelectedCards(int pile, Common::Array<uint> &indices) const {
	const Common::Array<int> &cards = _piles[pile];
	for (uint i = 0; i < cards.size(); ++i) {
		if (cards[i] != -1 && _cards[cards[i]].current.mode == kHighlighted) {
			indices.push_back(i);
		}
	}
}

int ScopaPuzzle::getHoveredCard(int pile, const Common::Point &pos) const {
	const Common::Array<int> &cards = _piles[pile];
	for (uint i = 0; i < cards.size(); ++i) {
		if (cards[i] != -1 && _cards[cards[i]].current.rect.contains(pos)) {
			return i;
		}
	}

	return -1;
}

// A click picks a card or lets go of it. Only one card of the hand can be picked.
void ScopaPuzzle::clickCard(int pile, uint index) {
	const int cardID = _piles[pile][index];
	const CardMode mode = _cards[cardID].current.mode;

	if (pile != kTable && mode != kHighlighted) {
		for (int other : _piles[pile]) {
			if (other != -1) {
				setCardMode(other, kFaceUp);
			}
		}
	}

	if (mode == kFaceUp) {
		setCardMode(cardID, kHighlighted);
	} else if (mode == kHighlighted) {
		setCardMode(cardID, kFaceUp);
	}

	updateButtons();
}

void ScopaPuzzle::tryPlay(bool sayWhyNotToDiscard) {
	const int handPile = getHandPile(_currentPlayer);
	Common::Array<uint> handCards;
	findSelectedCards(handPile, handCards);
	if (handCards.empty()) {
		return;
	}

	Play play;
	play.handIndex = handCards[0];
	play.card = _piles[handPile][play.handIndex];
	findSelectedCards(kTable, play.taken);

	switch (checkPlay(play)) {
	case kPlayAllowed:
		// The line about the play that wasn't allowed is cut short
		g_nancy->_sound->stopSound(_errorSound);
		_errorSound = SoundDescription();
		makePlay(play);

		_showsTake = _showsDiscard = false;
		_takeFader.hide(false);
		_discardFader.hide(false);
		_hasChanged = true;
		break;
	case kWrongSum:
		_errorSound = playSoundBlock(_wrongSumSound);
		break;
	case kHasToTake:
		if (sayWhyNotToDiscard) {
			_errorSound = playSoundBlock(_hasToTakeSound);
		}

		break;
	case kHasToTakeOneCard:
		_errorSound = playSoundBlock(_hasToTakeOneCardSound);
		break;
	}
}

void ScopaPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	const Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (_phase != kPlaying) {
		setDataCursor(kWaitCursor, false);
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	// Only the player's own cards and the ones on the table can be picked
	int hoveredPile = -1;
	int hoveredIndex = -1;
	if (_currentPlayer == kHuman) {
		hoveredPile = kHumanHand;
		hoveredIndex = getHoveredCard(kHumanHand, pos);
		if (hoveredIndex == -1) {
			hoveredPile = kTable;
			hoveredIndex = getHoveredCard(kTable, pos);
		}
	}

	const bool isOverDiscard = _showsDiscard && _discardDest.contains(pos);
	const bool isOverTake = _showsTake && _takeDest.contains(pos);

	setDataCursor(_players[_currentPlayer].cursor, (hoveredIndex != -1 && !_hasCheckedScopa) || isOverDiscard || isOverTake);

	if (_isBusy || !(input.input & NancyInput::kLeftMouseButtonUp)) {
		return;
	}

	if (hoveredIndex != -1) {
		clickCard(hoveredPile, hoveredIndex);
		input.eatMouseInput();
		return;
	}

	// A card can also be left on the table by clicking a place of the table
	bool isOverTable = false;
	if (_showsDiscard) {
		for (const Common::Rect &slot : _slots[kTable]) {
			isOverTable = isOverTable || slot.contains(pos);
		}
	}

	if (isOverDiscard || isOverTake || isOverTable) {
		tryPlay(!isOverTable);
		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
