/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/debug-channels.h"

#include "engines/grim/debug.h"

namespace Grim {

void Debug::registerDebugChannels() {
	DebugMan.addDebugChannel(Info, "info", "");
	DebugMan.addDebugChannel(Warning, "warning", "");
	DebugMan.addDebugChannel(Error, "error", "");
	DebugMan.addDebugChannel(Engine, "engine", "");
	DebugMan.addDebugChannel(Lua, "lua", "");
	DebugMan.addDebugChannel(Bitmaps, "bitmaps", "");
	DebugMan.addDebugChannel(Models, "models", "");
	DebugMan.addDebugChannel(Actors, "actors", "");
	DebugMan.addDebugChannel(Costumes, "costumes", "");
	DebugMan.addDebugChannel(Chores, "chores", "");
	DebugMan.addDebugChannel(Fonts, "fonts", "");
	DebugMan.addDebugChannel(Keyframes, "keyframes", "");
	DebugMan.addDebugChannel(Movie, "movie", "");
	DebugMan.addDebugChannel(Imuse, "imuse", "");
	DebugMan.addDebugChannel(Scripts, "scripts", "");
	DebugMan.addDebugChannel(Sets, "sets", "");
	DebugMan.addDebugChannel(TextObjects, "textobjects", "");
	DebugMan.addDebugChannel(All, "all", "");
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

		::debug(buf.c_str());
	}
}

void Debug::warning(DebugChannel channel, const char *s, ...) {
	if (isChannelEnabled(channel | Debug::Warning)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::warning(buf.c_str());
	}
}

void Debug::error(DebugChannel channel, const char *s, ...) {
	if (isChannelEnabled(channel | Debug::Error)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::error(buf.c_str());
	}
}

void Debug::error(const char *s, ...) {
	if (isChannelEnabled(Debug::Error)) {
		va_list args;
		va_start(args, s);
		Common::String buf = Common::String::vformat(s, args);
		va_end(args);

		::error(buf.c_str());
	}
}

}
