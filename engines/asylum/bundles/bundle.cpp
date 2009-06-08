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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "common/file.h"

#include "asylum/bundles/bundle.h"
#include "asylum/offsets.h"

namespace Asylum {

Bundle::Bundle() {
	size       = 0;
	numEntries = 0;
}

Bundle::Bundle(Common::String filename, uint32 index) {
	loadRaw(filename, index);
}

int Bundle::loadRaw(Common::String filename, uint32 index) {
	Common::File *file = new Common::File;

	if (!file || !file->open(filename)) {
		printf("Failed to load file %s", filename.c_str());
		return -1;
	}

	uint32 offset = res001[index][0];
	size = res001[index][1];

	data = (uint8*)malloc(size);
	file->seek(offset, SEEK_SET);
	file->read(data, size);

	file->close();
	file = NULL;

	update();

	return 0;
}

} // end of namespace Asylum
