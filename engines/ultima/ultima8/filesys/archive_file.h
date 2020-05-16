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

#ifndef ULTIMA8_FILESYS_ARCHIVEFILE_H
#define ULTIMA8_FILESYS_ARCHIVEFILE_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ArchiveFile {
public:
	virtual ~ArchiveFile() { }

	//! Check if constructed object is indeed a valid archive
	virtual bool isValid() const {
		return _valid;
	}

	//! Check if numbered object exists
	//! If the Flex has named objects, only objects with numerical names will
	//!  be returned (the filename without the extension must be an integer)
	//! \param index index of object to check for
	virtual bool exists(uint32 index) = 0;

	//! Check if named object exists
	//! If the Flex is not named, name must be an integer with
	//!  an optional extension
	//! \param name name of object to check for
	virtual bool exists(const Std::string &name) = 0;


	//! Get object from file; returns NULL if index is invalid.
	//! Must delete the returned buffer afterwards.
	//! See also exists(uint32 index)
	//! \param index index of object to fetch
	//! \param size if non-NULL, size of object is stored in *size
	virtual uint8 *getObject(uint32 index, uint32 *size = 0) = 0;

	//! Get named object from file; returns NULL if name is invalid.
	//! Must delete the returned buffer afterwards.
	//! See also exists(Std::string name)
	//! \param name name of object to fetch
	//! \param size if non-NULL, size of object is stored in *size
	virtual uint8 *getObject(const Std::string &name, uint32 *size = 0) = 0;


	//! Get size of object; returns zero if index is invalid.
	//! See also exists(uint32 index)
	//! \param index index of object to get size of
	virtual uint32 getSize(uint32 index) const = 0;

	//! Get size of named object; returns zero if name is invalid
	//! See also exists(Std::string name)
	//! \param index index of object to get size of
	virtual uint32 getSize(const Std::string &name) const = 0;

	//! Get object as a Common::SeekableReadStream
	//! Delete the SeekableReadStream afterwards; that will delete the data as well
	Common::SeekableReadStream *getDataSource(uint32 index, bool is_text = false);

	//! Get named as a Common::SeekableReadStream
	//! Delete the SeekableReadStream afterwards; that will delete the data as well
	Common::SeekableReadStream *getDataSource(const Std::string &name, bool is_text = false);

	//! Get upper bound for number of objects.
	//! In an indexed file this is (probably) the highest index plus one,
	//! while in a named file it's (probably) the actual count
	virtual uint32 getCount() const = 0;

	//! Get the highest index in the file
	//! Guaranteed to be sufficiently large for a vector that needs to
	//!  store the indexed entries of this file
	virtual uint32 getIndexCount() const = 0;

	//! is archive indexed?
	virtual bool isIndexed() const = 0;

	//! is archive named?
	virtual bool isNamed() const = 0;

protected:
	static bool extractIndexFromName(const Std::string &name, uint32 &index);

	bool _valid;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
