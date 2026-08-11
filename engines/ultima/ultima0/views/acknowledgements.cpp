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

#include "ultima/ultima0/views/acknowledgements.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

static const char *LINES[] = {
	"Akalabeth PC-Conversion",
	"",
	"",
	"Written by Richard Garriott",
	"",
	"",
	"",
	"PC-Conversion by",
	"Corey Roth",
	"with special assistance by",
	"Nathan Tucker",
	"",
	"",
	"",
	"",
	"Greets and Thanx go out to",
	"Jake Groshong",
	"Ronny McCrory",
	"Adrian Dilley",
	"Russell Kabir",
	"James Ashley",
	"Chris Harjo",
	"and everyone else out there",
	"who helped me out",
	"",
	"",
	"",
	"",
	"This game made possible by:",
	"",
	"Origin",
	"and",
	"JUiCEY Bird Productions"
};


bool Acknowledgements::msgFocus(const FocusMessage &msg) {
	// Set up the lines to display
	_ctr = -1;
	_lines.clear();
	for (const char *line : LINES)
		_lines.push(line);

	for (int i = 0; i < 25; ++i)
		_lines.push("");

	auto s = getSurface();
	s.clear();

	// Create a buffer for rendering new lines
	_pendingLine.create(s.w, Gfx::GLYPH_HEIGHT);

	return true;
}

void Acknowledgements::draw() {
	auto s = getSurface();

	// Shift up by one line
	s.blitFrom(s, Common::Rect(0, 1, s.w, s.h), Common::Point(0, 0));
	s.blitFrom(_pendingLine, Common::Rect(0, 0, _pendingLine.w, 1),
		Common::Point(0, s.h - 1));
	_pendingLine.blitFrom(_pendingLine, Common::Rect(0, 1, s.w, Gfx::GLYPH_HEIGHT),
		Common::Point(0, 0));
}

bool Acknowledgements::tick() {
	_ctr = (_ctr + 1) % Gfx::GLYPH_HEIGHT;
	if (_ctr == 0) {
		if (_lines.empty()) {
			showTitle();
		} else {
			Common::String line = _lines.pop();
			_pendingLine.writeString(Common::Point(20, 0), line, Graphics::kTextAlignCenter);
		}
	}

	redraw();
	return View::tick();
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
