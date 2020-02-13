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

#include "glk/jacl/constants.h"
#include "common/file.h"

namespace Glk {
namespace JACL {

#define DIR_SEPARATOR '/'
#define DATA_DIR "data/"
#define TEMP_DIR "temp/"
#define INCLUDE_DIR "include/"

// THIS STRUCTURE CONTAINS ALL THE INFORMATION THAT NEEDS TO BE
// SAVED IN ORDER TO CALL parse() RECURSIVELY
struct proxy_type {
	int             object_pointers[4];             // NOUN1 -> NOUN4
	int             integer[MAX_WORDS];             // THE COMANDS INTEGERS
	char            text[MAX_WORDS][256];           // THE COMANDS STRINGS
	char            command[MAX_WORDS][256];        // THE WHOLE COMMAND
	int             object_list[4][MAX_OBJECTS];    // THE RESOLVED OBJECTS
	int             list_size[4];                   // THE SIZE OF THE ABOVE LISTS
	int             max_size[4];                    // THE LAST USED INDEX OF THE ABOVE LISTS
	int             start_of_this_command;          // PREPARSE STATE
	int             start_of_last_command;          // PREPARSE STATE
	int             integercount;                   // THE NUMBER OF INTEGERS SAVED
	int             textcount;                      // THE NUMBER OF STRINGS SAVED
	int             commandcount;                   // THE NUMBER OF WORDS IN COMMAND
	int             last_exact;                     // WORD POINTER FOR MATCH
	int             after_from;                     // WORD POINTER FOR FROM WORD
};

struct stack_type {
	Common::SeekableReadStream *infile;
	Common::WriteStream *outfile;
	int             arguments[MAX_WORDS];
	char            str_arguments[MAX_WORDS][256];
	char            text_buffer[1024];
	char            called_name[1024];
	char            _override[84];
	char            scope_criterion[24];
	char            default_function[84];
	const char      *word[MAX_WORDS];
	int             quoted[MAX_WORDS];
	int             wp;
	int             argcount;
	int             *loop_integer;
	int             *select_integer;
	int             criterion_value;
	int             criterion_type;
	int             criterion_negate;
	int             current_level;
	int             execution_level;
#ifdef GLK
	int             top_of_loop;
	int         top_of_select;
	int         top_of_while;
	int         top_of_iterate;
	int         top_of_update;
	int         top_of_do_loop;
	int          address;
#else
	long            top_of_loop;
	long            top_of_select;
	long            top_of_while;
	long            top_of_iterate;
	long            top_of_update;
	long            top_of_do_loop;
	long            address;
#endif
	struct function_type *function;
};

struct object_type {
	char            label[44];
	char            article[12];
	char            definite[12];
	struct          name_type *first_name;
	struct          name_type *first_plural;
	char            inventory[44];
	char            described[84];
	int             user_attributes;
	int             user_attributes_backup;
	int             attributes;
	int             attributes_backup;
	int             integer[16];
	int             integer_backup[16];
	int             nosave;
};

struct integer_type {
	char            name[44];
	int             value;
	int             value_backup;
	struct integer_type *next_integer;
};

struct cinteger_type {
	char            name[44];
	int             value;
	struct cinteger_type *next_cinteger;
};

struct attribute_type {
	char            name[44];
	int             value;
	struct attribute_type *next_attribute;
};

struct string_type {
	char            name[44];
	char            value[256];
	struct string_type *next_string;
};

struct function_type {
	char            name[84];
#ifdef GLK
	glui32          position;
#else
	long            position;
#endif

	int             self;
	int             call_count;
	int             call_count_backup;
	struct function_type *next_function;
};

struct command_type {
	char            word[44];
	struct command_type *next;
};

#ifdef GLK
struct window_type {
	char            name[44];
	winid_t         glk_window;
	glui32          glk_type;
	struct window_type *next_window;
};
#endif

struct word_type {
	char            word[44];
	struct word_type *first_child;
	struct word_type *next_sibling;
};

struct synonym_type {
	char            original[44];
	char            standard[44];
	struct synonym_type *next_synonym;
};

struct name_type {
	char            name[44];
	struct name_type *next_name;
};

struct filter_type {
	char            word[44];
	struct filter_type *next_filter;
};

#ifndef GLK
struct parameter_type {
	char            name[44];
	char            container[44];
	int             low;
	int             high;
	struct parameter_type *next_parameter;
};
#endif

class File : public Common::File {
public:
	static File *openForReading(const Common::String &name) {
		File *f = new File();
		if (f->open(name))
			return f;

		delete f;
		return nullptr;
	}

	static Common::WriteStream *openForWriting(const Common::String &name) {
		Common::DumpFile *df = new Common::DumpFile();
		if (df->open(name))
			return df;

		delete df;
		return nullptr;
	}
public:
	File() : Common::File() {}
	File(const Common::String &name) {
		Common::File::open(name);
		assert(isOpen());
	}
};

} // End of namespace JACL
} // End of namespace Glk
