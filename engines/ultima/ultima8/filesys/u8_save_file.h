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

#ifndef ULTIMA8_FILESYS_U8SAVEFILE_H
#define ULTIMA8_FILESYS_U8SAVEFILE_H

#include "ultima/ultima8/filesys/named_archive_file.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

class U8SaveFile : public NamedArchiveFile {
public:
	//! create U8SaveFile from datasource; U8SaveFile takes ownership of ds
	//! and deletes it when destructed
	explicit U8SaveFile(Common::SeekableReadStream *rs);
	~U8SaveFile() override;

	bool exists(const Std::string &name) override;

	uint8 *getObject(const Std::string &name, uint32 *size = 0) override;

	uint32 getSize(const Std::string &name) const override;

	uint32 getCount() const override {
		return _count;
	}

	static bool isU8SaveFile(Common::SeekableReadStream *rs);

protected:
	Common::SeekableReadStream *_rs;
	uint32 _count;

	Common::HashMap<Common::String, uint32> _indices;
	Std::vector<uint32> _offsets;
	Std::vector<uint32> _sizes;

private:
	bool readMetadata();
	bool findIndex(const Std::string &name, uint32 &index) const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
