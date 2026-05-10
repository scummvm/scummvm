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

namespace Nancy {
namespace Action {

// Text-entry quiz with multiple text boxes.
// Different implementation for Nancy 8 vs Nancy 9+ (which has more
// features and a different data format)
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
	static const int kMaxBoxes = 8;

	// Format-specific read/execute implementations
	void readDataOld(Common::SeekableReadStream &stream); // Nancy 8
	void readDataNew(Common::SeekableReadStream &stream); // Nancy 9+
	void executeOld(); // Nancy 8 state machine
	void executeNew(); // Nancy 9+ state machine

	// Helpers (shared by Nancy 8 and Nancy 9)
	void drawText();
	void advanceToNextBox();
	bool checkAllSolved() const;
	bool checkAnswerForCurrentBox(); // checks, marks correct, sets event flag

	Common::String readSubtitle(Common::SeekableReadStream &stream);
	void showSubtitle(const Common::String &text);

	// ---- Data (Nancy 8) ----
	uint16 _fontID = 0;
	uint16 _cursorBlinkInterval = 500;
	uint16 _numBoxes = 0;

	// Text-box screen rects (viewport-relative), up to kMaxBoxes stored
	Common::Rect _boxRects[kMaxBoxes];

	// Per-box answer data: up to 3 valid answers (case-insensitive match), plus
	// an optional event flag to set when that box is answered correctly.
	Common::String _answers[kMaxBoxes][3];
	int16 _answerFlags[kMaxBoxes] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	SoundDescription _correctSound;  // Nancy 8: global correct sound
	Common::String _correctText;
	SoundDescription _wrongSound;    // Nancy 8: global wrong sound
	Common::String _wrongText;

	SoundDescription _doneSound;     // done sound (both Nancy 8 and Nancy 9)
	Common::String _doneText;

	SceneChangeWithFlag _solveScene;   // scene to go to when all boxes are solved
	SceneChangeWithFlag _cancelScene;  // scene to go to on cancel

	// ---- Data (Nancy 9+) ----
	char _cursorChar = '-';              // cursor character (configurable in Nancy 9)
	Common::String _allowedChars;        // allowed typing chars (empty = all allowed)
	bool _autoCheck = false;             // check answer after each char typed
	bool _skipEmptyOnEnter = false;      // if true, Enter on empty box does nothing

	uint16 _correctSoundChannel = 0;    // global channel for per-box correct sounds
	uint16 _wrongSoundChannel = 0;      // global channel for per-box wrong sounds

	// Per-box sounds for Nancy 9 (name + volume; channel from global above)
	Common::String _boxCorrectSoundName[kMaxBoxes];
	uint16 _boxCorrectSoundVolume[kMaxBoxes] = {};
	Common::String _boxCorrectText[kMaxBoxes];

	Common::String _boxWrongSoundName[kMaxBoxes];
	uint16 _boxWrongSoundVolume[kMaxBoxes] = {};
	Common::String _boxWrongText[kMaxBoxes];

	// Per-box max answer length (computed from answer strings, used in auto-check mode)
	uint16 _boxMaxLen[kMaxBoxes] = {};

	// ---- Runtime state (shared) ----
	Common::String _typedText[kMaxBoxes]; // current text in each box (may end with cursor char)
	bool _boxCorrect[kMaxBoxes] = {};     // true when box i has been answered correctly
	uint16 _currentBox = 0;              // which box currently receives keyboard input
	bool _solved = false;                 // all boxes answered correctly
	bool _cancelled = false;             // user cancelled

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

	// Runtime: assembled sound description for current per-box sound (Nancy 9)
	SoundDescription _activeBoxSound;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_QUIZPUZZLE_H
