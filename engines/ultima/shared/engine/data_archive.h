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

#ifndef ULTIMA_SHARED_ENGINE_DATA_ARCHIVE_H
#define ULTIMA_SHARED_ENGINE_DATA_ARCHIVE_H

#include "common/archive.h"
#include "common/str.h"

namespace Ultima {
namespace Shared {

/**
 * The data archive class encapsulates access to a specific subfolder within
 * the ultima.dat data file for a game. It wraps up the subfolder so it can
 * be accessed in each game as a generic "data" subfolder. This allows the
 * individual games to simplify their data loading code.
 */
class UltimaDataArchive : public Common::Archive {
private:
    Common::Archive *_zip;
	Common::String _publicFolder;
	Common::String _innerfolder;


    UltimaDataArchive(Common::Archive *zip, const Common::String &subfolder) :
		_zip(zip), _publicFolder("data/"), _innerfolder(subfolder + "/") {}

	Common::String innerToPublic(const Common::String &filename) const {
		assert(filename.hasPrefixIgnoreCase(_publicFolder));
		return _innerfolder + Common::String(filename.c_str() + _publicFolder.size());
	}
public:
	/**
	 * Creates a data archive wrapper for the ultima.dat datafile.
	 * If the required data is found, it returns the new archive.
	 * Otherwise, returns an error message in the errorMsg field
	 */
    static UltimaDataArchive *load(const Common::String &subfolder,
		int reqMajorVersion, int reqMinorVersion, Common::String &errorMsg);
public:
	virtual ~UltimaDataArchive() {
		delete _zip;
	}

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	virtual bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive matching the specified pattern to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of members added to list
	 */
	virtual int listMatchingMembers(Common::ArchiveMemberList &list,
		const Common::String &pattern) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	virtual int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name)
		const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	virtual Common::SeekableReadStream *createReadStreamForMember(
		const Common::String &name) const override;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
