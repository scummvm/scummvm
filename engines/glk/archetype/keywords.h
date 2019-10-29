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

#ifndef ARCHETYPE_KEYWORDS
#define ARCHETYPE_KEYWORDS

#include "common/stream.h"
#include "glk/archetype/array.h"
#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

// max length of any reserved word or operator
const int SHORT_STR_LEN = 9;

enum {
	NUM_RWORDS = 34,
	NUM_OPERS = 32,
	MAX_ELEMENTS = 34		// max(NUM_RWORDS, NUM_OPERS)
};

enum ReservedWordId {
	RW_ABSENT = 1,
	RW_FALSE = 2,
	RW_TRUE = 3,
	RW_UNDEFINED = 4,
	RW_BASED = 5,
	RW_BREAK = 6,
	RW_CASE = 7,
	RW_CREATE = 8,
	RW_DEFAULT = 9,
	RW_DESTROY = 10,
	RW_DO = 11,
	RW_EACH = 12,
	RW_ELSE = 13,
	RW_END = 14,
	RW_FOR = 15,
	RW_IF = 16,
	RW_INCLUDE = 17,
	RW_KEY = 18,
	RW_KEYWORD = 19,
	RW_MESSAGE = 20,
	RW_METHODS = 21,
	RW_NAMED = 22,
	RW_NULL = 23,
	RW_OF = 24,
	RW_ON = 25,
	RW_READ = 26,
	RW_SELF = 27,
	RW_SENDER = 28,
	RW_STOP = 29,
	RW_THEN = 30,
	RW_TYPE = 31,
	RW_WHILE = 32,
	RW_WRITE = 33,
	RW_WRITES = 34
};

enum OperatorId {
	OP_NOP = 0,
	OP_CONCAT = 1,
	OP_C_CONCAT = 2,
	OP_MULTIPLY = 3,
	OP_C_MULTIPLY = 4,
	OP_PLUS = 5,
	OP_C_PLUS = 6,
	OP_MINUS = 7,
	OP_PASS = 8,
	OP_C_MINUS = 9,
	OP_SEND = 10,
	OP_DOT = 11,
	OP_DIVIDE = 12,
	OP_C_DIVIDE = 13,
	OP_ASSIGN = 14,
	OP_LT = 15,
	OP_LE = 16,
	OP_EQ = 17,
	OP_GT = 18,
	OP_GE = 19,
	OP_RANDOM = 20,
	OP_POWER = 21,
	OP_AND = 22,
	OP_CHS = 23,
	OP_LEFTFROM = 24,
	OP_LENGTH = 25,
	OP_NOT = 26,
	OP_NUMERIC = 27,
	OP_OR = 28,
	OP_RIGHTFROM = 29,
	OP_STRING = 30,
	OP_WITHIN = 31,
	OP_NE = 32
};

//typedef char ShortStrType[SHORT_STR_LEN];
typedef const char *const LookupType[MAX_ELEMENTS + 1];

extern LookupType Reserved_Wds, Operators;

// Methods

/**
 * Loads an xarray of test literals into memory from the given file
 */
extern void load_text_list(Common::ReadStream *fIn, XArrayType &the_list);

/**
 * Dumps the given xarray of text literals to the given file
 */
extern void dump_text_list(Common::WriteStream *fOut, XArrayType &the_list);

/**
 * Disposes with all memory associated with the given xarray of text literals
 */
extern void dispose_text_list(XArrayType &the_list);

/**
 * Loads all ID information from the given binary file
 */
extern void load_id_info(Common::ReadStream *bfile);

/**
 * Dumps all ID information to the given binary file
 */
extern void dump_id_info(Common::WriteStream *bfile);

} // End of namespace Archetype
} // End of namespace Glk

#endif
