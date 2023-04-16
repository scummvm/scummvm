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

#ifndef NANCY_ACTION_TOWERIPPEDLETTERPUZZLE_H
#define NANCY_ACTION_TOWERIPPEDLETTERPUZZLE_H

#include "engines/nancy/renderobject.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class TowerPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kWaitForSound };
	TowerPuzzle() : RenderObject(7), _heldRing(8) {}
	virtual ~TowerPuzzle() {}

	void init() override;
	void registerGraphics() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "TowerPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawRing(uint poleID, uint position, uint ringID, bool clear = false);

	Common::String _imageName;
	Common::Array<uint16> _numRingsByDifficulty;
	// 2 unknown
	Common::Array<Common::Rect> _droppedRingSrcs; // 6
	Common::Array<Common::Rect> _heldRingSrcs; // 6

	Common::Array<Common::Rect> _hotspots; // 3

	Common::Array<Common::Array<Common::Array<Common::Rect>>> _destRects; // [ringID][poleID][position]

	SoundDescription _takeSound;
	SoundDescription _dropSound;

	SceneChangeDescription _solveExitScene;
	SoundDescription _solveSound;
	FlagDescription _flagOnSolve;

	SceneChangeDescription _exitScene;
	FlagDescription _flagOnExit;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;
	RenderObject _heldRing;
	int8 _heldRingID = -1;
	int8 _heldRingPoleID = -1;
	SolveState _solveState = kNotSolved;
	TowerPuzzleState *_puzzleState;
	uint _numRings = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TOWERIPPEDLETTERPUZZLE_H
