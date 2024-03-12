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

#ifndef NANCY_ACTION_TWODIALPUZZLE_H
#define NANCY_ACTION_TWODIALPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Puzzle with two circular dials overlaid on top of each other. Each dial has one correct
// position, and can only be rotated in one direction.
class TwoDialPuzzle : public RenderActionRecord {
public:
	TwoDialPuzzle() : RenderActionRecord(7) {}
	virtual ~TwoDialPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "TwoDialPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;

	bool _isClockwise[2] = { false, false };
	uint16 _startPositions[2] = { 0, 0 };

	Common::Rect _hotspots[2];
	Common::Rect _dests[2];
	Common::Array<Common::Rect> _srcs[2];

	uint16 _correctPositions[2] = { 0, 0 };

	SoundDescription _rotateSounds[2];

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	int16 _currentPositions[2] = { 0, 0 };

	bool _isSolved = false;
	uint32 _solveSoundDelayTime = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TWODIALPUZZLE_H
