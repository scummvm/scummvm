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
	_tagValue      = read32(item.data, pos);	
	_flag          = read32(item.data, pos+=4);
	_contentOffset = read32(item.data, pos+=4);
	_unknown1      = read32(item.data, pos+=4);
	_unknown2      = read32(item.data, pos+=4);
	_unknown3      = read32(item.data, pos+=4);
	_numEntries    = read32(item.data, pos+=4);
	_maxWidthSize  = read16(item.data, pos+=2);
}

GraphicResource::~GraphicResource()
{
}

void GraphicResource::dump()
{   
    printf( "Tag %d, Flag %d, ConOffset %d, U1 %d, U2 %d, U3 %d, Entries %d, MaxWidthSize %d\n", _tagValue, _flag, _contentOffset, _unknown1, _unknown2, _unknown3, _numEntries, _maxWidthSize );
}

} // end of namespace Asylum
