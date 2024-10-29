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

#ifndef DGDS_RESOURCE_H
#define DGDS_RESOURCE_H

#include "common/file.h"
#include "common/hashmap.h"
#include "common/platform.h"
#include "common/stream.h"

namespace Dgds {

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
	bool hasResource(Common::String name) const;

	const ResourceList &getResources() const { return _resources; }

private:
	Common::File _volumes[MAX_VOLUMES];
	ResourceList _resources;
};

class DgdsChunkReader {
public:
	DgdsChunkReader(Common::SeekableReadStream *stream);
	~DgdsChunkReader();

	bool isSection(const Common::String &section) const;
	bool isSection(DGDS_ID section) const;

	bool readNextHeader(DGDS_EX ex, const Common::String &filename);

	/// Get a stream of the (decompressed if appropriate) content.
	/// This stream is owned by this object and invalidated when
	/// another header or content block is read. Do not delete it.
	bool readContent(Decompressor *decompressor);

	/// Don't bother reading the current chunk, just move to the
	/// next one. This is the alternative to readContent().
	void skipContent();

	/// Duplicate the buffer in the current content stream so
	/// that it can be retained in another object.
	Common::SeekableReadStream *makeMemoryStream();

	Common::SeekableReadStream *getContent() { return _contentStream; }

	bool isContainer() const { return _container; }
	DGDS_ID getId() const { return _id; }
	const char *getIdStr() const { return _idStr; }
	uint32 getSize() const { return _size; }

	DGDS_EX _ex;

private:
	uint32 _size;
	uint64 _startPos;
	char _idStr[DGDS_TYPENAME_MAX + 1];
	DGDS_ID _id;
	bool _container;
	Common::SeekableReadStream *_contentStream;
	Common::SeekableReadStream *_sourceStream;

	bool isPacked() const;
	Common::SeekableReadStream *decodeStream(Decompressor *decompressor);
	Common::SeekableReadStream *readStream();
};

} // End of namespace Dgds

#endif // DGDS_RESOURCE_H
