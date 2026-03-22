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

#ifndef NANCY_ACTION_SOUNDMATCHPUZZLE_H
#define NANCY_ACTION_SOUNDMATCHPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Whale sound-matching puzzle used in Nancy 9 (Danger on Deception Island).
// The player hears a whale call by clicking one of 5 numbered buttons, then
// clicks the matching whale image. Correct pairs stay lit. Player wins when
// all the required pairs have been matched.
class SoundMatchPuzzle : public RenderActionRecord {
public:
	SoundMatchPuzzle() : RenderActionRecord(7) {}
	virtual ~SoundMatchPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "SoundMatchPuzzle"; }
	bool isViewportRelative() const override { return true; }

	// File data

	Common::Path _imageNameLitButtons;

	static const int kNumButtons = 5;

	SoundDescription _feedbackSoundWrong;  // played on incorrect whale click
	SoundDescription _feedbackSoundRight;  // played on correct whale click
	SceneChangeWithFlag _winScene;
	SoundDescription _winSound;            // played when all pairs are matched
	SceneChangeWithFlag _exitScene;

	Common::Rect _exitHotspot;

	uint16 _requiredPairs = kNumButtons;   // how many matches needed to win

	struct SoundButtonEntry {
		SoundDescription sound;   // whale call sound
		Common::String text;      // onomatopoeia shown in the text box
		Common::Rect numSrcRect;  // overlay src for numbered button lit state
		Common::Rect numDestRect; // screen dest for numbered button (hotspot)
		bool matched = false;     // whether the sound button has been correctly paired
	};

	SoundButtonEntry _soundButtons[kNumButtons];

	struct WhaleButtonEntry {
		Common::Rect whaleSrcRect;  // overlay src for whale button lit state
		Common::Rect whaleDestRect; // screen dest for whale button (hotspot)
		uint16 correctSound = 0;    // index (0..4) of the correct sound button
		bool matched = false;       // whether the whale button has been correctly paired
	};

	WhaleButtonEntry _whaleButtons[kNumButtons];

	// Runtime state

	Graphics::ManagedSurface _imageLitButtons;

	int  _selectedSoundButton  = -1;    // currently selected numbered button, -1 = none
	int  _matchedPairs    = 0;
	bool _isExiting       = false;

	enum SolveSubState {
		kIdle         = 0,
		kCheckMatch   = 1,
		kSoundPlaying = 2,
		kWinSound     = 4
	};
	SolveSubState _solveSubState = kIdle;

	// NOTE: In the original, the sound button and whale button
	// data were intertwined in the puzzle data, and the sound
	// button entries were not stored in order of their actual
	// button index. This array maps the sound button entries to
	// their actual button index (0..4) for easier handling at
	// runtime.
	// This means that this implementation is only valid for
	// Nancy 9's specific puzzle data, but that's not a problem
	// since this puzzle is only used in that game.
	uint16 _soundButtonIndex[kNumButtons] = { 3, 2, 4, 0, 1 };

	// Internal methods

	void redraw();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SOUNDMATCHPUZZLE_H
