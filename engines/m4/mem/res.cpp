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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/textconsole.h"
#include "m4/mem/res.h"
#include "m4/mem/memman.h"
#include "m4/mem/reloc.h"
#include "m4/globals.h"

namespace M4 {

#define	PRIME           8179
#define	HASHSIZE        MAX_RESOURCES
#define	FULLY_BUFFERED  1
#define MARKED_PURGE    0x80

Resources::~Resources() {
	delete _fp;
	// TODO: Delete resource entries
}


Resources::Entry *Resources::findAndSetResEntry(const Common::String &resourceName) {
	int orig_hash_val;
	int hash_val;
	Entry *res = nullptr;

	Common::String resName = resourceName;
	resName.toLowercase();
	hash_val = hash(resName);

	// If empty slot at this hash, then we're done
	if (!_resources[hash_val].Flags)
		goto got_one;

	// Flags is set, so scan until Flags is clear, or the resource name strings match
	orig_hash_val = hash_val;

	while ((_resources[hash_val].Flags & FULLY_BUFFERED)
			&& !resName.equalsIgnoreCase(_resources[hash_val].name)) {
		// if we searched every entry to no avail:
		if ((hash_val = ++hash_val & (HASHSIZE - 1)) == orig_hash_val)
			goto test4;
	}

	goto got_one;

test4:
	hash_val = orig_hash_val;
	while (!(_resources[hash_val].Flags & MARKED_PURGE))
		// if we searched every entry to no avail:
		if ((hash_val = ++hash_val & (HASHSIZE - 1)) == orig_hash_val) {
			error("Out of resource space");
		}

	res = &_resources[hash_val];
	delete[] res->RHandle;
	res->RHandle = nullptr;

got_one:
	res = &_resources[hash_val];
	res->name = resName;
	res->Flags = FULLY_BUFFERED;

	return res;
}

int Resources::hash(const Common::String &sym) const {
	int	ret_val = 0;

	const char *s = sym.c_str();
	while (*s) {
		ret_val += *s++;
		ret_val *= PRIME;
	}

	return ret_val & (HASHSIZE - 1);
}

Handle Resources::rget(const Common::String &resourceName, int32 *resourceSize) {
	Entry *resEntry;

	if (resourceSize)
		*resourceSize = 0;

	if (!(resEntry = findAndSetResEntry(resourceName))) {
		term_message("rgetting:%s  -> failed!", resourceName.c_str());
		return nullptr;
	}

	// Check if resource is fully buffered
	// All resources are currently fully buffered!
	if (!(resEntry->Flags & FULLY_BUFFERED)) {
		term_message("rgetting:%s  -> failed!", resourceName.c_str());
		return nullptr;
	}

	// If there's a handle with some memory in it
	// Resource is cached; unmark purge flag and return handle
	if (resEntry->RHandle && *resEntry->RHandle) {
		if (resourceSize)
			*resourceSize = resEntry->Size;

		HNoPurge(resEntry->RHandle);
		resEntry->Flags &= ~MARKED_PURGE;
		term_message("rgetting:%s  -> from memory", resourceName.c_str());
		return resEntry->RHandle; 
	}

	// Check if size known
	if ((resEntry->Size = get_file(resEntry->name)) == -1)
		error("Error getting entry %s", resEntry->name.c_str());

	// Check if buffer size	is set
	if (resEntry->BufferSize != resEntry->Size)
		resEntry->BufferSize = resEntry->Size;

	// Check if resource handle allocated
	if (!resEntry->RHandle)
		if (!(resEntry->RHandle = MakeNewHandle(resEntry->BufferSize, resEntry->name.c_str())))
			error("rgetting: %s  -> failed", resEntry->name.c_str());

	// Check if resource handle has valid memory block allocated to	it
	if (!*resEntry->RHandle)
		if (!mem_ReallocateHandle(resEntry->RHandle, resEntry->BufferSize, resEntry->name.c_str())) {
			if (MakeMem(resEntry->BufferSize, resEntry->name.c_str())) {
				if (!mem_ReallocateHandle(resEntry->RHandle, resEntry->BufferSize, resEntry->name.c_str())) {
					term_message("rgetting:%s  -> failed!", resourceName.c_str());
					return nullptr;
				}
			} else {
				term_message("rgetting:%s  -> failed!", resourceName.c_str());
				return nullptr;
			}
		}

	if (!do_file(resEntry->RHandle))
		error("rget: do_file -> %s", resourceName.c_str());

	if (resourceSize)		    // xi change
		*resourceSize = resEntry->Size;

	term_message("rgetting:%s  -> from disk", resourceName.c_str());
	return resEntry->RHandle;
}


int32 Resources::get_file(const Common::String &name) {
	assert(!_fp);
	_fp = new SysFile(name);
	assert(_fp);

	if (!_fp->exists())
		error("get_file - getting %s", name.c_str());

	return _fp->size();
}

bool Resources::do_file(MemHandle buffer) {
	_fp->seek(0);

	bool result = true;

	// read in resource from file
	if (!_fp->read(buffer)) {
		term_message("unable to read: %s", _fp->filename.c_str());
		result = false;
	}

	delete _fp;
	_fp = nullptr;

	return result;
}

} // namespace M4
