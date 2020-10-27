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

#include "glk/advsys/glk_interface.h"

namespace Glk {
namespace AdvSys {

bool GlkInterface::initialize() {
	_window = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	return _window != nullptr;
}

void GlkInterface::print(const Common::String &msg) {
	// Don't print out text if loading a savegame directly from the launcher, since we don't
	// want any of the intro text displayed by the startup code to show
	if (_saveSlot == -1)
		glk_put_string_stream(glk_window_get_stream(_window), msg.c_str());
}

void GlkInterface::print(const Common::U32String &msg) {
	// Don't print out text if loading a savegame directly from the launcher, since we don't
	// want any of the intro text displayed by the startup code to show
	if (_saveSlot == -1)
		glk_put_string_stream_uni(glk_window_get_stream(_window), msg.u32_str());
}

void GlkInterface::print(int number) {
	Common::String s = Common::String::format("%d", number);
	print(s);
}

Common::String GlkInterface::readLine() {
	event_t ev;
	char line[200];

	print(": ");

	if (!_pendingLine.empty()) {
		// The next input line has been manually provided, so return it
		print(_pendingLine);
		print("\n");

		Common::String l = _pendingLine;
		_pendingLine = "";
		return l;
	}

	glk_request_line_event(_window, line, 199, 0);

	do {
		glk_select(&ev);
		if (ev.type == evtype_Quit)
			return "";
		else if (ev.type == evtype_LineInput) {
			line[ev.val1] = '\0';
			return Common::String(line);
		}
	} while (!shouldQuit() && ev.type != evtype_Quit);

	return "";
}

} // End of namespace AdvSys
} // End of namespace Glk
