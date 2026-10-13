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

#ifndef NANCY_ACTION_MAGNETMAZEPUZZLE_H
#define NANCY_ACTION_MAGNETMAZEPUZZLE_H

#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Four metal pieces lie in a maze, partly hidden under wooden boards. With
// the magnet in hand, a piece is picked up by moving the cursor over it and
// then heads for the cursor at a limited speed; it is dropped as soon as the
// cursor gets away from it, e.g. because a wall holds the piece back. A piece
// that reaches its target zone snaps into place and locks. The reset button
// returns all loose pieces home. Wins when every piece is locked.
// Called in Nancy10 from scenes 3280 (normal mode, with overlaid boards) and
// 3281 (easy mode, without overlaid boards, triggered as a cheat mode when
// visiting the puzzle multiple times).
class MagnetMazePuzzle : public PuzzleRecord {
public:
	MagnetMazePuzzle() : PuzzleRecord(7) {}
	virtual ~MagnetMazePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MagnetMazePuzzle"; }

	static const int kNumMagnets  = 4;
	static const int kNumOverlays = 6;

	// File data

	Common::Path _boardImageName;
	Common::Path _mazeImageName;

	Common::Rect _overlaySrcRects[kNumOverlays];
	Common::Rect _overlayDestRects[kNumOverlays];

	bool _overlayTransparent = false;
	bool _hideOverlays       = false;

	Common::Rect _magnetSrcRects[kNumMagnets];
	Common::Rect _magnetHomeRects[kNumMagnets];
	Common::Rect _magnetTargetRects[kNumMagnets];

	int16 _requiredItem = -1;

	// When set, a piece is hovered while the center of the cursor image is
	// inside it; otherwise the whole cursor image is tested against the piece
	bool _hoverUsesCursorCenter = false;

	UIButtonRecord _resetButton;

	SoundDescription _pickupSound;
	Common::String _pickupTextKey;
	Common::String _pickupText;
	SoundDescription _dropSound;
	Common::String _dropTextKey;
	Common::String _dropText;
	SoundDescription _moveSound;
	SoundDescription _lockSound;

	// Runtime state

	enum SubState {
		kPlaying = 0,
		kWaitWinDelay,
		kWaitWinSound,
		kExitToWin,
		kExitToCancel
	};

	SubState _subState = kPlaying;

	Common::Rect _magnetPos[kNumMagnets];
	bool _magnetLocked[kNumMagnets] = {};
	bool _magnetGrabbed[kNumMagnets] = {};

	int  _heldMagnet  = -1;
	bool _wasDropped  = false; // the pickup sound only plays after a drop
	bool _isSolved    = false;
	bool _cantPlayed[kNumMagnets] = {};
	uint32 _winDelayEndTime = 0;
	uint32 _lastMoveTime = 0;

	bool _resetPressed = false;
	uint32 _resetPressEndTime = 0;

	Graphics::ManagedSurface _boardImage;
	Graphics::ManagedSurface _mazeImage; // wall mask for pixel-collision tests
	Graphics::ManagedSurface _resetButtonImage;

	// RGB of the maze image's "wall" pixel color; a magnet rect overlapping
	// any pixel of this color in the maze image is treated as colliding.
	byte _wallColorR = 0;
	byte _wallColorG = 0;
	byte _wallColorB = 0;

	void resetMagnets();
	void lockMagnetsOnTargets();
	void dropMagnets();
	bool moveHeldMagnet(const Common::Point &cursorCenter);
	bool isMagnetHovered(int idx, const Common::Rect &cursorRect, const Common::Point &cursorCenter) const;
	void playPuzzleSound(SoundDescription &sound);
	void persistState();
	void redraw();
	bool collidesAt(const Common::Rect &r) const;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MAGNETMAZEPUZZLE_H
