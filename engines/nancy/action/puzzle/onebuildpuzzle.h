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

#ifndef NANCY_ACTION_ONEBUILDPUZZLE_H
#define NANCY_ACTION_ONEBUILDPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {
namespace Action {

// Drag-and-drop assembly puzzle used in Nancy Drew: Danger on Deception Island (Nancy 9).
// Pieces can be rotated and must be placed in their correct slots.
// Left-clicking an unplaced piece picks it up; right-clicking any piece rotates it.
// If a piece is dropped (left-click) near its correct slot with correct rotation, it snaps in.
// Otherwise it returns to its previous position (or home in free placement mode).
class OneBuildPuzzle : public RenderActionRecord {
public:
	OneBuildPuzzle() : RenderActionRecord(7) {}
	virtual ~OneBuildPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "OneBuildPuzzle"; }

	struct Piece : RenderObject {
		Piece() : RenderObject(0) {}

		// File data
		Common::Rect srcRect;       // Source rect in source image
		Common::Rect slotRect;      // Correct placement rect (viewport coords)
		Common::Rect homeRect;      // Starting position (viewport coords)
		uint8 defaultRotation = 0;  // Rotation index that fits the slot
		bool isPreRotated = false;  // Piece starts already in place (slotRect position)

		// Runtime
		Common::Rect gameRect;      // Current viewport-space rect
		int curRotation = 0;
		bool placed = false;

		// Up to 4 rotation surfaces (rotation 1-3 only exist if canRotateAll or isPreRotated)
		Graphics::ManagedSurface rotateSurfaces[4];
		bool hasSurface[4] = {};

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }

		bool isViewportRelative() const override { return true; }
	};

	// --- File data ---

	Common::Path _imageName;
	uint16 _numPieces = 0;
	bool _freePlacement = false;   // Wrong drop restores to previous position, not home
	bool _canRotateAll = false;    // All pieces can be rotated
	int16 _slotTolerance = 0;      // Proximity for snapping to slot
	bool _orderedPlacement = false; // Pieces must be placed in a specific order
	Common::Array<int16> _placementOrder; // 1-indexed piece IDs in required placement order

	Common::Array<Piece> _pieces;

	SoundDescription _pickupSound;
	SoundDescription _rotateSound;
	SoundDescription _dropSound;
	Common::String _dropAlt1Filename;
	Common::String _dropAlt2Filename;

	SoundDescription _goodPlacementSound;
	Common::String _goodAlt1Filename;
	Common::String _goodAlt2Filename;
	Common::Array<Common::String> _goodTexts;    // 3 entries

	SoundDescription _badPlacementSound;
	Common::String _badAlt1Filename;
	Common::String _badAlt2Filename;
	Common::Array<Common::String> _badTexts;     // 3 entries

	SceneChangeWithFlag _solveScene;
	SoundDescription _completionSound;
	Common::String _completionText;

	SceneChangeWithFlag _cancelScene;
	Common::Rect _exitHotspot;

	// --- Runtime state ---

	Graphics::ManagedSurface _image;

	int16 _pickedUpPiece = -1;   // Index of currently dragged piece, -1 if none
	bool _isDragging = false;    // True while a piece is attached to the cursor
	bool _isSolved = false;
	bool _isCancelled = false;
	enum SolveState {
		kIdle             = 0, // normal interaction; handleInput drives piece movement
		kWaitTimer        = 1, // 300ms delay after pickup/drop before evaluating outcome
		kWaitPlaceSound   = 2, // waiting for good/bad placement sound (or 1s timer) to finish
		kWaitCompletion   = 3, // waiting for completion sound to finish before scene change
		kTriggerCompletion = 4  // play completion sound/text, then transition to kWaitCompletion
	};
	SolveState _solveState = kIdle;
	bool _isDropSound = false;       // True if last sound played was a drop sound
	bool _correctlyPlaced = false;   // True if the last drop was correctly placed
	uint16 _piecesPlaced = 0;    // Number of pieces correctly placed so far
	uint32 _timerEnd = 0;        // Millisecond timestamp when the current timer expires

	// Previous drag position (for freePlacement restore on wrong drop)
	Common::Rect _prevDragGameRect;

	// Current rotation surface dimensions for the picked-up piece
	int16 _pickedUpWidth = 0;
	int16 _pickedUpHeight = 0;

	// Currently playing sound (scratch copy updated each time a sound is played)
	SoundDescription _currentSound;

	// --- Internal methods ---

	void playPickupSound();	// FUN_0047239c
	void playRotateSoundAndStartTimer();	// FUN_0047212b
	void playDropSound();	// FUN_004721dc
	void playGoodPlacementSound();	// FUN_00472792
	void playBadPlacementSound();	// FUN_00472440
	void checkAllPlaced();	// FUN_00472ac6
	void rotatePiece(int pieceIdx);	// FUN_004719a5
	void updateDragPosition(Common::Point mouseVP);	// FUN_00471490
	// Update the render object for a piece (set _drawSurface and moveTo gameRect)
	void updatePieceRender(int pieceIdx);
	// Rotate a surface 90 degrees clockwise into dst (dst is allocated here)
	static void rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst);
	// Clamp rect to viewport bounds while preserving dimensions - FUN_004713b8
	void clampRectToViewport(Common::Rect &rect);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ONEBUILDPUZZLE_H
