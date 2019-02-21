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

#include "bladerunner/screen_effects.h"

#include "common/stream.h"

namespace BladeRunner {

ScreenEffects::ScreenEffects(BladeRunnerEngine *vm, int size) {
	_vm = vm;
	_dataSize = size;
	_data = new uint8[size];
	_entries.reserve(8);
}

ScreenEffects::~ScreenEffects() {
	delete[] _data;
}

void ScreenEffects::readVqa(Common::SeekableReadStream *stream) {
	uint8 *dataPtr = _data;
	int dataSize   = _dataSize;

	int entryCount = stream->readUint32LE();

	if (entryCount == 0) {
		return;
	}

	entryCount = MIN(entryCount, 7);
	_entries.resize(entryCount);

	for (Common::Array<Entry>::iterator entry = _entries.begin(); entry != _entries.end(); ++entry) {
		stream->read(&entry->palette, sizeof(Color256) * 16);

		entry->x      = stream->readUint16LE();
		entry->y      = stream->readUint16LE();
		entry->width  = stream->readUint16LE();
		entry->height = stream->readUint16LE();
		entry->z      = stream->readUint16LE();

		int entryDataSize = stream->readUint16LE();

		int pixelCount = entry->width * entry->height;

		if (pixelCount > dataSize) { // to big to fit
			entry->width = 0;
			entry->height = 0;
			entry->data = _data;
			continue;
			// there is a issue in the game code, because it's not skipping data of entry in this case
		}

		int pos = stream->pos();
		dataSize -= pixelCount;
		entry->data = dataPtr;
		do {
			uint8 count = stream->readByte();
			if (count & 0x80) { // repeat same data
				uint8 colors = stream->readByte();
				for (uint8 j = 0; j < (count & 0x7F) + 1; j++) {
					*(dataPtr++) = colors >> 4;  // upper 4 bit
					*(dataPtr++) = colors & 0xF; // lower 4 bit
					pixelCount -= 2;
				}
			} else { // copy data
				for (uint8 j = 0; j < count + 1; j++) {
					uint8 colors = stream->readByte();
					*(dataPtr++) = colors >> 4;  // upper 4 bit
					*(dataPtr++) = colors & 0xF; // lower 4 bit
					pixelCount -= 2;
				}
			}
		} while (pixelCount > 0);
		stream->seek(pos + entryDataSize, SEEK_SET);
	}
}

//TODO:
//bool ScreenEffects::isAffectingArea(int x, int y, int width, int height, int z) {
//	int xx = x >> 1;
//	int yy = y >> 1;
//	if (_entries.empty()) {
//		return false;
//	}
//
//	for(int i = 0; i < _entries.size(); i++) {
//		Entry &entry = _entries[i];
//		if (entry.z < z) {
//			if (entry.width < (width >> 1) + xx) {
//				if (entry.width + entry.x > xx) {
//					if (entry.height < (height >> 1) + yy) {
//						if(entry.height + entry.y > yy) {
//							return true;
//						}
//					}
//				}
//			}
//		}
//	}
//	return false;
//}

void ScreenEffects::getColor(Color256 *outColor, uint16 x, uint16 y, uint16 z) const {
	Color256 color = { 0, 0, 0 };
	for (Common::Array<const Entry>::iterator entry = _entries.begin(); entry != _entries.end(); ++entry) {
		uint16 x1 = (x / 2) - entry->x;
		uint16 y1 = (y / 2) - entry->y;
		if (x1 < entry->width && y1 < entry->height && z > entry->z) {
			int colorIndex = entry->data[y1 * entry->width + x1];
			Color256 entryColor = entry->palette[colorIndex];
			color.r += entryColor.r;
			color.g += entryColor.g;
			color.b += entryColor.b;
		}
	}
	*outColor = color;
}

} // End of namespace BladeRunner
