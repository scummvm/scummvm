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

#include "sherlock/tattoo/widget_text.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetText::WidgetText(SherlockEngine *vm) : WidgetBase(vm) {
}

void WidgetText::load(const Common::String &str) {
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::StringArray lines;

	// If bounds for a window have not yet been calculated, figure them out
	if (_surface.empty()) {
		int width = SHERLOCK_SCREEN_WIDTH / 3;
		int height;

		for (;;) {
			_remainingText = splitLines(str, lines, width - _surface.widestChar() * 2, 100);
			height = (screen.fontHeight() + 1) * lines.size() + 9;

			if ((width - _surface.widestChar() * 2 > height * 3 / 2) || (width - _surface.widestChar() * 2
					> SHERLOCK_SCREEN_WIDTH * 3 / 4))
				break;

			width += (width / 4);
		}

		// See if it's only a single line long
		if (height == _surface.fontHeight() + 10) {
			width = _surface.widestChar() * 2 + 6;
			
			const char *strP = str.c_str();
			while (*strP && (*strP < talk._opcodes[OP_SWITCH_SPEAKER] || *strP == talk._opcodes[OP_NULL]))
				width += _surface.charWidth(*strP++);
		}

		_bounds.setWidth(width);
		_bounds.setHeight(height);
		_bounds.translate(ui._lookPos.x - width / 2, ui._lookPos.y - height / 2);
		checkMenuPosition();
	} else {
		// Split up the string into lines in preparation for drawing
		_remainingText = splitLines(str, lines, _bounds.width() - _surface.widestChar() * 2, 
			(_bounds.height() - _surface.fontHeight() / 2) / (_surface.fontHeight() + 1));
	}

	// Allocate a surface for the window
	_surface.create(_bounds.width(), _bounds.height());

	// Form the background for the new window
	_surface.fillRect(Common::Rect(0, 0, _surface.w(), _surface.h()), TRANSPARENCY);
	makeInfoArea();

	int yp = 5;
	for (int lineNum = 0; yp < (_bounds.height() - _surface.fontHeight() / 2); ++lineNum) {
		_surface.writeString(lines[lineNum], Common::Point(_surface.widestChar(), yp), INFO_TOP);
		yp += _surface.fontHeight() + 1;
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
