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

#ifndef PICTURE_RESOURCE_H
#define PICTURE_RESOURCE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

/*
	TODO:
	
	ArchiveReader:
	    - Add support for external resources; and check if they're used

*/

const uint kMaxCacheItems = 1024;
const uint kMaxCacheSize = 8 * 1024 * 1024; // 8 MB


class ArchiveReader : public Common::File {
public:
    ArchiveReader();
    ~ArchiveReader();
    
    void openArchive(const char *filename);
    
    // Returns the size of the opened resource
    uint32 openResource(uint resIndex);
    // Closes the resource
    void closeResource();
    // Returns the size of the resource
    uint32 getResourceSize(uint resIndex);

    void dump(uint resIndex, const char *prefix = NULL);
    
protected:
	uint32 _offsets[10000];

};

class ResourceCache {
public:
    ResourceCache(PictureEngine *vm);
    ~ResourceCache();
    
    byte *load(uint resIndex);
    uint32 getCurItemSize() const { return _curItemSize; }

protected:

	struct CacheItem {
	    uint resIndex;
	    //int value;	// what is this?
	    int32 offset;	// offset into _base
	    uint32 size;    // size of the item
	};

    PictureEngine *_vm;
    
	byte *_base;
	uint32 _bytesUsed;
	uint32 _curItemOffset, _curItemSize;
	
	CacheItem _cache[kMaxCacheItems];
	uint _cacheCount;
	
	bool existsItem(uint resIndex);
	byte *addItem(uint resIndex, uint32 size);
	void checkCapacity(uint32 size);

};


} // End of namespace Picture

#endif /* PICTURE_H */
