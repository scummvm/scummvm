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

#include "asylum/graphics.h"
#include "asylum/utils.h"

#include "common/file.h"
#include "common/stream.h"

namespace Asylum {

GraphicResource::GraphicResource( ResourceItem item )
{
	int pos = 0;

	_packSize = item.size;

	// DEBUG
	// This logic is somewhat flawed, as the Flag value is
	// getting the tag value, and the tag value is getting
	// junk data, but the rest seems to be correct.
	// Since this is still a test though, it'll likely
	// be re-written at some point

	_tagValue      = read32( item.data, pos );
	_flag          = read32( item.data, pos );
	_contentOffset = read32( item.data, pos );
	_unknown1      = read32( item.data, pos );
	_unknown2      = read32( item.data, pos );
	_unknown3      = read32( item.data, pos );
	_numEntries    = read16( item.data, pos );
	_maxWidthSize  = read16( item.data, pos );

	Common::Array<uint32> offsets;

	// read the individual asset offsets
	for( int i = 0; i < _numEntries; i++ ){
		offsets.push_back( read32(item.data, pos) );
	}

	for( int i = 0; i < _numEntries; i++ ){
		GraphicAsset* gra = new GraphicAsset;

		uint32 size;

		// Allocate size based on offset differences
		// TODO
		// Handle zero sized entries
		if( i < _numEntries - 1 ){
			size = offsets[i + 1] - offsets[i];
		}else{
			size = _packSize - offsets[i];
		}

		gra->size = size;
		gra->data = (unsigned char*)malloc(size);

		for( uint32 j = 0; j < size; j++ ){
			gra->data[j] = item.data[j + offsets[i] + _contentOffset];
		}

		int entryPos = 0;
		gra->flag   = read32( gra->data, entryPos );
		gra->x      = read16( gra->data, entryPos );
		gra->y      = read16( gra->data, entryPos );
		gra->width  = read16( gra->data, entryPos );
		gra->height = read16( gra->data, entryPos );

		_items.push_back( *gra );

		gra->dump();
	}
}

GraphicResource::~GraphicResource()
{
}

void GraphicResource::dump()
{
    printf( "Tag %d, Flag %d, ConOffset %d, U1 %d, U2 %d, U3 %d, Entries %d, MaxWidthSize %d\n", _tagValue, _flag, _contentOffset, _unknown1, _unknown2, _unknown3, _numEntries, _maxWidthSize );
}

//////////////////
// GraphicAsset //
//////////////////

GraphicAsset::GraphicAsset()
{
}

GraphicAsset::~GraphicAsset()
{
}

void GraphicAsset::dump()
{
	printf( "Size: %d, Flag %d, Width: %d, Height: %d, x:%d, y: %d\n", size, flag, width, height, x, y );
}

} // end of namespace Asylum
