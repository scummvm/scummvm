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

#ifndef STARK_ARCHIVE_H
#define STARK_ARCHIVE_H

#include "common/archive.h"
#include "common/stream.h"

namespace Stark {
namespace Formats {

class XARCMember;

class XARCArchive : public Common::Archive {
public:
	bool open(const Common::String &filename);
	Common::String getFilename() const;

	// Archive API
	bool hasFile(const Common::Path &path) const;
	int listMatchingMembers(Common::ArchiveMemberList &list, const Common::Path &pattern) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const;

	Common::SeekableReadStream *createReadStreamForMember(const XARCMember *member) const;

private:
	Common::String _filename;
	Common::ArchiveMemberList _members;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_ARCHIVE_H
