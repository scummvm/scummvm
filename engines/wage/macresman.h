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

#ifndef WAGE_MACRESMAN_H
#define WAGE_MACRESMAN_H

namespace Wage {

typedef Common::Array<int16> MacResIDArray;

class MacResManager {

public:
	MacResManager(Common::String fileName);
	~MacResManager();
	byte *getResource(const char *typeID, int16 resID, int *size);
	void convertCursor(byte *data, int datasize, byte **cursor, int *w, int *h,
					  int *hotspot_x, int *hotspot_y, int *keycolor, bool colored, byte **palette, int *palSize);

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
		byte  *name;
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

} // End of namespace Wage

#endif
