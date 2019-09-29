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

#include "glk/quest/streams.h"
#include "common/debug.h"
#include "common/str.h"

namespace Glk {
namespace Quest {

ConsoleStream *g_cerr;
const char endl = '\n';

void Streams::initialize() {
	g_cerr = new ConsoleStream();
}

void Streams::deinitialize() {
	delete g_cerr;
}

uint32 ConsoleStream::write(const void *dataPtr, uint32 dataSize) {
	if (gDebugLevel > 0) {
		Common::String s((const char *)dataPtr, (const char *)dataPtr + dataSize);
		debug("%s", s.c_str());
	}

	return dataSize;
}

/*--------------------------------------------------------------------------*/

String ostringstream::str() {
	return String((const char *)getData(), (const char *)getData() + size());
}

/*--------------------------------------------------------------------------*/

Common::WriteStream &operator<<(Common::WriteStream &ws, const String &s) {
	ws.writeString(s);
	return ws;
}

Common::WriteStream &operator<<(Common::WriteStream &ws, const char *s) {
	ws.write(s, strlen(s));
	return ws;
}

Common::WriteStream &operator<<(Common::WriteStream &ws, char c) {
	ws.writeByte(c);
	return ws;
}

Common::WriteStream &operator<<(Common::WriteStream &ws, int i) {
	Common::String s = Common::String::format("%d", i);
	ws.writeString(s);
	return ws;
}

Common::WriteStream &operator<<(Common::WriteStream &ws, uint i) {
	Common::String s = Common::String::format("%u", i);
	ws.writeString(s);
	return ws;
}

} // End of namespace Quest
} // End of namespace Glk
