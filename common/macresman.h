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
 * $URL$
 * $Id$
 *
 */

#include "common/array.h"
#include "common/file.h"

#ifndef COMMON_MACRESMAN_H
#define COMMON_MACRESMAN_H

namespace Common {

typedef Common::Array<int16> MacResIDArray;

/**
 * Class for reading Mac Binary files.
 * Is able to read dumped resource forks too.
 */
class MacResManager {

public:
	MacResManager(Common::String fileName);
	~MacResManager();

	/**
	 * Read resource from the Mac Binary file
	 * @param typeID FourCC with type ID
	 * @param resID Resource ID to fetch
	 * @param size Pointer to int where loaded data size will be stored
	 * @return Pointer to memory with loaded resource. Malloc()'ed
	 */
	byte *getResource(const char *typeID, int16 resID, int *size);

	char *getResName(const char *typeID, int16 resID);
	/**
	 * Convert cursor from Mac format to format suitable for feeding to CursorMan
	 * @param data Pointer to the cursor data
	 * @param datasize Size of the cursor data
	 * @param cursor Pointer to memory where result cursor will be stored. The memory
	 *               block will be malloc()'ed
	 * @param w Pointer to int where the cursor width will be stored
	 * @param h Pointer to int where the cursor height will be stored
	 * @param hotspot_x Storage for cursor hotspot X coordinate
	 * @param hotspot_Y Storage for cursor hotspot Y coordinate
	 * @param keycolor Pointer to int where the transpared color value will be stored
	 * @param colored If set to true then colored cursor will be returned (if any).
	 *                b/w version will be used otherwise
	 * @param palette Pointer to memory where the cursor palette will be stored.
	 *                The memory will be malloc()'ed
	 * @param palSize Pointer to integer where the palette size will be stored.
	 */
	void convertCursor(byte *data, int datasize, byte **cursor, int *w, int *h,
					  int *hotspot_x, int *hotspot_y, int *keycolor, bool colored, byte **palette, int *palSize);

	/**
	 * Return list of resource IDs with specified type ID
	 */
	MacResIDArray getResIDArray(const char *typeID);

	Common::String getFileName() { return _fileName; }

private:
	int extractResource(int id, byte **buf);
	bool init();
	void readMap();

	struct ResMap {
		int16 resAttr;
		int16 typeOffset;
		int16 nameOffset;
		int16 numTypes;
	};

	struct ResType {
		char  id[5];
		int16 items;
		int16 offset;
	};

	struct Resource {
		int16 id;
		int16 nameOffset;
		byte  attr;
		int32 dataOffset;
		char  *name;
	};

	typedef Resource *ResPtr;

private:
	int _resOffset;
	int32 _dataOffset;
	int32 _dataLength;
	int32 _mapOffset;
	int32 _mapLength;
	ResMap _resMap;
	ResType *_resTypes;
	ResPtr  *_resLists;

	Common::String _fileName;
	Common::File _resFile;
};

} // End of namespace Common

#endif
