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

#ifndef GLK_ZCODE_PICS_H
#define GLK_ZCODE_PICS_H

#include "common/archive.h"
#include "common/array.h"
#include "common/file.h"

namespace Glk {
namespace ZCode {

enum PicturesMode {
	kMONO  = 0,
	kTEXT  = 1,
	kCGA   = 2,
	kMCGA  = 3,
	kEGA   = 4,
	kAmiga = 5
};

/**
 * Infocom graphics file manager
 */
class Pics : public Common::Archive {
	/**
	 * Describes a single index entry
	 */
	struct Entry {
		uint _number;
		size_t _width, _height;
		uint _flags;
		size_t _dataOffset;
		size_t _dataSize;
		size_t _paletteOffset;
		Common::String _filename;

		/**
		 * Constructor
		 */
		Entry() : _number(0), _width(0), _height(0), _flags(0), _dataOffset(0), _dataSize(0),
			_paletteOffset(0) {}
	};
private:
	Common::String _filename;
	Common::Array<Entry> _index;	///< list of entries
	uint _entrySize;
	uint _version;
	Common::Array<byte> *_palette;
private:
	/**
	 * Returns the filename for the pictures archive
	 */
	static Common::String getFilename();

	/**
	 * Read in the palette
	 */
	void loadPalette(Common::File &f, const Entry &e, Common::Array<byte> &palette) const;
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
	 * Destructor
	 */
	~Pics() override;

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
	bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
