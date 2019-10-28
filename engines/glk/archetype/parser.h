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

#ifndef ARCHETYPE_PARSER
#define ARCHETYPE_PARSER

#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

enum TargetListType { PARSER_VERBLIST, PARSER_NOUNLIST };

/**
 * Given a string, creates a string with one and only one space between each word
 * @param first			the string to be normalized
 * @param second		the normalized string
 */
extern void normalize_string(const String &first, String &second);

/**
 * Adds another word to one of the lists to match.  If the given word has vertical bars in it,
 * the bars are considered delimiters and each delimited word is added to the available list
 */
extern void add_parse_word(TargetListType which_list, String &the_word, int the_object);

/**
 * Parses the previously given sentence into a string of object references.
 * The verbpreps list is searched first, followed by the nounphrases list.
 * It does not identify any parts of speech; it is strictly substitutional.
 *
 * Also removes all instances of the words "a", "an", "the".
 *
 * NOTES:
 * When an object is matched, its name is replaced by the sequence
 * <percent sign><high byte><low byte><caret>.  The percent will
 * indicate the beginning of the encoded number; the caret indicates
 * the end and also serves the purpose of keeping the trim() procedure
 * from trimming off objects 9 or 13 or the like.
 *
 * Objects are matched as words; they must be surrounded by spaces.
 * When they are replaced in the Command string, they leave the spaces
 * on both sides so as not to interfere with further matching
 */
extern void parse_sentence();

/**
 * Pops the first object number off of the parsed Command string and sends the number back.
 * If Command does not begin with an object marker, sends back the unparseable string.
 * @param intback		will be -1 if there was no object; otherwise, the number of the object.
 * @param strback		will contain the (trimmed) unparseable chunk if intback is -1; unchanged otherwise
 * @returns				true if there was anything to be popped; false otherwise
 */
extern bool pop_object(int &intback, String &strback);

/**
 * Performs a subset of the normal parse_sentence algorithm.  Given a single string,
 * find the number of the first object that matches.
 */
extern int find_object(const String &s);

/**
 * Called in order to force a full deletion of the parse lists, in order that new ones may be built up
 */
extern void new_parse_list();

} // End of namespace Archetype
} // End of namespace Glk

#endif
