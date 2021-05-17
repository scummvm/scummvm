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

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/errclass.h"

#include "saga2/hresmgr.h"

#include "saga2/fta.h"

namespace Saga2 {

#if DEBUG
extern char *idname(long s);
#endif

/* ===================================================================== *
   Calls which do disk access
    bool hResContext::seek(hResID id)
      fseek
    void hResContext::rest(void)
      fclose
    bool hResContext::read(void *buffer, int32 size)
      fread
    uint32 hResContext::readbytes(void *buffer, uint32 size)
      fseek
      fread
    bool hResContext::skip(int32 amount)
      fseek
    RHANDLE hResContext::loadIndex( int16 index, const char desc[], bool cacheable )
      fseek
    hResource::hResource(char *resname, char *extname, const char desc[])
      fopen
      fseek
      fread
    hResource::~hResource()
      fclose

   Calls which deal with external files
    HR_FILE *hResContext::openExternal(HR_FILE *fh)
 * ===================================================================== */
/* ===================================================================== *
   Resource context
 * ===================================================================== */

hResContext::hResContext() {
#if DEBUG
	strncpy(description, "NULL context", 32);
#endif
	valid = FALSE;
	base = NULL;
	parent = NULL;
	data = NULL;
	numEntries = 0;
}

hResContext::hResContext(hResContext *sire, hResID id, const char desc[]) {
	hResEntry   *entry;
#if DEBUG
	strncpy(description, desc, 32);
#endif

	valid = FALSE;
	res = sire->res;
	numEntries = 0;
	bytecount = 0;
	bytepos = 0;

	if (!res->valid) return;

	parent = sire;

	if ((entry = parent->findEntry(id)) == NULL) return;

	numEntries = entry->resSize() / sizeof * entry;

	base = (hResEntry *)((uint8 *)res->groups +
	                     (entry->offset - res->firstGroupOffset));

	data = (RHANDLE *)RNewClearPtr(numEntries * sizeof(RHANDLE), NULL, desc);
	if (data == NULL) return;

	valid = TRUE;
}

hResContext::~hResContext() {
	if (data) {
		if (valid) {
			for (int i = 0; i < numEntries; i++) {
				RDisposeHandle(data[ i ]);
				data[i] = NULL;
			}
		}
		RDisposePtr(data);
		data = NULL;
	}
}

hResEntry *hResContext::findEntry(hResID id, RHANDLE **capture) {
	hResEntry       *entry;
	int16           i;

	bytecount = 0;
	bytepos = 0;
	if (!valid) return NULL;

	for (i = 0, entry = base; i < numEntries; i++, entry++) {
		if (entry->id == id) {
			if (capture) *capture = &data[ i ];
			return entry;
		}
	}

	return NULL;
}

uint32 hResContext::size(hResID id) {
	hResEntry   *entry;

	if (!valid) return 0;

	if ((entry = findEntry(id)) == NULL) return 0;

	return entry->resSize();
}

uint32 hResContext::count(void) {
	return numEntries;
}

uint32 hResContext::count(hResID id) {
	uint32          count = 0;
	hResEntry       *entry;
	int16           i;

	bytecount = 0;
	bytepos = 0;
	if (!valid) return 0;

	for (i = 0, entry = base; i < numEntries; i++, entry++) {
		if ((entry->id & 0x00ffffff) == (id & 0x00ffffff)) count++;
	}

	return count;
}

HR_FILE *hResContext::openExternal(HR_FILE *fh) {
	char    name[160];
	int     len;
	uint16  size;

	bytecount = 0;
	bytepos = 0;
	strcpy(name, res->externalPath);
	len = strlen(name);
	if (HR_READ(&size, sizeof size, 1, fh) != 1) return NULL;
	HR_READ(&name[len], size, 1, fh);
	name[len + size] = 0;
	return HR_OPEN(name, "rb");
}

// this function sets handle

bool hResContext::seek(hResID id) {
	hResEntry   *entry;

	bytecount = 0;
	bytepos = 0;
	if (!valid) return FALSE;

	if ((entry = findEntry(id)) == NULL) return FALSE;

	bytecount = entry->resSize();
	bytepos = entry->resOffset();

	if (HR_SEEK(res->handle, bytepos, SEEK_SET) != 0)
		error("Error seeking resource file:\n");

	if (entry->isExternal()) {
		// resource data is actually a path name

		handle = openExternal(res->handle);
		return (handle != NULL);
	}

	handle = res->handle;

	return TRUE;
}

void hResContext::rest(void) {
	bytecount = 0;
	bytepos = 0;
	if (valid && handle && handle != res->handle) {
		HR_CLOSE(handle);
		handle = NULL;
	}
}

bool hResContext::read(void *buffer, int32 size) {
	if (!valid) return FALSE;
	bytecount = 0;
	bytepos = 0;
	return (HR_READ(buffer, size, 1, handle) == 1);
}

bool hResContext::eor(void) {
	return (bytecount < 1);
}

uint32 hResContext::readbytes(void *buffer, uint32 size) {
	int32 bytesRead;
	if (!valid || bytecount < 1) return 0;

	if (HR_SEEK(res->handle, bytepos, SEEK_SET) != 0)
		error("Error seeking resource file:");
	bytesRead = HR_READ(buffer, 1, MIN(size, bytecount), handle);
	bytecount -= bytesRead;
	bytepos += bytesRead;
	return bytesRead;
}

bool hResContext::skip(int32 amount) {
	if (!valid) return FALSE;

	HR_SEEK(res->handle, amount, SEEK_CUR);
	bytecount -= amount;
	bytepos -= amount;
	return TRUE;
}

bool hResContext::get(hResID id, void *buffer, int32 size) {
	bool    result = FALSE;

	if (!valid) return FALSE;
	bytecount = 0;
	bytepos = 0;

	if (size == NATURAL_SIZE) {
		hResEntry   *entry;

		if ((entry = findEntry(id)) == NULL) return FALSE;

		size = entry->resSize();
	}

	if (seek(id)) {
		result = read(buffer, size);
		rest();
	}

	return result;
}


RHANDLE hResContext::load(hResID id, const char desc[], bool async, bool cacheable) {
	hResEntry   *entry;
	RHANDLE     *capture;

	if (!valid) return FALSE;
	bytecount = 0;
	bytepos = 0;

	if ((entry = findEntry(id, &capture)) == NULL) return FALSE;

	if (*capture != NULL && **capture != NULL) {
		RLockHandle(*capture);

		entry->use();
	} else {
		if (*capture == NULL)
			*capture = RNewHandle(entry->resSize(), NULL, desc);
		else {
			if (RAllocHandleData(*capture, entry->resSize(), desc) == NULL)
				return NULL;
		}

		if (*capture == NULL) return NULL;

		RLockHandle(*capture);
		if (cacheable)
			RCacheHandle(*capture);

		//  If it's an external resource, then load synchronously

#ifdef WINKLUDGE
		async = FALSE;
#endif
		if (entry->isExternal() || async == FALSE) {
			if (seek(id) && read(**capture, entry->resSize())) {
				entry->use();
			} else {
				RDisposeHandle(*capture);
				*capture = NULL;
			}

			rest();
		} else {
#ifndef WINKLUDGE
			RequestResource(
			    *capture,
			    entry->resOffset(),
			    entry->resSize());
#endif
			entry->use();
		}
	}

	return (*capture);
}

RHANDLE hResContext::loadIndex(int16 index, const char desc[], bool cacheable) {
	hResEntry   *entry;
	RHANDLE     *capture; //, handle;

	if (!valid) return FALSE;
	bytecount = 0;
	bytepos = 0;

	entry = &base[ index ];
	capture = &data[ index ];

	if (*capture != NULL && **capture != NULL) {
		RLockHandle(*capture);
		entry->use();
	} else {
		if (*capture == NULL)
			*capture = RNewHandle(entry->resSize(), NULL, desc);
		else
			RAllocHandleData(*capture, entry->resSize(), desc);

		if (*capture == NULL) return NULL;

		RLockHandle(*capture);
		if (cacheable)
			RCacheHandle(*capture);

		HR_SEEK(res->handle, entry->resOffset(), SEEK_SET);

		if (read(**capture, entry->resSize()) == FALSE) {
			RDisposeHandle(*capture);
			*capture = NULL;
		}
		entry->use();
		rest();
	}
	return (*capture);
}

void hResContext::release(RHANDLE p) {
//	if (p) RUnlockHandle( p );               // make movable, cacheable
	bytecount = 0;
	bytepos = 0;

	hResEntry   *entry;
	RHANDLE     *d;

	if (valid && p != NULL) {
		entry = base;
		d = data;

		while (entry->id != BAD_ID) {
			if ((RHANDLE)p == *d) {
#if DEBUG
				if (entry->useCount() > 32)
					throw gError("Suspicious usage count!\n");
#endif

				entry->abandon();
				if (!entry->isUsed()) {
					RDisposeHandle(p);
					*d = NULL;
				}
				return;
			}
			entry++;
			d++;
		}

//#define fatal     throw gError

//		fatal( "RESOURCE RELEASE PROBLEM\n" );

	}
}

/* ===================================================================== *
   Resource file
 * ===================================================================== */

hResource::hResource(char *resname, char *extname, const char desc[]) {
	hResEntry   origin;
	uint32      size;
#if DEBUG
	strncpy(description, desc, 32);
#endif

	valid = FALSE;
	base = NULL;
	parent = NULL;
	data = NULL;
	numEntries = 0;

	strncpy(externalPath, extname ? extname : "", EXTERNAL_PATH_SIZE);

	if ((handle = HR_OPEN(resname, "rb")) == NULL) return;

	if (HR_READ(&origin, sizeof origin, 1, handle) != 1) return;

	if (origin.id != HRES_ID) return;

	HR_SEEK(handle, origin.offset - sizeof(uint32), SEEK_SET);
	if (HR_READ(&firstGroupOffset, sizeof firstGroupOffset, 1, handle) != 1) {
		return;
	}

	// allocate buffers for root, groups and data

	base = (hResEntry *)RNewPtr(origin.resSize(), NULL, desc);
	size = origin.offset - firstGroupOffset - sizeof(uint32);
	groups = (hResEntry *)RNewPtr(size, NULL, desc);

	if (base == NULL || groups == NULL) return;

	if (HR_READ(base, origin.resSize(), 1, handle) != 1) return;
	HR_SEEK(handle, firstGroupOffset, SEEK_SET);
	if (HR_READ(groups, size, 1, handle) != 1) return;

	res = this;
	numEntries = origin.resSize() / sizeof origin;
	valid = TRUE;
}

hResource::~hResource() {
	if (base) RDisposePtr(base);
	if (groups) RDisposePtr(groups);
	if (handle) HR_CLOSE(handle);
}

hResContext *hResource::newContext(hResID id, const char desc[]) {
	hResContext *result;

	result = NEW_PRES hResContext(this, id, desc);
	if (result == NULL || !result->valid) {
#if DEBUG
		error("Error accessing resource group %s.", idname(id));
#else
		error("Error accessing resource group.");
#endif
	}
	return result;
}

void hResource::disposeContext(hResContext *con) {
	if (con) delete con;
}


/* ===================================================================== *
   Assorted functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	For handles which have been purged, but the handle structure is
//	still hanging around, we can free the handle structure and
//	set the actual handle pointer to NULL.

void washHandle(RHANDLE &handle) {
	if (handle != NULL && *handle == NULL) {
		RDisposeHandle(handle);
		handle = NULL;
	}
}

} // end of namespace Saga2
