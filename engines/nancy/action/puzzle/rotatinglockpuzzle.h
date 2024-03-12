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

#ifndef NANCY_ACTION_ROTATINGLOCKPUZZLE_H
#define NANCY_ACTION_ROTATINGLOCKPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class RotatingLockPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	RotatingLockPuzzle() : RenderActionRecord(7) {}
	virtual ~RotatingLockPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::Path _imageName;
	Common::Array<Common::Rect> _srcRects;
	Common::Array<Common::Rect> _destRects;
	Common::Array<Common::Rect> _upHotspots;
	Common::Array<Common::Rect> _downHotspots;
	Common::Array<byte> _correctSequence;
	Nancy::SoundDescription _clickSound;
	SceneChangeWithFlag _solveExitScene;
	uint16 _solveSoundDelay = 0;
	Nancy::SoundDescription _solveSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;
	Common::Array<byte> _currentSequence;
	Time _solveSoundPlayTime;

protected:
	Common::String getRecordTypeName() const override { return "RotatingLockPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawDial(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ROTATINGLOCKPUZZLE_H
