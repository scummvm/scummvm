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

#ifndef HARVESTER_XFILE_ARCHIVE_H
#define HARVESTER_XFILE_ARCHIVE_H

#include "common/archive.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/mutex.h"
#include "common/ptr.h"

namespace Harvester {

class XFileArchive : public Common::Archive {
public:
	XFileArchive();
	~XFileArchive() override;

	bool open(const Common::String &indexPath, const Common::String &dataPath);
	void close();

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	struct Entry {
		uint32 _archiveOffset = 0;
		uint32 _storedSize = 0;
		uint32 _packedFlag = 0;
		uint32 _unpackedSize = 0;
	};

	Common::SeekableReadStream *openStoredEntry(const Entry &entry) const;
	Common::SeekableReadStream *openPackedEntry(const Entry &entry) const;

	Common::Path _dataPath;
	Common::ScopedPtr<Common::File> _dataFile;
	mutable Common::Mutex _dataFileMutex;
	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> EntryMap;
	EntryMap _entries;
};

} // End of namespace Harvester

#endif // HARVESTER_XFILE_ARCHIVE_H
