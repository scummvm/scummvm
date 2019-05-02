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

#include "lilliput/stream.h"

namespace Lilliput {

ScriptStream::ScriptStream(byte *buf, int bufSize) : Common::MemoryReadStream(buf, bufSize) {
	_orgPtr = buf;
}

ScriptStream::~ScriptStream() {
}

void ScriptStream::writeUint16LE(int value, int relativePos) {
	int writePos = pos() + relativePos;
	assert((writePos >= 0) && (writePos + 2 < size()));

	Common::MemoryWriteStream tmpStream = Common::MemoryWriteStream(_orgPtr + writePos, size() - writePos);
	tmpStream.writeUint16LE(value);
}

} // End of namespace Lilliput
