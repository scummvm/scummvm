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

#ifndef NANCY_ACTION_BUILDPUZZLE_H
#define NANCY_ACTION_BUILDPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {
namespace Action {

// Nancy 14 reuses AR 166 for a rebuilt assembly puzzle (tea, cookies, parfait,
// clothes design). Pieces are dragged into zones, and a zone is satisfied once
// it holds the quantities its ingredient list asks for; a piece is placed by
// being assigned a zone index rather than by matching a rect.
class BuildPuzzle : public RenderActionRecord {
public:
	BuildPuzzle() : RenderActionRecord(7), _cursorItem(99) {}
	virtual ~BuildPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BuildPuzzle"; }

	static const uint kNumDigits = 10;

	// One entry of a zone's required contents.
	struct Ingredient {
		int16 pieceID = 0;
		byte quantity = 0;
		byte mode = 0;
	};

	// How a zone arranges a piece dropped into it. A zone with no fill mode just
	// absorbs the ingredient: nothing new is drawn and the piece goes back home.
	enum ZoneFill {
		kFillAbsorb = 0,
		kFillCentered = 1,
		kFillTopLeft = 2
	};

	static const byte kConsumedKind = 1;	// this kind is taken by the zone, not copied

	// A container pieces are dropped into.
	struct Zone {
		Common::Rect hotspot;
		uint16 capacity = 0;		// the zone counts as full at this many pieces
		ZoneFill fill = kFillAbsorb;
		byte marksPlaced = 0;
		Common::Array<Ingredient> ingredients;

		// Runtime
		Common::Array<byte> counts;	// how many of each ingredient are in the zone
		int16 numWrong = 0;			// pieces in here that no ingredient asked for
		int16 numHeld = 0;
	};

	// A measuring implement. Scooping with one adds `amount` of an ingredient at
	// a time instead of a single unit.
	struct HoldSlot : RenderObject {
		HoldSlot() : RenderObject(0) {}

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }
		bool isViewportRelative() const override { return true; }

		Common::Rect srcRect;			// empty
		Common::Rect destRect;
		Common::Rect fillSrcRect1;		// holding an ingredient, by its fillVariant
		Common::Rect fillSrcRect2;
		byte amount = 0;				// how much one scoop is worth
	};

	struct Piece : RenderObject {
		Piece() : RenderObject(0) {}

		void setZ(uint16 z) { _z = z; _needsRedraw = true; }
		bool isViewportRelative() const override { return true; }

		Common::Rect srcRect;			// art while the piece sits at home
		Common::Rect destRect;			// where the piece is drawn
		Common::Rect dragSrcRect;		// art while the piece is on the cursor; empty means srcRect
		Common::Rect placedSrcRect;		// art once the piece is in a zone; empty means dragSrcRect
		Common::Rect closeupSrcRect;
		Common::Rect closeupDestRect;	// empty means centered on the viewport
		Common::Rect placedDestRect;	// exact spot in a zone; overrides the zone's fill mode
		byte kind = 0;
		Common::String imageName;	// only when kind == 3
		int16 zoneID = -1;			// the only zone this piece may go in, -1 = any
		int16 itemID = 0;			// index into the shared item state, 255 = none
		Common::Array<int16> holds;	// the scoops this piece can be taken with, empty = by hand
		int16 fillVariant = 0;		// which of a scoop's two full images to show

		// Runtime
		Common::Rect liveRect;		// where the piece currently sits
		int16 sourceID = -1;		// index of the definition this piece was cloned from
		int16 assignedZone = -1;
		bool inUse = false;			// false for the spare slots kept for clones
	};

	Common::Path _imageName;
	Common::Path _altImageName;		// empty means the main image is used for both

	// Selects where a piece that is not in a zone, and any close-up, is drawn
	// from: 1 means the main image, anything else the alt one. A piece sitting
	// in a zone always comes from the alt image.
	byte _trayImageMode = 0;

	// Two animations played over the puzzle, each with the rect it plays in.
	Common::Path _anim1Name;
	Common::Rect _anim1Rect;
	Common::Path _anim2Name;
	Common::Rect _anim2Rect;

	// Shown while hovering a piece, and while carrying one.
	int16 _pieceCursorType = 0;
	int16 _heldPieceCursorType = 0;

	Common::Array<Zone> _zones;
	Common::Array<HoldSlot> _holds;
	Common::Array<Piece> _pieces;

	// Index into the shared item state whose value is drawn as a running count.
	// 255 means the puzzle has no counter.
	byte _counterItemID = 255;
	Common::Rect _digitSrcRects[kNumDigits];
	Common::Point _counterPos;
	int32 _counterSpacing = 0;

	SoundDescription _pickupSound;
	SoundDescription _dropSound;
	SoundDescription _notebookSound;
	SoundDescription _resetSound;
	SoundDescription _holdSound;

	// Both cleared when the puzzle starts from scratch.
	int16 _wrongIngredientFlag = -1;	// set once something not in a recipe is dropped in
	int16 _solvedFlag = -1;

	// When _usePlacedGate is set, the scene only changes once this many pieces
	// have been placed.
	uint16 _requiredPlaced = 0;
	byte _usePlacedGate = 0;
	uint16 _stateItemID = 255;			// shared item state tracking the placed count

	SceneChangeDescription _solveScene;
	FlagDescription _solveFlag;

	// Used instead of _solveScene when the player leaves the zones unfinished.
	SceneChangeDescription _failScene;
	FlagDescription _failFlag;

	// --- Runtime ---

	Graphics::ManagedSurface _image;
	Graphics::ManagedSurface _altImage;
	Graphics::ManagedSurface _pieceImage;	// a kind 3 piece's own close-up art
	Common::Path _pieceImageName;

	// Whatever is currently on the cursor: a scoop, or an ingredient.
	RenderObject _cursorItem;
	int16 _activeHold = -1;
	int8 _lastWrongFlag = -1;
	int8 _lastSolvedFlag = -1;
	int16 _heldPiece = -1;
	int16 _closeupPiece = -1;
	uint16 _numDefined = 0;			// pieces read from the record; the rest are spare slots
	bool _isSolved = false;
	bool _leaveRequested = false;
	bool _isInitialized = false;

	void setPieceCursor(bool isHeld);
	void setPieceZ(int16 pieceIdx, uint16 z);
	// Draw the carried art at the cursor, or hide it when nothing is carried
	void updateCursorItem(const Common::Point &mouseVP);
	// The scoop a piece is dropped with, 1 when it is carried by hand
	byte carriedAmount() const;
	// Writing an event flag re-triggers whatever reacts to it, so both of the
	// puzzle's flags are only written when their value actually changes.
	void setFlagOnChange(int16 label, bool value, int8 &last);
	// Show a piece's close-up, or dismiss the one that is showing
	void openCloseup(int16 pieceIdx);
	void closeCloseup();
	// Attach a piece to the cursor, and release it over a zone or back home
	void pickUpPiece(int16 pieceIdx);
	void placePiece(int16 pieceIdx, int16 zoneIdx, const Common::Point &dropPos);
	void returnPiece(int16 pieceIdx);
	// Add or remove a piece from a zone's tallies
	void adjustZone(int16 zoneIdx, int16 pieceID, int8 delta);
	// The spare slot a clone goes into, or -1 when the puzzle has run out
	int16 clonePiece(int16 pieceIdx);
	void updatePieceRender(int16 pieceIdx);
	bool checkSolved() const;

	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BUILDPUZZLE_H
