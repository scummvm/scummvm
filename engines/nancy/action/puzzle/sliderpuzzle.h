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

#ifndef NANCY_ACTION_SLIDERPUZZLE_H
#define NANCY_ACTION_SLIDERPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct SPUZ;
struct SliderPuzzleData;

namespace Action {

class SliderPuzzle: public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kWaitForSound };
	SliderPuzzle() : RenderActionRecord(7) {}
	virtual ~SliderPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	SliderPuzzleData *_puzzleState = nullptr;

	Common::Path _imageName;
	uint16 _width = 0;
	uint16 _height = 0;
	Common::Array<Common::Array<Common::Rect>> _srcRects;
	Common::Array<Common::Array<Common::Rect>> _destRects;
	Common::Array<Common::Array<int16>> _startTileOrder;
	Common::Array<Common::Array<int16>> _correctTileOrder;
	SoundDescription _clickSound;
	SceneChangeWithFlag _solveExitScene;
	SoundDescription _solveSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	bool _retainState = true;

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;

protected:
	Common::String getRecordTypeName() const override { return "SliderPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawTile(int tileID, uint posX, uint posY);
	void undrawTile(uint posX, uint posY);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SLIDERPUZZLE_H
