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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "access/data.h"

namespace Access {

TimerList::TimerList() : Common::Array<TimerEntry>() {
	_timersSavedFlag = false;
}

void TimerList::saveTimers() {
	if (!_timersSavedFlag /* && !_flashbackFlag */) {
		_savedTimers = *this;
		_timersSavedFlag = true;
	}
}

void TimerList::restoreTimers() {
	if (_timersSavedFlag /* && !_flashbackFlag */) {
		clear();
		*static_cast<Common::Array<TimerEntry> *>(this) = _savedTimers;
		_timersSavedFlag = false;
	}
}

/*------------------------------------------------------------------------*/

Font::Font() {
}

void Font::load(const int *index, const byte *data) {
	int count = index[0];
	_v1 = index[1];
	_v2 = index[2];

	_chars.clear();
	for (int idx = 0; idx < count; ++idx)
		_chars.push_back(data + index[idx + 3]);
}

int Font::charWidth(char c) {
	return *_chars[c - ' '];
}

int Font::stringWidth(const Common::String &msg) {
	int total = 0;

	for (const char *c = msg.c_str(); *c != '\0'; ++c)
		total += charWidth(*c);

	return 0;
}

bool Font::getLine(Common::String &s, int maxWidth, Common::String &line, int &width) {
	width = 0;
	const char *src = s.c_str();
	char c;

	while ((c = *src) != '\0') {
		if (c == '\r') {
			// End of line, so return calculated line
			line = Common::String(s.c_str(), src - 1);
			s = Common::String(src + 1);
			return false;
		}

		++src;
		width += charWidth(c);
		if (width < maxWidth)
			continue;

		// Reached maximum allowed. Work backwards to find space at the
		// start of the current word as a point to split the line on
		while (*src != ' ' && src >= s.c_str()) {
			width -= charWidth(*src);
			--src;
		}
		if (src < s.c_str())
			error("Could not fit line");

		// Split the line around the space
		line = Common::String(s.c_str(), src - 1);
		s = Common::String(src + 1);
		return false;
	}

	// Return entire string
	line = s;
	s = Common::String();
	return true;
}

/*------------------------------------------------------------------------*/

FontManager::FontManager() {
	_printMaxX = 0;
}

} // End of namespace Access
