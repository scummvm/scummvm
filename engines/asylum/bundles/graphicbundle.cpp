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
