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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ARCHETYPE_ID_TABLE
#define ARCHETYPE_ID_TABLE

#include "glk/archetype/misc.h"
#include "glk/archetype/string.h"

/**
 * Contains the necessary data structures andfunctions for adding to andreferring
 * to the ID table(a very busy little structure).
 *
 * The ID table is a 27-element hash table with one bucket for each letter; identifiers
 * are hashed according to their first letter. The last bucket is for identifiers beginning with
 * an underscore. The ID table is cross_indexed by an xarray containing pointers to id_records.
 * The ID table is complex enough that it should probably not be accessed directly but rather
 * only through its procedures.In this way the data type, its primary instantiation, andassociated
 * code comprise one stand-alone module which must be "used" by any module wishing to modify or
 * query the table.
 */
namespace Glk {
namespace Archetype {

const int BUCKETS = 27;		// 1 per letter of alphabet, plus the underscore

struct IdRecType {
	ClassifyType id_kind;
	int id_integer;			// What integer the ID gets written as
	int id_index;			// The ID's index in the ID table
	StringPtr id_name;
	IdRecType *next;

	IdRecType() : id_kind(UNDEFINED_ID), id_integer(0), id_index(0),
		id_name(nullptr), next(nullptr) {}
};
typedef IdRecType *IdRecPtr;

// Public methods
/**
 * Adds the given identifier to the ID table, andreturns its index. There are no duplications;
 * if the identifier already exists, its existing index is returned.
 * @param id_str		String containing identifier name
 * @returns				The index of the identifier
 */
extern int add_ident(const String &id_str);

/**
 * A quick little wrapper to the index_xarray function.
 * @param index		Number of the identifier
 * @param id_ptr	Out pointer to the id_record for that identifier
 * @returns			True if the requested identifier exists in the table
 */
extern bool index_ident(int index, IdRecPtr &id_ptr);

extern ClassifyType DefaultClassification;

} // End of namespace Archetype
} // End of namespace Glk

#endif
