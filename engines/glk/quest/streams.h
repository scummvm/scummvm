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

#ifndef GLK_QUEST_STREAMS
#define GLK_QUEST_STREAMS

#include "glk/quest/string.h"
#include "common/memstream.h"
#include "common/stream.h"

namespace Glk {
namespace Quest {

/**
 * Write stream wrapper around ScummVM debug calls. Can only handle text being written
 */
class ConsoleStream : public Common::WriteStream {
public:
	uint32 write(const void *dataPtr, uint32 dataSize) override;
	int32 pos() const override { return 0; }
};

class ostringstream : public Common::MemoryWriteStreamDynamic {
public:
	ostringstream() : Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES) {}

	String str();
};
class StringStream : public ostringstream {
public:
	StringStream &operator>>(String &rhs) {
		rhs = str();
		return *this;
	}
};

/**
 * Simple wrapper for managing streams initialization
 */
class Streams {
public:
	/**
	 * Initialization
	 */
	static void initialize();

	/**
	 * Deinitialization
	 */
	static void deinitialize();
};

extern ConsoleStream *g_cerr;
extern const char endl;

#define cerr (*g_cerr)

Common::WriteStream &operator<<(Common::WriteStream &, const String &);
Common::WriteStream &operator<<(Common::WriteStream &, const char *);
Common::WriteStream &operator<<(Common::WriteStream &, char);
Common::WriteStream &operator<<(Common::WriteStream &, int);
Common::WriteStream &operator<<(Common::WriteStream &, uint);
Common::WriteStream &operator<<(Common::WriteStream &, size_t);

} // End of namespace Quest
} // End of namespace Glk

#endif
