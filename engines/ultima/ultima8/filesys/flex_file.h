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

#ifndef ULTIMA8_FILESYS_FLEXFILE_H
#define ULTIMA8_FILESYS_FLEXFILE_H

#include "ultima/ultima8/filesys/archive_file.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class FlexFile : public ArchiveFile {
public:
	//! create FlexFile from datasource; FlexFile takes ownership of ds
	//! and deletes it when destructed
	explicit FlexFile(Common::SeekableReadStream *rs);
	~FlexFile() override;

	bool exists(uint32 index) override {
		return getSize(index) > 0;
	}
	bool exists(const Std::string &name) override {
		uint32 index;
		if (nameToIndex(name, index))
			return exists(index);
		else
			return false;
	}

	uint8 *getObject(uint32 index, uint32 *size = nullptr) override;
	uint8 *getObject(const Std::string &name, uint32 *size = nullptr) override {
		uint32 index;
		if (nameToIndex(name, index))
			return getObject(index, size);
		else
			return nullptr;
	}


	uint32 getSize(uint32 index) const override;
	uint32 getSize(const Std::string &name) const override {
		uint32 index;
		if (nameToIndex(name, index))
			return getSize(index);
		else
			return 0;
	}

	uint32 getCount() const override {
		return _count;
	}

	uint32 getIndexCount() const override {
		return _count;
	}

	bool isIndexed() const override {
		return true;
	}
	bool isNamed() const override {
		return false;
	}

	static bool isFlexFile(Common::SeekableReadStream *rs);

protected:
	bool nameToIndex(const Std::string &name, uint32 &index) const;

	Common::SeekableReadStream *_rs;
	uint32 _count;

private:
	uint32 getOffset(uint32 index);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
