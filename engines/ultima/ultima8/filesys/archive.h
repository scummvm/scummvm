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

#ifndef ULTIMA8_FILESYS_ARCHIVE_H
#define ULTIMA8_FILESYS_ARCHIVE_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ArchiveFile;

class Archive {
public:
	//! create Archive without any input sources
	Archive();

	//! create Archive with a single input source
	explicit Archive(ArchiveFile *af);

	//! create Archive with a single input source, autodetecting the type
	//! Will create FlexFile, U8SaveFile or ZipFile; ids will be deleted.
	explicit Archive(Common::SeekableReadStream *rs);

	virtual ~Archive();

	//! add input source.
	//! ArchiveFile will be deleted on destruction
	//! Input sources are used in the reversed order they are added.
	//! Effect of adding sources after having accessed objects is undef.
	bool addSource(ArchiveFile *af);

	//! add input source, autodetecting the type (as the constructor)
	bool addSource(Common::SeekableReadStream *rs);

	//! Cache all objects
	void cache();

	//! Cache a single object
	virtual void cache(uint32 index) = 0;

	//! Uncache all objects
	//! Potentially dangerous: all stored objects will be deleted; make sure
	//!  they are no longer in use.
	void uncache();

	//! Uncache a single object
	//! Potentially dangerous. See uncache()
	virtual void uncache(uint32 index) = 0;

	//! Check if an object is cached
	virtual bool isCached(uint32 index) const = 0;

	uint32 getCount() const {
		return _count;
	}

protected:
	uint32 _count;

	uint8 *getRawObject(uint32 index, uint32 *sizep = 0);
	uint32 getRawSize(uint32 index) const;

private:
	Std::vector<ArchiveFile *> _sources;

	ArchiveFile *findArchiveFile(uint32 index) const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
