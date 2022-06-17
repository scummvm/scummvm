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

#ifndef CHEWY_MOUSE_H
#define CHEWY_MOUSE_H

#include "common/keyboard.h"
#include "common/rect.h"

namespace Chewy {

#define MOUSE_LEFT 255
#define MOUSE_CENTER 254

class InputMgr {
public:
	InputMgr();
	~InputMgr();

	int16 findHotspot(const Common::Rect *hotspots);

	int16 getSwitchCode();

	int16 _hotkey = Common::KEYCODE_INVALID;
};

} // namespace Chewy

#endif
