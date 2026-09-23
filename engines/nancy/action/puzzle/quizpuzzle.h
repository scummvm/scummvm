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

#ifndef NANCY_ACTION_QUIZPUZZLE_H
#define NANCY_ACTION_QUIZPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/util.h"

namespace Nancy {
namespace Action {

// Text-entry quiz with multiple text boxes. Three data formats: Nancy 8,
// Nancy 9 - Nancy 14, and Nancy 15, which rewrote the record around
// count-prefixed answer lists and RandomSoundBlocks.
class QuizPuzzle : public RenderActionRecord {
public:
	QuizPuzzle() : RenderActionRecord(7) {}
	virtual ~QuizPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;
	void onPause(bool paused) override;

protected:
	Common::String getRecordTypeName() const override { return "QuizPuzzle"; }

private:
	// Sanity cap. Nancy 8 and Nancy 9 store a fixed five boxes; Nancy 15 keeps
	// 30 slots of 27 bytes per save slot, so 30 is the engine's own maximum.
	static const uint kMaxBoxes = 30;
	// Nancy 15 rejects a keystroke once the typed text no longer fits a save
	// slot's 27-byte text buffer, with room left for the cursor character.
	static const uint kMaxTypedLengthNancy15 = 24;
	static const uint kMaxTypedLength = 16;
	// From Nancy 9 to Nancy 14 a character is only accepted while the text
	// still clears this margin at the right edge of its box
	static const int kBoxTextMargin = 20;

	// One answer box: its screen rect, the answers accepted for it (matched
	// case-insensitively), the flags it drives, and the text typed into it.
	struct Box {
		Common::Rect rect;
		Common::Array<Common::String> answers;
		int16 correctFlag = -1;		// set when this box is answered correctly
		int16 nonEmptyFlag = -1;	// Nancy 15: mirrors "this box has text"

		RandomSoundBlock correctSound;	// Nancy 9+
		RandomSoundBlock wrongSound;	// Nancy 9+
		Common::String correctText;		// Nancy 9 - Nancy 14 per-box caption
		Common::String wrongText;

		uint16 maxAnswerLength = 0;		// longest answer, used in auto-check mode

		// -- Runtime state --
		Common::String typedText;	// may end with the cursor character
		bool correct = false;
	};

	// Format-specific read implementations
	void readDataNancy8(Common::SeekableReadStream &stream);
	void readDataNancy9(Common::SeekableReadStream &stream);
	void readDataNancy15(Common::SeekableReadStream &stream);

	// Format-specific state machines
	void executeNancy8();
	void executeNancy9();

	// Helpers
	void drawText();
	void advanceToNextBox();
	bool checkAllSolved() const;
	// Scores the current box. A silent check only marks the box solved, as the
	// original does after every keystroke; otherwise the box's flag is set too.
	bool checkAnswerForCurrentBox(bool silent = false);
	char getCursorChar() const;
	uint getMaxTypedLength() const;
	bool acceptsChar(char c) const;
	bool fitsInBox(const Box &box, const Common::String &text) const;

	// Picks one name out of `block` at random and starts it, tracking it in
	// _activeBoxSound so the state machine can wait for it. Returns false when
	// the block is empty or names no sound.
	bool playSoundBlock(const RandomSoundBlock &block);
	void showBoxSubtitle(const Common::String &perBoxText, const Common::String &perRecordText);

	uint32 getSaveKey();
	void loadSavedAnswers();
	void saveAnswer(uint boxIndex);

	Common::Array<Box> _boxes;

	// ---- Data (all formats) ----
	uint16 _fontID = 0;
	uint16 _cursorBlinkInterval = 500;

	SoundDescription _doneSound;		// Nancy 8 - Nancy 14 solve sound
	Common::String _doneText;
	RandomSoundBlock _doneSoundBlock;	// Nancy 15 solve sound

	SceneChangeWithFlag _solveScene;	// entered when all boxes are solved
	SceneChangeWithFlag _cancelScene;	// entered when the puzzle is given up on
	Common::Rect _exitHotspot;			// Nancy 9+: viewport-relative give-up hotspot
	uint16 _exitCursorType = 0;			// Nancy 15

	// ---- Data (Nancy 8) ----
	SoundDescription _correctSound;		// one sound shared by every box
	Common::String _correctText;
	SoundDescription _wrongSound;
	Common::String _wrongText;

	// ---- Data (Nancy 9+) ----
	char _cursorChar = '-';			// cursor character (configurable from Nancy 9)
	Common::String _allowedChars;	// typeable characters (empty = all allowed)
	bool _autoCheck = false;		// check the answer after each character typed
	bool _skipEmptyOnEnter = false;	// if true, Enter on an empty box does nothing

	// ---- Data (Nancy 15) ----
	int16 _restoreFlag = -1;			// when set, previous answers are restored
	bool _keepBoxOnWrongAnswer = false;	// don't move on after a wrong answer
	bool _cycleSolvedBoxes = false;		// cycling also stops on solved boxes
	uint16 _displaySourceScene = kNoScene;	// see _isDisplayOnly
	bool _isDisplayOnly = false;		// no cursor character: shows another quiz's answers
	Common::String _recordCorrectText;	// captions shared by every box
	Common::String _recordWrongText;
	RandomSoundBlock _invalidKeySound;	// played when a character is not allowed
	Common::Array<ExitHotspot> _hotspots;	// flag-setting regions around the boxes

	// ---- Runtime state ----
	uint _currentBox = 0;		// which box currently receives keyboard input
	bool _solved = false;		// all boxes answered correctly
	bool _cancelled = false;	// user gave up

	enum SolveState {
		kTyping          = 0, // waiting for key input; cursor blinks
		kCheckAnswer     = 1, // Enter pressed (or auto-check triggered); evaluate typed text
		kStartCorrect    = 2, // answer correct: play correct sound, advance box
		kWaitCorrect     = 3, // waiting for correct sound to finish
		kStartWrong      = 4, // answer wrong: clear text, play wrong sound
		kWaitWrong       = 5, // waiting for wrong sound to finish
		kStartDone       = 6, // all boxes solved: play done sound
		kWaitDone        = 7, // waiting for done sound to finish
		kFinish          = 8  // trigger scene transition
	};

	// Internal state machine mirroring the original engine's 0..8 states
	SolveState _internalState = kTyping;

	// Key input from handleInput, consumed in execute()
	bool _hasNewKey = false;
	bool _pendingReturn = false;
	bool _pendingBackspace = false;
	char _pendingChar = 0;

	Time _nextBlinkTime = 0;

	// The per-box sound currently playing, so it can be waited on and stopped
	SoundDescription _activeBoxSound;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_QUIZPUZZLE_H
