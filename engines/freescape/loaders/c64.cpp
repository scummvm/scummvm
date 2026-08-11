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

#include "freescape/freescape.h"

namespace Freescape {

byte *FreescapeEngine::decompressC64RLE(byte *buffer, int *size, byte marker) {
	Common::MemoryReadWriteStream *tmp = new Common::MemoryReadWriteStream(DisposeAfterUse::NO);
	// Format is: [ Byte, Marker, Length ] or [ Byte ]
	for (int i = 0; i < *size - 1; ) {
		if (buffer[i] == marker && i > 0) {
			int length = buffer[i + 1];
			byte value = buffer[i - 1];
			if (length == 0)
				tmp->writeByte(value);

			for (int j = 0; j < length; j++) {
				tmp->writeByte(value);
			}
			i += 2;
		} else {
			tmp->writeByte(buffer[i]);
			i += 1;
		}
	}
	*size = tmp->size();
	byte *data = tmp->getData();
	delete tmp;
	return data;
}

} // namespace Freescape
