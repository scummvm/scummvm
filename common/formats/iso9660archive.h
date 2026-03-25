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

/**
 * @file iso9660archive.h
 * Common::Archive adapter for an ISO 9660 filesystem image.
 *
 * Mounts the contents of an ISO image (or a subdirectory within it) as a
 * Common::Archive, making game files inside a disc image accessible through
 * SearchMan without extracting them to disk.
 *
 * Usage:
 * @code
 *   auto fs = Common::makeSharedPtr(new Common::ISO9660FileSystem(stream));
 *   SearchMan.add("game-iso", new Common::ISO9660Archive(fs));
 * @endcode
 *
 * For compilation CDs with one game per subdirectory, specify the subdir:
 * @code
 *   SearchMan.add("jones", new Common::ISO9660Archive(fs, "JONES"));
 * @endcode
 */

#ifndef COMMON_FORMATS_ISO9660ARCHIVE_H
#define COMMON_FORMATS_ISO9660ARCHIVE_H

#include "common/archive.h"
#include "common/formats/iso9660.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/str.h"

namespace Common {

/**
 * Common::Archive implementation backed by an ISO 9660 filesystem image.
 */
class ISO9660Archive : public Archive {
public:
	/**
	 * @param fs      Shared pointer to the underlying ISO9660FileSystem.
	 * @param subdir  Optional case-insensitive subdirectory within the ISO.
	 *                Files inside subdir are presented as top-level members.
	 *                Empty string (default) uses the ISO root.
	 */
	explicit ISO9660Archive(SharedPtr<ISO9660FileSystem> fs,
	                        const String &subdir = String());

	// Archive interface
	bool hasFile(const Path &path) const override;
	bool isPathDirectory(const Path &path) const override;
	int  listMembers(ArchiveMemberList &list) const override;
	const ArchiveMemberPtr getMember(const Path &path) const override;
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;
	bool getChildren(const Path &path, Array<String> &list,
	                 ListMode mode, bool hidden) const override;

private:
	struct FileRecord {
		ISO9660FileSystem::DirEntry entry;
		String                      archivePath; // lowercase, slash-separated
	};

	void buildIndex();
	void scanDirectory(const ISO9660FileSystem::DirEntry &dir, const String &prefix);

	SharedPtr<ISO9660FileSystem> _fs;
	String _subdir; // normalized to lowercase

	// Case-insensitive map from archive-relative path to file record
	using IndexMap = HashMap<Path, FileRecord, Path::IgnoreCase_Hash, Path::IgnoreCase_EqualTo>;
	IndexMap _index;
};

} // namespace Common

#endif // COMMON_FORMATS_ISO9660ARCHIVE_H
