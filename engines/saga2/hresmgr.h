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

#include "saga2/rmem.h"
#include "saga2/ioerrors.h"

namespace Saga2 {


#define USE_MEMORY_MAPPED_FILES 0

#if defined(_WIN32) && USE_MEMORY_MAPPED_FILES
#define HR_FILE MMFILE
#define HR_OPEN mmfopen
#define HR_CLOSE mmfclose
#define HR_SEEK mmfseek
#define HR_READ mmfread
#else
#define HR_FILE FILE
#define HR_OPEN ftaopen
#define HR_CLOSE ftaclose
#define HR_SEEK ftaseek
#define HR_READ ftaread
#endif


#ifdef _WIN32   //  Set structure alignment packing value to 1 byte
#pragma pack( push, 1 )
#endif

/* ===================================================================== *
   Constants and common types
 * ===================================================================== */

typedef uint32          hResID;

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
	RHANDLE        *_data; // allocated array of handles
	Common::File    _file;
	Common::File   *_handle;
	uint32          _bytecount;
	uint32          _bytepos;

	hResEntry      *findEntry(hResID id, RHANDLE **capture = NULL);
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
	RHANDLE     load(hResID id, const char [], bool async = false, bool cacheable = true);
	byte       *loadResource(hResID id, Common::String filename, const char desc[]);
	byte       *loadIndexResource(int16 index, const char desc[], Common::String filename);
	RHANDLE     loadIndex(int16 index, const char[], bool cacheable = true);
	void        release(RHANDLE p);
	Common::File     *resFileHandle(void) {
		return _handle;
	}

	RHANDLE     dataHandle(int16 index) {
		return _data[ index ];
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
	hResource(char *resname, char *extname, const char []);
	~hResource();

	hResContext *newContext(hResID id, const char []);
	void        disposeContext(hResContext *con);
	void        readResource(hResEntry &element);
};

/* ===================================================================== *
   Resource Type Macros
 * ===================================================================== */

#if 0
#define RES_ID(a,b,c,d) ((uint32)(a) | (uint32)(b) << 8 | \
                         (uint32)(c) << 16 | (uint32)(d) << 24)

#else
inline hResID RES_ID(uint8 a, uint8 b, uint8 c, uint8 d) {
	return ((uint32)(a) | (uint32)(b) << 8 | (uint32)(c) << 16 | (uint32)(d) << 24);
}
#endif

#define HRES_ID         MKTAG('H','R','E','S')


//  Handle-washing function.
void washHandle(RHANDLE &handle);

#ifdef _WIN32   //  Set structure alignment packing value to 1 byte
#pragma pack( pop )
#endif

} // end of namespace Saga2

#endif
