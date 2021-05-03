/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NANCY_ACTION_ROTATINGLOCKPUZZLE_H
#define NANCY_ACTION_ROTATINGLOCKPUZZLE_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class RotatingLockPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	RotatingLockPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom, 7) {}
	virtual ~RotatingLockPuzzle() {}

	virtual void init() override;

	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;
	virtual void handleInput(NancyInput &input) override;
	virtual void onPause(bool pause) override;

	Common::String _imageName; // 0x00
	// 0xA numDials
	Common::Array<Common::Rect> _srcRects; // 0xC, 10
	Common::Array<Common::Rect> _destRects; // 0xAC, 8
	Common::Array<Common::Rect> _upHotspots; // 0x12C, 8
	Common::Array<Common::Rect> _downHotspots; // 0x1AC, 8
	Common::Array<byte> _correctSequence; // 0x22C
	Nancy::SoundDescription _clickSound; // 0x234, kNormal
	SceneChangeDescription _solveExitScene; // 0x256
	EventFlagDescription _flagOnSolve; // 0x260
	uint16 _solveSoundDelay = 0; // 0x263
	Nancy::SoundDescription _solveSound; // 0x265
	SceneChangeDescription _exitScene; // 0x287
	EventFlagDescription _flagOnExit; // 0x291
	Common::Rect _exitHotspot; // 0x294

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;
	Common::Array<byte> _currentSequence;
	Time _solveSoundPlayTime;


protected:
	virtual Common::String getRecordTypeName() const override { return "RotatingLockPuzzle"; }
	virtual bool isViewportRelative() const override { return true; }

	void drawDial(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ROTATINGLOCKPUZZLE_H
