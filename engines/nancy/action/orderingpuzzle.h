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

#ifndef NANCY_ACTION_ORDERINGPUZZLE_H
#define NANCY_ACTION_ORDERINGPUZZLE_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class OrderingPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	OrderingPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom, 7) {}
	virtual ~OrderingPuzzle() {}

	virtual void init() override;

	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;
	virtual void handleInput(NancyInput &input) override;
	virtual void onPause(bool pause) override;

	Common::String _imageName; // 0x00
	Common::Array<Common::Rect> _srcRects; // 0xC, 15
	Common::Array<Common::Rect> _destRects; // 0xFC, 15
	uint16 _sequenceLength = 0; // 0x1EC;
	Common::Array<byte> _correctSequence; // 0x1EE, 15 bytes
	Nancy::SoundDescription _clickSound; // 0x1FD, kNormal
	SceneChangeDescription _solveExitScene; // 0x21F
	EventFlagDescription _flagOnSolve; // 0x229
	uint16 _solveSoundDelay = 0; // 0x22C
	Nancy::SoundDescription _solveSound; // 0x22E
	SceneChangeDescription _exitScene; // 0x250
	EventFlagDescription _flagOnExit; // 0x25A
	Common::Rect _exitHotspot; // 0x25D

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;
	Common::Array<int16> _clickedSequence;
	Common::Array<bool> _drawnElements;
	Time _solveSoundPlayTime;

protected:
	virtual Common::String getRecordTypeName() const override { return "OrderingPuzzle"; }
	virtual bool isViewportRelative() const override { return true; }

	void drawElement(uint id);
	void undrawElement(uint id);
	void clearAllElements();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ORDERINGPUZZLE_H
