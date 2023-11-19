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

#ifndef NANCY_ACTION_OVERRIDELOCKPUZZLE_H
#define NANCY_ACTION_OVERRIDELOCKPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class OverrideLockPuzzle : public RenderActionRecord {
public:
	static const byte kButtonsStayDown 	= 1;
	static const byte kButtonsPopUp 	= 2;

	static const byte kLightsCircular 	= 3;
	static const byte kLightsRandom 	= 4;

	OverrideLockPuzzle() : RenderActionRecord(7) {}
	virtual ~OverrideLockPuzzle() {}

	void init() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "OverrideLockPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawButton(uint buttonID, bool clear);
	void drawLights();

	enum SolveState { kNotSolved, kSolved };

	Common::String _imageName;

	byte _popButtons = kButtonsStayDown;
	byte _randomizeLights = kLightsCircular;

	Common::Array<Common::Rect> _buttonSrcs;
	Common::Array<Common::Rect> _buttonDests;
	Common::Array<Common::Rect> _hotspots;
	Common::Array<Common::Rect> _lightSrcs;
	Common::Array<Common::Rect> _lightDests;

	SoundDescription _buttonSound;
	SoundDescription _wrongSound;

	Time _buttonPopTime;

	SceneChangeWithFlag _solveExitScene;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	Common::Array<byte> _buttonOrder;
	Common::Array<byte> _lightsOrder;
	Common::Array<byte> _playerOrder;

	Time _timeToPop;
	SolveState _solveState = kNotSolved;
	int8 _lastPushedButton = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_OVERRIDELOCKPUZZLE_H
