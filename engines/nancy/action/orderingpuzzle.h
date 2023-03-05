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

#ifndef NANCY_ACTION_ORDERINGPUZZLE_H
#define NANCY_ACTION_ORDERINGPUZZLE_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class OrderingPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	OrderingPuzzle() : RenderObject(7) {}
	virtual ~OrderingPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;
	void onPause(bool pause) override;

	Common::String _imageName; // 0x00
	Common::Array<Common::Rect> _srcRects; // 0xC, 15
	Common::Array<Common::Rect> _destRects; // 0xFC, 15
	uint16 _sequenceLength = 0; // 0x1EC;
	Common::Array<byte> _correctSequence; // 0x1EE, 15 bytes
	Nancy::SoundDescription _clickSound; // 0x1FD, kNormal
	SceneChangeDescription _solveExitScene; // 0x21F
	FlagDescription _flagOnSolve; // 0x229
	uint16 _solveSoundDelay = 0; // 0x22C
	Nancy::SoundDescription _solveSound; // 0x22E
	SceneChangeDescription _exitScene; // 0x250
	FlagDescription _flagOnExit; // 0x25A
	Common::Rect _exitHotspot; // 0x25D

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;
	Common::Array<int16> _clickedSequence;
	Common::Array<bool> _drawnElements;
	Time _solveSoundPlayTime;

protected:
	Common::String getRecordTypeName() const override { return "OrderingPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawElement(uint id);
	void undrawElement(uint id);
	void clearAllElements();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ORDERINGPUZZLE_H
