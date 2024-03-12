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

#ifndef NANCY_ACTION_SAFEDIALPUZZLE_H
#define NANCY_ACTION_SAFEDIALPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Handles the nancy3 safe puzzle with Chinese characters on the dial,
// as well as nancy4's sextant puzzle
class SafeDialPuzzle : public RenderActionRecord {
public:
	SafeDialPuzzle() : RenderActionRecord(7) {}
	virtual ~SafeDialPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	enum AnimState { kNone, kSpin, kSelect, kReset, kResetAnim };
	Common::String getRecordTypeName() const override { return "SafeDialPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawDialFrame(uint frame);
	void pushSequence(uint id);

	Common::Path _imageName1;
	Common::Path _imageName2;
	Common::Path _resetImageName;

	bool _enableWraparound = true;

	Common::Rect _dialDest;

	Common::Array<Common::Rect> _dialSrcs;

	Common::Rect _resetDest;
	Common::Rect _resetSrc;
	Common::Rect _arrowDest;
	Common::Rect _arrowSrc;

	Common::Array<Common::Rect> _resetDialSrcs;

	uint16 _resetTurns = 0;

	Common::Array<uint16> _correctSequence;

	Common::Rect _ccwHotspot;
	Common::Rect _cwHotspot;

	bool _useMoveArrows = false;

	SoundDescription _spinSound;
	SoundDescription _selectSound;
	SoundDescription _resetSound;

	SceneChangeWithFlag _solveScene;
	uint _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image1, _image2, _resetImage;

	uint _numInbetweens = 1;

	Common::Array<uint16> _playerSequence;
	bool _solved = false;
	AnimState _animState = kNone;
	uint32 _nextAnim = 0;
	uint16 _current = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SAFEDIALPUZZLE_H
