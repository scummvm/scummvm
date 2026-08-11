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

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Four magnet pieces, four targets on a maze board. The player picks a piece
// from its home slot and drops it onto its target zone; an overlap snaps the
// piece to a fixed position and locks it. Reset button returns all pieces
// home. Wins when every piece is locked into its target.
// Called in Nancy10 from scenes 3280 (normal mode, with overlaid pieces) and
// 3281 (easy mode, without overlaid pieces, triggered as a cheat mode when
// visiting the puzzle multiple times).
class MagnetMazePuzzle : public RenderActionRecord {
public:
	MagnetMazePuzzle() : RenderActionRecord(7) {}
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

	Common::Rect _resetButtonHotspot;

	SoundDescription _pickupSound;
	SoundDescription _placeSound;
	SoundDescription _resetSound;
	SoundDescription _bumpSound;

	SceneChangeDescription _winScene;
	FlagDescription        _winFlag;
	uint16                 _winDelaySec = 0;
	SoundDescription       _winSound;

	SceneChangeDescription _cancelScene;
	FlagDescription        _cancelFlag;

	Common::Rect _exitHotspot;

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

	int  _heldMagnet  = -1;
	bool _isSolved    = false;
	bool _cantPlayed[kNumMagnets] = {};
	uint32 _winDelayEndTime = 0;

	Common::Point _heldDrawPos;

	Graphics::ManagedSurface _boardImage;
	Graphics::ManagedSurface _mazeImage; // wall mask for pixel-collision tests

	// RGB of the maze image's "wall" pixel color; a magnet rect overlapping
	// any pixel of this color in the maze image is treated as colliding.
	byte _wallColorR = 0;
	byte _wallColorG = 0;
	byte _wallColorB = 0;

	void resetMagnets();
	void snapMagnet(int idx);
	void persistState();
	void redraw();
	void checkSolved();
	bool collidesAt(const Common::Rect &r) const;
	void stepMagnetToward(Common::Rect &cur, const Common::Rect &target) const;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MAGNETMAZEPUZZLE_H
