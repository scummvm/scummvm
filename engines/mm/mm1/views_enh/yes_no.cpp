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

#include "mm/mm1/views_enh/yes_no.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

void YesNo::openYesNo() {
	_subviewVisible = true;
	redraw();
}

void YesNo::closeYesNo() {
	_subviewVisible = false;

	// Redraw the prior views to remove yes/no area
	g_events->redrawViews();
}

void YesNo::draw() {
	if (_subviewVisible)
		_subview.draw();

	ScrollView::draw();
}

bool YesNo::msgMouseDown(const MouseDownMessage &msg) {
	if (_subviewVisible && _subview.msgMouseDown(msg))
		return true;

	return ScrollView::msgMouseDown(msg);
}

bool YesNo::msgMouseUp(const MouseUpMessage &msg) {
	if (_subviewVisible && _subview.msgMouseUp(msg))
		return true;

	return ScrollView::msgMouseUp(msg);
}

/*------------------------------------------------------------------------*/

YesNoSubview::YesNoSubview() : ScrollView("YesNoSubview") {
	_bounds = Common::Rect(234, 144, 320, 200);

	addButton(&g_globals->_confirmIcons, Common::Point(0, 0), 0, Common::KEYCODE_y);
	addButton(&g_globals->_confirmIcons, Common::Point(25, 0), 2, Common::KEYCODE_n);
}

bool YesNoSubview::msgKeypress(const KeypressMessage &msg) {
	assert(g_events->focusedView() != this);
	return g_events->focusedView()->send(msg);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
