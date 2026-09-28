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

#ifndef ULTIMA2_DATA_SECRET_ARCHIVE_H
#define ULTIMA2_DATA_SECRET_ARCHIVE_H

#include "common/archive.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {


/**
 * Implements a special secret city map file just for ScummVM
 */
class SecretMapArchive : public Common::Archive {
public:
	~SecretMapArchive() override {
	}

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::Path &path) const override;

	/**
	 * Add all members of the Archive matching the specified pattern to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of members added to list
	 */
	int listMatchingMembers(Common::ArchiveMemberList &list,
		const Common::Path &pattern, bool matchPathComponents = false) const override {
		return 0;
	}

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override {
		return 0;
	}

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::Path &path)
		const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(
		const Common::Path &path) const override;
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
