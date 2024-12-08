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

#include "got/views/dialogs/select_option.h"
#include "got/metaengine.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

const char *ON_OFF[] = { "On", "Off", nullptr };
const char *YES_NO[] = { "Yes", "No", nullptr };

SelectOption::SelectOption(const Common::String &name, const char *title,
		const char *options[]) :
		Dialog(name), _title(title) {
	// Load the options list into the string array
	for (const char **option = options; *option; ++option)
		_options.push_back(*option);

	// Calculate the bounds for the dialog
	int w, h, x1, y1, x2, y2;
	w = strlen(title);
	if (w & 1)
		w++;
	w = (w * 8) + 32;

	h = (_options.size() * 16) + 32;
	x1 = (320 - w) / 2;
	x2 = (x1 + w);
	y1 = (192 - h) / 2;
	y2 = (y1 + h);

	_bounds = Common::Rect(x1 - 16, y1 - 16, x2 + 16, y2 + 16);
	_bounds.setBorderSize(16);
}

void SelectOption::draw() {
	// Clear the inner content first
	GfxSurface s = getSurface(true);
	s.clear(215);

	// Draw four corners
	s = getSurface();
	s.blitFrom(_G(bgPics)[192], Common::Point(0, 0));
	s.blitFrom(_G(bgPics)[193], Common::Point(_bounds.width() - 16, 0));
	s.blitFrom(_G(bgPics)[194], Common::Point(0, _bounds.height() - 16));
	s.blitFrom(_G(bgPics)[195], Common::Point(_bounds.width() - 16, _bounds.height() - 16));

	// Draw top/bottom horizontal lines
	for (int x = 16; x < _bounds.width() - 16; x += 16) {
		s.blitFrom(_G(bgPics)[196], Common::Point(x, 0));
		s.blitFrom(_G(bgPics)[197], Common::Point(x, _bounds.height() - 16));
	}

	// Draw left/right vertical lines
	for (int y = 16; y < _bounds.height() - 16; y += 16) {
		s.blitFrom(_G(bgPics)[198], Common::Point(0, y));
		s.blitFrom(_G(bgPics)[199], Common::Point(_bounds.width() - 16, y));
	}

	// Write the title
	s = getSurface(true);
	int titleStart = (s.w - _title.size() * 8) / 2;
	s.print(Common::Point(titleStart, 4), _title, 54);

	// Write the options
	for (uint i = 0; i < _options.size(); ++i)
		s.print(Common::Point(32, 28 + i * 16), _options[i], 14);

	// Draw selection pointer
	if (_smackCtr > 0) {
		// Selecting an item
		int xp = 8 + 2 * (_smackCtr < 3 ? (_smackCtr + 1) : (6 - _smackCtr));
		s.blitFrom(_G(hampic)[0], Common::Point(xp, 24 + (_selectedItem * 16)));
	} else {
		// Normal animated cursor
		s.blitFrom(_G(hampic)[_hammerFrame],
			Common::Point(8, 24 + (_selectedItem * 16)));
	}
}

bool SelectOption::msgAction(const ActionMessage &msg) {
	// Don't allow further actions if selection is in progress
	if (_smackCtr != 0)
		return true;

	switch (msg._action) {
	case KEYBIND_UP:
		play_sound(WOOP,1);
		if (--_selectedItem < 0)
			_selectedItem = (int)_options.size() - 1;
		break;

	case KEYBIND_DOWN:
		play_sound(WOOP, 1);
		if (++_selectedItem >= (int)_options.size())
			_selectedItem = 0;
		break;

	case KEYBIND_SELECT:
	case KEYBIND_FIRE:
	case KEYBIND_MAGIC:
		_smackCtr = 1;
		break;

	case KEYBIND_ESCAPE:
		closed();

	default:
		break;
	}

	return true;
}

bool SelectOption::tick() {
	if (++_hammerFrame == 4)
		_hammerFrame = 0;

	// Handle animation when an item is selected
	if (_smackCtr != 0) {
		++_smackCtr;
		if (_smackCtr == 3)
			play_sound(CLANG, 1);
		if (_smackCtr == 6) {
			_smackCtr = 0;
			selected();
		}	
	}

	redraw();
	return true;
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
