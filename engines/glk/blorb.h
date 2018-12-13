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

#ifndef GLK_BLORB_H
#define GLK_BLORB_H

#include "glk/glk_types.h"
#include "glk/streams.h"
#include "common/archive.h"
#include "common/array.h"

namespace Glk {

/**
 * Describes one chunk of the Blorb file.
 */
struct ChunkEntry {
	uint _type;
	uint _number;
	uint _id;
    size_t _offset;
	size_t _size;
	Common::String _filename;
};

/**
 * Blorb file manager
 */
class Blorb : public Common::Archive {
private:
	Common::String _filename;
	Common::FSNode _fileNode;
	InterpreterType _interpType;
	Common::Array<ChunkEntry> _chunks;	///< list of chunk descriptors
private:
	/**
	 * Parses the Blorb file index to load in a list of the chunks
	 */
	Common::ErrorCode load();
public:
	/**
	 * Constructor
	 */
	Blorb(const Common::String &filename, InterpreterType interpType);

	/**
	 * Constructor
	 */
	Blorb(const Common::FSNode &fileNode, InterpreterType interpType);

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

} // End of namespace Glk

#endif
