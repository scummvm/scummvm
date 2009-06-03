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

#ifndef ASYLUM_GRAPHICS_H
#define ASYLUM_GRAPHICS_H

#include "common/str.h"
#include "common/array.h"

#include "asylum/resource.h"

namespace Asylum {

class GraphicResource {
public:
    GraphicResource( ResourceItem item );
    ~GraphicResource();
    
	void dump();
	
private:
    Common::String _filename;    
    uint32 _packSize;
    uint32 _tagValue;
    uint32 _flag;
    uint32 _contentOffset;
    uint32 _unknown1;
    uint32 _unknown2;
    uint32 _unknown3;
    uint16 _numEntries;
    uint16 _maxWidthSize;
	
}; // end of class GraphicResource

class GraphicAsset {
public:
    GraphicAsset();
    ~GraphicAsset();
	
private:	
    uint8 _offset;
    uint8 _size;
    uint8 _flag;
    uint8 _offsetWidth;  // screenX position?
    uint8 _offsetHeight; // screenY position?
    uint8 _width;
    uint8 _height;
    uint8 _dataSize;     // data block size
    uint8 *_data;
	
}; // end of class GraphicAsset

} // end of namespace Asylum

#endif
