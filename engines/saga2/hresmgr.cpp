/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/debug.h"

#include "saga2/saga2.h"
#include "saga2/hresmgr.h"
#include "saga2/fta.h"

namespace Saga2 {

/* ===================================================================== *
   Resource context
 * ===================================================================== */

hResContext::hResContext() {
	_valid = false;
	_base = nullptr;
	_parent = nullptr;
	_numEntries = 0;
	_handle = &_file;
	_res = nullptr;
	_bytecount = _bytepos = 0;
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
	_base = nullptr;
	_parent = nullptr;

	if (!_res->_valid)
		return;

	_parent = sire;

	debugC(3, kDebugResources, "Creating context %x (%s), %s", id, tag2str(id), desc);
	if ((entry = _parent->findEntry(id)) == nullptr) {
		debugC(3, kDebugResources, "Could not create context");
		return;
	}

	_numEntries = entry->size / resourceSize;

	_base = (hResEntry *)((uint8*)_res->_table + entry->offset - _res->_firstGroupOffset);
	debugC(3, kDebugResources, "- _numEntries = %d, _base = %p, entry->offset = %d",
	                            _numEntries, (void *)_base, entry->offset);

	_valid = true;
}

hResContext::~hResContext() {
	releaseIndexData();
}

hResEntry *hResContext::findEntry(hResID id) {
	hResEntry       *entry;
	int16           i;

	_bytecount = 0;
	_bytepos = 0;
	if (!_valid) return nullptr;

	debugC(3, kDebugResources, "findEntry: looking for %x (%s)", id, tag2str(id));
	for (i = 0, entry = _base; i < _numEntries; i++, entry++) {
		debugC(3, kDebugResources, "%d: Trying ID: %x (%s)", i, entry->id, tag2str(entry->id));
		if (entry->id == id) {
			debugC(3, kDebugResources, "findEntry: found %x (%s)", entry->id, tag2str(entry->id));
			return entry;
		}
	}
	debugC(3, kDebugResources, "findEntry: No entry found");

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

uint32 hResContext::count() {
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

// this function sets _handle

bool hResContext::seek(hResID id) {
	hResEntry   *entry;

	_bytecount = 0;
	_bytepos = 0;
	if (!_valid)
		return false;

	if ((entry = findEntry(id)) == nullptr)
		return false;

	_bytecount = entry->size;
	_bytepos = entry->resOffset();

	_res->_handle->seek(_bytepos, SEEK_SET);

	if (entry->isExternal()) {
		error("hResContext: External entries are not supported");
	}

	_handle = _res->_handle;

	return true;
}

void hResContext::rest() {
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

bool hResContext::eor() {
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

uint32 hResContext::getSize(hResID id, const char desc[]) {
	hResEntry *entry;

	if ((entry = findEntry(id)) == nullptr) {
		warning("Resource %d, %s not found", id, desc);
		return 0;
	}
	debugC(3, kDebugResources, "Size for %x (%s): %d", id, desc, entry->size);

	return entry->size;
}

byte *hResContext::loadResource(hResID id, const char desc[], Common::String filename) {
	hResEntry *entry;

	debugC(3, kDebugResources, "Loading resource %x (%s)", id, desc);
	if ((entry = findEntry(id)) == nullptr) {
		warning("Resource %d, %s not found", id, desc);
		return nullptr;
	}

	byte *res = (byte*)malloc(entry->size);

	if (filename.equalsIgnoreCase(""))
		filename = _filename;

	if (!_file.isOpen())
		_file.open(filename);

	_file.seek(entry->offset, SEEK_SET);
	_file.read(res, entry->size);

	return res;
}

byte *hResContext::loadIndexResource(int16 index, const char desc[], Common::String filename) {
	hResEntry *entry;
	entry = &_base[index];

	debugC(5, kDebugResources, "Loading indexed resource: %d (%s)", index, desc);

	if (!_valid || entry == nullptr)
		return nullptr;

	if (_indexData.contains(index))
		return _indexData.getVal(index);

	byte *res = (byte*)malloc(entry->size);

	if (res == nullptr) {
		debugC(5, kDebugResources, "Could not allocate resources");
		return nullptr;
	}

	debugC(5, kDebugResources, "_indexData: pushing (%d, %p)", index, (void*)res);
	_indexData.setVal(index, res);

	if (filename.equalsIgnoreCase(""))
		filename = _filename;

	if (!_file.isOpen())
		_file.open(filename);

	_file.seek(entry->offset, SEEK_SET);
	_file.read(res, entry->size);

	return res;
}

void hResContext::releaseIndexData() {
	debugC(4, kDebugResources, "releaseIndexData():");
	for (DataMap::iterator i = _indexData.begin(); i != _indexData.end(); ++i) {
		debugC(4, kDebugResources, "... %d, %p", i->_key, (void*)i->_value);
		if (i->_value) {
			free(i->_value);
			i->_value = nullptr;
			_indexData.erase(i);
		}
	}
}

/* ===================================================================== *
   Resource file
 * ===================================================================== */

void hResource::readEntry(hResEntry &element) {
	element.id = _file.readUint32BE();
	element.offset = _file.readUint32LE();
	element.size = _file.readUint32LE();
	uint32 id = element.id;

	debugC(2, kDebugResources, "%s, offset: %x, size: %d", tag2str(id), element.offset, element.size);
}

void hResource::readResource(hResEntry &element) {
	element.id = _file.readUint32BE();
	element.offset = _file.readUint32LE();
	element.size = _file.readUint32LE();
	uint32 id = element.id;

	debugC(3, kDebugResources, "%s, offset: %x, size: %d", tag2str(id), element.offset, element.size);
}

hResource::hResource(const char *resname) {
	hResEntry   origin;
	int32      tableSize;
	const int32 resourceSize = 4 + 4 + 4; // id, offset, size

	_valid = false;
	_base = nullptr;
	_parent = nullptr;
	_table = nullptr;
	_firstGroupOffset = 0;
	_numEntries = 0;
	_filename = resname;

	debugC(1, kDebugResources, "Opening resource: %s", resname);
	if (!_file.open(resname))
		warning("Unable to open file %s", resname);

	readResource(origin);
	if (origin.id != HRES_ID)
		return;

	_file.seek(origin.offset - sizeof(uint32), SEEK_SET);
	_firstGroupOffset = _file.readUint32LE();

	// allocate buffers for root, groups and data

	_numEntries = origin.size / resourceSize;

	_base = new hResEntry[_numEntries]();
	tableSize = origin.offset - _firstGroupOffset - sizeof(uint32);
	_table = new hResEntry[tableSize / resourceSize]();

	if (_base == nullptr || _table == nullptr)
		return;

	debugC(1, kDebugResources, "Reading %d entries:", _numEntries);
	for (int i = 0; i < _numEntries; ++i)
		readEntry(_base[i]);

	debugC(1, kDebugResources, "Reading %d groups:", tableSize / resourceSize);
	_file.seek(_firstGroupOffset, SEEK_SET);
	for (int i = 0; i < tableSize / resourceSize; ++i) {
		readResource(_table[i]);
	}

	_res = this;
	_valid = true;
}

hResource::~hResource() {
	if (_base)
		delete[] _base;
	if (_table)
		delete[] _table;
	//if (_handle) free(_handle);
}

hResContext *hResource::newContext(hResID id, const char desc[]) {
	hResContext *result;

	result = new hResContext(this, id, desc);
	if (result == nullptr || !result->_valid) {
		error("Error accessing resource group.");
	}
	result->_filename = _filename;
	return result;
}

void hResource::disposeContext(hResContext *con) {
	if (con) delete con;
}

} // end of namespace Saga2
