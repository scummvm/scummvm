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

#ifndef ARCHETYPE_MISC
#define ARCHETYPE_MISC

#include "common/file.h"
#include "glk/archetype/crypt.h"
#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

#define MAX_STRING 255
#define NULL_CH '\0'
#define NEWLINE_CH '\r'

enum AclType {
	RESERVED, IDENT, MESSAGE, OPER, TEXT_LIT, QUOTE_LIT, NUMERIC, PUNCTUATION,
	STR_PTR, ATTR_PTR, BAD_TOKEN, NEWLINE
};

/**
 * Source program file/accounting structure.With such a file, it is important to keep
 * not only the file pointer, but also fields to keep track of position in the source file
 * andthe compiler state, or the context of the tokenizer
 */
class progfile {
private:
	Common::File _file;	// The physical file
public:
	String filename;					// to do with error tracking
	String line_buffer;
	int file_line;
	int line_pos;

	bool newlines;						// having to do with the tokenizer context
	char last_ch;
	bool consumed;
	AclType ttype;
	int tnum;
public:
	/**
	 * Constructor
	 */
	progfile() : file_line(0), line_pos(0), newlines(false), last_ch(NULL_CH),
		consumed(false), ttype(RESERVED), tnum(0) {}

	/**
	 * Opens an Archetype program source file.
	 * @param name			Filename
	 */
	bool open(const String &name);

	/**
	 * Closes an Archetype program source code file.
	 */
	void close();

	/**
	 * Reads a single character from the given progfile, performing all appropriate housekeeping.
	 *
	 * It appends an internal newline to the end of every line taken from the file; it is up to
	 * the tokenizer as to whether to consider it white space or a token.
	 * @param c				The output character
	 * @returns				True if the character was safely read from the file
	 */
	bool readChar(char &ch);

	/**
	 * Has the effect of putting a character back on the data stream.
	 * Closely cooperates with read_char above.
	 * @param ch			Character to un-read
	 */
	void unreadChar(char ch);

	/**
	 * Writes out the current position in the source file nicely for error messages
	 * and so forth.It will, however, only write this out once per execution of the program.
	 * This is to prevent messages scrolling uncontrollably off the screen.
	 */
	void sourcePos();
};

enum ClassifyType { TYPE_ID, OBJECT_ID, ATTRIBUTE_ID, ENUMERATE_ID, UNDEFINED_ID };

extern const char *const VERSION_STUB;
extern const double VERSION_NUM;
extern size_t Bytes;		// Bytes consumed by allocated memory
extern bool KeepLooking;
extern bool AllErrors;

/**
 * Performs initialization of fields local to the file
 */
extern void misc_init();

/**
 * Provides a method of keeping track of the size, in allocation, of the used part of the heap.
 * @param delta		if positive, the number allocated; if negative, the number deallocated.
 */
extern void add_bytes(int delta);

/**
 * Given a name andextension, tacks on the given extension if none given.
 * of <name> are ".<ext>", or else tacks those four characters on.
 * @param name			Filename
 * @param ext			Extension
 * @param replace		Whether to replace existing extension
 */
extern String formatFilename(const String &name, const String &ext, bool replace);

/**
 * Given an input stream, reads in a Pascal style string with the first byte being the length.
 * Secondarily, it also decrypts the string
 * @param fIn			Input file
 * @param the_string	Output string
 */
extern void load_string(Common::ReadStream *fIn, String &the_string);

/**
 * Given an untyped file variable anda string variable, writes to the file first the length
 * of the string andthen the string itself.
 * @param fOut			Output file
 * @param the_string	The string to output
 */
extern void dump_string(Common::WriteStream *fOut, const String &the_string);

/**
 * Used for allocating string space that is not expected to be disposed of
 * until the end of the program andis never expected to change.
 * Only the very minimum space necessary to store the string is used;
 * thereby using minimal space andincurring no fragmentation
 */
extern StringPtr NewConstStr(const String &s);

/**
 * Frees a const string
 */
extern void FreeConstStr(StringPtr &sp);

/**
 * Dynamic strings were originally strings where we need speed and yet we need to allocate
 * only the string space necessary. These days, we can simply just allocate a new string
 */
extern StringPtr NewDynStr(const String &s);

/**
 * Frees a dynamic string
 */
extern void FreeDynStr(StringPtr &sp);

} // End of namespace Archetype
} // End of namespace Glk

#endif
