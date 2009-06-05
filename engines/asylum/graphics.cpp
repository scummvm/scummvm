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

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"

namespace Asylum {

GraphicResource::GraphicResource( ResourceItem item )
{
	int pos = 0;

	_tagValue      = READ_UINT32( item.data + pos ); pos += 4;
	_flag          = READ_UINT32( item.data + pos ); pos += 4;
	_contentOffset = READ_UINT32( item.data + pos ); pos += 4;
	_unknown1      = READ_UINT32( item.data + pos ); pos += 4;
	_unknown2      = READ_UINT32( item.data + pos ); pos += 4;
	_unknown3      = READ_UINT32( item.data + pos ); pos += 4;
	_numEntries    = READ_UINT16( item.data + pos ); pos += 2;
	_maxWidthSize  = READ_UINT16( item.data + pos ); pos += 2;

	Common::Array<uint32> offsets;

	// read the individual asset offsets
	for( int i = 0; i < _numEntries; i++ ){
		offsets.push_back( READ_UINT32(item.data + pos) ); pos += 4;
	}

	for( int i = 0; i < _numEntries; i++ ){
		GraphicAsset* gra = new GraphicAsset;

		uint32 size = 0;

		// Allocate size based on offset differences
		// TODO
		// Handle zero sized entries
		if( i < _numEntries - 1 ){
			size = offsets[i + 1] - offsets[i];
		}else{
			size = item.size - offsets[i];
		}

		gra->size = size;
		gra->data = (unsigned char*)malloc(size);

		for( uint32 j = 0; j < size; j++ ){
			gra->data[j] = item.data[j + offsets[i] + _contentOffset];
		}

		int entryPos = 0;
		gra->flag   = READ_UINT32( gra->data + entryPos ); entryPos += 4;
		gra->x      = READ_UINT16( gra->data + entryPos ); entryPos += 2;
		gra->y      = READ_UINT16( gra->data + entryPos ); entryPos += 2;
		gra->width  = READ_UINT16( gra->data + entryPos ); entryPos += 2;
		gra->height = READ_UINT16( gra->data + entryPos );

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
