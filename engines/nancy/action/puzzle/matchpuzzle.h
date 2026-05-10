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

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

#include "graphics/managed_surface.h"
#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"

namespace Nancy {
namespace Action {

// Maritime Flag matching puzzle in Nancy 8.
// The player spots 3/4/5 flags of the same type in a row or column and clicks
// one to score points and extend the timer.  After every match the board is
// reshuffled.  The game ends when the score target is reached.
class MatchPuzzle : public RenderActionRecord {
public:
	MatchPuzzle() : RenderActionRecord(7) {}
	virtual ~MatchPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MatchPuzzle"; }

	// ---------- Inner types ----------

	struct GridCell {
		int16 flagType = 0;       // index into _flagSrcRects / _flagSoundNames
		bool  visible  = false;   // true once the cell has been shuffled in
		bool  matched  = false;   // true while cell is part of an active match
		Common::Rect destRect;    // viewport-relative draw destination
	};

	// ---------- Helpers ----------

	// Shuffle: if allCells=true all cells, otherwise only (col,row). FUN_0046421e
	void shuffleGrid(bool allCells, int col = 0, int row = 0);
	// Check cell (col,row) for a 3+ run; fills _match* fields. FUN_00464ba6
	void checkForMatch(int col, int row);
	// Compute the viewport-relative dest rect for cell (col,row). FUN_004643ef
	void computeDestRect(int col, int row);

	// Rendering helpers
	void drawCell(int col, int row);
	void eraseCell(int col, int row);
	void redrawAllCells();
	void drawScorePanel();  // FUN_004660ff

	// ---------- Data (read from stream) ----------

	Common::Path _overlayName;            // main flag sprite sheet (CIFTREE)
	Common::Path _flagPointBgName;        // score-panel background image

	int16 _rows         = 0;             // data+0x42
	int16 _cols         = 0;             // data+0x44
	int16 _numFlagTypes = 0;             // data+0x46  (rand % (_numFlagTypes-1))

	// data+0x48: source rect of the shuffle button within the sprite sheet
	Common::Rect _shuffleButtonSrcRect;
	Common::Array<Common::Rect> _flagSrcRects;   // 26 source rects in sprite sheet

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
	int16 _scorePerFlag      = 0;   // data+0x644 points per matched flag

	// Source rect for highlighted (matched) flag overlay (data+0x646)
	Common::Rect _matchedFlagSrcRect;

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

	SceneChangeWithFlag _solveSceneChange; // data+0x77D  win  scene
	SceneChangeWithFlag _exitSceneChange;  // data+0x7C9  quit scene

	Common::Rect _exitHotspot;             // data+0x7E2  bottom-strip exit hotspot

	// ---------- Runtime state ----------

	enum GameSubState {
		kPlaying       = 0, // normal gameplay; process clicks, update timer, check win/lose
		kStartEndSeq   = 1, // play win/time-up sound, set display-delay timer, go to kWaitDelay
		kMatchAnim     = 2, // wait 800 ms + sound to finish, then reshuffle matched cells
		kShuffleDelay  = 3, // wait for _shuffleTimer before applying full shuffle
		kWaitSound     = 4, // wait for win/time-up sound to finish, then go to kScoreDisplay
		kWaitDelay     = 5, // wait for display-delay timer, then go to kWaitSound
		kScoreDisplay  = 6  // show scores, insert into high-score list, then exit or reset
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
	int32 _highScores[5] = {0, 0, 0, 0, 0};

	// Rendering
	Common::Array<Common::Array<GridCell>> _grid; // _grid[col][row]

	Graphics::ManagedSurface _image;            // loaded sprite sheet
	Graphics::ManagedSurface _scorePanelImage;  // score-panel background
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MATCHPUZZLE_H
