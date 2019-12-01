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
#include "ultima8/filesys/flex_file.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(FlexFile, ArchiveFile);



FlexFile::FlexFile(IDataSource *ds_) {
	ds = ds_;
	count = 0;
	valid = isFlexFile(ds);

	if (valid) {
		ds->seek(0x54);
		count = ds->read4();
	}
}

FlexFile::~FlexFile() {
	delete ds;
}

//static
bool FlexFile::isFlexFile(IDataSource *ds) {
	ds->seek(0);
	int i;
	char buf[0x52];
	ds->read(buf, 0x52);

	for (i = 0; i < 0x52; ++i) {
		if (buf[i] == 0x1A) break;
	}

	if (i < 0x52) {
		for (++i; i < 0x52; ++i) {
			if (buf[i] != 0x1A) return false;
		}
		return true;
	}
	return false;
}

uint32 FlexFile::getOffset(uint32 index) {
	ds->seek(0x80 + 8 * index);
	return ds->read4();
}

uint8 *FlexFile::getObject(uint32 index, uint32 *sizep) {
	if (index >= count) return 0;

	uint32 size = getSize(index);
	if (size == 0) return 0;

	uint8 *object = new uint8[size];
	uint32 offset = getOffset(index);

	ds->seek(offset);
	ds->read(object, size);

	if (sizep) *sizep = size;

	return object;
}

uint32 FlexFile::getSize(uint32 index) {
	if (index >= count) return 0;

	ds->seek(0x84 + 8 * index);
	uint32 length = ds->read4();

	return length;
}

bool FlexFile::nameToIndex(const std::string &name, uint32 &index) {
	return extractIndexFromName(name, index);
}

} // End of namespace Ultima8
