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

#ifndef NANCY_ACTION_CUBEPUZZLE_H
#define NANCY_ACTION_CUBEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

// Similar idea to AssemblyPuzzle; the player is provided with broken pieces of
// a cube, and has to assemble it back together. However, the data is completely
// different to AssemblyPuzzle's, so we need separate implementations.
class CubePuzzle : public RenderActionRecord {
public:
	CubePuzzle() : RenderActionRecord(7) {}
	virtual ~CubePuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "CubePuzzle"; };
	bool isViewportRelative() const override { return true; }

	void rotateBase(int dir);

	Common::Path _imageName;

	Common::Rect _cwCursorDest;
	Common::Rect _ccwCursorDest;

	Common::Rect _placedDest;

	Common::Array<Common::Rect> _pieceSrcs; // Used only when not placed
	Common::Array<Common::Rect> _pieceDests; // Used only when not placed

	// 4 arrays with 9 rects each; every rect corresponds to a specific permutation of placed pieces:
	// - bottom piece only (start state)
	// - bottom & top piece
	// - front piece placed
	// - left piece placed
	// - back piece placed
	// - right piece placed
	// - left + back piece placed
	// - right + back piece placed
	// - left + right piece placed
	Common::Array<Common::Array<Common::Rect>> _placedSrcs;

	uint16 _startRotation = 0;

	SoundDescription _rotateSound;
	SoundDescription _pickUpSound;
	SoundDescription _placeDownSound;

	// Multiple solve scenes, one for each cube orientation
	Common::Array<uint> _solveSceneIDs;
	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;
	Misc::MouseFollowObject _curPiece;

	Common::Array<bool> _placedPieces;
	int _pickedUpPiece = -1;

	int _curRotation = 0;
	bool _completed = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CUBEPUZZLE_H
