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
#include "engines/nancy/cursor.h"
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

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MultiBuildPuzzle"; }
	bool isValidDrop() const;

	// A single puzzle piece. Each piece is its own RenderObject.
	// Unplaced: _drawSurface shows srcRect from primary image.
	// Placed:   _drawSurface shows the rotation sprite (from altSrcRect or srcRect).
	struct Piece : RenderObject {
		Piece() : RenderObject(0) {}

		Common::Rect srcRect;     // Source in primary image (unplaced visual)
		Common::Rect homeRect;    // Slot position in viewport coords
		Common::Rect altSrcRect;  // If non-empty: used as source for sprite creation
		Common::Rect cuSrcRect;   // Source in closeup image
		uint8 counterByte = 0;    // Non-zero: respawns on placement; doesn't count toward solve
		uint8 mustPlace = 0;      // Must be placed for solution
		uint8 mustNotPlace = 0;   // Placing this fails the solution check

		Common::Rect gameRect;    // Current viewport-space rect
		int curRotation = 0;
		bool isPlaced = false;
		int  typeIdx = -1;        // -1 for original pieces; source piece index for counter clones

		Graphics::ManagedSurface rotateSurfaces[4];
		bool hasSurface[4] = {};

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }

		bool isViewportRelative() const override { return true; }
	};

	Common::Path _primaryImageName;
	Common::Path _closeupImageName;
	bool _hasCloseupImage = false;

	uint16 _numPieces = 0;
	uint16 _requiredPieces = 0;        // Minimum placed pieces (counterByte==0) for solve check
	bool _autoSolveOnDrop = false;     // If true, solve check fires after each drop
	bool _canRotateAll = false;
	bool _useRotationHotspot = false;  // Rotation triggered only by clicking a hotspot rect
	int16 _rotHotspotHeight = 0;
	int16 _rotHotspotWidth = 0;
	bool _allowAltZoneSnap = false;    // Allow drop outside target zone if stacking on a moved piece
	bool _checkOverlapOnDrop = false;  // Reject drop if it overlaps an already-placed piece

	Common::Array<Piece> _pieces;

	SoundDescription _rotationSound;
	SoundDescription _pickupSound;
	SoundDescription _dropSound;

	int16 _dragCursorID  = 16;  // Cursor for hover/drag/drop (kCustom1 by default)
	int16 _exitCursorID1 = -1;  // -1: use _puzzleExitCursor
	int16 _exitCursorID2 = -1;

	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;
	Common::String _solveTextKey;  // Looked up in CONVO chunk first
	Common::String _solveText;     // Raw fallback used if key missing

	SceneChangeWithFlag _cancelScene;
	Common::Rect _exitHotspot;
	Common::Rect _exitHotspot2;
	Common::Rect _targetZone;      // Valid drop area (drawer/plate/...)

	Graphics::ManagedSurface _primaryImage;
	Graphics::ManagedSurface _closeupImage;

	int16 _selectedPiece = -1;  // Piece shown in closeup view, -1 if none
	int16 _pickedUpPiece = -1;  // Piece being dragged, -1 if none
	bool _isDragging = false;
	bool _isSolved = false;
	bool _isCancelled = false;

	enum SolveState {
		kIdle           = 0,
		kWaitTimer      = 1,
		kWaitSolveSound = 4,
		kPlaySolveSound = 5
	};
	SolveState _solveState = kIdle;
	uint32 _timerEnd = 0;

	int16 _pickedUpWidth = 0;
	int16 _pickedUpHeight = 0;

	bool _isInitialized = false;

	void checkIfSolved();
	void checkIfSolvedOnExit();
	void updatePieceRender(int pieceIdx);
	static void rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst);
	// Clone an existing piece at the end of _pieces (counter-piece respawn).
	void spawnCounterPiece(int srcIdx);
	// Drop is valid if it overhangs the top of a moved piece (sand-castle stacking).
	bool altZoneSnapValid() const;
	// Map data cursor id (0..21) to CursorManager::CursorType; out-of-range falls back.
	CursorManager::CursorType cursorFromDataID(int16 id, CursorManager::CursorType fallback) const;
	// Tests one exit hotspot and (if hovered) sets its cursor / handles the click.
	// Returns true when the cursor is inside `hot`, so the caller can skip the
	// other hotspot.
	bool checkExitHotspot(const Common::Rect &hot, int16 cursorID, const NancyInput &input);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MULTIBUILDPUZZLE_H
