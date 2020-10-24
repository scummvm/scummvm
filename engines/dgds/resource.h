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

#ifndef DGDS_RESOURCE_H
#define DGDS_RESOURCE_H

#include "common/file.h"
#include "common/hashmap.h"
#include "common/platform.h"

namespace Common {
class SeekableReadStream;
}

namespace Dgds {

class DgdsParser;
class Decompressor;

typedef uint32 DGDS_ID;
typedef uint32 DGDS_EX;
#define DGDS_TYPENAME_MAX 4

struct Resource {
	byte volume;
	uint32 pos;
	uint32 size;
	uint32 checksum;
};

typedef Common::HashMap<Common::String, Resource> ResourceList;

#define MAX_VOLUMES 10

class ResourceManager {
public:
	ResourceManager();
	virtual ~ResourceManager();

	Common::SeekableReadStream *getResource(Common::String name, bool ignorePatches = false);
	Resource getResourceInfo(Common::String name);
	ResourceList _resources;

private:
	Common::File _volumes[MAX_VOLUMES];
};

class DgdsChunk {
public:
	bool isSection(const Common::String &section);
	bool isSection(DGDS_ID section);
	bool isPacked(DGDS_EX ex);

	bool readHeader(DgdsParser &ctx);
	Common::SeekableReadStream *decodeStream(DgdsParser &ctx, Decompressor *decompressor);
	Common::SeekableReadStream *readStream(DgdsParser &ctx);

	char _idStr[DGDS_TYPENAME_MAX + 1];
	DGDS_ID _id;
	uint32 _size;
	bool container;
	Common::SeekableReadStream *_stream;
};

bool isFlatfile(Common::Platform platform, DGDS_EX _ex);
//int32 dgdsHash(const char *s, byte *idx);
//uint32 lookupVolume(const char *rmfName, const char *filename, char *volname);
//Common::SeekableReadStream *createReadStream(const char *rmfName, const char *filename);

} // End of namespace Dgds

#endif // DGDS_RESOURCE_H
