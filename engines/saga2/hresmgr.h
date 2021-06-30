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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_HRESMGR_H
#define SAGA2_HRESMGR_H

#include "common/file.h"

namespace Saga2 {


#define USE_MEMORY_MAPPED_FILES 0

/* ===================================================================== *
   Constants and common types
 * ===================================================================== */

typedef uint32          hResID;
typedef Common::HashMap<int16, byte*> DataMap;

#define BAD_ID          ((hResID)0xFFFFFFFFL)
#define NATURAL_SIZE    ((hResID)0xFFFFFFFFL)

struct hResEntry;
class hResContext;
class hResource;

#define EXTERNAL_PATH_SIZE 80

/* ===================================================================== *
   hResEntry struct
 * ===================================================================== */

struct hResEntry {
	hResID              id;                 // id of this entry or BAD_ID
	uint32              offset;             // offset in file
	uint32              size;               // size in file
public:

	hResEntry()         {
		id = BAD_ID;
		size = 0;
		offset = 0;
	}

	void                use(void) {
		size += 0x01000000L;
	}
	void                abandon(void) {
		size -= 0x01000000L;
	}
	uint8               useCount(void) {
		return size >> 24;
	}
	bool                isUsed(void) {
		return ((size & 0xFF000000L) != 0L);
	}

	bool                isExternal(void) {
		return ((offset & (1L << 31)) != 0L);
	}

	uint32              resOffset(void) {
		return (offset & 0x0FFFFFFFL);
	}
	uint32              resSize(void) {
		return (size & 0x00FFFFFF);
	}
};



/* ===================================================================== *
   Resource Context
 * ===================================================================== */


class hResContext {

protected:
	uint16         _numEntries;
	hResource      *_res;
	hResContext    *_parent;
	hResEntry      *_base;
	DataMap        _indexData; // allocated array of handles
	Common::File    _file;
	Common::File   *_handle;
	uint32          _bytecount;
	uint32          _bytepos;

	hResEntry      *findEntry(hResID id);
	Common::File   *openExternal(Common::File *fh);

public:
	bool            _valid;
	Common::String  _filename;

	hResContext();
	hResContext(hResContext *sire, hResID id, const char []);
	~hResContext();

	uint32      getResID(void) {
		return _base->id;
	}

	uint32      size(hResID id);
	uint32      count(void);
	uint32      count(hResID id);
	bool        seek(hResID id);
	void        rest(void);
	uint32          readbytes(void *buffer, uint32 size);
	bool        eor(void);
	inline size_t   bytesleft(void) {
		return _bytecount;
	}

	bool        read(void *buffer, uint32 size);
	bool        skip(uint32 amount);
	bool        get(hResID id, void *buffer, uint32 size);
	uint32       getSize(hResID id, const char desc[]);
	byte       *loadResource(hResID id, const char desc[], Common::String filename = "");
	byte       *loadIndexResource(int16 index, const char desc[], Common::String filename = "");
	void        releaseIndexData();
	Common::File     *resFileHandle(void) {
		return _handle;
	}
};

/* ===================================================================== *
   Resource file
 * ===================================================================== */

class hResource : public hResContext {

	friend class hResContext;

	uint32          _firstGroupOffset;
	char            _externalPath[EXTERNAL_PATH_SIZE];
	hResEntry      *_table;

public:
	hResource(const char *resname, const char *extname, const char []);
	~hResource();

	hResContext *newContext(hResID id, const char []);
	void        disposeContext(hResContext *con);
	void        readEntry(hResEntry &element);
	void        readResource(hResEntry &element);
};

#define HRES_ID         MKTAG('H','R','E','S')


} // end of namespace Saga2

#endif
