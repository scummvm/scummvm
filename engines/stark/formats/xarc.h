/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
	bool hasFile(const Common::String &name) const;
	int listMatchingMembers(Common::ArchiveMemberList &list, const Common::String &pattern) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

	Common::SeekableReadStream *createReadStreamForMember(const XARCMember *member) const;

private:
	Common::String _filename;
	Common::ArchiveMemberList _members;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_ARCHIVE_H
