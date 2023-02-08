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

#include "mm/mm1/views_enh/scroll_popup.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

ScrollPopup::ScrollPopup(const Common::String &name) :
		ScrollText(name) {
}

bool ScrollPopup::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

bool ScrollPopup::msgMouseUp(const MouseUpMessage &msg) {
	close();
	return true;
}

bool ScrollPopup::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}


} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
