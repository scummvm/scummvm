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

#ifndef NANCY_ACTION_MINDPUZZLE_H
#define NANCY_ACTION_MINDPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Mastermind clone introduced in Nancy12 (AR 165). The player guesses a hidden
// color code over a number of rows, by placing colored balls in each row. Each
// submitted guess is scored with flag poles (right color, right slot) and plain
// poles (right color, wrong slot).
class MindPuzzle : public RenderActionRecord {
public:
	MindPuzzle() : RenderActionRecord(7) {}
	virtual ~MindPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	static const uint kNumSounds = 6;
	static const uint kApplauseSound = 4;	// index into _sounds: the win cue
	static const uint kMaxRows = 10;
	static const uint kMaxColors = 10;	// fixed palette array size in the data
	static const uint kSlotsPerRow = 5;	// physical slots per row record (code length <= this)

	enum Feedback { kFeedbackNone = -1, kFeedbackWhite = 0, kFeedbackBlack = 1 };

protected:
	Common::String getRecordTypeName() const override { return "MindPuzzle"; }

	struct Row {
		// Per-slot dest rects: [0..codeLength) guess pegs, [5..5+codeLength) feedback pegs.
		Common::Rect slotRects[10];
	};

	// File data
	Common::Path _imageName;

	uint16 _numColors = 0;
	uint16 _codeLength = 0;
	uint16 _numRows = 0;
	byte _allowDuplicates = 0;
	uint16 _numGuesses = 0;

	Common::Array<Common::Rect> _ballSrcRects;	// each color's ball sprite in the overlay image (blit source)
	Common::Array<Common::Rect> _ballHitRects;	// bottom-row clickable ball positions, one per color
	Common::Rect _feedbackSrcRects[2];	// [0] right color, wrong slot (plain pole); [1] right color + slot (flag pole)
	Common::Rect _submitButtonRect;		// golf-club button: submit the current row
	Common::Rect _exitHotspot;			// give-up / leave hotspot
	Common::Array<Row> _rows;

	SceneChangeWithFlag _winScene;		// reached on a solved code
	SceneChangeWithFlag _loseScene;		// reached when out of guesses or when leaving via the exit hotspot

	RandomSoundBlock _sounds[kNumSounds];	// click / wall / wood / ball / applause / coin

	// Runtime state
	Common::Array<int16> _secret;		// _codeLength entries (color indices)
	Common::Array<int16> _secretColorCount;

	int16 _guess[kMaxRows][kSlotsPerRow];
	int16 _feedback[kMaxRows][kSlotsPerRow];

	int16 _currentRow = 0;
	int16 _heldColor = -1;
	Common::Point _heldDrawPos;		// viewport-local cursor while a ball is held
	int16 _remainingGuesses = 0;
	bool _solved = false;

	SoundDescription _outcomeSound;		// applause cue played once on a win
	bool _outcomeStarted = false;

	Graphics::ManagedSurface _image;

	void generateSecret();
	void scoreRow(int row);
	void drawPeg(int color, const Common::Rect &dest);
	void redraw();
	int  paletteHit(const Common::Point &mouseVP) const;
	bool slotHit(const Common::Point &mouseVP, int &slot) const;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MINDPUZZLE_H
