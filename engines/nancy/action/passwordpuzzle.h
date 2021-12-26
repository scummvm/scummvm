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

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class PasswordPuzzle : public ActionRecord, public RenderObject {
public:
	enum SolveState { kNotSolved, kFailed, kSolved };
	PasswordPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom, 7) {}
	virtual ~PasswordPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;
	void onPause(bool pause) override;

	uint16 _fontID = 0; // 0x00
	Time _cursorBlinkTime; // 0x2
	Common::Rect _nameBounds; // 0x4
	Common::Rect _passwordBounds; // 0x14
	// _screenPosition 0x24
	Common::String _name; // 0x34, 20 bytes long
	Common::String _password; // 0x48, 20 bytes long
	SceneChangeDescription _solveExitScene; // 0x5A
	EventFlagDescription _flagOnSolve; // 0x66
	SoundDescription _solveSound; // 0x69
	SceneChangeDescription _failExitScene; // 0x8B
	EventFlagDescription _flagOnFail; // 0x95
	SoundDescription _failSound; // 0x98
	SceneChangeDescription _exitScene; // 0xBA
	EventFlagDescription _flagOnExit; // 0xC4
	Common::Rect _exitHotspot; // 0xC7

	Common::String _playerNameInput;
	Common::String _playerPasswordInput;
	Time _nextBlinkTime;
	bool _passwordFieldIsActive = false;
	bool _playerHasHitReturn = false;
	SolveState _solveState = kNotSolved;

protected:
	Common::String getRecordTypeName() const override { return "PasswordPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawText();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PASSWORDPUZZLE_H
