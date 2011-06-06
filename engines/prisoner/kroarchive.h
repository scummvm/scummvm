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

#ifndef PRISONER_KROARCHIVE_H
#define PRISONER_KROARCHIVE_H

#include "common/file.h"
#include "common/array.h"

namespace Prisoner {

struct KroArchiveEntry {
	uint32 size, compressedSize;
	uint32 offset;
	uint32 compressionType;
};

struct PakDirectoryEntry {
	Common::String pakName;
	uint32 baseIndex;
};

struct _PakDirectoryEntry {
	const char *pakName;
	uint32 baseIndex;
};

class KroArchive {
public:
	KroArchive();
	~KroArchive();
	void open(const char *filename);
	void close();
	byte *load(uint index);
	uint32 getSize(uint index);
	uint getCount() const { return _entries.size(); }
	void loadDirectory(const char *filename, uint32 offset, bool isEncrypted);
	void loadDirectory(const _PakDirectoryEntry directory[]);
	uint32 getPakBaseIndex(Common::String &pakName);
	void setResourceTypes(const int16 *resourceTypes) { _resourceTypes = resourceTypes; }
	bool hasResourceType(int16 resourceType);
protected:
	Common::File *_fd;
	Common::Array<KroArchiveEntry> _entries;
	Common::Array<PakDirectoryEntry> _pakDirectory;
	const int16 *_resourceTypes;
};

} // End of namespace Prisoner

#endif /* PRISONER_KROARCHIVE_H */
