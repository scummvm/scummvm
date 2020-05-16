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

#ifndef ULTIMA8_FILESYS_RAWARCHIVE_H
#define ULTIMA8_FILESYS_RAWARCHIVE_H

#include "ultima/ultima8/filesys/archive.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ArchiveFile;
class IDataSource;

class RawArchive : public Archive {
public:
	RawArchive() : Archive() { }
	explicit RawArchive(ArchiveFile *af) : Archive(af) { }
	explicit RawArchive(Common::SeekableReadStream *rs) : Archive(rs) { }

	~RawArchive() override;

	void cache(uint32 index) override;
	void uncache(uint32 index) override;
	bool isCached(uint32 index) const override;

	//! return object. DON'T delete or modify!
	virtual const uint8 *get_object_nodel(uint32 index);

	//! return object. delete afterwards. This will not cache the object
	virtual uint8 *get_object(uint32 index);

	//! get size of object
	virtual uint32 get_size(uint32 index);

	//! return object as IDataSource. Delete the IDataSource afterwards,
	//! but DON'T delete/modify the buffer it points to.
	virtual IDataSource *get_datasource(uint32 index);

protected:
	Std::vector<uint8 *> _objects;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
