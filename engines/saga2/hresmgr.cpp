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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/saga2.h"
#include "saga2/rmemfta.h"
#include "saga2/hresmgr.h"
#include "saga2/fta.h"
#include "common/debug.h"

namespace Saga2 {

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
	_valid = false;
	_base = nullptr;
	_parent = nullptr;
	_data = nullptr;
	_numEntries = 0;
	_handle = &_file;
}

hResContext::hResContext(hResContext *sire, hResID id, const char desc[]) {
	hResEntry   *entry;
	const uint32 resourceSize = 12;

	_valid = false;
	_res = sire->_res;
	_numEntries = 0;
	_bytecount = 0;
	_bytepos = 0;
	_handle = &_file;

	if (!_res->_valid)
		return;

	_parent = sire;

	if ((entry = _parent->findEntry(id)) == nullptr)
		return;

	_numEntries = entry->size / resourceSize;

	_base = &_res->_table[entry->offset - _res->_firstGroupOffset];

	_data = new RHANDLE[_numEntries]();
	if (_data == nullptr)
		return;

	_valid = true;
}

hResContext::~hResContext() {
	if (_data) {
		delete[] _data;
		_data = nullptr;
	}
}

hResEntry *hResContext::findEntry(hResID id, RHANDLE **capture) {
	hResEntry       *entry;
	int16           i;

	_bytecount = 0;
	_bytepos = 0;
	if (!_valid) return nullptr;

	debugC(2, kDebugResources, "findEntry: looking for %x (%s)", id, tag2str(id));
	for (i = 0, entry = _base; i < _numEntries; i++, entry++) {
		debugC(2, kDebugResources, "%d: Trying ID: %x (%s)", i, entry->id, tag2str(entry->id));
		if (entry->id == id) {
			if (capture) *capture = &_data[ i ];
			debugC(2, kDebugResources, "findEntry: found %x (%s)", entry->id, tag2str(entry->id));
			return entry;
		}
	}
	debugC(2, kDebugResources, "findEntry: No entry found");

	return nullptr;
}

uint32 hResContext::size(hResID id) {
	hResEntry   *entry;

	if (!_valid)
		return 0;

	if ((entry = findEntry(id)) == nullptr)
		return 0;

	return entry->size;
}

uint32 hResContext::count(void) {
	return _numEntries;
}

uint32 hResContext::count(hResID id) {
	uint32          count = 0;
	hResEntry       *entry;
	int16           i;

	_bytecount = 0;
	_bytepos = 0;
	if (!_valid) return 0;

	for (i = 0, entry = _base; i < _numEntries; i++, entry++) {
		if ((entry->id & 0x00ffffff) == (id & 0x00ffffff)) count++;
	}

	return count;
}

Common::File *hResContext::openExternal(Common::File *fh) {
	char    name[160];
	int     len;
	uint16  size;
	Common::File *file;
	file = &_file;

	_bytecount = 0;
	_bytepos = 0;
	strcpy(name, _res->_externalPath);
	len = strlen(name);
	size = fh->readUint32LE();
	fh->read(&name, sizeof(name));
	name[len + size] = 0;
	file->open(name);
	return file;
}

// this function sets _handle

bool hResContext::seek(hResID id) {
	hResEntry   *entry;

	_bytecount = 0;
	_bytepos = 0;
	if (!_valid) return false;

	if ((entry = findEntry(id)) == nullptr) return false;

	_bytecount = entry->size;
	_bytepos = entry->resOffset();

	_res->_handle->seek(_bytepos, SEEK_SET);

	if (entry->isExternal()) {
		// resource _data is actually a path name

		_handle = openExternal(_res->_handle);
		return (_handle != nullptr);
	}

	_handle = _res->_handle;

	return true;
}

void hResContext::rest(void) {
	_bytecount = 0;
	_bytepos = 0;
	if (_valid && _handle && _handle != _res->_handle) {
		_handle->close();
		_handle = nullptr;
	}
}

bool hResContext::read(void *buffer, uint32 size) {
	assert(_handle);
	if (!_valid) return false;
	_bytecount = 0;
	_bytepos = 0;
	return (_handle->read(buffer, size) != 0);
}

bool hResContext::eor(void) {
	return (_bytecount < 1);
}

uint32 hResContext::readbytes(void *buffer, uint32 size) {
	int32 bytesRead;
	if (!_valid || _bytecount < 1) return 0;

	_res->_handle->seek(_bytepos, SEEK_SET);
	bytesRead = _handle->read(buffer, MIN(size, _bytecount));
	_bytecount -= bytesRead;
	_bytepos += bytesRead;
	return bytesRead;
}

bool hResContext::skip(uint32 amount) {
	if (!_valid) return false;

	_res->_handle->seek(amount, SEEK_CUR);
	_bytecount -= amount;
	_bytepos -= amount;
	return true;
}

bool hResContext::get(hResID id, void *buffer, uint32 size) {
	bool    result = false;

	if (!_valid) return false;
	_bytecount = 0;
	_bytepos = 0;

	if (size == NATURAL_SIZE) {
		hResEntry   *entry;

		if ((entry = findEntry(id)) == nullptr) return false;

		size = entry->size;
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

	if (!_valid)
		return nullptr;
	_bytecount = 0;
	_bytepos = 0;

	if ((entry = findEntry(id, &capture)) == nullptr)
		return nullptr;

	if (*capture != nullptr && **capture != nullptr) {
		entry->use();
	} else {
		if (*capture == nullptr)
			*capture = (RHANDLE)malloc(entry->size);

		if (*capture == nullptr) return nullptr;

		//  If it's an external resource, then load synchronously

#ifdef WINKLUDGE
		async = false;
#endif
		if (entry->isExternal() || async == false) {
			if (seek(id) && read(**capture, entry->size)) {
				entry->use();
			} else {
				free(*capture);
				*capture = nullptr;
			}

			rest();
		} else {
#ifndef WINKLUDGE
			RequestResource(*capture,
							entry->offset,
							entry->size);
#endif
			entry->use();
		}
	}

	return (*capture);
}

RHANDLE hResContext::loadIndex(int16 index, const char desc[], bool cacheable) {
	hResEntry   *entry;
	RHANDLE     *capture; //, _handle;

	if (!_valid)
		return nullptr;
	_bytecount = 0;
	_bytepos = 0;

	entry = &_base[ index ];
	capture = &_data[ index ];

	if (*capture != nullptr && **capture != nullptr) {
		entry->use();
	} else {
		if (*capture == nullptr)
			*capture = (RHANDLE)malloc(entry->size);

		if (*capture == nullptr) return nullptr;

		_res->_handle->seek(entry->resOffset(), SEEK_SET);

		if (read(**capture, entry->size) == false) {
			free(*capture);
			*capture = nullptr;
		}
		entry->use();
		rest();
	}
	return (*capture);
}

void hResContext::release(RHANDLE p) {
	_bytecount = 0;
	_bytepos = 0;

	hResEntry   *entry;
	RHANDLE     *d;

	if (_valid && p != nullptr) {
		entry = _base;
		d = _data;

		while (entry->id != BAD_ID) {
			if ((RHANDLE)p == *d) {

				entry->abandon();
				if (!entry->isUsed()) {
					RDisposeHandle(p);
					*d = nullptr;
				}
				return;
			}
			entry++;
			d++;
		}
	}
}

/* ===================================================================== *
   Resource file
 * ===================================================================== */

void hResource::readResource(hResEntry &element) {
	element.id = _file.readUint32BE();
	element.offset = _file.readUint32LE();
	element.size = _file.readUint32LE();
	uint32 id = element.id;

	debugC(3, kDebugResources, "%s, offset: %x, size: %d", tag2str(id), element.offset, element.size);
}

hResource::hResource(char *resname, char *extname, const char desc[]) {
	hResEntry   origin;
	int32      tableSize;
	const int32 resourceSize = 4 + 4 + 4; // id, offset, size

	_valid = false;
	_base = nullptr;
	_parent = nullptr;
	_data = nullptr;
	_numEntries = 0;

	strncpy(_externalPath, extname ? extname : "", EXTERNAL_PATH_SIZE);

	debugC(1, kDebugResources, "Opening resource: %s", resname);
	_file.open(resname);

	readResource(origin);
	if (origin.id != HRES_ID)
		return;

	_file.seek(origin.offset - sizeof(uint32), SEEK_SET);
	_firstGroupOffset = _file.readUint32LE();

	// allocate buffers for root, groups and data

	_numEntries = origin.size;

	_base = new hResEntry[_numEntries]();
	tableSize = origin.offset - _firstGroupOffset - sizeof(uint32);
	_table = new hResEntry[tableSize / resourceSize]();


	if (_base == nullptr || _table == nullptr) return;

	debugC(kDebugResources, "Reading %d categories:", _numEntries);
	for (int i = 0; i < _numEntries; ++i)
		readResource(_base[i]);

	debugC(kDebugResources, "Reading %d groups:", tableSize / resourceSize);
	_file.seek(_firstGroupOffset, SEEK_SET);
	for (int i = 0; i < tableSize / resourceSize; ++i) {
		readResource(_table[i]);
	}

	_res = this;
	_valid = true;
}

hResource::~hResource() {
	if (_base) delete[] _base;
	if (_table) delete[] _table;
	//if (_handle) free(_handle);
}

hResContext *hResource::newContext(hResID id, const char desc[]) {
	hResContext *result;

	result = new hResContext(this, id, desc);
	if (result == nullptr || !result->_valid) {
		error("Error accessing resource group.");
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
//	set the actual handle pointer to nullptr.

void washHandle(RHANDLE &handle) {
	if (handle != nullptr && *handle == nullptr) {
		RDisposeHandle(handle);
		handle = nullptr;
	}
}

} // end of namespace Saga2
