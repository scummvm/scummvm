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

#ifndef NANCY_ACTION_HANGMANPUZZLE_H
#define NANCY_ACTION_HANGMANPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct HangmanData;

namespace Action {

// Classic hangman, new in Nancy14 (AR 177). A word is drawn from a bank and
// shown as a row of blanks; the player clicks the a-z letter tiles to guess.
// A correct letter fills every matching blank; a wrong one draws the next
// hang-stage piece. The word is fully revealed -> win; the hang figure is
// completed (wrong guesses == number of hang pieces) -> lose. The chosen word
// is remembered across visits so it is not immediately repeated.
class HangmanPuzzle : public RenderActionRecord {
public:
	HangmanPuzzle() : RenderActionRecord(7) {}
	virtual ~HangmanPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "HangmanPuzzle"; }

	// One clickable letter tile. Its letter is the ASCII value stored in the
	// record; idleRect/hoverRect are its states in the puzzle sprite sheet and
	// glyphRect is the letter glyph (in the letters sprite sheet) that gets
	// stamped into the word blanks / guessed row when this letter is played.
	struct LetterTile {
		char letter = 0;
		Common::Rect idleRect;
		Common::Rect hoverRect;
		Common::Rect glyphRect;
		Common::String name;
		bool used = false;
	};

	// A win/lose transition stored at the tail of the record: a scene change
	// with an event flag, each with its own random-sound block.
	struct SceneOutcome {
		int16 sceneID = 0;
		int16 frameID = 0;
		FlagDescription flag;
		RandomSoundBlock sound;
	};

	HangmanData *getPuzzleData() const;
	void pickWord();
	Common::Rect glyphForLetter(char letter) const;	// glyph rect of the tile for this letter, or empty
	int tileAtCursor(const Common::Point &mousePos) const;
	void commitGuess(uint tileIndex);
	void safeBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, const Common::Point &destPos);
	void redraw();
	void applyOutcome(const SceneOutcome &outcome);

	// -- File data --
	Common::Path _puzzleImageName;		// 0x3d
	Common::Path _lettersImageName;		// 0x41
	int16 _field45 = 0;					// 0x45

	Common::Array<Common::String> _words;			// candidate word bank
	Common::Array<Common::Rect> _hangPieceRects;	// 0x57, hang-stage pieces
	Common::Rect _boardRect;						// 0x67
	Common::Array<Common::Rect> _letterSlotRects;	// 0xae, word-blank positions
	Common::Array<Common::Rect> _guessedRowRects;	// 0xbe, guessed-letters row
	Common::Array<LetterTile> _letters;				// 0x77, the a-z tiles

	int16 _fieldCE = 0;			// 0xce
	int32 _fieldD0 = 0;			// 0xd0
	int16 _fieldD4 = 0;			// 0xd4

	RandomSoundBlock _sounds[3];	// 0x12c/0x182/0x1d8, feedback sound blocks

	Common::Path _soundName;	// 0x236
	SceneOutcome _winScene;		// 0x290
	SceneOutcome _winScene2;	// 0x2f1
	SceneOutcome _loseScene;	// 0x352

	// Give-up hotspot (count-prefixed 23-byte trailer): click to leave the puzzle.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Graphics::ManagedSurface _puzzleImage;
	Graphics::ManagedSurface _lettersImage;

	Common::String _word;				// the word being guessed (lowercase)
	Common::Array<char> _guessed;		// letters played, in order
	Common::Array<bool> _revealed;		// per word position
	int _wrongCount = 0;
	int _hoverTile = -1;
	bool _solved = false;
	bool _lost = false;
	bool _outcomeApplied = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_HANGMANPUZZLE_H
