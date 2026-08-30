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

#ifndef NANCY_CURSOR_H
#define NANCY_CURSOR_H

#include "common/array.h"
#include "common/stream.h"

#include "graphics/managed_surface.h"

namespace Nancy {

class CursorManager {

public:
	enum CursorType {
		kNormal 				= 0,	// Eyeglass (except in TVD), non-highlighted
		kHotspot 				= 1,	// Eyeglass (except in TVD), highlighted
		kMove 					= 2,	// Used for movement in early games
		kExit 					= 3,	// Used for movement, some games use it for exiting puzzles
		kRotateCW 				= 4,	// Used in puzzles only
		kRotateCCW 				= 5,	// Used in puzzles only
		kMoveLeft 				= 6,	// Used for movement, some games used it for turning in 360 scenes
		kMoveRight 				= 7,	// Used for movement, some games used it for turning in 360 scenes
		kMoveForward			= 8,	// Used for movement
		kMoveBackward			= 9,	// Used for movement, some games use it for exiting puzzles
		kMoveUp					= 10,	// Used for movement
		kMoveDown				= 11,	// Used for movement
		kRotateLeft				= 12,	// Used in 360 scenes in nancy6 and up
		kRotateRight			= 13,	// Used in 360 scenes in nancy6 and up
		kInvertedRotateRight	= 14,	// Used in 360 scenes with inverted rotation; nancy6 and up
		kInvertedRotateLeft		= 15,	// Used in 360 scenes with inverted rotation; nancy6 and up
		kCustom1				= 16,	// Custom cursors change between games; Likely used in puzzles
		kCustom1Hotspot			= 17,
		kCustom2				= 18,
		kCustom2Hotspot			= 19,
		kNormalArrow			= 20,
		kHotspotArrow			= 21,
		kHotspotTalk			= 22,	// Speech-bubble hover cursor (Nancy 10+)
		kDragHand				= 23,	// Hand cursor used when dragging an item (Nancy 10+)
		kDropHand				= 24,	// Drop-hand cursor used while a piece is held over a target (Nancy 10+)
		kPuzzleArrow			= 25,	// Puzzle arrow cursor shown when hovering a clickable puzzle hotspot (Nancy 10+)
		kDialCW					= 26,	// Dial turn cursors used by SafeDialPuzzle (Nancy 10+); the older
		kDialCCW				= 27,	// games use kRotateCW / kRotateCCW instead

		// Cursors in Nancy10 and newer games. The CURS chunk holds 37 system
		// cursor types in pairs; type T's idle slot is (T*2) and its hotspot
		// slot is (T*2 + 1). Types 0–4 and 18–26 have visually distinct idle
		// vs. hotspot sprites; types 5–17 and 27–36 use the same sprite for
		// both. Types 18+ are Nancy 10-specific puzzle/inventory cursors.
		kNewNormal 				= 0,	// Type 0  — Eyeglass
		kNewHotspot 			= 1,	// Type 0  hotspot — Eyeglass highlighted
		kNewUse					= 2,	// Type 1  — Open-hand "use" cursor (interact with characters/objects)
		kNewHotspotUse			= 3,	// Type 1  hotspot
		kNewLockedUse			= 4,	// Type 2  — Locked variant of the use hand
		kNewHotspotLockedUse	= 5,	// Type 2  hotspot
		kNewTalk				= 6,	// Type 3  — Speech-bubble (talking to characters)
		kNewHotspotTalk			= 7,	// Type 3  hotspot
		kNewNormalArrow			= 8,	// Type 4  — Taskbar arrow
		kNewHotspotArrow		= 9,	// Type 4  hotspot
		kNewExit 				= 10,	// Type 5  — Exit / back movement
		kNewMoveLeft 			= 12,	// Type 6  — Movement / 360 turn
		kNewMoveRight 			= 14,	// Type 7  — Movement / 360 turn
		kNewMoveForward			= 16,	// Type 8  — Movement
		kNewMoveBackward		= 18,	// Type 9  — Movement / exit puzzles
		kNewMoveUp				= 20,	// Type 10 — Movement
		kNewMoveDown			= 22,	// Type 11 — Movement
		kNewRotateCW 			= 24,	// Type 12 — Puzzle rotation
		kNewRotateCCW 			= 26,	// Type 13 — Puzzle rotation
		kNewRotateRight			= 28,	// Type 14 — 360 scenes
		kNewRotateLeft			= 30,	// Type 15 — 360 scenes
		kNewInvertedRotateRight = 32,	// Type 16 — Inverted 360 rotation
		kNewInvertedRotateLeft	= 34,	// Type 17 — Inverted 360 rotation
		kNewUseHand				= 36,	// Type 18 — Open hand, used while using items and while carrying a puzzle piece
		kNewUseHandHotspot		= 37,	// Type 18 hotspot — Open hand shown over a slot a carried piece can be dropped into
		kNewDragHand			= 38,	// Type 19 — Closed hand, used while dragging puzzle pieces
		kNewDragHandHotspot		= 39,	// Type 19 hotspot — Closed hand shown over a piece that can be picked up
		kNewRotatePiece			= 40,	// Type 20 — Rotate arrows shown over a rotatable puzzle piece
		kNewDialCW				= 41,	// Type 20 hotspot — Dial turn cursors, used by SafeDialPuzzle
		kNewDialCCW				= 43,	// Type 21 hotspot
		kNewPuzzleArrow			= 45,	// Type 22 hotspot — Arrow cursor shown when hovering a clickable puzzle hotspot
		kNewPuzzleSlideUp		= 47,	// Type 23 hotspot — Slide-arrow shown over a movable tile in CollisionPuzzle/TileMovePuzzle
		kNewPuzzleSlideDown		= 49,	// Type 24 hotspot
		kNewPuzzleSlideLeft		= 51,	// Type 25 hotspot
		kNewPuzzleSlideRight	= 53,	// Type 26 hotspot
		kNewDropHand			= 64,	// Type 32 — Hand shown when a held piece is dropped (briefly set on the drop action)
		kNewBlank				= 72,	// Type 36 — Empty sprite, used while a puzzle draws a carried piece at the mouse itself

		// Cursor types in Nancy13 and newer games. Nancy13 rebuilt the CURS
		// sheet with 45 system cursor types, each stored as an [idle, hotspot]
		// pair at slots (type * 2, type * 2 + 1). These constants are Nancy13
		// system TYPE indices (not slot indices); resolveNancy13CursorID turns a
		// type into the idle or hotspot slot.
		// Types 25-34 are the blue in-puzzle variants of the plain ones.
		kNancy13Normal				= 0,	// Eyeglass
		kNancy13Talk				= 5,
		kNancy13MoveUp				= 6,
		kNancy13MoveDown			= 7,
		kNancy13MoveLeft			= 8,
		kNancy13MoveRight			= 9,
		kNancy13MoveBackward		= 10,
		kNancy13MoveForward			= 12,
		kNancy13Arrow				= 14,
		kNancy13RotateRight			= 17,
		kNancy13RotateLeft			= 18,
		kNancy13Exit				= 19,
		kNancy13InvertedRotateRight	= 20,
		kNancy13InvertedRotateLeft	= 21,
		// Empty. Selected while the cell phone's camera viewfinder is up, so
		// only the viewfinder is seen.
		kNancy13Blank				= 24,
		kNancy13PuzzleRotateCW		= 25,
		kNancy13PuzzleRotateCCW		= 26,
		kNancy13PuzzleDragHand		= 28,
		kNancy13PuzzleDropHand		= 29,
		kNancy13PuzzleArrow			= 34
	};

	CursorManager();

	void init(Common::SeekableReadStream *chunkStream);

	// Change the current cursor ID. Does not change the graphic.
	// When setFromScript is set, type is a raw Nancy 10+ system cursor type
	// index resolved to its flat slot; hotspotVariant then picks the hotspot
	// (type*2 + 1) or idle (type*2) sprite of the pair.
	void setCursor(CursorType type, int16 itemID, bool setFromScript, bool hotspotVariant = true);
	void setCursorType(CursorType type, bool setFromScript = false, bool hotspotVariant = true);
	void setCursorItemID(int16 itemID);
	void showCursor(bool shouldShow);

	void warpCursor(const Common::Point &pos);

	// Change the cursor graphic. Should be called right before drawing to screen
	void applyCursor();

	const Common::Point &getCurrentCursorHotspot() { return _cursors[_curCursorID].hotspot;}
	const Common::Rect &getPrimaryVideoInactiveZone() { return _primaryVideoInactiveZone; }
	const Common::Point &getPrimaryVideoInitialPos() { return _primaryVideoInitialPos; }

	const CursorType _puzzleExitCursor;

private:
	void adjustCursorHotspot();

	// Resolve a CursorType + held-item pair to a Nancy 10+ cursor ID.
	uint resolveNancy10CursorID(CursorType type, int16 itemID, bool setFromScript, bool hotspotVariant);

	// Nancy13 rebuilt the cursor sheet (45 system types, new layout) and split
	// held-item cursors into _invCursorsSurface. System cursors use a dedicated
	// Nancy13 slot table.
	uint resolveNancy13CursorID(CursorType type, int16 itemID, bool setFromScript, bool hotspotVariant);

	struct Cursor {
		Common::Rect bounds;
		Common::Point hotspot;
	};

	// CURS data
	Common::Array<Cursor> _cursors;

	Common::Rect _primaryVideoInactiveZone;
	Common::Point _primaryVideoInitialPos;

	Graphics::ManagedSurface _invCursorsSurface;
	Graphics::ManagedSurface _uiCursorsSurface;	// Nancy13+

	Common::Point _warpedMousePos;
	CursorType _curCursorType;
	int16 _curItemID;
	uint _curCursorID;
	uint _lastCursorID;
	bool _hasItem;
	bool _isInitialized;
	int _numCursorTypes;
};

} // End of namespace Nancy

#endif // NANCY_CURSOR_H
