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

#ifndef NANCY_ACTION_WORDFINDPUZZLE_H
#define NANCY_ACTION_WORDFINDPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct WordFindPuzzleData;

namespace Action {

// Word-find puzzle, new in Nancy13 (AR 170). Despite the name it is not a free
// grid drag-select: the player works one target word at a time (progressing across
// scene visits), building a connected chain of orthogonally-adjacent letter cells;
// once the chain has the word's length and matches the answer path, the word's
// animation plays and the next word becomes active.
class WordFindPuzzle : public RenderActionRecord {
public:
	WordFindPuzzle() : RenderActionRecord(7) {}
	virtual ~WordFindPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "WordFindPuzzle"; }

	struct Word {
		Common::Path gridImageName;		// +0x55 - the letter grid for this word
		Common::Path overlayImageName;	// +0x76 - overlay drawn on top
		Common::Path animName;			// +0x97 - the "found" animation (a movie)
		Common::Array<Common::Point> answerCoords;	// +0xb8 - the correct letter path (grid cells)
		int16 eventFlagLabel = -1;		// +0x10a - set when the word is found
		Common::Array<Common::Rect> letterRects;	// +0x10c - clickable letter cells
	};

	// Grid (col,row) of a letter rect, using the same pixel maths as the original.
	Common::Point gridCoord(const Common::Rect &rect) const;
	bool isAdjacent(const Common::Point &a, const Common::Point &b) const;
	int letterAtCursor(const Common::Point &mousePos) const;
	int numSelected() const;
	bool chainMatchesAnswer() const;
	void loadWord();
	void startFoundAnimation();
	void finishCurrentWord();
	void redraw();
	WordFindPuzzleData *getPuzzleData() const;

	// -- File data --
	uint16 _cursorType = 0;			// base 0x00
	uint16 _cellGapX = 0;			// base 0x06 - added to a letter's width to get the column pitch
	uint16 _cellGapY = 0;			// base 0x08 - row pitch component
	SceneChangeDescription _solveScene;		// base 0x2a - shown once every word is found
	Common::Path _solutionImageName;		// base 0x2f
	Common::Array<Word> _words;

	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;

	Common::Array<RandomSoundBlock> _sounds;	// 4 blocks

	// -- Runtime state --
	// The active word persists across scene visits (and saves) via WordFindPuzzleData.
	int _currentWord = 0;
	Common::Array<bool> _selected;	// per letter of the current word
	bool _allFound = false;

	// The "found word" animation that plays before advancing to the next word.
	MoviePlayer _movie;
	bool _playingMovie = false;

	Graphics::ManagedSurface _gridImage;
	Graphics::ManagedSurface _overlayImage;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WORDFINDPUZZLE_H
