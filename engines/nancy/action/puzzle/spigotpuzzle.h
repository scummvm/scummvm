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

#ifndef NANCY_ACTION_SPIGOTPUZZLE_H
#define NANCY_ACTION_SPIGOTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// A puzzle in nancy7 where you pull spigots to input a password
class SpigotPuzzle : public RenderActionRecord {
public:
	SpigotPuzzle() : RenderActionRecord(7) {}
	virtual ~SpigotPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "SpigotPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;

	uint _numSpigots = 0;
	uint _numLetters = 0;
	uint _numInbetweens = 0;

	Common::Array<uint16> _startOrder;
	Common::Array<uint16> _numSpins;

	Common::Array<Common::Rect> _spigotDests;
	Common::Array<Common::Rect> _spigotHotspots;
	Common::Array<Common::Rect> _letterDests;
	Common::Array<Common::Rect> _digitDests;
	Common::Array<Common::Rect> _buttonDests;

	Common::Array<Common::Array<Common::Rect>> _spigotAnimSrcs;
	Common::Array<Common::Array<Common::Rect>> _digitSrcs;
	Common::Array<Common::Rect> _buttonSrcs;
	Common::Array<Common::Array<Common::Rect>> _letterSrcs;

	uint16 _letterTime = 0; // 1 unit = 200ms
	Common::Array<uint16> _correctOrder;

	SoundDescription _buttonSound;
	SoundDescription _letterSound;
	SoundDescription _spigotSound;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	Common::Array<uint16> _currentOrder;
	Common::Array<uint16> _currentAnimOrder;
	Common::Array<uint16> _numPulls;

	int _animatingSpigotID = -1;
	int _animatingLetterID = -1;
	int _pushedButtonID = -1;
	uint _frameID = 0;
	uint32 _nextAnimTime = 0;

	bool _solved = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SPIGOTPUZZLE_H
