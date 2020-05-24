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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/filesys/archive_file.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

//static
bool ArchiveFile::extractIndexFromName(const Std::string &name, uint32 &index) {
	if (name.size() == 0) return false;

	char *endptr;
	long val;

	val = Std::strtol(name.c_str(), &endptr, 10);

	// if remainder of name doesn't start with a '.', invalid name
	if (*endptr != '\0' && *endptr != '.') return false;

	if (val < 0) return false;

	index = static_cast<uint32>(val);

	return true;
}

Common::SeekableReadStream *ArchiveFile::getDataSource(uint32 index, bool is_text) {
	uint32 size;
	uint8 *buf = getObject(index, &size);

	if (!buf)
		return nullptr;

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *ArchiveFile::getDataSource(const Std::string &name, bool is_text) {
	uint32 size;
	uint8 *buf = getObject(name, &size);

	if (!buf)
		return nullptr;

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

} // End of namespace Ultima8
} // End of namespace Ultima
