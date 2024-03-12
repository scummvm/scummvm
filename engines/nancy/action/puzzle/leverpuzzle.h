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

#ifndef NANCY_ACTION_LEVERPUZZLE_H
#define NANCY_ACTION_LEVERPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class LeverPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	LeverPuzzle() : RenderActionRecord(7) {}
	virtual ~LeverPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::Path _imageName;
	Common::Array<Common::Array<Common::Rect>> _srcRects;
	Common::Array<Common::Rect> _destRects;
	Common::Array<byte> _correctSequence;
	SoundDescription _moveSound;
	SoundDescription _noMoveSound;
	SceneChangeWithFlag _solveExitScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Common::Array<byte> _playerSequence;
	Common::Array<bool> _leverDirection;
	Graphics::ManagedSurface _image;
	Time _solveSoundPlayTime;
	SolveState _solveState = kNotSolved;

protected:
	Common::String getRecordTypeName() const override { return "LeverPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawLever(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_LEVERPUZZLE_H
