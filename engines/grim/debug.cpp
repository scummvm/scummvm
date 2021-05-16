/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/debug-channels.h"

#include "engines/grim/debug.h"

namespace Grim {

void Debug::registerDebugChannels() {
}

bool Debug::isChannelEnabled(DebugChannel chan) {
	return DebugMan.isDebugChannelEnabled(chan);
}

void Debug::debug(DebugChannel channel, const char *s, ...) {
	if (isChannelEnabled(channel | Debug::Info)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::debug("%s", buf.c_str());
	}
}

void Debug::warning(DebugChannel channel, const char *s, ...) {
	if (isChannelEnabled(channel | Debug::Warning)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::warning("%s", buf.c_str());
	}
}

void Debug::error(DebugChannel channel, const char *s, ...) {
	if (isChannelEnabled(channel | Debug::Error)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::error("%s", buf.c_str());
	}
}

void Debug::error(const char *s, ...) {
	if (isChannelEnabled(Debug::Error)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::error("%s", buf.c_str());
	}
}

}
