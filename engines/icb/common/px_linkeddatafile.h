/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_LINKED_DATA_FILE_H
#define ICB_PX_LINKED_DATA_FILE_H

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/debug.h"

#include "common/endian.h"

namespace ICB {

// This value is returned as an error condition.
#define PX_LINKED_DATA_FILE_ERROR (0xffffffff)

#define ORDER_PRESERVED_FLAG 1
#define NO_NAMES_FLAG 2
#define SUPPRESS_OUTPUT 4 // Suppress printing of output

typedef struct {
	int32 name_offset;      // offset to Null terminated name of the item
	int32 data_offset;      // Offset to the item
	int32 data_size;        // Size of the items data
	uint32 name_hash_value; // hash value of name item...
} LinkedDataFileEntry;

typedef struct {
	px_standard_header header;

	uint32 number_of_items;
	uint32 flags;

	LinkedDataFileEntry list[1];  // 1 used to prevent 0 sized array warnings
								  // This structure is a variable size and so should never
								  // be a parameter to sizeof anyway
} LinkedDataFile;

class LinkedDataObject { // Should be CObjectFile

public:
	static inline uint32 Fetch_number_of_items(LinkedDataFile *file) { // how many
		return (FROM_LE_32(file->number_of_items));
	}

	static inline uint32 GetHeaderVersion(LinkedDataFile *file) { return (FROM_LE_32(file->header.version)); }

	static inline int32 OrderPreserved(LinkedDataFile *file) { return (FROM_LE_32(file->flags) & (ORDER_PRESERVED_FLAG)); }

	static inline int32 NameSearchable(LinkedDataFile *file) { return (!OrderPreserved(file)); }

	static uint32 Fetch_item_number_by_hash(LinkedDataFile *file, const uint32 hash);
	static uint32 Fetch_item_number_by_name(LinkedDataFile *file, const char *name); // reference a number by its ascii name

	static void *Fetch_item_by_number(LinkedDataFile *file, uint32 number); // reference a resource by number

	static void *Fetch_item_by_name(LinkedDataFile *file, const char *name); // reference a resource by its ascii name

	static void *Fetch_items_name_by_number(LinkedDataFile *file, uint32 number); // return pointer to name of item number n

	static void *Try_fetch_item_by_name(LinkedDataFile *file, const char *);   // reference a resource by name
	static void *Try_fetch_item_by_hash(LinkedDataFile *file, uint32); // reference a resource by hash

	static void *Try_fetch_item_by_script_name(LinkedDataFile *file, const char *name);
};

// get DATA given NUMBER
inline void *LinkedDataObject::Fetch_item_by_number(LinkedDataFile *file, uint32 number) {
	// illegal reference number
	assert(number < FROM_LE_32(file->number_of_items));
	// return address of resource
	return (((uint8 *)&file->header) + FROM_LE_32(file->list[number].data_offset));
}

// get NAME given NUMBER
inline void *LinkedDataObject::Fetch_items_name_by_number(LinkedDataFile *file, uint32 number) {
	// illegal reference number
	assert(number < FROM_LE_32(file->number_of_items));
	// return name
	return (((uint8 *)&file->header) + FROM_LE_32(file->list[number].name_offset));
}

// this is the one that does the search...
// get NUMBER given NAME (does search)
inline uint32 LinkedDataObject::Fetch_item_number_by_name(LinkedDataFile *file, const char *name) {
	uint32 hash;

	if (!NameSearchable(file)) {
		Fatal_error("This file is not searchable by name and was created as such (name %s)", name);
	}

	hash = EngineHashString(name);

	return Fetch_item_number_by_hash(file, hash);
}

// get ITEM given NAME      (uses Try_fetch_item_by_name, fatal error if can't find)
inline void *LinkedDataObject::Fetch_item_by_name(LinkedDataFile *file, const char *name) {
	void *search;

	search = Try_fetch_item_by_name(file, name);

	if (search == 0) {
		Fatal_error("pxLinked_data_file::Fetch_item_by_name Object %s not found", name);
		// Note, for not the engine then the error is not caught which is a bad thing
		// but we need a generic Fatal_error type thing that converters & the engine can call
		// i.e. the converters need a Fatal_error function
		return (NULL);
	} else
		return search;
}

// get DATA given NAME (uses get NUMBER given NAME and returns 0 if not found or uses get DATA given NUMBER to return DATA)
inline void *LinkedDataObject::Try_fetch_item_by_name(LinkedDataFile *file, const char *name) {
	// as Fetch_item_with_name but will return 0 if entry could not be found as opposed to an assert
	uint32 search = 0;

	search = Fetch_item_number_by_name(file, name);

	if (search == PX_LINKED_DATA_FILE_ERROR)
		return 0; // not found (legal)
	else
		return Fetch_item_by_number(file, search);
}

// get DATA given HASH (uses get NUMBER given HASH and returns 0 if not found or uses get DATA given NUMBER to return DATA)
inline void *LinkedDataObject::Try_fetch_item_by_hash(LinkedDataFile *file, uint32 hash) {
	// as Fetch_item_with_name but will return 0 if entry could not be found as opposed to an assert
	uint32 search = 0;

	search = Fetch_item_number_by_hash(file, hash);

	if (search == PX_LINKED_DATA_FILE_ERROR)
		return 0; // not found (legal)
	else
		return Fetch_item_by_number(file, search);
}

inline void *LinkedDataObject::Try_fetch_item_by_script_name(LinkedDataFile *file, const char *name) {
	uint32 search = 0;

	do {
		if (strstr((const char *)((uint8 *)&file->header) + FROM_LE_32(file->list[search].name_offset), (const char *)name))
			return (((uint8 *)&file->header) + FROM_LE_32(file->list[search].data_offset));

		search++;
	} while (search < FROM_LE_32(file->number_of_items));

	// not found at all
	return (0);
}

} // End of namespace ICB

#endif
