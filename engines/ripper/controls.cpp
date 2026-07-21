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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/controls.h"

#include "common/debug.h"

#include "ripper/detection.h"

namespace Ripper {

uint UiControlRegistry::add(const Common::Rect &bounds, uint16 action) {
	_controls.push_back(UiControl(bounds, action));
	debugC(3, kDebugInput,
		"Ripper: registered UI control index=%u action=0x%04x bounds=%d,%d,%d,%d",
		_controls.size() - 1, action, bounds.left, bounds.top,
		bounds.right, bounds.bottom);
	return _controls.size() - 1;
}

int UiControlRegistry::findFirst(const Common::Point &point) const {
	for (uint i = 0; i < _controls.size(); ++i) {
		if (_controls[i].enabled && _controls[i].bounds.contains(point))
			return i;
	}
	return -1;
}

} // End of namespace Ripper
