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

#ifndef ULTIMA8_FILESYS_U8SAVEFILE_H
#define ULTIMA8_FILESYS_U8SAVEFILE_H

#include "common/archive.h"

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class U8SaveFile : public Common::Archive {
public:
	//! create U8SaveFile from datasource; U8SaveFile takes ownership of ds
	//! and deletes it when destructed
	explicit U8SaveFile(Common::SeekableReadStream *rs);
	~U8SaveFile() override;

	//! Check if constructed object is indeed a valid archive
	bool isValid() const {
		return _valid;
	}

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	static bool isU8SaveFile(Common::SeekableReadStream *rs);

protected:
	Common::SeekableReadStream *_rs;
	bool _valid;

	struct FileEntry {
		uint32 _offset;
		uint32 _size;
		FileEntry() : _offset(0), _size(0) {}
	};

	typedef Common::HashMap<Common::String, FileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> U8SaveFileMap;
	U8SaveFileMap _map;

private:
	bool readMetadata();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
