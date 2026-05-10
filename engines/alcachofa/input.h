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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ALCACHOFA_INPUT_H
#define ALCACHOFA_INPUT_H

#include "common/events.h"
#include "common/ptr.h"

namespace Alcachofa {

struct Task;
class Process;

class Input {
public:
	inline bool wasMouseLeftPressed() const { return _wasMouseLeftPressed; }
	inline bool wasMouseRightPressed() const { return _wasMouseRightPressed; }
	inline bool wasAnyMousePressed() const { return _wasMouseLeftPressed || _wasMouseRightPressed; }
	inline bool wasMouseLeftReleased() const { return _wasMouseLeftReleased; }
	inline bool wasMouseRightReleased() const { return _wasMouseRightReleased; }
	inline bool wasAnyMouseReleased() const { return _wasMouseLeftReleased || _wasMouseRightReleased; }
	inline bool isMouseLeftDown() const { return _isMouseLeftDown; }
	inline bool isMouseRightDown() const { return _isMouseRightDown; }
	inline bool isAnyMouseDown() const { return _isMouseLeftDown || _isMouseRightDown; }
	inline bool wasMenuKeyPressed() const { return _wasMenuKeyPressed; }
	inline bool wasInventoryKeyPressed() const { return _wasInventoryKeyPressed; }
	inline bool wasSubtitlesKeyPressed() const { return _wasSubtitlesKeyPressed; }
	inline Common::Point mousePos2D() const { return _mousePos2D; }
	inline Common::Point mousePos3D() const { return _mousePos3D; }
	const Input &debugInput() const { scumm_assert(_debugInput != nullptr); return *_debugInput; }

	void nextFrame();
	bool handleEvent(const Common::Event &event);
	void toggleDebugInput(bool debugMode); ///< Toggles input debug mode which blocks any input not retrieved with debugInput
	Task *waitForInput(Process &process);

private:
	void updateMousePos3D();

	bool
		_wasMouseLeftPressed = false,
		_wasMouseRightPressed = false,
		_wasMouseLeftReleased = false,
		_wasMouseRightReleased = false,
		_isMouseLeftDown = false,
		_isMouseRightDown = false,
		_wasMenuKeyPressed = false,
		_wasInventoryKeyPressed = false,
		_wasSubtitlesKeyPressed = false;
	Common::Point
		_mousePos2D,
		_mousePos3D;
	Common::ScopedPtr<Input> _debugInput;
};

}

#endif // ALCACHOFA_INPUT_H
