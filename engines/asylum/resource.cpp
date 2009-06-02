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

#include "asylum/resource.h"

namespace Asylum {

//////////////
// Resource //
//////////////

Resource::Resource() 
{
}

Resource::~Resource()
{
	for( uint8 i = 0; i < _numEntries; i++ ){
		free( _items[i].data );
	}
}

int Resource::load( Common::String filename )
{
	_filename = filename;
	
	Common::File* file = new Common::File;
	
	// load the file
	if( !file || !file->open(filename) ){
        printf( "Failed to load file %s", _filename.c_str() );        
        return -1;    
    }
	
	_size = file->size();
	
	// read the entry count
	file->read( &_numEntries, 4 );
		
	// create the resource item array and
	// set the item offset for each entry
	for( uint8 i = 0; i < _numEntries; i++ ){
		ResourceItem* item = new ResourceItem;
		file->read( &item->offset, 4 );
		_items.push_back( *item );
	}
	
	// set the last entry's offset to the filesize
	_items[_numEntries - 1].offset = _size - file->pos();
	
	// calculate each entry's size based on the offset
	// information
	for( uint8 j = 0; j < _numEntries; j++ ){
		if( _items[j].offset == 0 ){
			_items[j].size = 0;
			continue;
		}
		_items[j].size = getNextValidOffset( j+1 ) - _items[j].offset;		
		
		printf( "[%d] ", j );
		_items[j].dump();
	}
	
	// populate the data
	for( uint8 k = 0; k < _numEntries; k++ ){
		if( _items[k].size > 0 ){				
			_items[k].data = (unsigned char*)malloc(_items[k].size);
			file->seek( _items[k].offset, SEEK_SET );
			file->read( _items[k].data, _items[k].size );	
			printf( "[%d] %c%c%c%c\n", k, _items[k].data[0], _items[k].data[1],_items[k].data[2],_items[k].data[3] );								
		}
	}
		
	
	file->close();
	file = NULL;
	
	_loaded = true;
	
	return 0;
}

uint32 Resource::getNextValidOffset( uint8 startPos )
{
	for( uint8 i = startPos; i < _numEntries; i++ ){
		if( _items[i].offset != 0 ){
			return _items[i].offset;
		}		
	}
	
	return _size;
}

ResourceItem Resource::getResource( uint32 pos )
{
	if( pos >= 0 && pos < _size ){
		return _items[pos];
	}
}

void Resource::dump()
{
	printf( "File %s: Pack Size: %d, Entries: %d\n", _filename.c_str(), _size, _numEntries ); 
}
	
//////////////////
// ResourceItem //
//////////////////
	
ResourceItem::ResourceItem()
{
}

ResourceItem::~ResourceItem()
{
}

void ResourceItem::dump()
{			
	printf( "Size: %d, Offset: %d\n", size, offset );
}

int ResourceItem::save( Common::String filename )
{
	FILE *fd;
    fd = fopen(filename.c_str(),"wb+");
    fwrite( data, size, 1, fd);
    fclose(fd);
}

} // end of namespace Asylum
