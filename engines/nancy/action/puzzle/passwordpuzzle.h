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

#ifndef NANCY_ACTION_PASSWORDPUZZLE_H
#define NANCY_ACTION_PASSWORDPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class PasswordPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kFailed, kSolved };
	PasswordPuzzle() : RenderActionRecord(7) {}
	virtual ~PasswordPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void onPause(bool paused) override;
	void handleInput(NancyInput &input) override;

	uint16 _fontID = 0;
	uint16 _cursorBlinkTime = 500;
	Common::Rect _nameBounds;
	Common::Rect _passwordBounds;
	// _screenPosition 0x24
	Common::Array<Common::String> _names;
	Common::Array<Common::String> _passwords;
	SceneChangeWithFlag _solveExitScene;
	SoundDescription _solveSound;
	SceneChangeWithFlag _failExitScene;
	SoundDescription _failSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Common::String _playerNameInput;
	Common::String _playerPasswordInput;
	Time _nextBlinkTime;
	bool _passwordFieldIsActive = false;
	bool _playerHasHitReturn = false;
	SolveState _solveState = kNotSolved;

	uint _maxStringLength = 0;

protected:
	Common::String getRecordTypeName() const override { return "PasswordPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawText();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PASSWORDPUZZLE_H
