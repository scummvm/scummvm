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

// Two-player card game versus an AI opponent, new in Nancy 11 (Curse of Blackmoor Manor, AR 246).
// Cards are dealt into a shared grid; each player stacks up to three cards per column, and a full
// column scores a "set". Implementation is staged: this currently parses the data chunk and sets up
// the board; the deal/match/AI gameplay is being filled in incrementally.
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

	bool dealOne(int player); // draw a card from the deck to a side; false if the deck is empty
	void drawBoard();
	// True when the scene has a bottom button row (the player clicks those); false when it doesn't,
	// in which case the player clicks their own cards in the tableau directly.
	bool usesColumnButtons() const { return _columnButtons[0].top != _columnButtons[0].bottom; }
	// The column the player is pointing at (owning 1-2 cards in it), or -1 if none is under the mouse.
	int columnUnderMouse(const Common::Point &mousePos) const;
	// Current side takes every opponent card in the given column; returns true if a card moved.
	bool playColumn(int col);
	int aiPickColumn();       // the AI's column choice, or -1 if it has no productive move
	void finishMove();        // draw a card, pass the turn, run the AI, detect the end of the game
	void endGame();
	// Compare side 1's grid against a pre-move snapshot and start sliding the changed cards.
	void startMoveAnimation(const bool beforeGrid[kMaxRows][kMaxCols]);
	void playVoice(const Common::String &name); // play a voiced line / SFX on the card-game channel

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
	Common::String _matchVoice[2][kMaxCols]; // data+0xc2b (AI) / 0xf68 (player), keyed by column
	Common::String _enemyScoredVoiceName;  // data+0xee0 (a column completed for the AI)
	Common::String _playerScoredVoiceName; // data+0x121d (a column completed for the player)
	Common::String _endVoiceName[2];       // data+0xf22 (AI wins) / 0x125f (player wins)
	SoundDescription _voiceSound;

	// At game over, the winner's line plays before the result scene transition.
	bool _awaitingEnd = false;
	uint32 _endWaitUntil = 0;

	// Runtime board state
	Graphics::ManagedSurface _image;
	PlayerBoard _board[2];
	byte _availMap[kMaxRows][kMaxCols]; // shared deck: 1 = card still on the table
	int _deckRemaining = 0;
	int _currentTurn = 0;               // which side is to play (the player's side draws the grid)
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
