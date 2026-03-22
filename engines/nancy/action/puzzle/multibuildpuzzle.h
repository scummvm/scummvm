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

#ifndef NANCY_ACTION_MULTIBUILDPUZZLE_H
#define NANCY_ACTION_MULTIBUILDPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {
namespace Action {

// Click-and-drag assembly puzzle used in Nancy 9.
// Used for three puzzles:
// - book sorting: click and drag book pieces in a drawer, leaving no gaps (win
//   condition is checked on placement)
// - sandwich making: click and drag ingredients onto a plate. Some ingredients
//   are bad and lead to food poisoning (win condition is checked on exit)
// - sand castle building: free placement of sand pieces (no win condition)
class MultiBuildPuzzle : public RenderActionRecord {
public:
	MultiBuildPuzzle() : RenderActionRecord(7) {}
	virtual ~MultiBuildPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "MultiBuildPuzzle"; }
	bool isViewportRelative() const override { return true; }

	// A single puzzle piece. Each piece is its own RenderObject.
	// Unplaced: _drawSurface shows srcRect from primary image.
	// Placed:   _drawSurface shows the rotation sprite (from altSrcRect or srcRect).
	struct Piece : RenderObject {
		Piece() : RenderObject(0) {}

		// --- File data ---
		Common::Rect srcRect;     // Source rect in primary image (unplaced visual)
		Common::Rect homeRect;    // Screen position in viewport coords (= the slot)
		Common::Rect altSrcRect;  // If non-zero area: used as source for sprite creation
		Common::Rect cuSrcRect;   // Source rect in secondary image (overlay when placed)
		uint8 counterByte = 0;    // If 0, piece counts toward requiredPieces tally
		uint8 mustPlace = 0;      // If 1, piece MUST be placed for solution
		uint8 mustNotPlace = 0;   // If 1, placing this piece fails the solution check

		// --- Runtime ---
		Common::Rect gameRect;    // Current viewport-space rect (homeRect or cursor-following)
		int curRotation = 0;
		bool isPlaced = false;

		// Up to 4 rotation surfaces (rotation 1-3 only if canRotateAll or altSrcRect non-zero)
		Graphics::ManagedSurface rotateSurfaces[4];
		bool hasSurface[4] = {};

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }

	protected:
		bool isViewportRelative() const override { return true; }
	};

	// --- File data ---

	Common::Path _primaryImageName;
	Common::Path _closeupImageName;
	bool _hasCloseupImage = false;

	uint16 _numPieces = 0;
	uint16 _requiredPieces = 0;  // Minimum placed pieces (with counterByte==0) needed to trigger solve check
	bool _canRotateAll = false;

	Common::Array<Piece> _pieces;
	// For closeup puzzles (e.g. sandwich): permanent shelf visuals that stay at homeRect
	// so the shelf always shows the source ingredient regardless of where the active piece is.
	Common::Array<Piece> _shelfSlots;

	SoundDescription _sounds[3];  // [0]=pickup/move, [1]=placement, [2]=extra

	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;
	Common::String _solveText;

	SceneChangeWithFlag _cancelScene;
	Common::Rect _exitHotspot;
	Common::Rect _targetZone;  // Valid drop area (drawer/plate/etc.); pieces must be within this to solve

	// --- Runtime state ---

	Graphics::ManagedSurface _primaryImage;
	Graphics::ManagedSurface _closeupImage;

	int16 _selectedPiece = -1;  // Index of selected piece (CU view shown, not yet dragging), -1 if none
	int16 _pickedUpPiece = -1;  // Index of piece being dragged, -1 if none
	bool _isDragging = false;
	bool _isSolved = false;
	bool _isCancelled = false;

	enum SolveState {
		kIdle          = 0,
		kWaitTimer     = 1,
		kWaitSolveSound = 4,  // Waiting for solve sound to stop playing
		kPlaySolveSound = 5   // Trigger solve sound + text, then wait
	};
	SolveState _solveState = kIdle;
	uint32 _timerEnd = 0;

	int16 _pickedUpWidth = 0;
	int16 _pickedUpHeight = 0;

	// --- Internal methods ---

	void checkIfSolved();	// FUN_0046da47
	void checkIfSolvedOnExit();
	// Update a piece's _drawSurface and position to match its current state
	void updatePieceRender(int pieceIdx);
	// Rotate a surface 90 degrees clockwise into dst (dst is allocated here)
	static void rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MULTIBUILDPUZZLE_H
