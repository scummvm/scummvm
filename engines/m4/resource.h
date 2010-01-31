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

#ifndef M4_RESOURCE_H
#define M4_RESOURCE_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/ptr.h"

/*
	TODO:
		- change array to HashMap if it turns out to be slow
*/

namespace M4 {

#define MAX_RESOURCES 128
#define kM4MaxFilenameSize 33

enum {
	kResourcePurge  = 1 << 1
};


class FileSystem {
public:
	struct HashHagEntry {
		char filename[kM4MaxFilenameSize];
		byte fileIndex;
		Common::File *hagFile;
	};

	struct HashFileEntry {
		char filename[kM4MaxFilenameSize];
		byte hagfile;
		uint32 offset, size;
	};

	FileSystem(const char *hashFilename);
	~FileSystem();

	Common::SeekableReadStream *loadFile(const char *resourceName, bool preloadFlag);
	static void changeExtension(char *destName, const char *sourceName, const char *extension);
	const HashFileEntry *getHashFileEntry(const char *filename);

private:
	typedef Common::HashMap<Common::String,HashFileEntry,Common::IgnoreCase_Hash,Common::IgnoreCase_EqualTo> FileHashMap;

	HashHagEntry _hagEntries[10]; // GLOBAL.HAG and SECTION1.HAG to SECTION9.HAG
	FileHashMap _fileEntries;
};

struct Resource {
	char name[64];
	Common::SeekableReadStream *stream;
	uint8 *buffer;
	uint8 flags;
};

class ResourceManager {
protected:
	typedef Common::List<Common::SharedPtr<Resource> > ResourceList;
	typedef ResourceList::iterator ResourceIterator;
	ResourceList _resources;
	MadsM4Engine *_vm;

	virtual Common::SeekableReadStream *loadResource(const char *resourceName, bool loadFlag) = 0;
public:
	ResourceManager(MadsM4Engine *vm): _vm(vm) {};
	virtual ~ResourceManager();

	Common::SeekableReadStream *get(const char *resourceName, bool loadFlag = true);
	void toss(const char *resourceName);
	void purge();
	void dump();
	virtual bool resourceExists(const char *resourceName) = 0;

	Common::SeekableReadStream *openFile(const char *resourceName) { return get(resourceName, false); }
	void changeExtension(char *destName, const char *sourceName, const char *extension) {
		FileSystem::changeExtension(destName, sourceName, extension);
	}
};

enum ResourceType {RESTYPE_ROOM, RESTYPE_SC, RESTYPE_TEXT, RESTYPE_QUO, RESTYPE_I,
	RESTYPE_OB, RESTYPE_FONT, RESTYPE_SOUND, RESTYPE_SPEECH, RESTYPE_HAS_EXT, RESTYPE_NO_EXT};

class MADSResourceManager : public ResourceManager {
private:
	ResourceType getResourceType(const char *resourceName);
	const char *getResourceFilename(const char *resourceName);
protected:
	Common::SeekableReadStream *loadResource(const char *resourceName, bool loadFlag);
public:
	MADSResourceManager(MadsM4Engine *vm): ResourceManager(vm) {};
	bool resourceExists(const char *resourceName);
};

class M4ResourceManager : public ResourceManager {
protected:
	Common::SeekableReadStream *loadResource(const char *resourceName, bool loadFlag);
public:
	M4ResourceManager(MadsM4Engine *vm);
	~M4ResourceManager();
	bool resourceExists(const char *resourceName);

private:
	FileSystem *_hfs;
};

} // End of namespace M4


#endif
