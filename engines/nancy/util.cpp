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
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

namespace Nancy {

void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect) {
	inRect.left = stream.readSint32LE();
	inRect.top = stream.readSint32LE();
	inRect.right = stream.readSint32LE();
	inRect.bottom = stream.readSint32LE();

	// TVD's rects are non-inclusive
	if (g_nancy->getGameType() > kGameTypeVampire) {
		++inRect.right;
		++inRect.bottom;
	}
}

// Reads an 8-character filename from a 10-character source
void readFilename(Common::SeekableReadStream &stream, Common::String &inString) {
	char buf[10];
	stream.read(buf, 10);
	buf[9] = '\0';
	inString = buf;
}

} // End of namespace Nancy
