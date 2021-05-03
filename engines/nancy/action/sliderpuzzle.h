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

#ifndef NANCY_ACTION_SLIDERPUZZLE_H
#define NANCY_ACTION_SLIDERPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Common {
class Serializer;
}

namespace Nancy {
namespace Action {

class SliderPuzzle: public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kWaitForSound };
	SliderPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom, 7) {}
	virtual ~SliderPuzzle() {}

	virtual void init() override;

	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;
	virtual void handleInput(NancyInput &input) override;
	virtual void onPause(bool pause) override;

	Common::String _imageName; // 0x00
	uint16 _width = 0; // 0xA
	uint16 _height = 0; // 0xC
	Common::Array<Common::Array<Common::Rect>> _srcRects; // 0x0E, size 0x240
	Common::Array<Common::Array<Common::Rect>> _destRects; // 0x24E, size 0x240
	Common::Array<Common::Array<int16>> _correctTileOrder; // 0x48E, size 0x48
	SoundDescription _clickSound; // 0x4D6
	SceneChangeDescription _solveExitScene; // 0x4F8
	EventFlagDescription _flagOnSolve; // 0x502
	SoundDescription _solveSound; // 0x505
	SceneChangeDescription _exitScene; // 0x527
	EventFlagDescription _flagOnExit; // 0x531
	Common::Rect _exitHotspot; // 0x534

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;

protected:
	virtual Common::String getRecordTypeName() const override { return "SliderPuzzle"; }
	virtual bool isViewportRelative() const override { return true; }

	void drawTile(int tileID, uint posX, uint posY);
	void undrawTile(uint posX, uint posY);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SLIDERPUZZLE_H
