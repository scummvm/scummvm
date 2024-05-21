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

#ifndef INPUT_H
#define INPUT_H

#include "common/events.h"

namespace Alcachofa {

class Input {
public:
	inline bool wasMouseLeftPressed() const { return _wasMouseLeftPressed; }
	inline bool wasMouseRightPressed() const { return _wasMouseRightPressed; }
	inline bool isMouseLeftDown() const { return _isMouseLeftDown; }
	inline bool isMouseRightDown() const { return _isMouseRightDown; }
	inline const Common::Point &mousePos2D() const { return _mousePos2D; }
	inline const Common::Point &mousePos3D() const { return _mousePos3D; }

	void nextFrame();
	bool handleEvent(const Common::Event &event);

private:
	bool
		_wasMouseLeftPressed,
		_wasMouseRightPressed,
		_isMouseLeftDown,
		_isMouseRightDown;
	Common::Point
		_mousePos2D,
		_mousePos3D;
};

}

#endif // INPUT_H
