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

#ifndef ARCHETYPE_INTERPRETER
#define ARCHETYPE_INTERPRETER

#include "glk/archetype/expression.h"
#include "common/stream.h"

namespace Glk {
namespace Archetype {

enum DesiredType { LVALUE, RVALUE, NAME };

typedef ExprNode ResultType;

struct ContextType {
	int sender, self, each, message;

	ContextType() : sender(0), self(0), each(0), message(0) {}
};

extern int MainObject;

extern void interpreter_init();

/**
 * A short wrapper to NewDynStr which basically uses the stack as temporary string storage.
 * If you want to use a string constructor expression as an argument, call this function,
 * since it does not take strings by reference but by value.  Expensive on the stack but
 * only briefly; it saves cluttering eval_expr
 */
extern StringPtr MakeNewDynStr(const String &s);

/**
 * Given a string message, returns its number in the Vocabulary list, or -1 if it was not found.
 * At present, it simply uses a very inefficient O(N) lookup.  If speed begins to become a
 * consideration, this can be changed.
 * @param message		message to find number of
 * @returns				the number of the message in the Vocabulary list.
 */
extern int find_message(const String &message);

/**
 * Converts a scalar expression node to a target type. Deals primarily with numeric -> string
 * or string -> numeric conversions in their many incarnations.
 * @param target_type		type to convert to
 * @param the_scalar		scalar to convert
  */
extern bool convert_to(AclType target_type, ResultType &the_scalar);

/**
 * Used to initialize previously unused result records.  Does not expect that there might be
 * a string pointer lurking within.
 */
extern void undefine(ResultType &result);

/**
 * To be used on temporary result variables after their usefulness is finished.  Like 'undefine' above,
 * except that it is used only for results that have actually been used - in other words, results with
 * their "kind" field set properly.
 */
extern void cleanup(ResultType &result);

/**
 * Does an rvalue-like copy from r2 to r1
 */
extern void copy_result(ResultType &r1, const ResultType &r2);

/**
 * Compares two result nodes according to the given operator.
 * @returns true if they can; false if they cannot
 */
extern bool result_compare(short comparison, ResultType &r1, ResultType &r2);

/**
 * Given the result of an LVALUE evaluation and a result to assign to the attribute,
 * performs the assignment if possible.
 * @param target		hopefully points to attribute to receive assignment
 * @param value			Result to assign
 * @returns				Returns true if the assignment was successful; false otherwise
 */
extern bool assignment(ResultType &target, ResultType &value);

/**
 * Gets a textual version of a passed result
 */
extern String get_result_string(ResultType &result);

/**
 * Writes the given result to screen w/o terminating it with a newline
 */
extern void write_result(ResultType &result);

/**
 * For purposes of debugging.
 * Strings are enclosed in double quotes.
 * Messages are enclosed in single quotes.
 * Quote literals are preceded by >>
 */
extern void display_result(ResultType &result);

/**
 * Given an expression tree, displays the thing on screen.
 */
extern void display_expr(ExprTree the_tree);

/**
 * Loads a game into memory from a binary input file.  Checks for errors
 * in the header or incompatible versions.
 * @param f_in			Input file
 */
extern bool load_game(Common::ReadStream *f_in);

} // End of namespace Archetype
} // End of namespace Glk

#endif
