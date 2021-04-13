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
 */

#ifndef NANCY_UTIL_H
#define NANCY_UTIL_H

#include "common/rect.h"
#include "common/stream.h"

namespace Nancy {

inline void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect) {
	inRect.left = stream.readSint32LE();
	inRect.top = stream.readSint32LE();
	inRect.right = stream.readSint32LE() + 1;
	inRect.bottom = stream.readSint32LE() + 1;
}

// Reads an 8-character filename from a 10-character source
inline void readFilename(Common::SeekableReadStream &stream, Common::String &inString) {
	char buf[10];
	stream.read(buf, 10);
	buf[9] = '\0';
	inString = buf;
}

} // End of namespace Nancy

#endif // NANCY_UTIL_H
