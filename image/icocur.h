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

#ifndef IMAGE_ICOCUR_H
#define IMAGE_ICOCUR_H

#include "common/array.h"
#include "common/types.h"

namespace Common {

class SeekableReadStream;

} // End of namespace Common

namespace Graphics {

class Cursor;
struct Surface;

} // End of namespace Graphics

namespace Image {

class IcoCurDecoder {
public:
	enum Type {
		kTypeInvalid,

		kTypeICO,
		kTypeCUR,
	};

	struct Item {
		struct IconData {
			uint16 numPlanes;
			uint16 bitsPerPixel;
		};

		struct CursorData {
			uint16 hotspotX;
			uint16 hotspotY;
		};

		union DataUnion {
			IconData ico;
			CursorData cur;
		};

		uint16 width;
		uint16 height;
		uint8 numColors;	// May be 0
		DataUnion data;
		uint32 dataSize;
		uint32 dataOffset;
	};

	IcoCurDecoder();
	~IcoCurDecoder();

	bool open(Common::SeekableReadStream &stream, DisposeAfterUse::Flag = DisposeAfterUse::NO);
	void close();

	Type getType() const;
	uint numItems() const;
	const Item &getItem(uint itemIndex) const;

	/**
	 * Loads an item from the directory as a cursor.
	 *
	 * @param itemIndex The index of the item in the directory.
	 * @return Loaded cursor.
	 */
	Graphics::Cursor *loadItemAsCursor(uint itemIndex) const;

private:
	bool load();

	Type _type;
	Common::Array<Item> _items;

	Common::SeekableReadStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;
};

} // End of namespace Image

#endif
