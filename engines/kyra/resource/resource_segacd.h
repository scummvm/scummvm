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

#ifdef ENABLE_EOB

#ifndef KYRA_RESOURCE_SEGACD_H
#define KYRA_RESOURCE_SEGACD_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
	class SeekableReadStream;
	class SeekableReadStreamEndian;
}

namespace Kyra {

class Resource;
class SegaCDResource {
public:
	SegaCDResource(Resource *res);
	~SegaCDResource();

	bool loadContainer(const Common::String &filename, uint32 offset = 0, uint32 size = 0);
	void unloadContainer();

	Common::SeekableReadStreamEndian *resStreamEndian(int resID);
	Common::SeekableReadStream *resStream(int resID);

	uint8 *resData(int resID, uint32 *resLen = 0);

private:
	Resource *_res;

	struct TableEntry {
		TableEntry() : _offset(0), _len(0) {}
		uint32 _offset;
		uint32 _len;
	};

	TableEntry *_resTable;
	int _numResources;
	Common::SeekableReadStreamEndian *_str;
	Common::String _curFile;
	uint32 _curOffset;
	uint32 _curSize;
};

} // End of namespace Kyra

#endif
#endif // ENABLE_EOB
