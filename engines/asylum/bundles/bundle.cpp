/*
 * bundle.cpp
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
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
