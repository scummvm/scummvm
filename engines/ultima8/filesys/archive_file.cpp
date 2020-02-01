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

#include "ultima8/misc/pent_include.h"

#include "ultima8/filesys/archive_file.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(ArchiveFile)

//static
bool ArchiveFile::extractIndexFromName(const std::string &name, uint32 &index) {
	if (name.size() == 0) return false;

	char *endptr;
	long val;

	val = std::strtol(name.c_str(), &endptr, 10);

	// if remainder of name doesn't start with a '.', invalid name
	if (*endptr != '\0' && *endptr != '.') return false;

	if (val < 0) return false;

	index = static_cast<uint32>(val);

	return true;
}

IDataSource *ArchiveFile::getDataSource(uint32 index, bool is_text) {
	uint32 size;
	uint8 *buf = getObject(index, &size);

	if (!buf) return 0;

	return new IBufferDataSource(buf, size, is_text, true);
}

IDataSource *ArchiveFile::getDataSource(const std::string &name, bool is_text) {
	uint32 size;
	uint8 *buf = getObject(name, &size);

	if (!buf) return 0;

	return new IBufferDataSource(buf, size, is_text, true);
}

} // End of namespace Ultima8
