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

#ifndef NANCY_ACTION_MEMORYPUZZLE_H
#define NANCY_ACTION_MEMORYPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Memory matching puzzle, used in the Nancy 9 toy box.
// It has three tabs, each containing 24 cards(6x4 grid).
// The player flips cards to find matching pairs; matching pairs stay face-up.
// Player wins when matchedPairs >= requiredPairs.
class MemoryPuzzle : public RenderActionRecord {
public:
	MemoryPuzzle() : RenderActionRecord(7) {}
	virtual ~MemoryPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "MemoryPuzzle"; }
	bool isViewportRelative() const override { return true; }

	// File data

	Common::Path _imageName;

	static const int kMaxTypes    = 36; // 3 tabs x 12 pairs each
	static const int kCardsPerTab = 24; // hardcoded in original
	static const int kNumTabs     = 3;  // hardcoded in original

	Common::Rect _faceSrcRects[kMaxTypes];   // [type] -> face src rect on image
	Common::Rect _tabSrcRects[kNumTabs];     // [tab]  -> tab indicator src rect (drawn for active tab)

	Common::Rect _cardRects[kCardsPerTab];   // viewport-relative screen positions (shared across tabs)
	Common::Rect _tabRect;                   // screen rect where the active tab indicator is drawn
	Common::Rect _tabHotspots[kNumTabs][3];  // [currentTab][targetTab]: hit-test rects for tab switching
	Common::Rect _exitHotspot;

	uint32 _numPairs      = 12;    // pairs in the shuffled layout (clamped [4..36])
	uint32 _requiredPairs = 12;    // pairs needed to win (clamped [2..36])
	uint32 _flipDelay     = 1500;  // ms before non-matching cards flip back

	bool _shuffleGlobal = false;  // false = pairs stay within same tab; true = can cross tabs

	SoundDescription _cardFlipSound; // played when a card is flipped face-up
	SoundDescription _matchSound;    // played when a matching pair is found
	SceneChangeWithFlag _winScene;
	SoundDescription _winSound;

	// Runtime state

	struct CardState {
		int typeId     = -1; // 0..35; -1 = unassigned
		int flipState  = 0;  // 0 = face-down, 1 = face-up (player-flipped, unmatched)
		int matchState = 0;  // 0 = unmatched, 1 = matched (stays face-up)
	};

	// _cards[tab * kCardsPerTab + i] = state of card i on tab `tab`
	CardState _cards[kNumTabs * kCardsPerTab];

	Graphics::ManagedSurface _image;

	int    _currentTab      = 0;
	int    _firstFlip       = -1;   // absolute card index of first face-up unmatched card
	int    _secondFlip      = -1;   // absolute card index of second (timer pending)
	bool   _flipTimerActive = false;
	uint32 _flipTimerEnd    = 0;
	int    _matchedPairs    = 0;
	bool   _isSolved        = false;

	enum SolveSubState {
		kPlaying = 0,
		kPlayWinSound = 1,
		kWaitWinSound = 2
	};
	SolveSubState _solveSubState = kPlaying;

	// Internal methods

	void initCards();     // shuffle types into all 72 card slots
	void checkIfSolved();
	void flipBackCards(); // unflip non-matching pair after timer expires
	void redrawCards();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MEMORYPUZZLE_H
