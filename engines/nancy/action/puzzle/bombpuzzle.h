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

#ifndef NANCY_ACTION_BOMBPUZZLE_H
#define NANCY_ACTION_BOMBPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class BombPuzzle : public RenderActionRecord {
public:
	BombPuzzle() : RenderActionRecord(7) {}
	virtual ~BombPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "BombPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Common::String _imageName;
	Common::Array<Common::Rect> _wireSrcs;
	Common::Array<Common::Rect> _wireDests;
	Common::Array<Common::Rect> _digitSrcs;
	Common::Array<Common::Rect> _digitDests;
	Common::Rect _colonSrc;
	Common::Rect _colonDest;
	Common::Rect _displayBounds;
	Common::Array<byte> _solveOrder;
	SoundDescription _snipSound;
	SoundDescription _noToolSound;
	uint16 _toolID = 0;
	SceneChangeWithFlag _solveSceneChange;
	SoundDescription _solveSound;
	SceneChangeWithFlag _failSceneChange;
	SoundDescription _failSound;

	Graphics::ManagedSurface _image;
	Common::Array<byte> _playerOrder;

	Time _timerTotalTime;
	Time _timerBlinkTime;
	Time _nextBlinkTime;
	bool _isBlinking = false;
	uint _lastDrawnTime = 0;

	bool _failed = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BOMBPUZZLE_H
