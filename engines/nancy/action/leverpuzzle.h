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

#ifndef NANCY_ACTION_LEVERPUZZLE_H
#define NANCY_ACTION_LEVERPUZZLE_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class LeverPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	LeverPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom, 7) {}
	virtual ~LeverPuzzle() {}

	virtual void init() override;

	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;
	virtual void handleInput(NancyInput &input) override;
	virtual void onPause(bool pause) override;

	Common::String _imageName; // 0x0
	Common::Array<Common::Array<Common::Rect>> _srcRects; // 0xA, 0xC0 bytes
	Common::Array<Common::Rect> _destRects; // 0xCA, 0x30 bytes
	Common::Array<byte> _correctSequence; // 0xFA, 3 bytes
	SoundDescription _moveSound; // 0x100
	SoundDescription _noMoveSound; // 0x122
	SceneChangeDescription _solveExitScene; // 0x144
	EventFlagDescription _flagOnSolve; // 0x14E
	uint16 _solveSoundDelay = 0; // 0x151
	SoundDescription _solveSound; // 0x153
	SceneChangeDescription _exitScene; // 0x175
	EventFlagDescription _flagOnExit; // 0x17F
	Common::Rect _exitHotspot; // 0x182

	Common::Array<byte> _playerSequence;
	Common::Array<bool> _leverDirection;
	Graphics::ManagedSurface _image;
	Time _solveSoundPlayTime;
	SolveState _solveState = kNotSolved;

protected:
	virtual Common::String getRecordTypeName() const override { return "LeverPuzzle"; }
	virtual bool isViewportRelative() const override { return true; }

	void drawLever(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_LEVERPUZZLE_H
