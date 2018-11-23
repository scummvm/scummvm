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

#ifndef GLK_FROTZ_PICS
#define GLK_FROTZ_PICS

#include "common/archive.h"
#include "common/array.h"

namespace Glk {
namespace Frotz {

/**
 * Infocom graphics file manager
 */
class Pics : public Common::Archive {
	/**
	 * Describes one chunk of the Blorb file.
	 */
	struct Entry {
		uint _number;
		size_t _offset;
		size_t _size;
		Common::String _filename;
	};
private:
	Common::String _filename;
	Common::Array<Entry> _index;	///< list of entries
	uint _entrySize;
	uint _version;
private:
	/**
	 * Returns the filename for the pictures archive
	 */
	static Common::String getFilename();
public:
	/**
	 * Returns true if an mg1 file exists for the game
	 */
	static bool exists();
public:
	/**
	 * Constructor
	 */
	Pics();

	/**
	 * Return the number of entries in the file
	 */
	size_t size() const { return _index.size(); }

	/**
	 * Return the version of the file
	 */
	uint version() const { return _version; }

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	virtual bool hasFile(const Common::String &name) const override;

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
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
