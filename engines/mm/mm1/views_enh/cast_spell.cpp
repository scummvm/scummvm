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

#include "mm/mm1/views_enh/cast_spell.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

CastSpell::CastSpell() : ScrollView("CastSpell") {
	_bounds = Common::Rect(234, 0, 320, 144);

	addButton(&g_globals->_confirmIcons,
		Common::Point(14, 10), 0,
		Common::KeyState(Common::KEYCODE_y, 'y'));
	addButton(&g_globals->_confirmIcons,
		Common::Point(40, 10), 2,
		Common::KeyState(Common::KEYCODE_n, 'n'));
}

void CastSpell::draw() {
	ScrollView::draw();
}

bool CastSpell::msgFocus(const FocusMessage &msg) {
	return true;
}

bool CastSpell::msgKeypress(const KeypressMessage &msg) {
	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
