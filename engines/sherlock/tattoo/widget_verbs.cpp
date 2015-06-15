/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/tattoo/widget_verbs.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetVerbs::WidgetVerbs(SherlockEngine *vm) : _vm(vm) {
	_selector = _oldSelector = -1;
}

void WidgetVerbs::execute() {
	checkTabbingKeys(_verbCommands.size());

	// Highlight verb display as necessary
	highlightVerbControls();

	// TODO
}

void WidgetVerbs::checkTabbingKeys(int numOptions) {

}

void WidgetVerbs::highlightVerbControls() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();

	// Get highlighted verb
	_selector = -1;
	Common::Rect bounds = _bounds;
	bounds.grow(-3);
	if (bounds.contains(mousePos))
		_selector = (mousePos.y - bounds.top) / (screen.fontHeight() + 7);

	// See if a new verb is being pointed at
	if (_selector != _oldSelector) {
		// Redraw the verb list
		for (int idx = 0; idx < (int)_verbCommands.size(); ++idx) {
			byte color = (idx == _selector) ? COMMAND_HIGHLIGHTED : INFO_TOP;
			_surface.writeString(_verbCommands[idx], Common::Point((_bounds.width() - screen.stringWidth(_verbCommands[idx])) / 2,
				(screen.fontHeight() + 7) * idx + 5), color);
		}

		_oldSelector = _selector;
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
