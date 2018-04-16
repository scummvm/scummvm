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

#include "sherlock/tattoo/widget_tooltip.h"
#include "sherlock/tattoo/tattoo_map.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define MAX_TOOLTIP_WIDTH 150

void WidgetTooltipBase::draw() {
	Screen &screen = *_vm->_screen;

	// If there was a previously drawn frame in a different position that hasn't yet been erased, then erase it
	if (_oldBounds.width() > 0 && _oldBounds != _bounds)
		erase();

	if (_bounds.width() > 0 && !_surface.empty()) {
		restrictToScreen();

		// Blit the affected area to the screen
		screen.slamRect(_bounds);

		// Draw the widget directly onto the screen. Unlike other widgets, we don't draw to the back buffer,
		// since nothing should be drawing on top of tooltips, so there's no need to store in the back buffer
		screen.SHtransBlitFrom(_surface, Common::Point(_bounds.left - screen._currentScroll.x,
			_bounds.top - screen._currentScroll.y));

		// Store a copy of the drawn area for later erasing
		_oldBounds = _bounds;
	}
}

void WidgetTooltipBase::erase() {
	Screen &screen = *_vm->_screen;

	if (_oldBounds.width() > 0) {
		// Restore the affected area from the back buffer to the screen
		screen.slamRect(_oldBounds);

		// Reset the old bounds so it won't be erased again
		_oldBounds = Common::Rect(0, 0, 0, 0);
	}
}

/*----------------------------------------------------------------*/

WidgetTooltip::WidgetTooltip(SherlockEngine *vm) : WidgetTooltipBase (vm), _offsetY(0) {
}

void WidgetTooltip::setText(const Common::String &str) {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();
	bool reset = false;

	// Make sure that the description is present
	if (!str.empty()) {
		int width = _surface.stringWidth(str) + 2;
		int height = _surface.stringHeight(str) + 2;
		Common::String line1 = str, line2 = "";

		// See if we need to split it into two lines
		if (width > MAX_TOOLTIP_WIDTH) {
			// Go forward word by word to find out where to split the line
			const char *s = str.c_str();
			const char *space = nullptr;
			int dif = 10000;

			for (;;) {
				// Find end of next word
				s = strchr(s + 1, ' ');

				if (s == nullptr) {
					// Reached end of string
					if (space != nullptr) {
						line1 = Common::String(str.c_str(), space);
						line2 = Common::String(space + 1);
						height = _surface.stringHeight(line1) + _surface.stringHeight(line2) + 4;
					}
					break;
				}

				// Found space separating words, so see what width the string up to now is
				Common::String tempLine1 = Common::String(str.c_str(), s);
				Common::String tempLine2 = Common::String(s + 1);
				int width1 = _surface.stringWidth(tempLine1);
				int width2 = _surface.stringWidth(tempLine2);

				// See if we've found a split point that results in a less overall width
				if (ABS(width1 - width2) < dif) {
					// Found a better split point
					dif = ABS(width1 - width2);
					space = s;
					line1 = tempLine1;
					line2 = tempLine2;
				}
			}
		} else {
			// No line split needed
			height = _surface.stringHeight(str) + 2;
		}

		// Reallocate the text surface with the new size
		_surface.create(width, height);
		_surface.clear(TRANSPARENCY);

		if (line2.empty()) {
			// Only a single line
			_surface.writeFancyString(str, Common::Point(0, 0), BLACK, INFO_TOP);
		} else {
			// Two lines to display
			int xp, yp;
			xp = (width - _surface.stringWidth(line1) - 2) / 2;
			_surface.writeFancyString(line1, Common::Point(xp, 0), BLACK, INFO_TOP);

			xp = (width - _surface.stringWidth(line2) - 2) / 2;
			yp = _surface.stringHeight(line1) + 2;
			_surface.writeFancyString(line2, Common::Point(xp, yp), BLACK, INFO_TOP);
		}

		// Set the initial display position for the tooltip text
		int tagX = mousePos.x - width / 2;
		int tagY = mousePos.y - height - _offsetY;

		_bounds = Common::Rect(tagX, tagY, tagX + width, tagY + height);
	} else {
		reset = true;
	}

	if (reset && !_surface.empty()) {
		_surface.free();
	}
}

void WidgetTooltip::handleEvents() {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();

	// Set the new position for the tooltip
	int xp = mousePos.x - _bounds.width() / 2;
	int yp = mousePos.y - _bounds.height() - _offsetY;

	_bounds.moveTo(xp, yp);
}

/*----------------------------------------------------------------*/

void WidgetSceneTooltip::handleEvents() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	// See if thay are pointing at a different object and we need to regenerate the tooltip text
	if (ui._bgFound != ui._oldBgFound || (ui._bgFound != -1 && _surface.empty()) ||
			ui._arrowZone != ui._oldArrowZone || (ui._arrowZone != -1 && _surface.empty())) {
		// See if there is a new object to display text for
		if ((ui._bgFound != -1 && (ui._bgFound != ui._oldBgFound || (ui._bgFound != -1 && _surface.empty()))) ||
				(ui._arrowZone != -1 && (ui._arrowZone != ui._oldArrowZone || (ui._arrowZone != -1 && _surface.empty())))) {
			Common::String str;
			if (ui._bgFound != -1) {
				// Clear the Arrow Zone fields so it won't think we're displaying an Arrow Zone cursor
				if (scene._currentScene != OVERHEAD_MAP2)
					ui._arrowZone = ui._oldArrowZone = -1;

				// Get the description string
				str = (ui._bgFound < 1000) ? scene._bgShapes[ui._bgFound]._description :
					people[ui._bgFound - 1000]._description;

				// WORKAORUND: On the train ride to Cambridge, don't show any tooltips
				if (scene._currentScene == TRAIN_RIDE)
					str = "";
			} else {
				// Get the exit zone description
				str = scene._exits[ui._arrowZone]._dest;
			}

			setText(str.hasPrefix(" ") ? Common::String() : str);
		} else if ((ui._bgFound == -1 && ui._oldBgFound != -1) || (ui._arrowZone == -1 && ui._oldArrowZone != -1)) {
			setText("");
		}

		ui._oldBgFound = ui._bgFound;
	} else {
		// Set the new position for the tooltip
		int tagX = CLIP(mousePos.x - _bounds.width() / 2, 0, SHERLOCK_SCREEN_WIDTH - _bounds.width());
		int tagY = MAX(mousePos.y - _bounds.height() - _offsetY, 0);
		_bounds.moveTo(tagX, tagY);
	}

	ui._oldArrowZone = ui._arrowZone;

	WidgetTooltip::handleEvents();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
