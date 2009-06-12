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
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "asylum/bundle.h"

namespace Asylum {

Bundle::Bundle() {
	size       = 0;
	numEntries = 0;
	offset     = 0;
}

Bundle::Bundle(uint8 fileNum, uint32 index, uint32 length) {
	loadRawRecord(parseFilename(fileNum), index, length);
}

Bundle::Bundle(uint8 fileNum) {
	Common::File* file = new Common::File;
	Common::String filename = parseFilename(fileNum);

	id = fileNum;

	// load the file
	if (!file || !file->open(filename)) {
		printf("Failed to load file %s", filename.c_str());
	}else{
		// set the filesize
		size = file->size();

		// read the entry count
		file->read( &numEntries, 4 );

		// create the resource item array and
		// set the item offset for each entry
		for (uint8 i = 0; i < numEntries; i++) {
			Bundle *bun = new Bundle;
			file->read(&bun->offset, 4);
			bun->initialized = false;
			entries.push_back(bun);
		}

		// set the last entry's offset to the filesize
		entries[numEntries - 1]->offset = size - file->pos();

		// calculate each entry's size based on the offset
		// information
		for (uint8 j = 0; j < numEntries; j++) {
			if (entries[j]->offset == 0) {
				entries[j]->size = 0;
				continue;
			}

			entries[j]->size = getNextValidOffset(j+1) - entries[j]->offset;
		}

		/* Skip population phase
		// populate the data
		for (uint8 k = 0; k < numEntries; k++) {
			if (entries[k].size > 0) {
				entries[k].data = (uint8*)malloc(entries[k].size);
				file->seek(entries[k].offset, SEEK_SET);
				file->read(entries[k].data, entries[k].size);
			}

			// DEBUGGING
			// Dump bundles to file
			//char fn[20];
			//sprintf(fn, "RES000-0%02d.DAT", k);
			//entries[k].save(Common::String(fn));
		}
		 */
	}

	file->close();
	file = NULL;
}

void Bundle::loadRawRecord(Common::String filename, uint32 index, uint32 length) {

	Common::File *file = new Common::File;

	if (!file || !file->open(filename)) {
		printf("Failed to load file %s", filename.c_str());
	}

	offset = index;
	size   = length;

	data = (uint8*)malloc(size);
	file->seek(offset, SEEK_SET);
	file->read(data, size);

	file->close();
	file = NULL;

	update();
}

void Bundle::setEntry(uint32 index, Bundle* value) {
	value->initialized = true;
	entries[index] = value;
}

Common::String Bundle::parseFilename(uint8 fileNum) {
	char filename[20];
	sprintf(filename, RESMASK, fileNum);

	return Common::String(filename);
}

uint32 Bundle::getNextValidOffset(uint8 index) {
	for (uint8 i = index; i < numEntries; i++) {
		if (entries[i]->offset != 0) {
			return entries[i]->offset;
		}
	}

	return size;
}

} // end of namespace Asylum
