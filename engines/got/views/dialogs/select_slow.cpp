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

#include "got/views/dialogs/select_slow.h"
#include "got/got.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = { "On  (slow computer)", "Off (fast computer)", nullptr };

SelectSlow::SelectSlow() : SelectOption("SelectSlow", "Fast Mode", OPTIONS) {
}

void SelectSlow::selected() {
	switch (_selectedItem) {
	case 0:
		_G(slow_mode) = true;
		break;
	case 1:
		_G(slow_mode) = false;
		break;
	default:
		break;
	}

	// Next select scrolling
	addView("SelectScroll");
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
