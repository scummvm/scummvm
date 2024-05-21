
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

#ifndef ALCACHOFA_CONSOLE_H
#define ALCACHOFA_CONSOLE_H

#include "gui/debugger.h"

namespace Alcachofa {

class Console : public GUI::Debugger {
public:
	Console();
	~Console() override;

	inline bool showInteractables() const { return _showInteractables; }
	inline bool showFloor() const { return _showFloor; }

	inline bool isAnyDebugDrawingOn() const {
		return
			_showInteractables ||
			_showFloor;
	}

private:
	bool _showInteractables = true;
	bool _showFloor = true;
};

} // End of namespace Alcachofa

#endif // ALCACHOFA_CONSOLE_H
