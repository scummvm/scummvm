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

#ifndef NANCY_ACTION_MATCHPUZZLE_H
#define NANCY_ACTION_MATCHPUZZLE_H

#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/commontypes.h"

#include "graphics/managed_surface.h"
#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"

namespace Nancy {
namespace Action {

// Tile matching puzzle. Nancy 8 uses it for a maritime flag game, Nancy 14 for
// "Model Match". The player swaps two neighbouring tiles to line up 3/4/5 of the
// same type in a row or column, which scores points and extends the timer. The
// matched tiles are then replaced with fresh random ones.
class MatchPuzzle : public PuzzleRecord {
public:
	MatchPuzzle() : PuzzleRecord(7) {}
	virtual ~MatchPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MatchPuzzle"; }

	// ---------- Inner types ----------

	struct HighScore {
		Common::String name;
		int32 score = 0;
	};

	struct GridCell {
		int16 tileType = 0;       // index into _tileSrcRects
		bool  visible  = false;   // true once the cell has been shuffled in
		bool  matched  = false;   // true while cell is part of an active match
		Common::Rect destRect;    // viewport-relative draw destination
	};

	// ---------- Helpers ----------

	// Shuffle: if allCells=true all cells, otherwise only (col,row).
	void shuffleGrid(bool allCells, int col = 0, int row = 0);
	// Check cell (col,row) for a 3+ run; fills _match* fields.
	void checkForMatch(int col, int row);
	// Compute the viewport-relative dest rect for cell (col,row).
	void computeDestRect(int col, int row);

	// Rendering helpers
	void drawCell(int col, int row);
	void eraseCell(int col, int row);
	void redrawAllCells();
	void drawScorePanel();
	void drawText(const Common::String &str, const Common::Point &pos);
	void playMatchSound();
	bool isMatchSoundPlaying() const;

	// Nancy14 helpers
	void readDataNancy14(Common::SeekableReadStream &stream);
	void handleInputNancy14(NancyInput &input, const Common::Point &localMouse);
	void drawHighScoreScreen();
	void drawBoardNancy14();
	void startRound();
	void sortHighScores();
	void insertHighScore();

	// ---------- Data (read from stream) ----------

	Common::Path _overlayName;            // main flag sprite sheet (CIFTREE)
	Common::Path _flagPointBgName;        // score-panel background image

	int16 _rows         = 0;             // data+0x42
	int16 _cols         = 0;             // data+0x44
	int16 _numTileTypes = 0;             // data+0x46  (rand % (_numTileTypes-1))

	// data+0x48: source rect of the shuffle button within the sprite sheet
	Common::Rect _shuffleButtonSrcRect;
	Common::Array<Common::Rect> _tileSrcRects;   // 26 source rects in sprite sheet

	// Script execution (data+0x238..0x23A); _execScript also gates flag-name display
	bool  _execScript = false;
	int16 _scriptID   = 0;

	// Score-panel display font and label (data+0x23C..0x25E)
	//uint16 _scorePanelFontID    = 0;     // data+0x23C
	Common::String _displayLabelString;  // data+0x23E (33 bytes)

	// 26 per-flag-type names drawn in score panel on match (data+0x25F..0x5B8)
	Common::StringArray _flagSoundNames;

	// Score-panel display enable (data+0x63D)
	bool _showScoreDisplay = false;

	// Timing / scoring (from data+0x63E region)
	int16 _timeLimitSecs     = 0;   // data+0x63E (0 = no timer)
	int32 _scoreTarget       = 0;   // data+0x640
	int16 _scorePerTile      = 0;   // data+0x644 points per matched flag

	// Source rect for highlighted (matched) flag overlay (data+0x646)
	Common::Rect _matchedTileSrcRect;

	int16 _timeBonusFor3     = 0;   // data+0x656 extra seconds for 3-match
	int16 _scoreBonusFor4    = 0;   // data+0x658 extra points  for 4-match
	int16 _timeBonusFor4     = 0;   // data+0x65A extra seconds for 4-match
	int16 _scoreBonusFor5    = 0;   // data+0x65C extra points  for 5-match
	int16 _timeBonusFor5     = 0;   // data+0x65E extra seconds for 5-match
	int16 _gridOffX          = 0;   // data+0x660 grid x offset within viewport
	int16 _gridOffY          = 0;   // data+0x662 grid y offset within viewport
	int16 _rowSpacing        = 0;   // data+0x664 extra pixels between rows
	int16 _colSpacing        = 0;   // data+0x666 extra pixels between cols

	// Score-panel destination rects (data+0x668..0x6E7, 8 × 16 bytes)
	Common::Rect _labelStringRect;      // data+0x668 — where to draw _displayLabelString
	Common::Rect _shuffleButtonDestRect;// data+0x678 — on-screen position of shuffle button (hotspot)
	Common::Rect _goalValueRect;        // data+0x688 — where to draw goal value
	Common::Rect _scoreValueRect;       // data+0x698 — where to draw score value
	Common::Rect _timerValueRect;       // data+0x6A8 — where to draw timer
	Common::Rect _flagNameRect;         // data+0x6B8 — where to draw matched flag name
	Common::Rect _flagImageRect;        // data+0x6C8 — where to draw matched flag image
	// data+0x6D8 (16 bytes): high-score display positions — skipped

	// High-score display positions (data+0x6D8, 4 × int32 packed as a rect):
	//   left  = x-coord for the "final score" value
	//   top   = y-coord for the "final score" value
	//   right = x-coord for the high-score value list
	//   bottom= y-coord of the first high-score entry
	Common::Rect _hsDisplayRect;

	int16 _scoreDisplayDelay = 0;   // data+0x6E8 score display pause (seconds)

	// Sounds
	SoundDescription _slotWinSound;      // data+0x6EA — played during match anim
	SoundDescription _shuffleSound;      // data+0x71B
	SoundDescription _cardPlaceSound;    // data+0x74C
	SoundDescription _matchSuccessSound; // data+0x798 — played on win/time-up

	// ---------- Nancy14-only data ----------

	// Second sprite sheet, holding the pressed-down graphics of the two buttons
	Common::Path _buttonsImageName;
	// Full-screen backdrop of the high score list
	Common::Path _highScoreImageName;

	// Set while the puzzle waits on the high score screen instead of playing
	int16 _inProgressFlag = kEvNoEvent;
	// When set, the board starts frozen on the high score screen
	bool _startInactive = false;

	Common::Rect _doneButtonSrcRect;
	Common::Rect _doneButtonDestRect;

	uint16 _fontID = 0;
	// Picks between the two color variants baked into the font image
	uint16 _fontColor = 0;

	Common::String _timerSuffix;    // appended to the seconds left, e.g. "s"
	Common::String _winString;      // replaces the timer once the target is beaten
	Common::String _timeUpString;   // replaces the timer when time runs out

	// Opens the high score screen mid-game
	Common::Rect _highScoreButtonRect;

	// Used when the high score list is empty
	int32 _defaultScoreTarget = 0;

	Common::String _playerName;                     // name stored alongside a new high score
	Common::Array<Common::Rect> _highScoreRects;    // left/top = name pos, right/bottom = score pos

	RandomSoundBlock _matchSound;     // repeats while a match is highlighted
	RandomSoundBlock _selectSound;    // first click on a tile
	RandomSoundBlock _swapSound;      // second click, and the shuffle button
	RandomSoundBlock _winSound;
	RandomSoundBlock _timeUpSound;
	RandomSoundBlock _goButtonSound;

	SceneChangeWithFlag _doneSceneChange;   // leaving through the button next to the board

	// ---------- Runtime state ----------

	enum GameSubState {
		kPlaying       = 0, // normal gameplay; process clicks, update timer, check win/lose
		kStartEndSeq   = 1, // play win/time-up sound, set display-delay timer, go to kWaitDelay
		kMatchAnim     = 2, // wait 800 ms + sound to finish, then reshuffle matched cells
		kShuffleDelay  = 3, // wait for _shuffleTimer before applying full shuffle
		kWaitSound     = 4, // wait for win/time-up sound to finish, then go to kScoreDisplay
		kWaitDelay     = 5, // wait for display-delay timer, then go to kWaitSound
		kScoreDisplay  = 6, // show scores, insert into high-score list, then exit or reset

		// Nancy14 only
		kButtonDown    = 7, // a button is held down; apply its action once the timer runs out
		kEndDelay      = 8, // pause on the win/time-up message before the high score screen
		kHighScores    = 9  // high score screen, waiting for the GO button
	};

	GameSubState _gameSubState = kPlaying;
	bool _wonGame = false;

	// Timer tracking
	uint32 _timerDeadline = 0; // abs ms when timer expires
	uint32 _stateTimer    = 0; // abs ms for state timeouts
	uint32 _shuffleTimer  = 0; // abs ms for shuffle-button delay

	// Score
	int32 _score = 0;

	// First-click selection (before the swap) — no visual, just remembered internally
	bool _hasSelection = false;
	int  _selCol = 0, _selRow = 0;

	// Post-swap match-check queue (piece1 = first cell, piece2 = second cell)
	bool _hasPiece1 = false;
	int  _piece1Col = 0, _piece1Row = 0;
	bool _hasPiece2 = false;
	int  _piece2Col = 0, _piece2Row = 0;

	// Match results from checkForMatch
	int  _matchRowStart = 0, _matchRowEnd = 0;   // vertical run bounds (row indices)
	int  _matchColStart = 0, _matchColEnd = 0;   // horizontal run bounds (col indices)
	bool _hasVMatch = false;  // vertical   run >= 3
	bool _hasHMatch = false;  // horizontal run >= 3
	int16 _matchedFlagType = 0; // flag type of the matched run (for sound)

	// Score panel display strings (updated whenever values change)
	Common::String _goalStr;    // formatted goal target, set once at init
	Common::String _scoreStr;   // formatted current score
	Common::String _timerStr;   // formatted time remaining
	Common::String _flagNameStr;// name of the last matched flag type
	bool _showFlagName = false; // true while matched-flag info is visible (this+0x800)
	int  _prevTimerSecs = -1;   // last rendered timer value (seconds), for change detection

	// High scores (top 5, descending; stored in memory, not persisted)
	Common::Array<HighScore> _highScores;

	// Nancy14 runtime state
	bool _showHighScores = false;   // high score screen is up, waiting for the GO button
	bool _canResumeGame = false;    // the GO button restarts the round instead of resuming
	bool _shuffleButtonDown = false;
	bool _doneButtonDown = false;
	bool _leftThroughButton = false;

	// Rendering
	Common::Array<Common::Array<GridCell>> _grid; // _grid[col][row]

	Graphics::ManagedSurface _scorePanelImage;  // score-panel background
	Graphics::ManagedSurface _buttonsImage;     // Nancy14 pressed-button graphics
	Graphics::ManagedSurface _highScoreImage;   // Nancy14 high score backdrop
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MATCHPUZZLE_H
