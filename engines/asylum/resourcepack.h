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

#ifndef ASYLUM_RESOURCEPACK_H_
#define ASYLUM_RESOURCEPACK_H_

#include "common/array.h"
#include "common/file.h"

namespace Asylum {

struct ResourceEntry {
	byte *data;
	uint32 size;
	uint32 offset;
};

class ResourcePack {
public:
	ResourcePack(const char *resourceFile);
	ResourcePack(int resourceIndex);
	~ResourcePack();

	ResourceEntry *getResource(uint16 index);
	ResourceEntry *getResourceFromId(uint32 resourceId) { return getResource(resourceId & 0xFFFF); }
	uint32 getResourceCount() { return _resources.size(); }
	
private:
	Common::Array <ResourceEntry> _resources;
	Common::File _packFile;

	void init(const char *resourceFile);
};

} // end of namespace Asylum

#endif
