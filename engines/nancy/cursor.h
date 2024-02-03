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

#include "graphics/managed_surface.h"

namespace Nancy {

class NancyEngine;

class CursorManager {
	friend class NancyEngine;

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
		kNormalArrow,
		kHotspotArrow
	};

	CursorManager();

	void init(Common::SeekableReadStream *chunkStream);

	// Change the current cursor ID. Does not change the graphic
	void setCursor(CursorType type, int16 itemID);
	void setCursorType(CursorType type);
	void setCursorItemID(int16 itemID);

	void warpCursor(const Common::Point &pos);

	// Change the cursor graphic. Should be called right before drawing to screen
	void applyCursor();

	const Common::Point &getCurrentCursorHotspot() { return _cursors[_curCursorID].hotspot;}
	const Common::Rect &getPrimaryVideoInactiveZone() { return _primaryVideoInactiveZone; }
	const Common::Point &getPrimaryVideoInitialPos() { return _primaryVideoInitialPos; }

	const CursorType _puzzleExitCursor;

private:
	void showCursor(bool shouldShow);

	void adjustCursorHotspot();

	struct Cursor {
		Common::Rect bounds;
		Common::Point hotspot;
	};

	// CURS data
	Common::Array<Cursor> _cursors;

	Common::Rect _primaryVideoInactiveZone;
	Common::Point _primaryVideoInitialPos;

	Graphics::ManagedSurface _invCursorsSurface;

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
