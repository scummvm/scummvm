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
	OneBuildPuzzle() : RenderActionRecord(7), _finalAnimOverlay(99), _counterDisplay(99) {}
	virtual ~OneBuildPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void readDataNancy12(Common::SeekableReadStream &stream);
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "OneBuildPuzzle"; }

	// Nancy 12 repurposed the piece's trailing flag byte into the rotation the
	// piece must be in to be accepted, with this value standing in for the
	// pre-placed flag it used to be.
	static const uint8 kPrePlacedRotation = 10;

	enum PlacementMode {
		kPlacementNormal = 1,	// A placed piece stays on screen, drawn in its slot
		kPlacementCounter = 2	// A placed piece drops out of sight into its slot
	};

	// What the puzzle counts, both to decide when it is finished and to fill in
	// the on-screen counter.
	enum CountMode {
		kCountAllPieces = 0,	// No counter; the puzzle ends once every piece is in its slot
		kCountPlacements = 1,	// Correct placements
		kCountMistakes = 2		// Mistakes, or pieces left to place outside counter placement
	};

	static const uint kNumDigits = 10;

	struct Piece : RenderObject {
		Piece() : RenderObject(0) {}

		// File data
		Common::Rect srcRect;
		Common::Rect altSrcRect;    // At-home art, shown until first pickup
		Common::Rect slotRect;
		Common::Rect homeRect;
		uint8 defaultRotation = 0;
		// Rotation the piece must be in to be accepted into its slot. Always 0
		// before Nancy 12, which is why older games only accept upright pieces.
		uint8 requiredRotation = 0;
		bool isPreRotated = false;

		// Runtime
		Common::Rect gameRect;      // Current viewport-space rect
		int curRotation = 0;
		bool placed = false;

		// Rotations 1-3 only built when canRotateAll or the piece starts rotated
		Graphics::ManagedSurface rotateSurfaces[4];
		bool hasSurface[4] = {};

		Graphics::ManagedSurface altSurface;
		bool useAltSurface = false;

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }

		bool isViewportRelative() const override { return true; }
	};

	// --- File data ---

	Common::Path _imageName;
	uint16 _numPieces = 0;         // Number of piece descriptions in the puzzle data
	uint16 _totalPieces = 0;       // Number of pieces on screen; see init() for the extra ones
	bool _freePlacement = false;   // Wrong drop restores to previous position, not home
	bool _canRotateAll = false;    // All pieces can be rotated
	int16 _slotTolerance = 0;      // Proximity for snapping to slot
	bool _orderedPlacement = false; // Pieces must be placed in a specific order
	PlacementMode _placementMode = kPlacementNormal; // Nancy 12 only, earlier games are always normal
	Common::Array<int16> _placementOrder; // 1-indexed piece IDs in required placement order

	// Counter puzzles (Nancy 12): the puzzle is solved once _requiredPieces have
	// been placed correctly, and lost once the remaining pieces have all been
	// dropped in the wrong slot. The running count is drawn from digit sprites
	// found in the puzzle image.
	CountMode _countMode = kCountAllPieces;
	int16 _requiredPieces = 0;
	Common::Rect _digitSrcRects[kNumDigits];
	Common::Point _counterPos;
	int16 _counterSpacing = 0;

	// Stacking order of the pieces that start out already placed, 1-indexed.
	// TODO: not applied yet; pre-placed pieces keep their array order.
	Common::Array<int16> _preplacedZOrder;

	// --- Nancy 10 additions ---

	// Filename only (no SoundDescription metadata).
	Common::String _extraSoundName;

	// Cursor type shown while hovering a piece (Nancy 10+), and the one shown
	// while carrying it (Nancy 12+; the older games reuse the hover cursor).
	int16 _pieceCursorType = 0;
	int16 _heldPieceCursorType = 0;

	// Post-placement sprite-sheet animation. _animRectA is the on-screen
	// rect where the animation plays AND the click hotspot the user must
	// activate after placing all pieces (e.g. the music-box crank in scene
	// 3637; could be any handle/lever/switch in other puzzles).
	Common::Rect _animRectA;
	Common::Rect _animRectB;
	int16 _animLayout[6] = {}; // cols, framesPerStep, baseX, baseY, spacing, totalRows
	SoundDescription _animSound1;
	SoundDescription _animSound2;  // "bad" noise played when the crank is turned before the puzzle is solved
	bool _hasFinalAnim = false;   // true when _animRectA is non-empty (the animation atlas region)
	bool _hasCrank = false;       // true when _animRectB is non-empty; the puzzle is solved by turning the crank

	// Forks can only be dragged onto / released inside this region (the
	// contraption area), not the whole viewport. Empty when the puzzle has no
	// such constraint.
	Common::Rect _placementZone;

	// Nancy12: pieces with an empty home rect start scattered inside this zone.
	Common::Rect _scatterZone;

	Common::Array<Piece> _pieces;

	// Nancy12 stores the puzzle's sounds as this many random-sound blocks, in
	// this fixed on-disk order.
	static const uint kNumSounds = 6;
	static const uint kPickupSound = 0;
	static const uint kRotateSound = 1;
	static const uint kDropSound = 2;
	static const uint kGoodSound = 3;
	static const uint kBadSound = 4;
	static const uint kCompletionSound = 5;

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
		kTriggerCompletion = 4, // play completion sound/text, then transition to kWaitCompletion
		kAnimateFinal      = 5  // step the post-placement animation, then trigger completion
	};
	SolveState _solveState = kIdle;
	bool _isDropSound = false;       // True if last sound played was a drop sound
	bool _correctlyPlaced = false;   // True if the last drop was correctly placed
	uint16 _piecesPlaced = 0;    // Number of pieces correctly placed so far
	uint16 _mistakes = 0;        // Number of pieces dropped in the wrong slot
	uint32 _timerEnd = 0;        // Millisecond timestamp when the current timer expires

	bool _finalAnimDone = false;

	// Final-animation runtime state (matches original `+0xc35`/`+0xc33` per-tick counters).
	Graphics::ManagedSurface _animImage; // Source atlas loaded from _extraSoundName.
	RenderObject _finalAnimOverlay;      // Single-frame overlay rendered at _animRectA, z above pieces.
	int16 _animFrameCounter = 0;         // 0..framesPerStep-1, the X index within the current row.
	int16 _animRowCounter = 0;           // 0..totalRows-1, how many cycles have completed.

	RenderObject _counterDisplay;        // Digit sprites showing the running count.

	// Previous drag position (for freePlacement restore on wrong drop)
	Common::Rect _prevDragGameRect;

	// Current rotation surface dimensions for the picked-up piece
	int16 _pickedUpWidth = 0;
	int16 _pickedUpHeight = 0;

	// Currently playing sound (scratch copy updated each time a sound is played)
	SoundDescription _currentSound;

	// Initialization flag, used to ensure that the puzzle pieces have been initialized
	// before drawing them on screen
	bool _isInitialized = false;

	// --- Internal methods ---

	// Read a good/bad caption block: three AUTOTEXT keys then three inline
	// texts; each caption uses its key if known, else the inline text.
	void readPlacementTexts(Common::SeekableReadStream &stream, Common::Array<Common::String> &out);
	void setPieceCursor(bool isHeld = false);

	void playPickupSound();
	void playRotateSoundAndStartTimer();
	void playDropSound();
	void playGoodPlacementSound();
	void playBadPlacementSound();
	void checkAllPlaced();
	// Place a piece at a random spot inside _scatterZone (Nancy12 empty-home pieces)
	void scatterPiece(Piece &p);
	// Index of the first slot the given rect fits inside, or -1 if it fits none
	int16 findSlotAt(const Common::Rect &rect) const;
	// Redraw the counter with the value the puzzle's count mode calls for
	void updateCounter();
	void rotatePiece(int pieceIdx);
	void updateDragPosition(Common::Point mouseVP);
	// Update the render object for a piece (set _drawSurface and moveTo gameRect)
	void updatePieceRender(int pieceIdx);
	// Rotate a surface 90 degrees clockwise into dst (dst is allocated here)
	static void rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst);
	// Clamp rect to viewport bounds while preserving dimensions
	void clampRectToViewport(Common::Rect &rect);

	// Final-animation helpers.
	void startFinalAnimation();
	void stepFinalAnimation();
	// After a crank turn finishes: solve if every fork is placed, otherwise
	// play the "bad" noise and let the player try again.
	void finishCrankTurn();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ONEBUILDPUZZLE_H
