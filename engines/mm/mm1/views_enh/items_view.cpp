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

#include "mm/mm1/views_enh/items_view.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

ItemsView::ItemsView(const Common::String &name) : ScrollView(name) {
	setBounds(Common::Rect(0, 144, 234, 200));
}

bool ItemsView::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	_selectedItem = -1;
	return true;
}

void ItemsView::draw() {

}

bool ItemsView::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	return true;
}

bool ItemsView::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return true;
}

void ItemsView::timeout() {
	close();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
