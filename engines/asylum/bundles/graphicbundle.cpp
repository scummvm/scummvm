/*
 * graphicbundle.cpp
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#include "asylum/bundles/graphicbundle.h"

namespace Asylum {

GraphicBundle::GraphicBundle(Common::String filename, uint32 index) {
	loadRaw(filename, index);
}

void GraphicBundle::update() {
	uint32 pos = 0;

	_tagValue      = READ_UINT32(data + pos); pos += 4;
	_flag          = READ_UINT32(data + pos); pos += 4;
	_contentOffset = READ_UINT32(data + pos); pos += 4;
	_unknown1      = READ_UINT32(data + pos); pos += 4;
	_unknown2      = READ_UINT32(data + pos); pos += 4;
	_unknown3      = READ_UINT32(data + pos); pos += 4;
	numEntries     = READ_UINT16(data + pos); pos += 2;
	_maxWidth      = READ_UINT16(data + pos); pos += 2;

	Common::Array<uint32> offsets;

	// read the individual asset offsets
	for (uint32 i = 0; i < numEntries; i++) {
		offsets.push_back(READ_UINT32(data + pos)); pos += 4;
	}

	// read each asset
	for (uint32 i = 0; i < numEntries; i++) {
		GraphicResource *gra = new GraphicResource;

		pos = offsets[i] + _contentOffset;

		gra->size   = READ_UINT32(data + pos); pos += 4;
		gra->flag   = READ_UINT32(data + pos); pos += 4;
		gra->x      = READ_UINT16(data + pos); pos += 2;
		gra->y      = READ_UINT16(data + pos); pos += 2;
		gra->height = READ_UINT16(data + pos); pos += 2;
		gra->width  = READ_UINT16(data + pos); pos += 2;

		// allocate space for data and fill the array from
		// the end of the header block (read in above) to
		// the length specified by gra->size
		gra->data = (uint8*)malloc(gra->size - 16);
		memcpy(gra->data, data + pos, gra->size - 16);

		entries.push_back(*gra);
	}
}

GraphicResource GraphicBundle::getEntry(uint32 index) {
	return (GraphicResource)entries[index];
}

} // end of namespace Asylum
