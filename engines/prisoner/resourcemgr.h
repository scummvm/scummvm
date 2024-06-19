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

#ifndef PRISONER_RESOURCEMANAGER_H
#define PRISONER_RESOURCEMANAGER_H

#include "common/array.h"
#include "common/file.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/memstream.h"

#include "prisoner/kroarchive.h"

namespace Prisoner {

const int16 kMaxResourceSlots = 100;

class BaseResource {
public:
	virtual ~BaseResource() {}
	virtual void load(Common::MemoryReadStream &stream) = 0;
};

struct ResourceSlot {
	int16 type;
	uint refCount;
	Common::String pakName;
	int16 pakSlot;
	BaseResource *resource;
	ResourceSlot() : type(-1), refCount(0), resource(NULL), pakSlot(-1) {}
};

class ResourceLoader {
public:
	virtual ~ResourceLoader() {}
	virtual byte *load(Common::String &pakName, int16 pakSlot, int16 type, uint32 &dataSize) = 0;
};

class PrisonerResourceLoader : public ResourceLoader {
public:
	PrisonerResourceLoader(char languageChar);
	~PrisonerResourceLoader();
	byte *load(Common::String &pakName, int16 pakSlot, int16 type, uint32 &dataSize);
	KroArchive *getArchiveForType(int16 type);
	void addArchive(const char *filename, const _PakDirectoryEntry directory[], const int16 *resourceTypes);
	void addArchive(const char *filename, const char *directoryFilename, uint32 offset, bool isEncrypted, const int16 *resourceTypes);
protected:
	Common::Array<KroArchive*> _archives;
};

class ResourceManager {
public:
	ResourceManager(ResourceLoader *loader);
	~ResourceManager();
	template<class T>
	int16 load(Common::String &pakName, int16 pakSlot, int16 type) {
		int16 slotIndex = find(pakName, pakSlot, type);
		if (slotIndex != -1) {
			_slots[slotIndex].refCount++;
		} else {
			byte *data;
			uint32 dataSize;
			slotIndex = add(pakName, pakSlot, type);
			data = _loader->load(pakName, pakSlot, type, dataSize);
			Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::YES);
			_slots[slotIndex].resource = new T();
			_slots[slotIndex].resource->load(stream);
		}
		return slotIndex;
	}
	template<class T>
	T* get(int16 slotIndex) const { return (T*)_slots[slotIndex].resource; }
	void unload(int16 slotIndex);
	void purge(bool all);
	void getSlotInfo(int16 slotIndex, Common::String &pakName, int16 &pakSlot, int16 *type = NULL) {
		pakName = _slots[slotIndex].pakName;
		pakSlot = _slots[slotIndex].pakSlot;
		if (type)
			*type = _slots[slotIndex].type;
	}
	void dump(Common::String &pakName, int16 pakSlot, int16 type);
protected:
	ResourceSlot _slots[kMaxResourceSlots];
	ResourceLoader *_loader;
	int16 _freeSlotsCount;
	int16 find(Common::String &pakName, int16 pakSlot, int16 type);
	int16 add(Common::String &pakName, int16 pakSlot, int16 type);
};

} // End of namespace Prisoner

#endif /* PRISONER_RESOURCEMANAGER_H */
