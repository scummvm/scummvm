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

#ifndef CRYOMNI3D_ATLANTIS_BIGFILE_H
#define CRYOMNI3D_ATLANTIS_BIGFILE_H

#include "common/archive.h"
#include "common/array.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/str.h"
#include "common/stream.h"

namespace CryOmni3D {
namespace Atlantis {

// Reads files from Atlantis's "BigFile 1.00" archives.
// All game files are stored sequentially after the directory.
// Archive entries use uppercase backslash paths (e.g. "WAM\ATLAN1.WAM").
//
// The full game spans four discs (BIGCD1-4.BIG); open() is additive, so all
// four can be mounted into one merged index.  When a file appears on several
// discs (shared menus, fonts, common scripts) the first-mounted copy wins —
// duplicates across discs are byte-identical.
// Implements Common::Archive so it can be registered with SearchMan.
class BigFileArchive : public Common::Archive {
public:
	BigFileArchive();
	~BigFileArchive() override;

	// Mount one BigFile archive, merging its directory into the index.
	// Call repeatedly to mount additional discs.  Already-mounted discs are
	// left intact if a later call fails.
	bool open(const Common::Path &path);
	void close();

	// Common::Archive interface.
	// Incoming paths are normalised (uppercase, backslash) before lookup.
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	// Low-level lookup using the exact archive key ("SUBDIR\FILE.EXT").
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
	bool hasFile(const Common::String &name) const;

	// Number of distinct files across all mounted discs.
	uint numFiles() const { return _entries.size(); }

private:
	struct Entry {
		uint32 seqOffset; // byte offset from the owning disc's data base
		uint32 size;
		uint   disc;      // index into _files / _dataBases
	};

	// Normalise a ScummVM path to the archive key format: uppercase + backslash.
	static Common::String normalizePath(const Common::Path &path);

	Common::Array<Common::File *> _files;     // one handle per mounted .BIG
	Common::Array<uint32>         _dataBases; // data-section base, per disc
	Common::HashMap<Common::String, Entry> _entries;
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_BIGFILE_H
