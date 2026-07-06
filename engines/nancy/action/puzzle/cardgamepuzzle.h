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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_CARDGAMEPUZZLE_H
#define NANCY_ACTION_CARDGAMEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// A "Go fish!" card game, new in Nancy 11 (Curse of Blackmoor Manor, AR 246).
// There are two variants, one with the cards placed in a grid vs a human NPC, and
// one with the cards placed in columns vs an automaton.
class CardGamePuzzle : public RenderActionRecord {
public:
	CardGamePuzzle() : RenderActionRecord(7) {}
	virtual ~CardGamePuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	// Up to 13 columns / 4 rows are addressable in the data (per-row stride is 13 card rects).
	static const int kMaxCols = 13;
	static const int kMaxRows = 4;

	// Per-side board: a grid of dealt cards, per-column counts (capped at 3), per-column "complete"
	// flags (a full column of 3 scores a set) and the running score.
	struct PlayerBoard {
		int grid[kMaxRows][kMaxCols];
		int colCount[kMaxCols];
		int colComplete[kMaxCols];
		int score;
	};

	Common::String getRecordTypeName() const override { return "CardGamePuzzle"; }

	int dealOne(int player);  // draw a card from the deck to a side; returns its column, or -1 if the deck is empty
	void drawBoard();
	// True when the scene has a bottom button row (the player clicks those); false when it doesn't,
	// in which case the player clicks their own cards in the tableau directly.
	bool usesColumnButtons() const { return _columnButtons[0].top != _columnButtons[0].bottom; }
	// The column the player is pointing at (owning 1-2 cards in it), or -1 if none is under the mouse.
	int columnUnderMouse(const Common::Point &mousePos) const;
	bool hasPlayableColumn(int side) const; // whether a side holds any incomplete rank it can ask for
	int aiPickColumn();       // the AI's chosen rank to ask for, or -1 if it holds no incomplete rank

	// Go Fish turn flow, driven as a small state machine so asks and answers are voiced and sequenced.
	bool takeCards(int side, int col); // transfer every opponent card of a rank to the side; true if any moved
	void beginAsk(int side, int col);  // start an ask: play the "do you have any X?" line, enter kAskSound
	void resolveAsk();                 // ask voice done: take or go fish, play the answer, start its wait
	void advanceTurn();                // answer done: ask again, or pass the turn to the other side
	void startPlayerTurn();            // enter kWaitInput (or auto-go-fish + pass if no rank to ask)
	void startAiAsk();                 // the AI picks a rank and asks, or goes fish and passes
	void endGame();
	// Compare side 1's grid against a pre-move snapshot and start sliding the changed cards.
	void startMoveAnimation(const bool beforeGrid[kMaxRows][kMaxCols]);
	void playVoice(const Common::String &name); // play a voiced line / SFX on the card-game channel
	void showSubtitle(const Common::String &soundName); // push the line's AUTOTEXT caption to the textbox

	Common::Path _imageName;

	// Header flags / dimensions
	byte _unknown21 = 0;
	byte _switchTurnRule = 0; // data+0x22: how the turn passes after a play
	byte _startPlayer = 0;    // data+0x23: which side plays first (also the human side)
	byte _dealMode = 0;       // data+0x24: deal/scoring variant (0, 2 or 0xff)
	uint16 _numCols = 0;      // data+0x25
	uint16 _numRows = 0;      // data+0x27
	uint16 _dealRounds = 0;   // data+0x29

	// Source/destination rects (all addressed [row * kMaxCols + col] or [side * kMaxCols + col])
	Common::Rect _turnHighlightSrc[2];  // data+0x2b
	Common::Rect _turnHighlightDest[2]; // data+0x4b
	Common::Array<Common::Rect> _faceUpSrc;   // data+0x6b, 4 rows
	Common::Rect _suitScoreSrc;               // data+0x47b
	Common::Array<Common::Rect> _scoreDest;   // data+0x48b, 2 sides
	Common::Rect _cardDisplayDest[2];         // data+0x633
	Common::Array<Common::Rect> _columnButtons; // data+0x653, one per column
	Common::Array<Common::Rect> _faceDownSrc; // data+0x723, 3 rows
	Common::Rect _exitHotspot;                // data+0x1336

	// Animation timing (data+0x62b)
	uint16 _moveAnimSteps = 0;
	uint16 _moveAnimDelta = 0;
	uint32 _moveAnimDelay = 0;

	// Outcome scenes (data+0x1304 / 0x1320). The win block has two scene ids that share one set of
	// transition params (frame/scroll/sound), held in _winScene; the id is chosen by the result.
	uint16 _winSceneStartPlayer = 0; // data+0x1304
	uint16 _winSceneStartEnemy = 0;  // data+0x1306
	SceneChangeDescription _winScene;
	int16 _winFlagPlayer = -1;       // data+0x131c
	int16 _winFlagEnemy = -1;        // data+0x131e
	uint16 _exitScene = 0;           // data+0x1320
	SceneChangeDescription _exitSceneChange;
	bool _gaveUp = false;            // left via the exit hotspot rather than playing out

	// Voiced lines / SFX (all on the card-game channel). Read selectively from the 0xba3..0x1304 block.
	Common::String _moveVoiceName;         // data+0xbc4 (card-move SFX)
	Common::String _dealVoiceName;         // data+0xbe5 (card-deal SFX)
	Common::String _matchVoice[2][kMaxCols]; // the "do you have any X?" ask, by side; 0xc2b (AI) / 0xf68 (player)
	Common::String _madeMoveVoice[2];      // the "here you go" answer, by side; 0xe7d (player) / 0x11ba (AI)
	Common::String _noMoveVoice[2];        // the "go fish" answer, by side; 0xdd8 (player) / 0x1115 (AI)
	Common::String _enemyScoredVoiceName;  // data+0xee0 (a set completed for the AI)
	Common::String _playerScoredVoiceName; // data+0x121d (a set completed for the player)
	Common::String _endVoiceName[2];       // data+0xf22 (AI wins) / 0x125f (player wins)
	SoundDescription _voiceSound;

	// At game over, the winner's line plays before the result scene transition.
	bool _awaitingEnd = false;
	uint32 _endWaitUntil = 0;

	// Turn state machine. The mover asks, the ask voice plays, the cards move and the answer voice
	// plays, then the turn either repeats (a take) or passes. The AI's asks run through the same
	// phases so its turn is visible (Nancy's cards being taken) and voiced.
	enum Phase {
		kWaitInput,   // the player's turn: waiting for a card click
		kAskSound,    // a "do you have any X?" voice is playing
		kAnswerSound, // an answer voice is playing and/or the taken cards are sliding
		kAiDelay      // a short pause before the AI's next ask
	};
	Phase _phase = kWaitInput;
	int _mover = 1;        // which side is currently asking (0 = AI/Jane, 1 = player/Nancy)
	int _askedCol = -1;    // the rank being asked for
	bool _goAgain = false; // whether the mover asks again once the answer finishes
	uint32 _aiDelayUntil = 0;

	// Runtime board state
	Graphics::ManagedSurface _image;
	PlayerBoard _board[2];
	byte _availMap[kMaxRows][kMaxCols]; // shared deck: 1 = card still on the table
	int _deckRemaining = 0;
	int _currentTurn = 0;               // side owning the turn highlight (mirrors _mover)
	int _lastAiColumn = -1;             // the AI avoids immediately repeating its previous column
	bool _gameOver = false;

	// Slide animation for cards that changed hands on the last move (visual only; the board state
	// is already up to date). Appeared cards slide into place, departed cards slide away.
	static const int kSlidePerStep = 12;
	bool _appearing[kMaxRows][kMaxCols] = {};
	bool _leaving[kMaxRows][kMaxCols] = {};
	int _animStep = 0;
	uint32 _animNextStep = 0;
	bool _animating = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CARDGAMEPUZZLE_H
